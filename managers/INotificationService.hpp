#pragma once

#include "INotificationObserver.hpp"
using namespace std;

namespace cab {
class INotificationService {
public:
  virtual ~INotificationService() = default;
  virtual void subscribe(shared_ptr<INotificationObserver>) = 0;
  virtual void publish(const RideEvent &) = 0;
};
class NotificationService final : public INotificationService {
public:
  void subscribe(shared_ptr<INotificationObserver> observer) override {
    observers_.push_back(move(observer));
  }
  void publish(const RideEvent &event) override {
    for (const auto &observer : observers_)
      observer->onEvent(event);
  }

private:
  vector<shared_ptr<INotificationObserver>> observers_;
};
} // namespace cab
