#pragma once

#include "CabBookingFacade.hpp"
using namespace std;


class CabBookingApplication {
public:
  static int run() {
    CabBookingFacade system;

    const auto rider = system.registerRider("Alice", "+91-1000");
    const auto driver = system.registerDriver("Bob", "+91-2000");
    system.registerVehicle(driver, cab::VehicleType::Premium, "CAB-001");
    system.updateLocation(driver, {12.97, 77.59});
    system.goOnline(driver);

    const auto ride = system.requestRide(rider, {12.97, 77.59}, {12.99, 77.62}, cab::VehicleType::Premium);
    system.acceptRide(driver, ride);
    system.driverArrived(driver, ride);
    system.startRide(driver, ride);
    system.completeRide(driver, ride);
    system.pay(ride, cab::PaymentMethod::Card);
    system.rateDriver(rider, ride, 4);

    return 0;
  }
};