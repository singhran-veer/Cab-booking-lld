#pragma once
#include <stdexcept>
using namespace std;

namespace cab {
class DomainException : public runtime_error {
  using runtime_error::runtime_error;
};
class RiderNotFoundException : public DomainException {
  using DomainException::DomainException;
};
class DriverNotFoundException : public DomainException {
  using DomainException::DomainException;
};
class RideNotFoundException : public DomainException {
  using DomainException::DomainException;
};
class NoDriverAvailableException : public DomainException {
  using DomainException::DomainException;
};
class InvalidRideStateException : public DomainException {
  using DomainException::DomainException;
};
class DriverOfflineException : public DomainException {
  using DomainException::DomainException;
};
class DriverAlreadyBusyException : public DomainException {
  using DomainException::DomainException;
};
class InvalidPaymentException : public DomainException {
  using DomainException::DomainException;
};
class InvalidRatingException : public DomainException {
  using DomainException::DomainException;
};
} // namespace cab
