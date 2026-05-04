#include "ui/MainWindow.hpp"
#include "database/MassQueryConfig.hpp"
#include "electrical/ElectronicComponent.hpp"
#include "electrical/ElectronicComponents.hpp"
#include "ui_MainWindow.h"

#include "ui/AddItemDialog.hpp"
#include "ui/ViewItemDialog.hpp"
#include "ui/EditItemDialog.hpp"
#include "ui/Settings.hpp"

#include "Config.hpp"

#include <regex>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>

#include <stdexcept>

using namespace ecim;

#define ITEMS_PER_PAGE 20
#define NUMERIC_VALUE_SEARCH_TOLERANCE 1e-9

#define KILO 1e3
#define MEGA 1e6
#define GIGA 1e9
#define MILLI 1e-3
#define PERCENT 1e-2
#define MICRO 1e-6
#define NANO 1e-9
#define PICO 1e-12

static const char* s_componentTypeAsString(ElectronicComponent::Type type);
static const char* s_capacitorTypeAsString(Capacitor::Type type);
static const char* s_diodeTypeAsString(Diode::Type type);
static std::string s_toStandardUnits(double value, const char* unitSuffix);
static double s_fromStandardUnits(const std::string& value, bool& matched);
static QTableWidgetItem* s_newTableItemi(int);
static QTableWidgetItem* s_newTableItemd(double);
static QTableWidgetItem* s_newTableItem(const std::string&);
static QTableWidgetItem* s_newTableItem(const char*);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    //Every new function init goes AFTER setupUi.
    m_ui->setupUi(this);

    if(!m_dbManager.openDatabase())
        qDebug() << "Database failed to open";
    else
        qDebug() << "Database opened successfully";

    connect(m_ui->searchBar, &QLineEdit::textChanged,
            this, &MainWindow::onSearch);

    connect(m_ui->itemsTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::openItemView);

    connect(m_ui->addItem, &QAbstractButton::clicked, this, [this]() {
        addItem("", 0, 0, "");
    });

    connect(m_ui->editItem, &QAbstractButton::clicked, this, [this]() {
        int row = m_ui->itemsTable->currentRow();

        if(row < 0) {
            QMessageBox::information(this, "Edit Item", "Select an item to edit.");
            return;
        }

        openItemEdit(row);
    });

    connect(m_ui->settings, &QAbstractButton::clicked, this, [this]() {
        Settings settingsDialog(this);
        settingsDialog.exec();
    });

    connect(m_ui->deleteItem, &QAbstractButton::clicked, this, [this]() {
        int row = m_ui->itemsTable->currentRow();

        if(row < 0) {
            QMessageBox::information(this, "Delete Item", "Select an item to delete.");
            return;
        }

        QDialog confirmDialog(this, Qt::Popup | Qt::FramelessWindowHint);
        confirmDialog.setWindowTitle("Delete Item");
        
        QVBoxLayout* layout = new QVBoxLayout(&confirmDialog);
        QLabel* label = new QLabel("Are you sure you want to delete this item?", &confirmDialog);
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        
        QPushButton* yesButton = new QPushButton("Yes", &confirmDialog);
        QPushButton* noButton = new QPushButton("No", &confirmDialog);
        
        buttonLayout->addWidget(yesButton);
        buttonLayout->addWidget(noButton);
        
        layout->addWidget(label);
        layout->addLayout(buttonLayout);
        
        // Center the popup on the main window
        QScreen* screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - confirmDialog.width()) / 2;
        int y = (screenGeometry.height() - confirmDialog.height()) / 2;
        confirmDialog.move(x, y);

        connect(yesButton, &QPushButton::clicked, &confirmDialog, &QDialog::accept);
        connect(noButton, &QPushButton::clicked, &confirmDialog, &QDialog::reject);

        if(confirmDialog.exec() == QDialog::Accepted) {
            deleteItem(row);
        }
    });

    connect(m_ui->resistorDividerTool, &QAbstractButton::clicked, this, [this]() {
        openResistorDividerTool();
    });

    // Center the main window on screen
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    m_ui->itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->itemsTable->verticalHeader()->setVisible(false);
    setupTableColumns();

    connect(m_ui->catalogType, &QComboBox::currentIndexChanged, this, &MainWindow::onChangeCatalog);

    setupPaginator();

    m_ui->versionLabel->setText(QString("Version v" ECIM_VERSION));
    m_ui->buildLabel->setText(QString("Build " ECIM_BRANCH "(" ECIM_BUILD ")"));

    m_queryConfig = {
        .pagination = Pagination(1, ITEMS_PER_PAGE),
    };

    // Setup backup timer
    backupTimer = new QTimer(this);
    connect(backupTimer, &QTimer::timeout, this, &MainWindow::performBackup);
    startBackupTimer();
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::updatePartsFoundLabel(int num) {
    m_ui->partsFound->setText(
        QString("Parts Found: %1").arg(num)
    );
}

void MainWindow::updatePaginator() {
    auto* firstPage = m_ui->firstPage;
    auto* previousPage = m_ui->previousPage;
    auto* pageNumber = m_ui->pageNumber;
    auto* numPages = m_ui->numPages;
    auto* nextPage = m_ui->nextPage;
    auto* lastPage = m_ui->lastPage;

    pageNumber->setText(QString("%1").arg(m_dbResult.currentPage));
    numPages->setText(QString("/  %1").arg(m_dbResult.numPages));

    if(m_dbResult.numPages == 0) {
        firstPage->setEnabled(false);
        previousPage->setEnabled(false);
        pageNumber->setEnabled(false);
        numPages->setEnabled(false);
        nextPage->setEnabled(false);
        lastPage->setEnabled(false);

        return;
    }

    pageNumber->setEnabled(true);
    numPages->setEnabled(true);
    pageNumber->setValidator(new QIntValidator(1, m_dbResult.numPages));

    bool onFirstPage = m_dbResult.currentPage == 1;
    bool onLastPage = m_dbResult.currentPage == m_dbResult.numPages;
    firstPage->setEnabled(!onFirstPage);
    previousPage->setEnabled(!onFirstPage);
    nextPage->setEnabled(!onLastPage);
    lastPage->setEnabled(!onLastPage);
}

void MainWindow::setupPaginator() {
    auto* firstPage = m_ui->firstPage;
    auto* previousPage = m_ui->previousPage;
    auto* pageNumber = m_ui->pageNumber;
    auto* numPages = m_ui->numPages;
    auto* nextPage = m_ui->nextPage;
    auto* lastPage = m_ui->lastPage;

    connect(firstPage, &QAbstractButton::clicked, [this]() {
        m_queryConfig.pagination = Pagination(1, ITEMS_PER_PAGE);
        fetchDbAndPopulate();
    });

    connect(previousPage, &QAbstractButton::clicked, [this]() {
        m_queryConfig.pagination = Pagination(m_dbResult.currentPage - 1, ITEMS_PER_PAGE);
        fetchDbAndPopulate();
    });

    connect(pageNumber, &QLineEdit::returnPressed, [this]() {
        int page = m_ui->pageNumber->text().toInt();
        m_queryConfig.pagination = Pagination(page, ITEMS_PER_PAGE);
        fetchDbAndPopulate();
    });

    connect(nextPage, &QAbstractButton::clicked, [this]() {
        m_queryConfig.pagination = Pagination(m_dbResult.currentPage + 1, ITEMS_PER_PAGE);
        fetchDbAndPopulate();
    });

    connect(lastPage, &QAbstractButton::clicked, [this]() {
        m_queryConfig.pagination = Pagination(m_dbResult.numPages, ITEMS_PER_PAGE);
        fetchDbAndPopulate();
    });
}

void MainWindow::startBackupTimer() {
    QSettings settings("MyCompany", "InventoryApp");

    QString freq = settings.value("backupFrequency", "Never").toString();
    backupTimer->stop();

    if(freq == "Never")
    {
        qDebug() << "Aut-backup disabled.";
        return;
    }

    if(freq == "On Startup")
    {
        performBackup(); // run once
        return;
    }

    qint64 intervalMs = 0;

    if(freq == "Daily")
        intervalMs = 24LL * 60 * 60 * 1000;

    else if(freq == "Weekly")
        intervalMs = 7LL * 24 * 60 * 60 * 1000;

    else if(freq == "Monthly")
        intervalMs = 30LL * 24 * 60 * 60 * 1000;

    if(intervalMs > 0) {
        backupTimer->start(intervalMs);
        qDebug() << "Backup timer started:" << freq;
    }
}

void MainWindow::performBackup() {
    QString dbPath = m_dbManager.getDatabasePath();

    QString backupFolder = QDir::homePath() + "/InventoryBackups";
    QDir().mkpath(backupFolder);

    QString timestamp = QDateTime::currentDateTime()
                            .toString("yyyy-MM-dd_hh-mm-ss");

    QString backupPath = backupFolder + "/backup_" + timestamp + ".db";

    if(QFile::copy(dbPath, backupPath))
        qDebug() << "Backup created:" << backupPath;
    else
        qDebug() << "Backup failed";
}

void MainWindow::setupTableColumns() {
    QStringList headers;

    if(!m_catalogType.has_value()) {
        headers << "ID" << "Type" << "Name" << "Manufacturer" << "Part Number" << "Qty";
        m_ui->itemsTable->setColumnCount(6);
        m_ui->itemsTable->setHorizontalHeaderLabels(headers);
        m_ui->itemsTable->setColumnWidth(0, 20);

        m_columnPorportionalWidths[0] = 0.5f / 6;
        m_columnPorportionalWidths[1] = 0.25f; // Type column gets more space
        m_columnPorportionalWidths[2] = 0.25f; // Name column
        m_columnPorportionalWidths[3] = 0.2f;  // Manufacturer column
        m_columnPorportionalWidths[4] = 0.2f;  // Part Number column
        m_columnPorportionalWidths[5] = m_columnPorportionalWidths[0];

        return;
    }

    int numColumns = 4;
    headers << "ID" << "Manufacturer" << "Part Number";

    switch(m_catalogType.value()) {
    case ElectronicComponent::Type::Resistor:
        headers << "Resistance";
        numColumns++;
        break;
    case ElectronicComponent::Type::Capacitor:
        headers << "Type" << "Capacitance";
        numColumns += 2;
        break;
    case ElectronicComponent::Type::Inductor:
        headers << "Inductance";
        numColumns++;
        break;
    case ElectronicComponent::Type::Diode:
        headers << "Type" << "Forward Voltage";
        numColumns += 2;
        break;
    case ElectronicComponent::Type::BJTransistor:
        headers << "Gain hFE";
        numColumns++;
        break;
    case ElectronicComponent::Type::FETransistor:
        headers << "Threshold Voltage";
        numColumns++;
        break;
    case ElectronicComponent::Type::IntegratedCircuit:
        headers << "Pins";
        numColumns++;
        break;
    }

    headers << "Qty";

    m_ui->itemsTable->setColumnCount(numColumns);
    m_ui->itemsTable->setHorizontalHeaderLabels(headers);

    m_columnPorportionalWidths[0] = 0.5f / numColumns;
    for(int i = 1; i < numColumns - 1; i++)
        m_columnPorportionalWidths[i] = (1.0f - m_columnPorportionalWidths[0] * 2) / (numColumns - 2);
    m_columnPorportionalWidths[numColumns - 1] = m_columnPorportionalWidths[0];
}

void MainWindow::autoResizeTableColumns() {
    int numColumns = m_ui->itemsTable->columnCount();
    int width = m_ui->itemsTable->width();

    const int MARGIN = 25;

    width -= MARGIN;

    for(int i = 0; i < numColumns; i++) {
        int colWidth = m_columnPorportionalWidths[i] * width;
        m_ui->itemsTable->setColumnWidth(i, colWidth);
    }
}

void MainWindow::fetchDbAndPopulate() {
    setSearchFilters();
    fetchDatabase();
    populateTable();
    updatePartsFoundLabel(m_dbResult.totalNumItems);
    updatePaginator();
    updateDashboard();
}

void MainWindow::setSearchFilters() {
    if(m_searchQuery.empty()) {
        m_queryConfig.filters.reset();
        return;
    }

    auto filters = std::vector<Filter>{
        { static_cast<ComponentProperty>(ElectronicComponent::Property::Manufacturer), Filter::Operation::Contains, m_searchQuery },
        { static_cast<ComponentProperty>(ElectronicComponent::Property::PartNumber), Filter::Operation::Contains, m_searchQuery }
    };

    bool numericValueFound;
    double numericValue = s_fromStandardUnits(m_searchQuery, numericValueFound);

    if(m_catalogType.has_value() && numericValueFound) {
        std::pair<double, double> range = {
            numericValue * (1 - NUMERIC_VALUE_SEARCH_TOLERANCE),
            numericValue * (1 + NUMERIC_VALUE_SEARCH_TOLERANCE)
        };

        ComponentProperty property;

        switch(m_catalogType.value()) {
        case ElectronicComponent::Type::Resistor:
            property = static_cast<ComponentProperty>(Resistor::Property::Resistance);
            break;
        case ElectronicComponent::Type::Capacitor:
            property = static_cast<ComponentProperty>(Capacitor::Property::Capacitance);
            break;
        case ElectronicComponent::Type::Inductor:
            property = static_cast<ComponentProperty>(Inductor::Property::Inductance);
            break;
        case ElectronicComponent::Type::Diode:
            property = static_cast<ComponentProperty>(Diode::Property::ForwardVoltage);
            break;
        case ElectronicComponent::Type::BJTransistor:
            property = static_cast<ComponentProperty>(BJTransistor::Property::Gain);
            break;
        case ElectronicComponent::Type::FETransistor:
            property = static_cast<ComponentProperty>(FETransistor::Property::ThresholdVoltage);
            break;
        case ElectronicComponent::Type::IntegratedCircuit:
            property = static_cast<ComponentProperty>(IntegratedCircuit::Property::PinCount);
            break;
        default:
            property = static_cast<ComponentProperty>(ElectronicComponent::Property::Quantity);
        }

        filters.emplace_back(Filter{property, Filter::Operation::InRange, range});
    }

    m_queryConfig.filters = { std::move(filters), FilterNode::Type::Or };
}

void MainWindow::fetchDatabase() {
    if(!m_catalogType.has_value())
        m_dbResult = m_dbManager.getAllComponents(m_queryConfig);
    else
        m_dbResult = m_dbManager.getAllComponentsByType(m_catalogType.value(), m_queryConfig);
}

void MainWindow::populateComponentRow(int row, const std::unique_ptr<ElectronicComponent>& component) {
    int col = 1; // Skip ID column (already set)

    if(!m_catalogType.has_value()) {
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_componentTypeAsString(component->type())));
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(component->name()));
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(component->manufacturer()));
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(component->partNumber()));
        m_ui->itemsTable->setItem(row, col++, s_newTableItemi(component->quantity()));
        return;
    }

    m_ui->itemsTable->setItem(row, col++, s_newTableItem(component->manufacturer()));
    m_ui->itemsTable->setItem(row, col++, s_newTableItem(component->partNumber()));

    switch(m_catalogType.value()) {
    case ElectronicComponent::Type::Resistor: {
        auto* r = dynamic_cast<Resistor*>(component.get());
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_toStandardUnits(r->resistance(), "Ω")));
        break;
    }
    case ElectronicComponent::Type::Capacitor: {
        auto* c = dynamic_cast<Capacitor*>(component.get());
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_capacitorTypeAsString(c->capacitorType())));
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_toStandardUnits(c->capacitance(), "F")));
        break;
    }
    case ElectronicComponent::Type::Inductor: {
        auto* i = dynamic_cast<Inductor*>(component.get());
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_toStandardUnits(i->inductance(), "H")));
        break;
    }
    case ElectronicComponent::Type::Diode: {
        auto* d = dynamic_cast<Diode*>(component.get());
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_diodeTypeAsString(d->diodeType())));
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_toStandardUnits(d->forwardVoltage(), "V")));
        break;
    }
    case ElectronicComponent::Type::BJTransistor: {
        auto* t = dynamic_cast<BJTransistor*>(component.get());
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_toStandardUnits(t->gain(), "")));
        break;
    }
    case ElectronicComponent::Type::FETransistor: {
        auto* t = dynamic_cast<FETransistor*>(component.get());
        m_ui->itemsTable->setItem(row, col++, s_newTableItem(s_toStandardUnits(t->thresholdVoltage(), "V")));
        break;
    }
    case ElectronicComponent::Type::IntegratedCircuit: {
        auto* ic = dynamic_cast<IntegratedCircuit*>(component.get());
        m_ui->itemsTable->setItem(row, col++, s_newTableItemi(ic->pinCount()));
        break;
    }
    }

    m_ui->itemsTable->setItem(row, col++, s_newTableItemi(component->quantity()));
}

void MainWindow::populateTable() {
    QTableWidget* const table = m_ui->itemsTable;
    table->setRowCount(0);

    for(const auto& item : m_dbResult.items) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, s_newTableItemi(item->ID()));
        populateComponentRow(row, item);
    }
}

void MainWindow::onSearch() {
    m_searchQuery = m_ui->searchBar->text().toStdString();

    fetchDbAndPopulate();
}

std::unique_ptr<ElectronicComponent> MainWindow::createComponentFromDialog(
    ElectronicComponent::Type type,
    const ElectronicComponent::BaseConfig& base,
    const AddItemDialog& dialog)
{
    switch(type) {
    case ElectronicComponent::Type::Resistor:
        return std::make_unique<Resistor>(base, dialog.getResistance(), dialog.getTolerance());
    case ElectronicComponent::Type::Capacitor:
        return std::make_unique<Capacitor>(base, dialog.getCapacitorType(), dialog.getCapacitance());
    case ElectronicComponent::Type::Inductor:
        return std::make_unique<Inductor>(base, dialog.getInductance());
    case ElectronicComponent::Type::Diode:
        return std::make_unique<Diode>(base, dialog.getForwardVoltage(), dialog.getDiodeType());
    case ElectronicComponent::Type::BJTransistor:
        return std::make_unique<BJTransistor>(base, dialog.getGain());
    case ElectronicComponent::Type::FETransistor:
        return std::make_unique<FETransistor>(base, dialog.getThresholdVoltage());
    case ElectronicComponent::Type::IntegratedCircuit:
        return std::make_unique<IntegratedCircuit>(base, dialog.getPinCount(), 0.0, 0.0, 0.0);
    }
    return nullptr;
}

void MainWindow::addItem(const QString &name, int quantity, const QString &part_num, const QString &image_path) {
    AddItemDialog dialog(this);

    if(dialog.exec() != QDialog::Accepted)
        return;

    ElectronicComponent::BaseConfig base = {
        .rating = {},
        .name = dialog.getName().trimmed().toStdString(),
        .manufacturer = dialog.getManufacturer().toStdString(),
        .partNumber = dialog.getPartNumber().toStdString(),
        .description = dialog.getImagePath().toStdString(),
        .quantity = static_cast<size_t>(dialog.getQuantity())
    };

    try {
        auto component = createComponentFromDialog(dialog.getComponentType(), base, dialog);
        if(component) {
            m_dbManager.addComponent(*component);
        }
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Add Item", QString("Could not add item: %1").arg(e.what()));
        return;
    }

    fetchDbAndPopulate();
    updateDashboard();
}

void MainWindow::deleteItem(int row) {
    try {
        // Get the component ID from the first column
        QTableWidgetItem* idItem = m_ui->itemsTable->item(row, 0);
        if(!idItem) {
            QMessageBox::critical(this, "Delete Item", "Could not get item ID.");
            return;
        }

        ComponentID id = idItem->data(Qt::DisplayRole).toInt();
        m_dbManager.removeComponent(id);
        fetchDbAndPopulate();
        updateDashboard();
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Delete Item", QString("Could not delete item: %1").arg(e.what()));
    }
}

void MainWindow::openResistorDividerTool() {
    QDialog dividerDialog(this, Qt::Popup | Qt::FramelessWindowHint);
    dividerDialog.setWindowTitle("Resistor Divider Calculator");
    dividerDialog.setMinimumSize(400, 300);
    
    QVBoxLayout* layout = new QVBoxLayout(&dividerDialog);
    
    QLabel* inputLabel = new QLabel("Input Voltage (V):", &dividerDialog);
    QLineEdit* inputVoltage = new QLineEdit(&dividerDialog);
    
    QLabel* outputLabel = new QLabel("Output Voltage (V):", &dividerDialog);
    QLineEdit* outputVoltage = new QLineEdit(&dividerDialog);
    
    QPushButton* calculateButton = new QPushButton("Calculate", &dividerDialog);
    QLabel* resultLabel = new QLabel("Result: --", &dividerDialog);
    resultLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(inputLabel);
    layout->addWidget(inputVoltage);
    layout->addWidget(outputLabel);
    layout->addWidget(outputVoltage);
    layout->addWidget(calculateButton);
    layout->addWidget(resultLabel);
    
    // Center the popup
    if(parentWidget()) {
        dividerDialog.move(parentWidget()->x() + (parentWidget()->width() - dividerDialog.width()) / 2,
                         parentWidget()->y() + (parentWidget()->height() - dividerDialog.height()) / 2);
    } else {
        QScreen* screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - dividerDialog.width()) / 2;
        int y = (screenGeometry.height() - dividerDialog.height()) / 2;
        dividerDialog.move(x, y);
    }

    connect(calculateButton, &QPushButton::clicked, [&]() {
        bool ok1, ok2;
        double vin = inputVoltage->text().toDouble(&ok1);
        double vout = outputVoltage->text().toDouble(&ok2);
        
        if(!ok1 || !ok2 || vin <= 0) {
            resultLabel->setText("Invalid input voltage");
            return;
        }
        
        if(vout >= vin) {
            resultLabel->setText("Output must be less than input");
            return;
        }
        
        double ratio = vout / vin;
        resultLabel->setText(QString("Ratio: %1 (R2/(R1+R2))").arg(ratio, 0, 'f', 3));
    });

    dividerDialog.exec();
}

void MainWindow::updateDashboard() {
    try {
        int totalParts = 0;
        int resistors = 0;
        int capacitors = 0;
        int inductors = 0;
        int ics = 0;

        for(const auto& item : m_dbResult.items) {
            totalParts++;
            switch(item->type()) {
                case ElectronicComponent::Type::Resistor:
                    resistors++;
                    break;
                case ElectronicComponent::Type::Capacitor:
                    capacitors++;
                    break;
                case ElectronicComponent::Type::Inductor:
                    inductors++;
                    break;
                case ElectronicComponent::Type::IntegratedCircuit:
                    ics++;
                    break;
                default:
                    break;
            }
        }

        m_ui->totalPartsValue->setText(QString::number(totalParts));
        m_ui->resistorsValue->setText(QString::number(resistors));
        m_ui->capacitorsValue->setText(QString::number(capacitors));
        m_ui->inductorsValue->setText(QString::number(inductors));
        m_ui->icsValue->setText(QString::number(ics));
    }
    catch(const std::exception& e) {
        qDebug() << "Error updating dashboard: " << e.what();
    }
}

void MainWindow::openItemView(int row, int column) {
    (void)column;

    auto* idCell = m_ui->itemsTable->item(row, 0);

    if(!idCell)
        return;

    ComponentID id = static_cast<ComponentID>(idCell->data(Qt::DisplayRole).toULongLong());

    std::unique_ptr<ElectronicComponent> component;

    try {
        component = m_dbManager.getComponent(id);
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "View Item", QString("Could not load item: %1").arg(e.what()));
        return;
    }

    if(!component) {
        QMessageBox::warning(this, "View Item", "The selected item no longer exists.");
        fetchDbAndPopulate();
        return;
    }

    ViewItemDialog dialog(this);

    // Get component type as string
    QString componentTypeStr;
    switch(component->type()) {
    case ElectronicComponent::Type::Resistor:
        componentTypeStr = "Resistor";
        break;
    case ElectronicComponent::Type::Capacitor:
        componentTypeStr = "Capacitor";
        break;
    case ElectronicComponent::Type::Inductor:
        componentTypeStr = "Inductor";
        break;
    case ElectronicComponent::Type::Diode:
        componentTypeStr = "Diode";
        break;
    case ElectronicComponent::Type::BJTransistor:
        componentTypeStr = "BJ Transistor";
        break;
    case ElectronicComponent::Type::FETransistor:
        componentTypeStr = "FE Transistor";
        break;
    case ElectronicComponent::Type::IntegratedCircuit:
        componentTypeStr = "Integrated Circuit";
        break;
    }

    QString manufacturer = QString(component->manufacturer().c_str());
    QString property1, property2;

    // Get component-specific properties
    switch(component->type()) {
    case ElectronicComponent::Type::Resistor: {
        auto* typed = dynamic_cast<Resistor*>(component.get());
        if(typed) {
            property1 = QString::number(typed->resistance()) + " Ω";
            property2 = QString::number(typed->toleranceBand()) + " %";
        }
        break;
    }
    case ElectronicComponent::Type::Capacitor: {
        auto* typed = dynamic_cast<Capacitor*>(component.get());
        if(typed) {
            property1 = QString::number(typed->capacitance()) + " F";
            property2 = "-";
        }
        break;
    }
    case ElectronicComponent::Type::Inductor: {
        auto* typed = dynamic_cast<Inductor*>(component.get());
        if(typed) {
            property1 = QString::number(typed->inductance()) + " H";
            property2 = "-";
        }
        break;
    }
    case ElectronicComponent::Type::Diode: {
        auto* typed = dynamic_cast<Diode*>(component.get());
        if(typed) {
            property1 = QString::number(typed->forwardVoltage()) + " V";
            property2 = "-";
        }
        break;
    }
    case ElectronicComponent::Type::BJTransistor: {
        auto* typed = dynamic_cast<BJTransistor*>(component.get());
        if(typed) {
            property1 = QString::number(typed->gain()) + " hFE";
            property2 = "-";
        }
        break;
    }
    case ElectronicComponent::Type::FETransistor: {
        auto* typed = dynamic_cast<FETransistor*>(component.get());
        if(typed) {
            property1 = QString::number(typed->thresholdVoltage()) + " V";
            property2 = "-";
        }
        break;
    }
    case ElectronicComponent::Type::IntegratedCircuit: {
        auto* typed = dynamic_cast<IntegratedCircuit*>(component.get());
        if(typed) {
            property1 = QString::number(typed->pinCount()) + " pins";
            property2 = QString::number(typed->width()) + "x" + QString::number(typed->height()) + "x" + QString::number(typed->length()) + " mm";
        }
        break;
    }
    }

    dialog.setItemData(
        component->name().c_str(),
        static_cast<int>(component->quantity()),
        QString(component->partNumber().c_str()),
        component->description().c_str(),
        componentTypeStr,
        manufacturer,
        property1,
        property2
    );
    dialog.exec();
}

void MainWindow::updateComponentFromDialog(
    ComponentID id,
    const std::unique_ptr<ElectronicComponent>& component,
    const ElectronicComponent::BaseConfig& base)
{
    switch(component->type()) {
    case ElectronicComponent::Type::Resistor: {
        auto* typed = dynamic_cast<Resistor*>(component.get());
        if(!typed)
            throw std::runtime_error("Internal type mismatch for resistor");
        Resistor updated(base, typed->resistance(), typed->toleranceBand());
        m_dbManager.editComponent(id, updated);
        break;
    }
    case ElectronicComponent::Type::Capacitor: {
        auto* typed = dynamic_cast<Capacitor*>(component.get());
        if(!typed)
            throw std::runtime_error("Internal type mismatch for capacitor");
        Capacitor updated(base, typed->capacitorType(), typed->capacitance());
        m_dbManager.editComponent(id, updated);
        break;
    }
    case ElectronicComponent::Type::Inductor: {
        auto* typed = dynamic_cast<Inductor*>(component.get());
        if(!typed)
            throw std::runtime_error("Internal type mismatch for inductor");
        Inductor updated(base, typed->inductance());
        m_dbManager.editComponent(id, updated);
        break;
    }
    case ElectronicComponent::Type::Diode: {
        auto* typed = dynamic_cast<Diode*>(component.get());
        if(!typed)
            throw std::runtime_error("Internal type mismatch for diode");
        Diode updated(base, typed->forwardVoltage(), typed->diodeType());
        m_dbManager.editComponent(id, updated);
        break;
    }
    case ElectronicComponent::Type::BJTransistor: {
        auto* typed = dynamic_cast<BJTransistor*>(component.get());
        if(!typed)
            throw std::runtime_error("Internal type mismatch for BJ transistor");
        BJTransistor updated(base, typed->gain());
        m_dbManager.editComponent(id, updated);
        break;
    }
    case ElectronicComponent::Type::FETransistor: {
        auto* typed = dynamic_cast<FETransistor*>(component.get());
        if(!typed)
            throw std::runtime_error("Internal type mismatch for FE transistor");
        FETransistor updated(base, typed->thresholdVoltage());
        m_dbManager.editComponent(id, updated);
        break;
    }
    case ElectronicComponent::Type::IntegratedCircuit: {
        auto* typed = dynamic_cast<IntegratedCircuit*>(component.get());
        if(!typed)
            throw std::runtime_error("Internal type mismatch for integrated circuit");
        IntegratedCircuit updated(base, typed->pinCount(), typed->width(), typed->height(), typed->length());
        m_dbManager.editComponent(id, updated);
        break;
    }
    }
}

void MainWindow::openItemEdit(int row) {
    auto* idCell = m_ui->itemsTable->item(row, 0);

    if(!idCell)
        return;

    ComponentID id = static_cast<ComponentID>(idCell->data(Qt::DisplayRole).toULongLong());

    std::unique_ptr<ElectronicComponent> component;

    try {
        component = m_dbManager.getComponent(id);
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Edit Item", QString("Could not load item: %1").arg(e.what()));
        return;
    }

    if(!component) {
        QMessageBox::warning(this, "Edit Item", "The selected item no longer exists.");
        fetchDbAndPopulate();
        return;
    }

    EditItemDialog dialog(this);
    dialog.setItemData(
        component->name().c_str(),
        static_cast<int>(component->quantity()),
        QString(component->partNumber().c_str()),
        component->description().c_str()
    );

    connect(&dialog, &EditItemDialog::deleteRequested, this, [this, id](const QString &) {
        try {
            m_dbManager.removeComponent(id);
            fetchDbAndPopulate();
        }
        catch(const std::exception& e) {
            QMessageBox::critical(this, "Delete Item", QString("Could not delete item: %1").arg(e.what()));
        }
    });

    if(dialog.exec() != QDialog::Accepted)
        return;

    ElectronicComponent::BaseConfig base = {
        .rating = component->rating(),
        .name = dialog.getName().trimmed().toStdString(),
        .manufacturer = component->manufacturer(),
        .partNumber = dialog.getPartNumber().toStdString(),
        .description = dialog.getImagePath().toStdString(),
        .quantity = static_cast<size_t>(dialog.getQuantity()),
        .ID = component->ID()
    };

    try {
        updateComponentFromDialog(id, component, base);
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Edit Item", QString("Could not save item changes: %1").arg(e.what()));
        return;
    }

    fetchDbAndPopulate();
}

void MainWindow::onChangeCatalog(int selectionIndex) {
    if(selectionIndex == 0)
        m_catalogType = std::optional<ElectronicComponent::Type>();
    else {
        selectionIndex = std::min(std::max(1, selectionIndex), static_cast<int>(ElectronicComponent::Type::IntegratedCircuit));
        m_catalogType = static_cast<ElectronicComponent::Type>(selectionIndex);
    }

    setupTableColumns();
    autoResizeTableColumns();

    fetchDbAndPopulate();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    autoResizeTableColumns();
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);

    autoResizeTableColumns();

    fetchDbAndPopulate();
}

static const char* s_componentTypeAsString(ElectronicComponent::Type type) {
    switch(type) {
    case ElectronicComponent::Type::Resistor:
        return "Resistor";
    case ElectronicComponent::Type::Capacitor:
        return "Capacitor";
    case ElectronicComponent::Type::Inductor:
        return "Inductor";
    case ElectronicComponent::Type::Diode:
        return "Diode";
    case ElectronicComponent::Type::BJTransistor:
        return "BJ-Transistor";
    case ElectronicComponent::Type::FETransistor:
        return "FE-Transistor";
    case ElectronicComponent::Type::IntegratedCircuit:
        return "Integrated Circuit";
    }

    return "Generic";
}

static const char* s_capacitorTypeAsString(Capacitor::Type type) {
    switch(type) {
    case Capacitor::Type::AluminumPolymer:
        return "Aluminum Polymer";
    case Capacitor::Type::AluminumElectrolytic:
        return "Aluminum Electrolytic";
    case Capacitor::Type::Ceramic:
        return "Ceramic";
    case Capacitor::Type::ElectricDoubleLayer:
        return "Electric Double Layer";
    case Capacitor::Type::Film:
        return "Film";
    case Capacitor::Type::Mica:
        return "Mica";
    case Capacitor::Type::PTFE:
        return "PTFE";
    case Capacitor::Type::NiobiumOxide:
        return "Niobium Oxide";
    case Capacitor::Type::Silicon:
        return "Silicon";
    case Capacitor::Type::Tantalum:
        return "Tantalum";
    case Capacitor::Type::ThinFilm:
        return "Thin Film";
    case Capacitor::Type::ACMotor:
        return "AC Motor";
    case Capacitor::Type::LithiumHybrid:
        return "Lithium Hybrid";
    }

    return "Unknown";
}

static const char* s_diodeTypeAsString(Diode::Type type) {
    switch(type) {
    case Diode::Type::Regular:
        return "Regular";
    case Diode::Type::Schottky:
        return "Schottky";
    case Diode::Type::Zener:
        return "Zener";
    case Diode::Type::LED:
        return "LED";
    }

    return "Unknown";
}

static std::string s_toStandardUnits(double value, const char* unitSuffix) {
    char buffer[32];

    if (value >= GIGA)
        snprintf(buffer, sizeof(buffer), "%.4g G%s", value / GIGA, unitSuffix);
    else if (value >= MEGA)
        snprintf(buffer, sizeof(buffer), "%.4g M%s", value / MEGA, unitSuffix);
    else if (value >= KILO)
        snprintf(buffer, sizeof(buffer), "%.4g k%s", value / KILO, unitSuffix);
    else if (value >= 1.0)
        snprintf(buffer, sizeof(buffer), "%.4g %s", value, unitSuffix);
    else if (value >= MILLI)
        snprintf(buffer, sizeof(buffer), "%.4g m%s", value / MILLI, unitSuffix);
    else if (value >= MICRO)
        snprintf(buffer, sizeof(buffer), "%.4g µ%s", value / MICRO, unitSuffix);
    else if (value >= NANO)
        snprintf(buffer, sizeof(buffer), "%.4g n%s", value / NANO, unitSuffix);
    else if (value >= PICO)
        snprintf(buffer, sizeof(buffer), "%.4g n%s", value / PICO, unitSuffix);
    else
        snprintf(buffer, sizeof(buffer), "%.4g %s", value, unitSuffix);

    return std::string(buffer);
}

static double s_fromStandardUnits(const std::string& value, bool& matched) {
    const std::regex pattern(R"(([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*([pnumkMG]?)\s*[a-z%A-ZΩ°]*)");
    std::smatch match;

    matched = false;

    if(!std::regex_match(value, match, pattern))
        return 0.0;

    matched = true;

    double number = std::stod(match[1]);
    const auto& suffix = match[2].str();
    if(suffix.empty())
        return number;
    else if(suffix[0] == 'p')
        number *= PICO;
    else if(suffix[0] == 'n')
        number *= NANO;
    else if(suffix[0] == 'u')
        number *= MICRO;
    else if(suffix[0] == 'm')
        number *= MILLI;
    else if(suffix[0] == '%')
        number *= PERCENT;
    else if(suffix[0] == 'k')
        number *= KILO;
    else if(suffix[0] == 'M')
        number *= MEGA;
    else if(suffix[0] == 'G')
        number *= GIGA;

    return number;
}

static QTableWidgetItem* s_newTableItemi(int value) {
    auto newItem = new QTableWidgetItem();
    newItem->setData(Qt::DisplayRole, value);
    return newItem;
}

static QTableWidgetItem* s_newTableItemd(double value) {
    auto newItem = new QTableWidgetItem();
    newItem->setData(Qt::DisplayRole, value);
    return newItem;
}

static QTableWidgetItem* s_newTableItem(const std::string& value) {
    return new QTableWidgetItem(value.c_str());
}

static QTableWidgetItem* s_newTableItem(const char* value) {
    return new QTableWidgetItem(value);
}
