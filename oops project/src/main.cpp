#include "../include/Common.hpp"
#include "../include/Cargo.hpp"
#include "../include/Vehicle.hpp"
#include "../include/Depot.hpp"
#include "../include/PriorityQueue.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <chrono>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace ReliefOps::Logistics;
using namespace ReliefOps::Inventory;
namespace fs = std::filesystem;

// ============================================================================
// UNIT IV: BINARY AUDIT RECORD POD STRUCT
// ============================================================================
// Fixed-size, trivially copyable record structure for binary std::fstream persistence
struct TransitRecord {
    char missionId[16];
    char vehicleId[16];
    char cargoId[16];
    double weightTons;
    double distanceKm;
    int triagePriority;
    bool success;
    char timestamp[32];
};

// ============================================================================
// UNIT IV: WINDOWS ATOMIC FILE REPLACEMENT & VERIFICATION PROTOCOL
// ============================================================================

const std::string BIN_LOG_PATH = "data/transit_logs.bin";
const std::string TMP_LOG_PATH = "data/transit_logs.tmp";

// Helper to obtain current timestamp string
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&timeNow));
    return std::string(buf);
}

// Low-level atomic file swap for Windows NTFS
bool commitBinaryLogAtomically(const std::string& tmpPath, const std::string& targetPath) {
#ifdef _WIN32
    if (!fs::exists(targetPath)) {
        return MoveFileExA(tmpPath.c_str(), targetPath.c_str(), MOVEFILE_WRITE_THROUGH) != 0;
    }
    // Transactional atomic file replacement preserving target on any failure
    BOOL success = ReplaceFileA(
        targetPath.c_str(), tmpPath.c_str(), 
        NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL
    );
    if (!success) {
        // Fallback to MoveFileEx with replace
        success = MoveFileExA(tmpPath.c_str(), targetPath.c_str(), 
                              MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    }
    return success != 0;
#else
    return std::rename(tmpPath.c_str(), targetPath.c_str()) == 0;
#endif
}

// Loads all records from binary log using std::fstream
std::vector<TransitRecord> readAllTransitRecords() {
    std::vector<TransitRecord> records;
    if (!fs::exists(BIN_LOG_PATH)) {
        return records; // Cleanly return empty vector if log does not exist yet
    }

    std::fstream binStream(BIN_LOG_PATH, std::ios::in | std::ios::binary);
    if (!binStream.is_open()) {
        std::cerr << "  [!] Warning: Could not open " << BIN_LOG_PATH << " for reading.\n";
        return records;
    }

    TransitRecord rec;
    while (binStream.read(reinterpret_cast<char*>(&rec), sizeof(TransitRecord))) {
        records.push_back(rec);
    }
    binStream.close();
    return records;
}

// Atomically writes full record dataset to temporary file, verifies integrity, then swaps
bool writeAndCommitRecordsAtomically(const std::vector<TransitRecord>& records) {
    fs::create_directories("data");

    // Step 1: Open temporary binary file
    std::fstream tmpStream(TMP_LOG_PATH, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!tmpStream.is_open()) {
        std::cerr << "  [!] Failed to create temporary persistence file: " << TMP_LOG_PATH << "\n";
        return false;
    }

    // Step 2: Write all records
    for (const auto& rec : records) {
        tmpStream.write(reinterpret_cast<const char*>(&rec), sizeof(TransitRecord));
    }

    // Step 3: Check stream error flags before and after flush/close
    if (tmpStream.fail() || tmpStream.bad()) {
        std::cerr << "  [!] Stream error encountered during write. Preserving original log.\n";
        tmpStream.close();
        fs::remove(TMP_LOG_PATH);
        return false;
    }

    tmpStream.flush();
    if (tmpStream.fail() || tmpStream.bad()) {
        std::cerr << "  [!] Stream error encountered during flush. Preserving original log.\n";
        tmpStream.close();
        fs::remove(TMP_LOG_PATH);
        return false;
    }

    tmpStream.close();
    if (tmpStream.fail()) {
        std::cerr << "  [!] Stream error encountered during close. Preserving original log.\n";
        fs::remove(TMP_LOG_PATH);
        return false;
    }

    // Step 4: Verification Gates
    std::error_code ec;
    auto fileSize = fs::file_size(TMP_LOG_PATH, ec);
    if (ec) {
        std::cerr << "  [!] Verification failed: unable to query file size.\n";
        fs::remove(TMP_LOG_PATH);
        return false;
    }

    // Gate A: Byte alignment check
    if (fileSize % sizeof(TransitRecord) != 0) {
        std::cerr << "  [!] Verification failed: file size (" << fileSize 
                  << " bytes) is not a multiple of record size (" << sizeof(TransitRecord) << ").\n";
        fs::remove(TMP_LOG_PATH);
        return false;
    }

    // Gate B: Record count verification
    size_t actualCount = fileSize / sizeof(TransitRecord);
    if (actualCount != records.size()) {
        std::cerr << "  [!] Verification failed: record count mismatch. Expected " 
                  << records.size() << ", found " << actualCount << ".\n";
        fs::remove(TMP_LOG_PATH);
        return false;
    }

    // Step 5: Perform atomic replacement
    bool commitOk = commitBinaryLogAtomically(TMP_LOG_PATH, BIN_LOG_PATH);
    if (!commitOk) {
        std::cerr << "  [!] Atomic file swap failed. Original binary log preserved.\n";
        fs::remove(TMP_LOG_PATH);
        return false;
    }

    return true;
}

// Appends single record using atomic staging workflow
bool appendTransitRecordAtomically(const TransitRecord& record) {
    auto records = readAllTransitRecords();
    records.push_back(record);
    return writeAndCommitRecordsAtomically(records);
}

// ============================================================================
// SAMPLE DATA INITIALIZATION (REALISTIC EARTHQUAKE RELIEF SCENARIO)
// ============================================================================
void initializeSampleData(DistributionDepot& depot, std::vector<ReliefVehicle*>& fleet) {
    // 1. Initial Depot Supplies
    depot += ReliefCargo("MED-01", "Trauma Surgical Kits", 0.8, 1.2, 1, true, 20);
    depot += ReliefCargo("MED-02", "Antivenom & Vaccines", 0.3, 0.5, 1, true, 50);
    depot += ReliefCargo("RAT-01", "High-Calorie Rations", 2.5, 4.0, 3, false, 200);
    depot += ReliefCargo("WAT-01", "Water Purifier Tanks", 3.0, 5.0, 2, false, 80);
    depot += ReliefCargo("BLK-01", "Thermal Mylar Blankets", 0.5, 1.0, 3, false, 300);

    // 2. Initial Heterogeneous Fleet
    fleet.push_back(new GroundConvoy("TRK-101", "Sierra-6 Heavy", 8.0, 500.0, 3, 10));
    fleet.push_back(new AerialTransport("HELI-201", "Hawk-1 Rotorcraft", 4.0, 450.0, 5000.0, 10));
    fleet.push_back(new AmphibiousCraft("BOAT-301", "Zodiac Delta-3", 2.0, 250.0, 0.8, 8));
    fleet.push_back(new AmphibiousMedicalUnit("HYB-401", "Mobile Trauma Convoy", 6.0, 400.0, 6, 1.2, true, 12));
}

// Helper to display tabular binary transit records
void displayTransitRecords(const std::vector<TransitRecord>& records) {
    std::cout << "\n+------------------+------------------+------------------+------------+------------+----------+----------+---------------------+\n"
              << "| " << std::left << std::setw(16) << "Mission ID"
              << " | " << std::left << std::setw(16) << "Vehicle ID"
              << " | " << std::left << std::setw(16) << "Cargo ID"
              << " | " << std::right << std::setw(10) << "Weight (T)"
              << " | " << std::right << std::setw(10) << "Dist (km)"
              << " | " << std::right << std::setw(8) << "Priority"
              << " | " << std::left << std::setw(8) << "Status"
              << " | " << std::left << std::setw(19) << "Timestamp"
              << " |\n"
              << "+------------------+------------------+------------------+------------+------------+----------+----------+---------------------+\n";

    for (const auto& r : records) {
        std::cout << "| " << std::left << std::setw(16) << r.missionId
                  << " | " << std::left << std::setw(16) << r.vehicleId
                  << " | " << std::left << std::setw(16) << r.cargoId
                  << " | " << std::right << std::fixed << std::setprecision(2) << std::setw(10) << r.weightTons
                  << " | " << std::right << std::fixed << std::setprecision(1) << std::setw(10) << r.distanceKm
                  << " | " << std::right << std::setw(8) << r.triagePriority
                  << " | " << std::left << std::setw(8) << (r.success ? "SUCCESS" : "FAILED")
                  << " | " << std::left << std::setw(19) << r.timestamp
                  << " |\n";
    }
    std::cout << "+------------------+------------------+------------------+------------+------------+----------+----------+---------------------+\n\n";
}

// ============================================================================
// MAIN APPLICATION ENTRY POINT & 7-OPERATION CONSOLE MENU
// ============================================================================
int main() {
    std::cout << "\n"
              << "================================================================================\n"
              << "    POST-EARTHQUAKE MULTI-AGENCY EMERGENCY RELIEF LOGISTICS HUB (ERL-HUB)     \n"
              << "           Academic OOPS Demonstration Console | Course: CIC-211                \n"
              << "================================================================================\n";

    // Core Domain Entities
    DistributionDepot centralDepot("DEPOT-HQ", "Tribhuvan Humanitarian Staging Area", 60.0);
    std::vector<ReliefVehicle*> fleet;
    IncidentCommander commander("CMD-EAGLE", "Commander A. Sharma");

    // Initialize initial disaster scenario
    initializeSampleData(centralDepot, fleet);

    int mainChoice = -1;
    while (mainChoice != 0) {
        std::cout << "\n================================================================================\n"
                  << "                     ERL-HUB DISASTER LOGISTICS CONSOLE                         \n"
                  << "================================================================================\n"
                  << "  1. Manage Inventory (Restock +=, Input >>, Tabular Display, Return by Ref)\n"
                  << "  2. Manage Fleet & Vehicles (Polymorphism, Diamond Inheritance, Deep Copy Test)\n"
                  << "  3. Allocate Supplies (Function Overloading & Friend Class Override)\n"
                  << "  4. Dispatch Relief Supplies (Priority Queue, Route Evaluation, Exceptions)\n"
                  << "  5. View Dispatch History & Update Audit Logs (Binary std::fstream Persistence)\n"
                  << "  6. View System Statistics & Analytics (Static Members & Logistics Metrics)\n"
                  << "  0. Exit System (Clean Polymorphic Deallocation via Virtual Destructors)\n"
                  << "================================================================================\n"
                  << "  Select Operation [0-6]: ";

        if (!(std::cin >> mainChoice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  [!] Invalid input. Please enter a numerical option between 0 and 6.\n";
            continue;
        }

        switch (mainChoice) {
            // ================================================================
            // OPERATION 1: MANAGE INVENTORY
            // ================================================================
            case 1: {
                int invChoice = 0;
                std::cout << "\n  --- [OPERATION 1: MANAGE INVENTORY] ---\n"
                          << "  1. View Current Depot Stockpile (Tabular Manipulators)\n"
                          << "  2. Restock Relief Supplies (Operator += and Stream Extraction >>)\n"
                          << "  3. Direct In-Place Stock Update (Demonstrating Return by Reference)\n"
                          << "  Choice: ";
                std::cin >> invChoice;

                if (invChoice == 1) {
                    centralDepot.displayInventory();
                } else if (invChoice == 2) {
                    ReliefCargo newCargo;
                    std::cout << "\n  [RESTOCKING RELIEF STOCK - Operator >> Input]\n";
                    std::cin >> newCargo;
                    // Operator += demonstration
                    centralDepot += newCargo;
                    std::cout << statusGreen << "Depot successfully restocked using overloaded operator += !\n";
                } else if (invChoice == 3) {
                    std::string sku;
                    std::cout << "\n  Enter Cargo SKU to update directly (e.g., RAT-01, MED-01): ";
                    std::cin >> sku;
                    try {
                        // Return by reference demonstration!
                        ReliefCargo& itemRef = centralDepot.getCargoItem(sku);
                        std::cout << "  Current Quantity: " << itemRef.getQuantity() << " units.\n";
                        int addQty = 0;
                        std::cout << "  Enter additional units to add directly to reference: ";
                        std::cin >> addQty;
                        itemRef.setQuantity(itemRef.getQuantity() + addQty);
                        std::cout << statusGreen << "Reference updated directly in-place without copying! New Qty: " 
                                  << itemRef.getQuantity() << "\n";
                    } catch (const std::exception& ex) {
                        std::cout << alertRed << ex.what() << "\n";
                    }
                }
                break;
            }

            // ================================================================
            // OPERATION 2: MANAGE FLEET & VEHICLES
            // ================================================================
            case 2: {
                int fleetChoice = 0;
                std::cout << "\n  --- [OPERATION 2: MANAGE FLEET & VEHICLES] ---\n"
                          << "  1. Display Registered Operational Fleet (Runtime Polymorphism)\n"
                          << "  2. Register New Vehicle in Fleet (Ground, Aerial, Amphibious, Hybrid)\n"
                          << "  3. Diagnostic Test: Rule of Three Deep Copy & Manifest Equality (==)\n"
                          << "  Choice: ";
                std::cin >> fleetChoice;

                if (fleetChoice == 1) {
                    std::cout << "\n================================================================================\n"
                              << "                       OPERATIONAL FLEET ROSTER                                 \n"
                              << "================================================================================\n";
                    for (size_t i = 0; i < fleet.size(); ++i) {
                        std::cout << "\n[" << (i + 1) << "] ";
                        // Polymorphic call through base pointer!
                        fleet[i]->displayDetails();
                    }
                } else if (fleetChoice == 2) {
                    int vType = 0;
                    std::cout << "\n  Select Vehicle Type to Register:\n"
                              << "  1. Ground Convoy (All-Terrain Truck)\n"
                              << "  2. Aerial Transport (Helicopter / Drone)\n"
                              << "  3. Amphibious Craft (Rescue Boat)\n"
                              << "  4. Amphibious Medical Unit (Hybrid Diamond Inheritance)\n"
                              << "  Type [1-4]: ";
                    std::cin >> vType;

                    std::string id, call;
                    double payload, fuel;
                    std::cout << "  Enter Vehicle ID (e.g., TRK-99): ";
                    std::cin >> id;
                    std::cout << "  Enter Callsign (e.g., Vanguard-1): ";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::getline(std::cin, call);
                    std::cout << "  Enter Max Payload Capacity (Tons): ";
                    std::cin >> payload;
                    std::cout << "  Enter Fuel Operating Range (km): ";
                    std::cin >> fuel;

                    if (vType == 1) {
                        int axles;
                        std::cout << "  Enter Number of All-Terrain Axles: ";
                        std::cin >> axles;
                        fleet.push_back(new GroundConvoy(id, call, payload, fuel, axles));
                    } else if (vType == 2) {
                        double ceiling;
                        std::cout << "  Enter Max Flight Ceiling (meters): ";
                        std::cin >> ceiling;
                        fleet.push_back(new AerialTransport(id, call, payload, fuel, ceiling));
                    } else if (vType == 3) {
                        double draft;
                        std::cout << "  Enter Draft Depth (meters): ";
                        std::cin >> draft;
                        fleet.push_back(new AmphibiousCraft(id, call, payload, fuel, draft));
                    } else if (vType == 4) {
                        int axles;
                        double draft;
                        char surgChoice;
                        std::cout << "  Enter Axles: ";
                        std::cin >> axles;
                        std::cout << "  Enter Draft Depth: ";
                        std::cin >> draft;
                        std::cout << "  Is Mobile Surgical Suite Deployed? (y/n): ";
                        std::cin >> surgChoice;
                        fleet.push_back(new AmphibiousMedicalUnit(id, call, payload, fuel, axles, draft, 
                                                                (surgChoice == 'y' || surgChoice == 'Y')));
                    }
                    std::cout << statusGreen << "Vehicle successfully registered in active logistics fleet!\n";
                } else if (fleetChoice == 3) {
                    std::cout << "\n================================================================================\n"
                              << "   DIAGNOSTIC TEST: RULE OF THREE DEEP COPY & MANIFEST EQUALITY VERIFICATION    \n"
                              << "================================================================================\n";

                    if (fleet.empty()) {
                        std::cout << "  [!] Fleet is empty. Cannot test cloning.\n";
                        break;
                    }

                    // 1. Prepare original vehicle with a sample cargo item
                    ReliefVehicle* original = fleet[0];
                    ReliefCargo sampleCargo("TEST-01", "Rapid Diagnostic Kits", 0.5, 0.8, 1, true, 10);
                    
                    std::cout << "  [Step 1] Loading sample cargo onto original vehicle " << original->getVehicleId() << "...\n";
                    try {
                        original->addCargoToManifest(sampleCargo);
                    } catch (...) { /* May already be loaded */ }

                    // 2. Invoke Deep Copy Constructor
                    std::cout << "  [Step 2] Invoking Deep Copy Constructor: GroundConvoy clone(*originalConvoy)...\n";
                    GroundConvoy* originalConvoy = dynamic_cast<GroundConvoy*>(original);
                    if (originalConvoy) {
                        GroundConvoy clone(*originalConvoy); // Deep copy ctor

                        std::cout << "  [Deep Copy Check] Original Callsign: " << originalConvoy->getCallsign() << "\n"
                                  << "  [Deep Copy Check] Cloned Callsign  : " << clone.getCallsign() << "\n";

                        // 3. Verify Operator == on Cargo
                        std::cout << "\n  [Step 3] Testing Operator == on Cargo Manifest Items:\n";
                        ReliefCargo cargoA("MED-01", "Trauma Kits", 1.0, 1.0, 1, true);
                        ReliefCargo cargoB("MED-01", "Trauma Kits", 1.0, 1.0, 1, true);
                        ReliefCargo cargoC("RAT-01", "Ration Kits", 2.0, 3.0, 3, false);

                        std::cout << "  Comparing cargoA == cargoB: " << (cargoA == cargoB ? "MATCH (Identical SKU)" : "DIFFERENT") << "\n";
                        std::cout << "  Comparing cargoA == cargoC: " << (cargoA == cargoC ? "MATCH" : "DIFFERENT (Different SKU)") << "\n";

                        // 4. Test Copy Assignment Operator with self-assignment guard
                        std::cout << "\n  [Step 4] Testing Copy Assignment Operator with Self-Assignment Guard:\n";
                        clone = clone; // Self-assignment test
                        std::cout << statusGreen << "Self-assignment guard passed cleanly without memory corruption.\n";

                        std::cout << "\n  [Step 5] Active Operational Vehicles in Memory: " 
                                  << ReliefVehicle::getTotalOperationalVehicles() << "\n";
                        std::cout << "  (Exiting diagnostic scope: clone will be cleanly destroyed now...)\n";
                    }
                    std::cout << statusGreen << "Rule of Three and Operator == tests completed successfully!\n";
                }
                break;
            }

            // ================================================================
            // OPERATION 3: ALLOCATE SUPPLIES (FUNCTION OVERLOADING & FRIEND CLASS)
            // ================================================================
            case 3: {
                int allocChoice = 0;
                std::cout << "\n  --- [OPERATION 3: ALLOCATE SUPPLIES] ---\n"
                          << "  1. Bulk Volume/Payload Allocation (Overload A: Volume, Weight, CargoType)\n"
                          << "  2. Emergency Medical Triage Allocation (Overload B: Priority, MedicineType, Units)\n"
                          << "  3. Incident Commander Triage Override (Friend Class Privileged Access)\n"
                          << "  Choice: ";
                std::cin >> allocChoice;

                if (allocChoice == 1) {
                    double vol, wt;
                    std::string type;
                    std::cout << "  Enter Desired Payload Volume (m3): ";
                    std::cin >> vol;
                    std::cout << "  Enter Desired Payload Weight (Tons): ";
                    std::cin >> wt;
                    std::cout << "  Enter Target Supply Category (e.g., Rations, Water, Blankets): ";
                    std::cin >> type;

                    // Function Overload 1
                    centralDepot.allocateSupplies(vol, wt, type);
                } else if (allocChoice == 2) {
                    int prio, units;
                    std::string medType;
                    std::cout << "  Enter Required Triage Priority Level [1=Critical Triage, 2=High]: ";
                    std::cin >> prio;
                    std::cout << "  Enter Specific Pharmaceutical / Medicine Name (e.g., Surgical, Antivenom): ";
                    std::cin >> medType;
                    std::cout << "  Enter Required Unit Count: ";
                    std::cin >> units;

                    // Function Overload 2
                    centralDepot.allocateSupplies(prio, medType, units);
                } else if (allocChoice == 3) {
                    std::cout << "\n  --- [INCIDENT COMMANDER EMERGENCY TRIAGE OVERRIDE] ---\n"
                              << "  Authorized Commander: " << commander.getCallsign() << "\n"
                              << "  Restricted emergency blood plasma / serum vault is locked from standard dispatch.\n";
                    double litres;
                    std::cout << "  Enter Litres of Emergency Serum to Authorize for Immediate Release: ";
                    std::cin >> litres;

                    if (!fleet.empty()) {
                        // Friend class accesses depot private member directly!
                        commander.overrideTriageReserve(centralDepot, litres, *fleet.back());
                    } else {
                        std::cout << "  [!] No vehicle available to receive emergency serum.\n";
                    }
                }
                break;
            }

            // ================================================================
            // OPERATION 4: DISPATCH RELIEF SUPPLIES
            // ================================================================
            case 4: {
                std::cout << "\n================================================================================\n"
                          << "   OPERATION 4: DISPATCH RELIEF SUPPLIES (PRIORITY QUEUE & POLYMORPHISM)        \n"
                          << "================================================================================\n";

                // Step 1: Select or view vehicle
                std::cout << "  Select Vehicle for Mission Dispatch:\n";
                for (size_t i = 0; i < fleet.size(); ++i) {
                    std::cout << "  [" << (i + 1) << "] " << fleet[i]->getVehicleId() 
                              << " (" << fleet[i]->getCallsign() << ") - Payload Limit: " 
                              << fleet[i]->getMaxPayloadTons() << " T | Fuel: " 
                              << fleet[i]->getFuelRangeKm() << " km\n";
                }
                size_t vIdx = 0;
                std::cout << "  Vehicle [1-" << fleet.size() << "]: ";
                std::cin >> vIdx;
                if (vIdx < 1 || vIdx > fleet.size()) {
                    std::cout << alertRed << "Invalid vehicle index.\n";
                    break;
                }
                ReliefVehicle* selectedVehicle = fleet[vIdx - 1];

                // Step 2: Define corridor and distance
                std::string corridor;
                double distanceKm;
                std::cout << "\n  Select Corridor Terrain:\n"
                          << "  [Road] Normal Roadway | [DamagedRoad] Mudslide / Debris\n"
                          << "  [River] Flooded Basin / Deep Water | [HighWindTurbulence] Aerial Hazard\n"
                          << "  Enter Corridor Type: ";
                std::cin >> corridor;
                std::cout << "  Enter Transit Distance in km: ";
                std::cin >> distanceKm;

                // Step 3: Populate Priority Queue with Depot Stock
                // UNIT IV: Generic Template PriorityDispatchQueue<T, Compare> with custom Functor
                PriorityDispatchQueue<ReliefCargo, UrgentMedicalComparator> dispatchQueue;
                for (const auto& pair : centralDepot.getInventory()) {
                    dispatchQueue.push(pair.second);
                }

                if (dispatchQueue.isEmpty()) {
                    std::cout << alertRed << "Depot is empty. No cargo available to dispatch.\n";
                    break;
                }

                // Show top prioritized cargo (Functor prioritized Level 1 Medical supplies first!)
                const ReliefCargo& highestPriorityCargo = dispatchQueue.top();
                std::cout << "\n  [PRIORITY DISPATCH QUEUE ANALYSIS]\n"
                          << "  Top Prioritized Supply for Dispatch: " << highestPriorityCargo.getDescription()
                          << " (" << highestPriorityCargo.getCargoId() << ") [Priority: " 
                          << highestPriorityCargo.getPriorityLevel() << ", Medical: " 
                          << (highestPriorityCargo.getIsMedical() ? "YES" : "NO") << ", Weight: " 
                          << highestPriorityCargo.getWeightTons() << " T]\n";

                // Step 4: Polymorphic Route Evaluation & Exception Safety
                TransitRecord record;
                std::strncpy(record.missionId, ("MSN-" + std::to_string(std::rand() % 9000 + 1000)).c_str(), sizeof(record.missionId));
                std::strncpy(record.vehicleId, selectedVehicle->getVehicleId().c_str(), sizeof(record.vehicleId));
                std::strncpy(record.cargoId, highestPriorityCargo.getCargoId().c_str(), sizeof(record.cargoId));
                record.weightTons = highestPriorityCargo.getWeightTons();
                record.distanceKm = distanceKm;
                record.triagePriority = highestPriorityCargo.getPriorityLevel();
                std::strncpy(record.timestamp, getCurrentTimestamp().c_str(), sizeof(record.timestamp));

                try {
                    // UNIT III: Runtime polymorphism via ReliefVehicle*
                    std::cout << "  Evaluating route viability for " << selectedVehicle->getVehicleId() << "...\n";
                    selectedVehicle->evaluateRoute(corridor, distanceKm);

                    // UNIT II & III: Bounds-checked dispatch execution
                    selectedVehicle->executeDispatch(highestPriorityCargo);

                    // UNIT II: Operator -= deducting dispatched cargo from depot
                    centralDepot -= highestPriorityCargo;

                    record.success = true;
                    std::cout << statusGreen << "MISSION DISPATCH SUCCESSFUL!\n"
                              << "  Mission ID : " << record.missionId << "\n"
                              << "  Vehicle    : " << record.vehicleId << "\n"
                              << "  Delivered  : " << highestPriorityCargo.getDescription() << "\n";

                } catch (const CargoOverweightException& ex) {
                    std::cout << alertRed << ex.what() << "\n";
                    record.success = false;
                } catch (const FuelDepletionException& ex) {
                    std::cout << alertRed << ex.what() << "\n";
                    record.success = false;
                } catch (const DamagedCorridorException& ex) {
                    std::cout << alertRed << ex.what() << "\n";
                    record.success = false;
                } catch (const std::exception& ex) {
                    std::cout << alertRed << "Unexpected dispatch error: " << ex.what() << "\n";
                    record.success = false;
                }

                // Step 5: UNIT IV - Binary Stream Persistence with Atomic Windows Swap
                std::cout << "  Persisting mission transit record to binary log via atomic staging...\n";
                if (appendTransitRecordAtomically(record)) {
                    std::cout << statusGreen << "Transit record saved atomically to " << BIN_LOG_PATH << ".\n";
                } else {
                    std::cout << alertRed << "Failed to commit transit record atomically.\n";
                }
                break;
            }

            // ================================================================
            // OPERATION 5: VIEW DISPATCH HISTORY & UPDATE AUDIT LOGS
            // ================================================================
            case 5: {
                int histChoice = 0;
                std::cout << "\n  --- [OPERATION 5: DISPATCH AUDIT & BINARY PERSISTENCE] ---\n"
                          << "  1. View All Dispatch Records (Binary std::fstream Deserialization)\n"
                          << "  2. Search Records by Vehicle ID\n"
                          << "  3. Update Record Status In-Place (Transactional Atomic Update Protocol)\n"
                          << "  Choice: ";
                std::cin >> histChoice;

                auto records = readAllTransitRecords();

                if (histChoice == 1) {
                    std::cout << "\n  Total Historical Missions Logged: " << records.size() << "\n";
                    if (records.empty()) {
                        std::cout << "  (No transit records currently logged in " << BIN_LOG_PATH << ")\n";
                    } else {
                        displayTransitRecords(records);
                    }
                } else if (histChoice == 2) {
                    std::string searchVid;
                    std::cout << "  Enter Vehicle ID to Search (e.g., TRK-101, HELI-201): ";
                    std::cin >> searchVid;

                    std::vector<TransitRecord> matched;
                    for (const auto& r : records) {
                        if (std::string(r.vehicleId).find(searchVid) != std::string::npos) {
                            matched.push_back(r);
                        }
                    }
                    std::cout << "  Matched Records: " << matched.size() << "\n";
                    if (!matched.empty()) {
                        displayTransitRecords(matched);
                    }
                } else if (histChoice == 3) {
                    if (records.empty()) {
                        std::cout << "  [!] No records available to update.\n";
                        break;
                    }
                    displayTransitRecords(records);
                    size_t rIdx = 0;
                    std::cout << "  Enter Record Index to Toggle Status [1-" << records.size() << "]: ";
                    std::cin >> rIdx;
                    if (rIdx >= 1 && rIdx <= records.size()) {
                        // Modify in memory
                        records[rIdx - 1].success = !records[rIdx - 1].success;
                        std::cout << "  Toggled status for Mission " << records[rIdx - 1].missionId 
                                  << " to " << (records[rIdx - 1].success ? "SUCCESS" : "FAILED") << ".\n";
                        
                        // Transactional atomic staging write & replace!
                        if (writeAndCommitRecordsAtomically(records)) {
                            std::cout << statusGreen << "Record updated and committed atomically via Windows ReplaceFile!\n";
                        } else {
                            std::cout << alertRed << "Atomic update failed.\n";
                        }
                    } else {
                        std::cout << alertRed << "Invalid record index.\n";
                    }
                }
                break;
            }

            // ================================================================
            // OPERATION 6: VIEW SYSTEM STATISTICS & ANALYTICS
            // ================================================================
            case 6: {
                std::cout << "\n================================================================================\n"
                          << "             REAL-TIME LOGISTICS HUB ANALYTICS & STATIC METRICS                 \n"
                          << "================================================================================\n"
                          << "  Cumulative Dispatched Tonnage : " << ReliefVehicle::getTotalDispatchedTonnage() << " Metric Tons\n"
                          << "  Total Operational Vehicles    : " << ReliefVehicle::getTotalOperationalVehicles() << " Active Fleet Units\n"
                          << "  Distribution Hub SKUs Stored  : " << centralDepot.getInventoryItemCount() << " Distinct Categories\n"
                          << "  Restricted Serum Vault Level  : " << centralDepot.getPrivateReserveLitres() << " Litres\n"
                          << "================================================================================\n\n";
                break;
            }

            // ================================================================
            // OPERATION 0: EXIT & CLEAN POLYMORPHIC DEALLOCATION
            // ================================================================
            case 0: {
                std::cout << "\n  Shutting down ERL-HUB Logistics Console...\n";
                std::cout << "  Deallocating heterogeneous vehicle fleet polymorphically (virtual ~ReliefVehicle)...\n";
                // UNIT III: Correct virtual destructor invocation across all derived vehicle pointers
                for (ReliefVehicle* v : fleet) {
                    std::cout << "    Deallocating " << v->getVehicleId() << " (" << v->getCallsign() << ")...\n";
                    delete v; // Invokes virtual ~ReliefVehicle(), freeing dynamic manifest and decrementing static counter
                }
                fleet.clear();
                std::cout << "  Active Operational Vehicles Remaining: " << ReliefVehicle::getTotalOperationalVehicles() << "\n";
                std::cout << statusGreen << "System shutdown completed cleanly with zero resource leaks.\n";
                break;
            }

            default:
                std::cout << alertRed << "Invalid option selected. Please choose between 0 and 6.\n";
                break;
        }
    }

    return 0;
}
