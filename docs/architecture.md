# Architecture and Sequences

## Complete Class Diagram

```mermaid
classDiagram
    class main {
        +main()
    }
    class CabBookingApplication {
        +run()
    }
    main --> CabBookingApplication

    class User {
        <<abstract>>
        -uint64_t id
        -string name
        -string phone
    }
    class Rider {
        +addRide(RideId)
        +rideHistory()
    }
    class Driver {
        -DriverStatus status
        -Location location
        -shared_ptr~Vehicle~ vehicle
        +setVehicle(vehicle)
        +setStatus(status)
        +addRating(score)
        +rating()
    }
    User <|-- Rider
    User <|-- Driver

    class Location {
        +double latitude
        +double longitude
        +distanceTo(Location)
    }
    class Money {
        +double amount
        +string currency
    }
    class RideRequest {
        +RiderId riderId
        +Location pickup
        +Location destination
        +VehicleType vehicleType
    }
    class Fare {
        +Money amount
        +double distanceKm
        +double durationMinutes
    }
    class Payment {
        +PaymentStatus status
        +PaymentMethod method
        +Money amount
    }
    class Rating {
        +RideId rideId
        +RiderId riderId
        +DriverId driverId
        +int score
    }

    class Vehicle {
        <<abstract>>
        +VehicleType type()
        +rateMultiplier()
    }
    class Hatchback
    class Sedan
    class SUV
    class PremiumVehicle
    Vehicle <|-- Hatchback
    Vehicle <|-- Sedan
    Vehicle <|-- SUV
    Vehicle <|-- PremiumVehicle
    Driver "1" --> "0..1" Vehicle

    class VehicleFactory {
        +create(type) shared_ptr~Vehicle~
    }
    VehicleFactory ..> Vehicle

    class Ride {
        -shared_ptr~IRideState~ state
        -optional~DriverId~ driverId
        +beginSearch()
        +assign(driverId)
        +accept()
        +arrive()
        +start()
        +complete()
        +cancel()
    }
    Rider "1" --> "many" Ride
    Driver "0..1" --> "many" Ride
    Ride *-- RideRequest
    Ride *-- Fare
    Ride *-- IRideState

    class IRideState {
        <<interface>>
        +status()
        +allows(next)
    }
    IRideState <|.. RequestedState
    IRideState <|.. SearchingDriverState
    IRideState <|.. DriverAssignedState
    IRideState <|.. DriverArrivingState
    IRideState <|.. DriverArrivedState
    IRideState <|.. RideStartedState
    IRideState <|.. RideCompletedState
    IRideState <|.. CancelledState

    class IDriverRepository {
        <<interface>>
        +add(driver)
        +find(id)
        +available()
        +reserve(id, rideId)
        +release(id)
    }
    class InMemoryDriverRepository
    IDriverRepository <|.. InMemoryDriverRepository
    InMemoryDriverRepository o-- Driver

    class IDriverMatchingStrategy {
        <<interface>>
        +match(request, drivers)
    }
    IDriverMatchingStrategy <|.. NearestDriverStrategy
    IDriverMatchingStrategy <|.. HighestRatedDriverStrategy
    IDriverMatchingStrategy <|.. BestETAStrategy

    class IPricingStrategy {
        <<interface>>
        +calculate(request, vehicle)
    }
    IPricingStrategy <|.. NormalPricingStrategy
    IPricingStrategy <|.. SurgePricingStrategy
    IPricingStrategy <|.. PeakHourPricingStrategy
    IPricingStrategy <|.. DiscountPricingStrategy

    class IPaymentProvider {
        <<interface>>
        +charge(amount, method)
    }
    class IPaymentStrategy {
        <<interface>>
        +process(amount, method)
    }
    IPaymentProvider <|.. MockStripeProvider
    IPaymentProvider <|.. MockRazorpayProvider
    IPaymentProvider <|.. StripeAdapter
    IPaymentStrategy <|.. CashPaymentStrategy
    IPaymentStrategy <|.. ProviderPaymentStrategy
    ProviderPaymentStrategy --> IPaymentProvider

    class INotificationObserver {
        <<interface>>
        +onEvent(event)
    }
    class INotificationService {
        <<interface>>
        +subscribe(observer)
        +publish(event)
    }
    INotificationObserver <|.. LoggingObserver
    INotificationService <|.. NotificationService
    NotificationService o-- INotificationObserver

    class CabBookingSystem {
        -shared_ptr~IDriverRepository~ drivers
        -shared_ptr~IDriverMatchingStrategy~ matching
        -shared_ptr~IPricingStrategy~ pricing
        -shared_ptr~INotificationService~ notifications
        +requestRide(...)
        +completeRide(...)
        +pay(...)
        +rateDriver(...)
    }
    CabBookingSystem --> IDriverRepository
    CabBookingSystem --> IDriverMatchingStrategy
    CabBookingSystem --> IPricingStrategy
    CabBookingSystem --> INotificationService
    CabBookingSystem --> Ride
    CabBookingApplication --> CabBookingSystem
```

The composition root is `main.cpp`. It includes `CabBookingApplication.hpp` and the implementation translation unit once, which allows `g++ -std=c++20 -I. -Iinclude main.cpp` to build and run the complete example without a separate linker command.

## Ride Lifecycle State Diagram

```mermaid
stateDiagram-v2
    [*] --> REQUESTED
    REQUESTED --> SEARCHING_DRIVER: beginSearch()
    REQUESTED --> CANCELLED: cancel()
    SEARCHING_DRIVER --> DRIVER_ASSIGNED: assignDriver()
    SEARCHING_DRIVER --> CANCELLED: cancel()
    DRIVER_ASSIGNED --> DRIVER_ARRIVING: accept()
    DRIVER_ASSIGNED --> SEARCHING_DRIVER: reject and rematch
    DRIVER_ASSIGNED --> CANCELLED: cancel()
    DRIVER_ARRIVING --> DRIVER_ARRIVED: arrive()
    DRIVER_ARRIVING --> CANCELLED: cancel()
    DRIVER_ARRIVED --> RIDE_STARTED: start()
    RIDE_STARTED --> RIDE_COMPLETED: complete()
    CANCELLED --> [*]
    RIDE_COMPLETED --> [*]
```

## Why `shared_ptr`?

`shared_ptr` is used where several parts of the application need to refer to the same polymorphic object:

- `CabBookingSystem` shares injected repository, strategy, and notification implementations.
- `InMemoryDriverRepository` stores drivers while the facade and active rides may also refer to them.
- `Driver` refers to a polymorphic `Vehicle` created by `VehicleFactory`.
- `Ride` owns a polymorphic state through `unique_ptr`, because the ride exclusively owns its current state.

The pointer gives RAII lifetime management, supports abstract interfaces, and prevents dangling references when an object is shared across service boundaries. It is not a default for every object. Plain values are used for `Location`, `Money`, `Fare`, and request data; `unique_ptr` is preferable for exclusive ownership. In a larger production system, injected services could use references or `unique_ptr` when their lifetime is guaranteed by the composition root. The current `shared_ptr` choice keeps this small in-memory example easy to compose and test.

```mermaid
sequenceDiagram
    actor Rider
    participant Facade as CabBookingSystem
    participant Pricing as IPricingStrategy
    participant Match as IDriverMatchingStrategy
    participant Registry as IDriverRepository
    participant Ride

    Rider->>Facade: requestRide(pickup, destination, type)
    Facade->>Pricing: calculate(request, vehicle)
    Facade->>Registry: available()
    Facade->>Match: match(request, candidates)
    Match-->>Facade: driver
    Facade->>Registry: reserve(driver, ride)
    Facade->>Ride: beginSearch(), assign(driver)
    Facade-->>Rider: RideId and Fare
```

```mermaid
sequenceDiagram
    actor Driver
    participant Facade as CabBookingSystem
    participant Ride
    participant Payment as IPaymentStrategy
    participant Notify as NotificationService

    Driver->>Facade: completeRide(driver, ride)
    Facade->>Ride: complete()
    Facade->>Facade: release driver
    Driver->>Facade: pay(ride, method)
    Facade->>Payment: process(amount, method)
    Payment-->>Facade: Payment
    Facade->>Notify: publish(payment completed)
```

```mermaid
sequenceDiagram
    actor A as Rider A
    actor B as Rider B
    participant Registry as Driver Registry
    participant D1 as Driver D1

    A->>Registry: reserve(D1, rideA)
    B->>Registry: reserve(D1, rideB)
    Note over Registry: mutex protects check and status update
    Registry-->>A: success
    Registry-->>B: false; D1 already reserved
```

## Concurrency

The reservation critical section contains the availability check and transition to `Reserved`. `std::mutex` and `std::lock_guard` protect the driver map and status. This is sufficient for the in-memory implementation and avoids pretending that an atomic flag can protect a multi-field reservation operation.
