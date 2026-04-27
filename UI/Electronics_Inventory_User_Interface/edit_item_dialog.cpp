#include "edit_item_dialog.h"
#include "ui_edit_item_dialog.h"

Edit_Item_Dialog::Edit_Item_Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Edit_Item_Dialog)
{
    ui->setupUi(this);

    connect(ui->delete_button, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(originalPartNumber);
        reject();
    });
}

Edit_Item_Dialog::~Edit_Item_Dialog()
{
    delete ui;
}


void Edit_Item_Dialog::setItemData(const QString &name,
                                   int quantity,
                                   int partNumber,
                                   const QString &imagePath)
{
    ui->name_line_edit->setText(name);
    ui->quantity_spin_box->setValue(quantity);
    ui->part_number_line_edit->setText(QString::number(partNumber));

    this->imagePath = imagePath;
    originalPartNumber = partNumber;

    QPixmap pix(imagePath);
    ui->image_label->setPixmap(pix.scaled(100,100, Qt::KeepAspectRatio));
}

QString Edit_Item_Dialog::getName() const
{
    return ui->name_line_edit->text();
}

int Edit_Item_Dialog::getQuantity() const
{
    return ui->quantity_spin_box->value();
}

int Edit_Item_Dialog::getPartNumber() const
{
    return ui->part_number_line_edit->text().toInt();
}

QString Edit_Item_Dialog::getImagePath() const
{
    return imagePath;
}