#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QMessageBox"
#include "add_item_dialog.h"
#include <QHeaderView>
#include "view_item_dialog.h"

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
    //Also makes sure that Add Item dialog pops up ONLY when pressing the button.
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
    //Detects what happens when you double click on an item from inventory table.
    connect(ui->Inventory_Table,
            &QTableWidget::cellDoubleClicked,
            this,
            &MainWindow::open_item_view);
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

void MainWindow::open_item_view(int row, int column)
{
    Q_UNUSED(column);

    QString name = ui->Inventory_Table->item(row, 0)->text();
    int quantity = ui->Inventory_Table->item(row, 1) -> text().toInt();
    int part_number = ui->Inventory_Table->item(row, 2)->text().toInt();

    QString image_path = "";

    View_Item_Dialog dialog(this);

    dialog.Set_Item_Data(name, quantity, part_number, image_path);
    dialog.exec();
}

MainWindow::~MainWindow()
{
    delete ui;
}

