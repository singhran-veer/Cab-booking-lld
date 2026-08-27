#pragma once
#include "../strategies/IPaymentStrategy.hpp"
#include "IPaymentProvider.hpp"
using namespace std;

namespace cab {
class ProviderPaymentStrategy : public IPaymentStrategy {
public:
  explicit ProviderPaymentStrategy(shared_ptr<IPaymentProvider> provider) : provider_(move(provider)) {}
  Payment process(const Money &, PaymentMethod) override;

private:
  shared_ptr<IPaymentProvider> provider_;
};
class CashPaymentStrategy final : public IPaymentStrategy {
public:
  Payment process(const Money &, PaymentMethod) override;
};
class MockStripeProvider final : public IPaymentProvider {
public:
  PaymentResult charge(const Money &amount, PaymentMethod) override { return {amount.amount > 0, "stripe-mock"}; }
};
class MockRazorpayProvider final : public IPaymentProvider {
public:
  PaymentResult charge(const Money &amount, PaymentMethod) override { return {amount.amount > 0, "razorpay-mock"}; }
};
class StripeAdapter final : public IPaymentProvider {
public:
  explicit StripeAdapter(shared_ptr<IPaymentProvider> provider) : provider_(move(provider)) {}
  PaymentResult charge(const Money &amount, PaymentMethod method) override { return provider_->charge(amount, method); }

private:
  shared_ptr<IPaymentProvider> provider_;
};
class CardPaymentStrategy final : public ProviderPaymentStrategy {
public:
  CardPaymentStrategy()
      : ProviderPaymentStrategy(make_shared<StripeAdapter>(make_shared<MockStripeProvider>())) {}
};
class UPIPaymentStrategy final : public ProviderPaymentStrategy {
public:
  UPIPaymentStrategy() : ProviderPaymentStrategy(make_shared<MockRazorpayProvider>()) {}
};
class WalletPaymentStrategy final : public ProviderPaymentStrategy {
public:
  WalletPaymentStrategy() : ProviderPaymentStrategy(make_shared<MockRazorpayProvider>()) {}
};
} // namespace cab
