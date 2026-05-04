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
        const QString &part_number,
        const QString &image_path,
        const QString &component_type,
        const QString &manufacturer,
        const QString &property1,
        const QString &property2
    );

    void Set_Item_Data(
        const QString &name,
        int quantity,
        const QString &part_number,
        const QString &image_path,
        const QString &component_type,
        const QString &manufacturer,
        const QString &property1,
        const QString &property2
    );

private:
    Ui::ViewItemDialog *ui;
};

#endif // VIEW_ITEM_DIALOG_HPP
