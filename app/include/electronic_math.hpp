#pragma once

#include "electrical/ElectronicComponents.hpp"
#include <vector>
#include <stdexcept>

using std::vector;
using namespace ecim;

// NOTE: Components are non-copyable and non-movable, so all math functions
// accept vectors of const raw pointers rather than vectors of values.
namespace ecmath::internal {

    template<typename T, typename Getter>
    inline double sum(const vector<const T*>& elements, Getter getter)
    {
        double total = 0.0;
        for (const T* elem : elements) {
            total += getter(*elem);
        }
        return total;
    }

    template <typename T, typename Getter>
    inline double sumReciprocal(const vector<const T*>& items, Getter get)
    {
        double reciprocalTotal = 0.0;
        for (const T* item : items) {
            double value = get(*item);
            if (value == 0.0) {
                throw std::invalid_argument("Value cannot be zero in reciprocal calculation");
            }
            reciprocalTotal += 1.0 / value;
        }
        if (reciprocalTotal == 0.0) {
            throw std::invalid_argument("Total reciprocal cannot be zero");
        }
        return reciprocalTotal;
    }

} // namespace ecmath::internal

// electronic math
namespace ecmath {

    // Voltage divider: Vout = Vin * R2 / (R1 + R2)
    inline double computeVoltageDivider(double inputVoltage, double r1, double r2)
    {
        if (r1 + r2 == 0.0)
            throw std::invalid_argument("Total resistance cannot be zero in voltage divider");
        return inputVoltage * (r2 / (r1 + r2));
    }

    // Resistors in series: R_total = R1 + R2 + ...
    inline double computeSeriesResistance(const vector<const Resistor*>& resistors)
    {
        return internal::sum<Resistor>(resistors,
            [](const Resistor& r) { return r.resistance(); });
    }

    // Resistors in parallel: 1/R_total = 1/R1 + 1/R2 + ...
    inline double computeParallelResistance(const vector<const Resistor*>& resistors)
    {
        return 1.0 / internal::sumReciprocal<Resistor>(resistors,
            [](const Resistor& r) { return r.resistance(); });
    }

    // Capacitors in series: 1/C_total = 1/C1 + 1/C2 + ...
    inline double computeCapacitorsInSeries(const vector<const Capacitor*>& capacitors)
    {
        return 1.0 / internal::sumReciprocal<Capacitor>(capacitors,
            [](const Capacitor& c) { return c.capacitance(); });
    }

    // Capacitors in parallel: C_total = C1 + C2 + ...
    inline double computeCapacitorsInParallel(const vector<const Capacitor*>& capacitors)
    {
        return internal::sum<Capacitor>(capacitors,
            [](const Capacitor& c) { return c.capacitance(); });
    }

    // Inductors in series: L_total = L1 + L2 + ...
    inline double computeInductorsInSeries(const vector<const Inductor*>& inductors)
    {
        return internal::sum<Inductor>(inductors,
            [](const Inductor& l) { return l.inductance(); });
    }

    // Inductors in parallel: 1/L_total = 1/L1 + 1/L2 + ...
    inline double computeInductorsInParallel(const vector<const Inductor*>& inductors)
    {
        return 1.0 / internal::sumReciprocal<Inductor>(inductors,
            [](const Inductor& l) { return l.inductance(); });
    }

} // namespace ecmath
