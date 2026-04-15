#include "view_item_dialog.h"
#include "ui_view_item_dialog.h"
#include <QPixmap>

View_Item_Dialog::View_Item_Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::View_Item_Dialog)
{
    ui->setupUi(this);

}

View_Item_Dialog::~View_Item_Dialog()
{
    delete ui;
}

//Definition of Set Item Data
//This will fill in all the data from the table.
void View_Item_Dialog::Set_Item_Data(const QString &name,
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
