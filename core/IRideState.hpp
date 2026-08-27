#pragma once

#include "../models/DomainModels.hpp"
#include <memory>
#include <stdexcept>
using namespace std;


namespace cab {
class IRideState {
public:
  virtual ~IRideState() = default;
  virtual RideStatus status() const = 0;
  virtual bool allows(RideStatus) const = 0;
};
class RequestedState final : public IRideState {
public:
  RideStatus status() const override { return RideStatus::Requested; }
  bool allows(RideStatus n) const override { return n == RideStatus::SearchingDriver || n == RideStatus::Cancelled; }
};
class SearchingDriverState final : public IRideState {
public:
  RideStatus status() const override { return RideStatus::SearchingDriver; }
  bool allows(RideStatus n) const override { return n == RideStatus::DriverAssigned || n == RideStatus::Cancelled; }
};
class DriverAssignedState final : public IRideState {
public:
  RideStatus status() const override { return RideStatus::DriverAssigned; }
  bool allows(RideStatus n) const override { return n == RideStatus::DriverArriving || n == RideStatus::Cancelled; }
};
class DriverArrivingState final : public IRideState {
public:
  RideStatus status() const override { return RideStatus::DriverArriving; }
  bool allows(RideStatus n) const override { return n == RideStatus::DriverArrived || n == RideStatus::Cancelled; }
};
class DriverArrivedState final : public IRideState {
public:
  RideStatus status() const override { return RideStatus::DriverArrived; }
  bool allows(RideStatus n) const override { return n == RideStatus::RideStarted; }
};
class RideStartedState final : public IRideState {
public:
  RideStatus status() const override { return RideStatus::RideStarted; }
  bool allows(RideStatus n) const override { return n == RideStatus::RideCompleted; }
};
class RideCompletedState final : public IRideState {
public:
  RideStatus status() const override { return RideStatus::RideCompleted; }
  bool allows(RideStatus) const override { return false; }
};
class CancelledState final : public IRideState {
public:
  RideStatus status() const override { return RideStatus::Cancelled; }
  bool allows(RideStatus) const override { return false; }
};
class Ride {
public:
  Ride(RideId, RideRequest, Fare);
  RideId id() const;
  RideStatus status() const;
  const RideRequest &request() const;
  const Fare &fare() const;
  optional<DriverId> driverId() const;
  void beginSearch();
  void assign(DriverId);
  void accept();
  void arrive();
  void start();
  void complete();
  void cancel();

private:
  void transition(RideStatus);
  RideId id_;
  RideRequest request_;
  Fare fare_;
  unique_ptr<IRideState> state_{make_unique<RequestedState>()};
  optional<DriverId> driverId_;
};
} // namespace cab
