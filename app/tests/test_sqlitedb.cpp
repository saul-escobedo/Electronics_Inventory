#include "database/exceptions/Exceptions.hpp"
#include "database/SQLiteDatabase.hpp"
#include "electrical/ElectronicComponent.hpp"
#include "electrical/Resistor.hpp"

#include <gtest/gtest.h>

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
        12, 10, 5, 0.5
    },
    .name = "R1",
    .manufacturer = "Manuf"
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
    EXPECT_THROW(db->addComponent(Resistor({}, 470, 0.05)), UninitializedDatabaseException);
    EXPECT_THROW(db->removeComponent(65), UninitializedDatabaseException);
    EXPECT_THROW(db->editComponent(65, Resistor({}, 470, 0.05)), UninitializedDatabaseException);
    EXPECT_THROW(db->getComponent(65), UninitializedDatabaseException);
    EXPECT_THROW(db->getAllComponents(), UninitializedDatabaseException);
    EXPECT_THROW(db->getAllComponentsByType(ElectronicComponent::Type::Resistor), UninitializedDatabaseException);

    // Remove db file in case it was created
    removeFile("test_db.sqlite");
}

TEST(SQLiteBackend, AddAndGetSingleComponent) {
    SQLiteDatabase sqlite("test_db.sqlite");
    Database* db = &sqlite;

    ComponentID resistorID;
    std::unique_ptr<ElectronicComponent> component;
    Resistor* resistor;

    db->initialize();

    EXPECT_NO_THROW(resistorID = db->addComponent(Resistor(baseConfig, 123, 456)));
    EXPECT_NO_THROW(component = db->getComponent(resistorID));

    resistor = dynamic_cast<Resistor*>(component.get());

    // Ensure the component is a resistor class
    EXPECT_NE(resistor, nullptr);

    EXPECT_NEAR(resistor->resistance(), 123, 1e-9);
    EXPECT_NEAR(resistor->toleranceBand(), 456, 1e-9);

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
