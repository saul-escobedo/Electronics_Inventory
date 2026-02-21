#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    // Bipolar Junction Transistor
    class BJTransistor final : public ElectronicComponent {
        public:
            BJTransistor(
                const BaseConfig& config,
                double gain
            )
                : ElectronicComponent(config, Type::Transistor),
                m_gain(gain)
            {
                if (gain < 0)
                    throw invalid_argument("Gain cannot be negative");
            }

            double gain() const { return m_gain; }

        private:
            double m_gain;
    };
}
