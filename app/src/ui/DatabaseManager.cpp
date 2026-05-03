#include "ui/DatabaseManager.hpp"

#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include "database/exceptions/Exceptions.hpp"

using namespace ecim;

DatabaseManager::DatabaseManager()
    : m_db(nullptr), m_initialized(false)
{
    // Read db path from settings
    QSettings settings("MyCompany", "InventoryApp");

    QString folderPath = settings.value(
        "dbPath",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                     ).toString();

    QDir().mkpath(folderPath);  // Ensure folder exists

    m_dbPath = folderPath + "/inventory.db";

    qDebug() << "DB Path: " << m_dbPath;
}

DatabaseManager::~DatabaseManager() {
    if (m_db && m_initialized)
        m_db->shutdown();
}

bool DatabaseManager::openDatabase() {
    try {
        if (!m_db) {
            m_db = std::make_unique<ecim::SQLiteDatabase>(m_dbPath.toStdString());
            m_db->initialize();
            m_initialized = true;
        }
        return true;
    } catch (const ecim::DatabaseException& e) {
        qDebug() << "Error opening database: " << e.what();
        return false;
    }
}

void DatabaseManager::reopenDatabase() {
    if (m_db && m_initialized) {
        m_db->shutdown();
        m_initialized = false;
    }

    QSettings settings("MyCompany", "InventoryApp");

    QString folderPath = settings.value("dbPath").toString();
    m_dbPath = folderPath + "/inventory.db";

    if (!openDatabase())
        qDebug() << "Failed to reopen DB";
    else
        qDebug() << "DB reopened at: " << m_dbPath;
}

bool DatabaseManager::moveDatabase(const QString &newFolder) {
    QString newPath = newFolder + "/inventory.db";

    if (m_dbPath == newPath) {
        return true;
    }

    // Ensure folder exists
    QDir().mkpath(newFolder);

    // Close db before moving
    if (m_db && m_initialized) {
        m_db->shutdown();
        m_initialized = false;
    }

    // If destination already exists, remove it
    if (QFile::exists(newPath)) {
        if (!QFile::remove(newPath)) {
            qDebug() << "Failed to remove existing DB at new path.";
            return false;
        }
    }

    // Copy file
    if (!QFile::copy(m_dbPath, newPath)) {
        qDebug() << "Failed to copy DB file";
        return false;
    }

    // Remove old file
    if (!QFile::remove(m_dbPath)) {
        qDebug() << "Failed to remove old DB file";
    }

    // Update path and reopen
    m_dbPath = newPath;

    // Update settings
    QSettings settings("MyCompany", "InventoryApp");
    settings.setValue("dbPath", newFolder);

    if (!openDatabase()) {
        qDebug() << "Failed to open DB at new path.";
        return false;
    }

    qDebug() << "Database moved to: " << newPath;
    return true;
}

QString DatabaseManager::getDatabasePath() const {
    return m_dbPath;
}

ComponentID DatabaseManager::addComponent(const ElectronicComponent& newComponent) {
    return m_db->addComponent(newComponent);
}

std::unique_ptr<ElectronicComponent> DatabaseManager::removeComponent(ComponentID id) {
    return m_db->removeComponent(id);
}

void DatabaseManager::editComponent(
    ComponentID id,
    const ElectronicComponent& updatedComponent
) {
    m_db->editComponent(id, updatedComponent);
}

std::unique_ptr<ElectronicComponent> DatabaseManager::getComponent(ComponentID id) {
    return m_db->getComponent(id);
}

MassQueryResult DatabaseManager::getAllComponents(
    const MassQueryConfig& queryConfig
) {
    return m_db->getAllComponents(queryConfig);
}

MassQueryResult DatabaseManager::getAllComponentsByType(
    ElectronicComponent::Type type,
    const MassQueryConfig& queryConfig
) {
    return m_db->getAllComponentsByType(type, queryConfig);
}
