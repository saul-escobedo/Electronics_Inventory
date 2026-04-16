#include "settings.h"
#include "ui_settings.h"
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

    ui->usernameLineEdit->setText(settings.value("username", "").toString());
    ui->dbPathLineEdit->setText(settings.value("dbPath", "").toString());
}

void Settings::saveSettings()
{
    QSettings settings("MyCompany", "InventoryApp");

    settings.setValue("username", ui->usernameLineEdit->text());
    settings.setValue("dbPath", ui->dbPathLineEdit->text());

    accept(); // close dialog
}