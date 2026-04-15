#include "add_item_dialog.h"
#include "ui_add_item_dialog.h"
#include "QFileDialog"
#include "QPixmap"

Add_Item_Dialog::Add_Item_Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Add_Item_Dialog)
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

Add_Item_Dialog::~Add_Item_Dialog()
{
    delete ui;
}

QString Add_Item_Dialog::get_name() const
{
    return ui->edit_name->text();
}

int Add_Item_Dialog::get_part_number() const
{
    return ui->edit_part_num->text().toInt();
}

int Add_Item_Dialog::get_quantity() const
{
    return ui->edit_quantity->value();
}

QString Add_Item_Dialog::get_image_path() const
{
    return image_path;
}
