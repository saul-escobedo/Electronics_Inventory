#include "ui/Settings.hpp"
#include "ui_Settings.h"

#include <QSettings>
#include <QFileDialog>
#include <QScreen>
#include <QGuiApplication>

Settings::Settings(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);
    
    // Center the dialog on the parent widget or screen
    if(parent) {
        move(parent->x() + (parent->width() - width()) / 2,
             parentWidget()->y() + (parentWidget()->height() - height()) / 2);
    } else {
        // Center on screen if no parent
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    loadSettings();

    connect(ui->browseButton, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Folder");
        if(!path.isEmpty())
            ui->dbPathLineEdit->setText(path);
    });

    connect(ui->autoBackupCheckBox, &QCheckBox::stateChanged, this, [this]() {
        saveSettings();
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        saveSettings();
        accept();
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
    ui->autoBackupCheckBox->setChecked(settings.value("autoBackup", false).toBool());
}

void Settings::saveSettings()
{
    QSettings settings("MyCompany", "InventoryApp");
    //Save settings for the backup frequency combo box.
    settings.setValue("backupFrequency",
                      ui->backupFrequency->currentText());

    settings.setValue("username", ui->usernameLineEdit->text());
    
    // Validate and save db path
    QString dbPath = ui->dbPathLineEdit->text();
    if(dbPath.isEmpty() || dbPath.isNull()) {
        qDebug() << "Empty dbPath, not saving";
    } else {
        settings.setValue("dbPath", dbPath);
    }
    
    settings.setValue("autoBackup", ui->autoBackupCheckBox->isChecked());

    emit settingsChanged();
}
