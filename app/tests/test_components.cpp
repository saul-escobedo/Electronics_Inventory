#include <iostream>
#include "electronic_components.hpp"

using namespace eip;

int main() {
    try {
        ElectronicRating rating{5.0, 0.01, 0.05, 0.05};

        Resistor r1("R1", "Vishay", "V-RES-1000", "1k resistor", rating, 1000.0, 0.05);
        std::cout << "Resistor: " << r1.getName() << " | resistance=" << r1.resistance()
                  << " ohm | tolerance=" << r1.toleranceBand() << "\n";

        Capacitor c1("C1", "Murata", "M-CAP-1uF", "1uF cap", rating, "Ceramic", 1e-6);
        std::cout << "Capacitor: " << c1.getName() << " | capacitance=" << c1.capacitance()
                  << " F | type=" << c1.capacitorType() << "\n";

        Inductor l1("L1", "CoilCo", "L-10uH", "10uH inductor", rating, 10e-6);
        std::cout << "Inductor: " << l1.getName() << " | inductance=" << l1.inductance() << " H\n";

        Diode d1("D1", "DiodeInc", "D-1N4148", "Signal diode", rating, 0.7, "Signal");
        std::cout << "Diode: " << d1.getName() << " | fwdV=" << d1.forwardVoltage()
                  << " V | type=" << d1.diodeType() << "\n";

        Transistor t1("T1", "TransCo", "T-NPN-1", "NPN transistor", rating, 100.0);
        std::cout << "Transistor: " << t1.getName() << " | gain=" << t1.gain() << "\n";

        IntegratedCircuit ic1("IC1", "ICMaker", "IC-555", "Timer IC", rating, 8, 5.0, 3.0, 1.5);
        std::cout << "IC: " << ic1.getName() << " | pins=" << ic1.pinCount()
                  << " | dims=" << ic1.width() << "x" << ic1.height() << "x" << ic1.length() << "\n";

        // Demonstrate validation by attempting an invalid component
        try {
            Resistor bad("", "Mfg", "PN", "bad", rating, -1.0, 0.05);
        } catch (const std::exception &ex) {
            std::cout << "Caught expected validation error: " << ex.what() << "\n";
        }

    } catch (const std::exception &ex) {
        std::cerr << "Unhandled exception in tests: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
