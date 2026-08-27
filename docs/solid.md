# SOLID in This Project

## Single Responsibility

`Ride` owns ride data and delegates lifecycle transitions. Pricing strategies calculate fares, repositories store entities, the driver registry synchronizes availability, payment strategies process payments, and notification services publish events. Without this separation, one manager would become difficult to test and change.

## Open/Closed

`IDriverMatchingStrategy`, `IPricingStrategy`, `IPaymentStrategy`, `Vehicle`, and `INotificationObserver` allow new behavior through new classes. Adding a rating-based matching algorithm should not require modifying `RideService` or the facade.

## Liskov Substitution

Every vehicle implements `rateMultiplier`, every pricing strategy returns a `Fare`, and every payment strategy returns a `Payment`. Callers use those contracts rather than concrete types. A subclass that silently changed those guarantees would violate LSP.

## Interface Segregation

The project uses narrow contracts such as `IDriverRepository`, `IPaymentProvider`, `IPricingStrategy`, and `INotificationObserver`. Consumers do not depend on methods they do not use. A single all-purpose service interface would force unrelated dependencies on every caller.

## Dependency Inversion

`CabBookingSystem` receives repository, matching, pricing, and notification abstractions. Payment strategies depend on `IPaymentProvider`, not provider-specific APIs. This keeps business rules independent from storage and integrations.
