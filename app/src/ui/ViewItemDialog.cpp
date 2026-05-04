#include "ui/ViewItemDialog.hpp"
#include "ui_ViewItemDialog.h"

#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>

ViewItemDialog::ViewItemDialog(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint)
    , ui(new Ui::ViewItemDialog)
{
    ui->setupUi(this);
    setWindowTitle("View Item");
    ui->Image_Label->setAlignment(Qt::AlignCenter);

    // Center the popup on the parent widget or screen
    if(parent) {
        move(parent->x() + (parent->width() - width()) / 2,
             parent->y() + (parent->height() - height()) / 2);
    } else {
        // Center on screen if no parent
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

}

ViewItemDialog::~ViewItemDialog()
{
    delete ui;
}

void ViewItemDialog::setItemData(const QString &name,
                                 int quantity,
                                 const QString &part_number,
                                 const QString &image_path,
                                 const QString &component_type,
                                 const QString &manufacturer,
                                 const QString &property1,
                                 const QString &property2)
{
    ui->Name_Label->setText(name);
    ui->Part_Number_Label->setText(part_number);
    ui->Quantity_Label->setText(QString::number(quantity));
    ui->Type_Label->setText(component_type);
    ui->Manufacturer_Label->setText(manufacturer);
    ui->Property1_Label->setText(property1);
    ui->Property2_Label->setText(property2);

    QPixmap pix(image_path);
    if(pix.isNull()) {
        ui->Image_Label->setPixmap(QPixmap());
        ui->Image_Label->setText("No image selected");
    } else {
        ui->Image_Label->setText(QString());
        ui->Image_Label->setPixmap(pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void ViewItemDialog::Set_Item_Data(
    const QString &name,
    int quantity,
    const QString &part_number,
    const QString &image_path,
    const QString &component_type,
    const QString &manufacturer,
    const QString &property1,
    const QString &property2
) {
    setItemData(name, quantity, part_number, image_path, component_type, manufacturer, property1, property2);
}
