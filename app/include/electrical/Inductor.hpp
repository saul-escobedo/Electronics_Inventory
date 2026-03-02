#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    class Inductor final : public ElectronicComponent {
    public:
        enum class Property : ComponentProperty {
            Inductance = static_cast<ComponentProperty>(ElectronicComponent::Property::End),
        };

        Inductor(
            const BaseConfig& config,
            double inductance
        ) : ElectronicComponent(config, Type::Inductor),
            m_inductance(inductance)
        {
            if (inductance < 0)
                throw invalid_argument("Inductance cannot be negative");
        }

        double inductance() const { return m_inductance; }

    private:
        double m_inductance;
    };
}
