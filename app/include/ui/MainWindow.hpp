#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "electrical/ElectronicComponent.hpp"
#include "DatabaseManager.hpp"

#include <QMainWindow>
#include <QTimer>   //Used to be able to use time for the automatic backups.

#include <optional>

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
    Ui::MainWindow *m_ui;
    ecim::DatabaseManager m_dbManager;

    // This determines what columns the table has so that it can represent
    // generic electronic components or specilized types (like Resistors)
    std::optional<ecim::ElectronicComponent::Type> m_catalogType;
    float m_columnPorportionalWidths[16];

    std::vector<std::unique_ptr<ecim::ElectronicComponent>> m_items;

    //Functions for the setting to set backup frequency.
    QTimer *backupTimer;
    void startBackupTimer();
    void performBackup();

    //All functions for Dashboard are init here.

    //This is for updating the total parts in stock from Dashboard.
    void updateTotalPartsLabel(int num);

    // Setup the table's columns depending if it needs to represent generic
    // components, or specialized types (like Capacitors)
    void setupTableColumns();
    void autoResizeTableColumns();

    void fetchDatabase();
    void populateTable();

    // Event handlers
    void onSearchTextChanged(const QString &text);
    void onSearchEnterPressed();
    void addItem(const QString &name, int parts, int part_num, const QString &image_path);
    void openItemView(int row, int column);
    void onChangeCatalog(int selectionIndex);
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
};
#endif // MAINWINDOW_HPP
