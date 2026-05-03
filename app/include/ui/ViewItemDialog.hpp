#ifndef VIEW_ITEM_DIALOG_HPP
#define VIEW_ITEM_DIALOG_HPP

#include <QDialog>

namespace Ui {
class ViewItemDialog;
}

class ViewItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ViewItemDialog(QWidget *parent = nullptr);
    ~ViewItemDialog();

    void setItemData(
        const QString &name,
        int quantity,
        int part_number,
        const QString &image_path
    );

    //Function to fill data of item.
    void Set_Item_Data(
        const QString &name,
        int quantity,
        int part_number,
        const QString &image_path
    );

private:
    Ui::ViewItemDialog *ui;
};

#endif // VIEW_ITEM_DIALOG_HPP
