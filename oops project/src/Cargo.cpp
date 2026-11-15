#include "../include/Cargo.hpp"
#include <limits>

namespace ReliefOps {
    namespace Inventory {

        // Default Constructor
        ReliefCargo::ReliefCargo()
            : cargoId("UNASSIGNED"), description("Empty"), weightTons(0.0),
              volumeM3(0.0), priorityLevel(3), isMedical(false), quantity(0) {}

        // Parameterized Constructor
        ReliefCargo::ReliefCargo(std::string id, std::string desc, double weight, double vol, 
                                 int priority, bool medical, int qty)
            : cargoId(id), description(desc), 
              weightTons(weight > 0.0 ? weight : 0.0),
              volumeM3(vol > 0.0 ? vol : 0.0),
              priorityLevel(priority >= 1 && priority <= 3 ? priority : 3),
              isMedical(medical),
              quantity(qty >= 0 ? qty : 0) {}

        // Setters with validation
        void ReliefCargo::setQuantity(int qty) {
            if (qty >= 0) {
                quantity = qty;
            }
        }

        void ReliefCargo::setWeightTons(double weight) {
            if (weight >= 0.0) {
                weightTons = weight;
            }
        }

        // ====================================================================
        // UNIT II: OPERATOR OVERLOADING IMPLEMENTATIONS
        // ====================================================================

        // Operator == : Checks manifest identity and duplicate cargo
        bool ReliefCargo::operator==(const ReliefCargo& other) const {
            return (cargoId == other.cargoId);
        }

        // Operator << : Formatted stream insertion for tabular manifest output
        std::ostream& operator<<(std::ostream& os, const ReliefCargo& cargo) {
            os << "| " << std::left  << std::setw(10) << cargo.cargoId
               << " | " << std::left  << std::setw(20) << cargo.description.substr(0, 20)
               << " | " << std::right << std::fixed << std::setprecision(3) << std::setw(10) << cargo.weightTons
               << " | " << std::right << std::fixed << std::setprecision(2) << std::setw(10) << cargo.volumeM3
               << " | " << std::right << std::setw(8)  << cargo.priorityLevel
               << " | " << std::left  << std::setw(8)  << (cargo.isMedical ? "YES" : "NO")
               << " |";
            return os;
        }

        // Operator >> : User-friendly stream extraction for keyboard entry
        std::istream& operator>>(std::istream& is, ReliefCargo& cargo) {
            std::cout << "  -> Enter Cargo SKU / ID (e.g., MED-05, RAT-12): ";
            is >> cargo.cargoId;
            is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "  -> Enter Description (e.g., Blood Plasma Units): ";
            std::getline(is, cargo.description);

            std::cout << "  -> Enter Weight in Metric Tons (e.g., 1.5): ";
            while (!(is >> cargo.weightTons) || cargo.weightTons <= 0.0) {
                std::cout << "     [!] Invalid weight. Enter a positive number: ";
                is.clear();
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            std::cout << "  -> Enter Volume in Cubic Meters (e.g., 2.4): ";
            while (!(is >> cargo.volumeM3) || cargo.volumeM3 <= 0.0) {
                std::cout << "     [!] Invalid volume. Enter a positive number: ";
                is.clear();
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            std::cout << "  -> Enter Triage Priority Level [1=Critical Triage, 2=High, 3=Standard]: ";
            while (!(is >> cargo.priorityLevel) || cargo.priorityLevel < 1 || cargo.priorityLevel > 3) {
                std::cout << "     [!] Invalid priority. Enter 1, 2, or 3: ";
                is.clear();
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            char medChoice;
            std::cout << "  -> Is this specialized Medical / Pharmaceutical supply? (y/n): ";
            is >> medChoice;
            cargo.isMedical = (medChoice == 'y' || medChoice == 'Y');

            std::cout << "  -> Enter Package Quantity Count: ";
            while (!(is >> cargo.quantity) || cargo.quantity < 1) {
                std::cout << "     [!] Invalid quantity. Enter at least 1: ";
                is.clear();
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            return is;
        }

    } // namespace Inventory
} // namespace ReliefOps
