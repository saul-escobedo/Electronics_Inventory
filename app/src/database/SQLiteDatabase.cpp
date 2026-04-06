#include "database/SQLiteDatabase.hpp"
#include "database/EmbeddedSQLCode.hpp"
#include "database/exceptions/DatabaseException.hpp"
#include "database/exceptions/Exceptions.hpp"
#include "electrical/ElectronicComponent.hpp"
#include "electrical/Resistor.hpp"

#include <cstddef>

using namespace ecim;

#define DB_BACKEND_NAME "sqlite3"

static const std::pair<int, const char*> ACCESSORS_CODE[] = {
    { offsetof(SQLAccessors, addComponent), DBSQL_ADD_COMPONENT },
    { offsetof(SQLAccessors, addResistor), DBSQL_ADD_RESISTOR },
    { offsetof(SQLAccessors, addCapacitor), DBSQL_ADD_CAPACITOR },
    { offsetof(SQLAccessors, addInductor), DBSQL_ADD_INDUCTOR },
    { offsetof(SQLAccessors, addDiode), DBSQL_ADD_DIODE },
    { offsetof(SQLAccessors, addBJTransistor), DBSQL_ADD_BJ_TRANSISTOR },
    { offsetof(SQLAccessors, addFETransistor), DBSQL_ADD_FE_TRANSISTOR },
    { offsetof(SQLAccessors, addIntegratedCircuit), DBSQL_ADD_INTEGRATED_CIRCUIT }
};

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

    for(;;) {
        ret = sqlite3_step(m_accessors.addComponent);

        _checkResultCode(ret, "Unable to add electrical component");

        if(ret == SQLITE_DONE) break;
    }

    ComponentID newID = sqlite3_last_insert_rowid(m_db);

    if(newID == 0)
        throw DatabaseException(DB_BACKEND_NAME ": Unable to add electrical component, ComponentID should not be zero");

    _addAdditionalComponentProperties(newID, newComponent);

    return newID;
}

std::unique_ptr<ElectronicComponent> SQLiteDatabase::removeComponent(
    ComponentID id
) {
    _checkInitialization();

    return nullptr;
}

void SQLiteDatabase::editComponent(
    ComponentID id,
    const ElectronicComponent& updatedComponent
) {
    _checkInitialization();

}

std::unique_ptr<ElectronicComponent> SQLiteDatabase::getComponent(
    ComponentID id
) {
    _checkInitialization();

    return nullptr;
}

MassQueryResult SQLiteDatabase::getAllComponents(
    const MassQueryConfig& queryConfig
) {
    _checkInitialization();

    return MassQueryResult();
}

MassQueryResult SQLiteDatabase::getAllComponentsByType(
    ElectronicComponent::Type type,
    const MassQueryConfig& queryConfig
) {
    _checkInitialization();

    return MassQueryResult();
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

        ret = sqlite3_prepare(m_db, accessor.second, -1, a, nullptr);

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
}

void SQLiteDatabase::_addAdditionalComponentProperties(ComponentID ID, const ElectronicComponent& newComponent) {
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

    for(;;) {
        ret = sqlite3_step(m_accessors.addResistor);

        _checkResultCode(ret, "Unable to add resistor properties");

        if(ret == SQLITE_DONE) break;
    }
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

    for(;;) {
        ret = sqlite3_step(m_accessors.addCapacitor);

        _checkResultCode(ret, "Unable to add resistor properties");

        if(ret == SQLITE_DONE) break;
    }
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

    for(;;) {
        ret = sqlite3_step(m_accessors.addInductor);

        _checkResultCode(ret, "Unable to add inductor properties");

        if(ret == SQLITE_DONE) break;
    }
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

    for(;;) {
        ret = sqlite3_step(m_accessors.addDiode);

        _checkResultCode(ret, "Unable to add diode properties");

        if(ret == SQLITE_DONE) break;
    }
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

    for(;;) {
        ret = sqlite3_step(m_accessors.addBJTransistor);

        _checkResultCode(ret, "Unable to add BJTransistor properties");

        if(ret == SQLITE_DONE) break;
    }
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

    for(;;) {
        ret = sqlite3_step(m_accessors.addFETransistor);

        _checkResultCode(ret, "Unable to add FETransistor properties");

        if(ret == SQLITE_DONE) break;
    }
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

    for(;;) {
        ret = sqlite3_step(m_accessors.addIntegratedCircuit);

        _checkResultCode(ret, "Unable to add IntegratedCircuit properties");

        if(ret == SQLITE_DONE) break;
    }
}
