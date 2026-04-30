#include "ui/Settings.hpp"
#include "ui_Settings.h"

#include <QSettings>
#include <QFileDialog>

Settings::Settings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);

    loadSettings();

    connect(ui->saveButton, &QPushButton::clicked,
            this, &Settings::saveSettings);

    connect(ui->browseButton, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Folder");
        if(!path.isEmpty())
            ui->dbPathLineEdit->setText(path);
    });
}

Settings::~Settings()
{
    delete ui;
}

void Settings::loadSettings()
{
    QSettings settings("MyCompany", "InventoryApp");

    QString freq = settings.value("backupFrequency", "Never").toString();

    int index = ui->backupFrequency->findText(freq);
    if(index != -1)
        ui->backupFrequency->setCurrentIndex(index);

    ui->usernameLineEdit->setText(settings.value("username", "").toString());
    ui->dbPathLineEdit->setText(settings.value("dbPath", "").toString());
}

void Settings::saveSettings()
{
    QSettings settings("MyCompany", "InventoryApp");
    //Save settings for the backup frequency combo box.
    settings.setValue("backupFrequency",
                      ui->backupFrequency->currentText());

    settings.setValue("username", ui->usernameLineEdit->text());
    settings.setValue("dbPath", ui->dbPathLineEdit->text());

    emit settingsChanged();
    accept(); // close dialog
}
