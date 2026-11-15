#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <exception>

// ============================================================================
// UNIT I: LOGICAL NAMESPACES
// ============================================================================
namespace ReliefOps {
    namespace Inventory {
        class ReliefCargo;
        struct UrgentMedicalComparator;
    }

    namespace Logistics {
        class ReliefVehicle;
        // UNIT II & III NAMING RESOLUTION:
        // TransportVehicle is defined as a transparent type alias for ReliefVehicle.
        // This satisfies Unit II ("TransportVehicle") and Unit III ("ReliefVehicle")
        // without adding redundant inheritance wrappers.
        using TransportVehicle = ReliefVehicle;
        class DistributionDepot;
        class IncidentCommander;
    }
}

// ============================================================================
// UNIT IV: CUSTOM EXCEPTION FRAMEWORK DERIVED FROM std::exception
// ============================================================================
namespace ReliefOps {
    namespace Logistics {

        // Thrown when cargo weight or manifest item count exceeds vehicle capacity
        class CargoOverweightException : public std::exception {
        private:
            std::string message;
        public:
            explicit CargoOverweightException(const std::string& msg)
                : message("[DISASTER LOGISTICS ERROR - OVERWEIGHT] " + msg) {}

            const char* what() const noexcept override {
                return message.c_str();
            }
        };

        // Thrown when transit corridor distance exceeds vehicle fuel range
        class FuelDepletionException : public std::exception {
        private:
            std::string message;
        public:
            explicit FuelDepletionException(const std::string& msg)
                : message("[DISASTER LOGISTICS ERROR - FUEL RANGE] " + msg) {}

            const char* what() const noexcept override {
                return message.c_str();
            }
        };

        // Thrown when damaged infrastructure or terrain blocks the vehicle type
        class DamagedCorridorException : public std::exception {
        private:
            std::string message;
        public:
            explicit DamagedCorridorException(const std::string& msg)
                : message("[DISASTER LOGISTICS ERROR - DAMAGED CORRIDOR] " + msg) {}

            const char* what() const noexcept override {
                return message.c_str();
            }
        };

    } // namespace Logistics
} // namespace ReliefOps

// ============================================================================
// UNIT I: INLINE UTILITY METHODS FOR RAPID LOGISTICS COMPUTATION
// ============================================================================
namespace ReliefOps {
    namespace Logistics {

        // Computes straight-line Euclidean distance between field coordinates (km)
        inline double calculateDistance(double x1, double y1, double x2, double y2) {
            double dx = x2 - x1;
            double dy = y2 - y1;
            return std::sqrt(dx * dx + dy * dy);
        }

        // Computes current payload capacity ratio (0.0 to 1.0+)
        inline double computePayloadRatio(double currentWeight, double maxCapacity) {
            if (maxCapacity <= 0.0) return 0.0;
            return currentWeight / maxCapacity;
        }

        // Estimates fuel required based on transit corridor distance and burn rate
        inline double estimateFuelRequirement(double distanceKm, double burnRatePerKm) {
            return distanceKm * burnRatePerKm;
        }

    } // namespace Logistics
} // namespace ReliefOps

// ============================================================================
// UNIT IV: USER-DEFINED STREAM MANIPULATORS FOR TABULAR PRINTING
// ============================================================================
namespace ReliefOps {
    namespace Logistics {

        // Manipulator: Divider line for tabular manifest printing
        inline std::ostream& dividerLine(std::ostream& os) {
            os << "+------------+----------------------+------------+------------+----------+----------+";
            return os;
        }

        // Manipulator: Tabular column headers for inventory manifests
        inline std::ostream& tableHeader(std::ostream& os) {
            os << dividerLine << "\n"
               << "| " << std::left  << std::setw(10) << "Cargo ID"
               << " | " << std::left  << std::setw(20) << "Description"
               << " | " << std::right << std::setw(10) << "Weight (T)"
               << " | " << std::right << std::setw(10) << "Volume(m3)"
               << " | " << std::right << std::setw(8)  << "Priority"
               << " | " << std::left  << std::setw(8)  << "Medical?"
               << " |\n"
               << dividerLine;
            return os;
        }

        // Manipulator: High-priority emergency alert prefix
        inline std::ostream& alertRed(std::ostream& os) {
            os << ">>> [CRITICAL EMERGENCY ALERT] ";
            return os;
        }

        // Manipulator: Operational success prefix
        inline std::ostream& statusGreen(std::ostream& os) {
            os << "[STATUS: OPERATIONAL CONFIRMED] ";
            return os;
        }

    } // namespace Logistics
} // namespace ReliefOps

#endif // COMMON_HPP
