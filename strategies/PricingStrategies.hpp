#pragma once
#include "IPricingStrategy.hpp"
using namespace std;

namespace cab {
class NormalPricingStrategy final : public IPricingStrategy {
public:
  Fare calculate(const RideRequest &, const Vehicle &) const override;
};
class SurgePricingStrategy final : public IPricingStrategy {
public:
  explicit SurgePricingStrategy(double multiplier) : multiplier_(multiplier) {}
  Fare calculate(const RideRequest &, const Vehicle &) const override;

private:
  double multiplier_;
};
class PeakHourPricingStrategy final : public IPricingStrategy {
public:
  Fare calculate(const RideRequest &, const Vehicle &) const override;
};
class DiscountPricingStrategy final : public IPricingStrategy {
public:
  explicit DiscountPricingStrategy(double discount) : discount_(discount) {}
  Fare calculate(const RideRequest &, const Vehicle &) const override;

private:
  double discount_;
};
} // namespace cab
