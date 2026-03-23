#pragma once

#include "electrical/ElectronicComponents.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
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

    enum class DividerMode {
        TargetOutputVoltage,
        DropdownRatio,
    };

    struct DividerRequest {
        DividerMode mode = DividerMode::TargetOutputVoltage;
        double inputVoltage = 0.0;
        double targetOutputVoltage = 0.0; // Used when mode == TargetOutputVoltage
        double targetRatio = 0.0;         // Used when mode == DropdownRatio; expected in (0, 1)
        size_t maxResults = 10;
    };

    struct DividerCandidate {
        const Resistor* r1 = nullptr; // top resistor connected to input voltage
        const Resistor* r2 = nullptr; // bottom resistor connected to ground
        double outputVoltage = 0.0;
        double ratio = 0.0;
        double error = std::numeric_limits<double>::infinity();
    };

    inline double resolveDividerTargetRatio(const DividerRequest& request)
    {
        if (request.inputVoltage <= 0.0) {
            throw std::invalid_argument("Input voltage must be greater than zero");
        }

        if (request.mode == DividerMode::TargetOutputVoltage) {
            if (request.targetOutputVoltage < 0.0 || request.targetOutputVoltage > request.inputVoltage) {
                throw std::invalid_argument("Target output voltage must be between 0 and input voltage");
            }
            return request.targetOutputVoltage / request.inputVoltage;
        }

        if (request.targetRatio <= 0.0 || request.targetRatio >= 1.0) {
            throw std::invalid_argument("Dropdown ratio must be between 0 and 1");
        }
        return request.targetRatio;
    }

    inline vector<DividerCandidate> findBestVoltageDividerCombinations(
        const vector<const Resistor*>& resistors,
        const DividerRequest& request
    )
    {
        if (resistors.size() < 2) {
            throw std::invalid_argument("At least two resistors are required");
        }

        const double targetRatio = resolveDividerTargetRatio(request);
        vector<DividerCandidate> candidates;
        candidates.reserve(resistors.size() * (resistors.size() - 1));

        for (size_t i = 0; i < resistors.size(); ++i) {
            for (size_t j = 0; j < resistors.size(); ++j) {
                if (i == j) {
                    continue;
                }

                const Resistor* r1 = resistors[i];
                const Resistor* r2 = resistors[j];
                if (r1 == nullptr || r2 == nullptr) {
                    continue;
                }

                const double r1Value = r1->resistance();
                const double r2Value = r2->resistance();
                if (r1Value <= 0.0 || r2Value <= 0.0) {
                    continue;
                }

                const double outputVoltage = computeVoltageDivider(request.inputVoltage, r1Value, r2Value);
                const double ratio = r2Value / (r1Value + r2Value);
                const double error = std::abs(ratio - targetRatio);

                candidates.push_back(DividerCandidate{r1, r2, outputVoltage, ratio, error});
            }
        }

        std::sort(candidates.begin(), candidates.end(), [](const DividerCandidate& a, const DividerCandidate& b) {
            if (a.error != b.error) {
                return a.error < b.error;
            }

            const double aTotal = a.r1->resistance() + a.r2->resistance();
            const double bTotal = b.r1->resistance() + b.r2->resistance();
            if (aTotal != bTotal) {
                return aTotal < bTotal;
            }

            if (a.r1->resistance() != b.r1->resistance()) {
                return a.r1->resistance() < b.r1->resistance();
            }

            return a.r2->resistance() < b.r2->resistance();
        });

        if (request.maxResults > 0 && candidates.size() > request.maxResults) {
            candidates.resize(request.maxResults);
        }

        return candidates;
    }

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
