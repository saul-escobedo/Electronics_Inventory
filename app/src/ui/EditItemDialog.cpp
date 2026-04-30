#include "ui/EditItemDialog.hpp"
#include "ui_EditItemDialog.h"

#include <QPushButton>

EditItemDialog::EditItemDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditItemDialog)
{
    ui->setupUi(this);

    connect(ui->delete_button, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(originalPartNumber);
        reject();
    });
}

EditItemDialog::~EditItemDialog()
{
    delete ui;
}


void EditItemDialog::setItemData(const QString &name,
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

QString EditItemDialog::getName() const
{
    return ui->name_line_edit->text();
}

int EditItemDialog::getQuantity() const
{
    return ui->quantity_spin_box->value();
}

int EditItemDialog::getPartNumber() const
{
    return ui->part_number_line_edit->text().toInt();
}

QString EditItemDialog::getImagePath() const
{
    return imagePath;
}
