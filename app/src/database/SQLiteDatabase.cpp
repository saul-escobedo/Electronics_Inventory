#include "database/SQLiteDatabase.hpp"
#include "database/EmbeddedSQLCode.hpp"
#include "database/MassQueryConfig.hpp"
#include "database/exceptions/Exceptions.hpp"

#include "electrical/ElectronicComponent.hpp"
#include "electrical/ElectronicComponents.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
using namespace ecim;

#define DB_BACKEND_NAME "sqlite3"

#define SHOULD_BATCH_THRESHOLD 32
#define MIN_BATCH_SIZE 16
#define MAX_BATCH_SIZE 4096

// Statement accessor / SQL code pairs. Used to prepare the accessors'
// sqlite statements with their respective SQL code
static const std::pair<int, const char*> ACCESSORS_CODE[] = {
    { offsetof(SQLAccessors, addComponent), DBSQL_ADD_COMPONENT },
    { offsetof(SQLAccessors, addResistor), DBSQL_ADD_RESISTOR },
    { offsetof(SQLAccessors, addCapacitor), DBSQL_ADD_CAPACITOR },
    { offsetof(SQLAccessors, addInductor), DBSQL_ADD_INDUCTOR },
    { offsetof(SQLAccessors, addDiode), DBSQL_ADD_DIODE },
    { offsetof(SQLAccessors, addBJTransistor), DBSQL_ADD_BJ_TRANSISTOR },
    { offsetof(SQLAccessors, addFETransistor), DBSQL_ADD_FE_TRANSISTOR },
    { offsetof(SQLAccessors, addIntegratedCircuit), DBSQL_ADD_INTEGRATED_CIRCUIT },

    { offsetof(SQLAccessors, getComponent), DBSQL_GET_COMPONENT },
    { offsetof(SQLAccessors, getResistor), DBSQL_GET_RESISTOR },
    { offsetof(SQLAccessors, getCapacitor), DBSQL_GET_CAPACITOR },
    { offsetof(SQLAccessors, getInductor), DBSQL_GET_INDUCTOR },
    { offsetof(SQLAccessors, getDiode), DBSQL_GET_DIODE },
    { offsetof(SQLAccessors, getBJTransistor), DBSQL_GET_BJ_TRANSISTOR },
    { offsetof(SQLAccessors, getFETransistor), DBSQL_GET_FE_TRANSISTOR },
    { offsetof(SQLAccessors, getIntegratedCircuit), DBSQL_GET_INTEGRATED_CIRCUIT },

    { offsetof(SQLAccessors, removeComponent), DBSQL_REMOVE_COMPONENT },
    { offsetof(SQLAccessors, removeResistor), DBSQL_REMOVE_RESISTOR },
    { offsetof(SQLAccessors, removeCapacitor), DBSQL_REMOVE_CAPACITOR },
    { offsetof(SQLAccessors, removeInductor), DBSQL_REMOVE_INDUCTOR },
    { offsetof(SQLAccessors, removeDiode), DBSQL_REMOVE_DIODE },
    { offsetof(SQLAccessors, removeBJTransistor), DBSQL_REMOVE_BJ_TRANSISTOR },
    { offsetof(SQLAccessors, removeFETransistor), DBSQL_REMOVE_FE_TRANSISTOR },
    { offsetof(SQLAccessors, removeIntegratedCircuit), DBSQL_REMOVE_INTEGRATED_CIRCUIT },

    { offsetof(SQLAccessors, editComponent), DBSQL_EDIT_COMPONENT },
    { offsetof(SQLAccessors, editResistor), DBSQL_EDIT_RESISTOR },
    { offsetof(SQLAccessors, editCapacitor), DBSQL_EDIT_CAPACITOR },
    { offsetof(SQLAccessors, editInductor), DBSQL_EDIT_INDUCTOR },
    { offsetof(SQLAccessors, editDiode), DBSQL_EDIT_DIODE },
    { offsetof(SQLAccessors, editBJTransistor), DBSQL_EDIT_BJ_TRANSISTOR },
    { offsetof(SQLAccessors, editFETransistor), DBSQL_EDIT_FE_TRANSISTOR },
    { offsetof(SQLAccessors, editIntegratedCircuit), DBSQL_EDIT_INTEGRATED_CIRCUIT }
};

struct SQLiteDatabase::ComponentBase {
    ComponentID ID;
    ElectronicComponent::Type type;
    ElectronicComponent::BaseConfig config;
    size_t next;
};

struct SQLiteDatabase::BatchesCount {
    size_t numResistors;
    size_t numCapacitors;
    size_t numInductors;
    size_t numDiodes;
    size_t numBJTs;
    size_t numFETs;
    size_t numChips;
};

template <class T>
inline static void s_combineHash(uint64_t& s, const T& v) {
    std::hash<T> hash;
    s ^= hash(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
}

static std::string s_genSQLStatement4MassQuery(const MassQueryConfig& config, std::optional<ElectronicComponent::Type>);
static std::string s_genSQLStatement4Batches(ElectronicComponent::Type type, int batchSize);
static int s_chooseBestBatchSize(int sizeHint);
static int s_floorPowerOf2(int x);

SQLiteDatabase::SQLiteDatabase(
    const std::string& dbFilename
) : m_dbFilename(dbFilename),
    m_db(nullptr) {

}

void SQLiteDatabase::initialize() {
    int ret;
    char* errMessage;

    ret = sqlite3_open(m_dbFilename.c_str(), &m_db);

    if(ret)
        throw FailedInitializationException(DB_BACKEND_NAME, sqlite3_errmsg(m_db));

    ret = sqlite3_exec(m_db, DBSQL_SCHEMA, NULL, NULL, &errMessage);

    if(ret != SQLITE_OK) {
        std::string message = "Failed building the schema:\n";
        message += errMessage;

        sqlite3_free(errMessage);

        throw FailedInitializationException(DB_BACKEND_NAME, message);
    }

    _createSQLiteAccessors();
}

void SQLiteDatabase::shutdown() {
    _checkInitialization();
    _destroySQLiteAccessors();

    sqlite3_close(m_db);
}

ComponentID SQLiteDatabase::addComponent(
    const ElectronicComponent& newComponent
) {
    int ret;

    _checkInitialization();

    sqlite3_reset(m_accessors.addComponent);
    sqlite3_clear_bindings(m_accessors.addComponent);

    sqlite3_bind_text(
        m_accessors.addComponent,
        1,
        newComponent.name().c_str(),
        newComponent.name().length(),
        SQLITE_STATIC
    );

    sqlite3_bind_int(
        m_accessors.addComponent,
        2,
        static_cast<int>(newComponent.type())
    );

    sqlite3_bind_text(
        m_accessors.addComponent,
        3,
        newComponent.manufacturer().c_str(),
        newComponent.manufacturer().length(),
        SQLITE_STATIC
    );

    if(!newComponent.partNumber().empty())
        sqlite3_bind_text(
            m_accessors.addComponent,
            4,
            newComponent.partNumber().c_str(),
            newComponent.partNumber().length(),
            SQLITE_STATIC
        );

    if(!newComponent.description().empty())
        sqlite3_bind_text(
            m_accessors.addComponent,
            5,
            newComponent.description().c_str(),
            newComponent.description().length(),
            SQLITE_STATIC
        );

    sqlite3_bind_int(
        m_accessors.addComponent,
        6,
        newComponent.quantity()
    );

    sqlite3_bind_double(
        m_accessors.addComponent,
        7,
        newComponent.rating().voltage
    );

    sqlite3_bind_double(
        m_accessors.addComponent,
        8,
        newComponent.rating().current
    );

    sqlite3_bind_double(
        m_accessors.addComponent,
        9,
        newComponent.rating().power
    );

    ret = sqlite3_step(m_accessors.addComponent);

    _checkResultCode(ret, "Unable to add electrical component");

    ComponentID newID = sqlite3_last_insert_rowid(m_db);

    if(newID == 0)
        throw DatabaseException(DB_BACKEND_NAME ": Unable to add electrical component, ComponentID should not be zero");

    _addAdditionalComponentProperties(newID, newComponent);

    return newID;
}

std::unique_ptr<ElectronicComponent> SQLiteDatabase::removeComponent(
    ComponentID id
) {
    int ret;
    std::unique_ptr<ElectronicComponent> removedComponent;

    _checkInitialization();

    removedComponent = getComponent(id);

    if(!removedComponent)
        return nullptr;

    sqlite3_reset(m_accessors.removeComponent);
    sqlite3_bind_int64(m_accessors.removeComponent, 1, id);

    ret = sqlite3_step(m_accessors.removeComponent);

    _checkResultCode(ret, "Unable to remove component");

    return removedComponent;
}

void SQLiteDatabase::editComponent(
    ComponentID id,
    const ElectronicComponent& updatedComponent
) {
    int ret;

    _checkInitialization();

    auto previousComponent = getComponent(id);

    if(!previousComponent)
        throw DatabaseException(DB_BACKEND_NAME ": Unable to edit electrical component because it does not exist");

    if(updatedComponent.type() != previousComponent->type())
        throw DatabaseException(DB_BACKEND_NAME ": Unable to edit electrical component because it is of different type");

    sqlite3_reset(m_accessors.editComponent);
    sqlite3_clear_bindings(m_accessors.editComponent);

    sqlite3_bind_text(
        m_accessors.editComponent,
        1,
        updatedComponent.name().c_str(),
        updatedComponent.name().length(),
        SQLITE_STATIC
    );

    sqlite3_bind_text(
        m_accessors.editComponent,
        2,
        updatedComponent.manufacturer().c_str(),
        updatedComponent.manufacturer().length(),
        SQLITE_STATIC
    );

    if(!updatedComponent.partNumber().empty())
        sqlite3_bind_text(
            m_accessors.editComponent,
            3,
            updatedComponent.partNumber().c_str(),
            updatedComponent.partNumber().length(),
            SQLITE_STATIC
        );

    if(!updatedComponent.description().empty())
        sqlite3_bind_text(
            m_accessors.editComponent,
            4,
            updatedComponent.description().c_str(),
            updatedComponent.description().length(),
            SQLITE_STATIC
        );

    sqlite3_bind_int(
        m_accessors.editComponent,
        5,
        updatedComponent.quantity()
    );

    sqlite3_bind_double(
        m_accessors.editComponent,
        6,
        updatedComponent.rating().voltage
    );

    sqlite3_bind_double(
        m_accessors.editComponent,
        7,
        updatedComponent.rating().current
    );

    sqlite3_bind_double(
        m_accessors.editComponent,
        8,
        updatedComponent.rating().power
    );

    sqlite3_bind_int64(
        m_accessors.editComponent,
        9,
        id
    );

    ret = sqlite3_step(m_accessors.editComponent);

    _checkResultCode(ret, "Unable to edit electrical component");

    _editAdditionalComponentProperties(id, updatedComponent);
}

std::unique_ptr<ElectronicComponent> SQLiteDatabase::getComponent(
    ComponentID id
) {
    ElectronicComponent::BaseConfig config;
    ElectronicComponent::Type type;
    int ret;

    _checkInitialization();

    sqlite3_reset(m_accessors.getComponent);
    sqlite3_bind_int64(m_accessors.getComponent, 1, id);

    ret = sqlite3_step(m_accessors.getComponent);

    _checkResultCode(ret, "Unable to get electrical component");

    if(ret != SQLITE_ROW)
        return nullptr;

    config.name = reinterpret_cast<const char*>(
        sqlite3_column_text(m_accessors.getComponent, 1)
    );

    type = static_cast<ElectronicComponent::Type>(
        sqlite3_column_int(m_accessors.getComponent, 2)
    );

    config.manufacturer = reinterpret_cast<const char*>(
        sqlite3_column_text(m_accessors.getComponent, 3)
    );

    if(sqlite3_column_type(m_accessors.getComponent, 4) == SQLITE_TEXT)
        config.partNumber = reinterpret_cast<const char*>(
           sqlite3_column_text(m_accessors.getComponent, 4)
        );

    if(sqlite3_column_type(m_accessors.getComponent, 5) == SQLITE_TEXT)
        config.description = reinterpret_cast<const char*>(
            sqlite3_column_text(m_accessors.getComponent, 5)
        );

    config.quantity = sqlite3_column_int(m_accessors.getComponent, 6);

    config.rating.voltage = sqlite3_column_double(m_accessors.getComponent, 7);
    config.rating.current = sqlite3_column_double(m_accessors.getComponent, 8);
    config.rating.power = sqlite3_column_double(m_accessors.getComponent, 9);

    return _getAdditionalComponentProperties(id, config, type);
}

MassQueryResult SQLiteDatabase::getAllComponents(
    const MassQueryConfig& queryConfig
) {
    for(const Filter& filter : queryConfig.filters)
        if(static_cast<size_t>(filter.property) >= static_cast<size_t>(ElectronicComponent::Property::End))
            _throwError("Non-generic property used in a generic mass query");

    return _getAllComponents(queryConfig);
}

MassQueryResult SQLiteDatabase::getAllComponentsByType(
    ElectronicComponent::Type type,
    const MassQueryConfig& queryConfig
) {
    _checkInitialization();

    return _getAllComponents(queryConfig, type);
}

std::unique_ptr<Transaction> SQLiteDatabase::startTransaction() {
    _checkInitialization();

    return nullptr;
}

void SQLiteDatabase::_throwError(const char* m, const char* sqlCode) {
    std::string message = DB_BACKEND_NAME ": ";
    message += m;
    message += ":\n";
    message += sqlite3_errmsg(m_db);

    if(sqlCode) {
        message += "\n\nWith SQL Code:\n";
        message += sqlCode;
    }
    else
        message += '\n';

    throw DatabaseException(message);
}

void SQLiteDatabase::_checkResultCode(int code, const char* message) {
    if(code != SQLITE_OK && code != SQLITE_ROW && code != SQLITE_DONE)
        _throwError(message);
}

void SQLiteDatabase::_checkInitialization() {
    if(!m_db)
        throw UninitializedDatabaseException(DB_BACKEND_NAME);
}

void SQLiteDatabase::_createSQLiteAccessors() {
    int ret;

    for(auto accessor : ACCESSORS_CODE) {
        sqlite3_stmt** a = reinterpret_cast<sqlite3_stmt**>(
            reinterpret_cast<char*>(&m_accessors) + accessor.first);

        ret = sqlite3_prepare_v2(m_db, accessor.second, -1, a, nullptr);

        if(ret != SQLITE_OK)
            _throwError("Failed to prepare accessor SQL statement", accessor.second);
    }
}

void SQLiteDatabase::_destroySQLiteAccessors() {
    int ret;

    for(auto accessor : ACCESSORS_CODE) {
        sqlite3_stmt** a = reinterpret_cast<sqlite3_stmt**>(
            reinterpret_cast<char*>(&m_accessors) + accessor.first);

        ret = sqlite3_finalize(*a);

        if(ret != SQLITE_OK)
            _throwError("Failed to destroy accessor SQL statement", accessor.second);
    }

    // Finalize any mass accessors in cache
    for(auto& massAccessor : m_massQueryAccessorsCache) {
        ret = sqlite3_finalize(massAccessor.second);

        if(ret != SQLITE_OK)
            _throwError("Failed to destroy mass accessor SQL statement");
    }

    // Finalize any batch accessors in cache
    for(auto& batchAccessor : m_batchAccessorsCache) {
        ret = sqlite3_finalize(batchAccessor.second);

        if(ret != SQLITE_OK)
            _throwError("Failed to destroy batch accessor SQL statement");
    }
}

void SQLiteDatabase::_addAdditionalComponentProperties(
    ComponentID ID,
    const ElectronicComponent& newComponent
) {
    const ElectronicComponent* c = &newComponent;

    switch(newComponent.type()) {
    case ElectronicComponent::Type::Resistor:
        _addResistor(ID, *static_cast<const Resistor*>(c));
        break;
    case ElectronicComponent::Type::Capacitor:
        _addCapacitor(ID, *static_cast<const Capacitor*>(c));
        break;
    case ElectronicComponent::Type::Inductor:
        _addInductor(ID, *static_cast<const Inductor*>(c));
        break;
    case ElectronicComponent::Type::Diode:
        _addDiode(ID, *static_cast<const Diode*>(c));
        break;
    case ElectronicComponent::Type::BJTransistor:
        _addBJTransistor(ID, *static_cast<const BJTransistor*>(c));
        break;
    case ElectronicComponent::Type::FETransistor:
        _addFETransistor(ID, *static_cast<const FETransistor*>(c));
        break;
    case ElectronicComponent::Type::IntegratedCircuit:
        _addIntegratedCircuit(ID, *static_cast<const IntegratedCircuit*>(c));
        break;
    }
}

std::unique_ptr<ElectronicComponent>
SQLiteDatabase::_getAdditionalComponentProperties(
    ComponentID ID,
    ElectronicComponent::BaseConfig config,
    ElectronicComponent::Type type
) {
    switch(type) {
    case ElectronicComponent::Type::Resistor:
        return _getResistor(ID, config);
    case ElectronicComponent::Type::Capacitor:
        return _getCapacitor(ID, config);
    case ElectronicComponent::Type::Inductor:
        return _getInductor(ID, config);
    case ElectronicComponent::Type::Diode:
        return _getDiode(ID, config);
    case ElectronicComponent::Type::BJTransistor:
        return _getBJTransistor(ID, config);
    case ElectronicComponent::Type::FETransistor:
        return _getFETransistor(ID, config);
    case ElectronicComponent::Type::IntegratedCircuit:
        return _getIntegratedCircuit(ID, config);
    }

    return nullptr;
}

void SQLiteDatabase::_editAdditionalComponentProperties(
    ComponentID ID,
    const ElectronicComponent& updatedComponent
) {
    const ElectronicComponent* c = &updatedComponent;

    switch(updatedComponent.type()){
    case ElectronicComponent::Type::Resistor:
        _editResistor(ID, *static_cast<const Resistor*>(c));
        break;
    case ElectronicComponent::Type::Capacitor:
        _editCapacitor(ID, *static_cast<const Capacitor*>(c));
        break;
    case ElectronicComponent::Type::Inductor:
        _editInductor(ID, *static_cast<const Inductor*>(c));
        break;
    case ElectronicComponent::Type::Diode:
        _editDiode(ID, *static_cast<const Diode*>(c));
        break;
    case ElectronicComponent::Type::BJTransistor:
        _editBJTransistor(ID, *static_cast<const BJTransistor*>(c));
        break;
    case ElectronicComponent::Type::FETransistor:
        _editFETransistor(ID, *static_cast<const FETransistor*>(c));
        break;
    case ElectronicComponent::Type::IntegratedCircuit:
        _editIntegratedCircuit(ID, *static_cast<const IntegratedCircuit*>(c));
        break;
    }
}

void SQLiteDatabase::_addResistor(ComponentID ID, const Resistor& newResistor) {
    int ret;

    sqlite3_reset(m_accessors.addResistor);

    sqlite3_bind_int64(
        m_accessors.addResistor,
        1,
        ID
    );

    sqlite3_bind_double(
        m_accessors.addResistor,
        2,
        newResistor.resistance()
    );

    sqlite3_bind_double(
        m_accessors.addResistor,
        3,
        newResistor.toleranceBand()
    );

    ret = sqlite3_step(m_accessors.addResistor);

    _checkResultCode(ret, "Unable to add resistor properties");
}

void SQLiteDatabase::_addCapacitor(ComponentID ID, const Capacitor& newCapacitor) {
    int ret;

    sqlite3_reset(m_accessors.addCapacitor);

    sqlite3_bind_int64(
        m_accessors.addCapacitor,
        1,
        ID
    );

    sqlite3_bind_int(
        m_accessors.addCapacitor,
        2,
        static_cast<int>(newCapacitor.type())
    );

    sqlite3_bind_double(
        m_accessors.addCapacitor,
        3,
        newCapacitor.capacitance()
    );

    ret = sqlite3_step(m_accessors.addCapacitor);

    _checkResultCode(ret, "Unable to add resistor properties");
}

void SQLiteDatabase::_addInductor(ComponentID ID, const Inductor& newInductor) {
    int ret;

    sqlite3_reset(m_accessors.addInductor);

    sqlite3_bind_int64(
        m_accessors.addInductor,
        1,
        ID
    );

    sqlite3_bind_double(
        m_accessors.addInductor,
        2,
        newInductor.inductance()
    );

    ret = sqlite3_step(m_accessors.addInductor);

    _checkResultCode(ret, "Unable to add inductor properties");
}

void SQLiteDatabase::_addDiode(ComponentID ID, const Diode& newDiode) {
    int ret;

    sqlite3_reset(m_accessors.addDiode);

    sqlite3_bind_int64(
        m_accessors.addDiode,
        1,
        ID
    );

    sqlite3_bind_int(
        m_accessors.addDiode,
        2,
        static_cast<int>(newDiode.diodeType())
    );

    sqlite3_bind_double(
        m_accessors.addDiode,
        3,
        newDiode.forwardVoltage()
    );

    ret = sqlite3_step(m_accessors.addDiode);

    _checkResultCode(ret, "Unable to add diode properties");
}

void SQLiteDatabase::_addBJTransistor(ComponentID ID, const BJTransistor& newTransistor) {
    int ret;

    sqlite3_reset(m_accessors.addBJTransistor);

    sqlite3_bind_int64(
        m_accessors.addBJTransistor,
        1,
        ID
    );

    sqlite3_bind_double(
        m_accessors.addBJTransistor,
        2,
        newTransistor.gain()
    );

    ret = sqlite3_step(m_accessors.addBJTransistor);

    _checkResultCode(ret, "Unable to add BJTransistor properties");
}

void SQLiteDatabase::_addFETransistor(ComponentID ID, const FETransistor& newTransistor) {
    int ret;

    sqlite3_reset(m_accessors.addFETransistor);

    sqlite3_bind_int64(
        m_accessors.addFETransistor,
        1,
        ID
    );

    sqlite3_bind_double(
        m_accessors.addFETransistor,
        2,
        newTransistor.thresholdVoltage()
    );

    ret = sqlite3_step(m_accessors.addFETransistor);

    _checkResultCode(ret, "Unable to add FETransistor properties");
}

void SQLiteDatabase::_addIntegratedCircuit(ComponentID ID, const IntegratedCircuit& newChip) {
    int ret;

    sqlite3_reset(m_accessors.addIntegratedCircuit);

    sqlite3_bind_int64(
        m_accessors.addIntegratedCircuit,
        1,
        ID
    );

    sqlite3_bind_int(
        m_accessors.addIntegratedCircuit,
        2,
        newChip.pinCount()
    );

    sqlite3_bind_double(
        m_accessors.addIntegratedCircuit,
        3,
        newChip.width()
    );

    sqlite3_bind_double(
        m_accessors.addIntegratedCircuit,
        4,
        newChip.height()
    );

    sqlite3_bind_double(
        m_accessors.addIntegratedCircuit,
        5,
        newChip.length()
    );

    ret = sqlite3_step(m_accessors.addIntegratedCircuit);

    _checkResultCode(ret, "Unable to add IntegratedCircuit properties");
}

std::unique_ptr<ElectronicComponent>
SQLiteDatabase::_getResistor(ComponentID ID, const ElectronicComponent::BaseConfig& config) {
    int ret;
    double resistance;
    double toleranceBand;

    sqlite3_reset(m_accessors.getResistor);
    sqlite3_bind_int64(m_accessors.getResistor, 1, ID);

    ret = sqlite3_step(m_accessors.getResistor);

    _checkResultCode(ret, "Unable to get resistor");

    if(ret != SQLITE_ROW) {
        printf("[Warning]: Did not find resistor properties for component %llu. The database may be corrupted",
            static_cast<unsigned long long>(ID));
        return nullptr;
    }

    resistance = sqlite3_column_double(m_accessors.getResistor, 2);
    toleranceBand = sqlite3_column_double(m_accessors.getResistor, 3);

    return std::make_unique<Resistor>(config, resistance, toleranceBand);
}

std::unique_ptr<ElectronicComponent>
SQLiteDatabase::_getCapacitor(ComponentID ID, const ElectronicComponent::BaseConfig& config) {
    int ret;
    Capacitor::Type type;
    double capacitance;

    sqlite3_reset(m_accessors.getCapacitor);
    sqlite3_bind_int64(m_accessors.getCapacitor, 1, ID);

    ret = sqlite3_step(m_accessors.getCapacitor);

    _checkResultCode(ret, "Unable to get capacitor");

    if(ret != SQLITE_ROW) {
        printf("[Warning]: Did not find capacitor properties for component %llu. The database may be corrupted",
            static_cast<unsigned long long>(ID));
        return nullptr;
    }

    type = static_cast<Capacitor::Type>(
        sqlite3_column_int(m_accessors.getCapacitor, 2)
    );

    capacitance = sqlite3_column_double(m_accessors.getCapacitor, 3);

    return std::make_unique<Capacitor>(config, type, capacitance);
}

std::unique_ptr<ElectronicComponent>
SQLiteDatabase::_getInductor(ComponentID ID, const ElectronicComponent::BaseConfig& config) {
    int ret;
    double inductance;

    sqlite3_reset(m_accessors.getInductor);
    sqlite3_bind_int64(m_accessors.getInductor, 1, ID);

    ret = sqlite3_step(m_accessors.getInductor);

    _checkResultCode(ret, "Unable to get inductor");

    if(ret != SQLITE_ROW) {
        printf("[Warning]: Did not find inductor properties for component %llu. The database may be corrupted",
            static_cast<unsigned long long>(ID));
        return nullptr;
    }

    inductance = sqlite3_column_double(m_accessors.getInductor, 2);

    return std::make_unique<Inductor>(config, inductance);
}

std::unique_ptr<ElectronicComponent>
SQLiteDatabase::_getDiode(ComponentID ID, const ElectronicComponent::BaseConfig& config) {
    int ret;
    Diode::Type type;
    double forwardVoltage;

    sqlite3_reset(m_accessors.getDiode);
    sqlite3_bind_int64(m_accessors.getDiode, 1, ID);

    ret = sqlite3_step(m_accessors.getDiode);

    _checkResultCode(ret, "Unable to get diode");

    if(ret != SQLITE_ROW) {
        printf("[Warning]: Did not find diode properties for component %llu. The database may be corrupted",
            static_cast<unsigned long long>(ID));
        return nullptr;
    }

    type = static_cast<Diode::Type>(
        sqlite3_column_int(m_accessors.getDiode, 2)
    );

    forwardVoltage = sqlite3_column_double(m_accessors.getDiode, 3);

    return std::make_unique<Diode>(config, forwardVoltage, type);
}

std::unique_ptr<ElectronicComponent>
SQLiteDatabase::_getBJTransistor(ComponentID ID, const ElectronicComponent::BaseConfig& config) {
    int ret;
    double gain;

    sqlite3_reset(m_accessors.getBJTransistor);
    sqlite3_bind_int64(m_accessors.getBJTransistor, 1, ID);

    ret = sqlite3_step(m_accessors.getBJTransistor);

    _checkResultCode(ret, "Unable to get BJTransistor");

    if(ret != SQLITE_ROW) {
        printf("[Warning]: Did not find BJTransistor properties for component %llu. The database may be corrupted",
            static_cast<unsigned long long>(ID));
        return nullptr;
    }

    gain = sqlite3_column_double(m_accessors.getBJTransistor, 2);

    return std::make_unique<BJTransistor>(config, gain);
}

std::unique_ptr<ElectronicComponent>
SQLiteDatabase::_getFETransistor(ComponentID ID, const ElectronicComponent::BaseConfig& config) {
    int ret;
    double thresholdVoltage;

    sqlite3_reset(m_accessors.getFETransistor);
    sqlite3_bind_int64(m_accessors.getFETransistor, 1, ID);

    ret = sqlite3_step(m_accessors.getFETransistor);

    _checkResultCode(ret, "Unable to get FETransistor");

    if(ret != SQLITE_ROW) {
        printf("[Warning]: Did not find FETransistor properties for component %llu. The database may be corrupted",
            static_cast<unsigned long long>(ID));
        return nullptr;
    }

    thresholdVoltage = sqlite3_column_double(m_accessors.getFETransistor, 2);

    return std::make_unique<FETransistor>(config, thresholdVoltage);
}

std::unique_ptr<ElectronicComponent>
SQLiteDatabase::_getIntegratedCircuit(ComponentID ID, const ElectronicComponent::BaseConfig& config) {
    int ret;
    size_t pinCount;
    double width, height, length;

    sqlite3_reset(m_accessors.getIntegratedCircuit);
    sqlite3_bind_int64(m_accessors.getIntegratedCircuit, 1, ID);

    ret = sqlite3_step(m_accessors.getIntegratedCircuit);

    _checkResultCode(ret, "Unable to get IntegratedCircuit");

    if(ret != SQLITE_ROW) {
        printf("[Warning]: Did not find IntegratedCircuit properties for component %llu. The database may be corrupted",
            static_cast<unsigned long long>(ID));
        return nullptr;
    }

    pinCount = sqlite3_column_int(m_accessors.getIntegratedCircuit, 2);
    width = sqlite3_column_double(m_accessors.getIntegratedCircuit, 3);
    height = sqlite3_column_double(m_accessors.getIntegratedCircuit, 4);
    length = sqlite3_column_double(m_accessors.getIntegratedCircuit, 5);

    return std::make_unique<IntegratedCircuit>(config, pinCount, width, height, length);
}

void SQLiteDatabase::_editResistor(ComponentID ID, const Resistor& updatedResistor) {
    int ret;

    sqlite3_reset(m_accessors.editResistor);

    sqlite3_bind_double(
        m_accessors.editResistor,
        1,
        updatedResistor.resistance()
    );

    sqlite3_bind_double(
        m_accessors.editResistor,
        2,
        updatedResistor.toleranceBand()
    );

    sqlite3_bind_int64(
        m_accessors.editResistor,
        3,
        ID
    );

    ret = sqlite3_step(m_accessors.editResistor);

    _checkResultCode(ret, "Unable to edit resistor properties");
}

void SQLiteDatabase::_editCapacitor(ComponentID ID, const Capacitor& updatedCapacitor) {
    int ret;

    sqlite3_reset(m_accessors.editCapacitor);

    sqlite3_bind_int(
        m_accessors.editCapacitor,
        1,
        static_cast<int>(updatedCapacitor.capacitorType())
    );

    sqlite3_bind_double(
        m_accessors.editCapacitor,
        2,
        updatedCapacitor.capacitance()
    );

    sqlite3_bind_int64(
        m_accessors.editCapacitor,
        3,
        ID
    );

    ret = sqlite3_step(m_accessors.editCapacitor);

    _checkResultCode(ret, "Unable to edit capacitor properties");
}

void SQLiteDatabase::_editInductor(ComponentID ID, const Inductor& updatedInductor) {
    int ret;

    sqlite3_reset(m_accessors.editInductor);

    sqlite3_bind_double(
        m_accessors.editInductor,
        1,
        updatedInductor.inductance()
    );

    sqlite3_bind_int64(
        m_accessors.editInductor,
        2,
        ID
    );

    ret = sqlite3_step(m_accessors.editInductor);

    _checkResultCode(ret, "Unable to edit inductor properties");
}

void SQLiteDatabase::_editDiode(ComponentID ID, const Diode& updatedDiode) {
    int ret;

    sqlite3_reset(m_accessors.editDiode);

    sqlite3_bind_int(
        m_accessors.editDiode,
        1,
        static_cast<int>(updatedDiode.diodeType())
    );

    sqlite3_bind_double(
        m_accessors.editDiode,
        2,
        updatedDiode.forwardVoltage()
    );

    sqlite3_bind_int64(
        m_accessors.editDiode,
        3,
        ID
    );

    ret = sqlite3_step(m_accessors.editDiode);

    _checkResultCode(ret, "Unable to edit diode properties");
}

void SQLiteDatabase::_editBJTransistor(ComponentID ID, const BJTransistor& updatedTransistor) {
    int ret;

    sqlite3_reset(m_accessors.editBJTransistor);

    sqlite3_bind_double(
        m_accessors.editBJTransistor,
        1,
        updatedTransistor.gain()
    );

    sqlite3_bind_int64(
        m_accessors.editBJTransistor,
        2,
        ID
    );

    ret = sqlite3_step(m_accessors.editBJTransistor);

    _checkResultCode(ret, "Unable to edit BJTransistor properties");
}

void SQLiteDatabase::_editFETransistor(ComponentID ID, const FETransistor& updatedTransistor) {
    int ret;

    sqlite3_reset(m_accessors.editFETransistor);

    sqlite3_bind_double(
        m_accessors.editFETransistor,
        1,
        updatedTransistor.thresholdVoltage()
    );

    sqlite3_bind_int64(
        m_accessors.editFETransistor,
        2,
        ID
    );

    ret = sqlite3_step(m_accessors.editFETransistor);

    _checkResultCode(ret, "Unable to edit FETransistor properties");
}

void SQLiteDatabase::_editIntegratedCircuit(ComponentID ID, const IntegratedCircuit& updatedChip) {
    int ret;

    sqlite3_reset(m_accessors.editIntegratedCircuit);

    sqlite3_bind_int(
        m_accessors.editIntegratedCircuit,
        1,
        updatedChip.pinCount()
    );

    sqlite3_bind_double(
        m_accessors.editIntegratedCircuit,
        2,
        updatedChip.width()
    );

    sqlite3_bind_double(
        m_accessors.editIntegratedCircuit,
        3,
        updatedChip.height()
    );

    sqlite3_bind_double(
        m_accessors.editIntegratedCircuit,
        4,
        updatedChip.length()
    );

    sqlite3_bind_int64(
        m_accessors.editIntegratedCircuit,
        5,
        ID
    );

    ret = sqlite3_step(m_accessors.editIntegratedCircuit);

    _checkResultCode(ret, "Unable to edit IntegratedCircuit properties");
}

MassQueryResult SQLiteDatabase::_getAllComponents(
    const MassQueryConfig& config,
    std::optional<ElectronicComponent::Type> type
) {
    MassQueryResult result;
    int ret;

    _checkInitialization();

    _getStatistics(config, type, result.numPages, result.totalNumItems);

    result.currentPage = config.pagination.has_value() ?
        config.pagination.value().pageNumber : 1;

    result.currentPage = std::min(result.numPages, result.currentPage);

    if(!config.pagination.has_value() || result.totalNumItems == 0)
        result.numItems = result.totalNumItems;
    else {
        size_t itemsPerPage = config.pagination.value().itemsPerPage;
        result.numItems = result.totalNumItems - (result.currentPage - 1) * itemsPerPage;
        result.numItems = std::min(itemsPerPage, result.numItems);
    }

    if(result.totalNumItems == 0 || config.statisticsOnly)
        return result;

    sqlite3_stmt* accessor = _getMassQueryAccessor(config, type);
    sqlite3_reset(accessor);

    _applyMassQueryBindings(accessor, config, result.currentPage - 1);

    std::vector<ComponentBase> bases;
    BatchesCount amounts = {};
    bases.reserve(result.numItems);
    for(int i = 0;;i++) {
        ret = sqlite3_step(accessor);

        _checkResultCode(ret, "Unable to get electrical component");

        if(ret != SQLITE_ROW)
            break;

        bases.push_back({});

        bases[i].ID = sqlite3_column_int64(accessor, 0);

        bases[i].config.name = reinterpret_cast<const char*>(
            sqlite3_column_text(accessor, 1)
        );

        bases[i].type = static_cast<ElectronicComponent::Type>(
            sqlite3_column_int(accessor, 2)
        );

        bases[i].config.manufacturer = reinterpret_cast<const char*>(
            sqlite3_column_text(accessor, 3)
        );

        if(sqlite3_column_type(accessor, 4) == SQLITE_TEXT)
            bases[i].config.partNumber = reinterpret_cast<const char*>(
               sqlite3_column_text(accessor, 4)
            );

        if(sqlite3_column_type(accessor, 5) == SQLITE_TEXT)
            bases[i].config.description = reinterpret_cast<const char*>(
                sqlite3_column_text(accessor, 5)
            );

        bases[i].config.quantity = sqlite3_column_int(accessor, 6);

        bases[i].config.rating.voltage = sqlite3_column_double(accessor, 7);
        bases[i].config.rating.current = sqlite3_column_double(accessor, 8);
        bases[i].config.rating.power = sqlite3_column_double(accessor, 9);

        bases[i].next = i + 1;

        switch(bases[i].type) {
        case ElectronicComponent::Type::Resistor:
            amounts.numResistors++;
            break;
        case ElectronicComponent::Type::Capacitor:
            amounts.numCapacitors++;
            break;
        case ElectronicComponent::Type::Inductor:
            amounts.numInductors++;
            break;
        case ElectronicComponent::Type::Diode:
            amounts.numDiodes++;
            break;
        case ElectronicComponent::Type::BJTransistor:
            amounts.numBJTs++;
            break;
        case ElectronicComponent::Type::FETransistor:
            amounts.numFETs++;
            break;
        case ElectronicComponent::Type::IntegratedCircuit:
            amounts.numChips++;
        }

        if(bases[i].next == result.numItems)
            bases[i].next = 0; // Zero signifies that it is at the end of list
    }

    if(bases.size() != result.numItems)
        _throwError("The number of items expected does not equal the number of items returned from sqlite");

    // Perform simple fetching per row if there isn't too many items to process
    if(bases.size() < SHOULD_BATCH_THRESHOLD) {
        for(ComponentBase& base : bases)
            result.items.push_back(
                _getAdditionalComponentProperties(base.ID, base.config, base.type));

        return result;
    }

    // Perform batch fetching if there are many items to process
    _getAdditionalComponentPropertiesInBatches(bases, amounts, result.items);

    return result;
}

void SQLiteDatabase::_getStatistics(
    const MassQueryConfig& config,
    std::optional<ElectronicComponent::Type> type,
    size_t& totalPages,
    size_t& totalItems
) {
    int ret;

    MassQueryConfig cfgForStats = config;
    cfgForStats.statisticsOnly = true;
    cfgForStats.pagination.reset();

    sqlite3_stmt* accessor = _getMassQueryAccessor(cfgForStats, type);

    sqlite3_reset(accessor);

    _applyMassQueryBindings(accessor, cfgForStats);

    ret = sqlite3_step(accessor);

    _checkResultCode(ret, "Unable to get number of pages");

    if(ret != SQLITE_ROW) {
        printf("[Warning]: Unable to get number of pages, but sqlite did not throw an error; unknown behavior");
        totalPages = totalItems = 0;
    }

    totalItems = sqlite3_column_int(accessor, 0);

    if(config.pagination.has_value()){
        int itemsPerPage = config.pagination.value().itemsPerPage;

        totalPages = totalItems / itemsPerPage;
    }
    else
        totalPages = totalItems != 0 ? 1 : 0;
}

sqlite3_stmt* SQLiteDatabase::_getMassQueryAccessor(
    const MassQueryConfig& config,
    std::optional<ElectronicComponent::Type> type
) {
    uint64_t key = config.hash();
    s_combineHash(key, type);

    auto cachedAccessor = m_massQueryAccessorsCache.find(key);

    if(cachedAccessor != m_massQueryAccessorsCache.end())
        return cachedAccessor->second;

    std::string sqlStatement = s_genSQLStatement4MassQuery(config, type);
    sqlite3_stmt* newAccessor;

    int ret = sqlite3_prepare_v2(m_db, sqlStatement.c_str(), sqlStatement.length(), &newAccessor, nullptr);

    if(ret != SQLITE_OK)
        _throwError("Failed to prepare mass query accessor SQL statement", sqlStatement.c_str());

    m_massQueryAccessorsCache[key] = newAccessor;

    return newAccessor;
}

sqlite3_stmt* SQLiteDatabase::_getBatchAccessor(ElectronicComponent::Type type, int& batchSize) {
    uint64_t batchAccessorID = static_cast<uint64_t>(type) << 32;

    batchSize = s_chooseBestBatchSize(batchSize);

    batchAccessorID |= batchSize;

    auto cachedAccessor = m_batchAccessorsCache.find(batchAccessorID);

    if(cachedAccessor != m_batchAccessorsCache.end())
        return cachedAccessor->second;

    std::string sqlStatement = s_genSQLStatement4Batches(type, batchSize);
    sqlite3_stmt* newAccessor;

    int ret = sqlite3_prepare_v2(m_db, sqlStatement.c_str(), sqlStatement.length(), &newAccessor, nullptr);

    if(ret != SQLITE_OK)
        _throwError("Failed to prepare batch query accessor SQL statement", sqlStatement.c_str());

    m_batchAccessorsCache[batchAccessorID] = newAccessor;

    return newAccessor;
}

void SQLiteDatabase::_applyMassQueryBindings(sqlite3_stmt* accessor, const MassQueryConfig& config, size_t pageIndex) {
    sqlite3_clear_bindings(accessor);

    int paramIndex = 1;

    if(config.pagination.has_value()) {
        size_t itemsPerPage = config.pagination.value().itemsPerPage;
        sqlite3_bind_int(accessor, paramIndex++, itemsPerPage);
        sqlite3_bind_int(accessor, paramIndex++, pageIndex * itemsPerPage);
    }
}

void SQLiteDatabase::_getAdditionalComponentPropertiesInBatches(
    std::vector<ComponentBase>& bases,
    BatchesCount amounts,
    std::vector<std::unique_ptr<ElectronicComponent>>& items
) {
    int headIndex = 0;
    int offset = 0;

    if(bases.size() == 0)
        return;

    while(amounts.numResistors) {
        int batchSize = amounts.numResistors;
        _getResistorsInBatches(bases, batchSize, headIndex, offset, items);

        if(amounts.numResistors <= batchSize)
            break;

        amounts.numResistors -= batchSize;
    }

    offset = 0;
    while(amounts.numCapacitors) {
        int batchSize = amounts.numCapacitors;
        _getCapacitorsInBatches(bases, batchSize, headIndex, offset, items);

        if(amounts.numCapacitors <= batchSize)
            break;

        amounts.numCapacitors -= batchSize;
    }

    offset = 0;
    while(amounts.numInductors) {
        int batchSize = amounts.numInductors;
        _getInductorsInBatches(bases, batchSize, headIndex, offset, items);

        if(amounts.numInductors <= batchSize)
            break;

        amounts.numInductors -= batchSize;
    }

    offset = 0;
    while(amounts.numDiodes) {
        int batchSize = amounts.numDiodes;
        _getDiodesInBatches(bases, batchSize, headIndex, offset, items);

        if(amounts.numDiodes <= batchSize)
            break;

        amounts.numDiodes -= batchSize;
    }

    offset = 0;
    while(amounts.numBJTs) {
        int batchSize = amounts.numBJTs;
        _getBJTransistorsInBatches(bases, batchSize, headIndex, offset, items);

        if(amounts.numBJTs <= batchSize)
            break;

        amounts.numBJTs -= batchSize;
    }

    offset = 0;
    while(amounts.numFETs) {
        int batchSize = amounts.numFETs;
        _getFETransistorsInBatches(bases, batchSize, headIndex, offset, items);

        if(amounts.numFETs <= batchSize)
            break;

        amounts.numFETs -= batchSize;
    }

    offset = 0;
    while(amounts.numChips) {
        int batchSize = amounts.numChips;
        _getIntegratedCircuitsInBatches(bases, batchSize, headIndex, offset, items);

        if(amounts.numChips <= batchSize)
            break;

        amounts.numChips -= batchSize;
    }
}

void SQLiteDatabase::_bindIdsForBatching(
    ElectronicComponent::Type type,
    sqlite3_stmt* accessor,
    std::vector<ComponentBase>& bases,
    int batchSize,
    int& headIndex,
    int& offset
) {
    sqlite3_clear_bindings(accessor);
    m_baseIndexQueue.clear();

    size_t i = headIndex + offset;
    size_t prevI = headIndex + offset;
    size_t paramIndex = 0;
    do {
        if(paramIndex >= batchSize)
            break;

        if(bases[i].type != type)
            goto skip;

        sqlite3_bind_int64(accessor, paramIndex + 1, bases[i].ID);
        m_baseIndexQueue.emplace_back(i);

        if(i == prevI) { // Flag first element in linked-list as removed
            offset -= bases[i].next - headIndex;
            headIndex = bases[i].next;
        } else // Flag any other element as removed
            bases[prevI].next = bases[i].next;
skip:
        prevI = i;
        i = bases[i].next;
        paramIndex++;
        offset++;
    } while(i != 0);
}

void SQLiteDatabase::_getResistorsInBatches(
    std::vector<ComponentBase>& bases,
    int& batchSize,
    int& headIndex,
    int& offset,
    std::vector<std::unique_ptr<ElectronicComponent>>& items
) {
    sqlite3_stmt* accessor = _getBatchAccessor(ElectronicComponent::Type::Resistor, batchSize);

    sqlite3_reset(accessor);

    _bindIdsForBatching(ElectronicComponent::Type::Resistor, accessor, bases, batchSize, headIndex, offset);

    for(size_t i = 0;;i++) {
        int ret = sqlite3_step(accessor);

        _checkResultCode(ret, "Failed to get resistors from a batch");

        if(ret != SQLITE_ROW)
            break;

        double resistance = sqlite3_column_double(accessor, 2);
        double toleranceBand = sqlite3_column_double(accessor, 3);

        size_t baseIndex = m_baseIndexQueue[i];
        items.push_back(
            std::make_unique<Resistor>(bases[baseIndex].config, resistance, toleranceBand));
    }
}

void SQLiteDatabase::_getCapacitorsInBatches(
    std::vector<ComponentBase>& bases,
    int& batchSize,
    int& headIndex,
    int& offset,
    std::vector<std::unique_ptr<ElectronicComponent>>& items
) {
    sqlite3_stmt* accessor = _getBatchAccessor(ElectronicComponent::Type::Capacitor, batchSize);

    sqlite3_reset(accessor);

    _bindIdsForBatching(ElectronicComponent::Type::Capacitor, accessor, bases, batchSize, headIndex, offset);

    for(size_t i = 0;;i++) {
        int ret = sqlite3_step(accessor);

        _checkResultCode(ret, "Failed to get capacitors from a batch");

        if(ret != SQLITE_ROW)
            break;

        Capacitor::Type type = static_cast<Capacitor::Type>(
            sqlite3_column_int(accessor, 2)
        );
        double capacitance = sqlite3_column_double(accessor, 3);

        size_t baseIndex = m_baseIndexQueue[i];
        items.push_back(
            std::make_unique<Capacitor>(bases[baseIndex].config, type, capacitance));
    }
}

void SQLiteDatabase::_getInductorsInBatches(
    std::vector<ComponentBase>& bases,
    int& batchSize,
    int& headIndex,
    int& offset,
    std::vector<std::unique_ptr<ElectronicComponent>>& items
) {
    sqlite3_stmt* accessor = _getBatchAccessor(ElectronicComponent::Type::Inductor, batchSize);

    sqlite3_reset(accessor);

    _bindIdsForBatching(ElectronicComponent::Type::Inductor, accessor, bases, batchSize, headIndex, offset);

    for(size_t i = 0;;i++) {
        int ret = sqlite3_step(accessor);

        _checkResultCode(ret, "Failed to get inductors from a batch");

        if(ret != SQLITE_ROW)
            break;

        double inductance = sqlite3_column_double(accessor, 2);

        size_t baseIndex = m_baseIndexQueue[i];
        items.push_back(
            std::make_unique<Inductor>(bases[baseIndex].config, inductance));
    }
}

void SQLiteDatabase::_getDiodesInBatches(
    std::vector<ComponentBase>& bases,
    int& batchSize,
    int& headIndex,
    int& offset,
    std::vector<std::unique_ptr<ElectronicComponent>>& items
) {
    sqlite3_stmt* accessor = _getBatchAccessor(ElectronicComponent::Type::Diode, batchSize);

    sqlite3_reset(accessor);

    _bindIdsForBatching(ElectronicComponent::Type::Diode, accessor, bases, batchSize, headIndex, offset);

    for(size_t i = 0;;i++) {
        int ret = sqlite3_step(accessor);

        _checkResultCode(ret, "Failed to get diodes from a batch");

        if(ret != SQLITE_ROW)
            break;

        Diode::Type type = static_cast<Diode::Type>(
            sqlite3_column_int(accessor, 2)
        );
        double forwardVoltage = sqlite3_column_double(accessor, 3);

        size_t baseIndex = m_baseIndexQueue[i];
        items.push_back(
            std::make_unique<Diode>(bases[baseIndex].config, forwardVoltage, type));
    }
}

void SQLiteDatabase::_getBJTransistorsInBatches(
    std::vector<ComponentBase>& bases,
    int& batchSize,
    int& headIndex,
    int& offset,
    std::vector<std::unique_ptr<ElectronicComponent>>& items
) {
    sqlite3_stmt* accessor = _getBatchAccessor(ElectronicComponent::Type::BJTransistor, batchSize);

    sqlite3_reset(accessor);

    _bindIdsForBatching(ElectronicComponent::Type::BJTransistor, accessor, bases, batchSize, headIndex, offset);

    for(size_t i = 0;;i++) {
        int ret = sqlite3_step(accessor);

        _checkResultCode(ret, "Failed to get BJTransistors from a batch");

        if(ret != SQLITE_ROW)
            break;

        double gain = sqlite3_column_double(accessor, 2);

        size_t baseIndex = m_baseIndexQueue[i];
        items.push_back(
            std::make_unique<BJTransistor>(bases[baseIndex].config, gain));
    }
}

void SQLiteDatabase::_getFETransistorsInBatches(
    std::vector<ComponentBase>& bases,
    int& batchSize,
    int& headIndex,
    int& offset,
    std::vector<std::unique_ptr<ElectronicComponent>>& items
) {
    sqlite3_stmt* accessor = _getBatchAccessor(ElectronicComponent::Type::FETransistor, batchSize);

    sqlite3_reset(accessor);

    _bindIdsForBatching(ElectronicComponent::Type::FETransistor, accessor, bases, batchSize, headIndex, offset);

    for(size_t i = 0;;i++) {
        int ret = sqlite3_step(accessor);

        _checkResultCode(ret, "Failed to get FETransistors from a batch");

        if(ret != SQLITE_ROW)
            break;

        double thresholdVoltage = sqlite3_column_double(accessor, 2);

        size_t baseIndex = m_baseIndexQueue[i];
        items.push_back(
            std::make_unique<FETransistor>(bases[baseIndex].config, thresholdVoltage));
    }
}

void SQLiteDatabase::_getIntegratedCircuitsInBatches(
    std::vector<ComponentBase>& bases,
    int& batchSize,
    int& headIndex,
    int& offset,
    std::vector<std::unique_ptr<ElectronicComponent>>& items
) {
    sqlite3_stmt* accessor = _getBatchAccessor(ElectronicComponent::Type::IntegratedCircuit, batchSize);

    sqlite3_reset(accessor);

    _bindIdsForBatching(ElectronicComponent::Type::IntegratedCircuit, accessor, bases, batchSize, headIndex, offset);

    for(size_t i = 0;;i++) {
        int ret = sqlite3_step(accessor);

        _checkResultCode(ret, "Failed to get IntegratedCircuits from a batch");

        if(ret != SQLITE_ROW)
            break;

        size_t pinCount = sqlite3_column_int(accessor, 2);
        double width = sqlite3_column_double(accessor, 3);
        double height = sqlite3_column_double(accessor, 4);
        double length = sqlite3_column_double(accessor, 5);

        size_t baseIndex = m_baseIndexQueue[i];
        items.push_back(
            std::make_unique<IntegratedCircuit>(bases[baseIndex].config, pinCount, width, height, length));
    }
}

static std::string s_genSQLStatement4MassQuery(const MassQueryConfig& config, std::optional<ElectronicComponent::Type> type) {
    std::string sql;
    sql.reserve(256);

    if(config.statisticsOnly)
        sql = "SELECT COUNT(*) FROM ElectronicComponents ";
    else
        sql = "SELECT * FROM ElectronicComponents ";

    if(type.has_value()) {
        sql += "WHERE Type = ";
        sql += std::to_string(static_cast<int>(type.value()));
        sql += ' ';
    }

    if(config.pagination.has_value())
        sql += "LIMIT ? OFFSET ? ";

    return sql;
}

static std::string s_genSQLStatement4Batches(ElectronicComponent::Type type, int batchSize) {
    std::string sql;
    sql.reserve(256);

    sql = "SELECT * FROM ";

    switch(type){
    case ElectronicComponent::Type::Resistor:
        sql += "Resistors ";
        break;
    case ElectronicComponent::Type::Capacitor:
        sql += "Capacitors ";
        break;
    case ElectronicComponent::Type::Inductor:
        sql += "Inductors ";
        break;
    case ElectronicComponent::Type::Diode:
        sql += "Diodes ";
        break;
    case ElectronicComponent::Type::BJTransistor:
        sql += "BJTransistors ";
        break;
    case ElectronicComponent::Type::FETransistor:
        sql += "FETransistors ";
        break;
    case ElectronicComponent::Type::IntegratedCircuit:
        sql += "IntegratedCircuits ";
    }

    sql += "WHERE ComponentID IN (";

    for(int i = 1; i < batchSize; i++)
        sql += "?, ";

    sql += "?)";

    return sql;
}

static int s_chooseBestBatchSize(int sizeHint) {
    if(sizeHint < MIN_BATCH_SIZE)
        return MIN_BATCH_SIZE;

    if(MAX_BATCH_SIZE < sizeHint)
        return MAX_BATCH_SIZE;

    return s_floorPowerOf2(sizeHint);
}

static int s_floorPowerOf2(int x) {
    if (x == 0) return 0;

    if ((x & (x - 1)) == 0) return x;

    int v = x;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return (v >> 1) + 1;
}
