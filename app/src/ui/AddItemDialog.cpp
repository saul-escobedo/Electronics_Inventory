#include "ui/AddItemDialog.hpp"
#include "ui_AddItemDialog.h"

#include <QFileDialog>
#include <QPixmap>
#include <QIntValidator>
#include <QPushButton>

AddItemDialog::AddItemDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddItemDialog)
{
    ui->setupUi(this);

    //Make it so QLineEdit accepts ONLY integers.
    ui->edit_part_num->setValidator(
        new QIntValidator(0, 999999999, this)
    );

    //Button to add image when adding item.
    connect(ui->Select_Image, &QPushButton::clicked, this, [=]() {

        QString file = QFileDialog::getOpenFileName(
            this,
            "Select Image",
            "",
            "Images (*.png *.jpg *.jpeg)"
            );

        if(!file.isEmpty())
        {
            image_path = file;

            QPixmap pix(file);
            ui->Image_Label->setPixmap(pix.scaled(100, 100, Qt::KeepAspectRatio));
        }

    });
}

AddItemDialog::~AddItemDialog()
{
    delete ui;
}

QString AddItemDialog::getName() const
{
    return ui->edit_name->text();
}

int AddItemDialog::getPartNumber() const
{
    return ui->edit_part_num->text().toInt();
}

int AddItemDialog::getQuantity() const
{
    return ui->edit_quantity->value();
}

QString AddItemDialog::getImagePath() const
{
    return image_path;
}
