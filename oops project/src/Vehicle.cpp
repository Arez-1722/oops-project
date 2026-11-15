#include "../include/Vehicle.hpp"
#include <iostream>
#include <iomanip>

namespace ReliefOps {
    namespace Logistics {

        // ====================================================================
        // UNIT II: STATIC VARIABLE DEFINITIONS
        // ====================================================================
        double ReliefVehicle::totalDispatchedTonnage = 0.0;
        int ReliefVehicle::totalOperationalVehicles = 0;

        // ====================================================================
        // UNIT II: BASE CLASS ReliefVehicle (RULE OF THREE & DYNAMIC MEMORY)
        // ====================================================================

        // Dynamic Parameterized Constructor
        ReliefVehicle::ReliefVehicle(std::string id, std::string callsign, double maxPayload, 
                                     double fuelRange, int capacity)
            : vehicleId(id), callsign(callsign), maxPayloadTons(maxPayload), fuelRangeKm(fuelRange),
              beacon(27.7172, 85.3240, 100), lifeSupport(100, true, 2),
              manifest(nullptr), manifestCount(0), manifestCapacity(capacity > 0 ? capacity : 5) {
            
            // Dynamic allocation on heap for transport manifest
            manifest = new ReliefOps::Inventory::ReliefCargo[manifestCapacity];
            totalOperationalVehicles++; // Track active vehicle instance
        }

        // Deep Copy Constructor (Rule of Three)
        ReliefVehicle::ReliefVehicle(const ReliefVehicle& other)
            : vehicleId(other.vehicleId + "_COPY"),
              callsign(other.callsign + " (Cloned)"),
              maxPayloadTons(other.maxPayloadTons),
              fuelRangeKm(other.fuelRangeKm),
              beacon(other.beacon),
              lifeSupport(other.lifeSupport),
              manifest(nullptr),
              manifestCount(other.manifestCount),
              manifestCapacity(other.manifestCapacity) {
            
            // Allocate fresh, independent heap memory
            manifest = new ReliefOps::Inventory::ReliefCargo[manifestCapacity];
            for (int i = 0; i < manifestCount; ++i) {
                manifest[i] = other.manifest[i]; // Deep element-wise copy
            }
            totalOperationalVehicles++; // Cloned object is an active operational vehicle
        }

        // Copy Assignment Operator (Rule of Three)
        ReliefVehicle& ReliefVehicle::operator=(const ReliefVehicle& other) {
            if (this == &other) {
                return *this; // Protect against self-assignment
            }

            // Clean up existing dynamic memory
            delete[] manifest;

            // Copy attributes
            vehicleId = other.vehicleId;
            callsign = other.callsign;
            maxPayloadTons = other.maxPayloadTons;
            fuelRangeKm = other.fuelRangeKm;
            beacon = other.beacon;
            lifeSupport = other.lifeSupport;
            manifestCount = other.manifestCount;
            manifestCapacity = other.manifestCapacity;

            // Allocate fresh memory and deep copy
            manifest = new ReliefOps::Inventory::ReliefCargo[manifestCapacity];
            for (int i = 0; i < manifestCount; ++i) {
                manifest[i] = other.manifest[i];
            }

            // Note: totalOperationalVehicles is unchanged because no new vehicle object was instantiated
            return *this;
        }

        // Explicit Virtual Destructor (Rule of Three & Polymorphic Deallocation)
        ReliefVehicle::~ReliefVehicle() {
            delete[] manifest; // Release dynamically allocated heap manifest
            manifest = nullptr;
            totalOperationalVehicles--; // Decrement active vehicle count
        }

        // Dynamic Manifest bounds checking & inspection
        double ReliefVehicle::getCurrentPayloadWeight() const {
            double total = 0.0;
            for (int i = 0; i < manifestCount; ++i) {
                total += manifest[i].getWeightTons();
            }
            return total;
        }

        bool ReliefVehicle::canAcceptPayload(const ReliefOps::Inventory::ReliefCargo& cargo) const {
            if (manifestCount >= manifestCapacity) {
                return false; // Bounds guard
            }
            return (getCurrentPayloadWeight() + cargo.getWeightTons() <= maxPayloadTons);
        }

        void ReliefVehicle::addCargoToManifest(const ReliefOps::Inventory::ReliefCargo& cargo) {
            // Guard 1: Slot bounds check
            if (manifestCount >= manifestCapacity) {
                throw CargoOverweightException(
                    "Manifest capacity limit reached: Cannot allocate additional cargo slot (Limit: " +
                    std::to_string(manifestCapacity) + " slots)."
                );
            }
            // Guard 2: Weight payload check
            if (getCurrentPayloadWeight() + cargo.getWeightTons() > maxPayloadTons) {
                throw CargoOverweightException(
                    "Payload capacity exceeded: Attempted total weight " +
                    std::to_string(getCurrentPayloadWeight() + cargo.getWeightTons()) +
                    " T exceeds vehicle limit of " + std::to_string(maxPayloadTons) + " T."
                );
            }
            // Bounds-safe insertion
            manifest[manifestCount++] = cargo;
        }

        void ReliefVehicle::displayManifest() const {
            std::cout << "\n    --- MANIFEST AUDIT FOR VEHICLE: " << vehicleId << " (" << callsign << ") ---\n";
            std::cout << "    Payload: " << getCurrentPayloadWeight() << " / " << maxPayloadTons 
                      << " Tons (" << (computePayloadRatio(getCurrentPayloadWeight(), maxPayloadTons) * 100.0) << "%)\n";
            std::cout << "    Manifest Slots Used: " << manifestCount << " / " << manifestCapacity << "\n";
            if (manifestCount == 0) {
                std::cout << "    (Manifest is currently empty)\n";
                return;
            }
            std::cout << "    " << tableHeader << "\n";
            for (int i = 0; i < manifestCount; ++i) {
                std::cout << "    " << manifest[i] << "\n";
            }
            std::cout << "    " << dividerLine << "\n";
        }

        void ReliefVehicle::displayDetails() const {
            std::cout << "  Vehicle ID      : " << vehicleId << "\n"
                      << "  Callsign        : " << callsign << "\n"
                      << "  Max Payload     : " << maxPayloadTons << " Metric Tons\n"
                      << "  Fuel Range      : " << fuelRangeKm << " km\n"
                      << "  Manifest Items  : " << manifestCount << " / " << manifestCapacity << "\n";
            beacon.pingTelemetry();
            lifeSupport.displayLifeSupportStatus();
        }

        // ====================================================================
        // UNIT III: DERIVED CLASS 1 - GroundConvoy IMPLEMENTATION
        // ====================================================================
        GroundConvoy::GroundConvoy(std::string id, std::string callsign, double maxPayload, 
                                   double fuelRange, int axles, int capacity)
            : ReliefVehicle(id, callsign, maxPayload, fuelRange, capacity), allTerrainAxles(axles) {}

        bool GroundConvoy::evaluateRoute(const std::string& corridorType, double distanceKm) const {
            if (corridorType == "River" || corridorType == "FloodedBasin" || corridorType == "DeepWater") {
                throw DamagedCorridorException(
                    "Ground convoy " + vehicleId + " cannot traverse water barrier (" + corridorType + ")."
                );
            }
            if (distanceKm > fuelRangeKm) {
                throw FuelDepletionException(
                    "Ground transit distance of " + std::to_string(distanceKm) + " km exceeds fuel range of " +
                    std::to_string(fuelRangeKm) + " km for convoy " + vehicleId + "."
                );
            }
            return true;
        }

        void GroundConvoy::executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) {
            addCargoToManifest(cargo);
            totalDispatchedTonnage += cargo.getWeightTons();
            std::cout << "  [GROUND DISPATCH] " << vehicleId << " loaded with " 
                      << cargo.getDescription() << " (" << cargo.getWeightTons() << " T).\n";
        }

        void GroundConvoy::displayDetails() const {
            std::cout << "--- [HEAVY GROUND CONVOY] ---\n";
            ReliefVehicle::displayDetails();
            std::cout << "  All-Terrain Axles: " << allTerrainAxles << " (Heavy Differential Lock)\n";
        }

        // ====================================================================
        // UNIT III: DERIVED CLASS 2 - AerialTransport IMPLEMENTATION
        // ====================================================================
        AerialTransport::AerialTransport(std::string id, std::string callsign, double maxPayload, 
                                         double fuelRange, double ceiling, int capacity)
            : ReliefVehicle(id, callsign, maxPayload, fuelRange, capacity), maxFlightCeilingMeters(ceiling) {}

        bool AerialTransport::evaluateRoute(const std::string& corridorType, double distanceKm) const {
            if (corridorType == "SevereStormTurbulence" || corridorType == "DenseVolcanicAsh") {
                throw DamagedCorridorException(
                    "Aerial transport " + vehicleId + " grounded due to hazardous flight corridor (" + corridorType + ")."
                );
            }
            if (distanceKm > fuelRangeKm) {
                throw FuelDepletionException(
                    "Flight corridor distance of " + std::to_string(distanceKm) + " km exceeds fuel range of " +
                    std::to_string(fuelRangeKm) + " km for aircraft " + vehicleId + "."
                );
            }
            return true;
        }

        void AerialTransport::executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) {
            addCargoToManifest(cargo);
            totalDispatchedTonnage += cargo.getWeightTons();
            std::cout << "  [AIR DISPATCH] " << vehicleId << " loaded with " 
                      << cargo.getDescription() << " (" << cargo.getWeightTons() << " T).\n";
        }

        void AerialTransport::displayDetails() const {
            std::cout << "--- [AERIAL TRANSPORT - ROTORCRAFT / DRONE] ---\n";
            ReliefVehicle::displayDetails();
            std::cout << "  Flight Ceiling  : " << maxFlightCeilingMeters << " meters ASL\n";
        }

        // ====================================================================
        // UNIT III: DERIVED CLASS 3 - AmphibiousCraft IMPLEMENTATION
        // ====================================================================
        AmphibiousCraft::AmphibiousCraft(std::string id, std::string callsign, double maxPayload, 
                                         double fuelRange, double draft, int capacity)
            : ReliefVehicle(id, callsign, maxPayload, fuelRange, capacity), draftDepthMeters(draft) {}

        bool AmphibiousCraft::evaluateRoute(const std::string& corridorType, double distanceKm) const {
            if (corridorType == "SteepMountainRoad" || corridorType == "DryCanyonRidge") {
                throw DamagedCorridorException(
                    "Amphibious craft " + vehicleId + " cannot navigate dry steep mountain terrain."
                );
            }
            if (distanceKm > fuelRangeKm) {
                throw FuelDepletionException(
                    "Waterway transit distance of " + std::to_string(distanceKm) + " km exceeds fuel range of " +
                    std::to_string(fuelRangeKm) + " km for vessel " + vehicleId + "."
                );
            }
            return true;
        }

        void AmphibiousCraft::executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) {
            addCargoToManifest(cargo);
            totalDispatchedTonnage += cargo.getWeightTons();
            std::cout << "  [AMPHIBIOUS DISPATCH] " << vehicleId << " loaded with " 
                      << cargo.getDescription() << " (" << cargo.getWeightTons() << " T).\n";
        }

        void AmphibiousCraft::displayDetails() const {
            std::cout << "--- [AMPHIBIOUS RESCUE CRAFT] ---\n";
            ReliefVehicle::displayDetails();
            std::cout << "  Draft Depth     : " << draftDepthMeters << " meters\n";
        }

        // ====================================================================
        // UNIT III: DIAMOND INHERITANCE HYBRID - AmphibiousMedicalUnit IMPLEMENTATION
        // ====================================================================
        // Constructor initialization order:
        // 1. Virtual base ReliefVehicle is initialized by the most-derived class!
        // 2. Intermediate GroundConvoy is initialized.
        // 3. Intermediate AmphibiousCraft is initialized.
        AmphibiousMedicalUnit::AmphibiousMedicalUnit(
            std::string id, std::string callsign, double maxPayload, double fuelRange, 
            int axles, double draft, bool surgery, int capacity)
            : ReliefVehicle(id, callsign, maxPayload, fuelRange, capacity), // Virtual base init
              GroundConvoy(id, callsign, maxPayload, fuelRange, axles, capacity),
              AmphibiousCraft(id, callsign, maxPayload, fuelRange, draft, capacity),
              mobileSurgeryDeployed(surgery) {}

        // Explicit final overriders resolving competing intermediate base implementations:
        bool AmphibiousMedicalUnit::evaluateRoute(const std::string& corridorType, double distanceKm) const {
            if (distanceKm > fuelRangeKm) {
                throw FuelDepletionException(
                    "Hybrid mission transit distance of " + std::to_string(distanceKm) + 
                    " km exceeds hybrid fuel range of " + std::to_string(fuelRangeKm) + " km."
                );
            }
            // Hybrid advantage: can traverse both damaged roads AND flooded river basins!
            if (corridorType == "DamagedRoad" || corridorType == "Mudslide") {
                return GroundConvoy::evaluateRoute(corridorType, distanceKm);
            } else if (corridorType == "River" || corridorType == "FloodedBasin" || corridorType == "DeepWater") {
                return AmphibiousCraft::evaluateRoute(corridorType, distanceKm);
            }
            return true;
        }

        void AmphibiousMedicalUnit::executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) {
            addCargoToManifest(cargo); // Bounds checked
            totalDispatchedTonnage += cargo.getWeightTons();
            std::cout << "  [HYBRID MEDICAL DISPATCH] " << vehicleId << " (Mobile Surgical Team) deployed with "
                      << cargo.getDescription() << " (" << cargo.getWeightTons() << " T).\n";
        }

        void AmphibiousMedicalUnit::displayDetails() const {
            std::cout << "--- [DIAMOND HYBRID: AMPHIBIOUS MOBILE MEDICAL UNIT] ---\n";
            ReliefVehicle::displayDetails();
            std::cout << "  Drive System    : " << allTerrainAxles << "-Wheel Dual All-Terrain & Marine Waterjet\n"
                      << "  Water Draft     : " << draftDepthMeters << " meters\n"
                      << "  Mobile Surgery  : " << (mobileSurgeryDeployed ? "OPERATIONAL & STERILE" : "STANDBY") << "\n";
        }

    } // namespace Logistics
} // namespace ReliefOps
