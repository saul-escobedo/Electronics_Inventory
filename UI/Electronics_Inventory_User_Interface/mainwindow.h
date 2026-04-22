#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>   //Used to be able to use time for the automatic backups.
#include "database.h"

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
    int total_parts;
    void update_total_parts_label();
    void on_search_text_changed(const QString &text);
    void on_search_enter_pressed();
    void add_item(const QString &name, int parts, int part_num, const QString &image_path);
    void open_item_view(int row, int column);

};
#endif // MAINWINDOW_H
