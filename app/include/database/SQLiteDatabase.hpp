#pragma once

#include "Database.hpp"
#include "electrical/BJTransistor.hpp"
#include "electrical/ElectronicComponent.hpp"
#include "electrical/IntegratedCircuit.hpp"
#include "sqlite3.h"

namespace ecim {
    struct SQLAccessors {
        sqlite3_stmt* addComponent;
        sqlite3_stmt* addResistor;
        sqlite3_stmt* addCapacitor;
        sqlite3_stmt* addInductor;
        sqlite3_stmt* addDiode;
        sqlite3_stmt* addBJTransistor;
        sqlite3_stmt* addFETransistor;
        sqlite3_stmt* addIntegratedCircuit;
    };

    class SQLiteDatabase : public Database {
    public:
        SQLiteDatabase(
            const std::string& dbFilename
        );

        void initialize() override;

        void shutdown() override;

        ComponentID addComponent(
            const ElectronicComponent& newComponent
        ) override;

        std::unique_ptr<ElectronicComponent> removeComponent(
            ComponentID id
        ) override;

        void editComponent(
            ComponentID id,
            const ElectronicComponent& updatedComponent
        ) override;

        std::unique_ptr<ElectronicComponent> getComponent(
            ComponentID id
        ) override;

        MassQueryResult getAllComponents(
            const MassQueryConfig& queryConfig
        ) override;

        MassQueryResult getAllComponentsByType(
            ElectronicComponent::Type type,
            const MassQueryConfig& queryConfig
        ) override;

        std::unique_ptr<Transaction> startTransaction() override;
    private:
        sqlite3* m_db;
        std::string m_dbFilename;

        SQLAccessors m_accessors;

        // Helper function to throw generic database errors
        //
        // message: Brief description of the error
        // sqlCode: SQL to print if the error has something to do with it
        void _throwError(const char* message, const char* sqlCode = nullptr);

        // Helper function to check the status of a sqlite statement while
        // stepping through it. It will throw an exeception if the code is an
        // error code
        //
        // code: result code to check
        // message: the message to print if an error was thrown
        void _checkResultCode(int code, const char* message);

        // Check if the database is initialized before using accessor functions
        void _checkInitialization();
        void _createSQLiteAccessors();
        void _destroySQLiteAccessors();

        // Add specific component properties depending on its type
        void _addAdditionalComponentProperties(
            ComponentID ID, const ElectronicComponent& newComponent);

        // Speciazlied Accessor helpers
        void _addResistor(ComponentID ID, const Resistor& newResistor);
        void _addCapacitor(ComponentID ID, const Capacitor& newCapacitor);
        void _addInductor(ComponentID ID, const Inductor& newInductor);
        void _addDiode(ComponentID ID, const Diode& newDiode);
        void _addBJTransistor(ComponentID ID, const BJTransistor& newTransistor);
        void _addFETransistor(ComponentID ID, const FETransistor& newTransistor);
        void _addIntegratedCircuit(ComponentID ID, const IntegratedCircuit& newChip);
    };
}
