#include "ui/MainWindow.hpp"
#include "database/MassQueryConfig.hpp"
#include "electrical/ElectronicComponents.hpp"
#include "ui_MainWindow.h"

#include "ui/AddItemDialog.hpp"
#include "ui/ViewItemDialog.hpp"
#include "ui/EditItemDialog.hpp"
#include "ui/Settings.hpp"

#include "Config.hpp"

#include <QMessageBox>
#include <QSettings>
#include <QDir>

using namespace ecim;

#define ITEMS_PER_PAGE 20

static const char* s_componentTypeAsString(ElectronicComponent::Type type);
static const char* s_capacitorTypeAsString(Capacitor::Type type);
static const char* s_diodeTypeAsString(Diode::Type type);
static std::string s_toStandardUnits(double value, const char* unitSuffix);
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

    connect(m_ui->searchBar, &QLineEdit::returnPressed,
            this, &MainWindow::onSearchEnterPressed);

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
    numPages->setText(QString("/ %1").arg(m_dbResult.numPages));

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
        fetchDatabase();
        populateTable();
    });

    connect(previousPage, &QAbstractButton::clicked, [this]() {
        m_queryConfig.pagination = Pagination(m_dbResult.currentPage - 1, ITEMS_PER_PAGE);
        fetchDatabase();
        populateTable();
    });

    connect(pageNumber, &QLineEdit::returnPressed, [this]() {
        int page = m_ui->pageNumber->text().toInt();
        m_queryConfig.pagination = Pagination(page, ITEMS_PER_PAGE);
        fetchDatabase();
        populateTable();
    });

    connect(nextPage, &QAbstractButton::clicked, [this]() {
        m_queryConfig.pagination = Pagination(m_dbResult.currentPage + 1, ITEMS_PER_PAGE);
        fetchDatabase();
        populateTable();
    });

    connect(lastPage, &QAbstractButton::clicked, [this]() {
        m_queryConfig.pagination = Pagination(m_dbResult.numPages, ITEMS_PER_PAGE);
        fetchDatabase();
        populateTable();
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
        for(int i = 1; i < 5; i++)
            m_columnPorportionalWidths[i] = (1.0f - m_columnPorportionalWidths[0] * 2) / 4;
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

void MainWindow::fetchDatabase() {
    if(!m_catalogType.has_value())
        m_dbResult = m_dbManager.getAllComponents(m_queryConfig);
    else
        m_dbResult = m_dbManager.getAllComponentsByType(m_catalogType.value(), m_queryConfig);

    updatePartsFoundLabel(m_dbResult.totalNumItems);
    updatePaginator();
}

void MainWindow::populateTable() {
    QTableWidget* const table = m_ui->itemsTable;

    table->setRowCount(0);

    for(const auto& item : m_dbResult.items) {
        int row = table->rowCount();
        int col = 0;

        table->insertRow(row);

        table->setItem(row, col++, s_newTableItemi(item->ID()));

        if(!m_catalogType.has_value()) {
            table->setItem(row, col++, s_newTableItem( s_componentTypeAsString(item->type()) ));
            table->setItem(row, col++, s_newTableItem(item->name()));
            table->setItem(row, col++, s_newTableItem(item->manufacturer()));
            table->setItem(row, col++, s_newTableItem(item->partNumber()));
            table->setItem(row, col++, s_newTableItemi(item->quantity()));
            continue;
        }

        table->setItem(row, col++, s_newTableItem(item->manufacturer()));
        table->setItem(row, col++, s_newTableItem(item->partNumber()));

        switch(m_catalogType.value()) {
        case ElectronicComponent::Type::Resistor: {
            Resistor* r = dynamic_cast<Resistor*>(item.get());

            table->setItem(row, col++, s_newTableItem(s_toStandardUnits(r->resistance(), "Ω")));
            break;
        }
        case ElectronicComponent::Type::Capacitor: {
            Capacitor* c = dynamic_cast<Capacitor*>(item.get());

            table->setItem(row, col++, s_newTableItem(s_capacitorTypeAsString(c->capacitorType())));
            table->setItem(row, col++, s_newTableItem(s_toStandardUnits(c->capacitance(), "F")));
            break;
        }
        case ElectronicComponent::Type::Inductor: {
            Inductor* i = dynamic_cast<Inductor*>(item.get());

            table->setItem(row, col++, s_newTableItem(s_toStandardUnits(i->inductance(), "H")));
            break;
        }
        case ElectronicComponent::Type::Diode: {
            Diode* d = dynamic_cast<Diode*>(item.get());

            table->setItem(row, col++, s_newTableItem(s_diodeTypeAsString(d->diodeType())));
            table->setItem(row, col++, s_newTableItem(s_toStandardUnits(d->forwardVoltage(), "V")));
            break;
        }
        case ElectronicComponent::Type::BJTransistor: {
            BJTransistor* t = dynamic_cast<BJTransistor*>(item.get());

            table->setItem(row, col++, s_newTableItem(s_toStandardUnits(t->gain(), "")));
            break;
        }
        case ElectronicComponent::Type::FETransistor: {
            FETransistor* t = dynamic_cast<FETransistor*>(item.get());

            table->setItem(row, col++, s_newTableItem(s_toStandardUnits(t->thresholdVoltage(), "V")));
            break;
        }
        case ElectronicComponent::Type::IntegratedCircuit: {
            IntegratedCircuit* ic = dynamic_cast<IntegratedCircuit*>(item.get());

            table->setItem(row, col++, s_newTableItemi(ic->pinCount()));
            break;
        }
        }

        m_ui->itemsTable->setItem(row, col++, s_newTableItemi(item->quantity()));
    }
}

void MainWindow::onSearchEnterPressed() {
    std::string searchQuery = m_ui->searchBar->text().toStdString();

    m_queryConfig.filters = {
        std::vector<Filter>{
            { static_cast<ComponentProperty>(ElectronicComponent::Property::Manufacturer), Filter::Operation::Contains, searchQuery },
            { static_cast<ComponentProperty>(ElectronicComponent::Property::PartNumber), Filter::Operation::Contains, searchQuery }
        }, FilterNode::Type::Or
    };

    fetchDatabase();
    populateTable();
}

void MainWindow::addItem(const QString &name, int quantity, int part_num, const QString &image_path) {
}

void MainWindow::openItemView(int row, int column) {
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

    fetchDatabase();
    populateTable();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    autoResizeTableColumns();
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);

    autoResizeTableColumns();

    fetchDatabase();
    populateTable();
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
    constexpr double KILO = 1e3;
    constexpr double MEGA = 1e6;
    constexpr double GIGA = 1e9;
    constexpr double MILLI = 1e-3;
    constexpr double MICRO = 1e-6;
    constexpr double NANO = 1e-9;
    constexpr double PICO = 1e-12;

    char buffer[32];

    if (value >= GIGA) {
        snprintf(buffer, sizeof(buffer), "%.4g G%s", value / GIGA, unitSuffix);
    } else if (value >= MEGA) {
        snprintf(buffer, sizeof(buffer), "%.4g M%s", value / MEGA, unitSuffix);
    } else if (value >= KILO) {
        snprintf(buffer, sizeof(buffer), "%.4g k%s", value / KILO, unitSuffix);
    } else if (value >= 1.0) {
        snprintf(buffer, sizeof(buffer), "%.4g %s", value, unitSuffix);
    } else if (value >= MILLI) {
        snprintf(buffer, sizeof(buffer), "%.4g m%s", value / MILLI, unitSuffix);
    } else if (value >= MICRO) {
        snprintf(buffer, sizeof(buffer), "%.4g µ%s", value / MICRO, unitSuffix);
    } else if (value >= NANO) {
        snprintf(buffer, sizeof(buffer), "%.4g n%s", value / NANO, unitSuffix);
    } else if (value >= PICO) {
        snprintf(buffer, sizeof(buffer), "%.4g n%s", value / PICO, unitSuffix);
    } else {
        snprintf(buffer, sizeof(buffer), "%.4g %s", value, unitSuffix);
    }

    return std::string(buffer);
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
