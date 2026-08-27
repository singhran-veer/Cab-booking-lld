#pragma once

#include "../../common/Exceptions.hpp"
#include "../../core/IRideState.hpp"
#include "../../factories/VehicleFactory.hpp"
#include "../../managers/IDriverRepository.hpp"
#include "../../managers/INotificationService.hpp"
#include "../../models/DomainModels.hpp"
#include "../../payment/PaymentStrategies.hpp"
#include "../../strategies/DriverMatchingStrategies.hpp"
#include "../../strategies/IPricingStrategy.hpp"
#include "../../strategies/PricingStrategies.hpp"
using namespace std;


namespace cab {
class CabBookingSystem {
public:
  explicit CabBookingSystem(
      shared_ptr<IDriverRepository> drivers = make_shared<InMemoryDriverRepository>(),
      shared_ptr<IDriverMatchingStrategy> matching = make_shared<NearestDriverStrategy>(),
      shared_ptr<IPricingStrategy> pricing = make_shared<NormalPricingStrategy>(),
      shared_ptr<INotificationService> notifications = make_shared<NotificationService>());
  RiderId registerRider(string, string);
  DriverId registerDriver(string, string);
  VehicleId registerVehicle(DriverId, VehicleType, string);
  void goOnline(DriverId);
  void goOffline(DriverId);
  void updateLocation(DriverId, Location);
  RideId requestRide(RiderId, Location, Location, VehicleType);
  void acceptRide(DriverId, RideId);
  void driverArrived(DriverId, RideId);
  void startRide(DriverId, RideId);
  void completeRide(DriverId, RideId);
  void cancelRide(RiderId, RideId);
  Fare estimateFare(RideId) const;
  void pay(RideId, PaymentMethod);
  void rateDriver(RiderId, RideId, int);
  shared_ptr<Ride> findRide(RideId) const;

private:
  shared_ptr<IDriverRepository> drivers_;
  shared_ptr<IDriverMatchingStrategy> matching_;
  shared_ptr<IPricingStrategy> pricing_;
  shared_ptr<INotificationService> notifications_;
  unordered_map<RiderId, shared_ptr<Rider>> riders_;
  unordered_map<DriverId, shared_ptr<Driver>> driverObjects_;
  unordered_map<RideId, shared_ptr<Ride>> rides_;
  mutable mutex mutex_;
  uint64_t nextId_{1};
};
} // namespace cab
