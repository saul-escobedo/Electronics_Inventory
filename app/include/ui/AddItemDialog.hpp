#ifndef ADD_ITEM_DIALOG_HPP
#define ADD_ITEM_DIALOG_HPP

#include <QDialog>
#include "electrical/ElectronicComponent.hpp"
#include "electrical/ElectronicComponents.hpp"

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
    QString getManufacturer() const;
    QString getPartNumber() const;
    int getQuantity() const;
    QString getImagePath() const;
    ecim::ElectronicComponent::Type getComponentType() const;

    // Component-specific getters
    double getProperty1Value() const;
    QString getProperty2Value() const;
    double getResistance() const;
    double getTolerance() const;
    double getCapacitance() const;
    ecim::Capacitor::Type getCapacitorType() const;
    double getInductance() const;
    double getForwardVoltage() const;
    ecim::Diode::Type getDiodeType() const;
    double getGain() const;
    double getThresholdVoltage() const;
    int getPinCount() const;

private slots:
    void onComponentTypeChanged(int index);

private:
    Ui::AddItemDialog *ui;

    QString image_path;
    void showComponentSpecificFields(ecim::ElectronicComponent::Type type);
};

#endif // ADD_ITEM_DIALOG_HPP
