#include "database/SQLiteDatabase.hpp"
#include "database/EmbeddedSQLCode.hpp"
#include "database/exceptions/UninitializedDatabaseException.hpp"
#include "database/exceptions/FailedInitializationException.hpp"

using namespace ecim;

#define DB_BACKEND_NAME "sqlite3"

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
}

void SQLiteDatabase::shutdown() {
    _checkInitialization();

    sqlite3_close(m_db);
}

ComponentID SQLiteDatabase::addComponent(
    const ElectronicComponent& newComponent
) {
    _checkInitialization();

    return 0;
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

void SQLiteDatabase::_checkInitialization() {
    if(!m_db)
        throw UninitializedDatabaseException(DB_BACKEND_NAME);
}
