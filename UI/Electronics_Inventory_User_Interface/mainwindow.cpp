#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include "add_item_dialog.h"
#include <QHeaderView>
#include "view_item_dialog.h"
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include "edit_item_dialog.h"
#include "settings.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Every new function init goes AFTER setupUi.

    ui->setupUi(this);

    if(!dbManager.openDatabase())
    {
        qDebug() << "Database failed to open";
    } else
        qDebug() << "Database opened successfully";

    dbManager.createTable();

    update_total_parts_label();
    
    connect(ui->Search_Bar, &QLineEdit::textChanged,
            this, &MainWindow::on_search_text_changed);
    connect(ui->Search_Bar, &QLineEdit::returnPressed,
            this, &MainWindow::on_search_enter_pressed);

    // Initialize the table
    const int COL_NAME = 0;
    const int COL_QUANTITY = 1;
    const int COL_PART = 2;
    
    ui->Inventory_Table->setColumnCount(3);
    QStringList headers;
    headers << "Item Name" << "Parts in Stock" << "Part Number";
    ui->Inventory_Table->setHorizontalHeaderLabels(headers);
    ui->Inventory_Table->horizontalHeader()->setStretchLastSection(true);
    ui->Inventory_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->Inventory_Table->setSortingEnabled(true);

    // Load items into the table
    QVector<Item> items = dbManager.getAllItems();

    for(const Item &item : items)
    {
        add_item(item.name, item.quantity, item.partNumber, item.imagePath);
    }
    update_total_parts_label();

    // Add Item button
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
    
    // Double-click to view item
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

        // Handle Update
        if(dialog.exec() == QDialog::Accepted)
        {
            Item item;
            item.name = dialog.getName();
            item.quantity = dialog.getQuantity();
            item.partNumber = dialog.getPartNumber();
            item.imagePath = dialog.getImagePath();

            dbManager.updateItem(partNumber, item);

            // Update UI
            ui->Inventory_Table->item(row, 0)->setText(item.name);
            ui->Inventory_Table->item(row, 1)->setText(QString::number(item.quantity));
            ui->Inventory_Table->item(row, 2)->setText(QString::number(item.partNumber));
            ui->Inventory_Table->item(row, 0)->setData(Qt::UserRole, item.imagePath);
            update_total_parts_label();
        }

    });

    connect(ui->Settings_Button, &QPushButton::clicked, this, [this]() {
        Settings dialog(this);

        connect(&dialog, &Settings::settingsChanged,
                this, [this]()
                {
            QSettings settings("MyCompany", "InventoryApp");
            QString newFolder = settings.value("dbPath").toString();

            if(!dbManager.moveDatabase(newFolder))
            {
                QMessageBox::warning(this, "Error", "Failed to move database.");
                return;
            }
            
            ui->Inventory_Table->setRowCount(0);
            QVector<Item> items = dbManager.getAllItems();
            for(const Item &item : items)
            {
                add_item(item.name, item.quantity, item.partNumber, item.imagePath);
            }
            update_total_parts_label();
        });

        startBackupTimer();
        dialog.exec();
    });

    // Setup backup timer
    backupTimer = new QTimer(this);
    connect(backupTimer, &QTimer::timeout, this, &MainWindow::performBackup);
    startBackupTimer();

}

void MainWindow::update_total_parts_label()
{
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
    if (text.isEmpty()) {
        // Show all items when search is empty
        for (int i = 0; i < ui->Inventory_Table->rowCount(); ++i) {
            ui->Inventory_Table->setRowHidden(i, false);
        }
        return;
    }

    // Hide rows that don't match the search text
    for (int i = 0; i < ui->Inventory_Table->rowCount(); ++i) {
        QTableWidgetItem *nameItem = ui->Inventory_Table->item(i, 0);
        QTableWidgetItem *partItem = ui->Inventory_Table->item(i, 2);
        
        bool nameMatch = nameItem && nameItem->text().contains(text, Qt::CaseInsensitive);
        bool partMatch = partItem && partItem->text().contains(text, Qt::CaseInsensitive);
        
        ui->Inventory_Table->setRowHidden(i, !(nameMatch || partMatch));
    }
}

void MainWindow::on_search_enter_pressed()
{
    // Search is already handled in on_search_text_changed
    // This function can be used for additional actions if needed
}

void MainWindow::add_item(const QString &name, int quantity, int part_num, const QString &image_path)
{
    int row = ui->Inventory_Table->rowCount();
    ui->Inventory_Table->insertRow(row);

    QTableWidgetItem *name_item = new QTableWidgetItem(name);
    QTableWidgetItem *quantity_item = new QTableWidgetItem();
    quantity_item->setData(Qt::DisplayRole, quantity);
    QTableWidgetItem *part_item = new QTableWidgetItem();
    part_item->setData(Qt::DisplayRole, part_num);

    // Store image path as hidden data
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
    int quantity = ui->Inventory_Table->item(row, 1)->text().toInt();
    int part_number = ui->Inventory_Table->item(row, 2)->text().toInt();
    QString image_path = name_item->data(Qt::UserRole).toString();

    View_Item_Dialog dialog(this);
    dialog.Set_Item_Data(name, quantity, part_number, image_path);
    dialog.exec();
}

void MainWindow::startBackupTimer()
{
    QSettings settings("MyCompany", "InventoryApp");

    QString freq = settings.value("backupFrequency", "Never").toString();
    backupTimer->stop();

    if(freq == "Never")
    {
        qDebug() << "Aut-backup disabled.";
        return;
    }

    if(freq == "On Startup")
    {
        performBackup(); // run once
        return;
    }

    qint64 intervalMs = 0;

    if(freq == "Daily")
        intervalMs = 24LL * 60 * 60 * 1000;

    else if(freq == "Weekly")
        intervalMs = 7LL * 24 * 60 * 60 * 1000;

    else if(freq == "Monthly")
        intervalMs = 30LL * 24 * 60 * 60 * 1000;

    if(intervalMs > 0)
    {
        backupTimer->start(intervalMs);
        qDebug() << "Backup timer started:" << freq;
    }
}

void MainWindow::performBackup()
{
    QString dbPath = dbManager.getDatabasePath();

    QString backupFolder = QDir::homePath() + "/InventoryBackups";
    QDir().mkpath(backupFolder);

    QString timestamp = QDateTime::currentDateTime()
                            .toString("yyyy-MM-dd_hh-mm-ss");

    QString backupPath = backupFolder + "/backup_" + timestamp + ".db";

    if(QFile::copy(dbPath, backupPath))
    {
        qDebug() << "Backup created:" << backupPath;
    }
    else
    {
        qDebug() << "Backup failed";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

