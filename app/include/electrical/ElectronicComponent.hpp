#pragma once

#include <iostream>
#include <string>

using std::string, std::invalid_argument;

namespace ecim {
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

    using ComponentID = size_t;

    // forward declaration of the manager
    class ElectronicsManager;

    // Base class of all modeled electrical components
    class ElectronicComponent {
        // only the manager can set the component internal id;
        friend class ElectronicsManager;
    public:
        // Type of electronic component. Any new type of component
        // (like a new dervied class) must be added here
        enum class Type {
            Resistor,
            Capacitor,
            Inductor,
            Diode,
            Transistor,
            Mosfet,
            IntegratedCircuit
        };

        // Basic properties of an electronic component. It is used
        // to initialize a new component instance
        struct BaseConfig {
            ElectronicRating rating;
            string name;
            string manufacturer;
            string partNumber;
            string description;
            size_t quantity;
        };

        // Constructor; Must provide basic properties and type of component
        explicit ElectronicComponent(
            const BaseConfig& config,
            Type type
        ) : m_rating(config.rating),
            m_name(config.name),
            m_manufacturer(config.manufacturer),
            m_partNumber(config.partNumber),
            m_description(config.description),
            m_quantity(config.quantity),
            m_type(type) {
            // only description can be empty, all other fields must be validated
            if (config.name.empty())
                throw invalid_argument("Component name cannot be empty");

            // valid compnent type?

            if (config.manufacturer.empty())
                throw invalid_argument("Manufacturer cannot be empty");

            // Part number can be empty for generic components, but if provided
            // it should not be empty
            //if (config.partNumber.empty())
            //    throw invalid_argument("Part number cannot be empty");

            if (!config.rating.valid())
                throw invalid_argument("Invalid electronic rating values");
        }

        ElectronicComponent() = delete;
        ElectronicComponent(const ElectronicComponent&) = delete;
        ElectronicComponent(ElectronicComponent&&) = delete;

        const string &name() const { return m_name; }
        Type type() const { return m_type; }
        const string &manufacturer() const { return m_manufacturer; }
        const string &partNumber() const { return m_partNumber; }
        const string &description() const { return m_description; }
        const ElectronicRating &rating() const { return m_rating; }
        const ComponentID ID() const { return m_id; }
        size_t quantity() const { return m_quantity; }

        void addQuantity(size_t amount) { m_quantity += amount; }

        void removeQuantity(size_t amount) {
            if (amount > m_quantity)
                throw invalid_argument("Cannot remove more quantity than available");
            m_quantity -= amount;
        }

    private:
        ElectronicRating m_rating;
        string m_name;
        Type m_type;
        string m_manufacturer;
        string m_partNumber;
        string m_description;
        ComponentID m_id; // unique identifier assigned by the manager
        size_t m_quantity;
    };
}
