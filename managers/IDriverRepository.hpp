#pragma once

#include "../models/DomainModels.hpp"
#include <mutex>
#include <unordered_map>
using namespace std;

namespace cab {
class IDriverRepository {
public:
  virtual ~IDriverRepository() = default;
  virtual void add(shared_ptr<Driver>) = 0;
  virtual shared_ptr<Driver> find(DriverId) const = 0;
  virtual vector<shared_ptr<Driver>> available() const = 0;
  virtual bool reserve(DriverId, RideId) = 0;
  virtual void release(DriverId) = 0;
  virtual void setOnline(DriverId, bool) = 0;
  virtual void updateLocation(DriverId, Location) = 0;
};
class InMemoryDriverRepository final : public IDriverRepository {
public:
  void add(shared_ptr<Driver>) override;
  shared_ptr<Driver> find(DriverId) const override;
  vector<shared_ptr<Driver>> available() const override;
  bool reserve(DriverId, RideId) override;
  void release(DriverId) override;
  void setOnline(DriverId, bool) override;
  void updateLocation(DriverId, Location) override;

private:
  mutable mutex mutex_;
  unordered_map<DriverId, shared_ptr<Driver>> drivers_;
  unordered_map<DriverId, RideId> reservations_;
};
} // namespace cab
