# Post-Earthquake Multi-Agency Emergency Relief Logistics Hub (ERL-HUB)
### Academic Project for Object-Oriented Programming Using C++ (CIC-211)
**Student Assessment Project | B.Tech Computer Science & Engineering**

---

## 1. Project Overview & Domain Context

Following a major seismic disaster (such as the Gorkha or Tohoku earthquakes), multi-agency disaster response coordinators—including the National Disaster Response Force (NDRF), paramilitary relief convoys, non-governmental humanitarian agencies, and specialized emergency medical teams—establish regional field staging hubs. 

Relief coordinators must manage heterogeneous transport agents (heavy all-terrain trucks, emergency medical helicopters, search-and-rescue quadcopters, and amphibious rescue boats), allocate strictly limited critical supplies (high-calorie ration kits, trauma triage medicine, clean water tanks, and thermal mylar blankets), and coordinate safe dispatch routing amid damaged, mudslide-severed infrastructure.

The **Post-Earthquake Multi-Agency Emergency Relief Logistics Hub (ERL-HUB)** is a standard C++17 console-based logistics coordination system designed to model, manage, and audit disaster relief operations. It strictly fulfills all technical requirements specified across **Units I through IV** of the B.Tech CSE Object-Oriented Programming using C++ (CIC-211) curriculum.

---

## 2. Key Features & Menu Operations

The application features a streamlined **7-Operation Interactive Console Menu**:

1. **Manage Inventory**:
   - Tabular stockpile visualization using user-defined stream manipulators (`tableHeader`, `dividerLine`).
   - Restocking supplies using overloaded `DistributionDepot::operator+=` and keyboard stream extraction `ReliefCargo::operator>>`.
   - In-place stock quantity adjustments demonstrating **Return by Reference** (`getCargoItem()`).
2. **Manage Fleet & Vehicles**:
   - Polymorphic roster display across heterogeneous vehicles (`GroundConvoy`, `AerialTransport`, `AmphibiousCraft`, `AmphibiousMedicalUnit`).
   - Registration of new transport vehicles with composed `GPSBeacon` and `EmergencyLifeSupportUnit`.
   - Dedicated diagnostic testing demonstrating the **Rule of Three** (dynamic constructor, deep copy constructor, copy assignment operator with self-assignment guard, explicit virtual destructor) and `ReliefCargo::operator==`.
3. **Allocate Supplies**:
   - **Function Overloading**:
     - *Overload A*: Bulk payload volume/weight-based allocation for general sustenance (`allocateSupplies(double, double, string)`).
     - *Overload B*: Priority emergency medical triage allocation for critical pharmaceuticals (`allocateSupplies(int, string, int)`).
   - **Friend Class Override**: Incident Commander (`IncidentCommander`) privileged access unlocking restricted emergency blood plasma/serum vault reserves.
4. **Dispatch Relief Supplies**:
   - Automated mission staging using generic template class `PriorityDispatchQueue<T>` parameterized with custom functor `UrgentMedicalComparator` to ensure life-saving triage supplies dispatch before general rations.
   - Dynamic route evaluation across damaged infrastructure corridors (`DamagedRoad`, `Mudslide`, `FloodedRiver`, `AirTurbulence`) via runtime polymorphism (`ReliefVehicle*`).
   - Custom exception safety catching `CargoOverweightException`, `FuelDepletionException`, and `DamagedCorridorException`.
   - Stock deduction via `DistributionDepot::operator-=`.
   - Binary transit logging with atomic Windows file replacement.
5. **View Dispatch History & Audit Records**:
   - Deserialization and tabular display of binary transit records from `data/transit_logs.bin` using `std::fstream` in binary mode.
   - Mission record search by Vehicle ID.
   - Transactional in-place record updating using read-modify-write-swap atomic staging.
6. **View System Statistics & Live Analytics**:
   - Live analytics tracked via static member variables and functions: `totalDispatchedTonnage` and `totalOperationalVehicles`.
7. **Exit System**:
   - Clean, leak-free termination with polymorphic virtual destructor invocation (`virtual ~ReliefVehicle()`) across all allocated fleet pointers.

---

## 3. Syllabus Unit Coverage

### Unit I: Core Foundations & Call Semantics
- **Logical Namespaces**: Code cleanly partitioned into `ReliefOps::Logistics` and `ReliefOps::Inventory`.
- **Function Overloading**: Overloaded `DistributionDepot::allocateSupplies(...)` for volume/weight bulk payload vs. emergency medical triage priority.
- **Call by Reference**: Depots, vehicles, and cargo items passed via `&` and `const &` across allocation, dispatch, and logging routines.
- **Return by Reference**: `DistributionDepot::getCargoItem(const std::string& id)` returns an l-value reference allowing direct in-place stock updates.
- **Inline Utility Methods**: Rapid calculations defined with the `inline` specifier: `calculateDistance()`, `computePayloadRatio()`, `estimateFuelRequirement()`.
- **Friend Class**: `IncidentCommander` declared as `friend class` inside `DistributionDepot` and `ReliefVehicle` to bypass access barriers to private emergency serum vaults during Level-1 triage emergencies.

### Unit II: Encapsulation, Dynamic Memory & Operator Overloading
- **Abstract Data Types & Information Hiding**: `ReliefCargo`, `DistributionDepot`, and `ReliefVehicle` encapsulate private state behind validated public interfaces.
- **Real-Time Analytics via Static Members**: `ReliefVehicle::totalDispatchedTonnage` and `ReliefVehicle::totalOperationalVehicles` maintain cumulative metrics across all instances.
- **Dynamic Memory Management & Rule of Three**: `ReliefVehicle` manages a raw heap-allocated array (`ReliefCargo* manifest`):
  - Dynamic Parameterized Constructor allocating manifest on the heap.
  - Deep Copy Constructor allocating fresh heap memory and copying elements, properly tracking active vehicle counts.
  - Copy Assignment Operator (`operator=`) protecting against self-assignment (`if (this == &other) return *this;`) and reallocating memory safely.
  - Explicit Virtual Destructor (`virtual ~ReliefVehicle()`) deallocating heap memory (`delete[] manifest`).
- **Dynamic Manifest Safety**: Strict bounds checking (`manifestCount < manifestCapacity`) before every insertion, throwing `CargoOverweightException` to prevent buffer overruns.
- **Operator Overloading**:
  - `DistributionDepot::operator+=`: Restocks inventory.
  - `DistributionDepot::operator-=`: Deducts dispatched cargo.
  - `ReliefCargo::operator==`: Manifest equality and duplicate detection.
  - `ReliefCargo::operator<<`: Stream insertion for tabular printing.
  - `ReliefCargo::operator>>`: Stream extraction for user keyboard entry.

### Unit III: Inheritance, Containership & Polymorphism
- **Inheritance Hierarchy**: Abstract base `ReliefVehicle` derived into `GroundConvoy`, `AerialTransport`, and `AmphibiousCraft`.
- **Virtual Base Classes & Diamond Inheritance**: Hybrid `AmphibiousMedicalUnit` derives from both `GroundConvoy` and `AmphibiousCraft`, both of which inherit virtually from `ReliefVehicle`:
  ```
               ReliefVehicle (TransportVehicle) [Virtual Base]
                         /                 \
                        / (virtual)         \ (virtual)
                GroundConvoy               AmphibiousCraft
                        \                 /
                         \               /
                       AmphibiousMedicalUnit
  ```
  - Resolves base subobject duplication and eliminates member ambiguity.
  - The most-derived class `AmphibiousMedicalUnit` directly initializes the virtual base `ReliefVehicle(...)`.
  - Competing final overriders for `evaluateRoute()` and `executeDispatch()` are explicitly overridden in `AmphibiousMedicalUnit`.
- **Containership (Composition)**: Independent telemetry `GPSBeacon` and medical subsystem `EmergencyLifeSupportUnit` composed directly inside `ReliefVehicle` ("has-a" relationship).
- **Runtime Polymorphism**: Heterogeneous fleet managed via base pointers (`std::vector<ReliefVehicle*>`), invoking pure virtual `evaluateRoute()` and `executeDispatch()`.
- **Virtual Destructors**: Guaranteed polymorphic cleanup (`virtual ~ReliefVehicle()`).

### Unit IV: Streams, Persistence, Exception Safety & STL
- **Custom Exception Framework**: Hierarchy derived from `std::exception`:
  - `CargoOverweightException`: Payload exceeds vehicle capacity or manifest is full.
  - `FuelDepletionException`: Corridor distance exceeds vehicle fuel range.
  - `DamagedCorridorException`: Route infrastructure damage blocks the vehicle type.
- **Binary Persistence & Windows Atomic Replacement**:
  - Binary serialization with `std::fstream` (`std::ios::binary | std::ios::in | std::ios::out`).
  - Fixed-size POD struct `TransitRecord`.
  - Transactional staging to temporary file `data/transit_logs.tmp`.
  - Four verification gates: stream health flags, flush/close error checks, byte size multiple of `sizeof(TransitRecord)`, and expected record count match.
  - Atomic swap using Win32 API `MoveFileExA` / `ReplaceFileA` with `MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH`. Preserves the original file if replacement fails.
- **Generic Template Class**: `template <typename T, typename Compare> class PriorityDispatchQueue` implementing priority queue operations (`push`, `pop`, `top`, `isEmpty`, `size`).
- **Custom Functor**: `struct UrgentMedicalComparator` prioritizing emergency medical triage items.
- **STL Containers**: `std::vector` (polymorphic fleet, priority queue heap) and `std::unordered_map` (depot inventory O(1) SKU lookup).
- **Stream Manipulators**: `tableHeader`, `dividerLine`, `alertRed`, `statusGreen`.

---

## 4. Class Architecture & Design Decisions

### TransportVehicle Naming Resolution
The assignment syllabus mentions:
- **Unit II**: *"Construct classes ReliefCargo, DistributionDepot, and TransportVehicle adhering strictly to abstract data type standards..."*
- **Unit III**: *"Base class ReliefVehicle derived into AerialTransport, GroundConvoy, and AmphibiousCraft..."*

**Design Resolution**:
In `include/Vehicle.hpp`, inside namespace `ReliefOps::Logistics`:
```cpp
namespace ReliefOps::Logistics {
    class ReliefVehicle;
    using TransportVehicle = ReliefVehicle; // Standard type alias
}
```
**Rationale**: `TransportVehicle` and `ReliefVehicle` represent the identical conceptual domain entity. Introducing an artificial intermediate wrapper class would introduce dummy constructor boilerplate and redundant virtual dispatch overhead. A transparent type alias satisfies both Unit II and Unit III naming requirements with zero runtime overhead and 100% standard C++ idiomatic clarity.

---

## 5. File Structure

```
C:\Users\ADMIN\Downloads\oops project\
├── include\
│   ├── Common.hpp          // Namespaces, exceptions, inline utilities, stream manipulators
│   ├── Cargo.hpp           // ReliefCargo ADT, operator overloads, UrgentMedicalComparator functor
│   ├── Vehicle.hpp         // GPSBeacon, EmergencyLifeSupportUnit, ReliefVehicle, GroundConvoy,
│   │                       // AerialTransport, AmphibiousCraft, AmphibiousMedicalUnit, alias TransportVehicle
│   ├── Depot.hpp           // DistributionDepot, overloaded allocations, friend IncidentCommander
│   └── PriorityQueue.hpp   // Generic template class PriorityDispatchQueue<T>
├── src\
│   ├── Cargo.cpp           // ReliefCargo implementations and operator definitions
│   ├── Vehicle.cpp         // Vehicle dynamic manifest Rule of 3, route evaluations, static counters
│   ├── Depot.cpp           // Depot inventory management, overloaded allocation, commander override
│   └── main.cpp            // 7-operation console UI, binary fstream persistence, Windows atomic replacement
├── tests\
│   └── test_suite.cpp      // Comprehensive automated verification test harness for all Units I-IV
├── data\                   // Persistent storage directory for transit_logs.bin
├── build.bat               // Windows compile script using MinGW g++ (C++17)
├── CMakeLists.txt          // Standard CMake build definition
├── README.md               // Project documentation, architecture, and viva guide
└── OOPS_REQUIREMENTS.md    // Complete line-by-line requirement to code mapping document
```

---

## 6. Compilation & Execution Instructions

### Prerequisites
- A modern C++17 compatible compiler (e.g., `g++` via MinGW/MSYS2 or MSVC).
- Windows 10/11 operating system (for Win32 atomic file operations).

### Option A: Using the Build Script (Recommended)
Open Command Prompt or PowerShell in the project directory and run:
```bat
.\build.bat
```
Then run the compiled executable:
```bat
.\erl_hub.exe
```

### Option B: Manual Command-Line Compilation
```bat
g++ -std=c++17 -Wall -Wextra -Iinclude src/Cargo.cpp src/Vehicle.cpp src/Depot.cpp src/main.cpp -o erl_hub.exe
.\erl_hub.exe
```

### Option C: Compiling and Running the Automated Test Suite
To execute the automated test harness verifying all Unit I–IV assertions:
```bat
g++ -std=c++17 -Wall -Wextra -Iinclude src/Cargo.cpp src/Vehicle.cpp src/Depot.cpp tests/test_suite.cpp -o test_suite.exe
.\test_suite.exe
```

---

## 7. Sample Interactive Execution Walkthrough

```text
================================================================================
    POST-EARTHQUAKE MULTI-AGENCY EMERGENCY RELIEF LOGISTICS HUB (ERL-HUB)     
           Academic OOPS Demonstration Console | Course: CIC-211                
================================================================================

================================================================================
                     ERL-HUB DISASTER LOGISTICS CONSOLE                         
================================================================================
  1. Manage Inventory (Restock +=, Input >>, Tabular Display, Return by Ref)
  2. Manage Fleet & Vehicles (Polymorphism, Diamond Inheritance, Deep Copy Test)
  3. Allocate Supplies (Function Overloading & Friend Class Override)
  4. Dispatch Relief Supplies (Priority Queue, Route Evaluation, Exceptions)
  5. View Dispatch History & Update Audit Logs (Binary std::fstream Persistence)
  6. View System Statistics & Analytics (Static Members & Logistics Metrics)
  0. Exit System (Clean Polymorphic Deallocation via Virtual Destructors)
================================================================================
```

### Demonstrating Core Requirements in 5 Minutes during a Viva:
1. **Show Inventory & Return by Reference**: Select `1` $\rightarrow$ `1` (view table via custom manipulators), then `1` $\rightarrow$ `3` (modify cargo quantity directly in-place through l-value reference).
2. **Show Diamond Inheritance & Deep Copy**: Select `2` $\rightarrow$ `1` (view polymorphic fleet), then `2` $\rightarrow$ `3` (run Rule of Three cloning test proving independent heap manifest arrays and `operator==`).
3. **Show Overloading & Friend Class**: Select `3` $\rightarrow$ `1` (volume overload), `3` $\rightarrow$ `2` (triage overload), `3` $\rightarrow$ `3` (Incident Commander unlocks private serum vault).
4. **Show Priority Queue, Exceptions & Atomic Binary Persistence**: Select `4` $\rightarrow$ Select vehicle, corridor, distance. Observe `PriorityDispatchQueue` popping Priority 1 Medical supplies first, polymorphic route evaluation, exception handling, and atomic binary log update to `data/transit_logs.bin`.
5. **Show Binary Deserialization**: Select `5` $\rightarrow$ `1` (reads and prints binary records).
6. **Show Live Analytics & Polymorphic Cleanup**: Select `6` (view static counters), then `0` (clean virtual destructor cleanup).

---

## 8. Academic Viva Preparation Guide (Frequently Asked Questions)

**Q1: How does virtual inheritance solve the diamond problem in this project?**
> *Answer*: When `AmphibiousMedicalUnit` inherits from both `GroundConvoy` and `AmphibiousCraft`, both intermediate classes inherit virtually (`virtual public ReliefVehicle`). This ensures only one shared instance of `ReliefVehicle` exists in the derived object, eliminating ambiguity and avoiding duplicate dynamic manifests or GPS beacons. Furthermore, `AmphibiousMedicalUnit` explicitly overrides `evaluateRoute()` and `executeDispatch()` to eliminate competing final overrider ambiguities.

**Q2: What is the Rule of Three and why is it implemented in `ReliefVehicle`?**
> *Answer*: Because `ReliefVehicle` manages raw dynamically allocated heap memory (`ReliefCargo* manifest`), relying on default shallow copies would cause dangling pointers and double-free crashes. We implement:
> 1. Parameterized Dynamic Constructor allocating heap memory.
> 2. Deep Copy Constructor allocating a new array and copying elements.
> 3. Copy Assignment Operator (`operator=`) with a self-assignment guard (`if (this == &other) return *this;`) and safe cleanup.
> 4. Explicit Virtual Destructor (`virtual ~ReliefVehicle()`) deallocating heap memory (`delete[] manifest`).

**Q3: How is file replacement made atomic on Windows?**
> *Answer*: On Windows NTFS, standard `std::filesystem::copy_file` or `std::rename` cannot guarantee atomic updates when the destination file exists. We write records to a temporary file (`transit_logs.tmp`), flush, close, and verify file size and record counts. Only after passing all verification gates do we execute `ReplaceFileA` / `MoveFileExA` with `MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH`, which performs a transactional swap. If replacement fails, the original binary log remains untouched.
