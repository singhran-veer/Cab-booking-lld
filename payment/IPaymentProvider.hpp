#pragma once

#include "../models/DomainModels.hpp"
using namespace std;

namespace cab {
struct PaymentResult {
  bool success{};
  string providerReference;
};
class IPaymentProvider {
public:
  virtual ~IPaymentProvider() = default;
  virtual PaymentResult charge(const Money &, PaymentMethod) = 0;
};
} // namespace cab
