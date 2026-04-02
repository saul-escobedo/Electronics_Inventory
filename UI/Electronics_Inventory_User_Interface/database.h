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
private:
    QSqlDatabase db;
};

#endif // DATABASE_H
