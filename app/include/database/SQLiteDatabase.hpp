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
        sqlite3* m_db;
        std::string m_dbFilename;

        // Check if the database is initialized before using accessor functions
        void _checkInitialization();
    };
}
