#include "database.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include "database/exceptions/Exceptions.hpp"

Database::Database()
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

Database::~Database()
{
    if (m_db && m_initialized) {
        m_db->shutdown();
    }
}

bool Database::openDatabase()
{
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

void Database::createTable()
{
    // Tables are created automatically by SQLiteDatabase::initialize()
    // This method is kept for API compatibility
}

bool Database::addItem(const Item &item)
{
    if (!m_initialized) {
        if (!openDatabase()) {
            return false;
        }
    }

    try {
        auto component = item.toComponent();
        ecim::ComponentID id = m_db->addComponent(*component);
        return id != 0;
    } catch (const ecim::DatabaseException& e) {
        qDebug() << "Insert failed: " << e.what();
        return false;
    }
}

QVector<Item> Database::getAllItems()
{
    QVector<Item> items;

    if (!m_initialized) {
        if (!openDatabase()) {
            return items;
        }
    }

    try {
        ecim::MassQueryConfig config;
        ecim::MassQueryResult result = m_db->getAllComponents(config);

        for (const auto& component : result.items) {
            Item item = Item::fromComponent(*component, component->ID());
            items.append(item);
        }
    } catch (const ecim::DatabaseException& e) {
        qDebug() << "Query failed: " << e.what();
    }

    return items;
}

bool Database::updateItem(int originalPartNumber, const Item &item)
{
    if (!m_initialized) {
        if (!openDatabase()) {
            return false;
        }
    }

    try {
        auto component = item.toComponent();
        m_db->editComponent(originalPartNumber, *component);
        return true;
    } catch (const ecim::DatabaseException& e) {
        qDebug() << "Update failed: " << e.what();
        return false;
    }
}

bool Database::deleteItem(int partNumber)
{
    if (!m_initialized) {
        if (!openDatabase()) {
            return false;
        }
    }

    try {
        m_db->removeComponent(partNumber);
        return true;
    } catch (const ecim::DatabaseException& e) {
        qDebug() << "Delete failed: " << e.what();
        return false;
    }
}

void Database::reopenDatabase()
{
    if (m_db && m_initialized) {
        m_db->shutdown();
        m_initialized = false;
    }

    QSettings settings("MyCompany", "InventoryApp");

    QString folderPath = settings.value("dbPath").toString();
    m_dbPath = folderPath + "/inventory.db";

    if (!openDatabase()) {
        qDebug() << "Failed to reopen DB";
    } else {
        qDebug() << "DB reopened at: " << m_dbPath;
    }
}

QString Database::getDatabasePath() const
{
    return m_dbPath;
}

bool Database::moveDatabase(const QString &newFolder)
{
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














