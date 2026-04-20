#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QMessageBox"
#include "add_item_dialog.h"
#include <QHeaderView>
#include "view_item_dialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "edit_item_dialog.h"
#include "settings.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Every new function init goes AFTER setupUi.

    ui->setupUi(this);



    //Start implementing the database class.
    //This is a test
    //

    if(!dbManager.openDatabase())
    {
        qDebug() << "Database failed to opoen";
    } else
        qDebug() << "Database opened successfully";

    dbManager.createTable();


    //For the total parts stat.
    update_total_parts_label();
    //For text to show on search bar.
    connect(ui->Search_Bar, &QLineEdit::textChanged,
            this, &MainWindow::on_search_text_changed);
    //For pop-up after pressed enter on search bar.
    connect(ui->Search_Bar, &QLineEdit::returnPressed,
            this, &MainWindow::on_search_enter_pressed);

    //Init the table in Dashboard.
    ui->Inventory_Table->setColumnCount(3);
    QStringList headers;
    headers << "Item Name" << "Parts in Stock" << "Part Number";
    ui->Inventory_Table->setHorizontalHeaderLabels(headers);
    ui->Inventory_Table->horizontalHeader()->setStretchLastSection(true);
    ui->Inventory_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //Load items into the table and update the total parts label.
    QVector<Item> items = dbManager.getAllItems();

    for(const Item &item : items)
    {
        add_item(item.name, item.quantity, item.partNumber, item.imagePath);
    }
    update_total_parts_label();

    //Edits the Add Item button
    //What heppens when you use the ok or cancel from "Add Item".
    //Also makes sure that Add Item dialog pops up ONLY when pressing the button.
    connect(ui->Add_Item, &QPushButton::clicked, this, [this]()
    {
        Add_Item_Dialog dialog(this);
        if(dialog.exec() == QDialog::Accepted)
        {
            Item item;
            item.name = dialog.get_name();
            item.quantity = dialog.get_quantity();
            item.partNumber = dialog.get_part_number();
            item.imagePath = dialog.get_image_path();

            add_item(item.name, item.quantity, item.partNumber, item.imagePath);
            dbManager.addItem(item);

            update_total_parts_label();
        }
    });
    //Detects what happens when you double click on an item from inventory table.
    connect(ui->Inventory_Table,
            &QTableWidget::cellDoubleClicked,
            this,
            &MainWindow::open_item_view);

    connect(ui->Edit_Item, &QPushButton::clicked, this, [this] ()
    {
        int row = ui->Inventory_Table->currentRow();

        if(row < 0) {
            QMessageBox::warning(this, "Error", "Select an item first");
            return;
        }

        QTableWidgetItem *name_item = ui->Inventory_Table->item(row, 0);

        QString name = name_item->text();
        int quantity = ui->Inventory_Table->item(row,1)->text().toInt();
        int partNumber = ui->Inventory_Table->item(row,2)->text().toInt();
        QString imagePath = name_item->data(Qt::UserRole).toString();

        Edit_Item_Dialog dialog(this);
        dialog.setItemData(name, quantity, partNumber, imagePath);

        // Handle Delete
        connect(&dialog, &Edit_Item_Dialog::deleteRequested, this, [=](int partNum){
            dbManager.deleteItem(partNum);
            ui->Inventory_Table->removeRow(row);
            update_total_parts_label();
        });

        // ✏️ HANDLE UPDATE
        if(dialog.exec() == QDialog::Accepted)
        {
            Item item;
            item.name = dialog.getName();
            item.quantity = dialog.getQuantity();
            item.partNumber = dialog.getPartNumber();
            item.imagePath = dialog.getImagePath();

            dbManager.updateItem(partNumber, item);

            // update UI
            ui->Inventory_Table->item(row,0)->setText(item.name);
            ui->Inventory_Table->item(row,1)->setText(QString::number(item.quantity));
            ui->Inventory_Table->item(row,2)->setText(QString::number(item.partNumber));
            ui->Inventory_Table->item(row,0)->setData(Qt::UserRole, item.imagePath);
            update_total_parts_label();
        }

    });

    connect(ui->Settings_Button, &QPushButton::clicked, this, [this]() {
        Settings dialog(this);

        connect(&dialog, &Settings::settingsChanged,
                this, [this]()
                {
                    dbManager.reopenDatabase();

                    ui->Inventory_Table->setRowCount(0);

                    QVector<Item> items = dbManager.getAllItems();
                    for(const Item &item : items)
                    {
                        add_item(item.name, item.quantity, item.partNumber, item.imagePath);
                    }

                    update_total_parts_label();
                });

        dialog.exec();
    });
}

//Updates the number of current stock.
void MainWindow::update_total_parts_label()
{
    // ui->Total_Parts->setText(
    //     QString("Total parts: %1").arg(total_parts));
    int total = 0;
    int rows = ui->Inventory_Table->rowCount();
    for(int i = 0; i < rows; i++)
    {
        int quantity = ui->Inventory_Table->item(i, 1)->text().toInt();
        total += quantity;
    }

    total_parts = total;

    ui->Total_Parts->setText(
        QString("Total parts: %1").arg(total_parts));
}

void MainWindow::on_search_text_changed(const QString &text)
{
    qDebug() << "Searching for: " << text;
}

void MainWindow::on_search_enter_pressed()
{
    QString text = ui->Search_Bar->text();
    QMessageBox::information(
        this,
        "Search Entered",
        "You typed: " + text);
}

//Adds item to Inventory Table.
void MainWindow::add_item(const QString &name, int quantity, int part_num, const QString &image_path)
{
    int row = ui->Inventory_Table->rowCount();
    ui->Inventory_Table->insertRow(row);

    QTableWidgetItem *name_item = new QTableWidgetItem(name);
    QTableWidgetItem *quantity_item = new QTableWidgetItem(QString::number(quantity));
    QTableWidgetItem *part_item = new QTableWidgetItem(QString::number(part_num));

    //Store hidden data, image path
    name_item->setData(Qt::UserRole, image_path);

    ui->Inventory_Table->setItem(row, 0, name_item);
    ui->Inventory_Table->setItem(row, 1, quantity_item);
    ui->Inventory_Table->setItem(row, 2, part_item);
}

void MainWindow::open_item_view(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *name_item = ui->Inventory_Table->item(row, 0);

    QString name = name_item->text();
    int quantity = ui->Inventory_Table->item(row, 1) -> text().toInt();
    int part_number = ui->Inventory_Table->item(row, 2)->text().toInt();

    //Retrieve hidden data
    //We can use QT::UserRole to store hidden data within each item.
    QString image_path = name_item->data(Qt::UserRole).toString();

    View_Item_Dialog dialog(this);
    dialog.Set_Item_Data(name, quantity, part_number, image_path);
    dialog.exec();
}

MainWindow::~MainWindow()
{
    delete ui;
}

