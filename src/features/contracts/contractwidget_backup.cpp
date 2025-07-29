#include "contractwidget.h"
#include "contract.h"
#include "contractdialog.h"
#include "contractdatabasemanager.h"
#include "contractexportmanager.h"
#include "contractchatbotdialog.h"
#include "contractimportdialog.h"
#include "interfaces/icontractchatbot.h"
#include "interfaces/icontractimporter.h"
#include "utils/stylemanager.h"
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>
#include <QGroupBox>
#include <QDebug>
#include <QLocale>
#include <QApplication>
#include <QKeySequence>
#include <QShortcut>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QCursor>

ContractWidget::ContractWidget(QWidget *parent)
    : QWidget(parent)
    , m_dbManager(nullptr)
    , m_contractService(nullptr)
    , m_exportManager(new ContractExportManager(this))
    , m_chatbot(nullptr)
    , m_importer(nullptr)
    , m_searchTimer(new QTimer(this))
    , m_isLoading(false)
{
    setupUi();
    setupConnections();
    applyArchiFlowStyling();
    
    // Configure search timer for delayed search
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300); // 300ms delay
    connect(m_searchTimer, &QTimer::timeout, this, &ContractWidget::applyFilters);
    
    updateActionStates();
}

ContractWidget::~ContractWidget()
{
    // Clean up contracts
    qDeleteAll(m_contracts);
}

void ContractWidget::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // Setup components
    setupToolbar();
    setupSearchAndFilter();
    setupTable();
    setupStatusBar();
}

void ContractWidget::setupToolbar()
{
    m_toolbar = new QToolBar;
    m_toolbar->setObjectName("contractToolbar");
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Add contract actions
    m_addButton = new QPushButton("Add Contract");
    m_addButton->setObjectName("primaryButton");
    m_addButton->setIcon(QIcon(":/icons/add.png"));
    m_addButton->setShortcut(QKeySequence::New);
    m_toolbar->addWidget(m_addButton);

    m_toolbar->addSeparator();

    m_editButton = new QPushButton("Edit");
    m_editButton->setIcon(QIcon(":/icons/edit.png"));
    m_editButton->setShortcut(QKeySequence("F2"));
    m_toolbar->addWidget(m_editButton);

    m_deleteButton = new QPushButton("Delete");
    m_deleteButton->setIcon(QIcon(":/icons/delete.png"));
    m_deleteButton->setShortcut(QKeySequence::Delete);
    m_toolbar->addWidget(m_deleteButton);

    m_duplicateButton = new QPushButton("Duplicate");
    m_duplicateButton->setIcon(QIcon(":/icons/copy.png"));
    m_duplicateButton->setShortcut(QKeySequence("Ctrl+D"));
    m_toolbar->addWidget(m_duplicateButton);

    m_toolbar->addSeparator();
    
    m_viewDetailsButton = new QPushButton("View Details");
    m_viewDetailsButton->setIcon(QIcon(":/icons/view.png"));
    m_viewDetailsButton->setShortcut(QKeySequence("Enter"));
    m_toolbar->addWidget(m_viewDetailsButton);

    m_toolbar->addSeparator();

    // Statistics and analysis
    m_statisticsButton = new QPushButton("Statistics");
    m_statisticsButton->setIcon(QIcon(":/icons/chart.png"));
    m_statisticsButton->setShortcut(QKeySequence("F9"));
    m_toolbar->addWidget(m_statisticsButton);

    m_toolbar->addSeparator();

    // Import and Export operations
    m_importButton = new QPushButton("Import");
    m_importButton->setIcon(QIcon(":/icons/import.png"));
    m_importButton->setShortcut(QKeySequence("Ctrl+I"));
    m_toolbar->addWidget(m_importButton);

    // Export menu button with dropdown
    m_exportButton = new QPushButton("Export");
    m_exportButton->setIcon(QIcon(":/icons/export.png"));
    m_exportButton->setShortcut(QKeySequence("Ctrl+E"));
    
    QMenu *exportMenu = new QMenu(m_exportButton);
    exportMenu->addAction("Export to CSV", this, &ContractWidget::onExportToCSV);
    exportMenu->addAction("Export to PDF", this, &ContractWidget::onExportToPDF);
    exportMenu->addAction("Export to Excel", this, &ContractWidget::onExportToExcel);
    exportMenu->addAction("Export to JSON", this, &ContractWidget::onExportToJSON);
    exportMenu->addSeparator();
    exportMenu->addAction("Advanced Export...", this, &ContractWidget::onShowExportDialog);
    m_exportButton->setMenu(exportMenu);
    m_toolbar->addWidget(m_exportButton);

    m_toolbar->addSeparator();

    // AI and Advanced features
    m_chatbotButton = new QPushButton("AI Assistant");
    m_chatbotButton->setIcon(QIcon(":/icons/chatbot.png"));
    m_chatbotButton->setShortcut(QKeySequence("F8"));
    m_toolbar->addWidget(m_chatbotButton);

    m_advancedSearchButton = new QPushButton("Advanced Search");
    m_advancedSearchButton->setIcon(QIcon(":/icons/search.png"));
    m_advancedSearchButton->setShortcut(QKeySequence("Ctrl+F"));
    m_toolbar->addWidget(m_advancedSearchButton);    m_bulkOperationsButton = new QPushButton("Bulk Operations");
    m_bulkOperationsButton->setIcon(QIcon(":/icons/bulk.png"));
    m_bulkOperationsButton->setShortcut(QKeySequence("Ctrl+B"));
    m_toolbar->addWidget(m_bulkOperationsButton);

    m_toolbar->addSeparator();

    // Debug button for database diagnostics
    m_diagnosticsButton = new QPushButton("DB Diagnostics");
    m_diagnosticsButton->setIcon(QIcon(":/icons/view.png"));
    m_diagnosticsButton->setShortcut(QKeySequence("F12"));
    m_toolbar->addWidget(m_diagnosticsButton);

    m_mainLayout->addWidget(m_toolbar);
}

void ContractWidget::setupSearchAndFilter()
{
    // Filter group
    QGroupBox *filterGroup = new QGroupBox("Search & Filter");
    filterGroup->setObjectName("filterGroup");
    
    QHBoxLayout *filterLayout = new QHBoxLayout(filterGroup);
    filterLayout->setSpacing(15);

    // Search field
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search contracts by client name, status, or description...");
    m_searchEdit->setClearButtonEnabled(true);
    filterLayout->addWidget(m_searchEdit, 2);

    // Status filter
    m_statusFilterCombo = new QComboBox;
    m_statusFilterCombo->addItem("All Statuses", "");
    for (const QString &status : Contract::availableStatuses()) {
        m_statusFilterCombo->addItem(status, status);
    }
    filterLayout->addWidget(m_statusFilterCombo);

    // Date range filters
    filterLayout->addWidget(new QLabel("From:"));
    m_startDateFilter = new QDateEdit;
    m_startDateFilter->setDate(QDate::currentDate().addYears(-1));
    m_startDateFilter->setCalendarPopup(true);
    m_startDateFilter->setDisplayFormat("dd/MM/yyyy");
    filterLayout->addWidget(m_startDateFilter);

    filterLayout->addWidget(new QLabel("To:"));
    m_endDateFilter = new QDateEdit;
    m_endDateFilter->setDate(QDate::currentDate().addYears(1));
    m_endDateFilter->setCalendarPopup(true);
    m_endDateFilter->setDisplayFormat("dd/MM/yyyy");
    filterLayout->addWidget(m_endDateFilter);

    // Clear filters button
    m_clearFiltersButton = new QPushButton("Clear Filters");
    m_clearFiltersButton->setIcon(QIcon(":/icons/clear.png"));
    filterLayout->addWidget(m_clearFiltersButton);

    m_mainLayout->addWidget(filterGroup);
}

void ContractWidget::setupTable()
{
    m_contractsTable = new QTableWidget;
    m_contractsTable->setObjectName("contractsTable");
    m_contractsTable->setColumnCount(ColumnCount);
    
    // Set headers
    QStringList headers = {
        "Client Name", "Start Date", "End Date", "Value", 
        "Status", "Payment Terms", "Description"
    };
    m_contractsTable->setHorizontalHeaderLabels(headers);

    // Configure table properties
    m_contractsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_contractsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_contractsTable->setAlternatingRowColors(true);
    m_contractsTable->setSortingEnabled(true);
    m_contractsTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // Configure column widths
    QHeaderView *header = m_contractsTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(ClientNameColumn, 150);
    header->resizeSection(StartDateColumn, 100);
    header->resizeSection(EndDateColumn, 100);
    header->resizeSection(ValueColumn, 120);
    header->resizeSection(StatusColumn, 100);
    header->resizeSection(PaymentTermsColumn, 100);

    m_mainLayout->addWidget(m_contractsTable, 1);
}

void ContractWidget::setupStatusBar()
{
    m_statusWidget = new QWidget;
    QHBoxLayout *statusLayout = new QHBoxLayout(m_statusWidget);
    statusLayout->setContentsMargins(0, 5, 0, 5);

    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setObjectName("statusLabel");
    statusLayout->addWidget(m_statusLabel);

    statusLayout->addStretch();

    m_countLabel = new QLabel("0 contracts");
    m_countLabel->setObjectName("countLabel");
    statusLayout->addWidget(m_countLabel);

    m_progressBar = new QProgressBar;
    m_progressBar->setObjectName("progressBar");
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    statusLayout->addWidget(m_progressBar);

    m_mainLayout->addWidget(m_statusWidget);
}

void ContractWidget::setupConnections()
{    // Toolbar actions
    connect(m_addButton, &QPushButton::clicked, this, &ContractWidget::onAddContractClicked);
    connect(m_editButton, &QPushButton::clicked, this, &ContractWidget::onEditContractClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &ContractWidget::onDeleteContractClicked);
    connect(m_duplicateButton, &QPushButton::clicked, this, &ContractWidget::onDuplicateContractClicked);
    connect(m_viewDetailsButton, &QPushButton::clicked, this, &ContractWidget::onViewContractDetailsClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &ContractWidget::onShowExportDialog);
    connect(m_statisticsButton, &QPushButton::clicked, this, &ContractWidget::showStatistics);
    connect(m_importButton, &QPushButton::clicked, this, &ContractWidget::onImportContractsClicked);
    connect(m_chatbotButton, &QPushButton::clicked, this, &ContractWidget::onShowChatbotClicked);
    connect(m_advancedSearchButton, &QPushButton::clicked, this, &ContractWidget::onShowAdvancedSearchClicked);
    connect(m_bulkOperationsButton, &QPushButton::clicked, this, &ContractWidget::onBulkOperationsClicked);
    connect(m_diagnosticsButton, &QPushButton::clicked, this, &ContractWidget::showDatabaseDiagnostics);

    // Search and filter
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ContractWidget::onSearchTextChanged);
    connect(m_statusFilterCombo, &QComboBox::currentTextChanged, this, &ContractWidget::onFilterChanged);
    connect(m_startDateFilter, &QDateEdit::dateChanged, this, &ContractWidget::onDateRangeChanged);
    connect(m_endDateFilter, &QDateEdit::dateChanged, this, &ContractWidget::onDateRangeChanged);
    connect(m_clearFiltersButton, &QPushButton::clicked, this, &ContractWidget::onClearFiltersClicked);

    // Table interactions
    connect(m_contractsTable, &QTableWidget::itemSelectionChanged, this, &ContractWidget::onContractSelectionChanged);
    connect(m_contractsTable, &QTableWidget::itemDoubleClicked, this, &ContractWidget::onContractDoubleClicked);
    connect(m_contractsTable, &QTableWidget::customContextMenuRequested, this, &ContractWidget::onTableContextMenu);
}

void ContractWidget::applyArchiFlowStyling()
{
    QString primaryColor = StyleManager::getPrimaryColor();   // #3D485A
    QString accentColor = StyleManager::getAccentColor();     // #E3C6B0
    QString successColor = StyleManager::getSuccessColor();   // #4CAF50
    QString warningColor = StyleManager::getWarningColor();   // #FFA726
    QString errorColor = StyleManager::getErrorColor();       // #FF6B6B

    setStyleSheet(QString(R"(
        ContractWidget {
            background-color: white;
        }
        
        QGroupBox#filterGroup {
            font-weight: bold;
            color: %1;
            border: 2px solid %2;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        
        QGroupBox#filterGroup::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            background-color: white;
        }
        
        QToolBar#contractToolbar {
            background: %2;
            border: none;
            padding: 8px;
            border-radius: 6px;
        }
        
        QToolBar#contractToolbar QPushButton {
            padding: 8px 16px;
            margin: 2px;
            border: 2px solid %1;
            border-radius: 4px;
            background-color: white;
            color: %1;
            font-weight: bold;
        }
        
        QToolBar#contractToolbar QPushButton:hover {
            background-color: %2;
            color: %1;
        }
        
        QToolBar#contractToolbar QPushButton#primaryButton {
            background-color: %1;
            color: %2;
            border-color: %1;
        }
        
        QToolBar#contractToolbar QPushButton#primaryButton:hover {
            background-color: %3;
            border-color: %3;
        }
          QTableWidget#contractsTable {
            gridline-color: rgba(227, 198, 176, 0.3);
            background-color: %1;
            alternate-background-color: rgba(61, 72, 90, 0.8);
            selection-background-color: %2;
            selection-color: %1;
            color: %2;
        }
        
        QTableWidget#contractsTable::item {
            padding: 8px;
            border-bottom: 1px solid rgba(227, 198, 176, 0.2);
            background-color: %1;
            color: %2;
        }
        
        QTableWidget#contractsTable::item:selected {
            background-color: %2;
            color: %1;
        }
        
        QHeaderView::section {
            background-color: %1;
            color: %2;
            padding: 8px;
            border: none;
            font-weight: bold;
        }
        
        QLineEdit, QComboBox, QDateEdit {
            padding: 6px;
            border: 2px solid %2;
            border-radius: 4px;
            background-color: white;
            color: %1;
        }
        
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border-color: %3;
        }
        
        QLabel#statusLabel {
            color: %1;
            font-weight: bold;
        }
        
        QLabel#countLabel {
            color: %2;
            background-color: %1;
            padding: 4px 8px;
            border-radius: 3px;
            font-weight: bold;
        }
        
        QProgressBar#progressBar {
            border: 2px solid %2;
            border-radius: 3px;
            background-color: white;
        }
        
        QProgressBar#progressBar::chunk {
            background-color: %3;
            border-radius: 1px;
        }
    )").arg(primaryColor, accentColor, successColor));

    // Apply elevation to toolbar
    StyleManager::addElevation(m_toolbar, 1);
}

void ContractWidget::setDatabaseManager(ContractDatabaseManager *dbManager)
{
    if (m_dbManager) {
        disconnect(m_dbManager, nullptr, this, nullptr);
    }

    m_dbManager = dbManager;

    if (m_dbManager) {
        connect(m_dbManager, &ContractDatabaseManager::contractAdded, this, &ContractWidget::onContractAdded);
        connect(m_dbManager, &ContractDatabaseManager::contractUpdated, this, &ContractWidget::onContractUpdated);
        connect(m_dbManager, &ContractDatabaseManager::contractDeleted, this, &ContractWidget::onContractDeleted);
        connect(m_dbManager, &ContractDatabaseManager::databaseError, this, &ContractWidget::onDatabaseError);
          // Automatically load contracts when database manager is set
        qDebug() << "Database manager set, loading contracts...";
        m_dbManager->printDatabaseInfo(); // Diagnostic info
        loadContracts();
    }
}

void ContractWidget::setChatbot(IContractChatbot *chatbot)
{
    m_chatbot = chatbot;
    if (m_chatbotButton) {
        m_chatbotButton->setEnabled(m_chatbot != nullptr);
    }
}

void ContractWidget::setImporter(IContractImporter *importer)
{
    m_importer = importer;
    if (m_importButton) {
        m_importButton->setEnabled(m_importer != nullptr);
    }
}

void ContractWidget::setExportManager(ContractExportManager *exportManager)
{
    if (m_exportManager && m_exportManager->parent() == this) {
        m_exportManager->deleteLater();
    }
    
    m_exportManager = exportManager;
    if (m_exportButton) {
        m_exportButton->setEnabled(m_exportManager != nullptr);
    }
}

void ContractWidget::refreshContracts()
{
    qDebug() << "Refreshing contracts...";
    loadContracts();
}

void ContractWidget::loadContracts()
{
    if (!m_dbManager) {
        qDebug() << "Database manager not set";
        showMessage("Database manager not set", true);
        return;
    }

    qDebug() << "Loading contracts from database...";
    m_isLoading = true;
    m_progressBar->setVisible(true);
    m_statusLabel->setText("Loading contracts...");
    
    // Clear existing contracts
    qDeleteAll(m_contracts);
    m_contracts.clear();
    
    // Load from database
    m_contracts = m_dbManager->getAllContracts();
    qDebug() << "Loaded" << m_contracts.size() << "contracts from database";
    
    // Apply current filters
    applyFilters();
    
    m_isLoading = false;
    m_progressBar->setVisible(false);
    updateStatusBar();
}

void ContractWidget::populateTable(const QList<Contract*> &contracts)
{
    m_contractsTable->setRowCount(contracts.size());
    
    for (int row = 0; row < contracts.size(); ++row) {
        Contract *contract = contracts[row];
        addContractToTable(contract);
    }
    
    updateStatusBar();
}

void ContractWidget::addContractToTable(Contract *contract)
{
    if (!contract) return;

    int row = m_contractsTable->rowCount();
    m_contractsTable->insertRow(row);

    // Store contract ID in the first column as user data
    QTableWidgetItem *clientItem = new QTableWidgetItem(contract->clientName());
    clientItem->setData(Qt::UserRole, contract->id());
    m_contractsTable->setItem(row, ClientNameColumn, clientItem);

    // Start Date
    QTableWidgetItem *startDateItem = new QTableWidgetItem(formatDate(contract->startDate()));
    startDateItem->setData(Qt::UserRole, contract->startDate());
    m_contractsTable->setItem(row, StartDateColumn, startDateItem);

    // End Date
    QTableWidgetItem *endDateItem = new QTableWidgetItem(formatDate(contract->endDate()));
    endDateItem->setData(Qt::UserRole, contract->endDate());
    m_contractsTable->setItem(row, EndDateColumn, endDateItem);

    // Value
    QTableWidgetItem *valueItem = new QTableWidgetItem(formatCurrency(contract->value()));
    valueItem->setData(Qt::UserRole, contract->value());
    valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_contractsTable->setItem(row, ValueColumn, valueItem);

    // Status with icon and color
    QString statusText = getStatusIcon(contract->status()) + " " + contract->statusDisplayText();
    QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
    statusItem->setData(Qt::UserRole, contract->status());
    statusItem->setForeground(getStatusColor(contract->status()));
    m_contractsTable->setItem(row, StatusColumn, statusItem);

    // Payment Terms
    QTableWidgetItem *paymentItem = new QTableWidgetItem(QString("%1 days").arg(contract->paymentTerms()));
    paymentItem->setData(Qt::UserRole, contract->paymentTerms());
    m_contractsTable->setItem(row, PaymentTermsColumn, paymentItem);

    // Description
    QString description = contract->description();
    if (description.length() > 50) {
        description = description.left(47) + "...";
    }
    QTableWidgetItem *descItem = new QTableWidgetItem(description);
    descItem->setToolTip(contract->description());
    m_contractsTable->setItem(row, DescriptionColumn, descItem);
}

void ContractWidget::onAddContractClicked()
{
    if (!m_dbManager) {
        QMessageBox::warning(this, "Database Error", "Database manager is not available. Please restart the application.");
        return;
    }
    
    ContractDialog dialog(ContractDialog::AddMode, this);
    dialog.setDatabaseManager(m_dbManager);
    
    // Connect the dialog signal to refresh contracts
    connect(&dialog, &ContractDialog::contractSaved, this, [this](const QString &contractId) {
        qDebug() << "Contract saved signal received, refreshing contracts...";
        refreshContracts();
        emit contractAdded(contractId);
        showMessage("Contract added and display refreshed", false);
    });
    
    int result = dialog.exec();
    qDebug() << "Dialog exec result:" << result;
}

void ContractWidget::onEditContractClicked()
{
    Contract *contract = getSelectedContract();
    if (!contract) {
        showMessage("Please select a contract to edit", true);
        return;
    }    ContractDialog dialog(ContractDialog::EditMode, this);
    dialog.setContract(contract);
    dialog.setDatabaseManager(m_dbManager);
    
    // Connect the dialog signal to refresh contracts
    connect(&dialog, &ContractDialog::contractSaved, this, [this](const QString &contractId) {
        refreshContracts();
        emit contractUpdated(contractId);
    });
    
    dialog.exec();
}

void ContractWidget::onDeleteContractClicked()
{
    Contract *contract = getSelectedContract();
    if (!contract) {
        showMessage("Please select a contract to delete", true);
        return;
    }    int result = QMessageBox::question(this, "Delete Contract",
                                     QString("Are you sure you want to delete the contract for '%1'?\n\nThis action cannot be undone.")
                                     .arg(contract->clientName()),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);

    if (result == QMessageBox::Yes) {
        QString contractId = contract->id();
        if (m_dbManager && m_dbManager->deleteContract(contractId)) {
            showMessage("Contract deleted successfully");
            emit contractDeleted(contractId);
        } else {
            showMessage("Failed to delete contract: " + m_dbManager->getLastError(), true);
        }
    }
}

void ContractWidget::onDuplicateContractClicked()
{
    Contract *originalContract = getSelectedContract();
    if (!originalContract) {
        showMessage("Please select a contract to duplicate", true);
        return;
    }

    ContractDialog dialog(ContractDialog::AddMode, this);
    dialog.setDatabaseManager(m_dbManager);
    
    // Create a copy of the contract
    Contract *newContract = new Contract(&dialog);
    newContract->setClientName(originalContract->clientName() + " (Copy)");
    newContract->setStartDate(QDate::currentDate());
    newContract->setEndDate(QDate::currentDate().addYears(1));
    newContract->setValue(originalContract->value());
    newContract->setStatus("Draft");
    newContract->setDescription(originalContract->description());
    newContract->setPaymentTerms(originalContract->paymentTerms());
    newContract->setHasNonCompeteClause(originalContract->hasNonCompeteClause());
      dialog.setContract(newContract);
    
    // Connect the dialog signal to refresh contracts
    connect(&dialog, &ContractDialog::contractSaved, this, [this](const QString &contractId) {
        refreshContracts();
        emit contractAdded(contractId);
    });
      dialog.exec();
}

void ContractWidget::onViewContractDetailsClicked()
{
    Contract *contract = getSelectedContract();
    if (!contract) {
        showMessage("Please select a contract to view", true);
        return;
    }

    // For now, just open the edit dialog in read-only mode
    onEditContractClicked();
}

void ContractWidget::onSearchTextChanged()
{
    m_searchTimer->stop();
    m_searchTimer->start();
}

void ContractWidget::onFilterChanged()
{
    applyFilters();
}

void ContractWidget::onDateRangeChanged()
{
    applyFilters();
}

void ContractWidget::onClearFiltersClicked()
{
    m_searchEdit->clear();
    m_statusFilterCombo->setCurrentIndex(0);
    m_startDateFilter->setDate(QDate::currentDate().addYears(-1));
    m_endDateFilter->setDate(QDate::currentDate().addYears(1));
    applyFilters();
}

void ContractWidget::onContractSelectionChanged()
{
    updateActionStates();
    
    Contract *contract = getSelectedContract();
    if (contract) {
        emit contractSelected(contract->id());
    }
}

void ContractWidget::onContractDoubleClicked()
{
    onEditContractClicked();
}

void ContractWidget::onTableContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    
    Contract *contract = getSelectedContract();
    if (contract) {
        contextMenu.addAction("Edit", this, &ContractWidget::onEditContractClicked);
        contextMenu.addAction("Duplicate", this, &ContractWidget::onDuplicateContractClicked);
        contextMenu.addSeparator();
        contextMenu.addAction("Delete", this, &ContractWidget::onDeleteContractClicked);
    } else {
        contextMenu.addAction("Add New Contract", this, &ContractWidget::onAddContractClicked);
    }
    
    contextMenu.exec(m_contractsTable->mapToGlobal(pos));
}

void ContractWidget::onContractAdded(const QString &contractId)
{
    Q_UNUSED(contractId)
    refreshContracts();
}

void ContractWidget::onContractUpdated(const QString &contractId)
{
    Q_UNUSED(contractId)
    refreshContracts();
}

void ContractWidget::onContractDeleted(const QString &contractId)
{
    Q_UNUSED(contractId)
    refreshContracts();
}

void ContractWidget::onDatabaseError(const QString &error)
{
    showMessage("Database error: " + error, true);
}

void ContractWidget::updateStatusBar()
{
    int totalContracts = m_contracts.size();
    int filteredContracts = m_filteredContracts.size();
    
    if (totalContracts == filteredContracts) {
        m_countLabel->setText(QString("%1 contracts").arg(totalContracts));
        m_statusLabel->setText("Ready");
    } else {
        m_countLabel->setText(QString("%1 of %2 contracts").arg(filteredContracts).arg(totalContracts));
        m_statusLabel->setText("Filtered");
    }
}

void ContractWidget::updateActionStates()
{
    Contract *contract = getSelectedContract();
    bool hasSelection = (contract != nullptr);
    
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_duplicateButton->setEnabled(hasSelection);
    m_viewDetailsButton->setEnabled(hasSelection);
}

Contract* ContractWidget::getSelectedContract() const
{
    int currentRow = m_contractsTable->currentRow();
    if (currentRow < 0 || currentRow >= m_filteredContracts.size()) {
        return nullptr;
    }
    
    return m_filteredContracts[currentRow];
}

void ContractWidget::applyFilters()
{
    if (m_isLoading) return;
    
    m_filteredContracts.clear();
    
    QString searchText = m_searchEdit->text().trimmed().toLower();
    QString statusFilter = m_statusFilterCombo->currentData().toString();
    QDate startDate = m_startDateFilter->date();
    QDate endDate = m_endDateFilter->date();
    
    for (Contract *contract : m_contracts) {
        bool matches = true;
        
        // Text search
        if (!searchText.isEmpty()) {
            QString searchableText = QString("%1 %2 %3")
                .arg(contract->clientName())
                .arg(contract->status())
                .arg(contract->description()).toLower();
            
            if (!searchableText.contains(searchText)) {
                matches = false;
            }
        }
        
        // Status filter
        if (!statusFilter.isEmpty() && contract->status() != statusFilter) {
            matches = false;
        }
        
        // Date range filter
        if (contract->startDate() < startDate || contract->endDate() > endDate) {
            matches = false;
        }
        
        if (matches) {
            m_filteredContracts.append(contract);
        }
    }
    
    populateTable(m_filteredContracts);
}

QString ContractWidget::formatCurrency(double value) const
{
    return QLocale().toCurrencyString(value);
}

QString ContractWidget::formatDate(const QDate &date) const
{
    return date.toString("dd/MM/yyyy");
}

QString ContractWidget::getStatusIcon(const QString &status) const
{
    if (status == "Active") return "✅";
    if (status == "Completed") return "✓";
    if (status == "Expired") return "⚠️";
    if (status == "Cancelled") return "❌";
    return "📄"; // Draft
}

QColor ContractWidget::getStatusColor(const QString &status) const
{
    if (status == "Active") return QColor(StyleManager::getSuccessColor());
    if (status == "Completed") return QColor("#1976D2"); // Blue
    if (status == "Expired") return QColor(StyleManager::getErrorColor());
    if (status == "Cancelled") return QColor("#757575"); // Gray
    return QColor(StyleManager::getWarningColor()); // Draft - Orange
}

void ContractWidget::showMessage(const QString &message, bool isError)
{
    m_statusLabel->setText(message);
    
    if (isError) {
        m_statusLabel->setStyleSheet("color: " + StyleManager::getErrorColor() + ";");
    } else {
        m_statusLabel->setStyleSheet("color: " + StyleManager::getSuccessColor() + ";");
    }
    
    // Reset status after 3 seconds
    QTimer::singleShot(3000, this, [this] {
        m_statusLabel->setText("Ready");
        m_statusLabel->setStyleSheet("");
    });
}

void ContractWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_contracts.isEmpty() && m_dbManager) {
        loadContracts();
    }
}

void ContractWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // Adjust column widths based on widget size
    if (m_contractsTable) {
        int totalWidth = m_contractsTable->width() - 50; // Account for scrollbar
        m_contractsTable->setColumnWidth(ClientNameColumn, totalWidth * 0.20);
        m_contractsTable->setColumnWidth(StartDateColumn, totalWidth * 0.12);
        m_contractsTable->setColumnWidth(EndDateColumn, totalWidth * 0.12);
        m_contractsTable->setColumnWidth(ValueColumn, totalWidth * 0.15);
        m_contractsTable->setColumnWidth(StatusColumn, totalWidth * 0.12);
        m_contractsTable->setColumnWidth(PaymentTermsColumn, totalWidth * 0.12);
        // Description column will stretch to fill remaining space
    }
}

// IContractWidget interface implementations
void ContractWidget::setContractService(IContractService *service)
{
    // Store the service interface
    m_contractService = service;
    
    // If service is a ContractDatabaseManager, also set it as dbManager for legacy compatibility
    if (auto dbManager = dynamic_cast<ContractDatabaseManager*>(service)) {
        setDatabaseManager(dbManager);
    }
}

void ContractWidget::clearSelection()
{
    m_contractsTable->clearSelection();
    updateActionStates();
}

void ContractWidget::addContract()
{
    onAddContractClicked();
}

void ContractWidget::editContract(const QString &contractId)
{
    // Find and select the contract first
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == contractId) {
            m_contractsTable->selectRow(row);
            break;
        }
    }
    onEditContractClicked();
}

void ContractWidget::deleteContract(const QString &contractId)
{
    // Find and select the contract first
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == contractId) {
            m_contractsTable->selectRow(row);
            break;
        }
    }
    onDeleteContractClicked();
}

void ContractWidget::duplicateContract(const QString &contractId)
{
    // Find and select the contract first
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == contractId) {
            m_contractsTable->selectRow(row);
            break;
        }
    }
    onDuplicateContractClicked();
}

void ContractWidget::setSearchFilter(const QString &filter)
{
    m_searchEdit->setText(filter);
}

void ContractWidget::setStatusFilter(const QString &status)
{    int index = m_statusFilterCombo->findText(status);
    if (index >= 0) {
        m_statusFilterCombo->setCurrentIndex(index);
    }
}

void ContractWidget::setDateRangeFilter(const QDate &startDate, const QDate &endDate)
{    m_startDateFilter->setDate(startDate);
    m_endDateFilter->setDate(endDate);
}

void ContractWidget::clearFilters()
{
    onClearFiltersClicked();
}

QList<QString> ContractWidget::getSelectedContractIds()
{
    QList<QString> contractIds;
    QList<QTableWidgetItem*> selectedItems = m_contractsTable->selectedItems();
    QSet<int> selectedRows;
    
    // Get unique rows
    for (QTableWidgetItem *item : selectedItems) {
        selectedRows.insert(item->row());
    }
    
    // Get contract IDs from selected rows
    for (int row : selectedRows) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0);
        if (item) {
            contractIds.append(item->data(Qt::UserRole).toString());
        }
    }
    
    return contractIds;
}

Contract* ContractWidget::getCurrentContract()
{
    int currentRow = m_contractsTable->currentRow();
    if (currentRow >= 0) {
        QTableWidgetItem *item = m_contractsTable->item(currentRow, 0);
        if (item) {
            QString contractId = item->data(Qt::UserRole).toString();
            for (Contract *contract : m_contracts) {
                if (contract && contract->id() == contractId) {
                    return contract;
                }
            }
        }
    }
    return nullptr;
}

void ContractWidget::selectContract(const QString &contractId)
{    // Find and select the contract in the table
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0); // Assuming ID is in first column
        if (item && item->text() == contractId) {
            m_contractsTable->selectRow(row);
            emit contractSelected(contractId);
            break;
        }
    }
}

void ContractWidget::exportSelectedContracts()
{
    // TODO: Implement export functionality for selected contracts
    QMessageBox::information(this, tr("Export"), 
                           tr("Export selected contracts functionality will be implemented soon."));
}

void ContractWidget::exportAllContracts()
{
    // TODO: Implement export functionality for all contracts
    QMessageBox::information(this, tr("Export"), 
                           tr("Export all contracts functionality will be implemented soon."));
}

void ContractWidget::setViewMode(int mode)
{
    // TODO: Implement different view modes (List, Grid, Cards)
    Q_UNUSED(mode)
    // For now, we only support table view
}

void ContractWidget::setSortOrder(int column, bool ascending)
{
    m_contractsTable->sortItems(column, ascending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

void ContractWidget::setColumnsVisible(const QStringList &columns)
{
    // Hide all columns first
    for (int i = 0; i < m_contractsTable->columnCount(); ++i) {
        m_contractsTable->setColumnHidden(i, true);
    }
    
    // Show specified columns
    for (const QString &columnName : columns) {
        // Map column names to indices (this is a simplified implementation)
        if (columnName == "Title") m_contractsTable->setColumnHidden(0, false);
        else if (columnName == "Client") m_contractsTable->setColumnHidden(1, false);
        else if (columnName == "Value") m_contractsTable->setColumnHidden(2, false);
        else if (columnName == "Start Date") m_contractsTable->setColumnHidden(3, false);
        else if (columnName == "End Date") m_contractsTable->setColumnHidden(4, false);
        else if (columnName == "Status") m_contractsTable->setColumnHidden(5, false);
    }
}

void ContractWidget::showStatistics()
{
    // TODO: Implement statistics view
    QMessageBox::information(this, tr("Statistics"), 
                           tr("Contract statistics view will be implemented soon."));
}

void ContractWidget::showContractDetails(const QString &contractId)
{
    // Find and select the contract first
    selectContract(contractId);
    onViewContractDetailsClicked();
}

// Export functionality implementation

void ContractWidget::onExportClicked()
{
    onShowExportDialog();
}

void ContractWidget::onShowExportDialog()
{
    QMenu exportMenu;
    exportMenu.setTitle("Export Options");
    
    QAction *csvAction = exportMenu.addAction(QIcon(":/icons/csv.png"), "Export to CSV");
    QAction *pdfAction = exportMenu.addAction(QIcon(":/icons/pdf.png"), "Export to PDF");
    QAction *excelAction = exportMenu.addAction(QIcon(":/icons/excel.png"), "Export to Excel");
    QAction *jsonAction = exportMenu.addAction(QIcon(":/icons/json.png"), "Export to JSON");
    
    exportMenu.addSeparator();
    QAction *statisticsAction = exportMenu.addAction(QIcon(":/icons/chart.png"), "Export Statistics Only");
    
    connect(csvAction, &QAction::triggered, this, &ContractWidget::onExportToCSV);
    connect(pdfAction, &QAction::triggered, this, &ContractWidget::onExportToPDF);
    connect(excelAction, &QAction::triggered, this, &ContractWidget::onExportToExcel);
    connect(jsonAction, &QAction::triggered, this, &ContractWidget::onExportToJSON);
    connect(statisticsAction, &QAction::triggered, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, 
            "Export Statistics", 
            QString("contract_statistics_%1.csv").arg(QDate::currentDate().toString("yyyy-MM-dd")),
            "CSV Files (*.csv)");
        if (!fileName.isEmpty() && m_exportManager) {
            m_exportManager->exportStatisticsOnly(fileName, ContractExportManager::CSV);
        }
    });
    
    // Show menu at button position
    QPoint globalPos = m_exportButton->mapToGlobal(m_exportButton->rect().bottomLeft());
    exportMenu.exec(globalPos);
}

void ContractWidget::onExportToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Export Contracts to CSV", 
        QString("contracts_%1.csv").arg(QDate::currentDate().toString("yyyy-MM-dd")),
        "CSV Files (*.csv)");
    
    if (fileName.isEmpty() || !m_exportManager) {
        return;
    }
    
    // Set up export manager
    m_exportManager->setContracts(m_contracts);
    m_exportManager->setFilteredContracts(m_filteredContracts);
    
    // Get selected contracts
    QList<Contract*> selectedContracts;
    QList<QTableWidgetItem*> selectedItems = m_contractsTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        QSet<int> selectedRows;
        for (QTableWidgetItem *item : selectedItems) {
            selectedRows.insert(item->row());
        }
        for (int row : selectedRows) {
            QTableWidgetItem *clientItem = m_contractsTable->item(row, ClientNameColumn);
            if (clientItem) {
                QString clientName = clientItem->text();
                for (Contract *contract : m_contracts) {
                    if (contract->clientName() == clientName) {
                        selectedContracts.append(contract);
                        break;
                    }
                }
            }
        }
    }    m_exportManager->setSelectedContracts(selectedContracts);
    
    // Determine export scope
    ContractExportManager::ExportScope scope = ContractExportManager::AllContracts;
    if (!selectedContracts.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Export Scope");        msgBox.setText(QString("Export %1 selected contracts or all %2 contracts?")
            .arg(selectedContracts.count())
            .arg(m_filteredContracts.isEmpty() ? m_contracts.count() : m_filteredContracts.count()));        QPushButton* selectedBtn = msgBox.addButton("Selected", QMessageBox::AcceptRole);
        QPushButton* allBtn = msgBox.addButton("All", QMessageBox::RejectRole);
        Q_UNUSED(selectedBtn)
        Q_UNUSED(allBtn)
        QPushButton* cancelBtn = msgBox.addButton("Cancel", QMessageBox::DestructiveRole);
        msgBox.setDefaultButton(cancelBtn);
        
        int result = msgBox.exec();
        
        if (result == 2) return; // Cancel
        scope = (result == 0) ? ContractExportManager::SelectedContracts 
                              : (m_filteredContracts.isEmpty() ? ContractExportManager::AllContracts 
                                                               : ContractExportManager::FilteredContracts);
    } else if (!m_filteredContracts.isEmpty()) {
        scope = ContractExportManager::FilteredContracts;
    }
    
    // Perform export
    bool success = m_exportManager->exportContracts(fileName, ContractExportManager::CSV, scope);
    
    if (success) {
        showMessage(QString("Successfully exported %1 contracts to %2")
                   .arg(m_exportManager->getLastExportCount())
                   .arg(QFileInfo(fileName).fileName()));
    } else {
        showMessage("Export failed: " + m_exportManager->getLastError(), true);
    }
}

void ContractWidget::onExportToPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Export Contracts to PDF", 
        QString("contracts_%1.pdf").arg(QDate::currentDate().toString("yyyy-MM-dd")),
        "PDF Files (*.pdf)");
    
    if (fileName.isEmpty() || !m_exportManager) {
        return;
    }
    
    // Configure export manager for PDF
    m_exportManager->setContracts(m_contracts);
    m_exportManager->setFilteredContracts(m_filteredContracts);
    m_exportManager->setExportMetadata("Contract Report", "ArchiFlow Application", "Contract Management Export");
      // Get selected contracts
    QList<Contract*> selectedContracts;
    QList<QTableWidgetItem*> selectedItems = m_contractsTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        QSet<int> selectedRows;
        for (QTableWidgetItem *item : selectedItems) {
            selectedRows.insert(item->row());
        }
        for (int row : selectedRows) {
            QTableWidgetItem *clientItem = m_contractsTable->item(row, ClientNameColumn);
            if (clientItem) {
                QString clientName = clientItem->text();
                for (Contract *contract : m_contracts) {
                    if (contract->clientName() == clientName) {
                        selectedContracts.append(contract);
                        break;
                    }
                }
            }
        }
    }
    m_exportManager->setSelectedContracts(selectedContracts);
      // Determine export scope
    ContractExportManager::ExportScope scope = ContractExportManager::AllContracts;
    if (!selectedContracts.isEmpty()) {
        QMessageBox msgBox2(this);
        msgBox2.setWindowTitle("Export Scope");
        msgBox2.setText(QString("Export %1 selected contracts or all %2 contracts?")
            .arg(selectedContracts.count())
            .arg(m_filteredContracts.isEmpty() ? m_contracts.count() : m_filteredContracts.count()));        QPushButton* selectedBtn2 = msgBox2.addButton("Selected", QMessageBox::AcceptRole);
        QPushButton* allBtn2 = msgBox2.addButton("All", QMessageBox::RejectRole);
        Q_UNUSED(selectedBtn2)
        Q_UNUSED(allBtn2)
        QPushButton* cancelBtn2 = msgBox2.addButton("Cancel", QMessageBox::DestructiveRole);
        msgBox2.setDefaultButton(cancelBtn2);
        
        int result = msgBox2.exec();
        
        if (result == 2) return; // Cancel
        scope = (result == 0) ? ContractExportManager::SelectedContracts 
                              : (m_filteredContracts.isEmpty() ? ContractExportManager::AllContracts 
                                                               : ContractExportManager::FilteredContracts);
    } else if (!m_filteredContracts.isEmpty()) {
        scope = ContractExportManager::FilteredContracts;
    }
    
    // Perform export
    bool success = m_exportManager->exportContracts(fileName, ContractExportManager::PDF, scope);
    
    if (success) {
        showMessage(QString("Successfully exported %1 contracts to %2")
                   .arg(m_exportManager->getLastExportCount())
                   .arg(QFileInfo(fileName).fileName()));
    } else {
        showMessage("Export failed: " + m_exportManager->getLastError(), true);
    }
}

void ContractWidget::onExportToExcel()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Export Contracts to Excel", 
        QString("contracts_%1.xlsx").arg(QDate::currentDate().toString("yyyy-MM-dd")),
        "Excel Files (*.xlsx)");
    
    if (fileName.isEmpty() || !m_exportManager) {
        return;
    }
    
    // Note: This will export as CSV for now since we don't have Excel library
    showMessage("Excel export will save as CSV format (Excel compatible)", false);
    
    m_exportManager->setContracts(m_contracts);
    m_exportManager->setFilteredContracts(m_filteredContracts);
    
    ContractExportManager::ExportScope scope = m_filteredContracts.isEmpty() ? 
                                              ContractExportManager::AllContracts : 
                                              ContractExportManager::FilteredContracts;
    
    bool success = m_exportManager->exportContracts(fileName, ContractExportManager::Excel, scope);
    
    if (success) {
        showMessage(QString("Successfully exported %1 contracts to %2")
                   .arg(m_exportManager->getLastExportCount())
                   .arg(QFileInfo(fileName).fileName()));
    } else {
        showMessage("Export failed: " + m_exportManager->getLastError(), true);
    }
}

void ContractWidget::onExportToJSON()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Export Contracts to JSON", 
        QString("contracts_%1.json").arg(QDate::currentDate().toString("yyyy-MM-dd")),
        "JSON Files (*.json)");
    
    if (fileName.isEmpty() || !m_exportManager) {
        return;
    }
    
    m_exportManager->setContracts(m_contracts);
    m_exportManager->setFilteredContracts(m_filteredContracts);
    
    ContractExportManager::ExportScope scope = m_filteredContracts.isEmpty() ? 
                                              ContractExportManager::AllContracts : 
                                              ContractExportManager::FilteredContracts;
    
    bool success = m_exportManager->exportContracts(fileName, ContractExportManager::JSON, scope);
    
    if (success) {
        showMessage(QString("Successfully exported %1 contracts to %2")
                   .arg(m_exportManager->getLastExportCount())
                   .arg(QFileInfo(fileName).fileName()));
    } else {
        showMessage("Export failed: " + m_exportManager->getLastError(), true);
    }
}

void ContractWidget::onShowChatbotClicked()
{
    if (!m_chatbot) {
        showMessage("AI Chatbot is not available", true);
        return;
    }
    
    // Create and show chatbot dialog
    ContractChatbotDialog dialog(this);
    dialog.setChatbot(m_chatbot);
    dialog.setContracts(m_contracts);
    dialog.setDatabaseManager(m_dbManager);
    
    // Connect chatbot signals
    connect(&dialog, &ContractChatbotDialog::contractAnalysisRequested,
            this, &ContractWidget::contractSelected);
    connect(&dialog, &ContractChatbotDialog::contractCreationRequested,
            this, &ContractWidget::onAddContractClicked);
    connect(&dialog, &ContractChatbotDialog::errorOccurred,
            this, [this](const QString &error) { 
                showMessage("Chatbot Error: " + error, true); 
            });
    
    dialog.exec();
    
    emit chatbotRequested();
}

void ContractWidget::onImportContractsClicked()
{
    if (!m_importer) {
        showMessage("Contract importer is not available", true);
        return;
    }
    
    // Create and show import dialog
    ContractImportDialog dialog(this);
    dialog.setImporter(m_importer);
    dialog.setDatabaseManager(m_dbManager);
      // Connect import signals
    connect(&dialog, &ContractImportDialog::contractsImported,
            this, [this](int count, int total, const QString &summary) {
                showMessage(QString("Successfully imported %1 of %2 contracts. %3").arg(count).arg(total).arg(summary));
                refreshContracts();
            });
    connect(&dialog, &ContractImportDialog::importProgress,
            this, [this](int current, int total) {
                m_statusLabel->setText(QString("Importing: %1 of %2")
                                      .arg(current).arg(total));
                m_progressBar->setVisible(true);
                m_progressBar->setRange(0, total);
                m_progressBar->setValue(current);
            });
    connect(&dialog, &ContractImportDialog::errorOccurred,
            this, [this](const QString &error) { 
                showMessage("Import Error: " + error, true); 
            });
    
    if (dialog.exec() == QDialog::Accepted) {
        // Hide progress bar after import
        m_progressBar->setVisible(false);
        updateStatusBar();
    }
    
    emit importRequested();
}

void ContractWidget::onShowAdvancedSearchClicked()
{
    // For now, show a message that this feature is coming soon
    QMessageBox::information(this, "Advanced Search", 
                           "Advanced search functionality is coming soon!\n\n"
                           "This will include:\n"
                           "• Multi-criteria search\n"
                           "• Custom date ranges\n"
                           "• Financial filters\n"
                           "• Saved search queries");
}

void ContractWidget::onBulkOperationsClicked()
{
    QList<Contract*> selectedContracts;
    QList<QTableWidgetItem*> selectedItems = m_contractsTable->selectedItems();
    
    if (selectedItems.isEmpty()) {
        showMessage("Please select contracts for bulk operations", true);
        return;
    }
    
    // Get selected contracts
    QSet<int> selectedRows;
    for (QTableWidgetItem *item : selectedItems) {
        selectedRows.insert(item->row());
    }
    
    for (int row : selectedRows) {
        QTableWidgetItem *clientItem = m_contractsTable->item(row, ClientNameColumn);
        if (clientItem) {
            QString clientName = clientItem->text();
            for (Contract *contract : m_contracts) {
                if (contract->clientName() == clientName) {
                    selectedContracts.append(contract);
                    break;
                }
            }
        }
    }
    
    if (selectedContracts.isEmpty()) {
        showMessage("No valid contracts selected", true);
        return;
    }
    
    // Show bulk operations menu
    QMenu bulkMenu(this);
    bulkMenu.addAction("Update Status", [this, selectedContracts]() {
        bool ok;
        QStringList statusOptions = {"Active", "Completed", "Expired", "Draft", "Pending"};
        QString newStatus = QInputDialog::getItem(this, "Bulk Update Status",
                                                "Select new status:", statusOptions, 0, false, &ok);
        if (ok && !newStatus.isEmpty()) {
            int updated = 0;
            for (Contract *contract : selectedContracts) {
                contract->setStatus(newStatus);
                if (m_dbManager && m_dbManager->updateContract(contract)) {
                    updated++;
                }
            }
            showMessage(QString("Updated status for %1 contracts").arg(updated));
            refreshContracts();
        }
    });
    
    bulkMenu.addAction("Export Selected", [this]() {
        onExportToCSV();
    });
      bulkMenu.addAction("Delete Selected", [this, selectedContracts]() {
        int result = QMessageBox::question(this, "Bulk Delete",
                                         QString("Are you sure you want to delete %1 selected contracts?\n\n"
                                                "This action cannot be undone.")
                                         .arg(selectedContracts.count()),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
        
        if (result == QMessageBox::Yes) {
            int deleted = 0;
            for (Contract *contract : selectedContracts) {
                if (m_dbManager && m_dbManager->deleteContract(contract->id())) {
                    deleted++;
                }
            }
            showMessage(QString("Deleted %1 contracts").arg(deleted));
            refreshContracts();
        }
    });
    
    bulkMenu.exec(QCursor::pos());
}

void ContractWidget::showDatabaseDiagnostics()
{
    QString info = "=== DATABASE DIAGNOSTICS ===\n\n";
    
    if (!m_dbManager) {
        info += "❌ Database Manager: NOT SET\n";
        QMessageBox::warning(this, "Database Diagnostics", info);
        return;
    }
    
    info += "✅ Database Manager: SET\n";
    
    // Try to get all contracts
    QList<Contract*> contracts = m_dbManager->getAllContracts();
    info += QString("📊 Contracts found: %1\n\n").arg(contracts.size());
    
    if (contracts.isEmpty()) {
        info += "⚠️ No contracts found in database\n";
        info += "This could mean:\n";
        info += "- Database is empty\n";
        info += "- Database connection failed\n";
        info += "- Table doesn't exist\n";
    } else {
        info += "📋 Contracts in database:\n";
        for (int i = 0; i < contracts.size(); ++i) {
            Contract* contract = contracts.at(i);
            info += QString("%1. %2 (%3)\n")
                    .arg(i + 1)
                    .arg(contract->clientName())
                    .arg(contract->status());
        }
    }
    
    // Check current UI state
    info += QString("\n🖥️ UI Table rows: %1\n").arg(m_contractsTable->rowCount());
    info += QString("📝 Loaded contracts in memory: %1\n").arg(m_contracts.size());
    
    // Clean up
    qDeleteAll(contracts);
    
    QMessageBox::information(this, "Database Diagnostics", info);
}

void ContractWidget::populateTable(const QList<Contract*> &contracts)
{
    m_contractsTable->setRowCount(contracts.size());
    
    for (int row = 0; row < contracts.size(); ++row) {
        Contract *contract = contracts[row];
        addContractToTable(contract);
    }
    
    updateStatusBar();
}

void ContractWidget::addContractToTable(Contract *contract)
{
    if (!contract) return;

    int row = m_contractsTable->rowCount();
    m_contractsTable->insertRow(row);

    // Store contract ID in the first column as user data
    QTableWidgetItem *clientItem = new QTableWidgetItem(contract->clientName());
    clientItem->setData(Qt::UserRole, contract->id());
    m_contractsTable->setItem(row, ClientNameColumn, clientItem);

    // Start Date
    QTableWidgetItem *startDateItem = new QTableWidgetItem(formatDate(contract->startDate()));
    startDateItem->setData(Qt::UserRole, contract->startDate());
    m_contractsTable->setItem(row, StartDateColumn, startDateItem);

    // End Date
    QTableWidgetItem *endDateItem = new QTableWidgetItem(formatDate(contract->endDate()));
    endDateItem->setData(Qt::UserRole, contract->endDate());
    m_contractsTable->setItem(row, EndDateColumn, endDateItem);

    // Value
    QTableWidgetItem *valueItem = new QTableWidgetItem(formatCurrency(contract->value()));
    valueItem->setData(Qt::UserRole, contract->value());
    valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_contractsTable->setItem(row, ValueColumn, valueItem);

    // Status with icon and color
    QString statusText = getStatusIcon(contract->status()) + " " + contract->statusDisplayText();
    QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
    statusItem->setData(Qt::UserRole, contract->status());
    statusItem->setForeground(getStatusColor(contract->status()));
    m_contractsTable->setItem(row, StatusColumn, statusItem);

    // Payment Terms
    QTableWidgetItem *paymentItem = new QTableWidgetItem(QString("%1 days").arg(contract->paymentTerms()));
    paymentItem->setData(Qt::UserRole, contract->paymentTerms());
    m_contractsTable->setItem(row, PaymentTermsColumn, paymentItem);

    // Description
    QString description = contract->description();
    if (description.length() > 50) {
        description = description.left(47) + "...";
    }
    QTableWidgetItem *descItem = new QTableWidgetItem(description);
    descItem->setToolTip(contract->description());
    m_contractsTable->setItem(row, DescriptionColumn, descItem);
}

void ContractWidget::onAddContractClicked()
{
    if (!m_dbManager) {
        QMessageBox::warning(this, "Database Error", "Database manager is not available. Please restart the application.");
        return;
    }
    
    ContractDialog dialog(ContractDialog::AddMode, this);
    dialog.setDatabaseManager(m_dbManager);
    
    // Connect the dialog signal to refresh contracts
    connect(&dialog, &ContractDialog::contractSaved, this, [this](const QString &contractId) {
        qDebug() << "Contract saved signal received, refreshing contracts...";
        refreshContracts();
        emit contractAdded(contractId);
        showMessage("Contract added and display refreshed", false);
    });
    
    int result = dialog.exec();
    qDebug() << "Dialog exec result:" << result;
}

void ContractWidget::onEditContractClicked()
{
    Contract *contract = getSelectedContract();
    if (!contract) {
        showMessage("Please select a contract to edit", true);
        return;
    }    ContractDialog dialog(ContractDialog::EditMode, this);
    dialog.setContract(contract);
    dialog.setDatabaseManager(m_dbManager);
    
    // Connect the dialog signal to refresh contracts
    connect(&dialog, &ContractDialog::contractSaved, this, [this](const QString &contractId) {
        refreshContracts();
        emit contractUpdated(contractId);
    });
    
    dialog.exec();
}

void ContractWidget::onDeleteContractClicked()
{
    Contract *contract = getSelectedContract();
    if (!contract) {
        showMessage("Please select a contract to delete", true);
        return;
    }    int result = QMessageBox::question(this, "Delete Contract",
                                     QString("Are you sure you want to delete the contract for '%1'?\n\nThis action cannot be undone.")
                                     .arg(contract->clientName()),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);

    if (result == QMessageBox::Yes) {
        QString contractId = contract->id();
        if (m_dbManager && m_dbManager->deleteContract(contractId)) {
            showMessage("Contract deleted successfully");
            emit contractDeleted(contractId);
        } else {
            showMessage("Failed to delete contract: " + m_dbManager->getLastError(), true);
        }
    }
}

void ContractWidget::onDuplicateContractClicked()
{
    Contract *originalContract = getSelectedContract();
    if (!originalContract) {
        showMessage("Please select a contract to duplicate", true);
        return;
    }

    ContractDialog dialog(ContractDialog::AddMode, this);
    dialog.setDatabaseManager(m_dbManager);
    
    // Create a copy of the contract
    Contract *newContract = new Contract(&dialog);
    newContract->setClientName(originalContract->clientName() + " (Copy)");
    newContract->setStartDate(QDate::currentDate());
    newContract->setEndDate(QDate::currentDate().addYears(1));
    newContract->setValue(originalContract->value());
    newContract->setStatus("Draft");
    newContract->setDescription(originalContract->description());
    newContract->setPaymentTerms(originalContract->paymentTerms());
    newContract->setHasNonCompeteClause(originalContract->hasNonCompeteClause());
      dialog.setContract(newContract);
    
    // Connect the dialog signal to refresh contracts
    connect(&dialog, &ContractDialog::contractSaved, this, [this](const QString &contractId) {
        refreshContracts();
        emit contractAdded(contractId);
    });
      dialog.exec();
}

void ContractWidget::onViewContractDetailsClicked()
{
    Contract *contract = getSelectedContract();
    if (!contract) {
        showMessage("Please select a contract to view", true);
        return;
    }

    // For now, just open the edit dialog in read-only mode
    onEditContractClicked();
}

void ContractWidget::onSearchTextChanged()
{
    m_searchTimer->stop();
    m_searchTimer->start();
}

void ContractWidget::onFilterChanged()
{
    applyFilters();
}

void ContractWidget::onDateRangeChanged()
{
    applyFilters();
}

void ContractWidget::onClearFiltersClicked()
{
    m_searchEdit->clear();
    m_statusFilterCombo->setCurrentIndex(0);
    m_startDateFilter->setDate(QDate::currentDate().addYears(-1));
    m_endDateFilter->setDate(QDate::currentDate().addYears(1));
    applyFilters();
}

void ContractWidget::onContractSelectionChanged()
{
    updateActionStates();
    
    Contract *contract = getSelectedContract();
    if (contract) {
        emit contractSelected(contract->id());
    }
}

void ContractWidget::onContractDoubleClicked()
{
    onEditContractClicked();
}

void ContractWidget::onTableContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    
    Contract *contract = getSelectedContract();
    if (contract) {
        contextMenu.addAction("Edit", this, &ContractWidget::onEditContractClicked);
        contextMenu.addAction("Duplicate", this, &ContractWidget::onDuplicateContractClicked);
        contextMenu.addSeparator();
        contextMenu.addAction("Delete", this, &ContractWidget::onDeleteContractClicked);
    } else {
        contextMenu.addAction("Add New Contract", this, &ContractWidget::onAddContractClicked);
    }
    
    contextMenu.exec(m_contractsTable->mapToGlobal(pos));
}

void ContractWidget::onContractAdded(const QString &contractId)
{
    Q_UNUSED(contractId)
    refreshContracts();
}

void ContractWidget::onContractUpdated(const QString &contractId)
{
    Q_UNUSED(contractId)
    refreshContracts();
}

void ContractWidget::onContractDeleted(const QString &contractId)
{
    Q_UNUSED(contractId)
    refreshContracts();
}

void ContractWidget::onDatabaseError(const QString &error)
{
    showMessage("Database error: " + error, true);
}

void ContractWidget::updateStatusBar()
{
    int totalContracts = m_contracts.size();
    int filteredContracts = m_filteredContracts.size();
    
    if (totalContracts == filteredContracts) {
        m_countLabel->setText(QString("%1 contracts").arg(totalContracts));
        m_statusLabel->setText("Ready");
    } else {
        m_countLabel->setText(QString("%1 of %2 contracts").arg(filteredContracts).arg(totalContracts));
        m_statusLabel->setText("Filtered");
    }
}

void ContractWidget::updateActionStates()
{
    Contract *contract = getSelectedContract();
    bool hasSelection = (contract != nullptr);
    
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_duplicateButton->setEnabled(hasSelection);
    m_viewDetailsButton->setEnabled(hasSelection);
}

Contract* ContractWidget::getSelectedContract() const
{
    int currentRow = m_contractsTable->currentRow();
    if (currentRow < 0 || currentRow >= m_filteredContracts.size()) {
        return nullptr;
    }
    
    return m_filteredContracts[currentRow];
}

void ContractWidget::applyFilters()
{
    if (m_isLoading) return;
    
    m_filteredContracts.clear();
    
    QString searchText = m_searchEdit->text().trimmed().toLower();
    QString statusFilter = m_statusFilterCombo->currentData().toString();
    QDate startDate = m_startDateFilter->date();
    QDate endDate = m_endDateFilter->date();
    
    for (Contract *contract : m_contracts) {
        bool matches = true;
        
        // Text search
        if (!searchText.isEmpty()) {
            QString searchableText = QString("%1 %2 %3")
                .arg(contract->clientName())
                .arg(contract->status())
                .arg(contract->description()).toLower();
            
            if (!searchableText.contains(searchText)) {
                matches = false;
            }
        }
        
        // Status filter
        if (!statusFilter.isEmpty() && contract->status() != statusFilter) {
            matches = false;
        }
        
        // Date range filter
        if (contract->startDate() < startDate || contract->endDate() > endDate) {
            matches = false;
        }
        
        if (matches) {
            m_filteredContracts.append(contract);
        }
    }
    
    populateTable(m_filteredContracts);
}

QString ContractWidget::formatCurrency(double value) const
{
    return QLocale().toCurrencyString(value);
}

QString ContractWidget::formatDate(const QDate &date) const
{
    return date.toString("dd/MM/yyyy");
}

QString ContractWidget::getStatusIcon(const QString &status) const
{
    if (status == "Active") return "✅";
    if (status == "Completed") return "✓";
    if (status == "Expired") return "⚠️";
    if (status == "Cancelled") return "❌";
    return "📄"; // Draft
}

QColor ContractWidget::getStatusColor(const QString &status) const
{
    if (status == "Active") return QColor(StyleManager::getSuccessColor());
    if (status == "Completed") return QColor("#1976D2"); // Blue
    if (status == "Expired") return QColor(StyleManager::getErrorColor());
    if (status == "Cancelled") return QColor("#757575"); // Gray
    return QColor(StyleManager::getWarningColor()); // Draft - Orange
}

void ContractWidget::showMessage(const QString &message, bool isError)
{
    m_statusLabel->setText(message);
    
    if (isError) {
        m_statusLabel->setStyleSheet("color: " + StyleManager::getErrorColor() + ";");
    } else {
        m_statusLabel->setStyleSheet("color: " + StyleManager::getSuccessColor() + ";");
    }
    
    // Reset status after 3 seconds
    QTimer::singleShot(3000, this, [this] {
        m_statusLabel->setText("Ready");
        m_statusLabel->setStyleSheet("");
    });
}

void ContractWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_contracts.isEmpty() && m_dbManager) {
        loadContracts();
    }
}

void ContractWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // Adjust column widths based on widget size
    if (m_contractsTable) {
        int totalWidth = m_contractsTable->width() - 50; // Account for scrollbar
        m_contractsTable->setColumnWidth(ClientNameColumn, totalWidth * 0.20);
        m_contractsTable->setColumnWidth(StartDateColumn, totalWidth * 0.12);
        m_contractsTable->setColumnWidth(EndDateColumn, totalWidth * 0.12);
        m_contractsTable->setColumnWidth(ValueColumn, totalWidth * 0.15);
        m_contractsTable->setColumnWidth(StatusColumn, totalWidth * 0.12);
        m_contractsTable->setColumnWidth(PaymentTermsColumn, totalWidth * 0.12);
        // Description column will stretch to fill remaining space
    }
}

// IContractWidget interface implementations
void ContractWidget::setContractService(IContractService *service)
{
    // Store the service interface
    m_contractService = service;
    
    // If service is a ContractDatabaseManager, also set it as dbManager for legacy compatibility
    if (auto dbManager = dynamic_cast<ContractDatabaseManager*>(service)) {
        setDatabaseManager(dbManager);
    }
}

void ContractWidget::clearSelection()
{
    m_contractsTable->clearSelection();
    updateActionStates();
}

void ContractWidget::addContract()
{
    onAddContractClicked();
}

void ContractWidget::editContract(const QString &contractId)
{
    // Find and select the contract first
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == contractId) {
            m_contractsTable->selectRow(row);
            break;
        }
    }
    onEditContractClicked();
}

void ContractWidget::deleteContract(const QString &contractId)
{
    // Find and select the contract first
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == contractId) {
            m_contractsTable->selectRow(row);
            break;
        }
    }
    onDeleteContractClicked();
}

void ContractWidget::duplicateContract(const QString &contractId)
{
    // Find and select the contract first
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == contractId) {
            m_contractsTable->selectRow(row);
            break;
        }
    }
    onDuplicateContractClicked();
}

void ContractWidget::setSearchFilter(const QString &filter)
{
    m_searchEdit->setText(filter);
}

void ContractWidget::setStatusFilter(const QString &status)
{    int index = m_statusFilterCombo->findText(status);
    if (index >= 0) {
        m_statusFilterCombo->setCurrentIndex(index);
    }
}

void ContractWidget::setDateRangeFilter(const QDate &startDate, const QDate &endDate)
{    m_startDateFilter->setDate(startDate);
    m_endDateFilter->setDate(endDate);
}

void ContractWidget::clearFilters()
{
    onClearFiltersClicked();
}

QList<QString> ContractWidget::getSelectedContractIds()
{
    QList<QString> contractIds;
    QList<QTableWidgetItem*> selectedItems = m_contractsTable->selectedItems();
    QSet<int> selectedRows;
    
    // Get unique rows
    for (QTableWidgetItem *item : selectedItems) {
        selectedRows.insert(item->row());
    }
    
    // Get contract IDs from selected rows
    for (int row : selectedRows) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0);
        if (item) {
            contractIds.append(item->data(Qt::UserRole).toString());
        }
    }
    
    return contractIds;
}

Contract* ContractWidget::getCurrentContract()
{
    int currentRow = m_contractsTable->currentRow();
    if (currentRow >= 0) {
        QTableWidgetItem *item = m_contractsTable->item(currentRow, 0);
        if (item) {
            QString contractId = item->data(Qt::UserRole).toString();
            for (Contract *contract : m_contracts) {
                if (contract && contract->id() == contractId) {
                    return contract;
                }
            }
        }
    }
    return nullptr;
}

void ContractWidget::selectContract(const QString &contractId)
{    // Find and select the contract in the table
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *item = m_contractsTable->item(row, 0); // Assuming ID is in first column
        if (item && item->text() == contractId) {
            m_contractsTable->selectRow(row);
            emit contractSelected(contractId);
            break;
        }
    }
}

void ContractWidget::exportSelectedContracts()
{
    // TODO: Implement export functionality for selected contracts
    QMessageBox::information(this, tr("Export"), 
                           tr("Export selected contracts functionality will be implemented soon."));
}

void ContractWidget::exportAllContracts()
{
    // TODO: Implement export functionality for all contracts
    QMessageBox::information(this, tr("Export"), 
                           tr("Export all contracts functionality will be implemented soon."));
}

void ContractWidget::setViewMode(int mode)
{
    // TODO: Implement different view modes (List, Grid, Cards)
    Q_UNUSED(mode)
    // For now, we only support table view
}

void ContractWidget::setSortOrder(int column, bool ascending)
{
    m_contractsTable->sortItems(column, ascending ? Qt::Ascending