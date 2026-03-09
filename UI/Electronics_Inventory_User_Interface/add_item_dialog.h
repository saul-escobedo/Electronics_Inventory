#ifndef ADD_ITEM_DIALOG_H
#define ADD_ITEM_DIALOG_H

#include <QDialog>

namespace Ui {
class Add_Item_Dialog;
}

class Add_Item_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Add_Item_Dialog(QWidget *parent = nullptr);
    ~Add_Item_Dialog();

    QString get_name() const;
    int get_part_number() const;
    int get_quantity() const;
    QString get_image_path() const;

private:
    Ui::Add_Item_Dialog *ui;

    QString image_path;
};

#endif // ADD_ITEM_DIALOG_H
