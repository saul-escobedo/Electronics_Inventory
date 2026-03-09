#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QMessageBox"
#include "add_item_dialog.h"
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Every new function init goes AFTER setupUi.

    ui->setupUi(this);
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

    //Sample Data inside constructor for table in Dashboard.
    add_item("Screw", 45, 34565);
    add_item("Bolt", 120, 5678);
    add_item("Nut", 78, 5678);


    //What heppens when you use the ok or cancel from "Add Item".

    connect(ui->Add_Item, &QPushButton::clicked, this, [=]() {
        Add_Item_Dialog dialog(this);
        if(dialog.exec() == QDialog::Accepted)
        {
            add_item(
                dialog.get_name(),
                dialog.get_quantity(),
                dialog.get_part_number()
                );
        }
    });
}

//Updates the number of current stock.
void MainWindow::update_total_parts_label()
{
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
void MainWindow::add_item(const QString &name, int parts, int part_num)
{
    int row = ui->Inventory_Table->rowCount();
    ui->Inventory_Table->insertRow(row);
    ui->Inventory_Table->setItem(row, 0, new QTableWidgetItem(name));
    ui->Inventory_Table->setItem(row, 1, new QTableWidgetItem(QString::number(parts)));
    ui->Inventory_Table->setItem(row, 2, new QTableWidgetItem(QString::number(part_num)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

