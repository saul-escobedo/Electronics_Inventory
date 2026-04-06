#pragma once

#include "Database.hpp"
#include "electrical/ElectronicComponent.hpp"
#include "sqlite3.h"

namespace ecim {
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
        struct SQLAccessors {
            sqlite3_stmt* addComponent;
            sqlite3_stmt* addResistor;
            sqlite3_stmt* addCapacitor;
            sqlite3_stmt* addInductor;
            sqlite3_stmt* addBJTransistor;
            sqlite3_stmt* addFETransistor;
            sqlite3_stmt* addIntegratedCircuit;
        };

        sqlite3* m_db;
        std::string m_dbFilename;

        SQLAccessors m_accessors;

        // Check if the database is initialized before using accessor functions
        void _checkInitialization();
    };
}
