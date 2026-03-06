#pragma once

#include "Database.hpp"
#include "electrical/ElectronicComponent.hpp"

namespace ecim {
    class SQLiteDatabase : public Database {
    public:
        SQLiteDatabase(
            const char* dbFilename
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
    };
}
