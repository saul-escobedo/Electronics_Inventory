#include "ui/ViewItemDialog.hpp"
#include "ui_ViewItemDialog.h"

#include <QPixmap>

ViewItemDialog::ViewItemDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ViewItemDialog)
{
    ui->setupUi(this);
    setWindowTitle("View Item");
    ui->Image_Label->setAlignment(Qt::AlignCenter);

}

ViewItemDialog::~ViewItemDialog()
{
    delete ui;
}

void ViewItemDialog::setItemData(const QString &name,
                                 int quantity,
                                 int part_number,
                                 const QString &image_path)
{
    ui->Name_Label->setText(name);
    ui->Part_Number_Label->setText(QString::number(part_number));
    ui->Quantity_Label->setText(QString::number(quantity));

    QPixmap pix(image_path);
    if(pix.isNull()) {
        ui->Image_Label->setPixmap(QPixmap());
        ui->Image_Label->setText("No image selected");
        return;
    }

    ui->Image_Label->setText(QString());
    ui->Image_Label->setPixmap(pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

// Backwards-compatible wrapper for existing call sites.
void ViewItemDialog::Set_Item_Data(const QString &name,
                                   int quantity,
                                   int part_number,
                                   const QString &image_path)
{
    setItemData(name, quantity, part_number, image_path);
}
