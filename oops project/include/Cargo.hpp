#ifndef CARGO_HPP
#define CARGO_HPP

#include "Common.hpp"
#include <string>
#include <iostream>
#include <iomanip>

namespace ReliefOps {
    namespace Inventory {

        // ====================================================================
        // UNIT II: ABSTRACT DATA TYPE - ReliefCargo (INFORMATION HIDING)
        // ====================================================================
        class ReliefCargo {
        private:
            std::string cargoId;       // Unique SKU / Manifest Identifier (e.g., "MED-01")
            std::string description;   // Cargo Name (e.g., "Trauma Surgical Kits")
            double weightTons;         // Weight in Metric Tons
            double volumeM3;           // Volume in Cubic Meters
            int priorityLevel;         // 1 = Critical Triage, 2 = High, 3 = Standard
            bool isMedical;            // Flag indicating specialized pharmaceutical/medical supply
            int quantity;              // Package or unit count

        public:
            // Default Constructor
            ReliefCargo();

            // Parameterized Constructor
            ReliefCargo(std::string id, std::string desc, double weight, double vol, 
                        int priority, bool medical, int qty = 1);

            // Getters (Information Hiding)
            std::string getCargoId() const { return cargoId; }
            std::string getDescription() const { return description; }
            double getWeightTons() const { return weightTons; }
            double getVolumeM3() const { return volumeM3; }
            int getPriorityLevel() const { return priorityLevel; }
            bool getIsMedical() const { return isMedical; }
            int getQuantity() const { return quantity; }

            // Setters with validation
            void setQuantity(int qty);
            void setWeightTons(double weight);

            // ================================================================
            // UNIT II: OPERATOR OVERLOADING
            // ================================================================

            // Overloaded == : Verify manifest identity and detect duplicate cargo
            bool operator==(const ReliefCargo& other) const;

            // Overloaded << : Stream insertion for tabular printing and manifest display
            friend std::ostream& operator<<(std::ostream& os, const ReliefCargo& cargo);

            // Overloaded >> : Stream extraction for user-friendly keyboard input
            friend std::istream& operator>>(std::istream& is, ReliefCargo& cargo);
        };

        // ====================================================================
        // UNIT IV: CUSTOM FUNCTOR FOR SORTING URGENT MEDICAL REQUIREMENTS
        // ====================================================================
        // Functor used by PriorityDispatchQueue to prioritize life-saving supplies
        // In a max-heap priority queue, returning true means 'a' has lower urgency than 'b'.
        struct UrgentMedicalComparator {
            bool operator()(const ReliefCargo& a, const ReliefCargo& b) const {
                // Criterion 1: Smaller priority number means higher emergency triage urgency (1 > 2 > 3)
                if (a.getPriorityLevel() != b.getPriorityLevel()) {
                    return a.getPriorityLevel() > b.getPriorityLevel(); // 'a' has lower urgency if priority level is higher
                }
                // Criterion 2: If priority numbers are equal, medical cargo takes precedence
                if (a.getIsMedical() != b.getIsMedical()) {
                    return !a.getIsMedical() && b.getIsMedical(); // medical precedes non-medical
                }
                // Criterion 3: If still tied, heavier cargo or higher unit count takes precedence
                return a.getWeightTons() < b.getWeightTons();
            }
        };

    } // namespace Inventory
} // namespace ReliefOps

#endif // CARGO_HPP
