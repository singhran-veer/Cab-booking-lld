# Design Patterns

| Pattern | Module | Classes | Problem | Benefit |
|---|---|---|---|---|
| State | Ride | `Ride`, `IRideState`, concrete states | Lifecycle behavior changes by state | Local, explicit transitions |
| Strategy | Matching | `IDriverMatchingStrategy` and implementations | Multiple driver-selection algorithms | Algorithms are interchangeable |
| Strategy | Pricing | `IPricingStrategy` and implementations | Pricing rules vary | Rules are independently testable |
| Strategy | Payment | `IPaymentStrategy` and implementations | Payment methods differ | New methods do not modify rides |
| Factory | Vehicle | `VehicleFactory`, vehicle subclasses | Client should not construct concrete vehicles | Creation is centralized |
| Adapter | Payment | `IPaymentProvider`, mock providers, adapters | Provider APIs are incompatible | Domain remains provider-independent |
| Observer | Notification | `NotificationService`, observers | Many consumers need ride events | Loose event coupling |
| Repository | Persistence | `IDriverRepository`, in-memory implementation | Business code should not own storage | Storage can be replaced |
| Facade | Application | `CabBookingSystem` | Client should not coordinate services | Small public API |

## State

`Ride` owns an `IRideState`. Each concrete state declares its valid next states, and `Ride::transition` replaces the state object. This avoids placing lifecycle rules in a large status switch. It supports SRP, OCP, and LSP. A plain enum plus switch was considered, but would make every new lifecycle state modify central logic.

## Strategy

Matching, pricing, and payment each expose a focused interface. `CabBookingSystem` receives matching, pricing, and notification implementations through its constructor. This supports OCP and DIP. A mode enum with conditional logic was considered but would couple the application service to every algorithm.

## Factory

`VehicleFactory::create` selects `Hatchback`, `Sedan`, `SUV`, or `PremiumVehicle`. The client only supplies `VehicleType`. This supports SRP and OCP. Direct construction was rejected because it leaks concrete classes into the client.

## Adapter

Mock Stripe and Razorpay providers stand in for external APIs. `StripeAdapter` exposes the application's `IPaymentProvider` contract. The providers are intentionally local mocks: this project has no network or real payment integration. This supports DIP and OCP.

## Observer

`NotificationService` publishes `RideEvent` values to registered `INotificationObserver` objects. Rider, driver, logging, email, or SMS observers can be added independently. Direct service calls were rejected because they would couple ride orchestration to every notification channel.

## Repository

The application sees `IDriverRepository`, while `InMemoryDriverRepository` owns the map and its mutex. A future SQL or distributed implementation can satisfy the same contract. This supports DIP and SRP.

## Facade

`CabBookingSystem` is the only API used by `main.cpp`. It coordinates domain objects, repositories, strategies, and notifications while hiding those details from the client.
