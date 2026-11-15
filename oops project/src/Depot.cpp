#include "../include/Depot.hpp"
#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace ReliefOps {
    namespace Logistics {

        // Default Constructor
        DistributionDepot::DistributionDepot()
            : depotId("DEPOT-CENTRAL"), locationName("Kathmandu Relief Hub Alpha"),
              privateReserveSerumLitres(50.0) {}

        // Parameterized Constructor
        DistributionDepot::DistributionDepot(std::string id, std::string location, double initialReserveLitres)
            : depotId(id), locationName(location),
              privateReserveSerumLitres(initialReserveLitres > 0.0 ? initialReserveLitres : 0.0) {}

        // Check if cargo exists in depot
        bool DistributionDepot::hasCargo(const std::string& cargoId) const {
            return inventory.find(cargoId) != inventory.end();
        }

        // ====================================================================
        // UNIT I: RETURN BY REFERENCE
        // ====================================================================
        // Returns an l-value reference allowing direct in-place stock updates
        ReliefOps::Inventory::ReliefCargo& DistributionDepot::getCargoItem(const std::string& cargoId) {
            auto it = inventory.find(cargoId);
            if (it == inventory.end()) {
                throw std::out_of_range("Cargo SKU '" + cargoId + "' does not exist in depot inventory.");
            }
            return it->second; // Return l-value reference
        }

        // ====================================================================
        // UNIT I: FUNCTION OVERLOADING FOR SUPPLY ALLOCATION
        // ====================================================================

        // Overload 1: Volume/payload-based allocation for general sustenance (rations, water)
        bool DistributionDepot::allocateSupplies(double requiredVolumeM3, double requiredWeightTons, 
                                                 const std::string& cargoType) {
            std::cout << "\n  [SUPPLY ALLOCATION: VOLUME/PAYLOAD ROUTINE]\n"
                      << "  Evaluating bulk payload for " << cargoType << "...\n"
                      << "  Required Volume: " << requiredVolumeM3 << " m3 | Required Weight: " << requiredWeightTons << " T\n";

            for (auto& pair : inventory) {
                auto& item = pair.second;
                // Match non-medical bulk items by cargo description or SKU
                if (!item.getIsMedical() && (item.getDescription().find(cargoType) != std::string::npos ||
                                             item.getCargoId().find(cargoType) != std::string::npos)) {
                    if (item.getVolumeM3() >= requiredVolumeM3 && item.getWeightTons() >= requiredWeightTons) {
                        std::cout << statusGreen << "Matched stockpile: " << item.getDescription() 
                                  << " (" << item.getCargoId() << "). Available: " << item.getQuantity() << " units.\n";
                        return true;
                    }
                }
            }
            std::cout << alertRed << "Insufficient bulk stock to satisfy volume/payload request for " << cargoType << ".\n";
            return false;
        }

        // Overload 2: Priority emergency medical triage allocation for critical pharmaceuticals
        bool DistributionDepot::allocateSupplies(int triagePriorityLevel, const std::string& medicineType, 
                                                 int unitCount) {
            std::cout << "\n  [SUPPLY ALLOCATION: EMERGENCY MEDICAL TRIAGE ROUTINE]\n"
                      << "  Evaluating priority medical supplies for " << medicineType << "...\n"
                      << "  Triage Priority: Level " << triagePriorityLevel << " | Required Units: " << unitCount << "\n";

            for (auto& pair : inventory) {
                auto& item = pair.second;
                // Match medical triage items by priority level and type
                if (item.getIsMedical() && item.getPriorityLevel() <= triagePriorityLevel &&
                    item.getDescription().find(medicineType) != std::string::npos) {
                    if (item.getQuantity() >= unitCount) {
                        std::cout << statusGreen << "Triage allocation APPROVED for: " << item.getDescription()
                                  << " (" << item.getCargoId() << ") [Priority " << item.getPriorityLevel() << "].\n";
                        return true;
                    }
                }
            }
            std::cout << alertRed << "No matching medical triage supplies available for: " << medicineType 
                      << " at Priority Level " << triagePriorityLevel << ".\n";
            return false;
        }

        // ====================================================================
        // UNIT II: OPERATOR OVERLOADING
        // ====================================================================

        // Overload += : Restock supplies at transit depots
        DistributionDepot& DistributionDepot::operator+=(const ReliefOps::Inventory::ReliefCargo& incoming) {
            auto it = inventory.find(incoming.getCargoId());
            if (it != inventory.end()) {
                // Item exists: update quantity and aggregate weight in-place
                int newQty = it->second.getQuantity() + incoming.getQuantity();
                double newWt = it->second.getWeightTons() + incoming.getWeightTons();
                it->second.setQuantity(newQty);
                it->second.setWeightTons(newWt);
                std::cout << "  [DEPOT RESTOCKED] Existing item " << incoming.getCargoId() 
                          << " increased to " << newQty << " units (" << newWt << " T).\n";
            } else {
                // New SKU: insert into unordered_map
                inventory[incoming.getCargoId()] = incoming;
                std::cout << "  [DEPOT RESTOCKED] New SKU " << incoming.getCargoId() 
                          << " (" << incoming.getDescription() << ") registered with " 
                          << incoming.getQuantity() << " units.\n";
            }
            return *this;
        }

        // Overload -= : Deduct cargo upon vehicle departure
        DistributionDepot& DistributionDepot::operator-=(const ReliefOps::Inventory::ReliefCargo& dispatched) {
            auto it = inventory.find(dispatched.getCargoId());
            if (it != inventory.end()) {
                int currentQty = it->second.getQuantity();
                int dispatchQty = dispatched.getQuantity();
                if (currentQty <= dispatchQty) {
                    std::cout << "  [DEPOT DEDUCTION] Entire stock of " << it->first << " depleted and removed from hub.\n";
                    inventory.erase(it);
                } else {
                    it->second.setQuantity(currentQty - dispatchQty);
                    it->second.setWeightTons(it->second.getWeightTons() - dispatched.getWeightTons());
                    std::cout << "  [DEPOT DEDUCTION] " << dispatchQty << " units of " << it->first 
                              << " deducted. Remaining: " << (currentQty - dispatchQty) << " units.\n";
                }
            }
            return *this;
        }

        void DistributionDepot::displayInventory() const {
            std::cout << "\n================================================================================\n"
                      << "   DISTRIBUTION DEPOT INVENTORY AUDIT: " << depotId << " (" << locationName << ")\n"
                      << "   Total Active SKUs: " << inventory.size() << " | Restricted Plasma Reserve: " 
                      << privateReserveSerumLitres << " Litres\n"
                      << "================================================================================\n";

            if (inventory.empty()) {
                std::cout << "   (Depot inventory is currently empty)\n\n";
                return;
            }

            std::cout << tableHeader << "\n";
            for (const auto& pair : inventory) {
                std::cout << pair.second << "\n";
            }
            std::cout << dividerLine << "\n\n";
        }

        // ====================================================================
        // UNIT I: FRIEND CLASS IncidentCommander IMPLEMENTATION
        // ====================================================================
        // Accesses private member 'depot.privateReserveSerumLitres' directly!
        bool IncidentCommander::overrideTriageReserve(DistributionDepot& depot, double requiredLitres, ReliefVehicle& vehicle) {
            std::cout << alertRed << "INCIDENT COMMAND TRIAGE OVERRIDE INITIATED by " 
                      << callsign << " (" << commanderId << ")\n";
            std::cout << "  Checking restricted emergency serum vault at " << depot.getDepotId() << "...\n";

            // Direct access to private member of DistributionDepot
            if (depot.privateReserveSerumLitres < requiredLitres) {
                std::cout << "  [OVERRIDE DENIED] Restricted reserve has only " 
                          << depot.privateReserveSerumLitres << " Litres available. Cannot satisfy " 
                          << requiredLitres << " Litres.\n";
                return false;
            }

            depot.privateReserveSerumLitres -= requiredLitres;
            std::cout << "  [OVERRIDE APPROVED] " << requiredLitres 
                      << " Litres of restricted emergency serum released from vault!\n"
                      << "  Remaining restricted reserve: " << depot.privateReserveSerumLitres << " Litres.\n";

            // Create emergency triage cargo for immediate dispatch
            ReliefOps::Inventory::ReliefCargo emergencyCargo(
                "RES-PLASMA", "Emergency Blood Plasma / Antivenom", 
                requiredLitres * 0.001, requiredLitres * 0.0015, 1, true, 1
            );

            // Attempt to load into vehicle
            try {
                vehicle.addCargoToManifest(emergencyCargo);
                std::cout << statusGreen << "Emergency serum cargo loaded onto vehicle " 
                          << vehicle.getVehicleId() << " for immediate triage mission.\n";
                return true;
            } catch (const std::exception& ex) {
                std::cout << "  [OVERRIDE FAILED TO LOAD] " << ex.what() << "\n";
                // Return serum to reserve
                depot.privateReserveSerumLitres += requiredLitres;
                return false;
            }
        }

    } // namespace Logistics
} // namespace ReliefOps
