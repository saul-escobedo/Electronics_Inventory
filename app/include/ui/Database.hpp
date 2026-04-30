#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <QVector>
#include <QString>
#include <memory>
#include "database/Database.hpp"
#include "database/SQLiteDatabase.hpp"
#include "electrical/ElectronicComponent.hpp"

// Adapter to convert between UI's simple Item struct and backend's ElectronicComponent
struct Item
{
    QString name;
    int quantity;
    int partNumber;
    QString imagePath;
    QString manufacturer;
    QString description;

    // Convert from ElectronicComponent to Item
    static Item fromComponent(const ecim::ElectronicComponent& comp, uint64_t partNumber) {
        Item item;
        item.name = QString::fromStdString(comp.name());
        item.quantity = comp.quantity();
        item.partNumber = static_cast<int>(partNumber);
        item.manufacturer = QString::fromStdString(comp.manufacturer());
        item.description = QString::fromStdString(comp.description());
        return item;
    }

    // Convert from Item to ElectronicComponent (creates a generic component)
    std::unique_ptr<ecim::ElectronicComponent> toComponent() const {
        ecim::ElectronicComponent::BaseConfig config;
        config.name = name.toStdString();
        config.quantity = quantity;
        // Use "Unknown" as default manufacturer if empty
        config.manufacturer = manufacturer.isEmpty() ? "Unknown" : manufacturer.toStdString();
        config.partNumber = std::to_string(partNumber);
        config.description = description.toStdString();
        // Use Resistor as default type for generic components
        return std::make_unique<ecim::ElectronicComponent>(config, ecim::ElectronicComponent::Type::Resistor);
    }
};

class Database
{
public:
    Database();
    ~Database();

    bool openDatabase();
    void createTable();

    bool addItem(const Item &item);
    QVector<Item> getAllItems();

    bool updateItem(int originalPartNumber, const Item &item);
    bool deleteItem(int partNumber);
    void reopenDatabase();

    QString getDatabasePath() const;
    bool moveDatabase(const QString &newFolder);

private:
    std::unique_ptr<ecim::SQLiteDatabase> m_db;
    QString m_dbPath;
    bool m_initialized;
};

#endif // DATABASE_HPP
