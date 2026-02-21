#pragma once

#include "ElectronicComponent.hpp"

namespace ecim {
    class IntegratedCircuit final : public ElectronicComponent {
        public:
            IntegratedCircuit(
                const BaseConfig& config,
                size_t pinCount,
                double width,
                double height,
                double length
            )
                : ElectronicComponent(config, Type::IntegratedCircuit),
                m_pinCount(pinCount),
                m_width(width),
                m_height(height),
                m_length(length)
            {
                if (pinCount == 0)
                    throw invalid_argument("Pin count must be greater than zero");
                if (width < 0)
                    throw invalid_argument("Width cannot be negative");
                if (height < 0)
                    throw invalid_argument("Height cannot be negative");
                if (length < 0)
                    throw invalid_argument("Length cannot be negative");
            }

            size_t pinCount() const { return m_pinCount; }
            double width() const { return m_width; }
            double height() const { return m_height; }
            double length() const { return m_length; }

        private:
            size_t m_pinCount;
            double m_width;
            double m_height;
            double m_length;
    };
}
