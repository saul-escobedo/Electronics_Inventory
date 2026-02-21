#pragma once

#include <iostream>
#include <string>

using std::string, std::invalid_argument;

// electronic components inventory manager
namespace ecim {

    enum class ComponentType {
        Resistor,
        Capacitor,
        Inductor,
        Diode,
        Transistor,
        Mosfet,
        IntegratedCircuit
    };

    struct ElectronicRating {
        double voltage;
        double current;
        double power;
        double tolerance;

        bool valid() const {
            return voltage >= 0
                && current >= 0
                && power >= 0
                && tolerance >= 0;
        }
    };

    struct ComponentBaseConfig {
        ElectronicRating rating;
        string name;
        string manufacturer;
        string partNumber;
        string description;
        size_t quantity;
    };

    using componentId = size_t;

    // forward declaration of the manager
    class ElectronicsManager;

    class ElectronicComponent {
        // only the manager can set the component internal id;
        friend class ElectronicsManager;

    public:
    explicit ElectronicComponent(
            const ComponentBaseConfig& config,
            ComponentType type
    )
        : m_rating(config.rating),
        m_name(config.name),
        m_manufacturer(config.manufacturer),
        m_partNumber(config.partNumber),
        m_description(config.description),
        m_quantity(config.quantity),
        m_type(type)
    {
        // only description can be empty, all other fields must be validated
        if (config.name.empty())
            throw invalid_argument("Component name cannot be empty");

        // valid compnent type?

        if (config.manufacturer.empty())
            throw invalid_argument("Manufacturer cannot be empty");

        // Part number can be empty for generic components, but if provided it should not be empty
        //if (config.partNumber.empty())
        //    throw invalid_argument("Part number cannot be empty");

        if (!config.rating.valid())
            throw invalid_argument("Invalid electronic rating values");
    }

    ElectronicComponent() = delete;
    ElectronicComponent(const ElectronicComponent&) = delete;
    ElectronicComponent(ElectronicComponent&&) = delete;

    const string &name() const { return m_name; }
    ComponentType type() const { return m_type; }
    const string &manufacturer() const { return m_manufacturer; }
    const string &partNumber() const { return m_partNumber; }
    const string &description() const { return m_description; }
    const ElectronicRating &rating() const { return m_rating; }
    const componentId id() const { return m_id; }
    size_t quantity() const { return m_quantity; }

    void addQuantity(size_t amount) { m_quantity += amount; }

    void removeQuantity(size_t amount) {
        if (amount > m_quantity)
            throw invalid_argument("Cannot remove more quantity than available");
        m_quantity -= amount;
    }

    private:
        // only the manager can use this function.
        void _setId(componentId id) { m_id = id; }
        ElectronicRating m_rating;
        string m_name;
        ComponentType m_type;
        string m_manufacturer;
        string m_partNumber;
        string m_description;
        componentId m_id; // unique identifier assigned by the manager
        size_t m_quantity;
    };

    class Resistor final : public ElectronicComponent {
        public:
        Resistor(
            const ComponentBaseConfig& config,
            double resistance,
            double toleranceBand
        )
            : ElectronicComponent(config, ComponentType::Resistor),
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

    class Capacitor final : public ElectronicComponent {
        public:
            Capacitor(
                const ComponentBaseConfig& config,
                const string& capacitorType,
                double capacitance
            )
                : ElectronicComponent(config, ComponentType::Capacitor),
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

    class Inductor final : public ElectronicComponent {
        public:
            Inductor(
                const ComponentBaseConfig& config,
                double inductance
            )
                : ElectronicComponent(config, ComponentType::Inductor),
                m_inductance(inductance)
            {
                if (inductance < 0)
                    throw invalid_argument("Inductance cannot be negative");
            }

            double inductance() const { return m_inductance; }

        private:
            double m_inductance;
    };

    class Diode final : public ElectronicComponent {
        public:
            Diode(
                const ComponentBaseConfig& config,
                double forwardVoltage,
                const string& diodeType
            )
                : ElectronicComponent(config, ComponentType::Diode),
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

    class Transistor final : public ElectronicComponent {
        public:
            Transistor(
                const ComponentBaseConfig& config,
                double gain
            )
                : ElectronicComponent(config, ComponentType::Transistor),
                m_gain(gain)
            {
                if (gain < 0)
                    throw invalid_argument("Gain cannot be negative");
            }

            double gain() const { return m_gain; }

        private:
            double m_gain;
    };

    class Mosfet final : public ElectronicComponent {
        public:
            Mosfet(
                const ComponentBaseConfig& config,
                double thresholdVoltage
            )
                : ElectronicComponent(config, ComponentType::Mosfet),
                m_thresholdVoltage(thresholdVoltage)
            {
                if (thresholdVoltage < 0)
                    throw invalid_argument("Threshold voltage cannot be negative");
            }

            double thresholdVoltage() const { return m_thresholdVoltage; }

        private:
            double m_thresholdVoltage;
    };

    class IntegratedCircuit final : public ElectronicComponent {
        public:
            IntegratedCircuit(
                const ComponentBaseConfig& config,
                size_t pinCount,
                double width,
                double height,
                double length
            )
                : ElectronicComponent(config, ComponentType::IntegratedCircuit),
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
