#ifndef DEPOT_HPP
#define DEPOT_HPP

#include "Common.hpp"
#include "Cargo.hpp"
#include "Vehicle.hpp"
#include <string>
#include <unordered_map>
#include <iostream>

namespace ReliefOps {
    namespace Logistics {

        // ====================================================================
        // UNIT II: ABSTRACT DATA TYPE - DistributionDepot
        // ====================================================================
        class DistributionDepot {
        private:
            std::string depotId;
            std::string locationName;

            // UNIT IV: STL Container - std::unordered_map for O(1) inventory lookup
            std::unordered_map<std::string, ReliefOps::Inventory::ReliefCargo> inventory;

            // Restricted emergency triage reserve (accessible only via friend class IncidentCommander)
            double privateReserveSerumLitres;

            // UNIT I: Friend Class Declaration
            friend class IncidentCommander;

        public:
            // Constructors
            DistributionDepot();
            DistributionDepot(std::string id, std::string location, double initialReserveLitres = 50.0);

            // Getters
            std::string getDepotId() const { return depotId; }
            std::string getLocationName() const { return locationName; }
            double getPrivateReserveLitres() const { return privateReserveSerumLitres; }
            size_t getInventoryItemCount() const { return inventory.size(); }
            const std::unordered_map<std::string, ReliefOps::Inventory::ReliefCargo>& getInventory() const {
                return inventory;
            }

            // ================================================================
            // UNIT I: RETURN BY REFERENCE
            // ================================================================
            // Returns an l-value reference to the cargo item inside the depot inventory,
            // allowing direct in-place stock updates without copying.
            ReliefOps::Inventory::ReliefCargo& getCargoItem(const std::string& cargoId);
            bool hasCargo(const std::string& cargoId) const;

            // ================================================================
            // UNIT I: FUNCTION OVERLOADING FOR SUPPLY ALLOCATION
            // ================================================================

            // Overload 1: Volume/payload-based allocation for general sustenance (rations, blankets)
            bool allocateSupplies(double requiredVolumeM3, double requiredWeightTons, 
                                  const std::string& cargoType);

            // Overload 2: Priority emergency medical triage allocation for critical pharmaceuticals
            bool allocateSupplies(int triagePriorityLevel, const std::string& medicineType, 
                                  int unitCount);

            // ================================================================
            // UNIT II: OPERATOR OVERLOADING
            // ================================================================

            // Overload += : Restock supplies at transit depots
            DistributionDepot& operator+=(const ReliefOps::Inventory::ReliefCargo& incoming);

            // Overload -= : Deduct cargo upon vehicle departure
            DistributionDepot& operator-=(const ReliefOps::Inventory::ReliefCargo& dispatched);

            // Display routines
            void displayInventory() const;
        };

        // ====================================================================
        // UNIT I: FRIEND CLASS IncidentCommander
        // ====================================================================
        // Authorized emergency coordinator capable of overriding access barriers
        // to deploy private emergency reserves and bypass dispatch quotas.
        class IncidentCommander {
        private:
            std::string commanderId;
            std::string callsign;

        public:
            IncidentCommander(std::string id = "CMD-01", std::string call = "Commander Vance")
                : commanderId(id), callsign(call) {}

            std::string getCommanderId() const { return commanderId; }
            std::string getCallsign() const { return callsign; }

            // Accesses private data member 'privateReserveSerumLitres' in DistributionDepot directly!
            bool overrideTriageReserve(DistributionDepot& depot, double requiredLitres, ReliefVehicle& vehicle);
        };

    } // namespace Logistics
} // namespace ReliefOps

#endif // DEPOT_HPP
