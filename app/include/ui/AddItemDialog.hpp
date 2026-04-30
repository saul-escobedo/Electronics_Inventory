#ifndef ADD_ITEM_DIALOG_HPP
#define ADD_ITEM_DIALOG_HPP

#include <QDialog>

namespace Ui {
class AddItemDialog;
}

class AddItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddItemDialog(QWidget *parent = nullptr);
    ~AddItemDialog();

    QString getName() const;
    int getPartNumber() const;
    int getQuantity() const;
    QString getImagePath() const;

private:
    Ui::AddItemDialog *ui;

    QString image_path;
};

#endif // ADD_ITEM_DIALOG_HPP
