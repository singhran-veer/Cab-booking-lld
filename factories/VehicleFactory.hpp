#pragma once

#include "../models/DomainModels.hpp"
#include <memory>
#include <string>
using namespace std;

namespace cab {
class VehicleFactory {
public:
  static shared_ptr<Vehicle> create(VehicleId, VehicleType, const string &);
};
} // namespace cab