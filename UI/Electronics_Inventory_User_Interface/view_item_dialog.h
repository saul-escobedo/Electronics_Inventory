#ifndef VIEW_ITEM_DIALOG_H
#define VIEW_ITEM_DIALOG_H

#include <QDialog>

namespace Ui {
class View_Item_Dialog;
}

class View_Item_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit View_Item_Dialog(QWidget *parent = nullptr);
    ~View_Item_Dialog();

    //Function to fill data of item.
void Set_Item_Data(const QString &name,
                   int quantity,
                   int part_number,
                    const QString &image_path);



private:
    Ui::View_Item_Dialog *ui;
};

#endif // VIEW_ITEM_DIALOG_H
