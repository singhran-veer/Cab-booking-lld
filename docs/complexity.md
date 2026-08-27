# Complexity Analysis

Let `D` be the number of registered drivers and `R` the number of stored rides.

| Operation | Complexity | Reason |
|---|---:|---|
| `registerRider` | O(1) average | Hash insertion |
| `registerDriver` | O(1) average | Hash insertion |
| `findAvailableDrivers` | O(D) | Scans the in-memory registry |
| `findNearestDriver` | O(D) | Scans suitable candidates |
| `requestRide` | O(D) average | Availability scan and matching |
| `acceptRide` | O(1) average | Ride lookup and state transition |
| `cancelRide` | O(1) average | Ride lookup and release |
| `completeRide` | O(1) average | Ride lookup, transition, and release |

The current linear scan is intentional for a small LLD exercise. A production system could use geospatial indexes, partitioned registries, caching, or distributed matching, but those are outside this self-contained implementation.
