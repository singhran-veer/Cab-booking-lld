#pragma once

#include "../models/DomainModels.hpp"
using namespace std;

namespace cab {
class IPricingStrategy {
public:
  virtual ~IPricingStrategy() = default;
  virtual Fare calculate(const RideRequest &, const Vehicle &) const = 0;
};
} // namespace cab
