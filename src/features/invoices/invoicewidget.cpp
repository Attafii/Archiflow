#include "invoicewidget.h"
#include "invoice.h"
#include "invoiceitem.h"
#include "client.h"
#include "invoicedatabasemanager.h"
#include "invoicedialog.h"
#include "clientdialog.h"
#include "invoicepdfgenerator.h"
#include "invoiceaiassistantdialog.h"
#include "../materials/groqclient.h"
#include "../../utils/environmentloader.h"
#include "../../database/databaseservice.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QSplitter>
#include <QHeaderView>
#include <QMenu>
#include <QDebug>
#include <QUuid>
#include <QTime>

InvoiceWidget::InvoiceWidget(QWidget *parent)
    : QWidget(parent)
    , m_dbManager(nullptr)
    , m_pdfGenerator(nullptr)
    , m_groqClient(nullptr)
    , m_aiDialog(nullptr)
    , m_searchTimer(new QTimer(this))
    , m_isLoading(false)
    , m_currentTab(0)
{
    qDebug() << "InvoiceWidget: Initializing...";
    
    // Initialize database manager
    m_dbManager = new InvoiceDatabaseManager(this);
    if (!m_dbManager->initialize()) {
        qWarning() << "Failed to initialize invoice database:" << m_dbManager->lastError();
    }
    
    // Initialize PDF generator
    m_pdfGenerator = new InvoicePDFGenerator(this);
    
    // Setup search timer
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300);
    
    setupUI();
    setupConnections();
    initializeAIAssistant();
    
    // Load initial data
    refreshInvoices();
    refreshClients();
    updateDashboard();
    
    qDebug() << "InvoiceWidget: Initialization complete";
}

InvoiceWidget::~InvoiceWidget()
{
    qDeleteAll(m_invoices);
    qDeleteAll(m_clients);
}

void InvoiceWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    
    setupDashboardTab();
    setupInvoiceTab();
    setupClientTab();
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Apply styling
    setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #c0c0c0;
            background-color: #f5f5f5;
        }
        QTabBar::tab {
            background-color: #e0e0e0;
            padding: 8px 16px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #4a90e2;
            color: white;
        }
        QTableWidget {
            alternate-background-color: #f9f9f9;
            selection-background-color: #4a90e2;
        }
    )");
}

void InvoiceWidget::setupDashboardTab()
{
    m_dashboardTab = new QWidget();
    m_dashboardLayout = new QVBoxLayout(m_dashboardTab);
    
    // Statistics cards container
    m_statsContainer = new QWidget();
    m_statsLayout = new QHBoxLayout(m_statsContainer);
    
    // Create statistic cards
    m_totalRevenueCard = createStatCard("Total Revenue", "$0.00", "All time");
    m_totalInvoicesCard = createStatCard("Total Invoices", "0", "All invoices");
    m_paidInvoicesCard = createStatCard("Paid Invoices", "0", "Completed");
    m_overdueInvoicesCard = createStatCard("Overdue", "0", "Past due");
    m_draftInvoicesCard = createStatCard("Draft", "0", "In progress");
    m_pendingAmountCard = createStatCard("Pending Amount", "$0.00", "Unpaid");
    
    m_statsLayout->addWidget(m_totalRevenueCard);
    m_statsLayout->addWidget(m_totalInvoicesCard);
    m_statsLayout->addWidget(m_paidInvoicesCard);
    m_statsLayout->addWidget(m_overdueInvoicesCard);
    m_statsLayout->addWidget(m_draftInvoicesCard);
    m_statsLayout->addWidget(m_pendingAmountCard);
    
    // Recent activity list
    m_recentActivityList = new QListWidget();
    m_recentActivityList->setMaximumHeight(200);
    
    QLabel *activityLabel = new QLabel("Recent Activity");
    activityLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin: 10px 0;");
    
    m_dashboardLayout->addWidget(m_statsContainer);
    m_dashboardLayout->addWidget(activityLabel);
    m_dashboardLayout->addWidget(m_recentActivityList);
    m_dashboardLayout->addStretch();
    
    m_tabWidget->addTab(m_dashboardTab, "Dashboard");
}

void InvoiceWidget::setupInvoiceTab()
{
    m_invoiceTab = new QWidget();
    m_invoiceLayout = new QVBoxLayout(m_invoiceTab);
    
    // Toolbar
    m_invoiceToolbar = new QHBoxLayout();
    
    // Search and filters
    m_invoiceSearchEdit = new QLineEdit();
    m_invoiceSearchEdit->setPlaceholderText("Search invoices...");
    m_invoiceSearchEdit->setMaximumWidth(200);
    
    m_statusFilterCombo = new QComboBox();
    m_statusFilterCombo->addItem("All Statuses");
    m_statusFilterCombo->addItems(Invoice::availableStatuses());
    
    m_dateFromEdit = new QDateEdit();
    m_dateFromEdit->setDate(QDate::currentDate().addDays(-30));
    m_dateToEdit = new QDateEdit();
    m_dateToEdit->setDate(QDate::currentDate());
    
    m_clearFiltersBtn = new QPushButton("Clear Filters");
    
    // Action buttons
    m_addInvoiceBtn = new QPushButton("Add Invoice");
    m_editInvoiceBtn = new QPushButton("Edit");
    m_deleteInvoiceBtn = new QPushButton("Delete");
    m_duplicateInvoiceBtn = new QPushButton("Duplicate");
    m_viewInvoiceBtn = new QPushButton("View");
    m_generatePDFBtn = new QPushButton("Generate PDF");
    m_sendInvoiceBtn = new QPushButton("Send");
    m_markPaidBtn = new QPushButton("Mark Paid");
    m_aiAssistantBtn = new QPushButton("AI Assistant");
    
    // Initially disable action buttons
    m_editInvoiceBtn->setEnabled(false);
    m_deleteInvoiceBtn->setEnabled(false);
    m_duplicateInvoiceBtn->setEnabled(false);
    m_viewInvoiceBtn->setEnabled(false);
    m_generatePDFBtn->setEnabled(false);
    m_sendInvoiceBtn->setEnabled(false);
    m_markPaidBtn->setEnabled(false);
    
    m_invoiceToolbar->addWidget(m_invoiceSearchEdit);
    m_invoiceToolbar->addWidget(m_statusFilterCombo);
    m_invoiceToolbar->addWidget(m_dateFromEdit);
    m_invoiceToolbar->addWidget(m_dateToEdit);
    m_invoiceToolbar->addWidget(m_clearFiltersBtn);
    m_invoiceToolbar->addStretch();
    m_invoiceToolbar->addWidget(m_addInvoiceBtn);
    m_invoiceToolbar->addWidget(m_editInvoiceBtn);
    m_invoiceToolbar->addWidget(m_deleteInvoiceBtn);
    m_invoiceToolbar->addWidget(m_duplicateInvoiceBtn);
    m_invoiceToolbar->addWidget(m_viewInvoiceBtn);
    m_invoiceToolbar->addWidget(m_generatePDFBtn);
    m_invoiceToolbar->addWidget(m_sendInvoiceBtn);
    m_invoiceToolbar->addWidget(m_markPaidBtn);
    m_invoiceToolbar->addWidget(m_aiAssistantBtn);
    
    // Create splitter for table and detail panel
    m_invoiceSplitter = new QSplitter(Qt::Horizontal);
    
    // Invoice table
    m_invoiceTable = new QTableWidget();
    m_invoiceTable->setColumnCount(InvoiceColumnCount);
    QStringList headers = {"Invoice #", "Client", "Date", "Due Date", "Amount", "Status"};
    m_invoiceTable->setHorizontalHeaderLabels(headers);
    m_invoiceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_invoiceTable->setAlternatingRowColors(true);
    m_invoiceTable->horizontalHeader()->setStretchLastSection(true);
    m_invoiceTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Invoice detail panel
    setupInvoiceDetailPanel();
    
    m_invoiceSplitter->addWidget(m_invoiceTable);
    m_invoiceSplitter->addWidget(m_invoiceDetailPanel);
    m_invoiceSplitter->setSizes({600, 300});
    
    m_invoiceLayout->addLayout(m_invoiceToolbar);
    m_invoiceLayout->addWidget(m_invoiceSplitter);
    
    m_tabWidget->addTab(m_invoiceTab, "Invoices");
}

void InvoiceWidget::setupInvoiceDetailPanel()
{
    m_invoiceDetailPanel = new QWidget();
    m_invoiceDetailLayout = new QVBoxLayout(m_invoiceDetailPanel);
    
    m_invoiceDetailTitle = new QLabel("Invoice Details");
    m_invoiceDetailTitle->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    
    m_invoiceNumberLabel = new QLabel("Invoice #: ");
    m_clientNameLabel = new QLabel("Client: ");
    m_invoiceDateLabel = new QLabel("Date: ");
    m_dueDateLabel = new QLabel("Due Date: ");
    m_totalAmountLabel = new QLabel("Total: ");
    m_statusLabel = new QLabel("Status: ");
    
    m_notesDisplay = new QTextEdit();
    m_notesDisplay->setMaximumHeight(80);
    m_notesDisplay->setReadOnly(true);
    
    // Items table
    m_itemsTable = new QTableWidget();
    m_itemsTable->setColumnCount(5);
    QStringList itemHeaders = {"Description", "Qty", "Unit", "Price", "Total"};
    m_itemsTable->setHorizontalHeaderLabels(itemHeaders);
    m_itemsTable->horizontalHeader()->setStretchLastSection(true);
    
    m_invoiceDetailLayout->addWidget(m_invoiceDetailTitle);
    m_invoiceDetailLayout->addWidget(m_invoiceNumberLabel);
    m_invoiceDetailLayout->addWidget(m_clientNameLabel);
    m_invoiceDetailLayout->addWidget(m_invoiceDateLabel);
    m_invoiceDetailLayout->addWidget(m_dueDateLabel);
    m_invoiceDetailLayout->addWidget(m_totalAmountLabel);
    m_invoiceDetailLayout->addWidget(m_statusLabel);
    m_invoiceDetailLayout->addWidget(new QLabel("Notes:"));
    m_invoiceDetailLayout->addWidget(m_notesDisplay);
    m_invoiceDetailLayout->addWidget(new QLabel("Items:"));
    m_invoiceDetailLayout->addWidget(m_itemsTable);
}

void InvoiceWidget::setupClientTab()
{
    m_clientTab = new QWidget();
    m_clientLayout = new QVBoxLayout(m_clientTab);
    
    // Client toolbar
    m_clientToolbar = new QHBoxLayout();
    
    m_clientSearchEdit = new QLineEdit();
    m_clientSearchEdit->setPlaceholderText("Search clients...");
    m_clientSearchEdit->setMaximumWidth(200);
    
    m_addClientBtn = new QPushButton("Add Client");
    m_editClientBtn = new QPushButton("Edit Client");
    m_deleteClientBtn = new QPushButton("Delete Client");
    
    m_editClientBtn->setEnabled(false);
    m_deleteClientBtn->setEnabled(false);
    
    m_clientToolbar->addWidget(m_clientSearchEdit);
    m_clientToolbar->addStretch();
    m_clientToolbar->addWidget(m_addClientBtn);
    m_clientToolbar->addWidget(m_editClientBtn);
    m_clientToolbar->addWidget(m_deleteClientBtn);
    
    // Client table
    m_clientTable = new QTableWidget();
    m_clientTable->setColumnCount(ClientColumnCount);
    QStringList clientHeaders = {"Name", "Email", "Phone", "Company"};
    m_clientTable->setHorizontalHeaderLabels(clientHeaders);
    m_clientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_clientTable->setAlternatingRowColors(true);
    m_clientTable->horizontalHeader()->setStretchLastSection(true);
    m_clientTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    m_clientLayout->addLayout(m_clientToolbar);
    m_clientLayout->addWidget(m_clientTable);
    
    m_tabWidget->addTab(m_clientTab, "Clients");
}

void InvoiceWidget::setupConnections()
{
    // Tab changes
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &InvoiceWidget::onTabChanged);
    
    // Search and filters
    connect(m_invoiceSearchEdit, &QLineEdit::textChanged, this, &InvoiceWidget::onSearchTextChanged);
    connect(m_clientSearchEdit, &QLineEdit::textChanged, this, &InvoiceWidget::onSearchTextChanged);
    connect(m_statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvoiceWidget::onStatusFilterChanged);
    connect(m_dateFromEdit, &QDateEdit::dateChanged, this, &InvoiceWidget::onDateRangeChanged);
    connect(m_dateToEdit, &QDateEdit::dateChanged, this, &InvoiceWidget::onDateRangeChanged);
    connect(m_clearFiltersBtn, &QPushButton::clicked, [this]() {
        m_invoiceSearchEdit->clear();
        m_statusFilterCombo->setCurrentIndex(0);
        m_dateFromEdit->setDate(QDate::currentDate().addDays(-30));
        m_dateToEdit->setDate(QDate::currentDate());
    });
    
    // Invoice actions
    connect(m_addInvoiceBtn, &QPushButton::clicked, this, &InvoiceWidget::onAddInvoiceClicked);
    connect(m_editInvoiceBtn, &QPushButton::clicked, this, &InvoiceWidget::onEditInvoiceClicked);
    connect(m_deleteInvoiceBtn, &QPushButton::clicked, this, &InvoiceWidget::onDeleteInvoiceClicked);
    connect(m_duplicateInvoiceBtn, &QPushButton::clicked, this, &InvoiceWidget::onDuplicateInvoiceClicked);
    connect(m_viewInvoiceBtn, &QPushButton::clicked, this, &InvoiceWidget::onViewInvoiceClicked);
    connect(m_generatePDFBtn, &QPushButton::clicked, this, &InvoiceWidget::onGeneratePDFClicked);
    connect(m_sendInvoiceBtn, &QPushButton::clicked, this, &InvoiceWidget::onSendInvoiceClicked);
    connect(m_markPaidBtn, &QPushButton::clicked, this, &InvoiceWidget::onMarkAsPaidClicked);
    connect(m_aiAssistantBtn, &QPushButton::clicked, this, &InvoiceWidget::openAIAssistant);
    
    // Client actions
    connect(m_addClientBtn, &QPushButton::clicked, this, &InvoiceWidget::onAddClientClicked);
    connect(m_editClientBtn, &QPushButton::clicked, this, &InvoiceWidget::onEditClientClicked);
    connect(m_deleteClientBtn, &QPushButton::clicked, this, &InvoiceWidget::onDeleteClientClicked);
    
    // Table selections
    connect(m_invoiceTable, &QTableWidget::itemSelectionChanged, this, &InvoiceWidget::onInvoiceSelectionChanged);
    connect(m_clientTable, &QTableWidget::itemSelectionChanged, this, &InvoiceWidget::onClientSelectionChanged);
    
    // Context menus
    connect(m_invoiceTable, &QTableWidget::customContextMenuRequested, this, &InvoiceWidget::showInvoiceContextMenu);
    connect(m_clientTable, &QTableWidget::customContextMenuRequested, this, &InvoiceWidget::showClientContextMenu);
    
    // Search timer
    connect(m_searchTimer, &QTimer::timeout, this, &InvoiceWidget::applyFilters);
    
    // Database signals
    if (m_dbManager) {
        connect(m_dbManager, &InvoiceDatabaseManager::invoiceAdded, this, &InvoiceWidget::onInvoiceAdded);
        connect(m_dbManager, &InvoiceDatabaseManager::invoiceUpdated, this, &InvoiceWidget::onInvoiceUpdated);
        connect(m_dbManager, &InvoiceDatabaseManager::invoiceDeleted, this, &InvoiceWidget::onInvoiceDeleted);
        connect(m_dbManager, &InvoiceDatabaseManager::clientAdded, this, &InvoiceWidget::onClientAdded);
        connect(m_dbManager, &InvoiceDatabaseManager::clientUpdated, this, &InvoiceWidget::onClientUpdated);
        connect(m_dbManager, &InvoiceDatabaseManager::clientDeleted, this, &InvoiceWidget::onClientDeleted);
        connect(m_dbManager, &InvoiceDatabaseManager::errorOccurred, this, &InvoiceWidget::onDatabaseError);
    }
}

QWidget* InvoiceWidget::createStatCard(const QString &title, const QString &value, const QString &subtitle)
{
    QWidget *card = new QWidget();
    card->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 16px;
        }
    )");
    card->setMinimumSize(150, 100);
    
    QVBoxLayout *layout = new QVBoxLayout(card);
    
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 12px; color: #666; font-weight: bold;");
    
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet("font-size: 24px; color: #333; font-weight: bold; margin: 8px 0;");
    valueLabel->setObjectName("valueLabel");
    
    QLabel *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setStyleSheet("font-size: 10px; color: #999;");
    
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    layout->addWidget(subtitleLabel);
    layout->addStretch();
    
    return card;
}

void InvoiceWidget::initializeAIAssistant()
{
    qDebug() << "InvoiceWidget: Initializing AI Assistant...";
    
    // Initialize Groq client
    m_groqClient = new GroqClient(this);
    
    GroqConfig config;
    
    // Try to get API key from environment first
    QString apiKey = EnvironmentLoader::getEnv("GROQ_API_KEY");
    if (!apiKey.isEmpty()) {
        config.apiKey = apiKey;
    }
    config.model = "llama-3.3-70b-versatile";
    config.temperature = 0.7;
    config.maxTokens = 4096;
    
    m_groqClient->setConfiguration(config);
    
    // Set invoice-specific system prompt
    QString systemPrompt = R"(
You are an AI assistant specialized in invoice management and accounting for architecture and construction projects. 
You help users with invoice creation, client management, payment tracking, financial analysis, tax calculations, 
and business process optimization. Provide clear, professional, and accurate assistance with invoice-related tasks.

Key areas of expertise:
- Invoice generation and formatting
- Client relationship management
- Payment terms and collection strategies
- Tax calculations and compliance
- Financial reporting and analysis
- Cash flow management
- Professional communication templates
- Legal considerations for invoicing

Always maintain accuracy in financial calculations and provide practical, actionable advice.
    )";
    
    m_groqClient->setSystemPrompt(systemPrompt.trimmed());
    
    // Initialize AI dialog
    m_aiDialog = new InvoiceAIAssistantDialog(this);
    m_aiDialog->setGroqClient(m_groqClient);
    m_aiDialog->setInvoiceDatabaseManager(m_dbManager);
    
    qDebug() << "InvoiceWidget: AI Assistant initialized";
}

void InvoiceWidget::refreshInvoices()
{
    if (!m_dbManager) return;
    
    qDebug() << "InvoiceWidget: Refreshing invoices...";
    m_isLoading = true;
    
    // Clear existing data
    qDeleteAll(m_invoices);
    m_invoices.clear();
    
    // Load from database
    m_invoices = m_dbManager->getAllInvoices();
    populateInvoiceTable();
    
    m_isLoading = false;
    qDebug() << "InvoiceWidget: Loaded" << m_invoices.size() << "invoices";
}

void InvoiceWidget::refreshClients()
{
    if (!m_dbManager) return;
    
    qDebug() << "InvoiceWidget: Refreshing clients...";
    
    // Clear existing data
    qDeleteAll(m_clients);
    m_clients.clear();
    
    // Load from database
    m_clients = m_dbManager->getAllClients();
    populateClientTable();
    
    qDebug() << "InvoiceWidget: Loaded" << m_clients.size() << "clients";
}

void InvoiceWidget::populateInvoiceTable()
{
    if (m_isLoading) return;
    
    m_invoiceTable->setRowCount(m_invoices.size());
    
    for (int i = 0; i < m_invoices.size(); ++i) {
        addInvoiceToTable(m_invoices[i], i);
    }
}

void InvoiceWidget::populateClientTable()
{
    m_clientTable->setRowCount(m_clients.size());
    
    for (int i = 0; i < m_clients.size(); ++i) {
        addClientToTable(m_clients[i], i);
    }
}

void InvoiceWidget::addInvoiceToTable(const Invoice *invoice, int row)
{
    if (!invoice) return;
    
    if (row == -1) {
        row = m_invoiceTable->rowCount();
        m_invoiceTable->insertRow(row);
    }
    
    m_invoiceTable->setItem(row, InvoiceNumberColumn, new QTableWidgetItem(invoice->invoiceNumber()));
    m_invoiceTable->setItem(row, ClientNameColumn, new QTableWidgetItem(invoice->clientName()));
    m_invoiceTable->setItem(row, InvoiceDateColumn, new QTableWidgetItem(invoice->invoiceDate().toString("yyyy-MM-dd")));
    m_invoiceTable->setItem(row, DueDateColumn, new QTableWidgetItem(invoice->dueDate().toString("yyyy-MM-dd")));
    m_invoiceTable->setItem(row, TotalAmountColumn, new QTableWidgetItem(invoice->formatCurrency(invoice->totalAmount())));
    m_invoiceTable->setItem(row, StatusColumn, new QTableWidgetItem(invoice->statusDisplayText()));
    
    // Store invoice ID in the first column
    m_invoiceTable->item(row, InvoiceNumberColumn)->setData(Qt::UserRole, invoice->id());
}

void InvoiceWidget::addClientToTable(const Client *client, int row)
{
    if (!client) return;
    
    if (row == -1) {
        row = m_clientTable->rowCount();
        m_clientTable->insertRow(row);
    }
    
    m_clientTable->setItem(row, ClientNameCol, new QTableWidgetItem(client->displayName()));
    m_clientTable->setItem(row, ClientEmailCol, new QTableWidgetItem(client->email()));
    m_clientTable->setItem(row, ClientPhoneCol, new QTableWidgetItem(client->phone()));
    m_clientTable->setItem(row, ClientCompanyCol, new QTableWidgetItem(client->company()));
    
    // Store client ID in the first column
    m_clientTable->item(row, ClientNameCol)->setData(Qt::UserRole, client->id());
}

// Basic slot implementations (simplified for now)
void InvoiceWidget::onTabChanged(int index) { m_currentTab = index; }
void InvoiceWidget::onSearchTextChanged() { m_searchTimer->start(); }
void InvoiceWidget::onStatusFilterChanged() { applyFilters(); }
void InvoiceWidget::onDateRangeChanged() { applyFilters(); }
void InvoiceWidget::onInvoiceSelectionChanged() { updateActionStates(); }
void InvoiceWidget::onClientSelectionChanged() { updateActionStates(); }

void InvoiceWidget::updateActionStates()
{
    bool hasInvoiceSelection = m_invoiceTable->currentRow() >= 0;
    bool hasClientSelection = m_clientTable->currentRow() >= 0;
    
    m_editInvoiceBtn->setEnabled(hasInvoiceSelection);
    m_deleteInvoiceBtn->setEnabled(hasInvoiceSelection);
    m_duplicateInvoiceBtn->setEnabled(hasInvoiceSelection);
    m_viewInvoiceBtn->setEnabled(hasInvoiceSelection);
    m_generatePDFBtn->setEnabled(hasInvoiceSelection);
    m_sendInvoiceBtn->setEnabled(hasInvoiceSelection);
    m_markPaidBtn->setEnabled(hasInvoiceSelection);
    
    m_editClientBtn->setEnabled(hasClientSelection);
    m_deleteClientBtn->setEnabled(hasClientSelection);
}

void InvoiceWidget::applyFilters()
{
    // Basic filter implementation
    QString searchTerm = m_invoiceSearchEdit->text().toLower();
    QString statusFilter = m_statusFilterCombo->currentText();
    
    for (int i = 0; i < m_invoiceTable->rowCount(); ++i) {
        bool visible = true;
        
        if (!searchTerm.isEmpty()) {
            QString invoiceNumber = m_invoiceTable->item(i, InvoiceNumberColumn)->text().toLower();
            QString clientName = m_invoiceTable->item(i, ClientNameColumn)->text().toLower();
            visible = invoiceNumber.contains(searchTerm) || clientName.contains(searchTerm);
        }
        
        if (visible && statusFilter != "All Statuses") {
            QString status = m_invoiceTable->item(i, StatusColumn)->text();
            visible = status == statusFilter;
        }
        
        m_invoiceTable->setRowHidden(i, !visible);
    }
}

void InvoiceWidget::updateDashboard()
{
    if (!m_dbManager) return;
    
    // Update statistics
    double totalRevenue = m_dbManager->getTotalRevenue();
    int totalInvoices = m_dbManager->getTotalInvoiceCount();
    int paidInvoices = m_dbManager->getInvoiceCountByStatus("Paid");
    int overdueInvoices = m_dbManager->getOverdueInvoices().size();
    int draftInvoices = m_dbManager->getInvoiceCountByStatus("Draft");
    
    // Calculate pending amount
    double pendingAmount = 0.0;
    QList<Invoice*> unpaidInvoices = m_dbManager->getInvoicesByStatus("Sent");
    for (const Invoice *invoice : unpaidInvoices) {
        pendingAmount += invoice->totalAmount();
    }
    qDeleteAll(unpaidInvoices);
      // Update stat cards
    updateStatCard(m_totalRevenueCard, QString("$%1").arg(totalRevenue, 0, 'f', 2), "Total Revenue");
    updateStatCard(m_totalInvoicesCard, QString::number(totalInvoices), "Total Invoices");
    updateStatCard(m_paidInvoicesCard, QString::number(paidInvoices), "Paid");
    updateStatCard(m_overdueInvoicesCard, QString::number(overdueInvoices), "Overdue");
    updateStatCard(m_draftInvoicesCard, QString::number(draftInvoices), "Draft");
    updateStatCard(m_pendingAmountCard, QString("$%1").arg(pendingAmount, 0, 'f', 2), "Pending Amount");
}

void InvoiceWidget::updateStatCard(QWidget *card, const QString &value, const QString &subtitle)
{
    Q_UNUSED(subtitle)
    
    // Find the value label in the card
    QLabel *valueLabel = card->findChild<QLabel*>("valueLabel");
    if (valueLabel) {
        valueLabel->setText(value);
    }
}

void InvoiceWidget::markAsOverdue()
{
    // Implementation for marking invoices as overdue
    if (!m_dbManager) return;
    
    // Get current date
    QDate currentDate = QDate::currentDate();
    
    // Find all invoices past due date that are not paid
    QList<Invoice*> allInvoices = m_dbManager->getAllInvoices();
    int overdueCount = 0;
    
    for (Invoice *invoice : allInvoices) {
        if (invoice->dueDate() < currentDate && 
            invoice->status() != "Paid" && 
            invoice->status() != "Cancelled" &&
            invoice->status() != "Overdue") {
            
            invoice->setStatus("Overdue");
            m_dbManager->updateInvoice(invoice);
            overdueCount++;
        }
    }
      if (overdueCount > 0) {
        // Refresh the display
        refreshInvoices();
        refreshClients();
        updateDashboard();
        
        // Show notification
        QMessageBox::information(this, tr("Overdue Status Updated"), 
                               tr("Marked %1 invoices as overdue.").arg(overdueCount));
    }
}

// Basic CRUD operations (simplified implementations)
void InvoiceWidget::onAddInvoiceClicked() { addInvoice(); }
void InvoiceWidget::onEditInvoiceClicked() { editInvoice(); }
void InvoiceWidget::onDeleteInvoiceClicked() { deleteInvoice(); }
void InvoiceWidget::onDuplicateInvoiceClicked() { duplicateInvoice(); }
void InvoiceWidget::onViewInvoiceClicked() { viewInvoice(); }
void InvoiceWidget::onGeneratePDFClicked() { generatePDF(); }
void InvoiceWidget::onSendInvoiceClicked() { sendInvoice(); }
void InvoiceWidget::onMarkAsPaidClicked() { markAsPaid(); }

void InvoiceWidget::onAddClientClicked() { addClient(); }
void InvoiceWidget::onEditClientClicked() { editClient(); }
void InvoiceWidget::onDeleteClientClicked() { deleteClient(); }

// Simplified CRUD implementations
void InvoiceWidget::addInvoice()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    InvoiceDialog dialog(this);
    dialog.setDatabaseManager(m_dbManager);
    dialog.setClients(m_clients);
    
    if (dialog.exec() == QDialog::Accepted) {
        Invoice *invoice = dialog.invoice();
        if (invoice && m_dbManager->addInvoice(invoice)) {
            showMessage("Invoice created successfully");
            refreshInvoices();
            updateDashboard();
        } else {
            showMessage("Failed to create invoice: " + m_dbManager->lastError(), true);
        }
    }
}

void InvoiceWidget::editInvoice()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QString invoiceId = getSelectedInvoiceId();
    if (invoiceId.isEmpty()) {
        showMessage("Please select an invoice to edit", true);
        return;
    }
    
    Invoice *invoice = m_dbManager->getInvoice(invoiceId);
    if (!invoice) {
        showMessage("Failed to load invoice: " + m_dbManager->lastError(), true);
        return;
    }
    
    InvoiceDialog dialog(invoice, this);
    dialog.setDatabaseManager(m_dbManager);
    dialog.setClients(m_clients);
    
    if (dialog.exec() == QDialog::Accepted) {
        if (m_dbManager->updateInvoice(invoice)) {
            showMessage("Invoice updated successfully");
            refreshInvoices();
            updateDashboard();
        } else {
            showMessage("Failed to update invoice: " + m_dbManager->lastError(), true);
        }
    }
    
    // Clean up the invoice object
    invoice->deleteLater();
}

void InvoiceWidget::deleteInvoice()
{
    QString invoiceId = getSelectedInvoiceId();
    if (invoiceId.isEmpty()) return;
    
    int ret = QMessageBox::question(this, "Delete Invoice",
                                    "Are you sure you want to delete this invoice?",
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_dbManager->deleteInvoice(invoiceId)) {
            showMessage("Invoice deleted successfully");
            refreshInvoices();
            updateDashboard();
        } else {
            showMessage("Failed to delete invoice: " + m_dbManager->lastError(), true);
        }
    }
}

void InvoiceWidget::duplicateInvoice()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QString invoiceId = getSelectedInvoiceId();
    if (invoiceId.isEmpty()) {
        showMessage("Please select an invoice to duplicate", true);
        return;
    }
    
    Invoice *originalInvoice = m_dbManager->getInvoice(invoiceId);
    if (!originalInvoice) {
        showMessage("Failed to load invoice: " + m_dbManager->lastError(), true);
        return;
    }
    
    // Create a copy of the invoice
    Invoice *duplicatedInvoice = new Invoice(*originalInvoice, this);
    duplicatedInvoice->setId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    duplicatedInvoice->setInvoiceNumber(generateInvoiceNumber());
    duplicatedInvoice->setInvoiceDate(QDate::currentDate());
    duplicatedInvoice->setDueDate(QDate::currentDate().addDays(30));
    duplicatedInvoice->setStatus("Draft");
    
    // Open the dialog with the duplicated invoice
    InvoiceDialog dialog(duplicatedInvoice, this);
    dialog.setDatabaseManager(m_dbManager);
    dialog.setClients(m_clients);
    dialog.setWindowTitle("Duplicate Invoice");
    
    if (dialog.exec() == QDialog::Accepted) {
        if (m_dbManager->addInvoice(duplicatedInvoice)) {
            showMessage("Invoice duplicated successfully");
            refreshInvoices();
            updateDashboard();
        } else {
            showMessage("Failed to duplicate invoice: " + m_dbManager->lastError(), true);
        }
    }
    
    // Clean up
    originalInvoice->deleteLater();
    duplicatedInvoice->deleteLater();
}
void InvoiceWidget::viewInvoice()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QString invoiceId = getSelectedInvoiceId();
    if (invoiceId.isEmpty()) {
        showMessage("Please select an invoice to view", true);
        return;
    }
    
    Invoice *invoice = m_dbManager->getInvoice(invoiceId);
    if (!invoice) {
        showMessage("Failed to load invoice: " + m_dbManager->lastError(), true);
        return;
    }
    
    // Create a read-only dialog
    InvoiceDialog dialog(invoice, this);
    dialog.setDatabaseManager(m_dbManager);
    dialog.setClients(m_clients);
    dialog.setWindowTitle(QString("View Invoice - %1").arg(invoice->invoiceNumber()));
    
    // Make the dialog read-only by disabling the save button
    QPushButton *saveBtn = dialog.findChild<QPushButton*>();
    if (saveBtn) {
        saveBtn->setEnabled(false);
    }
    
    dialog.exec();
    
    // Clean up
    invoice->deleteLater();
}
void InvoiceWidget::generatePDF()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QString invoiceId = getSelectedInvoiceId();
    if (invoiceId.isEmpty()) {
        showMessage("Please select an invoice to generate PDF", true);
        return;
    }
    
    Invoice *invoice = m_dbManager->getInvoice(invoiceId);
    if (!invoice) {
        showMessage("Failed to load invoice: " + m_dbManager->lastError(), true);
        return;
    }
    
    // Ask user for save location
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Invoice PDF"),
        QString("Invoice_%1_%2.pdf")
            .arg(invoice->invoiceNumber())
            .arg(QDate::currentDate().toString("yyyyMMdd")),
        tr("PDF Files (*.pdf)"));
    
    if (!fileName.isEmpty()) {
        if (m_pdfGenerator->generatePDF(invoice, fileName)) {
            showMessage("PDF generated successfully");
            
            // Ask if user wants to open the PDF
            int ret = QMessageBox::question(this, "PDF Generated",
                "PDF generated successfully. Would you like to open it?",
                QMessageBox::Yes | QMessageBox::No);
            
            if (ret == QMessageBox::Yes) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
            }
        } else {
            showMessage("Failed to generate PDF: " + m_pdfGenerator->getLastError(), true);
        }
    }
    
    // Clean up
    invoice->deleteLater();
}
void InvoiceWidget::sendInvoice() { showMessage("Send invoice functionality to be implemented"); }
void InvoiceWidget::markAsPaid()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QString invoiceId = getSelectedInvoiceId();
    if (invoiceId.isEmpty()) {
        showMessage("Please select an invoice to mark as paid", true);
        return;
    }
    
    Invoice *invoice = m_dbManager->getInvoice(invoiceId);
    if (!invoice) {
        showMessage("Failed to load invoice: " + m_dbManager->lastError(), true);
        return;
    }
    
    if (invoice->status() == "Paid") {
        showMessage("Invoice is already marked as paid", false);
        invoice->deleteLater();
        return;
    }
    
    int ret = QMessageBox::question(this, "Mark as Paid",
        QString("Mark invoice %1 as paid?").arg(invoice->invoiceNumber()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        invoice->setStatus("Paid");
        
        if (m_dbManager->updateInvoice(invoice)) {
            showMessage("Invoice marked as paid successfully");
            refreshInvoices();
            updateDashboard();
        } else {
            showMessage("Failed to update invoice: " + m_dbManager->lastError(), true);
        }
    }
    
    // Clean up
    invoice->deleteLater();
}

void InvoiceWidget::addClient()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    ClientDialog dialog(this);
    dialog.setDatabaseManager(m_dbManager);
    
    if (dialog.exec() == QDialog::Accepted) {
        Client *client = dialog.client();
        if (client && m_dbManager->addClient(client)) {
            showMessage("Client created successfully");
            refreshClients();
            updateDashboard();
        } else {
            showMessage("Failed to create client: " + m_dbManager->lastError(), true);
        }
    }
}
void InvoiceWidget::editClient()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QString clientId = getSelectedClientId();
    if (clientId.isEmpty()) {
        showMessage("Please select a client to edit", true);
        return;
    }
    
    Client *client = m_dbManager->getClient(clientId);
    if (!client) {
        showMessage("Failed to load client: " + m_dbManager->lastError(), true);
        return;
    }
    
    ClientDialog dialog(client, this);
    dialog.setDatabaseManager(m_dbManager);
    
    if (dialog.exec() == QDialog::Accepted) {
        if (m_dbManager->updateClient(client)) {
            showMessage("Client updated successfully");
            refreshClients();
            updateDashboard();
        } else {
            showMessage("Failed to update client: " + m_dbManager->lastError(), true);
        }
    }
    
    // Clean up
    client->deleteLater();
}
void InvoiceWidget::deleteClient()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }
    
    QString clientId = getSelectedClientId();
    if (clientId.isEmpty()) {
        showMessage("Please select a client to delete", true);
        return;
    }
    
    // Check if client has any associated invoices
    QList<Invoice*> clientInvoices = m_dbManager->getInvoicesByClient(clientId);
    if (!clientInvoices.isEmpty()) {
        QMessageBox::warning(this, "Cannot Delete Client",
            QString("Cannot delete client because they have %1 associated invoice(s).\n"
                   "Please delete or reassign the invoices first.")
            .arg(clientInvoices.size()));
        
        // Clean up
        qDeleteAll(clientInvoices);
        return;
    }
    
    Client *client = m_dbManager->getClient(clientId);
    if (!client) {
        showMessage("Failed to load client: " + m_dbManager->lastError(), true);
        return;
    }
    
    int ret = QMessageBox::question(this, "Delete Client",
        QString("Are you sure you want to delete client '%1'?").arg(client->name()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_dbManager->deleteClient(clientId)) {
            showMessage("Client deleted successfully");
            refreshClients();
            updateDashboard();
        } else {
            showMessage("Failed to delete client: " + m_dbManager->lastError(), true);
        }
    }
    
    // Clean up
    client->deleteLater();
}

void InvoiceWidget::openAIAssistant()
{
    if (!m_groqClient->isConnected()) {
        showAISetupDialog();
        return;
    }
    
    if (m_aiDialog) {
        // Set current context
        QJsonObject context;
        context["totalInvoices"] = m_invoices.size();
        context["totalClients"] = m_clients.size();
        
        m_aiDialog->setInvoiceContext(context);
        m_aiDialog->show();
        m_aiDialog->raise();
        m_aiDialog->activateWindow();
    }
}

void InvoiceWidget::showAISetupDialog()
{
    QMessageBox::information(this, "AI Assistant Setup",
        "The AI Assistant requires a GROQ API key to function.\n\n"
        "The API key has been pre-configured for this demo.\n"
        "You can set your own API key in the application settings.");
}

QString InvoiceWidget::getSelectedInvoiceId() const
{
    int row = m_invoiceTable->currentRow();
    if (row < 0) return QString();
    
    QTableWidgetItem *item = m_invoiceTable->item(row, InvoiceNumberColumn);
    return item ? item->data(Qt::UserRole).toString() : QString();
}

QString InvoiceWidget::getSelectedClientId() const
{
    int row = m_clientTable->currentRow();
    if (row < 0) return QString();
    
    QTableWidgetItem *item = m_clientTable->item(row, 0); // Assuming first column has the ID
    return item ? item->data(Qt::UserRole).toString() : QString();
}

QString InvoiceWidget::generateInvoiceNumber() const
{
    // Generate a unique invoice number based on current date and time
    return QString("INV-%1-%2")
           .arg(QDate::currentDate().toString("yyyyMMdd"))
           .arg(QTime::currentTime().toString("hhmmss"));
}

void InvoiceWidget::showMessage(const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "Error", message);
    } else {
        QMessageBox::information(this, "Information", message);
    }
}

// Database event handlers (simplified)
void InvoiceWidget::onInvoiceAdded(const QString &invoiceId) { Q_UNUSED(invoiceId); refreshInvoices(); updateDashboard(); }
void InvoiceWidget::onInvoiceUpdated(const QString &invoiceId) { Q_UNUSED(invoiceId); refreshInvoices(); updateDashboard(); }
void InvoiceWidget::onInvoiceDeleted(const QString &invoiceId) { Q_UNUSED(invoiceId); refreshInvoices(); updateDashboard(); }
void InvoiceWidget::onClientAdded(const QString &clientId) { Q_UNUSED(clientId); refreshClients(); }
void InvoiceWidget::onClientUpdated(const QString &clientId) { Q_UNUSED(clientId); refreshClients(); }
void InvoiceWidget::onClientDeleted(const QString &clientId) { Q_UNUSED(clientId); refreshClients(); }
void InvoiceWidget::onDatabaseError(const QString &error) { showMessage("Database Error: " + error, true); }

// Context menu handlers (simplified)
void InvoiceWidget::showInvoiceContextMenu(const QPoint &pos) { Q_UNUSED(pos); }
void InvoiceWidget::showClientContextMenu(const QPoint &pos) { Q_UNUSED(pos); }

// Empty implementations for now
void InvoiceWidget::clearSelection() {}
void InvoiceWidget::showInvoiceStatistics() {}
void InvoiceWidget::exportInvoices() {}
void InvoiceWidget::importInvoices() {}
void InvoiceWidget::updateStatistics() {}
void InvoiceWidget::updateCharts() {}
void InvoiceWidget::updateRecentActivity() {}

void InvoiceWidget::setDatabaseManager(InvoiceDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
    
    if (m_dbManager) {
        qDebug() << "InvoiceWidget: Database manager set successfully";
          // Initialize the database and load data
        m_dbManager->initialize();
        refreshInvoices();
        refreshClients();
        updateDashboard();
    } else {
        qWarning() << "InvoiceWidget: Database manager is null";
    }
}
