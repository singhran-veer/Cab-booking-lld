#pragma once

#include "../models/DomainModels.hpp"
#include <iostream>
using namespace std;

namespace cab {
struct RideEvent {
  RideId rideId{};
  RideStatus status{};
  string message;
};
class INotificationObserver {
public:
  virtual ~INotificationObserver() = default;
  virtual void onEvent(const RideEvent &) = 0;
};
class LoggingObserver final : public INotificationObserver {
public:
  void onEvent(const RideEvent &event) override { lastEvent_ = event; }

private:
  optional<RideEvent> lastEvent_;
};

class ConsoleNotificationObserver final : public INotificationObserver {
public:
  void onEvent(const RideEvent &event) override {
    cout << event.message;

    if (event.rideId != 0) {
      cout << " [Ride ID: " << event.rideId << "]";
    }

    cout << '\n';
  }
};
} // namespace cab
