#include "ui/MainWindow.hpp"
#include "ui_MainWindow.h"

#include "ui/AddItemDialog.hpp"
#include "ui/ViewItemDialog.hpp"
#include "ui/EditItemDialog.hpp"
#include "ui/Settings.hpp"

#include "Config.hpp"

#include <QMessageBox>
#include <QSettings>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Every new function init goes AFTER setupUi.
    ui->setupUi(this);

    if(!dbManager.openDatabase())
        qDebug() << "Database failed to open";
    else
        qDebug() << "Database opened successfully";

    dbManager.createTable();

    updateTotalPartsLabel();

    connect(ui->Search_Bar, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);
    connect(ui->Search_Bar, &QLineEdit::returnPressed,
            this, &MainWindow::onSearchEnterPressed);

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

    ui->versionLabel->setText(QString("Version v" ECIM_VERSION));
    ui->buildLabel->setText(QString("Build " ECIM_BRANCH " (" ECIM_BUILD ")"));

    // Load items into the table
    QVector<Item> items = dbManager.getAllItems();

    for(const Item &item : items)
    {
        addItem(item.name, item.quantity, item.partNumber, item.imagePath);
    }
    updateTotalPartsLabel();

    // Add Item button
    connect(ui->Add_Item, &QPushButton::clicked, this, [this]()
    {
        AddItemDialog dialog(this);
        if(dialog.exec() == QDialog::Accepted)
        {
            Item item;
            item.name = dialog.getName();
            item.quantity = dialog.getQuantity();
            item.partNumber = dialog.getPartNumber();
            item.imagePath = dialog.getImagePath();

            addItem(item.name, item.quantity, item.partNumber, item.imagePath);
            dbManager.addItem(item);

            updateTotalPartsLabel();
        }
    });

    // Double-click to view item
    connect(ui->Inventory_Table,
            &QTableWidget::cellDoubleClicked,
            this,
            &MainWindow::openItemView);


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

        EditItemDialog dialog(this);
        dialog.setItemData(name, quantity, partNumber, imagePath);

        // Handle Delete
        connect(&dialog, &EditItemDialog::deleteRequested, this, [=](int partNum){
            dbManager.deleteItem(partNum);
            ui->Inventory_Table->removeRow(row);
            updateTotalPartsLabel();
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
            updateTotalPartsLabel();
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
                addItem(item.name, item.quantity, item.partNumber, item.imagePath);
            }
            updateTotalPartsLabel();
        });

        startBackupTimer();
        dialog.exec();
    });

    // Setup backup timer
    backupTimer = new QTimer(this);
    connect(backupTimer, &QTimer::timeout, this, &MainWindow::performBackup);
    startBackupTimer();

}

void MainWindow::updateTotalPartsLabel()
{
    int total = 0;
    int rows = ui->Inventory_Table->rowCount();
    for(int i = 0; i < rows; i++)
    {
        int quantity = ui->Inventory_Table->item(i, 1)->text().toInt();
        total += quantity;
    }

    totalParts = total;

    ui->Total_Parts->setText(
        QString("Total parts: %1").arg(totalParts));
}

void MainWindow::onSearchTextChanged(const QString &text)
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

void MainWindow::onSearchEnterPressed()
{
    // Search is already handled in on_search_text_changed
    // This function can be used for additional actions if needed
}

void MainWindow::addItem(const QString &name, int quantity, int part_num, const QString &image_path)
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

void MainWindow::openItemView(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *name_item = ui->Inventory_Table->item(row, 0);

    QString name = name_item->text();
    int quantity = ui->Inventory_Table->item(row, 1)->text().toInt();
    int part_number = ui->Inventory_Table->item(row, 2)->text().toInt();
    QString image_path = name_item->data(Qt::UserRole).toString();

    ViewItemDialog dialog(this);
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
