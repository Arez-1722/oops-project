#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include "Common.hpp"
#include "Cargo.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

namespace ReliefOps {
    namespace Logistics {

        // ====================================================================
        // UNIT III: CONTAINERSHIP (COMPOSITION) - INDEPENDENT SUBSYSTEMS
        // ====================================================================

        // Independent GPS telemetry beacon composed inside vehicle agents
        class GPSBeacon {
        private:
            double latitude;
            double longitude;
            int batteryPct;

        public:
            GPSBeacon(double lat = 27.7172, double lon = 85.3240, int battery = 100)
                : latitude(lat), longitude(lon), batteryPct(battery) {}

            double getLatitude() const { return latitude; }
            double getLongitude() const { return longitude; }
            int getBatteryPct() const { return batteryPct; }

            void updateCoordinates(double lat, double lon) {
                latitude = lat;
                longitude = lon;
            }

            void pingTelemetry() const {
                std::cout << "    [GPS Beacon PING] Lat: " << std::fixed << std::setprecision(4) << latitude
                          << ", Lon: " << longitude << " | Battery: " << batteryPct << "%\n";
            }
        };

        // Independent Emergency Life Support Unit composed inside vehicle agents
        class EmergencyLifeSupportUnit {
        private:
            int oxygenLevelPct;
            bool defibrillatorReady;
            int triageBedCapacity;

        public:
            EmergencyLifeSupportUnit(int o2 = 100, bool defib = true, int beds = 2)
                : oxygenLevelPct(o2), defibrillatorReady(defib), triageBedCapacity(beds) {}

            int getOxygenLevelPct() const { return oxygenLevelPct; }
            bool isDefibrillatorReady() const { return defibrillatorReady; }
            int getTriageBedCapacity() const { return triageBedCapacity; }

            bool verifyReadiness() const {
                return (oxygenLevelPct > 20 && defibrillatorReady);
            }

            void displayLifeSupportStatus() const {
                std::cout << "    [Life Support Unit] O2 Level: " << oxygenLevelPct 
                          << "% | Defib Ready: " << (defibrillatorReady ? "YES" : "NO")
                          << " | Triage Beds: " << triageBedCapacity << "\n";
            }
        };

        // ====================================================================
        // UNIT II & III: BASE CLASS ReliefVehicle (ALIASED AS TransportVehicle)
        // ====================================================================
        class ReliefVehicle {
        protected:
            std::string vehicleId;      // Unique Identifier (e.g., "TRK-01", "HELI-02")
            std::string callsign;       // Field Callsign (e.g., "Sierra-1", "Hawk-3")
            double maxPayloadTons;      // Maximum Carrying Capacity in Metric Tons
            double fuelRangeKm;         // Maximum Operating Range without Refueling in km

            // UNIT III: Containership (has-a relationship)
            GPSBeacon beacon;
            EmergencyLifeSupportUnit lifeSupport;

            // UNIT II: Dynamic Memory Allocation for Transport Manifest (Rule of Three)
            ReliefOps::Inventory::ReliefCargo* manifest;
            int manifestCount;
            int manifestCapacity;

            // UNIT II: Real-time Logistics Analytics (Static Variables)
            static double totalDispatchedTonnage;
            static int totalOperationalVehicles;

            // UNIT I: Friend Class Declaration
            friend class IncidentCommander;

        public:
            // Dynamic Parameterized Constructor
            ReliefVehicle(std::string id, std::string callsign, double maxPayload, 
                          double fuelRange, int capacity = 10);

            // UNIT II: Deep Copy Constructor (Rule of Three)
            ReliefVehicle(const ReliefVehicle& other);

            // UNIT II: Copy Assignment Operator (Rule of Three)
            ReliefVehicle& operator=(const ReliefVehicle& other);

            // UNIT II & III: Explicit Virtual Destructor (Rule of Three & Polymorphism)
            virtual ~ReliefVehicle();

            // Getters
            std::string getVehicleId() const { return vehicleId; }
            std::string getCallsign() const { return callsign; }
            double getMaxPayloadTons() const { return maxPayloadTons; }
            double getFuelRangeKm() const { return fuelRangeKm; }
            int getManifestCount() const { return manifestCount; }
            int getManifestCapacity() const { return manifestCapacity; }

            // Dynamic Manifest bounds checking & inspection
            double getCurrentPayloadWeight() const;
            bool canAcceptPayload(const ReliefOps::Inventory::ReliefCargo& cargo) const;
            void addCargoToManifest(const ReliefOps::Inventory::ReliefCargo& cargo);
            void displayManifest() const;

            // UNIT II: Static Analytics Functions
            static double getTotalDispatchedTonnage() { return totalDispatchedTonnage; }
            static int getTotalOperationalVehicles() { return totalOperationalVehicles; }
            static void resetAnalytics() { totalDispatchedTonnage = 0.0; }

            // ================================================================
            // UNIT III: RUNTIME POLYMORPHISM - PURE VIRTUAL INTERFACES
            // ================================================================
            virtual bool evaluateRoute(const std::string& corridorType, double distanceKm) const = 0;
            virtual void executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) = 0;
            virtual void displayDetails() const;
        };

        // ====================================================================
        // UNIT II & III: NAMING RESOLUTION TYPE ALIAS
        // ====================================================================
        // TransportVehicle is declared as a type alias for ReliefVehicle.
        // This guarantees strict compliance with Unit II and Unit III naming without
        // introducing redundant inheritance layers.
        using TransportVehicle = ReliefVehicle;

        // ====================================================================
        // UNIT III: DERIVED CLASS 1 - GroundConvoy (VIRTUAL INHERITANCE)
        // ====================================================================
        class GroundConvoy : virtual public ReliefVehicle {
        protected:
            int allTerrainAxles;

        public:
            GroundConvoy(std::string id, std::string callsign, double maxPayload, 
                         double fuelRange, int axles = 4, int capacity = 10);

            int getAxles() const { return allTerrainAxles; }

            bool evaluateRoute(const std::string& corridorType, double distanceKm) const override;
            void executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) override;
            void displayDetails() const override;
        };

        // ====================================================================
        // UNIT III: DERIVED CLASS 2 - AerialTransport (DIRECT INHERITANCE)
        // ====================================================================
        class AerialTransport : public ReliefVehicle {
        protected:
            double maxFlightCeilingMeters;

        public:
            AerialTransport(std::string id, std::string callsign, double maxPayload, 
                            double fuelRange, double ceiling = 4500.0, int capacity = 10);

            double getFlightCeiling() const { return maxFlightCeilingMeters; }

            bool evaluateRoute(const std::string& corridorType, double distanceKm) const override;
            void executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) override;
            void displayDetails() const override;
        };

        // ====================================================================
        // UNIT III: DERIVED CLASS 3 - AmphibiousCraft (VIRTUAL INHERITANCE)
        // ====================================================================
        class AmphibiousCraft : virtual public ReliefVehicle {
        protected:
            double draftDepthMeters;

        public:
            AmphibiousCraft(std::string id, std::string callsign, double maxPayload, 
                            double fuelRange, double draft = 1.2, int capacity = 10);

            double getDraftDepth() const { return draftDepthMeters; }

            bool evaluateRoute(const std::string& corridorType, double distanceKm) const override;
            void executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) override;
            void displayDetails() const override;
        };

        // ====================================================================
        // UNIT III: DIAMOND INHERITANCE HYBRID - AmphibiousMedicalUnit
        // ====================================================================
        // Inherits from GroundConvoy and AmphibiousCraft, both of which inherit
        // virtually from ReliefVehicle. Resolves base duplication cleanly!
        class AmphibiousMedicalUnit : public GroundConvoy, public AmphibiousCraft {
        private:
            bool mobileSurgeryDeployed;

        public:
            AmphibiousMedicalUnit(std::string id, std::string callsign, double maxPayload, 
                                  double fuelRange, int axles = 6, double draft = 1.5, 
                                  bool surgery = true, int capacity = 10);

            bool isSurgeryDeployed() const { return mobileSurgeryDeployed; }

            // UNIT III: Explicit Final Overriders eliminating competing base overrides
            bool evaluateRoute(const std::string& corridorType, double distanceKm) const override;
            void executeDispatch(const ReliefOps::Inventory::ReliefCargo& cargo) override;
            void displayDetails() const override;
        };

    } // namespace Logistics
} // namespace ReliefOps

#endif // VEHICLE_HPP
