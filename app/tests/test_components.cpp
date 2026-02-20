#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <stdexcept>
#include "electronic_components.hpp"
#include "electronics_manager.hpp"
#include "electronic_math.hpp"

using namespace eip;

// ── Test helpers ─────────────────────────────────────────────────────────────

static int g_failures = 0;

static void section(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

static bool check(const char* label, bool condition) {
    if (condition) {
        std::cout << "  [PASS] " << label << "\n";
    } else {
        std::cout << "  [FAIL] " << label << "\n";
        ++g_failures;
    }
    return condition;
}

static bool checkVal(const char* label, double got, double expected, double tol = 1e-9) {
    bool ok = std::fabs(got - expected) <= tol;
    std::cout << std::fixed << std::setprecision(10)
              << (ok ? "  [PASS] " : "  [FAIL] ")
              << label << ": got=" << got << "  expected=" << expected << "\n";
    if (!ok) ++g_failures;
    return ok;
}

// ── Helper: build a ComponentBaseConfig quickly ───────────────────────────────
static ComponentBaseConfig cfg(
    const string& name,
    const string& mfg, const string& pn, const string& desc,
    const ElectronicRating& rating, size_t qty = 1)
{
    return ComponentBaseConfig{rating, name, mfg, pn, desc, qty};
}

int main() {
    try {
        ElectronicRating rating{5.0, 0.01, 0.05, 0.05};

        // ── Component construction ────────────────────────────────────────────
        section("Component construction");

        Resistor r1(cfg("R1", "Vishay", "V-RES-1K", "1k resistor", rating), 1000.0, 0.05);
        check("Resistor name",        r1.name()        == "R1");
        check("Resistor manufacturer",r1.manufacturer()== "Vishay");
        check("Resistor partNumber",  r1.partNumber()  == "V-RES-1K");
        check("Resistor quantity",    r1.quantity()    == 1);
        checkVal("Resistor resistance", r1.resistance(), 1000.0);
        checkVal("Resistor tolerance",  r1.toleranceBand(), 0.05);

        Capacitor c1(cfg("C1", "Murata", "M-CAP-1uF", "1uF cap", rating), "Ceramic", 1e-6);
        check("Capacitor name",       c1.name()         == "C1");
        check("Capacitor type",       c1.capacitorType()== "Ceramic");
        checkVal("Capacitor capacitance", c1.capacitance(), 1e-6);

        Inductor l1(cfg("L1", "CoilCo", "L-10uH", "10uH inductor", rating), 10e-6);
        check("Inductor name",        l1.name() == "L1");
        checkVal("Inductor inductance", l1.inductance(), 10e-6);

        Diode d1(cfg("D1", "DiodeInc", "D-1N4148", "Signal diode", rating), 0.7, "Signal");
        check("Diode name",           d1.name()         == "D1");
        check("Diode type",           d1.diodeType()    == "Signal");
        checkVal("Diode forward voltage", d1.forwardVoltage(), 0.7);

        Transistor t1(cfg("T1", "TransCo", "T-NPN-1", "NPN transistor", rating), 100.0);
        check("Transistor name",      t1.name() == "T1");
        checkVal("Transistor gain",   t1.gain(), 100.0);

        Mosfet m1(cfg("M1", "MosCo", "M-NMOS-1", "N-MOSFET", rating), 2.5);
        check("Mosfet name",          m1.name() == "M1");
        checkVal("Mosfet threshold",  m1.thresholdVoltage(), 2.5);

        IntegratedCircuit ic1(cfg("IC1", "ICMaker", "IC-555", "Timer IC", rating), 8, 5.0, 3.0, 1.5);
        check("IC name",              ic1.name()     == "IC1");
        check("IC pin count",         ic1.pinCount() == 8);
        checkVal("IC width",          ic1.width(),  5.0);
        checkVal("IC height",         ic1.height(), 3.0);
        checkVal("IC length",         ic1.length(), 1.5);

        // ── Quantity management ───────────────────────────────────────────────
        section("Quantity management");

        Resistor qr(cfg("QR", "Test", "T-1", "", rating, 10), 100.0, 0.01);
        check("Initial quantity=10",  qr.quantity() == 10);
        qr.addQuantity(5);
        check("After addQuantity(5)=15", qr.quantity() == 15);
        qr.removeQuantity(3);
        check("After removeQuantity(3)=12", qr.quantity() == 12);

        try {
            qr.removeQuantity(999);
            check("Over-remove should throw", false);
        } catch (const std::exception&) {
            check("Over-remove throws as expected", true);
        }

        // ── Validation errors ─────────────────────────────────────────────────
        section("Validation errors");

        auto tryBadRating = [&]() {
            ElectronicRating bad{-1.0, 0.01, 0.05, 0.05};
            Resistor r(cfg("X", "M", "P", "", bad), 100.0, 0.01);
        };
        try { tryBadRating(); check("Negative voltage rating should throw", false); }
        catch (const std::exception&) { check("Negative voltage rating throws", true); }

        auto tryEmptyName = [&]() {
            Resistor r(cfg("", "M", "P", "", rating), 100.0, 0.01);
        };
        try { tryEmptyName(); check("Empty name should throw", false); }
        catch (const std::exception&) { check("Empty name throws", true); }

        auto tryNegResistance = [&]() {
            Resistor r(cfg("X", "M", "P", "", rating), -1.0, 0.01);
        };
        try { tryNegResistance(); check("Negative resistance should throw", false); }
        catch (const std::exception&) { check("Negative resistance throws", true); }

        auto tryNegCapacitance = [&]() {
            Capacitor c(cfg("X", "M", "P", "", rating), "Ceramic", -1e-6);
        };
        try { tryNegCapacitance(); check("Negative capacitance should throw", false); }
        catch (const std::exception&) { check("Negative capacitance throws", true); }

        auto tryZeroPins = [&]() {
            IntegratedCircuit ic(cfg("X", "ICMaker", "IC-555", "Timer IC", rating), 0, 1.0, 1.0, 1.0);
        };
        try { tryZeroPins(); check("Zero pin count should throw", false); }
        catch (const std::exception&) { check("Zero pin count throws", true); }

        // ── ElectronicsManager ────────────────────────────────────────────────
        section("ElectronicsManager — add & retrieve");

        ElectronicsManager& mgr = ElectronicsManager::instance();

        // Add one of every type
        mgr.addComponent(std::make_unique<Resistor>  (cfg("R2",  "Vishay",   "V-RES-4K7",  "4.7k resistor",   rating, 5), 4700.0,  0.01));
        mgr.addComponent(std::make_unique<Resistor>  (cfg("R3",  "Yageo",    "RC-10K",     "10k resistor",    rating, 3), 10000.0, 0.05));
        mgr.addComponent(std::make_unique<Resistor>  (cfg("R4",  "KOA",      "RK-47K",     "47k resistor",    rating, 2), 47000.0, 0.05));
        mgr.addComponent(std::make_unique<Capacitor> (cfg("C2",  "Murata",   "M-CAP-10n",  "10nF cap",        rating, 10), "Ceramic",      10e-9));
        mgr.addComponent(std::make_unique<Capacitor> (cfg("C3",  "KEMET",    "K-CAP-100u", "100uF electro",   rating, 4), "Electrolytic", 100e-6));
        mgr.addComponent(std::make_unique<Inductor>  (cfg("L2",  "CoilCo",   "L-100uH",    "100uH inductor",  rating, 2), 100e-6));
        mgr.addComponent(std::make_unique<Inductor>  (cfg("L3",  "CoilCo",   "L-1mH",      "1mH inductor",    rating, 1), 1e-3));
        mgr.addComponent(std::make_unique<Diode>     (cfg("D2",  "DiodeInc", "D-1N5819",   "Schottky diode",  rating, 6), 0.3,  "Schottky"));
        mgr.addComponent(std::make_unique<Transistor>(cfg("T2",  "TransCo",  "T-PNP-1",    "PNP transistor",  rating, 3), 80.0));
        mgr.addComponent(std::make_unique<Mosfet>    (cfg("M2",  "MosCo",    "M-IRF540",   "Power N-MOSFET",  rating, 2), 4.0));
        mgr.addComponent(std::make_unique<IntegratedCircuit>(cfg("IC2", "ICMaker", "IC-NE556", "Dual timer IC", rating, 1), 14, 5.0, 4.0, 2.0));

        std::vector<ElectronicComponent*> all;
        mgr.getAllComponents(all);
        check("getAllComponents returns 11", all.size() == 11);

        std::vector<ElectronicComponent*> mgr_resistors;
        mgr.getAllComponentsByType(ComponentType::Resistor, mgr_resistors);
        check("3 resistors in manager", mgr_resistors.size() == 3);

        std::vector<ElectronicComponent*> mgr_caps;
        mgr.getAllComponentsByType(ComponentType::Capacitor, mgr_caps);
        check("2 capacitors in manager", mgr_caps.size() == 2);

        std::vector<ElectronicComponent*> mgr_inductors;
        mgr.getAllComponentsByType(ComponentType::Inductor, mgr_inductors);
        check("2 inductors in manager", mgr_inductors.size() == 2);

        std::vector<ElectronicComponent*> mgr_mosfets;
        mgr.getAllComponentsByType(ComponentType::Mosfet, mgr_mosfets);
        check("1 mosfet in manager", mgr_mosfets.size() == 1);

        // ── ElectronicsManager stress test ───────────────────────────────────
        section("ElectronicsManager — stress test (500 components)");

        for (int i = 0; i < 500; ++i) {
            double res = 100.0 * (i + 1);
            string name = "SR" + std::to_string(i);
            mgr.addComponent(std::make_unique<Resistor>(
                cfg(name, "StressMfg", "S-RES", "stress", rating), res, 0.01));
        }

        std::vector<ElectronicComponent*> stress_res;
        mgr.getAllComponentsByType(ComponentType::Resistor, stress_res);
        check("503 resistors after stress add (3 + 500)", stress_res.size() == 503);

        std::vector<ElectronicComponent*> all_after;
        mgr.getAllComponents(all_after);
        check("514 total components after stress (11 + 500 resistors - 0 removed)", all_after.size() == 511);

        // removeComponent with invalid id — expect false
        bool removed = mgr.removeComponent(99999);
        check("removeComponent(invalid id) returns false", removed == false);

        // ── electronic_math calculations ──────────────────────────────────────
        section("ecmath — voltage divider");

        // Vout = 12 * 2000 / (1000 + 2000) = 8.0 V
        checkVal("Voltage divider 12V, 1k/2k → 8V",
            ecmath::computeVoltageDivider(12.0, 1000.0, 2000.0), 8.0);
        // Equal resistors → half voltage
        checkVal("Voltage divider 5V, equal → 2.5V",
            ecmath::computeVoltageDivider(5.0, 500.0, 500.0), 2.5);

        try {
            ecmath::computeVoltageDivider(5.0, 0.0, 0.0);
            check("Zero-resistance divider should throw", false);
        } catch (const std::exception&) {
            check("Zero-resistance divider throws", true);
        }

        section("ecmath — series resistance");

        // R2=4700, R3=10000, R4=47000 → 61700
        {
            vector<const Resistor*> rp;
            for (auto* c : mgr_resistors) rp.push_back(static_cast<const Resistor*>(c));
            checkVal("Series R2+R3+R4 = 61700 ohm",
                ecmath::computeSeriesResistance(rp), 61700.0);
        }

        // Two known resistors
        Resistor ra(cfg("RA", "T", "T", "", rating), 300.0, 0.01);
        Resistor rb(cfg("RB", "T", "T", "", rating), 700.0, 0.01);
        {
            vector<const Resistor*> rp = {&ra, &rb};
            checkVal("Series 300+700 = 1000 ohm",
                ecmath::computeSeriesResistance(rp), 1000.0);
        }

        section("ecmath — parallel resistance");

        {
            // 1/(1/300 + 1/700) = 1/(7/2100 + 3/2100) = 2100/10 = 210
            vector<const Resistor*> rp = {&ra, &rb};
            checkVal("Parallel 300‖700 = 210 ohm",
                ecmath::computeParallelResistance(rp), 210.0);
        }
        {
            // Two equal 1k → 500
            Resistor re1(cfg("RE1", "T", "T", "", rating), 1000.0, 0.01);
            Resistor re2(cfg("RE2", "T", "T", "", rating), 1000.0, 0.01);
            vector<const Resistor*> rp = {&re1, &re2};
            checkVal("Parallel 1k‖1k = 500 ohm",
                ecmath::computeParallelResistance(rp), 500.0);
        }

        section("ecmath — capacitors");

        Capacitor ca(cfg("CA", "T", "T", "", rating), "Ceramic", 10e-6);
        Capacitor cb(cfg("CB", "T", "T", "", rating), "Ceramic", 40e-6);
        {
            vector<const Capacitor*> cp = {&ca, &cb};
            // Parallel: 10u + 40u = 50u
            checkVal("Capacitors parallel 10u+40u = 50uF",
                ecmath::computeCapacitorsInParallel(cp), 50e-6, 1e-18);
            // Series: 1/(1/10u + 1/40u) = 1/(5/40u) = 8uF
            checkVal("Capacitors series 10u‖40u = 8uF",
                ecmath::computeCapacitorsInSeries(cp), 8e-6, 1e-18);
        }

        section("ecmath — inductors");

        Inductor la(cfg("LA", "T", "T", "", rating), 200e-6);
        Inductor lb(cfg("LB", "T", "T", "", rating), 300e-6);
        {
            vector<const Inductor*> lp = {&la, &lb};
            // Series: 200u + 300u = 500u
            checkVal("Inductors series 200u+300u = 500uH",
                ecmath::computeInductorsInSeries(lp), 500e-6, 1e-18);
            // Parallel: 1/(1/200u + 1/300u) = 120uH
            checkVal("Inductors parallel 200u‖300u = 120uH",
                ecmath::computeInductorsInParallel(lp), 120e-6, 1e-18);
        }

    } catch (const std::exception &ex) {
        std::cerr << "Unhandled exception in tests: " << ex.what() << "\n";
        return 1;
    }

    std::cout << "\n────────────────────────────────────────\n";
    if (g_failures == 0) {
        std::cout << "All tests PASSED.\n";
    } else {
        std::cout << g_failures << " test(s) FAILED.\n";
    }

    return g_failures == 0 ? 0 : 1;
}
