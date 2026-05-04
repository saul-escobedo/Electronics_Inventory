#include "ui/AddItemDialog.hpp"
#include "ui_AddItemDialog.h"
#include "electrical/ElectronicComponents.hpp"

#include <QFileDialog>
#include <QPixmap>
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>

AddItemDialog::AddItemDialog(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint)
    , ui(new Ui::AddItemDialog)
{
    ui->setupUi(this);
    setWindowTitle("Add Item");
    
    // Center the popup on the parent widget or screen
    if(parent) {
        move(parent->x() + (parent->width() - width()) / 2,
             parent->y() + (parent->height() - height()) / 2);
    } else {
        // Center on screen if no parent
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    ui->Image_Label->setAlignment(Qt::AlignCenter);
    ui->Image_Label->setText("No image selected");
    ui->edit_quantity->setMinimum(0);

    auto updateConfirmState = [this]() {
        const bool hasName = !ui->edit_name->text().trimmed().isEmpty();
        const bool hasPartNumber = !ui->edit_part_num->text().trimmed().isEmpty();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasName && hasPartNumber);
    };

    connect(ui->edit_name, &QLineEdit::textChanged, this, [updateConfirmState]() {
        updateConfirmState();
    });
    connect(ui->edit_part_num, &QLineEdit::textChanged, this, [updateConfirmState]() {
        updateConfirmState();
    });
    updateConfirmState();

    //Button to add image when adding item.
    connect(ui->Select_Image, &QPushButton::clicked, this, [this]() {

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
            if(!pix.isNull()) {
                ui->Image_Label->setText(QString());
                ui->Image_Label->setPixmap(pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                ui->Image_Label->setPixmap(QPixmap());
                ui->Image_Label->setText("Image preview unavailable");
            }
        }

    });

    // Connect component type selector to show/hide fields
    connect(ui->componentType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddItemDialog::onComponentTypeChanged);

    // Hide component-specific group box initially
    ui->componentSpecificGroupBox->hide();

    // Show fields for initial selection
    showComponentSpecificFields(ecim::ElectronicComponent::Type::Resistor);
}

AddItemDialog::~AddItemDialog()
{
    delete ui;
}

void AddItemDialog::onComponentTypeChanged(int index)
{
    ecim::ElectronicComponent::Type type = static_cast<ecim::ElectronicComponent::Type>(index + 1);
    showComponentSpecificFields(type);
}

void AddItemDialog::showComponentSpecificFields(ecim::ElectronicComponent::Type type)
{
    // Show the group box for all component types
    ui->componentSpecificGroupBox->show();

    // Set labels and input types based on component type
    switch(type) {
    case ecim::ElectronicComponent::Type::Resistor:
        ui->Property1_Label->setText("Resistance (Ω):");
        ui->Property2_Label->setText("Tolerance (%):");
        break;
    case ecim::ElectronicComponent::Type::Capacitor:
        ui->Property1_Label->setText("Capacitance (F):");
        ui->Property2_Label->setText("Capacitor Type:");
        break;
    case ecim::ElectronicComponent::Type::Inductor:
        ui->Property1_Label->setText("Inductance (H):");
        ui->Property2_Label->setText("");
        ui->edit_property2->hide();
        ui->Property2_Label->hide();
        break;
    case ecim::ElectronicComponent::Type::Diode:
        ui->Property1_Label->setText("Forward Voltage (V):");
        ui->Property2_Label->setText("Diode Type:");
        break;
    case ecim::ElectronicComponent::Type::BJTransistor:
        ui->Property1_Label->setText("Gain (hFE):");
        ui->Property2_Label->setText("");
        ui->edit_property2->hide();
        ui->Property2_Label->hide();
        break;
    case ecim::ElectronicComponent::Type::FETransistor:
        ui->Property1_Label->setText("Threshold Voltage (V):");
        ui->Property2_Label->setText("");
        ui->edit_property2->hide();
        ui->Property2_Label->hide();
        break;
    case ecim::ElectronicComponent::Type::IntegratedCircuit:
        ui->Property1_Label->setText("Pin Count:");
        ui->Property2_Label->setText("");
        ui->edit_property2->hide();
        ui->Property2_Label->hide();
        break;
    }

    // Show both inputs if needed
    if(type == ecim::ElectronicComponent::Type::Resistor ||
       type == ecim::ElectronicComponent::Type::Capacitor ||
       type == ecim::ElectronicComponent::Type::Diode) {
        ui->edit_property1->show();
        ui->Property1_Label->show();
        ui->edit_property2->show();
        ui->Property2_Label->show();
    } else {
        ui->edit_property1->show();
        ui->Property1_Label->show();
    }
}

QString AddItemDialog::getName() const
{
    return ui->edit_name->text();
}

QString AddItemDialog::getManufacturer() const
{
    QString manufacturer = ui->edit_manufacturer->text().trimmed();
    if(manufacturer.isEmpty())
        return "Unknown manufacturer";
    return manufacturer;
}

QString AddItemDialog::getPartNumber() const
{
    return ui->edit_part_num->text();
}

int AddItemDialog::getQuantity() const
{
    return ui->edit_quantity->value();
}

QString AddItemDialog::getImagePath() const
{
    return image_path;
}

ecim::ElectronicComponent::Type AddItemDialog::getComponentType() const
{
    return static_cast<ecim::ElectronicComponent::Type>(ui->componentType->currentIndex() + 1);
}

double AddItemDialog::getProperty1Value() const
{
    return ui->edit_property1->text().toDouble();
}

QString AddItemDialog::getProperty2Value() const
{
    return ui->edit_property2->text();
}

double AddItemDialog::getResistance() const
{
    return getProperty1Value();
}

double AddItemDialog::getTolerance() const
{
    return getProperty2Value().toDouble();
}

double AddItemDialog::getCapacitance() const
{
    return getProperty1Value();
}

ecim::Capacitor::Type AddItemDialog::getCapacitorType() const
{
    QString typeStr = getProperty2Value();
    if(typeStr == "Ceramic") return ecim::Capacitor::Type::Ceramic;
    if(typeStr == "Aluminum Electrolytic") return ecim::Capacitor::Type::AluminumElectrolytic;
    if(typeStr == "Tantalum") return ecim::Capacitor::Type::Tantalum;
    if(typeStr == "Film") return ecim::Capacitor::Type::Film;
    return ecim::Capacitor::Type::Ceramic; // Default
}

double AddItemDialog::getInductance() const
{
    return getProperty1Value();
}

double AddItemDialog::getForwardVoltage() const
{
    return getProperty1Value();
}

ecim::Diode::Type AddItemDialog::getDiodeType() const
{
    QString typeStr = getProperty2Value();
    if(typeStr == "Regular") return ecim::Diode::Type::Regular;
    if(typeStr == "Schottky") return ecim::Diode::Type::Schottky;
    if(typeStr == "Zener") return ecim::Diode::Type::Zener;
    if(typeStr == "LED") return ecim::Diode::Type::LED;
    return ecim::Diode::Type::Regular; // Default
}

double AddItemDialog::getGain() const
{
    return getProperty1Value();
}

double AddItemDialog::getThresholdVoltage() const
{
    return getProperty1Value();
}

int AddItemDialog::getPinCount() const
{
    return static_cast<int>(getProperty1Value());
}
