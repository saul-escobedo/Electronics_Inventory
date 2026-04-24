#pragma once

#include "MassQueryConfig.hpp"
#include "MassQueryResult.hpp"
#include "Transaction.hpp"

#include "electrical/ElectronicComponents.hpp"

namespace ecim {
    /// Electrical component database API. This class is the interface
    /// for database implmentations.
    ///
    /// @note Typically, SQLite is the primary backend, but for testing, a
    /// "Mock" backend can be used.
    class Database {
    public:
        /// @brief Initialize the database backend.
        ///
        /// @throws DatabaseException if database did not initialize correctly.
        virtual void initialize() = 0;

        /// @brief Gracefully save and close the database.
        ///
        /// @throws DatabaseException if database did not shutdown
        /// gracefully.
        virtual void shutdown() = 0;

        /// @brief Create a new component.
        ///
        /// @param newComponent New component information
        ///
        /// @return ID of the new component
        virtual ComponentID addComponent(
            const ElectronicComponent& newComponent) = 0;

        /// @brief Remove a component
        ///
        /// @param id ID of component to be removed
        ///
        /// @return unique_ptr<ElectronicComponent> contaning information
        /// that was removed
        virtual std::unique_ptr<ElectronicComponent> removeComponent(
            ComponentID id) = 0;

        /// @brief Update a component
        ///
        /// @param id ID of component to be updated
        /// @param updatedComponent New component information
        virtual void editComponent(
            ComponentID id,
            const ElectronicComponent& updatedComponent) = 0;

        /// @brief Read a component, based off of its ID
        ///
        /// @param id ID of component to be read
        ///
        /// @return unique_ptr<ElectronicComponent> containing the component
        /// information
        virtual std::unique_ptr<ElectronicComponent> getComponent(
            ComponentID id) = 0;

        /// @brief Returns all components of any type that match the provided
        /// mass query.
        ///
        /// @param querySettings Additional settings for the mass query.
        ///
        /// @return MassQueryResult containing the matching components.
        virtual MassQueryResult getAllComponents(
            const MassQueryConfig& queryConfig = MassQueryConfig()) = 0;

        /// @brief Returns all components of a specific type that match the
        /// provided mass query.
        ///
        /// @param type Type of components to be retrieved.
        /// @param querySettings Additional settings for the mass query.
        ///
        /// @return MassQueryResult containing the matching components.
        virtual MassQueryResult getAllComponentsByType(
            ElectronicComponent::Type type,
            const MassQueryConfig& queryConfig = MassQueryConfig()) = 0;

        /// @brief Start a database transaction.
        ///
        /// What is a transaction? Read `Transaction.hpp` to help you get up to
        /// speed on what it is and why it is useful.
        ///
        /// @throws DatabaseException if database cannot initiate a
        /// transaction.
        ///
        /// @return the handle of the new transaction. Discarding it would be
        /// moot, so don't discard it!
        ///
        /// @note call `tx->commit()` if changes are meant to be saved. When
        /// the instance is destroyed, changes are reverted.
        ///
        [[nodiscard]] virtual std::unique_ptr<Transaction> startTransaction() = 0;
    };
}
