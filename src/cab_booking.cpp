#include "../include/cab_booking/cab_booking.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
using namespace std;


namespace cab {

Ride::Ride(RideId id, RideRequest request, Fare fare) : id_(id), request_(move(request)), fare_(move(fare)) {}
RideId Ride::id() const { return id_; }
RideStatus Ride::status() const { return state_->status(); }
const RideRequest &Ride::request() const { return request_; }
const Fare &Ride::fare() const { return fare_; }
optional<DriverId> Ride::driverId() const { return driverId_; }

double Location::distanceTo(const Location &other) const {
  const auto dx = latitude - other.latitude;
  const auto dy = longitude - other.longitude;
  return sqrt(dx * dx + dy * dy) * 111.0;
}

shared_ptr<Vehicle> VehicleFactory::create(VehicleId id, VehicleType type, const string &registration) {
  switch (type) {
  case VehicleType::Hatchback:
    return make_shared<Hatchback>(id, type, registration);
  case VehicleType::Sedan:
    return make_shared<Sedan>(id, type, registration);
  case VehicleType::SUV:
    return make_shared<SUV>(id, type, registration);
  case VehicleType::Premium:
    return make_shared<PremiumVehicle>(id, type, registration);
  }
  throw DomainException("unsupported vehicle type");
}

void InMemoryDriverRepository::add(shared_ptr<Driver> driver) {
  lock_guard lock(mutex_);
  if (!drivers_.emplace(driver->id(), move(driver)).second)
    throw DomainException("duplicate driver");
}
shared_ptr<Driver> InMemoryDriverRepository::find(DriverId id) const {
  lock_guard lock(mutex_);
  auto it = drivers_.find(id);
  return it == drivers_.end() ? nullptr : it->second;
}
vector<shared_ptr<Driver>> InMemoryDriverRepository::available() const {
  lock_guard lock(mutex_);
  vector<shared_ptr<Driver>> result;
  for (const auto &[id, driver] : drivers_)
    if (driver->status() == DriverStatus::Available)
      result.push_back(driver);
  return result;
}
bool InMemoryDriverRepository::reserve(DriverId id, RideId rideId) {
  lock_guard lock(mutex_);
  auto it = drivers_.find(id);
  if (it == drivers_.end())
    throw DriverNotFoundException("driver not found");
  if (it->second->status() != DriverStatus::Available)
    return false;
  it->second->setStatus(DriverStatus::Reserved);
  reservations_[id] = rideId;
  return true;
}
void InMemoryDriverRepository::release(DriverId id) {
  lock_guard lock(mutex_);
  auto it = drivers_.find(id);
  if (it == drivers_.end())
    throw DriverNotFoundException("driver not found");
  it->second->setStatus(DriverStatus::Available);
  reservations_.erase(id);
}
void InMemoryDriverRepository::setOnline(DriverId id, bool online) {
  lock_guard lock(mutex_);
  auto it = drivers_.find(id);
  if (it == drivers_.end())
    throw DriverNotFoundException("driver not found");
  if (it->second->vehicle() == nullptr)
    throw DomainException("driver needs a vehicle");
  if (it->second->status() == DriverStatus::Reserved || it->second->status() == DriverStatus::OnTrip)
    throw DriverAlreadyBusyException("driver is busy");
  it->second->setStatus(online ? DriverStatus::Available : DriverStatus::Offline);
}
void InMemoryDriverRepository::updateLocation(DriverId id, Location location) {
  lock_guard lock(mutex_);
  auto it = drivers_.find(id);
  if (it == drivers_.end())
    throw DriverNotFoundException("driver not found");
  it->second->setLocation(location);
}

shared_ptr<Driver> NearestDriverStrategy::match(const RideRequest &request,
                                                     const vector<shared_ptr<Driver>> &drivers) const {
  shared_ptr<Driver> best;
  double distance = numeric_limits<double>::max();
  for (const auto &driver : drivers)
    if (driver->vehicle()->type() == request.vehicleType && driver->location().distanceTo(request.pickup) < distance) {
      best = driver;
      distance = driver->location().distanceTo(request.pickup);
    }
  return best;
}
shared_ptr<Driver> HighestRatedDriverStrategy::match(const RideRequest &request,
                                                          const vector<shared_ptr<Driver>> &drivers) const {
  shared_ptr<Driver> best;
  for (const auto &driver : drivers)
    if (driver->vehicle()->type() == request.vehicleType && (!best || driver->rating() > best->rating()))
      best = driver;
  return best;
}
shared_ptr<Driver> BestETAStrategy::match(const RideRequest &request,
                                               const vector<shared_ptr<Driver>> &drivers) const {
  return NearestDriverStrategy{}.match(request, drivers);
}

Fare NormalPricingStrategy::calculate(const RideRequest &request, const Vehicle &vehicle) const {
  const auto distance = request.pickup.distanceTo(request.destination);
  return {{100.0 + distance * 15.0 * vehicle.rateMultiplier(), "INR"}, distance, distance * 2.0};
}
Fare SurgePricingStrategy::calculate(const RideRequest &request, const Vehicle &vehicle) const {
  auto fare = NormalPricingStrategy{}.calculate(request, vehicle);
  fare.amount.amount *= multiplier_;
  return fare;
}
Fare PeakHourPricingStrategy::calculate(const RideRequest &request, const Vehicle &vehicle) const {
  return SurgePricingStrategy{1.25}.calculate(request, vehicle);
}
Fare DiscountPricingStrategy::calculate(const RideRequest &request, const Vehicle &vehicle) const {
  auto fare = NormalPricingStrategy{}.calculate(request, vehicle);
  fare.amount.amount = max(0.0, fare.amount.amount - discount_);
  return fare;
}

Payment ProviderPaymentStrategy::process(const Money &amount, PaymentMethod method) {
  const auto result = provider_->charge(amount, method);
  return {result.success ? PaymentStatus::Completed : PaymentStatus::Failed, method, amount};
}

Payment CashPaymentStrategy::process(const Money &amount, PaymentMethod method) {
  return {PaymentStatus::Completed, method, amount};
}

void Ride::transition(RideStatus next) {
  if (!state_->allows(next))
    throw InvalidRideStateException("invalid ride transition");
  switch (next) {
  case RideStatus::SearchingDriver:
    state_ = make_unique<SearchingDriverState>();
    break;
  case RideStatus::DriverAssigned:
    state_ = make_unique<DriverAssignedState>();
    break;
  case RideStatus::DriverArriving:
    state_ = make_unique<DriverArrivingState>();
    break;
  case RideStatus::DriverArrived:
    state_ = make_unique<DriverArrivedState>();
    break;
  case RideStatus::RideStarted:
    state_ = make_unique<RideStartedState>();
    break;
  case RideStatus::RideCompleted:
    state_ = make_unique<RideCompletedState>();
    break;
  case RideStatus::Cancelled:
    state_ = make_unique<CancelledState>();
    break;
  case RideStatus::Requested:
    state_ = make_unique<RequestedState>();
    break;
  }
}
void Ride::beginSearch() { transition(RideStatus::SearchingDriver); }
void Ride::assign(DriverId id) {
  transition(RideStatus::DriverAssigned);
  driverId_ = id;
}
void Ride::accept() { transition(RideStatus::DriverArriving); }
void Ride::arrive() { transition(RideStatus::DriverArrived); }
void Ride::start() { transition(RideStatus::RideStarted); }
void Ride::complete() { transition(RideStatus::RideCompleted); }
void Ride::cancel() { transition(RideStatus::Cancelled); }

CabBookingSystem::CabBookingSystem(shared_ptr<IDriverRepository> drivers,
                                   shared_ptr<IDriverMatchingStrategy> matching,
                                   shared_ptr<IPricingStrategy> pricing,
                                   shared_ptr<INotificationService> notifications)
    : drivers_(move(drivers)), matching_(move(matching)), pricing_(move(pricing)),
      notifications_(move(notifications)) {
      notifications_->subscribe(make_shared<ConsoleNotificationObserver>());
    }
RiderId CabBookingSystem::registerRider(string name, string phone) {
  lock_guard lock(mutex_);
  auto id = nextId_++;
  riders_.emplace(id, make_shared<Rider>(id, move(name), move(phone)));
  notifications_->publish({0, RideStatus::Requested, "Rider registered (ID: " + to_string(id) + ")"});
  return id;
}
DriverId CabBookingSystem::registerDriver(string name, string phone) {
  lock_guard lock(mutex_);
  auto id = nextId_++;
  auto driver = make_shared<Driver>(id, move(name), move(phone));
  driverObjects_[id] = driver;
  drivers_->add(driver);
  notifications_->publish({0, RideStatus::Requested, "Driver registered (ID: " + to_string(id) + ")"});
  return id;
}
VehicleId CabBookingSystem::registerVehicle(DriverId id, VehicleType type, string registration) {
  lock_guard lock(mutex_);
  auto it = driverObjects_.find(id);
  if (it == driverObjects_.end())
    throw DriverNotFoundException("driver not found");
  auto vehicle = VehicleFactory::create(nextId_++, type, registration);
  it->second->setVehicle(vehicle);
  notifications_->publish({0, RideStatus::Requested, "Vehicle registered (ID: " + to_string(vehicle->id()) + ")"});
  return vehicle->id();
}
void CabBookingSystem::goOnline(DriverId id) {
  drivers_->setOnline(id, true);
  notifications_->publish({0, RideStatus::Requested, "Driver is online and available (ID: " + to_string(id) + ")"});
}
void CabBookingSystem::goOffline(DriverId id) {
  drivers_->setOnline(id, false);
  notifications_->publish({0, RideStatus::Requested, "Driver is offline (ID: " + to_string(id) + ")"});
}
void CabBookingSystem::updateLocation(DriverId id, Location location) {
  drivers_->updateLocation(id, location);
  notifications_->publish({0, RideStatus::Requested, "Driver location updated (ID: " + to_string(id) + ")"});
}
RideId CabBookingSystem::requestRide(RiderId rider, Location pickup, Location destination, VehicleType type) {
  lock_guard lock(mutex_);
  auto riderIt = riders_.find(rider);
  if (riderIt == riders_.end())
    throw RiderNotFoundException("rider not found");
  RideRequest request{rider, pickup, destination, type};
  auto candidates = drivers_->available();
  auto driver = matching_->match(request, candidates);
  if (!driver)
    throw NoDriverAvailableException("no suitable driver");
  auto fare = pricing_->calculate(request, *driver->vehicle());
  auto id = nextId_++;
  auto ride = make_shared<Ride>(id, request, fare);
  notifications_->publish({id, ride->status(), "Ride REQUESTED"});
  ride->beginSearch();
  notifications_->publish({id, ride->status(), "Ride SEARCHING_DRIVER"});
  if (!drivers_->reserve(driver->id(), id))
    throw NoDriverAvailableException("driver was reserved");
  ride->assign(driver->id());
  rides_[id] = ride;
  riderIt->second->addRide(id);
  notifications_->publish({id, ride->status(), "Ride DRIVER_ASSIGNED (Driver ID: " + to_string(driver->id()) + ")"});
  return id;
}
shared_ptr<Ride> CabBookingSystem::findRide(RideId id) const {
  lock_guard lock(mutex_);
  auto it = rides_.find(id);
  if (it == rides_.end())
    throw RideNotFoundException("ride not found");
  return it->second;
}
void CabBookingSystem::acceptRide(DriverId driver, RideId ride) {
  auto r = findRide(ride);
  if (r->driverId() != driver)
    throw DomainException("driver is not assigned");
  r->accept();
  notifications_->publish({ride, r->status(), "Ride DRIVER_ARRIVING"});
}
void CabBookingSystem::driverArrived(DriverId driver, RideId ride) {
  auto r = findRide(ride);
  if (r->driverId() != driver)
    throw DomainException("driver is not assigned");
  r->arrive();
  notifications_->publish({ride, r->status(), "Ride DRIVER_ARRIVED"});
}
void CabBookingSystem::startRide(DriverId driver, RideId ride) {
  auto r = findRide(ride);
  if (r->driverId() != driver)
    throw DomainException("driver is not assigned");
  r->start();
  drivers_->find(driver)->setStatus(DriverStatus::OnTrip);
  notifications_->publish({ride, r->status(), "Ride RIDE_STARTED"});
}
void CabBookingSystem::completeRide(DriverId driver, RideId ride) {
  auto r = findRide(ride);
  if (r->driverId() != driver)
    throw DomainException("driver is not assigned");
  r->complete();
  drivers_->release(driver);
  notifications_->publish({ride, r->status(), "Ride RIDE_COMPLETED"});
}
void CabBookingSystem::cancelRide(RiderId rider, RideId ride) {
  auto r = findRide(ride);
  if (r->request().riderId != rider)
    throw DomainException("rider is not owner");
  r->cancel();
  if (r->driverId())
    drivers_->release(*r->driverId());
  notifications_->publish({ride, r->status(), "Ride CANCELLED"});
}
Fare CabBookingSystem::estimateFare(RideId id) const { return findRide(id)->fare(); }
void CabBookingSystem::pay(RideId id, PaymentMethod method) {
  auto ride = findRide(id);
  if (ride->status() != RideStatus::RideCompleted)
    throw InvalidPaymentException("ride is not complete");
  unique_ptr<IPaymentStrategy> strategy;
  switch (method) {
  case PaymentMethod::Cash:
    strategy = make_unique<CashPaymentStrategy>();
    break;
  case PaymentMethod::Card:
    strategy = make_unique<CardPaymentStrategy>();
    break;
  case PaymentMethod::UPI:
    strategy = make_unique<UPIPaymentStrategy>();
    break;
  case PaymentMethod::Wallet:
    strategy = make_unique<WalletPaymentStrategy>();
    break;
  }
  if (strategy->process(ride->fare().amount, method).status != PaymentStatus::Completed)
    throw InvalidPaymentException("payment failed");
  notifications_->publish({id, ride->status(), "Payment COMPLETED"});
}
void CabBookingSystem::rateDriver(RiderId rider, RideId id, int score) {
  if (score < 1 || score > 5)
    throw InvalidRatingException("rating must be between 1 and 5");
  auto ride = findRide(id);
  if (ride->request().riderId != rider || !ride->driverId())
    throw InvalidRatingException("invalid rating subject");
  drivers_->find(*ride->driverId())->addRating(score);
  notifications_->publish({id, ride->status(), "Driver rated " + to_string(score) + "/5"});
}

} // namespace cab
