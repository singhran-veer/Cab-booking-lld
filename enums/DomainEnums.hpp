using namespace std;

#pragma once

namespace cab {
enum class VehicleType { Hatchback, Sedan, SUV, Premium };
enum class DriverStatus { Offline, Available, Reserved, OnTrip };
enum class RideStatus {
  Requested,
  SearchingDriver,
  DriverAssigned,
  DriverArriving,
  DriverArrived,
  RideStarted,
  RideCompleted,
  Cancelled
};
enum class PaymentStatus { Pending, Processing, Completed, Failed, Refunded };
enum class PaymentMethod { Cash, Card, UPI, Wallet };
} // namespace cab