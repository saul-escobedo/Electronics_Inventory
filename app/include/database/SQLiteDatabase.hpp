#pragma once

#include "Database.hpp"
#include "database/MassQueryConfig.hpp"
#include "electrical/BJTransistor.hpp"
#include "electrical/ElectronicComponent.hpp"
#include "electrical/IntegratedCircuit.hpp"
#include "sqlite3.h"

#include <unordered_map>

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

        sqlite3_stmt* getComponent;
        sqlite3_stmt* getResistor;
        sqlite3_stmt* getCapacitor;
        sqlite3_stmt* getInductor;
        sqlite3_stmt* getDiode;
        sqlite3_stmt* getBJTransistor;
        sqlite3_stmt* getFETransistor;
        sqlite3_stmt* getIntegratedCircuit;

        sqlite3_stmt* removeComponent;
        sqlite3_stmt* removeResistor;
        sqlite3_stmt* removeCapacitor;
        sqlite3_stmt* removeInductor;
        sqlite3_stmt* removeDiode;
        sqlite3_stmt* removeBJTransistor;
        sqlite3_stmt* removeFETransistor;
        sqlite3_stmt* removeIntegratedCircuit;

        sqlite3_stmt* editComponent;
        sqlite3_stmt* editResistor;
        sqlite3_stmt* editCapacitor;
        sqlite3_stmt* editInductor;
        sqlite3_stmt* editDiode;
        sqlite3_stmt* editBJTransistor;
        sqlite3_stmt* editFETransistor;
        sqlite3_stmt* editIntegratedCircuit;
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
        struct ComponentBase;
        struct BatchesCount;

        sqlite3* m_db;
        std::string m_dbFilename;

        SQLAccessors m_accessors;
        std::unordered_map<uint64_t, sqlite3_stmt*> m_massQueryAccessorsCache;
        std::unordered_map<uint64_t, sqlite3_stmt*> m_batchAccessorsCache;
        std::vector<size_t> m_baseIndexQueue;

        // Helper function to throw generic database errors
        //
        // message: Brief description of the error
        // sqlCode: SQL to print if it has something to do with the error
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
            ComponentID ID, const ElectronicComponent& newComponent
        );

        // Get specific component properties depending on its type
        std::unique_ptr<ElectronicComponent> _getAdditionalComponentProperties(
            ComponentID ID,
            ElectronicComponent::BaseConfig config,
            ElectronicComponent::Type type
        );

        // Edit specific component properties depending on its type
        void _editAdditionalComponentProperties(
            ComponentID ID,
            const ElectronicComponent& updatedComponent
        );

        // Speciazlied Accessor helpers
        void _addResistor(ComponentID ID, const Resistor& newResistor);
        void _addCapacitor(ComponentID ID, const Capacitor& newCapacitor);
        void _addInductor(ComponentID ID, const Inductor& newInductor);
        void _addDiode(ComponentID ID, const Diode& newDiode);
        void _addBJTransistor(ComponentID ID, const BJTransistor& newTransistor);
        void _addFETransistor(ComponentID ID, const FETransistor& newTransistor);
        void _addIntegratedCircuit(ComponentID ID, const IntegratedCircuit& newChip);
        std::unique_ptr<ElectronicComponent>
        _getResistor(ComponentID ID, const ElectronicComponent::BaseConfig& config);
        std::unique_ptr<ElectronicComponent>
        _getCapacitor(ComponentID ID, const ElectronicComponent::BaseConfig& config);
        std::unique_ptr<ElectronicComponent>
        _getInductor(ComponentID ID, const ElectronicComponent::BaseConfig& config);
        std::unique_ptr<ElectronicComponent>
        _getDiode(ComponentID ID, const ElectronicComponent::BaseConfig& config);
        std::unique_ptr<ElectronicComponent>
        _getBJTransistor(ComponentID ID, const ElectronicComponent::BaseConfig& config);
        std::unique_ptr<ElectronicComponent>
        _getFETransistor(ComponentID ID, const ElectronicComponent::BaseConfig& config);
        std::unique_ptr<ElectronicComponent>
        _getIntegratedCircuit(ComponentID ID, const ElectronicComponent::BaseConfig& config);
        void _editResistor(ComponentID ID, const Resistor& updatedResistor);
        void _editCapacitor(ComponentID ID, const Capacitor& updatedCapacitor);
        void _editInductor(ComponentID ID, const Inductor& updatedInductor);
        void _editDiode(ComponentID ID, const Diode& updatedDiode);
        void _editBJTransistor(ComponentID ID, const BJTransistor& updatedTransistor);
        void _editFETransistor(ComponentID ID, const FETransistor& updatedTransistor);
        void _editIntegratedCircuit(ComponentID ID, const IntegratedCircuit& updatedChip);

        // Mass Query Accessor helpers
        MassQueryResult _getAllComponents(
            const MassQueryConfig& config,
            std::optional<ElectronicComponent::Type> type = std::optional<ElectronicComponent::Type>()
        );
        void _getStatistics(
            const MassQueryConfig& config,
            std::optional<ElectronicComponent::Type> type,
            size_t& totalPages,
            size_t& totalItems
        );
        sqlite3_stmt* _getMassQueryAccessor(
            const MassQueryConfig& config,
            std::optional<ElectronicComponent::Type> type
        );
        sqlite3_stmt* _getBatchAccessor(ElectronicComponent::Type type, int& batchSize);
        void _applyMassQueryBindings(
            sqlite3_stmt* accessor,
            const MassQueryConfig& config,
            size_t pageIndex = 0,
            std::optional<ElectronicComponent::Type> type = std::optional<ElectronicComponent::Type>()
        );
        void _getAdditionalComponentPropertiesInBatches(
            std::vector<ComponentBase>& bases,
            BatchesCount amounts,
            std::vector<std::unique_ptr<ElectronicComponent>>& items
        );
        void _bindIdsForBatching(
            ElectronicComponent::Type type,
            sqlite3_stmt* accessor,
            std::vector<ComponentBase>& bases,
            int batchSize,
            int& headIndex,
            int& offset
        );
        void _getResistorsInBatches(
            std::vector<ComponentBase>& bases,
            int& batchSize,
            int& headIndex,
            int& offset,
            std::vector<std::unique_ptr<ElectronicComponent>>& items
        );
        void _getCapacitorsInBatches(
            std::vector<ComponentBase>& bases,
            int& batchSize,
            int& headIndex,
            int& offset,
            std::vector<std::unique_ptr<ElectronicComponent>>& items
        );
        void _getInductorsInBatches(
            std::vector<ComponentBase>& bases,
            int& batchSize,
            int& headIndex,
            int& offset,
            std::vector<std::unique_ptr<ElectronicComponent>>& items
        );
        void _getDiodesInBatches(
            std::vector<ComponentBase>& bases,
            int& batchSize,
            int& headIndex,
            int& offset,
            std::vector<std::unique_ptr<ElectronicComponent>>& items
        );
        void _getBJTransistorsInBatches(
            std::vector<ComponentBase>& bases,
            int& batchSize,
            int& headIndex,
            int& offset,
            std::vector<std::unique_ptr<ElectronicComponent>>& items
        );
        void _getFETransistorsInBatches(
            std::vector<ComponentBase>& bases,
            int& batchSize,
            int& headIndex,
            int& offset,
            std::vector<std::unique_ptr<ElectronicComponent>>& items
        );
        void _getIntegratedCircuitsInBatches(
            std::vector<ComponentBase>& bases,
            int& batchSize,
            int& headIndex,
            int& offset,
            std::vector<std::unique_ptr<ElectronicComponent>>& items
        );
    };
}
