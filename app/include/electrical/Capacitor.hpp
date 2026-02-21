#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    class Capacitor final : public ElectronicComponent {
        public:
            Capacitor(
                const BaseConfig& config,
                const string& capacitorType,
                double capacitance
            )
                : ElectronicComponent(config, Type::Capacitor),
                m_capacitorType(capacitorType),
                m_capacitance(capacitance)
        {
            if (capacitance < 0)
                throw invalid_argument("Capacitance cannot be negative");
            if (capacitorType.empty())
                throw invalid_argument("Capacitor type cannot be empty");
        }

        const string& capacitorType() const { return m_capacitorType; }
        double capacitance() const { return m_capacitance; }

        private:
            string m_capacitorType; // e.g., "Ceramic", "Electrolytic", "Tantalum"
            double m_capacitance;
    };
}
