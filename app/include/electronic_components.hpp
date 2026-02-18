#pragma once

#include <iostream>
#include <string>

using std::string, std::invalid_argument;

// electronic inventory project
namespace eip {
    struct ElectronicRating {
        double voltage;
        double current;
        double power;
        double tolerance;
    };

    class ElectronicComponent {
    public:
    explicit ElectronicComponent(
        const string& name, 
        const string& type, 
        const string& manufacturer,
        const string& partNumber, 
        const string& description, 
        const ElectronicRating& rating
    )
        : m_name(name), 
        m_type(type), 
        m_manufacturer(manufacturer),
        m_partNumber(partNumber),
        m_description(description),
        m_rating(rating)
    {
        // only description can be empty, all other fields must be validated
        if (name.empty()) 
            throw invalid_argument("Component name cannot be empty"); 
        if (type.empty()) 
            throw invalid_argument("Component type cannot be empty"); 
        if (manufacturer.empty()) 
            throw invalid_argument("Manufacturer cannot be empty");
            
        // Part number can be empty for generic components, but if provided it should not be empty
        //if (partNumber.empty())
        //    throw invalid_argument("Part number cannot be empty");
        if (rating.voltage < 0)
            throw invalid_argument("Voltage rating cannot be negative");
        if (rating.current < 0)
            throw invalid_argument("Current rating cannot be negative");
        if (rating.power < 0)
            throw invalid_argument("Power rating cannot be negative");
        if (rating.tolerance < 0)
            throw invalid_argument("Tolerance cannot be negative");
    }

    ElectronicComponent() = delete;
    ElectronicComponent(const ElectronicComponent&) = delete;
    ElectronicComponent(ElectronicComponent&&) = delete;

    const string &getName() const { return m_name; }
    const string &getType() const { return m_type; }
    const string &getManufacturer() const { return m_manufacturer; }
    const string &getPartNumber() const { return m_partNumber; }
    const string &getDescription() const { return m_description; }
    const ElectronicRating &getRating() const { return m_rating; }

    private:
        ElectronicRating m_rating;
        string m_name;
        string m_type;
        string m_manufacturer;
        string m_partNumber;
        string m_description;
    };

    class Resistor final : public ElectronicComponent {
        public:
        Resistor(
            const string& name, 
            const string& manufacturer,
            const string& partNumber, 
            const string& description, 
            const ElectronicRating& rating,
            double resistance,
            double toleranceBand
        )
            : ElectronicComponent(
                name, 
                "Resistor", 
                manufacturer, 
                partNumber, 
                description, 
                rating
            ),
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
                const string& name, 
                const string& manufacturer,
                const string& partNumber, 
                const string& description, 
                const ElectronicRating& rating,
                const string& capacitorType,
                double capacitance
            )
                : ElectronicComponent(
                    name, 
                    "Capacitor", 
                    manufacturer, 
                    partNumber, 
                    description, 
                    rating
                ),
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
                const string& name, 
                const string& manufacturer,
                const string& partNumber, 
                const string& description, 
                const ElectronicRating& rating,
                double inductance
            )
                : ElectronicComponent(
                    name, 
                    "Inductor", 
                    manufacturer, 
                    partNumber, 
                    description, 
                    rating
                ),
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
                const string& name, 
                const string& manufacturer,
                const string& partNumber, 
                const string& description, 
                const ElectronicRating& rating,
                double forwardVoltage,
                const string& diodeType
            )
                : ElectronicComponent(
                    name, 
                    "Diode", 
                    manufacturer, 
                    partNumber, 
                    description, 
                    rating
                ),
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
                const string& name, 
                const string& manufacturer,
                const string& partNumber, 
                const string& description, 
                const ElectronicRating& rating,
                double gain
            )
                : ElectronicComponent(
                    name, 
                    "Transistor", 
                    manufacturer, 
                    partNumber, 
                    description, 
                    rating
                ),
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
                const string& name, 
                const string& manufacturer,
                const string& partNumber, 
                const string& description, 
                const ElectronicRating& rating,
                double thresholdVoltage
            )
                : ElectronicComponent(
                    name, 
                    "MOSFET", 
                    manufacturer, 
                    partNumber, 
                    description, 
                    rating
                ),
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
                const string& name, 
                const string& manufacturer,
                const string& partNumber, 
                const string& description, 
                const ElectronicRating& rating,
                size_t pinCount,
                double width,
                double height,
                double length
            )
                : ElectronicComponent(
                    name, 
                    "Integrated Circuit",
                    manufacturer, 
                    partNumber, 
                    description, 
                    rating
                ),
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