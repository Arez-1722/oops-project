#include "../include/Common.hpp"
#include "../include/Cargo.hpp"
#include "../include/Vehicle.hpp"
#include "../include/Depot.hpp"
#include "../include/PriorityQueue.hpp"

#include <iostream>
#include <cassert>
#include <sstream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace ReliefOps::Logistics;
using namespace ReliefOps::Inventory;
namespace fs = std::filesystem;

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

void runUnit1Tests() {
    std::cout << "\n==================================================\n";
    std::cout << "  RUNNING UNIT I TESTS: CORE FOUNDATIONS & CALLS  \n";
    std::cout << "==================================================\n";

    // 1. Namespaces Check
    ReliefOps::Inventory::ReliefCargo cargo("C-01", "Blankets", 1.0, 2.0, 3, false, 50);
    ReliefOps::Logistics::DistributionDepot depot("DEPOT-1", "Base Alpha", 100.0);
    depot += cargo;
    std::cout << "  [PASS] Logical namespaces ReliefOps::Inventory and ReliefOps::Logistics accessible.\n";

    // 2. Function Overloading Check
    // Overload 1: Volume/Weight allocation
    bool allocVol = depot.allocateSupplies(1.5, 0.8, "Blankets");
    assert(allocVol == true);
    std::cout << "  [PASS] Function Overload 1 (Volume-based payload) succeeded.\n";

    // Overload 2: Emergency triage medical allocation
    ReliefOps::Inventory::ReliefCargo medCargo("MED-99", "Emergency Antibiotics", 0.2, 0.4, 1, true, 30);
    depot += medCargo;
    bool allocMed = depot.allocateSupplies(1, "Antibiotics", 10);
    assert(allocMed == true);
    std::cout << "  [PASS] Function Overload 2 (Priority medical triage) succeeded.\n";

    // 3. Call by Reference & Return by Reference Check
    ReliefCargo& itemRef = depot.getCargoItem("C-01"); // Return by reference
    assert(itemRef.getQuantity() == 50);
    itemRef.setQuantity(75); // Modify via reference
    assert(depot.getCargoItem("C-01").getQuantity() == 75); // Verified in-place update
    std::cout << "  [PASS] Return by Reference: in-place modification verified.\n";

    // 4. Inline Utilities Check
    double dist = calculateDistance(0.0, 0.0, 3.0, 4.0);
    assert(std::abs(dist - 5.0) < 0.001);
    double ratio = computePayloadRatio(5.0, 10.0);
    assert(std::abs(ratio - 0.5) < 0.001);
    double fuel = estimateFuelRequirement(100.0, 0.25);
    assert(std::abs(fuel - 25.0) < 0.001);
    std::cout << "  [PASS] Inline utility methods (calculateDistance, computePayloadRatio, estimateFuelRequirement) verified.\n";

    // 5. Friend Class IncidentCommander Check
    GroundConvoy truck("TRK-T1", "Alpha Convoy", 5.0, 300.0, 2);
    IncidentCommander commander("CMD-99", "General Stone");
    assert(depot.getPrivateReserveLitres() == 100.0);
    bool overrideResult = commander.overrideTriageReserve(depot, 20.0, truck);
    assert(overrideResult == true);
    assert(depot.getPrivateReserveLitres() == 80.0);
    std::cout << "  [PASS] Friend class IncidentCommander privileged access to private reserves verified.\n";
}

void runUnit2Tests() {
    std::cout << "\n==================================================\n";
    std::cout << "  RUNNING UNIT II TESTS: ENCAPSULATION & DYNAMIC MEMORY \n";
    std::cout << "==================================================\n";

    int initialVehicles = ReliefVehicle::getTotalOperationalVehicles();

    // 1. ADT & Encapsulation
    ReliefCargo cargoA("BOX-1", "Water Cans", 2.0, 3.0, 2, false, 10);
    assert(cargoA.getCargoId() == "BOX-1");
    assert(cargoA.getWeightTons() == 2.0);
    std::cout << "  [PASS] Abstract Data Type & Encapsulation verified.\n";

    // 2. Dynamic Memory & Rule of Three (Copy Constructor & Destructor)
    {
        GroundConvoy original("GC-01", "Original Convoy", 10.0, 500.0, 3, 5);
        assert(ReliefVehicle::getTotalOperationalVehicles() == initialVehicles + 1);

        original.addCargoToManifest(cargoA);
        assert(original.getManifestCount() == 1);

        // Deep copy constructor invocation
        GroundConvoy cloned(original);
        assert(ReliefVehicle::getTotalOperationalVehicles() == initialVehicles + 2);
        assert(cloned.getManifestCount() == 1);

        // Verify independent manifest arrays by modifying original
        ReliefCargo cargoB("BOX-2", "Ration Packs", 1.0, 1.5, 3, false, 5);
        original.addCargoToManifest(cargoB);
        assert(original.getManifestCount() == 2);
        assert(cloned.getManifestCount() == 1); // Cloned manifest must remain unaffected!

        // Copy Assignment Operator
        GroundConvoy assigned("GC-02", "Assigned Convoy", 8.0, 400.0, 2, 5);
        assigned = original; // Copy assignment
        assert(assigned.getManifestCount() == 2);

        // Self-assignment guard test
        assigned = assigned;
        assert(assigned.getManifestCount() == 2);

        std::cout << "  [PASS] Rule of Three (Dynamic ctor, deep copy ctor, copy assignment, explicit dtor) verified.\n";
    }
    // All local vehicles destroyed; count must return to initialVehicles
    assert(ReliefVehicle::getTotalOperationalVehicles() == initialVehicles);
    std::cout << "  [PASS] Static vehicle counter totalOperationalVehicles verified during construction & destruction.\n";

    // 3. Operator Overloading: += and -= on DistributionDepot
    DistributionDepot testDepot("DEP-TEST", "Test Location", 50.0);
    ReliefCargo c1("SKU-100", "Tents", 1.0, 2.0, 3, false, 20);
    testDepot += c1; // +=
    assert(testDepot.hasCargo("SKU-100"));
    assert(testDepot.getCargoItem("SKU-100").getQuantity() == 20);

    ReliefCargo dispatchCargo("SKU-100", "Tents", 0.5, 1.0, 3, false, 5);
    testDepot -= dispatchCargo; // -=
    assert(testDepot.getCargoItem("SKU-100").getQuantity() == 15);
    std::cout << "  [PASS] Operator += and Operator -= on DistributionDepot verified.\n";

    // 4. Operator Overloading: ==, <<, >> on ReliefCargo
    ReliefCargo sameA("EQ-01", "Blankets", 1.0, 1.0, 3, false);
    ReliefCargo sameB("EQ-01", "Blankets", 1.0, 1.0, 3, false);
    ReliefCargo diffC("EQ-02", "Blankets", 1.0, 1.0, 3, false);
    assert(sameA == sameB);
    assert(!(sameA == diffC));
    std::cout << "  [PASS] Operator == (Manifest / Cargo identity check) verified.\n";

    // Stream << check
    std::ostringstream oss;
    oss << sameA;
    assert(!oss.str().empty());
    std::cout << "  [PASS] Operator << (Stream insertion) verified.\n";

    // Stream >> check via sstream input
    std::istringstream iss("TEST-SKU\nTest Description\n2.5\n3.5\n2\ny\n15\n");
    ReliefCargo extracted;
    iss >> extracted;
    assert(extracted.getCargoId() == "TEST-SKU");
    assert(extracted.getIsMedical() == true);
    assert(extracted.getQuantity() == 15);
    std::cout << "  [PASS] Operator >> (Stream extraction) verified.\n";
}

void runUnit3Tests() {
    std::cout << "\n==================================================\n";
    std::cout << "  RUNNING UNIT III TESTS: INHERITANCE & POLYMORPHISM \n";
    std::cout << "==================================================\n";

    // 1. Vehicle Hierarchy & Virtual Diamond Inheritance
    // Base pointers holding derived types
    std::vector<ReliefVehicle*> fleet;
    fleet.push_back(new GroundConvoy("GC-10", "Ground-10", 6.0, 400.0, 3));
    fleet.push_back(new AerialTransport("AT-20", "Air-20", 3.0, 500.0, 4500.0));
    fleet.push_back(new AmphibiousCraft("AC-30", "Aqua-30", 2.5, 200.0, 0.9));
    
    // Most-derived hybrid resolving diamond problem
    fleet.push_back(new AmphibiousMedicalUnit("HYB-40", "HybridMed-40", 5.0, 350.0, 4, 1.1, true));

    std::cout << "  [PASS] Heterogeneous fleet created with virtual diamond inheritance (AmphibiousMedicalUnit).\n";

    // 2. Runtime Polymorphism via base pointers (ReliefVehicle*)
    for (ReliefVehicle* v : fleet) {
        // Evaluate route viability polymorphically
        if (v->getVehicleId() == "GC-10") {
            assert(v->evaluateRoute("DamagedRoad", 100.0) == true);
        } else if (v->getVehicleId() == "AC-30") {
            assert(v->evaluateRoute("FloodedBasin", 50.0) == true);
        } else if (v->getVehicleId() == "HYB-40") {
            // Hybrid can traverse both DamagedRoad and FloodedBasin!
            assert(v->evaluateRoute("DamagedRoad", 50.0) == true);
            assert(v->evaluateRoute("FloodedBasin", 50.0) == true);
        }
    }
    std::cout << "  [PASS] Runtime polymorphism (evaluateRoute via ReliefVehicle*) verified.\n";

    // 3. Virtual Destructor Clean Invocation
    int beforeCount = ReliefVehicle::getTotalOperationalVehicles();
    for (ReliefVehicle* v : fleet) {
        delete v; // Polymorphic delete invoking virtual ~ReliefVehicle()
    }
    fleet.clear();
    int afterCount = ReliefVehicle::getTotalOperationalVehicles();
    assert(afterCount == beforeCount - 4);
    std::cout << "  [PASS] Virtual destructor correctly decremented operational vehicle count upon base pointer deletion.\n";
}

void runUnit4Tests() {
    std::cout << "\n==================================================\n";
    std::cout << "  RUNNING UNIT IV TESTS: PERSISTENCE, STL & QUEUE \n";
    std::cout << "==================================================\n";

    // 1. Custom Exceptions Check
    GroundConvoy truck("EX-01", "TestTruck", 2.0, 100.0, 2, 2);

    // Test CargoOverweightException
    bool caughtOverweight = false;
    try {
        ReliefCargo heavy("HVY-01", "Heavy Steel Bridge Parts", 5.0, 2.0, 3, false);
        truck.addCargoToManifest(heavy);
    } catch (const CargoOverweightException& ex) {
        caughtOverweight = true;
    }
    assert(caughtOverweight == true);
    std::cout << "  [PASS] CargoOverweightException thrown and caught on excessive payload.\n";

    // Test FuelDepletionException
    bool caughtFuel = false;
    try {
        truck.evaluateRoute("Road", 250.0); // Range is only 100.0 km
    } catch (const FuelDepletionException& ex) {
        caughtFuel = true;
    }
    assert(caughtFuel == true);
    std::cout << "  [PASS] FuelDepletionException thrown and caught on excessive transit corridor.\n";

    // Test DamagedCorridorException
    bool caughtCorridor = false;
    try {
        truck.evaluateRoute("River", 50.0); // Ground convoy cannot traverse river
    } catch (const DamagedCorridorException& ex) {
        caughtCorridor = true;
    }
    assert(caughtCorridor == true);
    std::cout << "  [PASS] DamagedCorridorException thrown and caught on impassable corridor.\n";

    // 2. Generic Template PriorityDispatchQueue with Custom Functor UrgentMedicalComparator
    PriorityDispatchQueue<ReliefCargo, UrgentMedicalComparator> queue;
    ReliefCargo cNormal("RAT-01", "Standard Rations", 1.0, 2.0, 3, false);
    ReliefCargo cHighNonMed("WAT-01", "Clean Water Tank", 2.0, 3.0, 2, false);
    ReliefCargo cCriticalMed("MED-01", "Trauma Blood Units", 0.5, 0.5, 1, true);

    // Push in mixed order
    queue.push(cNormal);
    queue.push(cHighNonMed);
    queue.push(cCriticalMed);

    assert(queue.size() == 3);
    // Highest emergency triage urgency (Priority 1 Medical) must be at the top!
    assert(queue.top().getCargoId() == "MED-01");
    queue.pop();

    // Next is Priority 2
    assert(queue.top().getCargoId() == "WAT-01");
    queue.pop();

    // Last is Priority 3
    assert(queue.top().getCargoId() == "RAT-01");
    queue.pop();

    assert(queue.isEmpty() == true);
    std::cout << "  [PASS] Generic PriorityDispatchQueue<T> and UrgentMedicalComparator functor verified.\n";

    // 3. Binary Stream Persistence with Verification Gates & Windows Atomic Replacement
    fs::create_directories("data");
    std::string testBin = "data/test_transit.bin";
    std::string testTmp = "data/test_transit.tmp";

    TransitRecord r1;
    std::strcpy(r1.missionId, "MSN-101");
    std::strcpy(r1.vehicleId, "TRK-01");
    std::strcpy(r1.cargoId, "MED-01");
    r1.weightTons = 1.5;
    r1.distanceKm = 45.0;
    r1.triagePriority = 1;
    r1.success = true;
    std::strcpy(r1.timestamp, "2026-11-15 12:00:00");

    // Write to staging tmp file using std::fstream in binary mode
    std::fstream tmpOut(testTmp, std::ios::out | std::ios::trunc | std::ios::binary);
    assert(tmpOut.is_open());
    tmpOut.write(reinterpret_cast<const char*>(&r1), sizeof(TransitRecord));
    assert(!tmpOut.fail() && !tmpOut.bad());
    tmpOut.flush();
    assert(!tmpOut.fail() && !tmpOut.bad());
    tmpOut.close();
    assert(!tmpOut.fail());

    // Verification Gate: size check
    auto sz = fs::file_size(testTmp);
    assert(sz == sizeof(TransitRecord));
    assert(sz % sizeof(TransitRecord) == 0);

    // Atomic replacement test
#ifdef _WIN32
    if (!fs::exists(testBin)) {
        MoveFileExA(testTmp.c_str(), testBin.c_str(), MOVEFILE_WRITE_THROUGH);
    } else {
        ReplaceFileA(testBin.c_str(), testTmp.c_str(), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL);
    }
#else
    std::rename(testTmp.c_str(), testBin.c_str());
#endif

    assert(fs::exists(testBin));

    // Read back using binary std::fstream
    std::fstream binIn(testBin, std::ios::in | std::ios::binary);
    assert(binIn.is_open());
    TransitRecord readRec;
    binIn.read(reinterpret_cast<char*>(&readRec), sizeof(TransitRecord));
    binIn.close();

    assert(std::string(readRec.missionId) == "MSN-101");
    assert(std::string(readRec.cargoId) == "MED-01");
    assert(readRec.weightTons == 1.5);
    assert(readRec.success == true);

    // Clean up test file
    fs::remove(testBin);
    std::cout << "  [PASS] Binary std::fstream persistence and atomic Windows swap verified.\n";
}

int main() {
    std::cout << "\n====================================================================\n";
    std::cout << "   ERL-HUB COMPREHENSIVE AUTOMATED VERIFICATION TEST HARNESS        \n";
    std::cout << "====================================================================\n";

    runUnit1Tests();
    runUnit2Tests();
    runUnit3Tests();
    runUnit4Tests();

    std::cout << "\n====================================================================\n";
    std::cout << "  ALL UNIT I-IV REQUIREMENTS SUCCESSFULLY VERIFIED WITH ZERO ERRORS \n";
    std::cout << "====================================================================\n\n";
    return 0;
}
