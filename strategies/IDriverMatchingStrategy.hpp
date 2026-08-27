#pragma once

#include "../models/DomainModels.hpp"
#include <memory>
#include <vector>
using namespace std;

namespace cab {
class IDriverMatchingStrategy {
public:
  virtual ~IDriverMatchingStrategy() = default;
  virtual shared_ptr<Driver> match(const RideRequest &, const vector<shared_ptr<Driver>> &) const = 0;
};
} // namespace cab
