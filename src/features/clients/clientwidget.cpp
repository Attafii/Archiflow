#include "clientwidget.h"
#include "client.h"
#include "clientdatabasemanager.h"
#include "clientdialog.h"
#include "clientaiassistant.h"
#include "../../utils/mapboxhandler.h"
#include "../materials/groqclient.h"
#include "../../utils/environmentloader.h"
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

ClientWidget::ClientWidget(QWidget *parent)
    : QWidget(parent)
    , m_dbManager(nullptr)
    , m_mapHandler(nullptr)
    , m_groqClient(nullptr)
    , m_searchTimer(new QTimer(this))
    , m_isLoading(false)
    , m_currentTab(0)
{
    qDebug() << "ClientWidget: Initializing...";
    
    // Initialize database manager
    m_dbManager = new ClientDatabaseManager(this);
    if (!m_dbManager->initialize()) {
        qWarning() << "Failed to initialize client database:" << m_dbManager->lastError();
    }
    
    // Initialize map handler
    m_mapHandler = new MapboxHandler(this);
    
    // Initialize AI client
    m_groqClient = new GroqClient(this);
    
    // Setup search timer
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300);
    
    setupUI();
    setupConnections();
    
    // Load initial data
    refreshClients();
    updateDashboard();
    
    qDebug() << "ClientWidget: Initialization complete";
}

ClientWidget::~ClientWidget()
{
    // Clear existing data
    // qDeleteAll(m_clients); // This was causing issues with QObject parenting
    for (ClientContact* client : m_clients) {
        client->deleteLater(); // Use deleteLater for QObjects
    }
    m_clients.clear();
}

void ClientWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    
    setupDashboardTab();
    setupClientTab();
    setupMapTab();
    
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
            gridline-color: #d0d0d0;
            background-color: white;
            alternate-background-color: #f9f9f9;
        }
        QTableWidget::item:selected {
            background-color: #4a90e2;
            color: white;
        }
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton:pressed {
            background-color: #2968a3;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
    )");
}

void ClientWidget::setupDashboardTab()
{
    m_dashboardTab = new QWidget();
    m_dashboardLayout = new QVBoxLayout(m_dashboardTab);
    
    // Statistics cards
    m_statsContainer = new QWidget();
    m_statsLayout = new QHBoxLayout(m_statsContainer);
    
    m_totalClientsCard = createStatCard("Total Clients", "0", "All registered clients");
    m_activeClientsCard = createStatCard("Active Clients", "0", "Clients with recent activity");
    m_citiesCard = createStatCard("Cities", "0", "Unique cities");
    m_countriesCard = createStatCard("Countries", "0", "Unique countries");
    
    m_statsLayout->addWidget(m_totalClientsCard);
    m_statsLayout->addWidget(m_activeClientsCard);
    m_statsLayout->addWidget(m_citiesCard);
    m_statsLayout->addWidget(m_countriesCard);
    
    m_dashboardLayout->addWidget(m_statsContainer);
    
    // Recent activity
    QGroupBox *activityGroup = new QGroupBox("Recent Activity");
    QVBoxLayout *activityLayout = new QVBoxLayout(activityGroup);
    
    m_recentActivityList = new QListWidget();
    activityLayout->addWidget(m_recentActivityList);
    
    m_dashboardLayout->addWidget(activityGroup);
    
    m_tabWidget->addTab(m_dashboardTab, "Dashboard");
}

void ClientWidget::setupClientTab()
{
    m_clientTab = new QWidget();
    m_clientLayout = new QVBoxLayout(m_clientTab);
    
    // Toolbar
    m_clientToolbar = new QHBoxLayout();
    
    m_addClientBtn = new QPushButton("Add Client");
    m_editClientBtn = new QPushButton("Edit");
    m_deleteClientBtn = new QPushButton("Delete");
    m_viewClientBtn = new QPushButton("View Details");
    m_showOnMapBtn = new QPushButton("Show on Map");
    m_aiInsightBtn = new QPushButton("AI Insight");
    
    m_clientToolbar->addWidget(m_addClientBtn);
    m_clientToolbar->addWidget(m_editClientBtn);
    m_clientToolbar->addWidget(m_deleteClientBtn);
    m_clientToolbar->addWidget(m_viewClientBtn);
    m_clientToolbar->addWidget(m_showOnMapBtn);
    m_clientToolbar->addWidget(m_aiInsightBtn);
    m_clientToolbar->addStretch();
    
    m_exportBtn = new QPushButton("Export");
    m_importBtn = new QPushButton("Import");
    m_statisticsBtn = new QPushButton("Statistics");
    
    m_clientToolbar->addWidget(m_exportBtn);
    m_clientToolbar->addWidget(m_importBtn);
    m_clientToolbar->addWidget(m_statisticsBtn);
    
    m_clientLayout->addLayout(m_clientToolbar);
    
    // Filters
    m_clientFilters = new QHBoxLayout();
    
    m_clientFilters->addWidget(new QLabel("Search:"));
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Search clients...");
    m_clientFilters->addWidget(m_searchEdit);
    
    m_clientFilters->addWidget(new QLabel("City:"));
    m_cityFilterCombo = new QComboBox();
    m_cityFilterCombo->addItem("All Cities", "");
    m_clientFilters->addWidget(m_cityFilterCombo);
    
    m_clientFilters->addWidget(new QLabel("Country:"));
    m_countryFilterCombo = new QComboBox();
    m_countryFilterCombo->addItem("All Countries", "");
    m_clientFilters->addWidget(m_countryFilterCombo);
    
    m_clearFiltersBtn = new QPushButton("Clear Filters");
    m_clientFilters->addWidget(m_clearFiltersBtn);
    
    m_clientFilters->addStretch();
    
    m_clientLayout->addLayout(m_clientFilters);
    
    // Main content area with splitter
    m_clientSplitter = new QSplitter(Qt::Horizontal);
    
    // Client table
    m_clientTable = new QTableWidget();
    m_clientTable->setColumnCount(ClientColumnCount);
    QStringList headers = {"Name", "Company", "Email", "Phone", "City", "Country"};
    m_clientTable->setHorizontalHeaderLabels(headers);
    m_clientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_clientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_clientTable->setAlternatingRowColors(true);
    m_clientTable->setSortingEnabled(true);
    m_clientTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Configure table
    QHeaderView *header = m_clientTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(NameColumn, 150);
    header->resizeSection(CompanyColumn, 150);
    header->resizeSection(EmailColumn, 200);
    header->resizeSection(PhoneColumn, 120);
    header->resizeSection(CityColumn, 100);
    
    m_clientSplitter->addWidget(m_clientTable);
    
    // Client detail panel
    m_clientDetailPanel = new QWidget();
    m_clientDetailLayout = new QVBoxLayout(m_clientDetailPanel);
    
    m_clientDetailTitle = new QLabel("Client Details");
    m_clientDetailTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #4a90e2;");
    m_clientDetailLayout->addWidget(m_clientDetailTitle);
    
    m_clientNameLabel = new QLabel("No client selected");
    m_companyNameLabel = new QLabel();
    m_emailLabel = new QLabel();
    m_phoneLabel = new QLabel();
    m_addressLabel = new QLabel();
    m_addressLabel->setWordWrap(true);
    
    m_clientDetailLayout->addWidget(new QLabel("Name:"));
    m_clientDetailLayout->addWidget(m_clientNameLabel);
    m_clientDetailLayout->addWidget(new QLabel("Company:"));
    m_clientDetailLayout->addWidget(m_companyNameLabel);
    m_clientDetailLayout->addWidget(new QLabel("Email:"));
    m_clientDetailLayout->addWidget(m_emailLabel);
    m_clientDetailLayout->addWidget(new QLabel("Phone:"));
    m_clientDetailLayout->addWidget(m_phoneLabel);
    m_clientDetailLayout->addWidget(new QLabel("Address:"));
    m_clientDetailLayout->addWidget(m_addressLabel);
    
    m_clientDetailLayout->addWidget(new QLabel("Notes:"));
    m_notesDisplay = new QTextEdit();
    m_notesDisplay->setReadOnly(true);
    m_notesDisplay->setMaximumHeight(100);
    m_clientDetailLayout->addWidget(m_notesDisplay);
    
    m_clientDetailLayout->addStretch();
    
    m_clientDetailPanel->setMaximumWidth(300);
    m_clientSplitter->addWidget(m_clientDetailPanel);
    
    m_clientSplitter->setSizes({700, 300});
    m_clientLayout->addWidget(m_clientSplitter);
    
    m_tabWidget->addTab(m_clientTab, "Clients");
}

void ClientWidget::setupMapTab()
{
    m_mapTab = new QWidget();
    m_mapLayout = new QVBoxLayout(m_mapTab);
    
    QSplitter *mapSplitter = new QSplitter(Qt::Horizontal);
    
    // Map container
    m_mapContainer = new QWidget();
    m_mapContainer->setMinimumSize(600, 400);
    m_mapContainer->setStyleSheet("background-color: #e0e0e0; border: 1px solid #c0c0c0;");
    
    QVBoxLayout *mapContainerLayout = new QVBoxLayout(m_mapContainer);
    QLabel *mapPlaceholder = new QLabel("Map View\n(Mapbox Integration)");
    mapPlaceholder->setAlignment(Qt::AlignCenter);
    mapPlaceholder->setStyleSheet("color: #666666; font-size: 16px;");
    mapContainerLayout->addWidget(mapPlaceholder);
    
    mapSplitter->addWidget(m_mapContainer);
    
    // Client list for map
    QWidget *mapSidebar = new QWidget();
    QVBoxLayout *mapSidebarLayout = new QVBoxLayout(mapSidebar);
    
    mapSidebarLayout->addWidget(new QLabel("Clients with Locations:"));
    
    m_mapClientList = new QListWidget();
    mapSidebarLayout->addWidget(m_mapClientList);
    
    mapSidebar->setMaximumWidth(250);
    mapSplitter->addWidget(mapSidebar);
    
    mapSplitter->setSizes({750, 250});
    m_mapLayout->addWidget(mapSplitter);
    
    m_tabWidget->addTab(m_mapTab, "Map");
}

void ClientWidget::setupConnections()
{
    // Tab widget
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &ClientWidget::onTabChanged);
    
    // Search and filters
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ClientWidget::onSearchTextChanged);
    connect(m_cityFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ClientWidget::onCityFilterChanged);
    connect(m_countryFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ClientWidget::onCountryFilterChanged);
    connect(m_clearFiltersBtn, &QPushButton::clicked, [this]() {
        m_searchEdit->clear();
        m_cityFilterCombo->setCurrentIndex(0);
        m_countryFilterCombo->setCurrentIndex(0);
        applyFilters();
    });
    
    // Action buttons
    connect(m_addClientBtn, &QPushButton::clicked, this, &ClientWidget::onAddClientClicked);
    connect(m_editClientBtn, &QPushButton::clicked, this, &ClientWidget::onEditClientClicked);
    connect(m_deleteClientBtn, &QPushButton::clicked, this, &ClientWidget::onDeleteClientClicked);
    connect(m_viewClientBtn, &QPushButton::clicked, this, &ClientWidget::onViewClientClicked);
    connect(m_showOnMapBtn, &QPushButton::clicked, this, &ClientWidget::onShowOnMapClicked);
    connect(m_aiInsightBtn, &QPushButton::clicked, this, &ClientWidget::onGetAIInsightClicked);
    
    connect(m_exportBtn, &QPushButton::clicked, this, &ClientWidget::exportClients);
    connect(m_importBtn, &QPushButton::clicked, this, &ClientWidget::importClients);
    connect(m_statisticsBtn, &QPushButton::clicked, this, &ClientWidget::showStatistics);
    
    // Table
    connect(m_clientTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ClientWidget::onClientSelectionChanged);
    connect(m_clientTable, &QTableWidget::customContextMenuRequested,
            this, &ClientWidget::showClientContextMenu);
    connect(m_clientTable, &QTableWidget::itemDoubleClicked, 
            this, &ClientWidget::onEditClientClicked);
    
    // Search timer
    connect(m_searchTimer, &QTimer::timeout, this, &ClientWidget::applyFilters);
    
    // Database signals
    if (m_dbManager) {
        connect(m_dbManager, &ClientDatabaseManager::clientAdded, 
                this, &ClientWidget::onClientAdded);
        connect(m_dbManager, &ClientDatabaseManager::clientUpdated, 
                this, &ClientWidget::onClientUpdated);
        connect(m_dbManager, &ClientDatabaseManager::clientDeleted, 
                this, &ClientWidget::onClientDeleted);
        connect(m_dbManager, &ClientDatabaseManager::errorOccurred, 
                this, &ClientWidget::onDatabaseError);
    }
    
    // Context menu
    m_clientContextMenu = new QMenu(this);
    m_clientContextMenu->addAction("Edit", this, &ClientWidget::editClient);
    m_clientContextMenu->addAction("Delete", this, &ClientWidget::deleteClient);
    m_clientContextMenu->addSeparator();
    m_clientContextMenu->addAction("View Details", this, &ClientWidget::viewClient);
    m_clientContextMenu->addAction("Show on Map", this, &ClientWidget::showClientOnMap);
    m_clientContextMenu->addAction("Get AI Insight", this, &ClientWidget::getAIInsight);
}

void ClientWidget::refreshClients()
{
    if (!m_dbManager || m_isLoading) return;
    
    m_isLoading = true;
    
    // Clear existing data
    // qDeleteAll(m_clients); // This was causing issues with QObject parenting
    for (ClientContact* client : m_clients) {
        client->deleteLater(); // Use deleteLater for QObjects
    }
    m_clients.clear();
    
    // Load clients from database
    m_clients = m_dbManager->getAllClients(); // This should now return QList<ClientContact*>
    
    // Update UI
    populateClientTable();
    populateFilters();
    updateDashboard();
    updateMapMarkers();
    
    m_isLoading = false;
    
    qDebug() << "ClientWidget: Loaded" << m_clients.size() << "clients";
}

void ClientWidget::clearSelection()
{
    m_clientTable->clearSelection();
    onClientSelectionChanged();
}

void ClientWidget::setDatabaseManager(ClientDatabaseManager *dbManager)
{
    if (m_dbManager != dbManager) {
        m_dbManager = dbManager;
        
        if (m_dbManager) {
            // Reconnect database signals
            connect(m_dbManager, &ClientDatabaseManager::clientAdded, 
                    this, &ClientWidget::onClientAdded);
            connect(m_dbManager, &ClientDatabaseManager::clientUpdated, 
                    this, &ClientWidget::onClientUpdated);
            connect(m_dbManager, &ClientDatabaseManager::clientDeleted, 
                    this, &ClientWidget::onClientDeleted);
            connect(m_dbManager, &ClientDatabaseManager::errorOccurred, 
                    this, &ClientWidget::onDatabaseError);
            
            refreshClients();
        }
    }
}

// CRUD Operations
void ClientWidget::addClient()
{
    onAddClientClicked();
}

void ClientWidget::editClient()
{
    onEditClientClicked();
}

void ClientWidget::deleteClient()
{
    onDeleteClientClicked();
}

void ClientWidget::viewClient()
{
    onViewClientClicked();
}

void ClientWidget::showClientOnMap()
{
    onShowOnMapClicked();
}

void ClientWidget::getAIInsight()
{
    onGetAIInsightClicked();
}

// Event handlers
void ClientWidget::onClientSelectionChanged()
{
    updateActionStates();
    
    QString clientId = getSelectedClientId();
    if (clientId.isEmpty()) {
        m_clientNameLabel->setText("No client selected");
        m_companyNameLabel->clear();
        m_emailLabel->clear();
        m_phoneLabel->clear();
        m_addressLabel->clear();
        m_notesDisplay->clear();
        return;
    }
    
    ClientContact *client = getSelectedClient();
    if (client) {
        m_clientNameLabel->setText(client->name());
        m_companyNameLabel->setText(client->companyName());
        m_emailLabel->setText(client->email());
        m_phoneLabel->setText(client->phoneNumber());
        m_addressLabel->setText(client->fullAddress());
        m_notesDisplay->setPlainText(client->notes());
    }
}

void ClientWidget::onSearchTextChanged()
{
    m_searchTimer->start();
}

void ClientWidget::onCityFilterChanged()
{
    applyFilters();
}

void ClientWidget::onCountryFilterChanged()
{
    applyFilters();
}

void ClientWidget::onTabChanged(int index)
{
    m_currentTab = index;
    if (index == 2) { // Map tab
        updateMapMarkers();
    }
}

// CRUD slot implementations
void ClientWidget::onAddClientClicked()
{
    if (!m_dbManager) {
        showMessage("Database manager not available", true);
        return;
    }    ClientContactDialog dialog(this);
    dialog.setDatabaseManager(m_dbManager);
    
    if (dialog.exec() == QDialog::Accepted) {
        ClientContact *client = dialog.client(); // Assuming dialog returns ClientContact*
        if (client && m_dbManager->addClient(client)) {
            showMessage("Client added successfully");
            refreshClients();
        } else {
            showMessage("Failed to add client: " + m_dbManager->lastError(), true);
        }
    }
}

void ClientWidget::onEditClientClicked()
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
    
    ClientContact *client = m_dbManager->getClient(clientId); // Should return ClientContact*
    if (!client) {
        showMessage("Failed to load client: " + m_dbManager->lastError(), true);        return;
    }
    
    ClientContactDialog dialog(client, this);
    dialog.setDatabaseManager(m_dbManager);
    
    if (dialog.exec() == QDialog::Accepted) {
        if (m_dbManager->updateClient(client)) {
            showMessage("Client updated successfully");
            refreshClients();
        } else {
            showMessage("Failed to update client: " + m_dbManager->lastError(), true);
        }
    }
    
    client->deleteLater();
}

void ClientWidget::onDeleteClientClicked()
{
    QString clientId = getSelectedClientId();
    if (clientId.isEmpty()) return;
    
    ClientContact *client = getSelectedClient();
    if (!client) return;
    
    int ret = QMessageBox::question(this, "Delete Client",
                                    QString("Are you sure you want to delete client '%1'?").arg(client->displayName()),
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_dbManager->deleteClient(clientId)) {
            showMessage("Client deleted successfully");
            refreshClients();
        } else {
            showMessage("Failed to delete client: " + m_dbManager->lastError(), true);
        }
    }
}

void ClientWidget::onViewClientClicked()
{
    QString clientId = getSelectedClientId();
    if (clientId.isEmpty()) {
        showMessage("Please select a client to view", true);
        return;
    }
    
    ClientContact *client = getSelectedClient();
    if (!client) return;
    
    // Create a detailed view dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Client Details - " + client->displayName());
    dialog.resize(400, 500);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    // Create form layout for details
    QGroupBox *detailsGroup = new QGroupBox("Client Information");
    QFormLayout *formLayout = new QFormLayout(detailsGroup);
    
    formLayout->addRow("Name:", new QLabel(client->name()));
    formLayout->addRow("Company:", new QLabel(client->companyName()));
    formLayout->addRow("Email:", new QLabel(client->email()));
    formLayout->addRow("Phone:", new QLabel(client->phoneNumber()));
    formLayout->addRow("Address:", new QLabel(client->fullAddress()));
    
    if (client->hasValidCoordinates()) {
        formLayout->addRow("Coordinates:", 
                          new QLabel(QString("Lat: %1, Lng: %2")
                                   .arg(client->latitude(), 0, 'f', 6)
                                   .arg(client->longitude(), 0, 'f', 6)));
    }
    
    formLayout->addRow("Created:", new QLabel(client->createdAt().toString("yyyy-MM-dd hh:mm:ss")));
    formLayout->addRow("Updated:", new QLabel(client->updatedAt().toString("yyyy-MM-dd hh:mm:ss")));
    
    layout->addWidget(detailsGroup);
    
    // Notes
    if (!client->notes().isEmpty()) {
        QGroupBox *notesGroup = new QGroupBox("Notes");
        QVBoxLayout *notesLayout = new QVBoxLayout(notesGroup);
        QTextEdit *notesEdit = new QTextEdit();
        notesEdit->setPlainText(client->notes());
        notesEdit->setReadOnly(true);
        notesLayout->addWidget(notesEdit);
        layout->addWidget(notesGroup);
    }
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    layout->addLayout(buttonLayout);
    
    dialog.exec();
}

void ClientWidget::onShowOnMapClicked()
{
    QString clientId = getSelectedClientId();
    if (clientId.isEmpty()) {
        showMessage("Please select a client to view on map", true);
        return;
    }
    
    ClientContact *client = getSelectedClient();
    if (!client) return;
    
    if (!client->hasValidCoordinates()) {
        showMessage("Client does not have valid coordinates", true);
        return;
    }
    
    // Switch to map tab
    m_tabWidget->setCurrentIndex(2);
    
    // Show client on map (implementation depends on MapboxHandler)
    if (m_mapHandler) {
        m_mapHandler->showLocation(client->latitude(), client->longitude(), client->displayName());
    }
    
    showMessage(QString("Showing %1 on map").arg(client->displayName()));
}

void ClientWidget::onGetAIInsightClicked()
{
    QString clientId = getSelectedClientId();
    if (clientId.isEmpty()) {
        showMessage("Please select a client to get AI insight", true);
        return;
    }
    
    ClientContact *client = getSelectedClient();
    if (!client) return;
    
    // Open AI Assistant with the selected client
    ClientAIAssistant *aiAssistant = new ClientAIAssistant(client, this);
    aiAssistant->show();
    aiAssistant->raise();
    aiAssistant->activateWindow();
}

// Context menu
void ClientWidget::showClientContextMenu(const QPoint &pos)
{
    if (m_clientTable->itemAt(pos)) {
        m_clientContextMenu->exec(m_clientTable->mapToGlobal(pos));
    }
}

// Database event handlers
void ClientWidget::onClientAdded(const QString &clientId)
{
    Q_UNUSED(clientId)
    updateRecentActivity();
}

void ClientWidget::onClientUpdated(const QString &clientId)
{
    Q_UNUSED(clientId)
    updateRecentActivity();
}

void ClientWidget::onClientDeleted(const QString &clientId)
{
    Q_UNUSED(clientId)
    updateRecentActivity();
}

void ClientWidget::onDatabaseError(const QString &error)
{
    showMessage("Database error: " + error, true);
}

// Helper methods
void ClientWidget::populateClientTable()
{
    m_clientTable->setRowCount(0);
    m_filteredClients = m_clients;
    
    for (int i = 0; i < m_filteredClients.size(); ++i) {
        addClientToTable(m_filteredClients[i], i);
    }
    
    updateActionStates();
}

void ClientWidget::populateFilters()
{
    // Populate city filter
    m_cityFilterCombo->clear();
    m_cityFilterCombo->addItem("All Cities", "");
    
    QStringList cities = m_dbManager->getAllCities();
    for (const QString &city : cities) {
        m_cityFilterCombo->addItem(city, city);
    }
    
    // Populate country filter
    m_countryFilterCombo->clear();
    m_countryFilterCombo->addItem("All Countries", "");
    
    QStringList countries = m_dbManager->getAllCountries();
    for (const QString &country : countries) {
        m_countryFilterCombo->addItem(country, country);
    }
}

void ClientWidget::applyFilters()
{
    if (m_isLoading) return;
    
    m_currentSearchTerm = m_searchEdit->text().trimmed();
    m_currentCityFilter = m_cityFilterCombo->currentData().toString();
    m_currentCountryFilter = m_countryFilterCombo->currentData().toString();
    
    m_filteredClients.clear();
    
    for (ClientContact *client : m_clients) {
        bool matches = true;
        
        // Search term filter
        if (!m_currentSearchTerm.isEmpty()) {
            bool searchMatch = client->name().contains(m_currentSearchTerm, Qt::CaseInsensitive) ||
                             client->companyName().contains(m_currentSearchTerm, Qt::CaseInsensitive) ||
                             client->email().contains(m_currentSearchTerm, Qt::CaseInsensitive);
            if (!searchMatch) matches = false;
        }
        
        // City filter
        if (!m_currentCityFilter.isEmpty()) {
            if (client->addressCity() != m_currentCityFilter) matches = false;
        }
        
        // Country filter
        if (!m_currentCountryFilter.isEmpty()) {
            if (client->addressCountry() != m_currentCountryFilter) matches = false;
        }
        
        if (matches) {
            m_filteredClients.append(client);
        }
    }
    
    // Update table
    m_clientTable->setRowCount(0);
    for (int i = 0; i < m_filteredClients.size(); ++i) {
        addClientToTable(m_filteredClients[i], i);
    }
    
    updateActionStates();
}

void ClientWidget::updateActionStates()
{
    bool hasSelection = !getSelectedClientId().isEmpty();
    
    m_editClientBtn->setEnabled(hasSelection);
    m_deleteClientBtn->setEnabled(hasSelection);
    m_viewClientBtn->setEnabled(hasSelection);
    m_showOnMapBtn->setEnabled(hasSelection);
    m_aiInsightBtn->setEnabled(hasSelection);
}

void ClientWidget::showMessage(const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "Client Management", message);
    } else {
        QMessageBox::information(this, "Client Management", message);
    }
}

QString ClientWidget::getSelectedClientId() const
{
    int row = m_clientTable->currentRow();
    if (row >= 0 && row < m_filteredClients.size()) {
        return m_filteredClients[row]->id();
    }
    return QString();
}

ClientContact* ClientWidget::getSelectedClient() const
{
    int row = m_clientTable->currentRow();
    if (row >= 0 && row < m_filteredClients.size()) {
        return m_filteredClients[row];
    }
    return nullptr;
}

void ClientWidget::addClientToTable(const ClientContact *client, int row)
{
    if (row == -1) {
        row = m_clientTable->rowCount();
        m_clientTable->insertRow(row);
    }
    
    m_clientTable->setItem(row, NameColumn, new QTableWidgetItem(client->name()));
    m_clientTable->setItem(row, CompanyColumn, new QTableWidgetItem(client->companyName()));
    m_clientTable->setItem(row, EmailColumn, new QTableWidgetItem(client->email()));
    m_clientTable->setItem(row, PhoneColumn, new QTableWidgetItem(client->phoneNumber()));
    m_clientTable->setItem(row, CityColumn, new QTableWidgetItem(client->addressCity()));
    m_clientTable->setItem(row, CountryColumn, new QTableWidgetItem(client->addressCountry()));
}

void ClientWidget::updateClientInTable(const ClientContact *client, int row)
{
    if (row >= 0 && row < m_clientTable->rowCount()) {
        m_clientTable->item(row, NameColumn)->setText(client->name());
        m_clientTable->item(row, CompanyColumn)->setText(client->companyName());
        m_clientTable->item(row, EmailColumn)->setText(client->email());
        m_clientTable->item(row, PhoneColumn)->setText(client->phoneNumber());
        m_clientTable->item(row, CityColumn)->setText(client->addressCity());
        m_clientTable->item(row, CountryColumn)->setText(client->addressCountry());
    }
}

void ClientWidget::removeClientFromTable(int row)
{
    if (row >= 0 && row < m_clientTable->rowCount()) {
        m_clientTable->removeRow(row);
    }
}

QWidget* ClientWidget::createStatCard(const QString &title, const QString &value, const QString &subtitle)
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
    card->setFixedHeight(120);
    
    QVBoxLayout *layout = new QVBoxLayout(card);
    
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #666666; font-size: 12px;");
    layout->addWidget(titleLabel);
    
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet("color: #4a90e2; font-size: 24px; font-weight: bold;");
    valueLabel->setObjectName("valueLabel");
    layout->addWidget(valueLabel);
    
    if (!subtitle.isEmpty()) {
        QLabel *subtitleLabel = new QLabel(subtitle);
        subtitleLabel->setStyleSheet("color: #999999; font-size: 10px;");
        subtitleLabel->setObjectName("subtitleLabel");
        layout->addWidget(subtitleLabel);
    }
    
    layout->addStretch();
    
    return card;
}

void ClientWidget::updateStatCard(QWidget *card, const QString &value, const QString &subtitle)
{
    QLabel *valueLabel = card->findChild<QLabel*>("valueLabel");
    if (valueLabel) {
        valueLabel->setText(value);
    }
    
    if (!subtitle.isEmpty()) {
        QLabel *subtitleLabel = card->findChild<QLabel*>("subtitleLabel");
        if (subtitleLabel) {
            subtitleLabel->setText(subtitle);
        }
    }
}

void ClientWidget::updateDashboard()
{
    if (!m_dbManager) return;
    
    int totalClients = m_clients.size();
    int activeClients = totalClients; // Could be refined based on recent activity
    int uniqueCities = m_dbManager->getAllCities().size();
    int uniqueCountries = m_dbManager->getAllCountries().size();
    
    updateStatCard(m_totalClientsCard, QString::number(totalClients));
    updateStatCard(m_activeClientsCard, QString::number(activeClients));
    updateStatCard(m_citiesCard, QString::number(uniqueCities));
    updateStatCard(m_countriesCard, QString::number(uniqueCountries));
}

void ClientWidget::updateStatistics()
{
    updateDashboard();
}

void ClientWidget::updateRecentActivity()
{
    m_recentActivityList->clear();
    
    // Add recent activity items
    QTime currentTime = QTime::currentTime();
    m_recentActivityList->addItem(QString("[%1] Client data refreshed").arg(currentTime.toString("hh:mm:ss")));
}

void ClientWidget::initializeMap()
{
    // Initialize map integration
    if (m_mapHandler) {
        // Setup map with API key from environment
    }
}

void ClientWidget::updateMapMarkers()
{
    if (!m_mapHandler) return;
    
    // Clear existing markers
    m_mapClientList->clear();
    
    // Add clients with coordinates to map
    for (ClientContact *client : m_clients) {
        if (client->hasValidCoordinates()) {
            QString itemText = QString("%1 (%2)")
                             .arg(client->displayName())
                             .arg(client->addressCity());
            m_mapClientList->addItem(itemText);
              // Add marker to map
            m_mapHandler->addMarker(client->latitude(), client->longitude(), 
                                  client->displayName());
        }
    }
}

// Export/Import/Statistics placeholder implementations
void ClientWidget::exportClients()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Clients", 
                                                  "clients.json", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        // Export implementation would go here
        showMessage("Export functionality to be implemented");
    }
}

void ClientWidget::importClients()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Clients", 
                                                  "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        // Import implementation would go here
        showMessage("Import functionality to be implemented");
    }
}

void ClientWidget::showStatistics()
{
    // Show detailed statistics dialog
    showMessage("Statistics dialog to be implemented");
}
