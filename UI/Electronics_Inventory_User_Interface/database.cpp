#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>     //Used for the directory of the database file.
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>


Database::Database()
{
    db = QSqlDatabase::addDatabase("QSQLITE");

    QSettings settings(""
    //This sets the .db directory to the executable directory.
    QString path = QCoreApplication::applicationDirPath() + "/inventory.db";
    db.setDatabaseName(path);

    qDebug() << "DB Path: " << path;
}

bool Database::openDatabase()
{
    if(!db.open())
    {
        qDebug() << "Error opening database: " << db.lastError().text();
        return false;
    }
    return true;
}

void Database::createTable()
{
    QSqlQuery query;

    query.exec(
        "CREATE TABLE IF NOT EXISTS items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "quantity INTEGER,"
        "partNumber INTEGER,"
        "imagePath TEXT)"
        );
}

bool Database::addItem(const Item &item)
{
    QSqlQuery query;

    query.prepare("INSERT INTO items (name, quantity, partNumber, imagePath) "
                  "VALUES (?, ?, ?, ?)");

    query.addBindValue(item.name);
    query.addBindValue(item.quantity);
    query.addBindValue(item.partNumber);
    query.addBindValue(item.imagePath);

    if(!query.exec())
    {
        qDebug() << "Insert failed: " << query.lastError().text();
        return false;
    }
    return true;
}

QVector<Item> Database::getAllItems()
{
    QVector<Item> items;

    QSqlQuery query("SELECT name, quantity, partNumber, imagePath FROM items");

    while (query.next()) {
        Item item;
        item.name = query.value("name").toString();
        item.quantity = query.value("quantity").toInt();
        item.partNumber = query.value("partNumber").toInt();
        item.imagePath = query.value("imagePath").toString();

        items.append(item);
    }
    return items;
}

bool Database::updateItem(int originalPartNumber, const Item &item)
{
    QSqlQuery query;
    query.prepare("UPDATE items SET name=?, quantity=?, partNumber=?, imagePath=? "
                  "WHERE partNumber=?");

    query.addBindValue(item.name);
    query.addBindValue(item.quantity);
    query.addBindValue(item.partNumber);
    query.addBindValue(item.imagePath);
    query.addBindValue(originalPartNumber);

    return query.exec();
}

bool Database::deleteItem(int partNumber)
{
    QSqlQuery query;
    query.prepare("DELETE FROM items WHERE partNumber=?");

    query.addBindValue(partNumber);

    return query.exec();
}

