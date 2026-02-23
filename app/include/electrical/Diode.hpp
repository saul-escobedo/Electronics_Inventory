#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    class Diode final : public ElectronicComponent {
    public:
        enum class Property : ComponentProperty {
            Type = static_cast<ComponentProperty>(ElectronicComponent::Property::End),
            ForwardVoltage
        };

        enum class Type : size_t {
            Regular,
            Schottky,
            Zener,
            LED
        };

        Diode(
            const BaseConfig& config,
            double forwardVoltage,
            Type diodeType
        )
            : ElectronicComponent(config, ElectronicComponent::Type::Diode),
            m_forwardVoltage(forwardVoltage),
            m_diodeType(diodeType)
        {
            if (forwardVoltage < 0)
                throw invalid_argument("Forward voltage cannot be negative");
        }

        Type diodeType() const { return m_diodeType; }
        double forwardVoltage() const { return m_forwardVoltage; }

    private:
        Type m_diodeType;
        double m_forwardVoltage;
    };
}
