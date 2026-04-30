#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "Database.hpp"

#include <QMainWindow>
#include <QTimer>   //Used to be able to use time for the automatic backups.

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Database dbManager;

    //Functions for the setting to set backup frequency.
    QTimer *backupTimer;
    void startBackupTimer();
    void performBackup();

    //All functions for Dashboard are init here.

    //This is for updating the total parts in stock from Dashboard.
    int totalParts;
    void updateTotalPartsLabel();
    void onSearchTextChanged(const QString &text);
    void onSearchEnterPressed();
    void addItem(const QString &name, int parts, int part_num, const QString &image_path);
    void openItemView(int row, int column);

};
#endif // MAINWINDOW_HPP
