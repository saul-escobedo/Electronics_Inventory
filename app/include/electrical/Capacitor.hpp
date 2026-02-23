#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    class Capacitor final : public ElectronicComponent {
    public:
        enum class Property : ComponentProperty {
            Capacitance = static_cast<ComponentProperty>(ElectronicComponent::Property::End),
            Type
        };

        enum class Type : size_t {
            AluminumPolymer,
            AluminumElectrolytic,
            ElectricDoubleLayer, // Supercapacitors
            Film,
            Mica,
            PTFE,
            NiobiumOxide,
            Silicon,
            Tantalum,
            ThinFilm,
            ACMotor,
            LithiumHybrid
        };

        Capacitor(
            const BaseConfig& config,
            Type capacitorType,
            double capacitance
        ) : ElectronicComponent(config, ElectronicComponent::Type::Capacitor),
            m_capacitorType(capacitorType),
            m_capacitance(capacitance)
        {
            if (capacitance < 0)
                throw invalid_argument("Capacitance cannot be negative");
        }

        Type capacitorType() const { return m_capacitorType; }
        double capacitance() const { return m_capacitance; }

    private:
        Type m_capacitorType;
        double m_capacitance;
    };
}
