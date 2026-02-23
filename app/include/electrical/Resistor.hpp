#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    class Resistor final : public ElectronicComponent {
    public:
        enum class Property : ComponentProperty {
            Resistance = static_cast<ComponentProperty>(ElectronicComponent::Property::End),
            ToleranceBand
        };

        Resistor(
            const BaseConfig& config,
            double resistance,
            double toleranceBand
        ) : ElectronicComponent(config, Type::Resistor),
            m_resistance(resistance),
            m_toleranceBand(toleranceBand)
        {
            if (resistance < 0)
                throw invalid_argument("Resistance cannot be negative");
            if (toleranceBand < 0)
                throw invalid_argument("Tolerance band cannot be negative");
        }

        double resistance() const { return m_resistance; }
        double toleranceBand() const { return m_toleranceBand; }

    private:
            double m_resistance;
            double m_toleranceBand; // last band of the resistor color code, e.g., 5% = 0.05
    };
}
