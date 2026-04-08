#include "database/exceptions/Exceptions.hpp"
#include "database/SQLiteDatabase.hpp"
#include "electrical/ElectronicComponent.hpp"
#include "electrical/Resistor.hpp"

#include <gtest/gtest.h>

// Base config used for testing base electrical properties of components
#define BCONFIG_NAME "R1"
#define BCONFIG_MANUFACTURER "Manuf"
#define BCONFIG_DESCRIPTION "sum kind of electrical thingy"
#define BCONFIG_PART_NUMBER "PART-123"
#define BCONFIG_QUANTITY 394
#define BCONFIG_VOLTAGE_RATING 12
#define BCONFIG_CURRENT_RATING 10
#define BCONFIG_POWER_RATING 5

using namespace ecim;

bool fileExists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

bool removeFile(const char* filename) {
    if (fileExists(filename)) {
        remove(filename);
        return true;
    }
    return false;
}

ElectronicComponent::BaseConfig baseConfig = {
    .rating = {
        BCONFIG_VOLTAGE_RATING,
        BCONFIG_CURRENT_RATING,
        BCONFIG_POWER_RATING
    },
    .name = BCONFIG_NAME,
    .manufacturer = BCONFIG_MANUFACTURER,
    .partNumber = BCONFIG_PART_NUMBER,
    .description = BCONFIG_DESCRIPTION,
    .quantity = BCONFIG_QUANTITY
};

TEST(SQLiteBackend, InitializationAndShutdown) {
    SQLiteDatabase sqlite("test_db.sqlite");
    Database* db = &sqlite;

    db->initialize();

    ASSERT_TRUE(fileExists("test_db.sqlite"));

    db->shutdown();

    ASSERT_TRUE(fileExists("test_db.sqlite"));

    removeFile("test_db.sqlite");
}

TEST(SQLiteBackend, CheckInitialization) {
    SQLiteDatabase sqlite("test_db.sqlite");
    Database* db = &sqlite;

    // Try to access the database without it being initialized, should throw an error
    EXPECT_THROW(db->shutdown(), UninitializedDatabaseException);
    EXPECT_THROW(db->addComponent(Resistor(baseConfig, 470, 0.05)), UninitializedDatabaseException);
    EXPECT_THROW(db->removeComponent(65), UninitializedDatabaseException);
    EXPECT_THROW(db->editComponent(65, Resistor(baseConfig, 470, 0.05)), UninitializedDatabaseException);
    EXPECT_THROW(db->getComponent(65), UninitializedDatabaseException);
    EXPECT_THROW(db->getAllComponents(), UninitializedDatabaseException);
    EXPECT_THROW(db->getAllComponentsByType(ElectronicComponent::Type::Resistor), UninitializedDatabaseException);

    // Remove db file in case it was created
    removeFile("test_db.sqlite");
}

TEST(SQLiteBackend, AddAndGetSingleComponents) {
    SQLiteDatabase sqlite("test_db.sqlite");
    Database* db = &sqlite;

    ComponentID componentID;
    std::unique_ptr<ElectronicComponent> component;

    db->initialize();

    // Test Resistor class
    EXPECT_NO_THROW(componentID = db->addComponent(Resistor(baseConfig, 123, 456)));
    EXPECT_NO_THROW(component = db->getComponent(componentID));
    Resistor* resistor = dynamic_cast<Resistor*>(component.get());
    // Ensure the component is a resistor class
    EXPECT_NE(resistor, nullptr);
    EXPECT_NEAR(resistor->resistance(), 123, 1e-9);
    EXPECT_NEAR(resistor->toleranceBand(), 456, 1e-9);

    // Test Base class
    EXPECT_EQ(resistor->name(), BCONFIG_NAME);
    EXPECT_EQ(resistor->manufacturer(), BCONFIG_MANUFACTURER);
    EXPECT_EQ(resistor->partNumber(), BCONFIG_PART_NUMBER);
    EXPECT_EQ(resistor->description(), BCONFIG_DESCRIPTION);
    EXPECT_EQ(resistor->quantity(), BCONFIG_QUANTITY);
    EXPECT_EQ(resistor->rating().voltage, BCONFIG_VOLTAGE_RATING);
    EXPECT_EQ(resistor->rating().current, BCONFIG_CURRENT_RATING);
    EXPECT_EQ(resistor->rating().power, BCONFIG_POWER_RATING);

    // Test Capacitor class
    EXPECT_NO_THROW(componentID = db->addComponent(Capacitor(baseConfig, Capacitor::Type::Ceramic, 100e-6)));
    EXPECT_NO_THROW(component = db->getComponent(componentID));
    Capacitor* capacitor = dynamic_cast<Capacitor*>(component.get());
    EXPECT_NE(capacitor, nullptr);
    EXPECT_EQ(capacitor->capacitorType(), Capacitor::Type::Ceramic);
    EXPECT_NEAR(capacitor->capacitance(), 100e-6, 1e-9);

    // Test Inductor class
    EXPECT_NO_THROW(componentID = db->addComponent(Inductor(baseConfig, 10e-3)));
    EXPECT_NO_THROW(component = db->getComponent(componentID));
    Inductor* inductor = dynamic_cast<Inductor*>(component.get());
    EXPECT_NE(inductor, nullptr);
    EXPECT_NEAR(inductor->inductance(), 10e-3, 1e-9);

    // Test Diode class
    EXPECT_NO_THROW(componentID = db->addComponent(Diode(baseConfig, 0.7, Diode::Type::LED)));
    EXPECT_NO_THROW(component = db->getComponent(componentID));
    Diode* diode = dynamic_cast<Diode*>(component.get());
    EXPECT_NE(diode, nullptr);
    EXPECT_EQ(diode->diodeType(), Diode::Type::LED);
    EXPECT_NEAR(diode->forwardVoltage(), 0.7, 1e-9);

    // Test BJTransistor class
    EXPECT_NO_THROW(componentID = db->addComponent(BJTransistor(baseConfig, 100)));
    EXPECT_NO_THROW(component = db->getComponent(componentID));
    BJTransistor* bjTransistor = dynamic_cast<BJTransistor*>(component.get());
    EXPECT_NE(bjTransistor, nullptr);
    EXPECT_NEAR(bjTransistor->gain(), 100, 1e-9);

    // Test FETransistor class
    EXPECT_NO_THROW(componentID = db->addComponent(FETransistor(baseConfig, 1.5)));
    EXPECT_NO_THROW(component = db->getComponent(componentID));
    FETransistor* feTransistor = dynamic_cast<FETransistor*>(component.get());
    EXPECT_NE(feTransistor, nullptr);
    EXPECT_NEAR(feTransistor->thresholdVoltage(), 1.5, 1e-9);

    // Test IntegratedCircuit class
    EXPECT_NO_THROW(componentID = db->addComponent(IntegratedCircuit(baseConfig, 16, 10.0, 5.0, 2.0)));
    EXPECT_NO_THROW(component = db->getComponent(componentID));
    IntegratedCircuit* ic = dynamic_cast<IntegratedCircuit*>(component.get());
    EXPECT_NE(ic, nullptr);
    EXPECT_EQ(ic->pinCount(), 16);
    EXPECT_NEAR(ic->width(), 10.0, 1e-9);
    EXPECT_NEAR(ic->height(), 5.0, 1e-9);
    EXPECT_NEAR(ic->length(), 2.0, 1e-9);

    db->shutdown();

    removeFile("test_db.sqlite");
}

TEST(SQLiteBackend, RemoveComponent) {
    SQLiteDatabase sqlite("test_db.sqlite");
    Database* db = &sqlite;

    ComponentID resistorID;
    std::unique_ptr<ElectronicComponent> component;

    db->initialize();

    // Add a resistor to the database
    EXPECT_NO_THROW(resistorID = db->addComponent(Resistor(baseConfig, 470, 0.05)));

    // Ensure the component exists
    EXPECT_NO_THROW(component = db->getComponent(resistorID));
    EXPECT_NE(component, nullptr);

    // Remove the component
    EXPECT_NO_THROW(component = db->removeComponent(resistorID));

    // Ensure the component no longer exists
    EXPECT_NO_THROW(component = db->getComponent(resistorID));
    EXPECT_EQ(component, nullptr);

    db->shutdown();

    removeFile("test_db.sqlite");
}

TEST(SQLiteBackend, EditComponent) {
    SQLiteDatabase sqlite("test_db.sqlite");
    Database* db = &sqlite;

    ComponentID resistorID;
    std::unique_ptr<ElectronicComponent> component;

    db->initialize();

    // Add a resistor to the database
    EXPECT_NO_THROW(resistorID = db->addComponent(Resistor(baseConfig, 470, 0.05)));

    // Ensure the component exists
    EXPECT_NO_THROW(component = db->getComponent(resistorID));
    Resistor* resistor = dynamic_cast<Resistor*>(component.get());
    EXPECT_NE(resistor, nullptr);
    EXPECT_NEAR(resistor->resistance(), 470, 1e-9);
    EXPECT_NEAR(resistor->toleranceBand(), 0.05, 1e-9);

    // Edit the resistor's properties
    Resistor updatedResistor(baseConfig, 220, 0.1);
    EXPECT_NO_THROW(db->editComponent(resistorID, updatedResistor));

    // Retrieve the updated component
    EXPECT_NO_THROW(component = db->getComponent(resistorID));
    resistor = dynamic_cast<Resistor*>(component.get());
    EXPECT_NE(resistor, nullptr);
    EXPECT_NEAR(resistor->resistance(), 220, 1e-9);
    EXPECT_NEAR(resistor->toleranceBand(), 0.1, 1e-9);

    // Make sure that it rejects it if the component is a different type
    Capacitor capacitor(baseConfig, Capacitor::Type::Ceramic, 100e-6);
    EXPECT_THROW(db->editComponent(resistorID, capacitor), DatabaseException);

    // Throw an error if the component does not exist; no edits can be done
    EXPECT_THROW(db->editComponent(9999, updatedResistor), DatabaseException);

    // Edit and test other component types ----------

    // Capacitor
    Capacitor updatedCapacitor(baseConfig, Capacitor::Type::Film, 220e-6);
    ComponentID capacitorID = db->addComponent(Capacitor(baseConfig, Capacitor::Type::Ceramic, 100e-6));
    EXPECT_NO_THROW(db->editComponent(capacitorID, updatedCapacitor));
    EXPECT_NO_THROW(component = db->getComponent(capacitorID));
    Capacitor* cap = dynamic_cast<Capacitor*>(component.get());
    EXPECT_NE(cap, nullptr);
    EXPECT_EQ(cap->capacitorType(), Capacitor::Type::Film);
    EXPECT_NEAR(cap->capacitance(), 220e-6, 1e-9);

    // Inductor
    Inductor updatedInductor(baseConfig, 20e-3);
    ComponentID inductorID = db->addComponent(Inductor(baseConfig, 10e-3));
    EXPECT_NO_THROW(db->editComponent(inductorID, updatedInductor));
    EXPECT_NO_THROW(component = db->getComponent(inductorID));
    Inductor* inductor = dynamic_cast<Inductor*>(component.get());
    EXPECT_NE(inductor, nullptr);
    EXPECT_NEAR(inductor->inductance(), 20e-3, 1e-9);

    // Diode
    Diode updatedDiode(baseConfig, 1.2, Diode::Type::Zener);
    ComponentID diodeID = db->addComponent(Diode(baseConfig, 0.7, Diode::Type::LED));
    EXPECT_NO_THROW(db->editComponent(diodeID, updatedDiode));
    EXPECT_NO_THROW(component = db->getComponent(diodeID));
    Diode* diode = dynamic_cast<Diode*>(component.get());
    EXPECT_NE(diode, nullptr);
    EXPECT_EQ(diode->diodeType(), Diode::Type::Zener);
    EXPECT_NEAR(diode->forwardVoltage(), 1.2, 1e-9);

    // BJTransistor
    BJTransistor updatedBJTransistor(baseConfig, 200);
    ComponentID bjTransistorID = db->addComponent(BJTransistor(baseConfig, 100));
    EXPECT_NO_THROW(db->editComponent(bjTransistorID, updatedBJTransistor));
    EXPECT_NO_THROW(component = db->getComponent(bjTransistorID));
    BJTransistor* bjTransistor = dynamic_cast<BJTransistor*>(component.get());
    EXPECT_NE(bjTransistor, nullptr);
    EXPECT_NEAR(bjTransistor->gain(), 200, 1e-9);

    // FETransistor
    FETransistor updatedFETransistor(baseConfig, 2.5);
    ComponentID feTransistorID = db->addComponent(FETransistor(baseConfig, 1.5));
    EXPECT_NO_THROW(db->editComponent(feTransistorID, updatedFETransistor));
    EXPECT_NO_THROW(component = db->getComponent(feTransistorID));
    FETransistor* feTransistor = dynamic_cast<FETransistor*>(component.get());
    EXPECT_NE(feTransistor, nullptr);
    EXPECT_NEAR(feTransistor->thresholdVoltage(), 2.5, 1e-9);

    // IntegratedCircuit
    IntegratedCircuit updatedIC(baseConfig, 32, 20.0, 10.0, 5.0);
    ElectronicComponent::BaseConfig boggusConfig = {
        .rating = { 4, 2, 0.1 },
        .name = "noname",
        .manufacturer = "nomanu",
        .partNumber = "1EE7",
        .description = "lol",
        .quantity = 500000
    };
    ComponentID icID = db->addComponent(IntegratedCircuit(boggusConfig, 16, 10.0, 5.0, 2.0));
    EXPECT_NO_THROW(db->editComponent(icID, updatedIC));
    EXPECT_NO_THROW(component = db->getComponent(icID));
    IntegratedCircuit* ic = dynamic_cast<IntegratedCircuit*>(component.get());
    EXPECT_NE(ic, nullptr);
    EXPECT_EQ(ic->pinCount(), 32);
    EXPECT_NEAR(ic->width(), 20.0, 1e-9);
    EXPECT_NEAR(ic->height(), 10.0, 1e-9);
    EXPECT_NEAR(ic->length(), 5.0, 1e-9);

    // Base properties check
    EXPECT_EQ(ic->name(), BCONFIG_NAME);
    EXPECT_EQ(ic->manufacturer(), BCONFIG_MANUFACTURER);
    EXPECT_EQ(ic->partNumber(), BCONFIG_PART_NUMBER);
    EXPECT_EQ(ic->description(), BCONFIG_DESCRIPTION);
    EXPECT_EQ(ic->rating().voltage, BCONFIG_VOLTAGE_RATING);
    EXPECT_EQ(ic->rating().current, BCONFIG_CURRENT_RATING);
    EXPECT_EQ(ic->rating().power, BCONFIG_POWER_RATING);
    EXPECT_EQ(ic->quantity(), BCONFIG_QUANTITY);

    db->shutdown();

    removeFile("test_db.sqlite");
}

TEST(SQLiteBackend, EnsurePersistance) {
    Database* db;
    ComponentID resistorID;
    std::unique_ptr<ElectronicComponent> component;
    Resistor* resistor;

    {
        SQLiteDatabase sqlite("test_db.sqlite");
        db = &sqlite;

        db->initialize();

        resistorID = db->addComponent(Resistor(baseConfig, 470, 0.05));

        db->shutdown();
    }

    // Launch the database backend again to check persistance
    {
        SQLiteDatabase sqlite("test_db.sqlite");
        db = &sqlite;

        db->initialize();

        EXPECT_NO_THROW(component = db->getComponent(resistorID));

        resistor = dynamic_cast<Resistor*>(component.get());

        // Ensure the component is a resistor class
        EXPECT_NE(resistor, nullptr);

        EXPECT_NEAR(resistor->resistance(), 470, 1e-9);

        db->shutdown();
    }

    removeFile("test_db.sqlite");
}
