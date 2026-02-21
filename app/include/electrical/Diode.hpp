#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    class Diode final : public ElectronicComponent {
        public:
            Diode(
                const BaseConfig& config,
                double forwardVoltage,
                const string& diodeType
            )
                : ElectronicComponent(config, Type::Diode),
                m_forwardVoltage(forwardVoltage),
                m_diodeType(diodeType)
            {
                if (forwardVoltage < 0)
                    throw invalid_argument("Forward voltage cannot be negative");
                if (diodeType.empty())
                    throw invalid_argument("Diode type cannot be empty");
            }

            const string& diodeType() const { return m_diodeType; }
            double forwardVoltage() const { return m_forwardVoltage; }

        private:
            string m_diodeType; // e.g., "Zener", "Schottky", "LED"
            double m_forwardVoltage;
    };
}
