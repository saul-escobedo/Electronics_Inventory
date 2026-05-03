#include "ui/EditItemDialog.hpp"
#include "ui_EditItemDialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QIntValidator>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>

EditItemDialog::EditItemDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditItemDialog)
{
    ui->setupUi(this);
    setWindowTitle("Edit Item");

    ui->image_label->setAlignment(Qt::AlignCenter);
    ui->image_label->setText("No image selected");
    ui->part_number_line_edit->setValidator(new QIntValidator(1, 999999999, this));
    ui->quantity_spin_box->setMinimum(0);

    auto updateSaveState = [this]() {
        const bool hasName = !ui->name_line_edit->text().trimmed().isEmpty();
        const bool hasPartNumber = !ui->part_number_line_edit->text().trimmed().isEmpty();
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(hasName && hasPartNumber);
    };

    connect(ui->name_line_edit, &QLineEdit::textChanged, this, [updateSaveState]() {
        updateSaveState();
    });
    connect(ui->part_number_line_edit, &QLineEdit::textChanged, this, [updateSaveState]() {
        updateSaveState();
    });

    connect(ui->select_image_button, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(
            this,
            "Select Image",
            "",
            "Images (*.png *.jpg *.jpeg)"
        );

        if(file.isEmpty()) {
            return;
        }

        imagePath = file;
        QPixmap pix(file);

        if(pix.isNull()) {
            ui->image_label->setPixmap(QPixmap());
            ui->image_label->setText("Image preview unavailable");
            return;
        }

        ui->image_label->setText(QString());
        ui->image_label->setPixmap(pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    });

    connect(ui->delete_button, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(originalPartNumber);
        reject();
    });

    updateSaveState();
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
    if(pix.isNull()) {
        ui->image_label->setPixmap(QPixmap());
        ui->image_label->setText("No image selected");
    } else {
        ui->image_label->setText(QString());
        ui->image_label->setPixmap(pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
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
