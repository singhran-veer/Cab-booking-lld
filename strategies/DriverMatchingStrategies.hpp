#pragma once
#include "IDriverMatchingStrategy.hpp"
using namespace std;

namespace cab {
class NearestDriverStrategy final : public IDriverMatchingStrategy {
public:
  shared_ptr<Driver> match(const RideRequest &, const vector<shared_ptr<Driver>> &) const override;
};
class HighestRatedDriverStrategy final : public IDriverMatchingStrategy {
public:
  shared_ptr<Driver> match(const RideRequest &, const vector<shared_ptr<Driver>> &) const override;
};
class BestETAStrategy final : public IDriverMatchingStrategy {
public:
  shared_ptr<Driver> match(const RideRequest &, const vector<shared_ptr<Driver>> &) const override;
};
} // namespace cab
