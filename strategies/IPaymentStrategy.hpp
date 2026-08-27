#pragma once

#include "../models/DomainModels.hpp"
using namespace std;

namespace cab {
class IPaymentStrategy {
public:
  virtual ~IPaymentStrategy() = default;
  virtual Payment process(const Money &, PaymentMethod) = 0;
};
} // namespace cab
