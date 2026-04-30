#include "ui/ViewItemDialog.hpp"
#include "ui_ViewItemDialog.h"

#include <QPixmap>

ViewItemDialog::ViewItemDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ViewItemDialog)
{
    ui->setupUi(this);

}

ViewItemDialog::~ViewItemDialog()
{
    delete ui;
}

//Definition of Set Item Data
//This will fill in all the data from the table.
void ViewItemDialog::Set_Item_Data(const QString &name,
                                     int quantity,
                                     int part_number,
                                     const QString &image_path)
{
    ui->Name_Label->setText(name);
    ui->Part_Number_Label->setText(QString::number(part_number));
    ui->Quantity_Label->setText(QString::number(quantity));

    QPixmap pix(image_path);
    ui->Image_Label->setPixmap(
        pix.scaled(200, 200, Qt::KeepAspectRatio)
        );
}
