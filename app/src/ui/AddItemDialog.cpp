#include "ui/AddItemDialog.hpp"
#include "ui_AddItemDialog.h"

#include <QFileDialog>
#include <QPixmap>
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>

AddItemDialog::AddItemDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddItemDialog)
{
    ui->setupUi(this);
    setWindowTitle("Add Item");

    ui->Image_Label->setAlignment(Qt::AlignCenter);
    ui->Image_Label->setText("No image selected");
    ui->edit_quantity->setMinimum(0);

    //Make it so QLineEdit accepts ONLY integers.
    ui->edit_part_num->setValidator(
        new QIntValidator(1, 999999999, this)
    );

    auto updateConfirmState = [this]() {
        const bool hasName = !ui->edit_name->text().trimmed().isEmpty();
        const bool hasPartNumber = !ui->edit_part_num->text().trimmed().isEmpty();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasName && hasPartNumber);
    };

    connect(ui->edit_name, &QLineEdit::textChanged, this, [updateConfirmState]() {
        updateConfirmState();
    });
    connect(ui->edit_part_num, &QLineEdit::textChanged, this, [updateConfirmState]() {
        updateConfirmState();
    });
    updateConfirmState();

    //Button to add image when adding item.
    connect(ui->Select_Image, &QPushButton::clicked, this, [this]() {

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
            if(!pix.isNull()) {
                ui->Image_Label->setText(QString());
                ui->Image_Label->setPixmap(pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                ui->Image_Label->setPixmap(QPixmap());
                ui->Image_Label->setText("Image preview unavailable");
            }
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
