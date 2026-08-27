#pragma once

#include "../enums/DomainEnums.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
using namespace std;


namespace cab {
using RiderId = uint64_t;
using DriverId = uint64_t;
using VehicleId = uint64_t;
using RideId = uint64_t;
struct Location {
  double latitude{};
  double longitude{};
  double distanceTo(const Location &) const;
};
struct Money {
  double amount{};
  string currency{"INR"};
};
class User {
public:
  User(uint64_t id, string name, string phone)
      : id_(id), name_(move(name)), phone_(move(phone)) {}
  virtual ~User() = default;
  uint64_t id() const { return id_; }
  const string &name() const { return name_; }

private:
  uint64_t id_;
  string name_;
  string phone_;
};
class Rider final : public User {
public:
  using User::User;
  void addRide(RideId id) { rideHistory_.push_back(id); }
  const vector<RideId> &rideHistory() const { return rideHistory_; }

private:
  vector<RideId> rideHistory_;
};
class Vehicle {
public:
  Vehicle(VehicleId id, VehicleType type, string registration)
      : id_(id), type_(type), registration_(move(registration)) {}
  virtual ~Vehicle() = default;
  VehicleId id() const { return id_; }
  VehicleType type() const { return type_; }
  virtual double rateMultiplier() const = 0;

private:
  VehicleId id_;
  VehicleType type_;
  string registration_;
};
class Hatchback final : public Vehicle {
public:
  using Vehicle::Vehicle;
  double rateMultiplier() const override { return 1.0; }
};
class Sedan final : public Vehicle {
public:
  using Vehicle::Vehicle;
  double rateMultiplier() const override { return 1.2; }
};
class SUV final : public Vehicle {
public:
  using Vehicle::Vehicle;
  double rateMultiplier() const override { return 1.5; }
};
class PremiumVehicle final : public Vehicle {
public:
  using Vehicle::Vehicle;
  double rateMultiplier() const override { return 2.0; }
};
class Driver final : public User {
public:
  using User::User;
  void setVehicle(shared_ptr<Vehicle> vehicle) { vehicle_ = move(vehicle); }
  shared_ptr<Vehicle> vehicle() const { return vehicle_; }
  void setStatus(DriverStatus status) { status_ = status; }
  DriverStatus status() const { return status_; }
  void setLocation(Location location) { location_ = location; }
  Location location() const { return location_; }
  void addRating(int score) {
    ratingTotal_ += score;
    ++ratingCount_;
  }
  double rating() const { return ratingCount_ == 0 ? 5.0 : static_cast<double>(ratingTotal_) / ratingCount_; }

private:
  DriverStatus status_{DriverStatus::Offline};
  Location location_{};
  shared_ptr<Vehicle> vehicle_;
  int ratingTotal_{};
  int ratingCount_{};
};
struct RideRequest {
  RiderId riderId{};
  Location pickup{};
  Location destination{};
  VehicleType vehicleType{};
};
struct Fare {
  Money amount{};
  double distanceKm{};
  double durationMinutes{};
};
struct Payment {
  PaymentStatus status{PaymentStatus::Pending};
  PaymentMethod method{};
  Money amount{};
};
struct Rating {
  RideId rideId{};
  RiderId riderId{};
  DriverId driverId{};
  int score{};
  string comment;
};
} // namespace cab