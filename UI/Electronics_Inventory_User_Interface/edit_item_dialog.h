#ifndef EDIT_ITEM_DIALOG_H
#define EDIT_ITEM_DIALOG_H

#include <QDialog>

namespace Ui {
class Edit_Item_Dialog;
}

class Edit_Item_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Edit_Item_Dialog(QWidget *parent = nullptr);
    ~Edit_Item_Dialog();

    void setItemData(const QString &name,
                 int quantity,
                 int partNumber,
                 const QString &imagePath);

    QString getName() const;
    int getQuantity() const;
    int getPartNumber() const;
    QString getImagePath() const;

private:
    Ui::Edit_Item_Dialog *ui;
    QString imagePath;
    int originalPartNumber;     //To identify item in database.

signals:
    void deleteRequested(int partNumber);

};

#endif // EDIT_ITEM_DIALOG_H
