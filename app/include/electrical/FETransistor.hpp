#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    // Field Effect Transistor
    class FETransistor final : public ElectronicComponent {
    public:
        enum class Property : ComponentProperty {
            ThresholdVoltage = static_cast<ComponentProperty>(ElectronicComponent::Property::End)
        };

        FETransistor(
            const BaseConfig& config,
            double thresholdVoltage
        )
            : ElectronicComponent(config, Type::Mosfet),
            m_thresholdVoltage(thresholdVoltage)
        {
            if (thresholdVoltage < 0)
                throw invalid_argument("Threshold voltage cannot be negative");
        }

        double thresholdVoltage() const { return m_thresholdVoltage; }

    private:
        double m_thresholdVoltage;
    };
}
