#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QVector>
#include <QString>

struct Item
{
    //Item() {}
    QString name;
    int quantity;
    int partNumber;
    QString imagePath;
};

class Database
{
public:
    Database();

    bool openDatabase();
    void createTable();

    bool addItem(const Item &item);
    QVector<Item> getAllItems();

    bool updateItem(int originalPartNumber, const Item &item);
    bool deleteItem(int partNumber);
    void reopenDatabase();      //Used to restart connection so sqlite creates a new
                                //db file in inputed default path.
    //Helper functions to be able to copy db if path changes.
    QString getDatabasePath() const;
    bool moveDatabase(const QString &newFolder);

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
