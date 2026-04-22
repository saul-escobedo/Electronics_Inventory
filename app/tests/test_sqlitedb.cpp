#include "database/MassQueryConfig.hpp"
#include "database/exceptions/DatabaseException.hpp"
#include "database/exceptions/Exceptions.hpp"
#include "database/SQLiteDatabase.hpp"
#include "electrical/ElectronicComponent.hpp"
#include "electrical/Resistor.hpp"

#include <gtest/gtest.h>

#include <random>

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
    SQLiteDatabase sqlite(":memory:");
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
}

TEST(SQLiteBackend, RemoveComponent) {
    SQLiteDatabase sqlite(":memory:");
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
}

TEST(SQLiteBackend, EditComponent) {
    SQLiteDatabase sqlite(":memory:");
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

TEST(SQLiteBackend, GetAllComponents) {
    SQLiteDatabase sqlite(":memory:");
    Database* db = &sqlite;

    db->initialize();

    Resistor resistor(baseConfig, 220, 0.05);
    Capacitor capacitor(baseConfig, Capacitor::Type::AluminumElectrolytic, 100e-6);
    Inductor inductor(baseConfig, 80e-6);
    Diode diode(baseConfig, 0.4, Diode::Type::Schottky);
    BJTransistor bjTransistor(baseConfig, 120);
    FETransistor feTransistor(baseConfig, 2.4);
    IntegratedCircuit chip(baseConfig, 8, 8e-3, 6e-3, 2e-3);

    db->addComponent(resistor);
    db->addComponent(capacitor);
    db->addComponent(inductor);
    db->addComponent(diode);
    db->addComponent(bjTransistor);
    db->addComponent(feTransistor);
    db->addComponent(chip);

    MassQueryResult result = db->getAllComponents();

    EXPECT_EQ(result.numItems, 7);
    EXPECT_EQ(result.numPages, 1);

    // Determines if all types of components was returned once
    int i = 0;

    Resistor* resistorPtr;
    Capacitor* capacitorPtr;
    Inductor* inductorPtr;
    Diode* diodePtr;
    BJTransistor* bjTransistorPtr;
    FETransistor* feTransistorPtr;
    IntegratedCircuit* chipPtr;
    for(std::unique_ptr<ElectronicComponent>& p : result.items) {
        ElectronicComponent* c = p.get();

        switch(c->type()) {
        case ElectronicComponent::Type::Resistor:
            resistorPtr = dynamic_cast<Resistor*>(c);
            EXPECT_NE(resistorPtr, nullptr);
            EXPECT_EQ(resistorPtr->resistance(), 220);
            i |= 1 << 0;
            break;
        case ElectronicComponent::Type::Capacitor:
            capacitorPtr = dynamic_cast<Capacitor*>(c);
            EXPECT_NE(capacitorPtr, nullptr);
            EXPECT_NEAR(capacitorPtr->capacitance(), 100e-6, 1e-9);
            i |= 1 << 1;
            break;
        case ElectronicComponent::Type::Inductor:
            inductorPtr = dynamic_cast<Inductor*>(c);
            EXPECT_NE(inductorPtr, nullptr);
            EXPECT_NEAR(inductorPtr->inductance(), 80e-6, 1e-9);
            i |= 1 << 2;
            break;
        case ElectronicComponent::Type::Diode:
            diodePtr = dynamic_cast<Diode*>(c);
            EXPECT_NE(diodePtr, nullptr);
            EXPECT_NEAR(diodePtr->forwardVoltage(), 0.4, 1e-9);
            i |= 1 << 3;
            break;
        case ElectronicComponent::Type::BJTransistor:
            bjTransistorPtr = dynamic_cast<BJTransistor*>(c);
            EXPECT_NE(bjTransistorPtr, nullptr);
            EXPECT_EQ(bjTransistorPtr->gain(), 120);
            i |= 1 << 4;
            break;
        case ElectronicComponent::Type::FETransistor:
            feTransistorPtr = dynamic_cast<FETransistor*>(c);
            EXPECT_NE(feTransistorPtr, nullptr);
            EXPECT_NEAR(feTransistorPtr->thresholdVoltage(), 2.4, 1e-9);
            i |= 1 << 5;
            break;
        case ElectronicComponent::Type::IntegratedCircuit:
            chipPtr = dynamic_cast<IntegratedCircuit*>(c);
            EXPECT_NE(chipPtr, nullptr);
            EXPECT_EQ(chipPtr->pinCount(), 8);
            i |= 1 << 6;
            break;
        }
    }

    // Did the db return all types of components as expected?
    EXPECT_EQ(i, 0b1111111);

    result = db->getAllComponents({ .statisticsOnly = true });

    // Did it only return statistics?
    EXPECT_EQ(result.numItems, 7);
    EXPECT_EQ(result.numPages, 1);
    EXPECT_EQ(result.items.size(), 0);

    // Should throw if trying to filter by a property that is not generic
    MassQueryConfig problematicQuery = {
        .filters =
            Filter{ static_cast<ComponentProperty>(Resistor::Property::Resistance), Filter::Operation::Equals, 220.0 }
    };
    EXPECT_THROW(db->getAllComponents(problematicQuery), DatabaseException);

    db->shutdown();
}

TEST(SQLiteBackend, GetAllComponentsByType) {
    SQLiteDatabase sqlite(":memory:");
    Database* db = &sqlite;

    db->initialize();

    const int numResistors = 38;
    const int numCapacitors = 45;
    const int numInductors = 15;
    const int numDiodes = 18;
    const int numBJTs = 19;
    const int numFETs = 20;
    const int numChips = 70;
    int value = 0;

    for(int i = 0; i < numResistors; i++)
        db->addComponent(Resistor(baseConfig, value++, 0.05));

    for(int i = 0; i < numCapacitors; i++)
        db->addComponent(Capacitor(baseConfig, Capacitor::Type::Ceramic, value++));

    for(int i = 0; i < numInductors; i++)
        db->addComponent(Inductor(baseConfig, value++));

    for(int i = 0; i < numDiodes; i++)
        db->addComponent(Diode(baseConfig, value++, Diode::Type::Regular));

    for(int i = 0; i < numBJTs; i++)
        db->addComponent(BJTransistor(baseConfig, value++));

    for(int i = 0; i < numFETs; i++)
        db->addComponent(FETransistor(baseConfig, value++));

    for(int i = 0; i < numChips; i++)
        db->addComponent(IntegratedCircuit(baseConfig, value++, 1, 1, 1));

    value = 0;

    // Test if all the resistors have been correctly returned
    MassQueryResult result = db->getAllComponentsByType(ElectronicComponent::Type::Resistor);
    EXPECT_EQ(result.items.size(), numResistors);
    for(auto& resistor : result.items) {
        Resistor* ptr = dynamic_cast<Resistor*>(resistor.get());
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(value++, ptr->resistance());
    }

    // Test if all the capacitor have been correctly returned
    result = db->getAllComponentsByType(ElectronicComponent::Type::Capacitor);
    EXPECT_EQ(result.items.size(), numCapacitors);
    for(auto& capacitor : result.items) {
        Capacitor* ptr = dynamic_cast<Capacitor*>(capacitor.get());
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(value++, ptr->capacitance());
    }

    // Test if all the inductors have been correctly returned
    result = db->getAllComponentsByType(ElectronicComponent::Type::Inductor);
    EXPECT_EQ(result.items.size(), numInductors);
    for(auto& inductor : result.items) {
        Inductor* ptr = dynamic_cast<Inductor*>(inductor.get());
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(value++, ptr->inductance());
    }

    // Test if all the diodes have been correctly returned
    result = db->getAllComponentsByType(ElectronicComponent::Type::Diode);
    EXPECT_EQ(result.items.size(), numDiodes);
    for(auto& diode : result.items) {
        Diode* ptr = dynamic_cast<Diode*>(diode.get());
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(value++, ptr->forwardVoltage());
    }

    // Test if all the BJTransistors have been correctly returned
    result = db->getAllComponentsByType(ElectronicComponent::Type::BJTransistor);
    EXPECT_EQ(result.items.size(), numBJTs);
    for(auto& bjt : result.items) {
        BJTransistor* ptr = dynamic_cast<BJTransistor*>(bjt.get());
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(value++, ptr->gain());
    }

    // Test if all the FETransistors have been correctly returned
    result = db->getAllComponentsByType(ElectronicComponent::Type::FETransistor);
    EXPECT_EQ(result.items.size(), numFETs);
    for(auto& fet : result.items) {
        FETransistor* ptr = dynamic_cast<FETransistor*>(fet.get());
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(value++, ptr->thresholdVoltage());
    }

    // Test if all the IntegratedCircuits have been correctly returned
    result = db->getAllComponentsByType(ElectronicComponent::Type::IntegratedCircuit);
    EXPECT_EQ(result.items.size(), numChips);
    for(auto& chip : result.items) {
        IntegratedCircuit* ptr = dynamic_cast<IntegratedCircuit*>(chip.get());
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(value++, ptr->pinCount());
    }

    db->shutdown();
}

TEST(SQLiteBackend, Pagination) {
    SQLiteDatabase sqlite(":memory:");
    Database* db = &sqlite;

    db->initialize();

    const int itemsPerPage = 20;

    const int numResistors = 360;
    const int numCapacitors = 280;
    const int numInductors = 120;
    const int total = numResistors + numCapacitors + numInductors;
    int value = 0;

    for(int i = 0; i < numResistors; i++)
        db->addComponent(Resistor(baseConfig, value++, 0.05));

    for(int i = 0; i < numCapacitors; i++)
        db->addComponent(Capacitor(baseConfig, Capacitor::Type::Ceramic, value++));

    for(int i = 0; i < numInductors; i++)
        db->addComponent(Inductor(baseConfig, value++));

    MassQueryResult result;

    EXPECT_NO_THROW(result = db->getAllComponents({
        .pagination = Pagination(1, itemsPerPage),
        .statisticsOnly = true
    }));

    // statisticsOnly = true should not make it return items
    EXPECT_EQ(result.items.size(), 0);
    EXPECT_EQ(result.numItems, itemsPerPage);
    EXPECT_EQ(result.numPages, total / itemsPerPage);
    EXPECT_EQ(result.currentPage, 1);

    EXPECT_NO_THROW(result = db->getAllComponents({
        .pagination = Pagination(1500, itemsPerPage),
        .statisticsOnly = true
    }));

    // Current Page should be set within limits
    EXPECT_EQ(result.currentPage, total / itemsPerPage);
    EXPECT_EQ(result.numItems, itemsPerPage);

    // Turn to the page where the capacitors are
    EXPECT_NO_THROW(result = db->getAllComponents({
        .pagination = Pagination(numResistors / itemsPerPage + 1, itemsPerPage),
    }));

    // See if we get the capacitors (because they only exist on later pages)
    value = numResistors;
    for(auto& item : result.items){
        Capacitor* cap = dynamic_cast<Capacitor*>(item.get());
        EXPECT_NE(cap, nullptr);
        EXPECT_EQ(cap->capacitance(), value++);
    }

    // Turn to the page where the inductors are
    EXPECT_NO_THROW(result = db->getAllComponents({
        .pagination = Pagination((numResistors + numCapacitors) / itemsPerPage + 1, itemsPerPage),
    }));

    // See if we get the inductors (because they only exist on the last pages)
    value = numResistors + numCapacitors;
    for(auto& item : result.items){
        Inductor* ind = dynamic_cast<Inductor*>(item.get());
        EXPECT_NE(ind, nullptr);
        EXPECT_EQ(ind->inductance(), value++);
    }

    db->shutdown();
}

TEST(SQLiteBackend, Sorting) {
    SQLiteDatabase sqlite(":memory:");
    Database* db = &sqlite;

    db->initialize();

    const int numResistors = 65;
    const int numCapacitors = 55;

    std::vector<int> values;
    std::minstd_rand randomizer;

    // Add a shuffled set of resistors with different unique resistances
    values.resize(numResistors, -1);
    std::uniform_int_distribution<> r(0, numResistors);
    for(int value = 0; value < numResistors; value++) {
        int i = r(randomizer);

        while(values[i] != -1)
            i = ++i % values.size();

        values[i] = value;
        db->addComponent(Resistor(baseConfig, value, 0.05));
    }

    // Add a shuffled set of capacitors with different unique capacitances
    values.clear();
    values.resize(numCapacitors, -1);
    r = std::uniform_int_distribution<>(0, numCapacitors);
    for(int value = 0; value < numCapacitors; value++) {
        int i = r(randomizer);

        while(values[i] != -1)
            i = ++i % values.size();

        values[i] = value;
        db->addComponent(Capacitor(baseConfig, Capacitor::Type::Ceramic, value));
    }

    MassQueryResult result;
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Resistor, {
        .order = SortOrder::Acending,
        .sortBy = static_cast<ComponentProperty>(Resistor::Property::Resistance)
    }));

    // Check if the resistors are sorted in acending order
    int value = 0;
    for(auto& item : result.items) {
        Resistor* r = dynamic_cast<Resistor*>(item.get());
        EXPECT_NE(r, nullptr);
        EXPECT_EQ(r->resistance(), value++);
    }

    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Capacitor, {
        .order = SortOrder::Decending,
        .sortBy = static_cast<ComponentProperty>(Capacitor::Property::Capacitance)
    }));

    // Check if the capacitors are sorted in decending order
    value = numCapacitors - 1;
    for(auto& item : result.items) {
        Capacitor* cap = dynamic_cast<Capacitor*>(item.get());
        EXPECT_NE(cap, nullptr);
        EXPECT_EQ(cap->capacitance(), value--);
    }

    db->shutdown();
}

TEST(SQLiteBackend, Filtering) {
    SQLiteDatabase sqlite(":memory:");
    Database* db = &sqlite;

    db->initialize();

    const std::string recognizableName = "TestComponent";
    const int numResistors = 10;
    const int numCapacitors = 10;
    const int numInductors = 10;

    for (int i = 0; i < numResistors; i++) {
        ElectronicComponent::BaseConfig config = baseConfig;
        if (i == 5) config.name = recognizableName;
        db->addComponent(Resistor(config, i * 10.0, 0.05));
    }

    for (int i = 0; i < numCapacitors; i++) {
        ElectronicComponent::BaseConfig config = baseConfig;
        if (i == 5) config.name = recognizableName;
        db->addComponent(Capacitor(config, Capacitor::Type::Ceramic, i * 1e-6));
    }

    for (int i = 0; i < numInductors; i++) {
        ElectronicComponent::BaseConfig config = baseConfig;
        if (i == 5) config.name = recognizableName;
        db->addComponent(Inductor(config, i * 1e-3));
    }

    // Test 1: Filter by recognizable name using Equals, StartsWith, and EndsWith
    MassQueryConfig queryConfig;
    queryConfig.filters = Filter{ static_cast<ComponentProperty>(ElectronicComponent::Property::Name), Filter::Operation::Equals, recognizableName };
    MassQueryResult result;
    EXPECT_NO_THROW(result = db->getAllComponents(queryConfig));
    EXPECT_EQ(result.items.size(), 3);

    queryConfig.filters = Filter{ static_cast<ComponentProperty>(ElectronicComponent::Property::Name), Filter::Operation::StartsWith, recognizableName.substr(0, 4) };
    EXPECT_NO_THROW(result = db->getAllComponents(queryConfig));
    EXPECT_EQ(result.items.size(), 3);

    queryConfig.filters = Filter{ static_cast<ComponentProperty>(ElectronicComponent::Property::Name), Filter::Operation::EndsWith, recognizableName.substr(recognizableName.size() - 4, 4) };
    EXPECT_NO_THROW(result = db->getAllComponents(queryConfig));
    EXPECT_EQ(result.items.size(), 3);


    // Test 2: Filter capacitor by capacitance equal to a known value
    double knownCapacitance = 5 * 1e-6;
    queryConfig.filters = std::vector<Filter>{ // Test initialization of filter tree with a vector of one filter ---------
        { static_cast<ComponentProperty>(Capacitor::Property::Capacitance), Filter::Operation::Equals, knownCapacitance }
    };
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Capacitor, queryConfig));
    EXPECT_EQ(result.items.size(), 1);


    // Test 3: Filter inductors by InRange and NotInRange
    std::pair<double, double> range = { 2e-3, 5e-3 };
    queryConfig.filters = Filter{ static_cast<ComponentProperty>(Inductor::Property::Inductance), Filter::Operation::InRange, range };
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Inductor, queryConfig));
    EXPECT_EQ(result.items.size(), 4);

    queryConfig.filters = Filter{ static_cast<ComponentProperty>(Inductor::Property::Inductance), Filter::Operation::NotInRange, range };
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Inductor, queryConfig));
    EXPECT_EQ(result.items.size(), 6);


    // Test 4: Filter resistors by LessThan and GreaterThan
    queryConfig.filters = Filter{ static_cast<ComponentProperty>(Resistor::Property::Resistance), Filter::Operation::LessThan, 50.0 };
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Resistor, queryConfig));
    EXPECT_EQ(result.items.size(), 5);

    queryConfig.filters = Filter{ static_cast<ComponentProperty>(Resistor::Property::Resistance), Filter::Operation::GreaterThan, 50.0 };
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Resistor, queryConfig));
    EXPECT_EQ(result.items.size(), 4);


    // Test 5: Use three filters to get capacitor
    queryConfig.filters = std::vector<Filter>{
        { static_cast<ComponentProperty>(ElectronicComponent::Property::Name), Filter::Operation::Equals, recognizableName },
        { static_cast<ComponentProperty>(Capacitor::Property::Capacitance), Filter::Operation::Equals, 5 * 1e-6 },
        { static_cast<ComponentProperty>(Capacitor::Property::Type), Filter::Operation::Equals, static_cast<size_t>(Capacitor::Type::Ceramic) }
    };
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Capacitor, queryConfig));
    EXPECT_EQ(result.items.size(), 1);


    // Test 6: Use three filters to get 3 capacitors
    queryConfig.filters = { std::vector<Filter>{
        { static_cast<ComponentProperty>(Capacitor::Property::Capacitance), Filter::Operation::Equals, 4 * 1e-6 },
        { static_cast<ComponentProperty>(Capacitor::Property::Capacitance), Filter::Operation::Equals, 5 * 1e-6 },
        { static_cast<ComponentProperty>(Capacitor::Property::Capacitance), Filter::Operation::Equals, 6 * 1e-6 },
    }, FilterNode::Type::Or };
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Capacitor, queryConfig));
    EXPECT_EQ(result.items.size(), 3);


    // Test 7: More comprehensive filter tree test
    // (resistance == 40 || resistance == 50) && name == "TestComponent"
    auto orNode = FilterNode(
        std::vector<Filter>{
            { static_cast<ComponentProperty>(Resistor::Property::Resistance), Filter::Operation::Equals, 40.0 },
            { static_cast<ComponentProperty>(Resistor::Property::Resistance), Filter::Operation::Equals, 50.0 },
        }, FilterNode::Type::Or
    );
    queryConfig.filters = {
        std::vector<FilterNode>{
            orNode,
            Filter{static_cast<ComponentProperty>(ElectronicComponent::Property::Name), Filter::Operation::Equals, recognizableName }
        },
        FilterNode::Type::And
    };
    EXPECT_NO_THROW(result = db->getAllComponentsByType(ElectronicComponent::Type::Resistor, queryConfig));
    EXPECT_EQ(result.items.size(), 1);

    db->shutdown();
}
