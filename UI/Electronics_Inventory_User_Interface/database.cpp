#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>     //Used for the directory of the database file.
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>


Database::Database()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    //REad db path from settings in Database class.
    QSettings settings("MyCompany", "InventoryApp");

    QString folderPath = settings.value(
        "dbPath",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                     ).toString();

    QDir().mkpath(folderPath);      //enddure folder exists

    QString fullPath = folderPath + "/inventory.db";

    //This sets the .db directory to the executable directory.
    //QString path = QCoreApplication::applicationDirPath() + "/inventory.db";
    db.setDatabaseName(fullPath);

    qDebug() << "DB Path: " << fullPath;
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

void Database::reopenDatabase()
{
    if(db.isOpen())
        db.close();

    QSettings settings("MyCompany", "InventoryApp");

    QString folderPath = settings.value("dbPath").toString();
    QString fullPath = folderPath + "/inventory.db";

    db.setDatabaseName(fullPath);

    if(!db.open())
    {
        qDebug() << "Failed to reopen DB: " << db.lastError().text();
    } else {
        qDebug() << "DB reopened at: " << fullPath;
    }
}

QString Database::getDatabasePath() const
{
    return db.databaseName();
}

bool Database::moveDatabase(const QString &newFolder)
{
    QString oldPath = db.databaseName();
    QString newPath = newFolder + "/inventory.db";

    if(oldPath == newPath)
        return true;

    //Ensure folder exists
    QDir().mkpath(newFolder);

    //Close db before copying
    if(db.isOpen())
        db.close();

    //If destination already exists, remove it
    if(QFile::exists(newPath))
    {
        if(!QFile::remove(newPath))
        {
            qDebug() << "Failed to remove existing DB at new path.";
            return false;
        }
    }

    //Copy file
    if(!QFile::copy(oldPath, newPath))
    {
        qDebug() << "Failed to copy DB file";
    }

    //Reopen with new path
    db.setDatabaseName(newPath);

    if(!db.open())
    {
        qDebug() << "Failed to open DB at new path.";
        return false;
    }

    qDebug() << "Database moved to: " << newPath;
    return true;
}














