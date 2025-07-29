#include "contractwidget.h"
#include "contract.h"
#include "contractdialog.h"
#include "contractdatabasemanager.h"
#include "contractexportmanager.h"
#include "contractchatbotdialog.h"
#include "contractimportdialog.h"
#include "contractaiassistantdialog.h"
#include "groqcontractchatbot.h"
#include "../materials/groqclient.h"
#include "interfaces/icontractchatbot.h"
#include "interfaces/icontractimporter.h"
#include "utils/stylemanager.h"
#include "utils/environmentloader.h"
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>
#include <QGroupBox>
#include <QLocale>
#include <QApplication>
#include <QKeySequence>
#include <QShortcut>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QCursor>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QDateTime>
#include <QSettings>

ContractWidget::ContractWidget(QWidget *parent)
    : QWidget(parent)
    , m_dbManager(nullptr)
    , m_contractService(nullptr)
    , m_exportManager(nullptr) // new ContractExportManager(this) - DISABLED TEMPORARILY
    , m_chatbot(nullptr)
    , m_importer(nullptr)
    , m_groqClient(nullptr)
    , m_aiDialog(nullptr)
    , m_groqChatbot(nullptr)
    , m_searchTimer(new QTimer(this))
    , m_isLoading(false)
{    qDebug() << "ContractWidget constructor starting...";    
    // Set up UI exactly like MaterialWidget
    setupUi();
    setupConnections();
    
    // Initialize AI Assistant (like MaterialWidget does)
    initializeAIAssistant();
    
    // applyArchiFlowStyling(); // TEMPORARILY DISABLED TO DEBUG CRASH
    
    qDebug() << "About to configure search timer...";
    // Configure search timer for delayed search
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300); // 300ms delay
    connect(m_searchTimer, &QTimer::timeout, this, &ContractWidget::applyFilters);
    
    qDebug() << "About to call updateActionStates...";
    // Initial button states - exactly like MaterialWidget (after UI is set up)
    updateActionStates();
    
    qDebug() << "ContractWidget constructor completed successfully";
}

ContractWidget::~ContractWidget()
{
    // Clean up contracts
    qDeleteAll(m_contracts);
}

void ContractWidget::setupUi()
{
    qDebug() << "ContractWidget setupUi starting...";
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    qDebug() << "About to call setupActions...";
    // Setup top toolbar - exactly like MaterialWidget
    setupActions();
    
    qDebug() << "About to call setupSearchAndFilter...";
    // Setup search and filters
    setupSearchAndFilter();
    
    qDebug() << "About to call setupTable...";
    // Setup table
    setupTable();
    
    qDebug() << "About to call setupStatusBar...";
    // Setup status bar
    setupStatusBar();
    
    qDebug() << "ContractWidget setupUi completed successfully";
}

void ContractWidget::setupActions()
{
    qDebug() << "ContractWidget setupActions starting...";
    
    m_actionsWidget = new QWidget(this);
    m_actionsLayout = new QHBoxLayout(m_actionsWidget);
    m_actionsLayout->setSpacing(10);
    m_actionsLayout->setContentsMargins(0, 0, 0, 0);
    
    // CRUD buttons section - exactly like MaterialWidget
    m_addButton = new QPushButton("+ Add");
    m_editButton = new QPushButton("✏ Edit");
    m_deleteButton = new QPushButton("🗑 Delete");
    m_viewDetailsButton = new QPushButton("👁 View");
    m_duplicateButton = new QPushButton("📄 Duplicate");
    
    // Style CRUD buttons
    m_addButton->setObjectName("addButton");
    m_editButton->setObjectName("editButton");
    m_deleteButton->setObjectName("deleteButton");
    m_viewDetailsButton->setObjectName("viewButton");
    m_duplicateButton->setObjectName("duplicateButton");
    
    QSize crudButtonSize(80, 40);
    m_addButton->setFixedSize(crudButtonSize);
    m_editButton->setFixedSize(crudButtonSize);
    m_deleteButton->setFixedSize(crudButtonSize);
    m_viewDetailsButton->setFixedSize(crudButtonSize);
    m_duplicateButton->setFixedSize(crudButtonSize);
    
    // Initially disable buttons that require selection - exactly like MaterialWidget
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_viewDetailsButton->setEnabled(false);
    m_duplicateButton->setEnabled(false);
    
    // Toolbar buttons section - matching MaterialWidget structure
    m_importButton = new QPushButton("Import");
    m_exportButton = new QPushButton("Export");
    m_statisticsButton = new QPushButton("Statistics");
    m_chatbotButton = new QPushButton("AI Chatbot");
    
    // Style the buttons to match MaterialWidget
    m_importButton->setObjectName("toolbarButton");
    m_exportButton->setObjectName("toolbarButton");
    m_statisticsButton->setObjectName("toolbarButton");
    m_chatbotButton->setObjectName("toolbarButton");
    
    // Set fixed sizes for consistent appearance
    QSize buttonSize(100, 40);
    m_importButton->setFixedSize(buttonSize);
    m_exportButton->setFixedSize(buttonSize);
    m_statisticsButton->setFixedSize(buttonSize);
    m_chatbotButton->setFixedSize(buttonSize);
    
    // Add CRUD buttons first
    m_actionsLayout->addWidget(m_addButton);
    m_actionsLayout->addWidget(m_editButton);
    m_actionsLayout->addWidget(m_deleteButton);
    m_actionsLayout->addWidget(m_viewDetailsButton);
    m_actionsLayout->addWidget(m_duplicateButton);
    m_actionsLayout->addSpacing(20); // Separator
    
    // Add toolbar buttons
    m_actionsLayout->addWidget(m_importButton);
    m_actionsLayout->addWidget(m_exportButton);
    m_actionsLayout->addWidget(m_statisticsButton);
    m_actionsLayout->addWidget(m_chatbotButton);
    m_actionsLayout->addStretch();
    
    // Add actions widget to main layout
    m_mainLayout->addWidget(m_actionsWidget);
}

void ContractWidget::setupToolbar()
{
    // This method is now replaced by setupActions()
    // Keeping for backward compatibility but doing nothing
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
{
    qDebug() << "ContractWidget setupConnections starting...";
    
    // CRUD button connections - exactly like MaterialWidget
    if (m_addButton) {
        connect(m_addButton, &QPushButton::clicked, this, &ContractWidget::onAddContractClicked);
    }
    if (m_editButton) {
        connect(m_editButton, &QPushButton::clicked, this, &ContractWidget::onEditContractClicked);
    }
    if (m_deleteButton) {
        connect(m_deleteButton, &QPushButton::clicked, this, &ContractWidget::onDeleteContractClicked);
    }
    if (m_viewDetailsButton) {
        connect(m_viewDetailsButton, &QPushButton::clicked, this, &ContractWidget::onViewContractDetailsClicked);
    }
    if (m_duplicateButton) {
        connect(m_duplicateButton, &QPushButton::clicked, this, &ContractWidget::onDuplicateContractClicked);
    }
    
    qDebug() << "CRUD buttons connected";
    
    // Toolbar button connections
    if (m_exportButton) {
        connect(m_exportButton, &QPushButton::clicked, this, &ContractWidget::onShowExportDialog);
    }
    if (m_statisticsButton) {
        connect(m_statisticsButton, &QPushButton::clicked, this, &ContractWidget::showStatistics);
    }
    if (m_importButton) {
        connect(m_importButton, &QPushButton::clicked, this, &ContractWidget::onImportContractsClicked);
    }
    if (m_chatbotButton) {
        connect(m_chatbotButton, &QPushButton::clicked, this, &ContractWidget::onShowChatbotClicked);
    }

    // Search and filter connections
    if (m_searchEdit) {
        connect(m_searchEdit, &QLineEdit::textChanged, this, &ContractWidget::onSearchTextChanged);
    }
    if (m_statusFilterCombo) {
        connect(m_statusFilterCombo, &QComboBox::currentTextChanged, this, &ContractWidget::onFilterChanged);
    }
    if (m_startDateFilter) {
        connect(m_startDateFilter, &QDateEdit::dateChanged, this, &ContractWidget::onDateRangeChanged);
    }
    if (m_endDateFilter) {
        connect(m_endDateFilter, &QDateEdit::dateChanged, this, &ContractWidget::onDateRangeChanged);
    }
    if (m_clearFiltersButton) {
        connect(m_clearFiltersButton, &QPushButton::clicked, this, &ContractWidget::onClearFiltersClicked);
    }

    // Table interactions - exactly like MaterialWidget
    if (m_contractsTable) {
        connect(m_contractsTable, &QTableWidget::itemSelectionChanged, this, &ContractWidget::onContractSelectionChanged);
        connect(m_contractsTable, &QTableWidget::itemDoubleClicked, this, &ContractWidget::onContractDoubleClicked);
        connect(m_contractsTable, &QTableWidget::customContextMenuRequested, this, &ContractWidget::onTableContextMenu);
    }
    
    qDebug() << "ContractWidget setupConnections completed successfully";
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
            gridline-color: %2;
            background-color: white;
            alternate-background-color: rgba(227, 198, 176, 0.1);
            selection-background-color: %2;
            selection-color: %1;
        }
        
        QTableWidget#contractsTable::item {
            padding: 8px;
            border-bottom: 1px solid rgba(227, 198, 176, 0.3);
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
    qDebug() << "Database manager set to ContractWidget:" << (dbManager ? "valid" : "null");

    if (m_dbManager) {
        connect(m_dbManager, &ContractDatabaseManager::contractAdded, this, &ContractWidget::onContractAdded);
        connect(m_dbManager, &ContractDatabaseManager::contractUpdated, this, &ContractWidget::onContractUpdated);
        connect(m_dbManager, &ContractDatabaseManager::contractDeleted, this, &ContractWidget::onContractDeleted);
        connect(m_dbManager, &ContractDatabaseManager::databaseError, this, &ContractWidget::onDatabaseError);
        
        // Load contracts when database manager is set
        qDebug() << "Loading contracts after database manager set...";
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
    loadContracts();
}

void ContractWidget::loadContracts()
{
    qDebug() << "Loading contracts...";
    
    if (!m_dbManager) {
        showMessage("Database manager not set", true);
        qDebug() << "Database manager is null in loadContracts!";
        return;
    }

    // Check database connection
    if (!m_dbManager->isDatabaseConnected()) {
        showMessage("Database connection lost. Please check your connection.", true);
        return;
    }

    m_isLoading = true;
    m_progressBar->setVisible(true);
    m_statusLabel->setText("Loading contracts...");
    
    // Set cursor to indicate loading
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    try {
        // Clear existing contracts
        qDeleteAll(m_contracts);
        m_contracts.clear();
        
        // Load from database with better error handling
        qDebug() << "Requesting all contracts from database...";
        m_contracts = m_dbManager->getAllContracts();
        qDebug() << "Loaded" << m_contracts.size() << "contracts from database";
        
        // Check for database errors
        if (m_contracts.isEmpty() && !m_dbManager->getLastError().isEmpty()) {
            throw std::runtime_error(m_dbManager->getLastError().toStdString());
        }
        
        // Apply current filters
        applyFilters();
        
        m_statusLabel->setText(QString("Loaded %1 contracts").arg(m_contracts.size()));
        
    } catch (const std::exception &e) {
        qWarning() << "Error loading contracts:" << e.what();
        showMessage(QString("Error loading contracts: %1").arg(e.what()), true);
        m_statusLabel->setText("Error loading contracts");
    }
      // Cleanup
    QApplication::restoreOverrideCursor();
    m_isLoading = false;
    m_progressBar->setVisible(false);
    updateStatusBar();
    updateActionStates(); // Update button states after loading
    
    qDebug() << "Contract loading completed";
}

void ContractWidget::populateTable(const QList<Contract*> &contracts)
{
    m_contractsTable->setRowCount(contracts.size());    
    for (int row = 0; row < contracts.size(); ++row) {
        Contract *contract = contracts[row];
        addContractToTable(contract, row); // Pass the row index
    }
    
    updateStatusBar();
    updateActionStates(); // Update button states after table is populated
}

void ContractWidget::addContractToTable(Contract *contract, int row)
{
    if (!contract) return;

    // If row is -1, append to the end (backward compatibility)
    if (row == -1) {
        row = m_contractsTable->rowCount();
        m_contractsTable->insertRow(row);
    }
    // Otherwise, use the specified row (assumed to already exist)

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
    qDebug() << "Add contract button clicked";
    
    if (!m_dbManager) {
        showMessage("Database manager not available. Cannot add contract.", true);
        qDebug() << "Database manager is null!";
        return;
    }
    
    // Check database connection before proceeding
    if (!m_dbManager->isDatabaseConnected()) {
        showMessage("Database connection lost. Please check your connection and try again.", true);
        return;
    }
    
    ContractDialog dialog(ContractDialog::AddMode, this);
    dialog.setDatabaseManager(m_dbManager);
    
    // Connect the dialog signal to refresh contracts
    connect(&dialog, &ContractDialog::contractSaved, this, [this](const QString &contractId) {
        qDebug() << "Contract saved with ID:" << contractId << "- Refreshing contracts...";
        
        // Optimized refresh - only add the new contract instead of full reload
        if (auto contract = m_dbManager->getContract(contractId)) {
            addContractToTable(contract);
            updateStatusBar();
            emit contractAdded(contractId);
            showMessage(QString("Contract '%1' added successfully").arg(contract->clientName()));
        } else {
            // Fallback to full refresh if individual fetch fails
            refreshContracts();
            emit contractAdded(contractId);
            showMessage("Contract added successfully");
        }
    });
    
    // Handle errors from dialog
    connect(&dialog, &ContractDialog::errorOccurred, this, [this](const QString &error) {
        showMessage(QString("Error adding contract: %1").arg(error), true);
    });
    
    qDebug() << "Opening contract dialog...";
    dialog.exec();
    qDebug() << "Contract dialog closed";
}

void ContractWidget::onEditContractClicked()
{
    Contract *contract = getSelectedContract();
    if (!contract) {
        showMessage("Please select a contract to edit", true);
        return;
    }
    
    // Check database connection before proceeding
    if (!m_dbManager->isDatabaseConnected()) {
        showMessage("Database connection lost. Please check your connection and try again.", true);
        return;
    }
    
    // Store original contract ID for tracking
    QString contractId = contract->id();
    
    ContractDialog dialog(ContractDialog::EditMode, this);
    dialog.setContract(contract);
    dialog.setDatabaseManager(m_dbManager);
    
    // Connect the dialog signal to refresh contracts
    connect(&dialog, &ContractDialog::contractSaved, this, [this, contractId](const QString &savedContractId) {
        qDebug() << "Contract updated with ID:" << savedContractId;
        
        // Optimized refresh - only update the specific contract
        if (auto updatedContract = m_dbManager->getContract(savedContractId)) {
            updateContractInTable(updatedContract);
            updateStatusBar();
            emit contractUpdated(savedContractId);
            showMessage(QString("Contract '%1' updated successfully").arg(updatedContract->clientName()));
        } else {
            // Fallback to full refresh if individual fetch fails
            refreshContracts();
            emit contractUpdated(savedContractId);
            showMessage("Contract updated successfully");
        }
    });
    
    // Handle errors from dialog
    connect(&dialog, &ContractDialog::errorOccurred, this, [this](const QString &error) {
        showMessage(QString("Error updating contract: %1").arg(error), true);
    });
    
    dialog.exec();
}

void ContractWidget::onDeleteContractClicked()
{
    Contract *contract = getSelectedContract();
    if (!contract) {
        showMessage("Please select a contract to delete", true);
        return;
    }
    
    // Check database connection before proceeding
    if (!m_dbManager->isDatabaseConnected()) {
        showMessage("Database connection lost. Please check your connection and try again.", true);
        return;
    }
    
    QString contractId = contract->id();
    QString clientName = contract->clientName();
    
    // Check if contract can be deleted (business rules)
    if (!m_dbManager->canDeleteContract(contractId)) {
        showMessage(QString("Contract '%1' cannot be deleted. Active contracts must be completed or cancelled first.").arg(clientName), true);
        return;
    }
    
    // Confirm deletion with detailed information
    QString confirmMessage = QString(
        "Are you sure you want to delete the following contract?\n\n"
        "Client: %1\n"
        "Value: %2\n"
        "Status: %3\n"
        "Start Date: %4\n"
        "End Date: %5\n\n"
        "This action cannot be undone."
    ).arg(clientName)
     .arg(formatCurrency(contract->value()))
     .arg(contract->status())
     .arg(contract->startDate().toString("dd/MM/yyyy"))
     .arg(contract->endDate().toString("dd/MM/yyyy"));
    
    int result = QMessageBox::question(this, "Delete Contract", confirmMessage,
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);

    if (result == QMessageBox::Yes) {
        // Show progress during deletion
        QApplication::setOverrideCursor(Qt::WaitCursor);
        
        if (m_dbManager->deleteContract(contractId)) {
            // Optimized removal - directly remove from table
            removeContractFromTable(contractId);
            updateStatusBar();
            
            QApplication::restoreOverrideCursor();
            showMessage(QString("Contract '%1' deleted successfully").arg(clientName));
            emit contractDeleted(contractId);
        } else {
            QApplication::restoreOverrideCursor();
            QString errorMsg = m_dbManager->getLastError();
            showMessage(QString("Failed to delete contract '%1': %2").arg(clientName, errorMsg), true);
            qWarning() << "Delete contract failed:" << errorMsg;
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
    qDebug() << "Contract selection changed";
    qDebug() << "Table currentRow:" << m_contractsTable->currentRow();
    qDebug() << "Table selectedRows count:" << m_contractsTable->selectionModel()->selectedRows().size();
    qDebug() << "Filtered contracts size:" << m_filteredContracts.size();
    
    // Use currentRow like getSelectedContract() does for consistency
    Contract *contract = getSelectedContract();
    bool hasSelection = (contract != nullptr);
    
    if (hasSelection) {
        // Enable edit/delete buttons - exactly like MaterialWidget
        m_editButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
        m_viewDetailsButton->setEnabled(true);
        m_duplicateButton->setEnabled(true);
        
        qDebug() << "Selected contract:" << contract->clientName();
        emit contractSelected(contract->id());
    } else {
        // Disable buttons when no selection - exactly like MaterialWidget
        m_editButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
        m_viewDetailsButton->setEnabled(false);
        m_duplicateButton->setEnabled(false);
        
        qDebug() << "No contract selected";
    }
    
    emit selectionChanged();
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
    qDebug() << "updateActionStates: Starting...";
    
    Contract *contract = getSelectedContract();
    bool hasSelection = (contract != nullptr);
    bool databaseConnected = (m_dbManager && m_dbManager->isDatabaseConnected());
    
    qDebug() << "updateActionStates: hasSelection=" << hasSelection 
             << "databaseConnected=" << databaseConnected;
    
    // Check if buttons exist before enabling/disabling them
    if (!m_editButton || !m_deleteButton || !m_duplicateButton || !m_viewDetailsButton) {
        qWarning() << "updateActionStates: Some buttons are null!";
        return;
    }
    
    // Enable/disable buttons based on selection and database state
    m_editButton->setEnabled(hasSelection && databaseConnected);
    m_deleteButton->setEnabled(hasSelection && databaseConnected);
    m_duplicateButton->setEnabled(hasSelection && databaseConnected);
    m_viewDetailsButton->setEnabled(hasSelection);
    
    // Add button should only depend on database connection
    if (m_addButton) {
        m_addButton->setEnabled(databaseConnected);
    }
    
    // Export/statistics buttons depend on having data
    bool hasContracts = !m_filteredContracts.isEmpty();
    if (m_exportButton) {
        m_exportButton->setEnabled(hasContracts);
    }
    if (m_statisticsButton) {
        m_statisticsButton->setEnabled(hasContracts);
    }
    
    qDebug() << "updateActionStates: Completed successfully";
}

Contract* ContractWidget::getSelectedContract() const
{
    int currentRow = m_contractsTable->currentRow();
    qDebug() << "getSelectedContract: currentRow =" << currentRow 
             << "filteredContracts.size() =" << m_filteredContracts.size();
    
    if (currentRow < 0 || currentRow >= m_filteredContracts.size()) {
        qDebug() << "No valid selection - returning nullptr";
        return nullptr;
    }
    
    Contract* contract = m_filteredContracts[currentRow];
    qDebug() << "Selected contract:" << (contract ? contract->clientName() : "null");
    return contract;
}

QList<Contract*> ContractWidget::getSelectedContracts() const
{
    QList<Contract*> selectedContracts;
    
    QList<QTableWidgetItem*> selectedItems = m_contractsTable->selectedItems();
    QSet<int> selectedRows;
    
    // Get unique row numbers from selected items
    for (QTableWidgetItem* item : selectedItems) {
        selectedRows.insert(item->row());
    }
    
    // Convert row numbers to contracts
    for (int row : selectedRows) {
        if (row >= 0 && row < m_filteredContracts.size()) {
            selectedContracts.append(m_filteredContracts[row]);
        }
    }
    
    return selectedContracts;
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
    openAIAssistant();
}

void ContractWidget::openAIAssistant()
{
    if (!m_groqClient || !m_groqClient->isConnected()) {
        showAISetupDialog();
        return;
    }
    
    if (!m_aiDialog) {
        m_aiDialog = new ContractAIAssistantDialog(this);
        m_aiDialog->setGroqClient(m_groqClient);
        m_aiDialog->setContractDatabaseManager(m_dbManager);
        
        // Set current contract context if one is selected
        Contract *currentContract = getCurrentContract();
        if (currentContract) {
            m_aiDialog->setCurrentContract(currentContract);
        }
    }
    
    m_aiDialog->show();
    m_aiDialog->raise();
    m_aiDialog->activateWindow();
    
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
    // Create bulk operations menu
    QMenu bulkMenu(this);
    bulkMenu.setTitle("Bulk Operations");
    
    // CRUD operations
    QAction *addMultipleAction = bulkMenu.addAction(QIcon(":/icons/add.png"), "Add Multiple Contracts");
    QAction *editMultipleAction = bulkMenu.addAction(QIcon(":/icons/edit.png"), "Edit Multiple Contracts");
    QAction *deleteMultipleAction = bulkMenu.addAction(QIcon(":/icons/delete.png"), "Delete Multiple Contracts");
    QAction *bulkStatusAction = bulkMenu.addAction(QIcon(":/icons/status.png"), "Update Status");
    
    bulkMenu.addSeparator();
    
    // Export operations
    QAction *bulkExportAction = bulkMenu.addAction(QIcon(":/icons/export.png"), "Export Selected");
    
    bulkMenu.addSeparator();
    
    // Database operations
    QAction *syncAction = bulkMenu.addAction(QIcon(":/icons/sync.png"), "Synchronize Database");
    QAction *optimizeAction = bulkMenu.addAction(QIcon(":/icons/optimize.png"), "Optimize Database");
    QAction *backupAction = bulkMenu.addAction(QIcon(":/icons/backup.png"), "Backup Database");
    QAction *restoreAction = bulkMenu.addAction(QIcon(":/icons/restore.png"), "Restore Database");
    
    bulkMenu.addSeparator();
    
    // Cache operations
    QAction *refreshCacheAction = bulkMenu.addAction(QIcon(":/icons/refresh.png"), "Refresh Cache");
    QAction *statusAction = bulkMenu.addAction(QIcon(":/icons/info.png"), "Database Status");
    
    // Check if contracts are selected for operations that need them
    QList<Contract*> selectedContracts = getSelectedContracts();
    bool hasSelection = !selectedContracts.isEmpty();
    
    editMultipleAction->setEnabled(hasSelection);
    deleteMultipleAction->setEnabled(hasSelection);
    bulkStatusAction->setEnabled(hasSelection);
    bulkExportAction->setEnabled(hasSelection);
    
    // Execute selected action
    QAction *selectedAction = bulkMenu.exec(QCursor::pos());
    
    if (selectedAction == addMultipleAction) {
        addMultipleContracts();
    } else if (selectedAction == editMultipleAction) {
        editMultipleContracts();
    } else if (selectedAction == deleteMultipleAction) {
        deleteMultipleContracts();
    } else if (selectedAction == bulkStatusAction) {
        bulkUpdateStatus();
    } else if (selectedAction == bulkExportAction) {
        exportSelectedContracts();
    } else if (selectedAction == syncAction) {
        synchronizeDatabase();
    } else if (selectedAction == optimizeAction) {
        optimizeDatabase();
    } else if (selectedAction == backupAction) {
        backupDatabase();
    } else if (selectedAction == restoreAction) {
        restoreDatabase();
    } else if (selectedAction == refreshCacheAction) {        refreshDatabaseCache();
    } else if (selectedAction == statusAction) {
        showDatabaseStatus();
    }
}

void ContractWidget::addMultipleContracts()
{
    if (!m_dbManager || !m_dbManager->isDatabaseConnected()) {
        showMessage("Database not available for batch operations", true);
        return;
    }
    
    bool ok;
    int count = QInputDialog::getInt(this, "Add Multiple Contracts", 
                                   "Number of contracts to add:", 1, 1, 100, 1, &ok);
    if (!ok) return;
    
    QList<Contract*> contracts;
    QStringList addedIds;
    QString errorMessage;
    
    // Create multiple contract dialogs
    for (int i = 0; i < count; ++i) {
        ContractDialog dialog(ContractDialog::AddMode, this);
        dialog.setDatabaseManager(m_dbManager);
        dialog.setWindowTitle(QString("Add Contract %1 of %2").arg(i + 1).arg(count));
        
        if (dialog.exec() == QDialog::Accepted) {
            if (Contract* contract = dialog.getContract()) {
                contracts.append(new Contract(*contract));
            }
        } else {
            break; // User cancelled
        }
    }
    
    if (!contracts.isEmpty()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        
        if (m_dbManager->addContracts(contracts, addedIds, errorMessage)) {
            QApplication::restoreOverrideCursor();
            showMessage(QString("Successfully added %1 contracts").arg(addedIds.size()));
            refreshContracts();
            
            // Emit signals for each added contract
            for (const QString &contractId : addedIds) {
                emit contractAdded(contractId);
            }
        } else {
            QApplication::restoreOverrideCursor();
            showMessage(QString("Batch add failed: %1").arg(errorMessage), true);
        }
    }
    
    // Cleanup
    qDeleteAll(contracts);
}

void ContractWidget::editMultipleContracts()
{
    QList<Contract*> selectedContracts = getSelectedContracts();
    if (selectedContracts.isEmpty()) {
        showMessage("Please select contracts to edit", true);
        return;
    }
    
    if (!m_dbManager || !m_dbManager->isDatabaseConnected()) {
        showMessage("Database not available for batch operations", true);
        return;
    }
    
    // Show bulk edit dialog
    QDialog dialog(this);
    dialog.setWindowTitle(QString("Bulk Edit %1 Contracts").arg(selectedContracts.size()));
    dialog.setModal(true);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    // Add common fields for bulk editing
    QFormLayout *formLayout = new QFormLayout;
    
    QComboBox *statusCombo = new QComboBox;
    statusCombo->addItem("Keep Current", "");
    for (const QString &status : Contract::availableStatuses()) {
        statusCombo->addItem(status, status);
    }
    formLayout->addRow("Status:", statusCombo);
    
    QSpinBox *paymentTermsSpin = new QSpinBox;
    paymentTermsSpin->setRange(0, 365);
    paymentTermsSpin->setSpecialValueText("Keep Current");
    paymentTermsSpin->setValue(0);
    formLayout->addRow("Payment Terms:", paymentTermsSpin);
    
    QCheckBox *updateDescription = new QCheckBox("Update Description");
    QTextEdit *descriptionEdit = new QTextEdit;
    descriptionEdit->setMaximumHeight(100);
    descriptionEdit->setEnabled(false);
    connect(updateDescription, &QCheckBox::toggled, descriptionEdit, &QTextEdit::setEnabled);
    formLayout->addRow(updateDescription);
    formLayout->addRow("Description:", descriptionEdit);
    
    layout->addLayout(formLayout);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *applyBtn = new QPushButton("Apply Changes");
    QPushButton *cancelBtn = new QPushButton("Cancel");
    buttonLayout->addStretch();
    buttonLayout->addWidget(applyBtn);
    buttonLayout->addWidget(cancelBtn);
    layout->addLayout(buttonLayout);
    
    connect(applyBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Apply bulk changes
        QList<Contract*> contractsToUpdate;
        
        for (Contract* contract : selectedContracts) {
            Contract* updatedContract = new Contract(*contract);
            
            if (!statusCombo->currentData().toString().isEmpty()) {
                updatedContract->setStatus(statusCombo->currentData().toString());
            }
            
            if (paymentTermsSpin->value() > 0) {
                updatedContract->setPaymentTerms(paymentTermsSpin->value());
            }
            
            if (updateDescription->isChecked()) {
                updatedContract->setDescription(descriptionEdit->toPlainText());
            }
            
            contractsToUpdate.append(updatedContract);
        }
        
        QString errorMessage;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        
        if (m_dbManager->updateContracts(contractsToUpdate, errorMessage)) {
            QApplication::restoreOverrideCursor();
            showMessage(QString("Successfully updated %1 contracts").arg(contractsToUpdate.size()));
            refreshContracts();
            
            // Emit signals for each updated contract
            for (Contract* contract : contractsToUpdate) {
                emit contractUpdated(contract->id());
            }
        } else {
            QApplication::restoreOverrideCursor();
            showMessage(QString("Batch update failed: %1").arg(errorMessage), true);
        }
        
        qDeleteAll(contractsToUpdate);
    }
}

void ContractWidget::deleteMultipleContracts()
{
    QList<Contract*> selectedContracts = getSelectedContracts();
    if (selectedContracts.isEmpty()) {
        showMessage("Please select contracts to delete", true);
        return;
    }
    
    if (!m_dbManager || !m_dbManager->isDatabaseConnected()) {
        showMessage("Database not available for batch operations", true);
        return;
    }
    
    // Check which contracts can be deleted
    QStringList deletableIds;
    QStringList undeletableNames;
    
    for (Contract* contract : selectedContracts) {
        if (m_dbManager->canDeleteContract(contract->id())) {
            deletableIds.append(contract->id());
        } else {
            undeletableNames.append(contract->clientName());
        }
    }
    
    if (deletableIds.isEmpty()) {
        showMessage("None of the selected contracts can be deleted (active contracts must be completed or cancelled first)", true);
        return;
    }
    
    QString message = QString("Are you sure you want to delete %1 contracts?").arg(deletableIds.size());
    if (!undeletableNames.isEmpty()) {
        message += QString("\n\nNote: %1 contracts cannot be deleted:\n%2")
                  .arg(undeletableNames.size())
                  .arg(undeletableNames.join(", "));
    }
    message += "\n\nThis action cannot be undone.";
    
    int result = QMessageBox::question(this, "Delete Multiple Contracts", message,
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        QString errorMessage;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        
        if (m_dbManager->deleteContracts(deletableIds, errorMessage)) {
            QApplication::restoreOverrideCursor();
            showMessage(QString("Successfully deleted %1 contracts").arg(deletableIds.size()));
            refreshContracts();
            
            // Emit signals for each deleted contract
            for (const QString &contractId : deletableIds) {
                emit contractDeleted(contractId);
            }
        } else {
            QApplication::restoreOverrideCursor();
            showMessage(QString("Batch delete failed: %1").arg(errorMessage), true);
        }
    }
}

void ContractWidget::bulkUpdateStatus()
{
    QList<Contract*> selectedContracts = getSelectedContracts();
    if (selectedContracts.isEmpty()) {
        showMessage("Please select contracts to update status", true);
        return;
    }
    
    bool ok;
    QString newStatus = QInputDialog::getItem(this, "Bulk Status Update",
                                            "Select new status for selected contracts:",
                                            Contract::availableStatuses(), 0, false, &ok);
    if (!ok) return;
    
    // Create updated contracts
    QList<Contract*> contractsToUpdate;
    for (Contract* contract : selectedContracts) {
        Contract* updatedContract = new Contract(*contract);
        updatedContract->setStatus(newStatus);
        contractsToUpdate.append(updatedContract);
    }
    
    QString errorMessage;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    if (m_dbManager->updateContracts(contractsToUpdate, errorMessage)) {
        QApplication::restoreOverrideCursor();
        showMessage(QString("Successfully updated status to '%1' for %2 contracts")
                   .arg(newStatus).arg(contractsToUpdate.size()));
        refreshContracts();
        
        // Emit signals for each updated contract
        for (Contract* contract : contractsToUpdate) {
            emit contractUpdated(contract->id());
        }
    } else {
        QApplication::restoreOverrideCursor();
        showMessage(QString("Bulk status update failed: %1").arg(errorMessage), true);
    }
    
    qDeleteAll(contractsToUpdate);
}

void ContractWidget::synchronizeDatabase()
{
    if (!m_dbManager || !m_dbManager->isDatabaseConnected()) {
        showMessage("Database not available for synchronization", true);
        return;
    }
    
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    if (m_dbManager->synchronizeDatabase()) {
        QApplication::restoreOverrideCursor();
        showMessage("Database synchronized successfully");
        refreshContracts();
    } else {
        QApplication::restoreOverrideCursor();
        showMessage(QString("Database synchronization failed: %1").arg(m_dbManager->getLastError()), true);
    }
}

void ContractWidget::optimizeDatabase()
{
    if (!m_dbManager || !m_dbManager->isDatabaseConnected()) {
        showMessage("Database not available for optimization", true);
        return;
    }
    
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    if (m_dbManager->optimizeDatabase()) {
        QApplication::restoreOverrideCursor();
        showMessage("Database optimized successfully");
    } else {
        QApplication::restoreOverrideCursor();
        showMessage("Database optimization failed", true);
    }
}

void ContractWidget::backupDatabase()
{
    if (!m_dbManager || !m_dbManager->isDatabaseConnected()) {
        showMessage("Database not available for backup", true);
        return;
    }
    
    QString backupPath = QFileDialog::getSaveFileName(this, "Backup Database",
                                                    QString("contracts_backup_%1.db")
                                                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")),
                                                    "Database Files (*.db)");
    if (backupPath.isEmpty()) return;
    
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    if (m_dbManager->backupDatabase(backupPath)) {
        QApplication::restoreOverrideCursor();
        showMessage(QString("Database backed up successfully to: %1").arg(backupPath));
    } else {
        QApplication::restoreOverrideCursor();
        showMessage(QString("Database backup failed: %1").arg(m_dbManager->getLastError()), true);
    }
}

void ContractWidget::restoreDatabase()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QString backupPath = QFileDialog::getOpenFileName(this, "Restore Database",
                                                    "", "Database Files (*.db)");
    if (backupPath.isEmpty()) return;
    
    int result = QMessageBox::warning(this, "Restore Database",
                                    "Restoring from backup will replace all current data.\n\n"
                                    "Are you sure you want to continue?",
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        
        if (m_dbManager->restoreDatabase(backupPath)) {
            QApplication::restoreOverrideCursor();
            showMessage("Database restored successfully");
            refreshContracts();
        } else {
            QApplication::restoreOverrideCursor();
            showMessage(QString("Database restore failed: %1").arg(m_dbManager->getLastError()), true);
        }
    }
}

void ContractWidget::refreshDatabaseCache()
{
    if (!m_dbManager || !m_dbManager->isDatabaseConnected()) {
        showMessage("Database not available", true);
        return;
    }
    
    m_dbManager->refreshCache();
    showMessage("Database cache refreshed");
}

void ContractWidget::showDatabaseStatus()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Database Status");
    dialog.setModal(true);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QTextEdit *statusText = new QTextEdit;
    statusText->setReadOnly(true);
    
    QString status;
    status += QString("Connection Status: %1\n").arg(m_dbManager->isDatabaseConnected() ? "Connected" : "Disconnected");
    status += QString("Caching Enabled: %1\n").arg(m_dbManager->isCachingEnabled() ? "Yes" : "No");
    if (m_dbManager->isCachingEnabled()) {
        status += QString("Cache Size: %1 contracts\n").arg(m_dbManager->getCacheSize());
    }
    status += QString("Total Contracts: %1\n").arg(m_dbManager->getContractCount());
    status += QString("Active Contracts: %1\n").arg(m_dbManager->getActiveContractsCount());
    status += QString("Expired Contracts: %1\n").arg(m_dbManager->getExpiredContracts());
    status += QString("Total Contract Value: %1\n").arg(formatCurrency(m_dbManager->getTotalContractValue()));
    
    if (!m_dbManager->getLastError().isEmpty()) {
        status += QString("\nLast Error: %1").arg(m_dbManager->getLastError());
    }
    
    statusText->setPlainText(status);
    layout->addWidget(statusText);
    
    QPushButton *closeBtn = new QPushButton("Close");    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    dialog.exec();
}

void ContractWidget::updateContractInTable(Contract *contract)
{
    if (!contract || !m_contractsTable) {
        return;
    }
    
    // Find the row with the matching contract ID
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = m_contractsTable->item(row, 0);
        if (idItem && idItem->text() == contract->id()) {
            // Update the row with new contract data
            m_contractsTable->setItem(row, 1, new QTableWidgetItem(contract->clientName()));
            m_contractsTable->setItem(row, 2, new QTableWidgetItem(contract->description()));
            m_contractsTable->setItem(row, 3, new QTableWidgetItem(QString::number(contract->value(), 'f', 2)));
            m_contractsTable->setItem(row, 4, new QTableWidgetItem(contract->startDate().toString("yyyy-MM-dd")));
            m_contractsTable->setItem(row, 5, new QTableWidgetItem(contract->endDate().toString("yyyy-MM-dd")));
            m_contractsTable->setItem(row, 6, new QTableWidgetItem(contract->status()));
            break;
        }
    }
}

void ContractWidget::removeContractFromTable(const QString &contractId)
{
    if (!m_contractsTable || contractId.isEmpty()) {
        return;
    }
    
    // Find and remove the row with the matching contract ID
    for (int row = 0; row < m_contractsTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = m_contractsTable->item(row, 0);
        if (idItem && idItem->text() == contractId) {
            m_contractsTable->removeRow(row);
            break;
        }
    }
}

void ContractWidget::initializeAIAssistant()
{
    // Initialize Groq client
    m_groqClient = new GroqClient(this);
    
    GroqConfig config;
    
    // Try to get API key from environment first
    QString apiKey = EnvironmentLoader::getEnv("GROQ_API_KEY");
    if (apiKey.isEmpty()) {
        // Try settings
        QSettings settings;
        apiKey = settings.value("AI/GroqApiKey", "").toString();
    }
    
    if (!apiKey.isEmpty()) {
        config.apiKey = apiKey;
    } else {
        qDebug() << "Using provided API key for contract management";
    }
    
    config.baseUrl = "https://api.groq.com/openai/v1";
    config.model = "llama-3.3-70b-versatile";
    config.timeout = 30000;
    config.maxTokens = 4096;
    config.temperature = 0.7;
    
    m_groqClient->setConfiguration(config);
    
    // Set contract-specific system prompt
    QString systemPrompt = 
        "You are an AI assistant specialized in contract management and legal document analysis for architecture and construction projects. "
        "You help users with contract review, risk assessment, clause analysis, compliance checking, payment terms evaluation, "
        "and contract lifecycle management. Provide clear, professional, and legally-informed assistance while noting that "
        "users should consult with qualified legal professionals for important decisions. Focus on construction industry best practices "
        "and ArchiFlow application features. When analyzing contracts, consider typical construction project phases, "
        "milestone payments, material delivery schedules, and industry-standard clauses.";
    
    m_groqClient->setSystemPrompt(systemPrompt);
    
    // Create the contract-specific chatbot
    m_groqChatbot = new GroqContractChatbot(m_groqClient, this);
    
    // Connect to the legacy interface for backward compatibility
    m_chatbot = m_groqChatbot;
    
    // Initialize AI dialog
    m_aiDialog = new ContractAIAssistantDialog(this);
    m_aiDialog->setGroqClient(m_groqClient);
}

void ContractWidget::showAISetupDialog()
{
    if (!m_groqClient || !m_groqClient->isConnected()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("AI Assistant Setup");
        msgBox.setIcon(QMessageBox::Information);
        
        QString message = 
            "The AI Assistant requires a Groq API key to function.\n\n"
            "To get started:\n"
            "• Get a free API key from console.groq.com\n"
            "• Sign up for a free account\n"
            "You can set the API key in the application settings or as an environment variable 'GROQ_API_KEY'.";
        
        msgBox.setText(message);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    
    // If we have a client but it's not connected, show connection error
    QMessageBox::warning(this, "Connection Error", 
                        "Unable to connect to the AI service. Please check your internet connection and try again.");
}
