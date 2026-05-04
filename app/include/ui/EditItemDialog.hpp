#ifndef EDIT_ITEM_DIALOG_HPP
#define EDIT_ITEM_DIALOG_HPP

#include <QDialog>

namespace Ui {
class EditItemDialog;
}

class EditItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditItemDialog(QWidget *parent = nullptr);
    ~EditItemDialog();

    void setItemData(const QString &name,
                 int quantity,
                 const QString &partNumber,
                 const QString &imagePath);

    QString getName() const;
    int getQuantity() const;
    QString getPartNumber() const;
    QString getImagePath() const;

private:
    Ui::EditItemDialog *ui;
    QString imagePath;
    QString originalPartNumber = ""; // To identify item in database.

signals:
    void deleteRequested(const QString &partNumber);
};

#endif // EDIT_ITEM_DIALOG_HPP
