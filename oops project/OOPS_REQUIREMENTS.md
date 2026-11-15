# ERL-HUB: Comprehensive OOPS Requirement-to-Code Mapping
### Course: Object-Oriented Programming Using C++ (CIC-211)
**Project Option 2: Post-Earthquake Multi-Agency Emergency Relief Logistics Hub**

---

This document provides an exhaustive, line-by-line audit mapping every single requirement from the official assignment description to its exact implementation construct, source file, class, method, and interactive demonstration workflow.

---

## Complete Requirements Verification Matrix

| # | Syllabus Unit & Requirement | Implementation Construct | Source File & Class/Method | How It Is Demonstrated |
|---|---|---|---|---|
| **1** | **Unit I: Logical Namespaces**<br>`ReliefOps::Logistics` & `ReliefOps::Inventory` | Namespaces partitioning inventory and logistics domains | `include/Common.hpp`<br>`include/Cargo.hpp`<br>`include/Vehicle.hpp`<br>`include/Depot.hpp` | Clean scoping throughout entire codebase; demonstrated across all operations |
| **2** | **Unit I: Function Overloading**<br>Supply allocation routines (volume vs. triage) | Overloaded method `allocateSupplies` with 2 distinct signatures | `include/Depot.hpp` (lines 47-52)<br>`src/Depot.cpp` (lines 37-78)<br>`DistributionDepot::allocateSupplies(...)` | Menu Option 3:<br>- Choice 1 executes Volume/Payload overload<br>- Choice 2 executes Emergency Medical Triage overload |
| **3** | **Unit I: Call by Reference**<br>Passing depots and vehicles without copying | Passing objects via `&` and `const &` to preserve state | `include/Depot.hpp`<br>`src/Depot.cpp`<br>`src/main.cpp` (lines 280, 360, 400) | `DistributionDepot&` and `ReliefVehicle&` passed into allocation, dispatch, and friend methods without cloning |
| **4** | **Unit I: Return by Reference**<br>Direct in-place stock updating | L-value reference return: `ReliefCargo& getCargoItem(...)` | `include/Depot.hpp` (line 40)<br>`src/Depot.cpp` (lines 24-30)<br>`DistributionDepot::getCargoItem(...)` | Menu Option 1 (Choice 3): Fetches cargo reference and updates quantity directly in-place without copying |
| **5** | **Unit I: Inline Utility Methods**<br>Rapid distance/payload/fuel calculation | Methods defined with `inline` specifier | `include/Common.hpp` (lines 85-100)<br>`calculateDistance()`, `computePayloadRatio()`, `estimateFuelRequirement()` | Menu Option 4: Invoked during dispatch and manifest auditing to compute payload ratios and fuel requirements |
| **6** | **Unit I: Friend Class Access**<br>`IncidentCommander` triage overrides | Declared as `friend class IncidentCommander;` | `include/Depot.hpp` (line 28)<br>`include/Vehicle.hpp` (line 90)<br>`src/Depot.cpp` (lines 143-178) | Menu Option 3 (Choice 3): Commander bypasses normal restrictions to access `privateReserveSerumLitres` directly |
| **7** | **Unit II: ADTs & Encapsulation**<br>`ReliefCargo`, `DistributionDepot`, `ReliefVehicle` | Abstract Data Types with private state and public interfaces | `include/Cargo.hpp`<br>`include/Depot.hpp`<br>`include/Vehicle.hpp` | Demonstrated across all menu operations; private data members protected from direct external tampering |
| **8** | **Unit II: Static Members & Analytics**<br>Cumulative tonnage and operational fleet | Static variables and static accessor methods | `include/Vehicle.hpp` (lines 86-87, 114-116)<br>`src/Vehicle.cpp` (lines 10-11)<br>`totalDispatchedTonnage`, `totalOperationalVehicles` | Menu Option 6: System Statistics displays live aggregate counters across all vehicle instances |
| **9** | **Unit II: Dynamic Memory Allocation**<br>Transport manifest on heap | Raw heap array allocation: `ReliefCargo* manifest` | `include/Vehicle.hpp` (line 80)<br>`src/Vehicle.cpp` (lines 19-28)<br>`ReliefVehicle::ReliefVehicle(...)` | Menu Option 2 & 4: Parameterized dynamic constructor allocates heap manifest based on vehicle capacity |
| **10** | **Unit II: Deep Copy Constructor**<br>Independent manifest duplication | Copy constructor with separate heap allocation | `include/Vehicle.hpp` (line 98)<br>`src/Vehicle.cpp` (lines 31-48)<br>`ReliefVehicle::ReliefVehicle(const ReliefVehicle&)` | Menu Option 2 (Choice 3): Clones a vehicle, proving independent heap memory addresses and data isolation |
| **11** | **Unit II: Copy Assignment Operator**<br>Rule of Three with self-assignment guard | Overloaded `operator=` with cleanup and reallocation | `include/Vehicle.hpp` (line 101)<br>`src/Vehicle.cpp` (lines 51-78)<br>`ReliefVehicle::operator=(...)` | Menu Option 2 (Choice 3): Tests `clone = clone` self-assignment guard and deep copy assignment |
| **12** | **Unit II: Explicit Destructor**<br>Clean deallocation of dynamic manifest | Virtual destructor releasing dynamic manifest | `include/Vehicle.hpp` (line 104)<br>`src/Vehicle.cpp` (lines 81-85)<br>`ReliefVehicle::~ReliefVehicle()` | Menu Option 0 & runtime: Executes `delete[] manifest` and decrements `totalOperationalVehicles` |
| **13** | **Unit II: Operator `+=`**<br>Restock supplies at transit depots | Overloaded `operator+=` on `DistributionDepot` | `include/Depot.hpp` (line 58)<br>`src/Depot.cpp` (lines 85-103)<br>`DistributionDepot::operator+=` | Menu Option 1 (Choice 2): Enters new cargo and restocks depot inventory directly via `depot += cargo` |
| **14** | **Unit II: Operator `-=`**<br>Deduct cargo upon vehicle departure | Overloaded `operator-=` on `DistributionDepot` | `include/Depot.hpp` (line 61)<br>`src/Depot.cpp` (lines 106-121)<br>`DistributionDepot::operator-=` | Menu Option 4: Dispatched supplies automatically deducted from depot stock via `depot -= cargo` |
| **15** | **Unit II: Operator `==`**<br>Verify manifest identity & duplicate check | Overloaded `operator==` on `ReliefCargo` | `include/Cargo.hpp` (line 41)<br>`src/Cargo.cpp` (lines 36-38)<br>`ReliefCargo::operator==` | Menu Option 2 (Choice 3): Tests equality between matching and non-matching cargo SKUs |
| **16** | **Unit II: Operator `<<`**<br>Stream insertion for manifest display | Overloaded `friend operator<<` | `include/Cargo.hpp` (line 44)<br>`src/Cargo.cpp` (lines 41-51)<br>`operator<<(ostream&, const ReliefCargo&)` | Menu Option 1 (Choice 1): Prints inventory table rows formatted neatly |
| **17** | **Unit II: Operator `>>`**<br>Stream extraction for keyboard input | Overloaded `friend operator>>` | `include/Cargo.hpp` (line 47)<br>`src/Cargo.cpp` (lines 54-95)<br>`operator>>(istream&, ReliefCargo&)` | Menu Option 1 (Choice 2): Prompts user for cargo SKU, description, weight, volume, and priority |
| **18** | **Unit III: Base and Derived Vehicle Classes**<br>`ReliefVehicle` derived into 3 types | Base `ReliefVehicle` $\rightarrow$ `GroundConvoy`, `AerialTransport`, `AmphibiousCraft` | `include/Vehicle.hpp` (lines 71-184)<br>`src/Vehicle.cpp` (lines 151-239) | Menu Option 2: Roster displays specialized ground, aerial, and marine transports |
| **19** | **Unit III: Virtual Diamond Inheritance**<br>Hybrid amphibious medical unit | `GroundConvoy` and `AmphibiousCraft` inherit virtually; `AmphibiousMedicalUnit` derives from both | `include/Vehicle.hpp` (lines 138, 172, 191)<br>`src/Vehicle.cpp` (lines 247-285) | Menu Option 2 & 4: Hybrid vehicle `HYB-401` constructed with virtual base initialization; traverses both roads and waterways |
| **20** | **Unit III: Competing Overriders Resolution**<br>Explicit final overrides in hybrid class | Explicit `evaluateRoute()` & `executeDispatch()` in `AmphibiousMedicalUnit` | `include/Vehicle.hpp` (lines 204-206)<br>`src/Vehicle.cpp` (lines 255-278) | Eliminates compiler error regarding competing intermediate overriders from Ground and Amphibious classes |
| **21** | **Unit III: Containership (Composition)**<br>`GPSBeacon` & `EmergencyLifeSupportUnit` | Composed as member objects inside `ReliefVehicle` ("has-a") | `include/Vehicle.hpp` (lines 18-66, 77-78) | Menu Option 2 & 4: Telemetry pings and life-support readiness displayed during vehicle inspections |
| **22** | **Unit III: Runtime Polymorphism**<br>Dynamic route evaluation via base pointer | Pure virtual `evaluateRoute()` called on `ReliefVehicle*` | `include/Vehicle.hpp` (line 119)<br>`src/main.cpp` (lines 417-422) | Menu Option 4: Dispatches evaluated through `ReliefVehicle*`, dynamically executing derived terrain checks |
| **23** | **Unit III: Virtual Destructor**<br>Polymorphic clean fleet deallocation | `virtual ~ReliefVehicle()` | `include/Vehicle.hpp` (line 104)<br>`src/Vehicle.cpp` (lines 81-85) | Menu Option 0: Deletes `ReliefVehicle*` pointers in loop; derived destructors execute cleanly |
| **24** | **Unit IV: Custom Exception Framework**<br>Derived from `std::exception` | `CargoOverweightException`<br>`FuelDepletionException`<br>`DamagedCorridorException` | `include/Common.hpp` (lines 35-75) | Menu Option 4: Thrown and caught with descriptive context when payload, fuel, or corridor limits are violated |
| **25** | **Unit IV: Binary File Persistence**<br>Binary serialization using `std::fstream` | `std::fstream` binary mode with fixed-size `TransitRecord` | `src/main.cpp` (lines 48-160)<br>`readAllTransitRecords()`, `writeAndCommitRecordsAtomically()` | Menu Option 4 (write) & Option 5 (read): Records persisted to and retrieved from `data/transit_logs.bin` |
| **26** | **Unit IV: Windows Atomic Replacement**<br>Safe transactional file commit | Win32 `MoveFileExA` / `ReplaceFileA` with `WRITE_THROUGH` | `src/main.cpp` (lines 48-67)<br>`commitBinaryLogAtomically()` | Menu Option 4 & 5: Temporary staging file verified through 4 gates before atomic swap; preserves original data on failure |
| **27** | **Unit IV: Generic Template Class**<br>`PriorityDispatchQueue<T, Compare>` | Custom template priority queue parameterized over type and comparator | `include/PriorityQueue.hpp` (lines 17-64) | Menu Option 4: Stages depot cargo and automatically dequeues the most urgent supply item first |
| **28** | **Unit IV: Custom Functor Object**<br>Sorting urgent medical requirements | Functor `UrgentMedicalComparator` | `include/Cargo.hpp` (lines 56-71) | Menu Option 4: Orders Priority 1 Medical supplies ahead of standard food rations in the dispatch queue |
| **29** | **Unit IV: STL Containers**<br>`std::vector` & `std::unordered_map` | Fleet vector and O(1) inventory hash map | `include/Depot.hpp` (line 22)<br>`include/PriorityQueue.hpp` (line 20)<br>`src/main.cpp` (line 224) | Storing fleet roster (`std::vector<ReliefVehicle*>`), inventory map (`std::unordered_map<string, ReliefCargo>`) |
| **30** | **Unit IV: Stream Manipulators**<br>Tabular manifest formatting | User-defined manipulators: `tableHeader`, `dividerLine`, `alertRed`, `statusGreen` | `include/Common.hpp` (lines 110-139) | Menu Option 1, 4, 5: Formats inventory tables, alert banners, and status confirmations |

---

## Technical Architecture Summary

### Unit II & Unit III Naming Resolution
- In `include/Vehicle.hpp`:
  ```cpp
  namespace ReliefOps::Logistics {
      class ReliefVehicle;
      using TransportVehicle = ReliefVehicle; // Type alias satisfying Unit II & Unit III
  }
  ```
- Satisfies Unit II's mention of `TransportVehicle` and Unit III's mention of `ReliefVehicle` without creating unnecessary wrapper classes.

### Diamond Inheritance Structure
```
            ReliefVehicle (TransportVehicle) [Virtual Base]
                     /                 \
                    / (virtual)         \ (virtual)
            GroundConvoy               AmphibiousCraft
                    \                 /
                     \               /
                   AmphibiousMedicalUnit
```
- Both `GroundConvoy` and `AmphibiousCraft` inherit with `virtual public ReliefVehicle`.
- `AmphibiousMedicalUnit` directly initializes the virtual base `ReliefVehicle(...)`.
- Competing final overriders for `evaluateRoute()` and `executeDispatch()` are cleanly resolved by explicit overrides in `AmphibiousMedicalUnit`.
