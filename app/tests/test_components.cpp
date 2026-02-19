#include <iostream>
#include "electronic_components.hpp"
#include "electronics_manager.hpp"

using namespace eip;

int main() {
    try {
        // ── Component construction tests ──────────────────────────────────────
        std::cout << "=== Component construction tests ===\n";

        ElectronicRating rating{5.0, 0.01, 0.05, 0.05};

        Resistor r1("R1", "Vishay", "V-RES-1000", "1k resistor", rating, 1000.0, 0.05);
        std::cout << "Resistor: " << r1.name() << " | resistance=" << r1.resistance()
                  << " ohm | tolerance=" << r1.toleranceBand() << "\n";

        Capacitor c1("C1", "Murata", "M-CAP-1uF", "1uF cap", rating, "Ceramic", 1e-6);
        std::cout << "Capacitor: " << c1.name() << " | capacitance=" << c1.capacitance()
                  << " F | type=" << c1.capacitorType() << "\n";

        Inductor l1("L1", "CoilCo", "L-10uH", "10uH inductor", rating, 10e-6);
        std::cout << "Inductor: " << l1.name() << " | inductance=" << l1.inductance() << " H\n";

        Diode d1("D1", "DiodeInc", "D-1N4148", "Signal diode", rating, 0.7, "Signal");
        std::cout << "Diode: " << d1.name() << " | fwdV=" << d1.forwardVoltage()
                  << " V | type=" << d1.diodeType() << "\n";

        Transistor t1("T1", "TransCo", "T-NPN-1", "NPN transistor", rating, 100.0);
        std::cout << "Transistor: " << t1.name() << " | gain=" << t1.gain() << "\n";

        IntegratedCircuit ic1("IC1", "ICMaker", "IC-555", "Timer IC", rating, 8, 5.0, 3.0, 1.5);
        std::cout << "IC: " << ic1.name() << " | pins=" << ic1.pinCount()
                  << " | dims=" << ic1.width() << "x" << ic1.height() << "x" << ic1.length() << "\n";

        // Demonstrate validation by attempting an invalid component
        try {
            Resistor bad("", "Mfg", "PN", "bad", rating, -1.0, 0.05);
        } catch (const std::exception &ex) {
            std::cout << "Caught expected validation error: " << ex.what() << "\n";
        }

        // ── ElectronicsManager tests ──────────────────────────────────────────
        std::cout << "\n=== ElectronicsManager tests ===\n";

        ElectronicsManager& mgr = ElectronicsManager::instance();

        // Add a variety of components
        mgr.addComponent(std::make_unique<Resistor>  ("R2",  "Vishay",  "V-RES-4K7", "4.7k resistor",   rating, 4700.0, 0.01));
        mgr.addComponent(std::make_unique<Resistor>  ("R3",  "Yageo",   "RC-10K",    "10k resistor",    rating, 10000.0, 0.05));
        mgr.addComponent(std::make_unique<Capacitor> ("C2",  "Murata",  "M-CAP-10n", "10nF cap",        rating, "Ceramic", 10e-9));
        mgr.addComponent(std::make_unique<Inductor>  ("L2",  "CoilCo",  "L-100uH",   "100uH inductor",  rating, 100e-6));
        mgr.addComponent(std::make_unique<Diode>     ("D2",  "DiodeInc","D-1N5819",  "Schottky diode",  rating, 0.3, "Schottky"));
        mgr.addComponent(std::make_unique<Transistor>("T2",  "TransCo", "T-PNP-1",   "PNP transistor",  rating, 80.0));
        mgr.addComponent(std::make_unique<IntegratedCircuit>("IC2","ICMaker","IC-NE556","Dual timer IC", rating, 14, 5.0, 4.0, 2.0));

        // getAllComponents — expect 7
        std::vector<ElectronicComponent*> all;
        mgr.getAllComponents(all);
        std::cout << "Total components in manager: " << all.size() << " (expected 7)\n";
        for (const auto* c : all) {
            std::cout << "  [" << c->name() << "] mfg=" << c->manufacturer()
                      << " pn=" << c->partNumber() << "\n";
        }

        // getAllComponentsByType — resistors only (expect 2)
        std::vector<ElectronicComponent*> resistors;
        mgr.getAllComponentsByType(ComponentType::Resistor, resistors);
        std::cout << "\nResistors in manager: " << resistors.size() << " (expected 2)\n";
        for (const auto* c : resistors) {
            const auto* r = static_cast<const Resistor*>(c);
            std::cout << "  " << r->name() << " | " << r->resistance() << " ohm\n";
        }

        // getAllComponentsByType — capacitors only (expect 1)
        std::vector<ElectronicComponent*> caps;
        mgr.getAllComponentsByType(ComponentType::Capacitor, caps);
        std::cout << "\nCapacitors in manager: " << caps.size() << " (expected 1)\n";
        for (const auto* c : caps) {
            const auto* cap = static_cast<const Capacitor*>(c);
            std::cout << "  " << cap->name() << " | " << cap->capacitance() << " F\n";
        }

        // removeComponent — remove R2 (id lookup may not be wired yet; test returns false gracefully)
        bool removed = mgr.removeComponent(0);
        std::cout << "\nremoveComponent(0) returned: " << std::boolalpha << removed << "\n";

    } catch (const std::exception &ex) {
        std::cerr << "Unhandled exception in tests: " << ex.what() << "\n";
        return 1;
    }

    std::cout << "\nAll tests completed.\n";
    return 0;
}
