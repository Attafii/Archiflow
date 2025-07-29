#include "materialwidget.h"
#include "materialmodel.h"
#include "materialdialog.h"
#include "materialdetailsdialog.h"
#include "supplierwidget.h"
#include "groqclient.h"
#include "aiassistantdialog.h"
#include "aipredictiondialog.h"
#include "utils/stylemanager.h"
#include "utils/animationmanager.h"
#include "utils/environmentloader.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QUuid>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QRegularExpression>
#include <QPalette>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSettings>
#include <QGroupBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QScrollArea>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegend>

MaterialWidget::MaterialWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_tabWidget(nullptr)
    , m_materialsTab(nullptr)
    , m_suppliersTab(nullptr)
    , m_model(nullptr)
    , m_proxyModel(nullptr)
    , m_groqClient(nullptr)
    , m_aiDialog(nullptr)    , m_aiPredictionDialog(nullptr)
    , m_supplierWidget(nullptr)
    , m_rightPanel(nullptr)
    , m_togglePanelButton(nullptr)
    , m_rightPanelAnimation(nullptr)
    , m_rightPanelVisible(true)
{
    qDebug() << "MaterialWidget constructor starting...";
    
    // Force ArchiFlow theme on this widget
    setStyleSheet("MaterialWidget { background-color: #3D485A; color: #E3C6B0; }");
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    
    QPalette archiFlowPalette = palette();
    archiFlowPalette.setColor(QPalette::Window, QColor(61, 72, 90)); // #3D485A
    archiFlowPalette.setColor(QPalette::WindowText, QColor(227, 198, 176)); // #E3C6B0
    archiFlowPalette.setColor(QPalette::Base, QColor(42, 51, 64)); // #2A3340
    archiFlowPalette.setColor(QPalette::Text, QColor(227, 198, 176)); // #E3C6B0
    setPalette(archiFlowPalette);
    
    qDebug() << "About to call setupUI...";
    setupUI();
    qDebug() << "setupUI completed";
    
    qDebug() << "About to call setupConnections...";
    setupConnections();
    qDebug() << "setupConnections completed";
    
    qDebug() << "About to initialize AI Assistant...";
    // Initialize AI Assistant
    initializeAIAssistant();
    qDebug() << "AI Assistant initialized";
      
    qDebug() << "About to create MaterialModel...";
    // Initialize model
    m_model = new MaterialModel(this);
    qDebug() << "MaterialModel created, setting proxy source...";
    m_proxyModel->setSourceModel(m_model);
    qDebug() << "Proxy model source set";
    
    qDebug() << "Setting up table selection connections...";
    // Set up table selection connections after model is set
    if (m_tableView && m_tableView->selectionModel()) {
        connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MaterialWidget::onTableSelectionChanged);
        qDebug() << "Table selection connection established";
    } else {
        qDebug() << "Warning: m_tableView or selectionModel is null!";
    }
      
    qDebug() << "Setting up delayed refresh...";
    // Initial data load
    QTimer::singleShot(0, this, &MaterialWidget::refreshData);
    qDebug() << "Delayed refresh scheduled";
    
    qDebug() << "Initializing button states...";
    // Initialize button states
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_viewButton->setEnabled(false);
    m_detailSaveBtn->setEnabled(false);
    m_detailDeleteBtn->setEnabled(false);
    qDebug() << "Button states initialized";
    
    qDebug() << "Applying styling...";
    // Apply modern styling
    StyleManager::applyMaterialDesign(this);
    qDebug() << "MaterialWidget constructor completed successfully";
}

void MaterialWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Setup top toolbar
    setupActions();
    
    // Create main content area with three panels
    QWidget *mainContentWidget = new QWidget();
    QHBoxLayout *mainContentLayout = new QHBoxLayout(mainContentWidget);
    mainContentLayout->setContentsMargins(0, 0, 0, 0);
    mainContentLayout->setSpacing(0);
    
    // Left sidebar for materials navigation
    QWidget *leftSidebar = new QWidget();
    leftSidebar->setFixedWidth(200);
    leftSidebar->setObjectName("materialsSidebar");
    
    QVBoxLayout *sidebarLayout = new QVBoxLayout(leftSidebar);
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
    sidebarLayout->setSpacing(5);
    
    // Add Materials header
    QLabel *materialsHeader = new QLabel("Materials");
    materialsHeader->setObjectName("materialsHeader");
    materialsHeader->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; padding: 10px;");    // Add navigation buttons for materials sections
    m_materialsBtn = new QPushButton("Materials");
    m_dashboardBtn = new QPushButton("Dashboard");
    m_suppliersBtn = new QPushButton("Suppliers");
    m_reportsBtn = new QPushButton("Reports");
    m_settingsBtn = new QPushButton("Settings");
    
    // Style the sidebar buttons
    QString sidebarButtonStyle = "QPushButton { text-align: left; padding: 10px; border: none; background-color: transparent; color: #666; } "
                                 "QPushButton:hover { background-color: #f0f0f0; } "
                                 "QPushButton:checked { background-color: #3498db; color: white; }";
      m_materialsBtn->setStyleSheet(sidebarButtonStyle);
    m_dashboardBtn->setStyleSheet(sidebarButtonStyle);
    m_suppliersBtn->setStyleSheet(sidebarButtonStyle);
    m_reportsBtn->setStyleSheet(sidebarButtonStyle);
    m_settingsBtn->setStyleSheet(sidebarButtonStyle);
      m_materialsBtn->setCheckable(true);
    m_dashboardBtn->setCheckable(true);
    m_suppliersBtn->setCheckable(true);
    m_reportsBtn->setCheckable(true);
    m_settingsBtn->setCheckable(true);
    m_materialsBtn->setChecked(true); // Default selection
      sidebarLayout->addWidget(materialsHeader);
    sidebarLayout->addWidget(m_materialsBtn);
    sidebarLayout->addWidget(m_dashboardBtn);
    sidebarLayout->addWidget(m_suppliersBtn);
    sidebarLayout->addWidget(m_reportsBtn);
    sidebarLayout->addWidget(m_settingsBtn);
    sidebarLayout->addStretch();
      // Center content area
    m_centerContent = new QWidget();
    QVBoxLayout *centerLayout = new QVBoxLayout(m_centerContent);
    centerLayout->setContentsMargins(10, 10, 10, 10);
    centerLayout->setSpacing(10);
    
    // Create dashboard widget
    setupDashboard();
    
    // Create materials list widget
    m_materialsListWidget = new QWidget();
    QVBoxLayout *materialsLayout = new QVBoxLayout(m_materialsListWidget);
    materialsLayout->setContentsMargins(0, 0, 0, 0);
    materialsLayout->setSpacing(10);
    
    // Setup filters and table for materials list
    setupFilters();
    setupTable();
      materialsLayout->addWidget(m_filtersWidget);
    materialsLayout->addWidget(m_tableWidget);    // Create suppliers widget
    m_supplierWidget = new SupplierWidget();
    
    // Create reports widget
    setupReportsWidget();
    
    // Create settings widget
    setupSettingsWidget();
    
    // Add all widgets to center content (initially show materials list)
    centerLayout->addWidget(m_dashboardWidget);
    centerLayout->addWidget(m_materialsListWidget);
    centerLayout->addWidget(m_supplierWidget);
    centerLayout->addWidget(m_reportsWidget);
    centerLayout->addWidget(m_settingsWidget);
    
    // Initially show materials list, hide others
    m_dashboardWidget->hide();
    m_materialsListWidget->show();
    m_supplierWidget->hide();
    m_reportsWidget->hide();
    m_settingsWidget->hide();      // Right details panel    
    m_rightPanel = new QWidget();
    m_rightPanel->setMinimumWidth(0);
    m_rightPanel->setMaximumWidth(300);
    m_rightPanel->setObjectName("materialDetailsPanel");
    
    QVBoxLayout *rightLayout = new QVBoxLayout(m_rightPanel);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    rightLayout->setSpacing(10);
    
    // Material Details header with toggle button
    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(5);
    
    QLabel *detailsHeader = new QLabel("Material Details");
    detailsHeader->setStyleSheet("font-size: 14px; font-weight: bold; color: #333; padding: 5px;");
      m_togglePanelButton = new QPushButton("⮞");
    m_togglePanelButton->setFixedSize(24, 24);
    m_togglePanelButton->setStyleSheet(
        "QPushButton { "
        "border: 1px solid #ccc; "
        "border-radius: 12px; "
        "background-color: #f8f9fa; "
        "font-weight: bold; "
        "color: #495057; "
        "} "
        "QPushButton:hover { "
        "background-color: #e9ecef; "
        "} "
        "QPushButton:pressed { "
        "background-color: #dee2e6; "
        "}"
    );
    m_togglePanelButton->setToolTip("Show Material Details Panel");
    
    headerLayout->addWidget(detailsHeader);
    headerLayout->addStretch();
    headerLayout->addWidget(m_togglePanelButton);
      rightLayout->addWidget(headerWidget);
    
    // Add form fields for material details
    setupMaterialDetailsForm(m_rightPanel, rightLayout);
      
    // Setup animation for right panel
    m_rightPanelAnimation = new QPropertyAnimation(m_rightPanel, "maximumWidth", this);
    m_rightPanelAnimation->setDuration(300);
    m_rightPanelAnimation->setEasingCurve(QEasingCurve::InOutQuad);    
    // Add panels to main content layout
    mainContentLayout->addWidget(leftSidebar);
    mainContentLayout->addWidget(m_centerContent, 1); // Give center content more space
    mainContentLayout->addWidget(m_rightPanel);
      // Initially hide right panel - it will be shown only in Materials section
    m_rightPanelVisible = false;
    m_rightPanel->setMaximumWidth(0);
    m_rightPanel->hide();
    
    // Add everything to main layout
    m_mainLayout->addWidget(m_actionsWidget);
    m_mainLayout->addWidget(mainContentWidget, 1);
}



void MaterialWidget::setupFilters()
{
    m_filtersWidget = new QWidget(this);
    m_filtersWidget->setObjectName("filtersWidget");
    
    QVBoxLayout *mainFiltersLayout = new QVBoxLayout(m_filtersWidget);
    mainFiltersLayout->setContentsMargins(10, 10, 10, 10);
    mainFiltersLayout->setSpacing(10);
    
    // Search and Filter header
    QLabel *filterHeader = new QLabel("Search and Filter");
    filterHeader->setStyleSheet("font-size: 14px; font-weight: bold; color: #333; padding: 5px;");
    
    // Search and filter controls layout
    QHBoxLayout *filterControlsLayout = new QHBoxLayout();
    filterControlsLayout->setSpacing(15);
    
    // Search field
    QLabel *searchLabel = new QLabel("Search:");
    searchLabel->setStyleSheet("font-weight: 500; color: #495057;");
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Enter search term...");
    m_searchEdit->setMinimumWidth(200);
    m_searchEdit->setObjectName("searchEdit");
    
    // Category filter
    QLabel *categoryLabel = new QLabel("Category:");
    categoryLabel->setStyleSheet("font-weight: 500; color: #495057;");
    
    m_categoryFilter = new QComboBox();
    m_categoryFilter->addItem("All Categories");
    m_categoryFilter->addItems({"Construction", "Plumbing", "Metal", "Other"});
    m_categoryFilter->setMinimumWidth(150);
    m_categoryFilter->setObjectName("categoryFilter");
    
    // Clear filters button
    m_clearFiltersButton = new QPushButton("Clear Filters");
    m_clearFiltersButton->setObjectName("clearFiltersButton");
    
    // Add to controls layout
    filterControlsLayout->addWidget(searchLabel);
    filterControlsLayout->addWidget(m_searchEdit);
    filterControlsLayout->addWidget(categoryLabel);
    filterControlsLayout->addWidget(m_categoryFilter);
    filterControlsLayout->addWidget(m_clearFiltersButton);
    filterControlsLayout->addStretch();
    
    // Add to main layout
    mainFiltersLayout->addWidget(filterHeader);
    mainFiltersLayout->addLayout(filterControlsLayout);
}

void MaterialWidget::setupTable()
{
    m_tableWidget = new QWidget(this);
    m_tableLayout = new QVBoxLayout(m_tableWidget);
    m_tableLayout->setContentsMargins(0, 0, 0, 0);
    
    // Table view
    m_tableView = new QTableView();
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSortingEnabled(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->setShowGrid(false);
    
    // Setup proxy model for filtering and sorting
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1); // Filter all columns
    
    m_tableView->setModel(m_proxyModel);
    
    // Configure headers
    QHeaderView *header = m_tableView->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(QHeaderView::Interactive);
    
    m_tableLayout->addWidget(m_tableView);
    
    // Apply styling
    StyleManager::addElevation(m_tableView, 1);
}

void MaterialWidget::setupActions()
{
    m_actionsWidget = new QWidget(this);
    m_actionsLayout = new QHBoxLayout(m_actionsWidget);
    m_actionsLayout->setSpacing(10);
    m_actionsLayout->setContentsMargins(0, 0, 0, 0);
    
    // CRUD buttons section
    m_addButton = new QPushButton("+ Add");
    m_editButton = new QPushButton("✏ Edit");
    m_deleteButton = new QPushButton("🗑 Delete");
    m_viewButton = new QPushButton("👁 View");
    
    // Style CRUD buttons
    m_addButton->setObjectName("addButton");
    m_editButton->setObjectName("editButton");
    m_deleteButton->setObjectName("deleteButton");
    m_viewButton->setObjectName("viewButton");
    
    QSize crudButtonSize(80, 40);
    m_addButton->setFixedSize(crudButtonSize);
    m_editButton->setFixedSize(crudButtonSize);
    m_deleteButton->setFixedSize(crudButtonSize);
    m_viewButton->setFixedSize(crudButtonSize);
    
    // Top toolbar buttons to match the design
    m_importButton = new QPushButton("Import");
    m_exportButton = new QPushButton("Export");
    m_resetDbButton = new QPushButton("Reset DB");
    m_scanButton = new QPushButton("Scan");
    m_aiAssistantButton = new QPushButton("AI Assistant");
    m_aiPredictionsButton = new QPushButton("AI Predictions");
    
    // Style the buttons to match the design
    m_importButton->setObjectName("toolbarButton");
    m_exportButton->setObjectName("toolbarButton");
    m_resetDbButton->setObjectName("resetDbButton");
    m_scanButton->setObjectName("toolbarButton");
    m_aiAssistantButton->setObjectName("toolbarButton");
    m_aiPredictionsButton->setObjectName("aiPredictionsButton");
    
    // Set fixed sizes for consistent appearance
    QSize buttonSize(100, 40);
    m_importButton->setFixedSize(buttonSize);
    m_exportButton->setFixedSize(buttonSize);
    m_resetDbButton->setFixedSize(buttonSize);
    m_scanButton->setFixedSize(buttonSize);
    m_aiAssistantButton->setFixedSize(buttonSize);
    m_aiPredictionsButton->setFixedSize(buttonSize);
    
    // Add CRUD buttons first
    m_actionsLayout->addWidget(m_addButton);
    m_actionsLayout->addWidget(m_editButton);
    m_actionsLayout->addWidget(m_deleteButton);
    m_actionsLayout->addWidget(m_viewButton);
    m_actionsLayout->addSpacing(20); // Separator
    
    // Add toolbar buttons
    m_actionsLayout->addWidget(m_importButton);
    m_actionsLayout->addWidget(m_exportButton);
    m_actionsLayout->addWidget(m_resetDbButton);
    m_actionsLayout->addWidget(m_scanButton);
    m_actionsLayout->addWidget(m_aiAssistantButton);
    m_actionsLayout->addWidget(m_aiPredictionsButton);
    m_actionsLayout->addStretch();
}

void MaterialWidget::setupConnections()
{
    qDebug() << "setupConnections starting...";
    
    // Navigation button connections
    if (m_dashboardBtn) {
        connect(m_dashboardBtn, &QPushButton::clicked, this, &MaterialWidget::showDashboard);
    }
    if (m_materialsBtn) {
        connect(m_materialsBtn, &QPushButton::clicked, this, &MaterialWidget::showMaterialsList);
    }
    if (m_suppliersBtn) {
        connect(m_suppliersBtn, &QPushButton::clicked, this, &MaterialWidget::showSuppliers);
    }
    if (m_reportsBtn) {
        connect(m_reportsBtn, &QPushButton::clicked, this, &MaterialWidget::showReports);
    }
    if (m_settingsBtn) {
        connect(m_settingsBtn, &QPushButton::clicked, this, &MaterialWidget::showSettings);
    }
    
    qDebug() << "Navigation buttons connected";
    
    // Filter connections
    if (m_searchEdit) {
        connect(m_searchEdit, &QLineEdit::textChanged,
                this, &MaterialWidget::onSearchTextChanged);
    }
    if (m_categoryFilter) {
        connect(m_categoryFilter, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
                this, &MaterialWidget::onCategoryFilterChanged);
    }
    if (m_clearFiltersButton) {
        connect(m_clearFiltersButton, &QPushButton::clicked, [this]() {
            if (m_searchEdit) m_searchEdit->clear();
            if (m_categoryFilter) m_categoryFilter->setCurrentIndex(0);
        });
    }
    
    qDebug() << "Filter connections set";
    
    // CRUD button connections
    if (m_addButton) {
        connect(m_addButton, &QPushButton::clicked, this, &MaterialWidget::addMaterial);
    }
    if (m_editButton) {
        connect(m_editButton, &QPushButton::clicked, this, &MaterialWidget::editMaterial);
    }
    if (m_deleteButton) {
        connect(m_deleteButton, &QPushButton::clicked, this, &MaterialWidget::deleteMaterial);
    }
    if (m_viewButton) {
        connect(m_viewButton, &QPushButton::clicked, this, &MaterialWidget::showMaterialDetails);
    }
    
    qDebug() << "CRUD buttons connected";
    
    // Detail panel button connections
    if (m_detailNewBtn) {
        connect(m_detailNewBtn, &QPushButton::clicked, this, &MaterialWidget::addMaterial);
    }
    if (m_detailSaveBtn) {
        connect(m_detailSaveBtn, &QPushButton::clicked, this, &MaterialWidget::saveMaterialFromDetail);
    }
    if (m_detailDeleteBtn) {
        connect(m_detailDeleteBtn, &QPushButton::clicked, this, &MaterialWidget::deleteMaterial);
    }
    
    qDebug() << "Detail panel buttons connected";
    
    // Toggle panel button connection
    if (m_togglePanelButton) {
        connect(m_togglePanelButton, &QPushButton::clicked, this, &MaterialWidget::toggleRightPanel);
    }
    
    qDebug() << "Toggle panel button connected";
    
    // Toolbar button connections
    if (m_importButton) {
        connect(m_importButton, &QPushButton::clicked, this, &MaterialWidget::importFromCSV);
    }
    if (m_exportButton) {
        connect(m_exportButton, &QPushButton::clicked, this, &MaterialWidget::exportToCSV);
    }
    if (m_resetDbButton) {
        connect(m_resetDbButton, &QPushButton::clicked, [this]() {
            QMessageBox::information(this, "Reset DB", "Database reset functionality not yet implemented.");
        });
    }
    if (m_scanButton) {
        connect(m_scanButton, &QPushButton::clicked, [this]() {
            QMessageBox::information(this, "Scan", "Scan functionality not yet implemented.");
        });
    }
    if (m_aiAssistantButton) {
        connect(m_aiAssistantButton, &QPushButton::clicked, this, &MaterialWidget::openAIAssistant);
    }
    if (m_aiPredictionsButton) {
        connect(m_aiPredictionsButton, &QPushButton::clicked, this, &MaterialWidget::openAIPrediction);
    }
    
    qDebug() << "Toolbar buttons connected";
    
    // Table double-click connection
    if (m_tableView) {
        connect(m_tableView, &QTableView::doubleClicked,
                this, &MaterialWidget::showMaterialDetails);
    }
    
    qDebug() << "setupConnections completed successfully";
}



void MaterialWidget::refreshData()
{
    qDebug() << "MaterialWidget::refreshData() starting...";
    if (m_model) {
        qDebug() << "Calling m_model->refresh()...";
        m_model->refresh();
        qDebug() << "Model refresh completed";
        
        // Update category filter
        qDebug() << "Updating category filter...";
        QStringList categories = m_model->getCategories();
        m_categoryFilter->clear();
        m_categoryFilter->addItem("All Categories");
        m_categoryFilter->addItems(categories);
        qDebug() << "Category filter updated";
        
        // Update dashboard statistics when data is refreshed
        qDebug() << "Updating dashboard stats...";
        updateDashboardStats();
        qDebug() << "Dashboard stats updated";
    } else {
        qDebug() << "Warning: m_model is null in refreshData()";
    }
    qDebug() << "MaterialWidget::refreshData() completed";
}

void MaterialWidget::selectMaterial(int materialId)
{
    // TODO: Implement material selection by ID
    Q_UNUSED(materialId)
}

void MaterialWidget::addMaterial()
{
    MaterialDialog dialog(MaterialDialog::AddMode, this);
    
    // Set default values for new material
    Material newMaterial;
    newMaterial.id = 0; // Will be auto-assigned
    newMaterial.quantity = 0;
    newMaterial.minimumStock = 0;
    newMaterial.maximumStock = 100;
    newMaterial.reorderPoint = 10;
    newMaterial.price = 0.0;
    newMaterial.supplierId = 0;
    newMaterial.status = "active";
    newMaterial.createdAt = QDateTime::currentDateTime();
    newMaterial.updatedAt = QDateTime::currentDateTime();
    newMaterial.createdBy = "Current User"; // TODO: Get from user session
    newMaterial.updatedBy = "Current User";
    
    dialog.setMaterial(newMaterial);
    
    if (dialog.exec() == QDialog::Accepted) {
        Material material = dialog.getMaterial();
        
        // Assign new ID (in a real app, this would be done by the database)
        material.id = m_model->getNextId();
        material.createdAt = QDateTime::currentDateTime();
        material.updatedAt = QDateTime::currentDateTime();
          if (m_model->addMaterial(material)) {
            // Animate the addition
            if (m_model->rowCount() > 0) {
                QModelIndex lastIndex = m_proxyModel->index(m_model->rowCount() - 1, 0);
                m_tableView->scrollTo(lastIndex);
                m_tableView->selectRow(lastIndex.row());
            }
            
            // Show success message
            QMessageBox::information(this, "Success", 
                                   QString("Material '%1' has been added successfully.")
                                   .arg(material.name));
        } else {
            QMessageBox::warning(this, "Error", 
                               "Failed to add material. Please try again.");
        }
    }
}

void MaterialWidget::editMaterial()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Edit Material", "Please select a material to edit.");
        return;
    }
    
    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    Material currentMaterial = m_model->getMaterial(sourceIndex.row());
    
    MaterialDialog dialog(MaterialDialog::EditMode, this);
    dialog.setMaterial(currentMaterial);
    
    if (dialog.exec() == QDialog::Accepted) {
        Material updatedMaterial = dialog.getMaterial();
        
        // Keep the original ID and created info
        updatedMaterial.id = currentMaterial.id;
        updatedMaterial.createdAt = currentMaterial.createdAt;
        updatedMaterial.createdBy = currentMaterial.createdBy;
        
        // Update modification info
        updatedMaterial.updatedAt = QDateTime::currentDateTime();
        updatedMaterial.updatedBy = "Current User"; // TODO: Get from user session
          if (m_model->updateMaterial(sourceIndex.row(), updatedMaterial)) {
            // Show success message
            QMessageBox::information(this, "Success", 
                                   QString("Material '%1' has been updated successfully.")
                                   .arg(updatedMaterial.name));
        } else {
            QMessageBox::warning(this, "Error", 
                               "Failed to update material. Please try again.");
        }
    }
}

void MaterialWidget::deleteMaterial()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }
    
    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    Material material = m_model->getMaterial(sourceIndex.row());
    
    int ret = QMessageBox::question(this, "Delete Material",
                                   QString("Are you sure you want to delete the material '%1'?")
                                   .arg(material.name),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
      if (ret == QMessageBox::Yes) {
        m_model->removeMaterial(sourceIndex.row());
    }
}

void MaterialWidget::importFromCSV()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                   "Import Materials from CSV",
                                                   QString(),
                                                   "CSV Files (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Import Error", 
                           QString("Could not open file: %1").arg(fileName));
        return;
    }
    
    QTextStream in(&file);
    QStringList lines = in.readAll().split('\n', Qt::SkipEmptyParts);
    
    if (lines.isEmpty()) {
        QMessageBox::warning(this, "Import Error", "The CSV file is empty.");
        return;
    }
    
    // Skip header line if it exists
    if (!lines.isEmpty() && lines.first().contains("name", Qt::CaseInsensitive)) {
        lines.removeFirst();
    }
    
    int imported = 0;
    int failed = 0;
    
    for (const QString &line : lines) {
        QStringList fields = line.split(',');
        if (fields.size() < 4) { // Minimum required fields
            failed++;
            continue;
        }
        
        Material material;
        material.id = m_model->getNextId() + imported;
        material.name = fields[0].trimmed().remove('"');
        material.description = fields.size() > 1 ? fields[1].trimmed().remove('"') : "";
        material.category = fields.size() > 2 ? fields[2].trimmed().remove('"') : "General";
        material.quantity = fields.size() > 3 ? fields[3].trimmed().toInt() : 0;
        material.unit = fields.size() > 4 ? fields[4].trimmed().remove('"') : "pcs";
        material.price = fields.size() > 5 ? fields[5].trimmed().toDouble() : 0.0;
        material.location = fields.size() > 6 ? fields[6].trimmed().remove('"') : "";
        material.minimumStock = fields.size() > 7 ? fields[7].trimmed().toInt() : 10;
        material.status = "active";
        material.createdAt = QDateTime::currentDateTime();
        material.updatedAt = QDateTime::currentDateTime();
        material.createdBy = "Import";
        material.updatedBy = "Import";
        
        if (!material.name.isEmpty() && m_model->addMaterial(material)) {
            imported++;
        } else {
            failed++;
        }    }
    
    QString message = QString("Import completed:\n- %1 materials imported\n- %2 failed")
                     .arg(imported).arg(failed);
    QMessageBox::information(this, "Import Results", message);
}

void MaterialWidget::exportToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                   "Export Materials to CSV",
                                                   "materials.csv",
                                                   "CSV Files (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Error", 
                           QString("Could not create file: %1").arg(fileName));
        return;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "Name,Description,Category,Quantity,Unit,Price,Location,Minimum Stock,Status,Created At\n";
    
    // Write data
    for (int i = 0; i < m_model->rowCount(); ++i) {
        Material material = m_model->getMaterial(i);
        out << QString("\"%1\",\"%2\",\"%3\",%4,\"%5\",%6,\"%7\",%8,\"%9\",\"%10\"\n")
               .arg(material.name)
               .arg(material.description)
               .arg(material.category)
               .arg(material.quantity)
               .arg(material.unit)
               .arg(material.price, 0, 'f', 2)
               .arg(material.location)
               .arg(material.minimumStock)
               .arg(material.status)
               .arg(material.createdAt.toString("yyyy-MM-dd hh:mm:ss"));
    }
    
    QMessageBox::information(this, "Export Complete", 
                           QString("Successfully exported %1 materials to %2")
                           .arg(m_model->rowCount()).arg(fileName));
}

void MaterialWidget::showMaterialDetails()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Material Details", "Please select a material to view details.");
        return;
    }
    
    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    Material material = m_model->getMaterial(sourceIndex.row());
    
    MaterialDetailsDialog *dialog = new MaterialDetailsDialog(material, this);
    
    // Connect edit signal to open edit dialog
    connect(dialog, &MaterialDetailsDialog::editRequested, [this, dialog]() {
        dialog->accept(); // Close details dialog
        editMaterial(); // Open edit dialog
    });
    
    dialog->exec();
    dialog->deleteLater();
}

void MaterialWidget::setupMaterialDetailsForm(QWidget *parent, QVBoxLayout *layout)
{
    qDebug() << "setupMaterialDetailsForm starting...";
    Q_UNUSED(parent)
    
    // Create form fields to match the design
    
    // ID field
    QLabel *idLabel = new QLabel("ID:");
    idLabel->setStyleSheet("font-weight: 500; color: #495057;");
    m_detailIdEdit = new QLineEdit();
    m_detailIdEdit->setObjectName("materialIdEdit");
    m_detailIdEdit->setEnabled(false); // Read-only
    
    // Name field
    QLabel *nameLabel = new QLabel("Name:");
    nameLabel->setStyleSheet("font-weight: 500; color: #495057;");
    m_detailNameEdit = new QLineEdit();
    m_detailNameEdit->setObjectName("materialNameEdit");
    
    // Description field
    QLabel *descLabel = new QLabel("Description:");
    descLabel->setStyleSheet("font-weight: 500; color: #495057;");
    m_detailDescEdit = new QTextEdit();
    m_detailDescEdit->setObjectName("materialDescEdit");
    m_detailDescEdit->setMaximumHeight(60);
    
    // Category dropdown
    QLabel *categoryLabel = new QLabel("Category:");
    categoryLabel->setStyleSheet("font-weight: 500; color: #495057;");
    m_detailCategoryCombo = new QComboBox();
    m_detailCategoryCombo->setObjectName("materialCategoryCombo");
    m_detailCategoryCombo->addItems({"Construction", "Plumbing", "Metal", "Other"});
    
    // Quantity field
    QLabel *quantityLabel = new QLabel("Quantity:");
    quantityLabel->setStyleSheet("font-weight: 500; color: #495057;");
    m_detailQuantitySpinBox = new QSpinBox();
    m_detailQuantitySpinBox->setObjectName("materialQuantitySpinBox");
    m_detailQuantitySpinBox->setMaximum(99999);
    
    // Unit dropdown
    QLabel *unitLabel = new QLabel("Unit:");
    unitLabel->setStyleSheet("font-weight: 500; color: #495057;");
    m_detailUnitCombo = new QComboBox();
    m_detailUnitCombo->setObjectName("materialUnitCombo");
    m_detailUnitCombo->addItems({"Piece", "meters", "bags", "pieces", "kg", "liters"});
    
    // Price field
    QLabel *priceLabel = new QLabel("Price:");
    priceLabel->setStyleSheet("font-weight: 500; color: #495057;");
    m_detailPriceSpinBox = new QDoubleSpinBox();
    m_detailPriceSpinBox->setObjectName("materialPriceSpinBox");
    m_detailPriceSpinBox->setMaximum(999999.99);
    m_detailPriceSpinBox->setDecimals(2);
    
    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_detailNewBtn = new QPushButton("New");
    m_detailSaveBtn = new QPushButton("Save");
    m_detailDeleteBtn = new QPushButton("Delete");
    
    m_detailNewBtn->setObjectName("detailNewButton");
    m_detailSaveBtn->setObjectName("detailSaveButton");
    m_detailDeleteBtn->setObjectName("detailDeleteButton");
      buttonLayout->addWidget(m_detailNewBtn);
    buttonLayout->addWidget(m_detailSaveBtn);
    buttonLayout->addWidget(m_detailDeleteBtn);
    
    qDebug() << "Detail panel buttons created:" << m_detailNewBtn << m_detailSaveBtn << m_detailDeleteBtn;
    
    // Add all fields to layout
    layout->addWidget(idLabel);
    layout->addWidget(m_detailIdEdit);
    layout->addWidget(nameLabel);
    layout->addWidget(m_detailNameEdit);
    layout->addWidget(descLabel);
    layout->addWidget(m_detailDescEdit);
    layout->addWidget(categoryLabel);
    layout->addWidget(m_detailCategoryCombo);
    layout->addWidget(quantityLabel);
    layout->addWidget(m_detailQuantitySpinBox);
    layout->addWidget(unitLabel);
    layout->addWidget(m_detailUnitCombo);
    layout->addWidget(priceLabel);
    layout->addWidget(m_detailPriceSpinBox);    layout->addStretch();
    layout->addLayout(buttonLayout);
    
    qDebug() << "setupMaterialDetailsForm completed successfully";
}

void MaterialWidget::onSearchTextChanged(const QString &text)
{
    if (m_model) {
        m_model->setFilter(text);
    }
}

void MaterialWidget::onCategoryFilterChanged(const QString &category)
{
    if (m_model) {        QString filterValue = (category == "All Categories") ? QString() : category;
        m_model->setCategoryFilter(filterValue);
    }
}

void MaterialWidget::onTableSelectionChanged()
{
    // Update UI based on table selection if needed
    // For now, just update material details panel
    bool hasSelection = !m_tableView->selectionModel()->selectedRows().isEmpty();
    
    if (hasSelection) {
        QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
        QModelIndex proxyIndex = selected.first();
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        Material material = m_model->getMaterial(sourceIndex.row());
        populateDetailForm(material);
        
        // Enable edit/delete buttons
        m_editButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
        m_viewButton->setEnabled(true);
        m_detailSaveBtn->setEnabled(true);
        m_detailDeleteBtn->setEnabled(true);
    } else {
        clearDetailForm();
        
        // Disable edit/delete buttons
        m_editButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
        m_viewButton->setEnabled(false);
        m_detailSaveBtn->setEnabled(false);
        m_detailDeleteBtn->setEnabled(false);
    }
}

void MaterialWidget::saveMaterialFromDetail()
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    
    // Check if we're adding a new material or updating an existing one
    if (selected.isEmpty()) {
        // Adding new material
        addMaterialFromDetail();
        return;
    }
    
    // Updating existing material
    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    Material currentMaterial = m_model->getMaterial(sourceIndex.row());
    
    // Validate input
    if (m_detailNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Material name cannot be empty.");
        m_detailNameEdit->setFocus();
        return;
    }
    
    // Create updated material from form
    Material updatedMaterial;
    updatedMaterial.id = currentMaterial.id;
    updatedMaterial.name = m_detailNameEdit->text().trimmed();
    updatedMaterial.description = m_detailDescEdit->toPlainText().trimmed();
    updatedMaterial.category = m_detailCategoryCombo->currentText();
    updatedMaterial.quantity = m_detailQuantitySpinBox->value();
    updatedMaterial.unit = m_detailUnitCombo->currentText();
    updatedMaterial.price = m_detailPriceSpinBox->value();
    updatedMaterial.location = currentMaterial.location;
    updatedMaterial.minimumStock = currentMaterial.minimumStock;
    updatedMaterial.maximumStock = currentMaterial.maximumStock;
    updatedMaterial.reorderPoint = currentMaterial.reorderPoint;
    updatedMaterial.supplierId = currentMaterial.supplierId;
    updatedMaterial.barcode = currentMaterial.barcode;
    updatedMaterial.status = currentMaterial.status;
    updatedMaterial.createdAt = currentMaterial.createdAt;
    updatedMaterial.createdBy = currentMaterial.createdBy;
    updatedMaterial.updatedAt = QDateTime::currentDateTime();
    updatedMaterial.updatedBy = "Current User"; // TODO: Get from user session
    
    if (m_model->updateMaterial(sourceIndex.row(), updatedMaterial)) {
        QMessageBox::information(this, "Success", 
                               QString("Material '%1' has been updated successfully.")
                               .arg(updatedMaterial.name));
    } else {
        QMessageBox::warning(this, "Error", 
                           "Failed to update material. Please try again.");
    }
}

void MaterialWidget::addMaterialFromDetail()
{
    // Validate input
    if (m_detailNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Material name cannot be empty.");
        m_detailNameEdit->setFocus();
        return;
    }
    
    // Create new material from form
    Material newMaterial;
    newMaterial.id = m_model->getNextId();
    newMaterial.name = m_detailNameEdit->text().trimmed();
    newMaterial.description = m_detailDescEdit->toPlainText().trimmed();
    newMaterial.category = m_detailCategoryCombo->currentText();
    newMaterial.quantity = m_detailQuantitySpinBox->value();
    newMaterial.unit = m_detailUnitCombo->currentText();
    newMaterial.price = m_detailPriceSpinBox->value();
    newMaterial.location = "";
    newMaterial.minimumStock = 10;
    newMaterial.maximumStock = 100;
    newMaterial.reorderPoint = 5;
    newMaterial.supplierId = 0;
    newMaterial.barcode = "";
    newMaterial.status = "active";
    newMaterial.createdAt = QDateTime::currentDateTime();
    newMaterial.updatedAt = QDateTime::currentDateTime();
    newMaterial.createdBy = "Current User"; // TODO: Get from user session
    newMaterial.updatedBy = "Current User";
    
    if (m_model->addMaterial(newMaterial)) {
        // Clear form after successful addition
        clearDetailForm();
        
        // Select the newly added material
        if (m_model->rowCount() > 0) {
            QModelIndex lastIndex = m_proxyModel->index(m_model->rowCount() - 1, 0);
            m_tableView->scrollTo(lastIndex);
            m_tableView->selectRow(lastIndex.row());
        }
        
        QMessageBox::information(this, "Success", 
                               QString("Material '%1' has been added successfully.")
                               .arg(newMaterial.name));
    } else {
        QMessageBox::warning(this, "Error", 
                           "Failed to add material. Please try again.");
    }
}

void MaterialWidget::clearDetailForm()
{
    m_detailIdEdit->clear();
    m_detailNameEdit->clear();
    m_detailDescEdit->clear();
    m_detailCategoryCombo->setCurrentIndex(0);
    m_detailQuantitySpinBox->setValue(0);
    m_detailUnitCombo->setCurrentIndex(0);
    m_detailPriceSpinBox->setValue(0.0);
}

void MaterialWidget::populateDetailForm(const Material &material)
{
    m_detailIdEdit->setText(QString::number(material.id));
    m_detailNameEdit->setText(material.name);
    m_detailDescEdit->setPlainText(material.description);
    
    // Set category
    int categoryIndex = m_detailCategoryCombo->findText(material.category);
    if (categoryIndex >= 0) {
        m_detailCategoryCombo->setCurrentIndex(categoryIndex);
    }
    
    m_detailQuantitySpinBox->setValue(material.quantity);
    
    // Set unit
    int unitIndex = m_detailUnitCombo->findText(material.unit);
    if (unitIndex >= 0) {
        m_detailUnitCombo->setCurrentIndex(unitIndex);
    }
    
    m_detailPriceSpinBox->setValue(material.price);
}

void MaterialWidget::resetDatabase()
{
    // Offer to backup data first if there are materials
    if (m_model->rowCount() > 0) {
        QMessageBox::StandardButton backup = QMessageBox::question(this,
            "Backup Before Reset",
            QString("You have %1 materials in the database.\n\n"
                   "Would you like to export them to CSV before resetting?\n"
                   "This will create a backup of your current data.")
                   .arg(m_model->rowCount()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Yes);
            
        if (backup == QMessageBox::Cancel) {
            return;
        } else if (backup == QMessageBox::Yes) {
            // Auto-generate backup filename with timestamp
            QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
            QString backupFileName = QString("materials_backup_%1.csv").arg(timestamp);
            
            QString fileName = QFileDialog::getSaveFileName(this,
                                                           "Backup Materials to CSV",
                                                           backupFileName,
                                                           "CSV Files (*.csv)");
            if (!fileName.isEmpty()) {
                // Export current data
                QFile file(fileName);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&file);
                    
                    // Write header
                    out << "Name,Description,Category,Quantity,Unit,Price,Location,Minimum Stock,Status,Created At\n";
                    
                    // Write data
                    for (int i = 0; i < m_model->rowCount(); ++i) {
                        Material material = m_model->getMaterial(i);
                        out << QString("\"%1\",\"%2\",\"%3\",%4,\"%5\",%6,\"%7\",%8,\"%9\",\"%10\"\n")
                               .arg(material.name)
                               .arg(material.description)
                               .arg(material.category)
                               .arg(material.quantity)
                               .arg(material.unit)
                               .arg(material.price, 0, 'f', 2)
                               .arg(material.location)
                               .arg(material.minimumStock)
                               .arg(material.status)
                               .arg(material.createdAt.toString("yyyy-MM-dd hh:mm:ss"));
                    }
                    
                    QMessageBox::information(this, "Backup Complete",
                                           QString("Backup saved successfully to:\n%1").arg(fileName));
                } else {
                    QMessageBox::warning(this, "Backup Failed",
                                       "Could not create backup file. Reset cancelled for safety.");
                    return;
                }
            } else {
                // User cancelled backup dialog
                return;
            }
        }
    }
    
    // Show confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
        "Reset Database", 
        "Are you sure you want to reset the materials database?\n\n"
        "This will:\n"
        "• Delete all existing materials\n"
        "• Load fresh sample data\n"
        "• Clear all filters and selections\n\n"
        "This action cannot be undone!",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // Show second confirmation for extra safety
    QMessageBox::StandardButton finalReply = QMessageBox::warning(this,
        "Final Confirmation",
        "Last chance! This will permanently delete all material data.\n\n"
        "Click Yes to proceed with the reset.",
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);
        
    if (finalReply != QMessageBox::Yes) {
        return;
    }
    
    // Clear the database
    m_model->clearAllMaterials();
    
    // Show options for what to do next
    QMessageBox::StandardButton loadSample = QMessageBox::question(this,
        "Database Reset Complete",
        "The materials database has been reset successfully.\n\n"
        "Would you like to load sample data for testing?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    
    if (loadSample == QMessageBox::Yes) {
        m_model->loadSampleData();
        QMessageBox::information(this, "Sample Data Loaded", 
                               "Sample materials have been loaded successfully.");
    }
    
    // Clear all filters and selections
    m_searchEdit->clear();
    m_categoryFilter->setCurrentIndex(0);
    clearDetailForm();
    
    // Refresh the view
    refreshData();
    
    qDebug() << "Database reset completed";
}

void MaterialWidget::initializeAIAssistant()
{
    // Initialize Groq client
    m_groqClient = new GroqClient(this);
      // Set up configuration - Load API key from environment or settings
    GroqConfig config;
    
    // First try to load from .env file, then fall back to QSettings
    QString apiKey = EnvironmentLoader::getEnv("GROQ_API_KEY");
    
    if (apiKey.isEmpty()) {
        // Fall back to QSettings
        QSettings settings;
        apiKey = settings.value("AI/GroqApiKey", "").toString();
    }
    
    if (apiKey.isEmpty()) {
        qDebug() << "No Groq API key found. Please set GROQ_API_KEY environment variable or configure in settings.";
        // Use empty string - user needs to configure
        apiKey = "";
        qDebug() << "Groq API key not configured. Please set your API key in settings.";
    }
    
    config.apiKey = apiKey;
    config.model = "llama-3.3-70b-versatile";
    config.maxTokens = 4096;
    config.temperature = 0.7;
    config.timeout = 30000;
    
    m_groqClient->setConfiguration(config);
    
    // Set materials-specific system prompt
    QString systemPrompt = 
        "You are an AI assistant specialized in materials management for architecture and construction projects. "
        "You have expertise in:\n"
        "- Building materials specifications and properties\n"
        "- Cost estimation and budget analysis\n"
        "- Supplier recommendations and sourcing\n"
        "- Inventory management best practices\n"
        "- Sustainable and eco-friendly material alternatives\n"
        "- Construction standards and building codes\n"
        "- Material quantity calculations and waste optimization\n"
        "- Quality control and inspection procedures\n\n"
        "Provide clear, professional, and actionable advice. When discussing costs, always mention that prices may vary by location and market conditions. "
        "Include relevant technical specifications when appropriate. Focus on practical solutions for construction professionals.";
    
    m_groqClient->setSystemPrompt(systemPrompt);
      // Initialize AI dialog
    m_aiDialog = new AIAssistantDialog(this);
    m_aiDialog->setGroqClient(m_groqClient);
    
    // Connect to handle dialog destruction (safety measure)
    connect(m_aiDialog, &QObject::destroyed, this, [this]() {
        m_aiDialog = nullptr;
    });
}

void MaterialWidget::openAIAssistant()
{
    // Always ensure we have a valid dialog
    if (!m_aiDialog) {
        initializeAIAssistant();
    }
    
    if (m_aiDialog) {
        // Check if API key is configured
        if (!m_groqClient->isConnected()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "AI Assistant Setup",
                "The AI Assistant requires a Groq API key to function.\n\n"
                "Would you like to:\n"
                "• Get a free API key from console.groq.com\n"
                "• Configure your existing API key\n\n"
                "You can set the API key in the application settings or as an environment variable 'GROQ_API_KEY'.",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
            );
            
            if (reply == QMessageBox::Yes) {
                showAISetupDialog();
                return;
            }
        }
        
        // Show the AI dialog
        m_aiDialog->show();
        m_aiDialog->raise();
        m_aiDialog->activateWindow();
        
        // Add some context about current materials if any are selected
        QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedRows();
        if (!selectedIndexes.isEmpty()) {
            QModelIndex sourceIndex = m_proxyModel->mapToSource(selectedIndexes.first());
            Material material = m_model->getMaterial(sourceIndex.row());
            
            QString contextMessage = QString(
                "I'm currently viewing material: %1 (%2)\n"
                "Category: %3\n"
                "Quantity: %4 %5\n"
                "Price: $%6\n\n"
                "How can I help you with this material or materials management in general?"
            ).arg(material.name)
             .arg(material.description)
             .arg(material.category)
             .arg(material.quantity)
             .arg(material.unit)
             .arg(material.price, 0, 'f', 2);
             
            // This will be shown as context in the AI dialog
            // The user can see this but it won't be sent automatically
        }
    }
}

void MaterialWidget::showAISetupDialog()
{
    // Create a simple setup dialog
    QDialog *setupDialog = new QDialog(this);
    setupDialog->setWindowTitle("AI Assistant Setup");
    setupDialog->setModal(true);
    setupDialog->resize(500, 300);
    
    QVBoxLayout *layout = new QVBoxLayout(setupDialog);
    
    QLabel *titleLabel = new QLabel("🤖 AI Assistant Setup");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #E3C6B0; margin-bottom: 10px;");
    
    QLabel *infoLabel = new QLabel(
        "To use the AI Assistant, you need a Groq API key:\n\n"
        "1. Visit: https://console.groq.com\n"
        "2. Sign up for a free account\n"
        "3. Generate an API key\n"
        "4. Enter the key below\n\n"
        "Your API key will be stored securely and only used for AI requests."
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #E3C6B0; line-height: 1.4;");
    
    QLineEdit *apiKeyEdit = new QLineEdit();
    apiKeyEdit->setPlaceholderText("Enter your Groq API key (gsk_...)");
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyEdit->setStyleSheet(
        "background-color: #2A3340; "
        "border: 2px solid #E3C6B0; "
        "border-radius: 8px; "
        "padding: 8px; "
        "color: #E3C6B0; "
        "font-family: 'Courier New', monospace;"
    );
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *saveButton = new QPushButton("Save & Continue");
    QPushButton *cancelButton = new QPushButton("Cancel");
    
    saveButton->setStyleSheet(
        "background-color: #E3C6B0; "
        "color: #3D485A; "
        "border: none; "
        "border-radius: 8px; "
        "padding: 10px 20px; "
        "font-weight: bold;"
    );
    
    cancelButton->setStyleSheet(
        "background-color: transparent; "
        "color: #E3C6B0; "
        "border: 1px solid #E3C6B0; "
        "border-radius: 8px; "
        "padding: 10px 20px;"
    );
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);
    
    layout->addWidget(titleLabel);
    layout->addWidget(infoLabel);
    layout->addSpacing(20);
    layout->addWidget(apiKeyEdit);
    layout->addSpacing(20);
    layout->addLayout(buttonLayout);
    
    // Apply ArchiFlow theme to dialog
    setupDialog->setStyleSheet(
        "QDialog { background-color: #2A3340; }"
        "QLabel { color: #E3C6B0; }"
    );
    
    connect(saveButton, &QPushButton::clicked, [this, apiKeyEdit, setupDialog]() {
        QString apiKey = apiKeyEdit->text().trimmed();
        if (apiKey.isEmpty()) {
            QMessageBox::warning(setupDialog, "Invalid API Key", "Please enter a valid API key.");
            return;
        }
        
        // Save the API key
        QSettings settings;
        settings.setValue("AI/GroqApiKey", apiKey);
        
        // Update the Groq client configuration
        GroqConfig config = m_groqClient->configuration();
        config.apiKey = apiKey;
        m_groqClient->setConfiguration(config);
        
        setupDialog->accept();
        
        // Now open the AI assistant
        QTimer::singleShot(100, this, &MaterialWidget::openAIAssistant);
    });
    
    connect(cancelButton, &QPushButton::clicked, setupDialog, &QDialog::reject);
    
    setupDialog->exec();
    setupDialog->deleteLater();
}

void MaterialWidget::openAIPrediction()
{
    // Always ensure we have a valid dialog
    if (!m_aiPredictionDialog) {
        m_aiPredictionDialog = new AIPredictionDialog(this);
    }
    
    if (m_aiPredictionDialog) {
        // Show the AI Prediction dialog
        m_aiPredictionDialog->show();
        m_aiPredictionDialog->raise();
        m_aiPredictionDialog->activateWindow();
    }
}

void MaterialWidget::setupDashboard()
{
    m_dashboardWidget = new QWidget();
    
    // Create scroll area for responsive design
    QScrollArea *scrollArea = new QScrollArea(m_dashboardWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Main dashboard layout
    QVBoxLayout *mainLayout = new QVBoxLayout(m_dashboardWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);
    
    // Scrollable content widget
    QWidget *scrollableContent = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(scrollableContent);
    dashboardLayout->setContentsMargins(20, 20, 20, 20);
    dashboardLayout->setSpacing(20);
    
    // Dashboard header
    QLabel *dashboardTitle = new QLabel("Materials Dashboard");
    dashboardTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    dashboardLayout->addWidget(dashboardTitle);
    
    // Stats cards container with responsive wrapping
    QWidget *statsContainer = new QWidget();
    QGridLayout *statsLayout = new QGridLayout(statsContainer);
    statsLayout->setSpacing(15);
    
    // Create statistics cards
    m_totalMaterialsCard = createStatCard("Total Materials", "0", "#3498db");
    m_lowStockCard = createStatCard("Low Stock", "0", "#e74c3c");
    m_totalValueCard = createStatCard("Total Value", "$0", "#27ae60");
    m_categoriesCard = createStatCard("Categories", "0", "#f39c12");
    
    // Use grid layout for better responsive behavior
    statsLayout->addWidget(m_totalMaterialsCard, 0, 0);
    statsLayout->addWidget(m_lowStockCard, 0, 1);
    statsLayout->addWidget(m_totalValueCard, 0, 2);
    statsLayout->addWidget(m_categoriesCard, 0, 3);
    
    // Make stats layout responsive
    for (int i = 0; i < 4; ++i) {
        statsLayout->setColumnStretch(i, 1);
    }
    
    dashboardLayout->addWidget(statsContainer);
    
    // Charts section with responsive grid layout
    QWidget *chartsContainer = new QWidget();
    QGridLayout *chartsLayout = new QGridLayout(chartsContainer);
    chartsLayout->setSpacing(15);
    
    // Setup charts
    setupCharts();
    
    // Arrange charts in a responsive grid (2x2 for larger screens, 1x3 for smaller)
    chartsLayout->addWidget(m_categoryPieChart, 0, 0);
    chartsLayout->addWidget(m_stockLevelsBarChart, 0, 1);
    chartsLayout->addWidget(m_valueDistributionChart, 1, 0, 1, 2); // Span 2 columns
    
    // Set stretch factors for responsive design
    chartsLayout->setColumnStretch(0, 1);
    chartsLayout->setColumnStretch(1, 1);
    chartsLayout->setRowStretch(0, 1);
    chartsLayout->setRowStretch(1, 1);
    
    dashboardLayout->addWidget(chartsContainer);
    
    // Statistics summary section
    QWidget *statisticsWidget = new QWidget();
    QVBoxLayout *statisticsLayout = new QVBoxLayout(statisticsWidget);
    
    QLabel *statisticsTitle = new QLabel("Advanced Statistics");
    statisticsTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    statisticsLayout->addWidget(statisticsTitle);
    
    // Create a grid for additional statistics
    QWidget *statsGridWidget = new QWidget();
    QGridLayout *statsGrid = new QGridLayout(statsGridWidget);
    statsGrid->setSpacing(10);
      // Add additional statistics cards
    m_avgPriceCard = createStatCard("Avg Price", "$0", "#8e44ad");
    m_mostStockedCard = createStatCard("Most Stocked", "N/A", "#16a085");
    m_leastStockedCard = createStatCard("Least Stocked", "N/A", "#d35400");
    m_totalCostCard = createStatCard("Inventory Cost", "$0", "#c0392b");
    
    statsGrid->addWidget(m_avgPriceCard, 0, 0);
    statsGrid->addWidget(m_mostStockedCard, 0, 1);
    statsGrid->addWidget(m_leastStockedCard, 0, 2);
    statsGrid->addWidget(m_totalCostCard, 0, 3);
    
    statisticsLayout->addWidget(statsGridWidget);
    
    dashboardLayout->addWidget(statisticsWidget);
    
    // Recent activity section
    QWidget *recentActivityWidget = new QWidget();
    QVBoxLayout *activityLayout = new QVBoxLayout(recentActivityWidget);
    
    QLabel *activityTitle = new QLabel("Recent Activity");
    activityTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    activityLayout->addWidget(activityTitle);
    
    m_recentActivityList = new QListWidget();
    m_recentActivityList->setMaximumHeight(200);
    m_recentActivityList->setStyleSheet(
        "QListWidget { "
        "border: 1px solid #bdc3c7; "
        "border-radius: 5px; "
        "background-color: white; "
        "} "
        "QListWidget::item { "
        "padding: 10px; "
        "border-bottom: 1px solid #ecf0f1; "
        "} "
        "QListWidget::item:hover { "
        "background-color: #f8f9fa; "
        "}"
    );
    activityLayout->addWidget(m_recentActivityList);
    
    dashboardLayout->addWidget(recentActivityWidget);
    
    // Quick actions section
    QWidget *quickActionsWidget = new QWidget();
    QVBoxLayout *quickActionsLayout = new QVBoxLayout(quickActionsWidget);
    
    QLabel *quickActionsTitle = new QLabel("Quick Actions");
    quickActionsTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    quickActionsLayout->addWidget(quickActionsTitle);
    
    QHBoxLayout *actionsButtonsLayout = new QHBoxLayout();
    
    QPushButton *addMaterialBtn = new QPushButton("Add Material");
    QPushButton *importDataBtn = new QPushButton("Import Data");
    QPushButton *generateReportBtn = new QPushButton("Generate Report");
    
    QString buttonStyle = 
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "} "
        "QPushButton:pressed { "
        "background-color: #21618c; "
        "}";
    
    addMaterialBtn->setStyleSheet(buttonStyle);
    importDataBtn->setStyleSheet(buttonStyle);
    generateReportBtn->setStyleSheet(buttonStyle);
    
    actionsButtonsLayout->addWidget(addMaterialBtn);
    actionsButtonsLayout->addWidget(importDataBtn);
    actionsButtonsLayout->addWidget(generateReportBtn);
    actionsButtonsLayout->addStretch();
    
    quickActionsLayout->addLayout(actionsButtonsLayout);
      dashboardLayout->addWidget(quickActionsWidget);
    dashboardLayout->addStretch();
    
    // Set the scrollable content to the scroll area
    scrollArea->setWidget(scrollableContent);
      // Connect quick action buttons
    connect(addMaterialBtn, &QPushButton::clicked, this, &MaterialWidget::addMaterial);
    connect(importDataBtn, &QPushButton::clicked, this, &MaterialWidget::importFromCSV);
    connect(generateReportBtn, &QPushButton::clicked, this, &MaterialWidget::exportToCSV);
}

QWidget* MaterialWidget::createStatCard(const QString &title, const QString &value, const QString &color)
{
    QWidget *card = new QWidget();
    card->setMinimumSize(180, 100);
    card->setMaximumHeight(140);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setStyleSheet(QString(
        "QWidget { "
        "background-color: white; "
        "border-radius: 10px; "
        "border-left: 5px solid %1; "
        "}"
    ).arg(color));
    
    // Add shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);
    
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 12px; color: #7f8c8d; font-weight: bold;");
    
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet(QString("font-size: 24px; color: %1; font-weight: bold;").arg(color));
    valueLabel->setObjectName("valueLabel");
    
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(valueLabel);
    cardLayout->addStretch();
    
    return card;
}

void MaterialWidget::showDashboard()
{
    // Update button states
    m_dashboardBtn->setChecked(true);
    m_materialsBtn->setChecked(false);
    m_suppliersBtn->setChecked(false);
    m_reportsBtn->setChecked(false);
    m_settingsBtn->setChecked(false);
    
    // Show dashboard, hide materials list
    m_dashboardWidget->show();
    m_materialsListWidget->hide();
    m_supplierWidget->hide();
    m_reportsWidget->hide();
    m_settingsWidget->hide();
      // Hide the Material Details panel when not in Materials section
    hideRightPanel();
    
    // Update dashboard statistics and charts
    if (m_model) {
        updateDashboardStats();
        updateCharts();
    }
}

void MaterialWidget::showMaterialsList()
{
    // Update button states
    m_materialsBtn->setChecked(true);
    m_dashboardBtn->setChecked(false);
    m_suppliersBtn->setChecked(false);
    m_reportsBtn->setChecked(false);
    m_settingsBtn->setChecked(false);
    
    // Show materials list, hide others
    m_materialsListWidget->show();
    m_dashboardWidget->hide();
    m_supplierWidget->hide();
    m_reportsWidget->hide();
    m_settingsWidget->hide();
    
    // Show the Material Details panel only when Materials is selected
    showRightPanel();
}

void MaterialWidget::showSuppliers()
{
    // Update button states
    m_suppliersBtn->setChecked(true);
    m_materialsBtn->setChecked(false);
    m_dashboardBtn->setChecked(false);
    m_reportsBtn->setChecked(false);
    m_settingsBtn->setChecked(false);
    
    // Show suppliers widget, hide others
    m_supplierWidget->show();
    m_materialsListWidget->hide();
    m_dashboardWidget->hide();
    m_reportsWidget->hide();
    m_settingsWidget->hide();
    
    // Hide the Material Details panel when not in Materials section
    hideRightPanel();
}

void MaterialWidget::showReports()
{
    // Update button states
    m_reportsBtn->setChecked(true);
    m_materialsBtn->setChecked(false);
    m_dashboardBtn->setChecked(false);
    m_suppliersBtn->setChecked(false);
    m_settingsBtn->setChecked(false);
    
    // Show reports widget, hide others
    m_reportsWidget->show();
    m_materialsListWidget->hide();
    m_dashboardWidget->hide();
    m_supplierWidget->hide();
    m_settingsWidget->hide();
    
    // Hide the Material Details panel when not in Materials section
    hideRightPanel();
}

void MaterialWidget::showSettings()
{
    // Update button states
    m_settingsBtn->setChecked(true);
    m_materialsBtn->setChecked(false);
    m_dashboardBtn->setChecked(false);
    m_suppliersBtn->setChecked(false);
    m_reportsBtn->setChecked(false);
    
    // Show settings widget, hide others
    m_settingsWidget->show();
    m_materialsListWidget->hide();
    m_dashboardWidget->hide();
    m_supplierWidget->hide();
    m_reportsWidget->hide();
    
    // Hide the Material Details panel when not in Materials section
    hideRightPanel();
}

void MaterialWidget::updateDashboardStats()
{
    if (!m_model) {
        qDebug() << "MaterialWidget::updateDashboardStats() - Model is null";
        return;
    }
    
    qDebug() << "MaterialWidget::updateDashboardStats() - Starting stats update";
    
    int totalMaterials = m_model->rowCount();
    int lowStockCount = 0;
    double totalValue = 0.0;
    QSet<QString> categories;
    
    qDebug() << "MaterialWidget::updateDashboardStats() - Total materials:" << totalMaterials;// Calculate statistics
    double totalInventoryCost = 0.0;
    QList<double> prices;
    QMap<QString, int> materialQuantities;
    
    for (int row = 0; row < totalMaterials; ++row) {
        // Get quantity and check for low stock
        QModelIndex quantityIndex = m_model->index(row, 4); // QuantityColumn = 4
        int quantity = m_model->data(quantityIndex).toInt();
          // Calculate total value (price * quantity)
        // Use Qt::UserRole to get the raw Material object, then access price directly
        QModelIndex materialIndex = m_model->index(row, 0);
        QVariant materialVariant = m_model->data(materialIndex, Qt::UserRole);
        
        double price = 0.0;
        QString materialName;
        if (materialVariant.isValid()) {
            Material material = materialVariant.value<Material>();
            price = material.price;
            materialName = material.name;
        } else {
            // Fallback: try to parse the formatted price string
            QModelIndex priceIndex = m_model->index(row, 6); // PriceColumn = 6
            QString priceStr = m_model->data(priceIndex).toString();
            // Remove currency symbol and parse
            priceStr.remove('$').remove(',');
            price = priceStr.toDouble();
            
            QModelIndex nameIndex = m_model->index(row, 1);
            materialName = m_model->data(nameIndex).toString();
        }
        
        qDebug() << "Row" << row << "- Quantity:" << quantity << "Price:" << price;
        
        if (quantity < 10) { // Arbitrary low stock threshold
            lowStockCount++;
        }
        
        totalValue += price * quantity;
        totalInventoryCost += price * quantity;
        prices.append(price);
        materialQuantities[materialName] = quantity;
        
        // Collect categories
        QModelIndex categoryIndex = m_model->index(row, 3); // CategoryColumn = 3
        QString category = m_model->data(categoryIndex).toString();
        if (!category.isEmpty()) {
            categories.insert(category);
        }
    }
    
    // Calculate additional statistics
    double averagePrice = totalMaterials > 0 ? (totalValue / totalMaterials) : 0.0;
    
    // Find most and least stocked materials
    QString mostStocked = "N/A";
    QString leastStocked = "N/A";
    int maxQuantity = 0;
    int minQuantity = INT_MAX;
    
    for (auto it = materialQuantities.begin(); it != materialQuantities.end(); ++it) {
        if (it.value() > maxQuantity) {
            maxQuantity = it.value();
            mostStocked = it.key();
        }
        if (it.value() < minQuantity) {
            minQuantity = it.value();
            leastStocked = it.key();
        }
    }
      qDebug() << "Final stats - Total Value:" << totalValue << "Low Stock:" << lowStockCount << "Categories:" << categories.size();
    
    // Update stat cards
    updateStatCard(m_totalMaterialsCard, QString::number(totalMaterials));
    updateStatCard(m_lowStockCard, QString::number(lowStockCount));
    updateStatCard(m_totalValueCard, QString("$%L1").arg(totalValue, 0, 'f', 2));
    updateStatCard(m_categoriesCard, QString::number(categories.size()));
    
    // Update additional stat cards
    updateStatCard(m_avgPriceCard, QString("$%L1").arg(averagePrice, 0, 'f', 2));
    updateStatCard(m_mostStockedCard, mostStocked);
    updateStatCard(m_leastStockedCard, leastStocked);
    updateStatCard(m_totalCostCard, QString("$%L1").arg(totalInventoryCost, 0, 'f', 2));
      // Update recent activity
    updateRecentActivity();
    
    // Temporarily disable chart updates to isolate crash
    // updateCharts();
}

void MaterialWidget::updateStatCard(QWidget *card, const QString &newValue)
{
    if (!card) return;
    
    QLabel *valueLabel = card->findChild<QLabel*>("valueLabel");
    if (valueLabel) {
        valueLabel->setText(newValue);
    }
}

void MaterialWidget::updateRecentActivity()
{
    if (!m_recentActivityList) return;
    
    m_recentActivityList->clear();
    
    // Add some sample recent activity items
    // In a real application, this would come from a database or activity log
    QStringList activities = {
        "Material 'Steel Beam' quantity updated",
        "New category 'Electrical' added",
        "Low stock alert for 'Copper Wire'",
        "Material 'Concrete Mix' added to inventory",
        "Supplier information updated"
    };
    
    for (const QString &activity : activities) {
        QListWidgetItem *item = new QListWidgetItem(activity);
        item->setIcon(QIcon(":/icons/activity.png")); // You may need to add this icon
        m_recentActivityList->addItem(item);
    }
}

void MaterialWidget::onDashboardCardClicked()
{
    // Handle dashboard card clicks
    // This could be used to show detailed views or navigate to specific sections
    QWidget *clickedCard = qobject_cast<QWidget*>(sender());
    if (!clickedCard) return;
    
    // You can add specific handling based on which card was clicked
    // For now, just update the dashboard stats
    updateDashboardStats();
}

void MaterialWidget::setupCharts()
{
    // Create category pie chart
    m_categoryPieChart = createCategoryPieChart();
    
    // Create stock levels bar chart
    m_stockLevelsBarChart = createStockLevelsBarChart();
    
    // Create value distribution chart
    m_valueDistributionChart = createValueDistributionChart();
}

QChartView* MaterialWidget::createCategoryPieChart()
{
    // Create pie series for categories
    m_categoryPieSeries = new QPieSeries();
    
    // Create chart
    QChart *chart = new QChart();
    chart->addSeries(m_categoryPieSeries);
    chart->setTitle("Materials by Category");
    chart->setTitleFont(QFont("Arial", 14, QFont::Bold));
    
    // Style the chart
    chart->setBackgroundBrush(QBrush(Qt::white));
    chart->setBackgroundRoundness(10);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    // Create chart view with fully responsive sizing
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setMinimumHeight(300); // Set reasonable minimum height only
    
    return chartView;
}

QChartView* MaterialWidget::createStockLevelsBarChart()
{
    // Create bar series for stock levels
    m_stockBarSeries = new QBarSeries();
    
    // Create chart
    QChart *chart = new QChart();
    chart->addSeries(m_stockBarSeries);
    chart->setTitle("Stock Levels by Category");
    chart->setTitleFont(QFont("Arial", 14, QFont::Bold));
    
    // Create axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Quantity");
    
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    m_stockBarSeries->attachAxis(axisX);
    m_stockBarSeries->attachAxis(axisY);
    
    // Style the chart
    chart->setBackgroundBrush(QBrush(Qt::white));
    chart->setBackgroundRoundness(10);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    // Create chart view with fully responsive sizing
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setMinimumHeight(300); // Set reasonable minimum height only
    
    return chartView;
}

QChartView* MaterialWidget::createValueDistributionChart()
{
    // Create bar series for value distribution
    m_valueBarSeries = new QBarSeries();
    
    // Create chart
    QChart *chart = new QChart();
    chart->addSeries(m_valueBarSeries);
    chart->setTitle("Value Distribution by Category");
    chart->setTitleFont(QFont("Arial", 14, QFont::Bold));
    
    // Create axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value ($)");
    
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    m_valueBarSeries->attachAxis(axisX);
    m_valueBarSeries->attachAxis(axisY);
    
    // Style the chart
    chart->setBackgroundBrush(QBrush(Qt::white));
    chart->setBackgroundRoundness(10);
    chart->legend()->setAlignment(Qt::AlignBottom);
      // Create chart view with fully responsive sizing
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300); // Set reasonable minimum height only
    
    return chartView;
}

void MaterialWidget::updateCharts()
{
    if (!m_model || !m_categoryPieSeries || !m_stockBarSeries || !m_valueBarSeries) {
        qDebug() << "updateCharts: Missing model or chart series";
        return;
    }
    
    if (!m_stockLevelsBarChart || !m_valueDistributionChart) {
        qDebug() << "updateCharts: Chart views not initialized yet";
        return;
    }
    
    qDebug() << "updateCharts: Starting chart update with" << m_model->rowCount() << "materials";
    
    // Clear existing data
    m_categoryPieSeries->clear();
    m_stockBarSeries->clear();
    m_valueBarSeries->clear();
    
    // Collect data for charts
    QMap<QString, int> categoryQuantities;
    QMap<QString, double> categoryValues;
    QStringList categories;
    
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QModelIndex materialIndex = m_model->index(row, 0);
        QVariant materialData = m_model->data(materialIndex, Qt::UserRole);
        
        if (materialData.canConvert<Material>()) {
            Material material = materialData.value<Material>();
            
            qDebug() << "Processing material:" << material.name << "Category:" << material.category 
                     << "Quantity:" << material.quantity << "Price:" << material.price;
            
            // Accumulate data by category
            categoryQuantities[material.category] += material.quantity;
            categoryValues[material.category] += material.price * material.quantity;
            
            if (!categories.contains(material.category)) {
                categories.append(material.category);
            }
        } else {
            qDebug() << "Row" << row << "doesn't contain valid Material data";
        }
    }
    
    qDebug() << "Found categories:" << categories;
    qDebug() << "Category quantities:" << categoryQuantities;
    qDebug() << "Category values:" << categoryValues;
    
    if (categories.isEmpty()) {
        qDebug() << "No categories found, charts will be empty";
        return;
    }
    
    // Update pie chart for category distribution
    QStringList pieColors = {"#3498db", "#e74c3c", "#27ae60", "#f39c12", "#9b59b6", "#1abc9c", "#34495e", "#e67e22"};
    int colorIndex = 0;
    
    for (auto it = categoryQuantities.begin(); it != categoryQuantities.end(); ++it) {
        if (it.value() > 0) {  // Only add slices with positive values
            QPieSlice *slice = m_categoryPieSeries->append(it.key(), it.value());
            slice->setColor(QColor(pieColors[colorIndex % pieColors.size()]));
            slice->setLabelVisible(true);
                       slice->setLabel(QString("%1 (%2)").arg(it.key()).arg(it.value()));
            colorIndex++;
        }
    }
    
    // Update stock levels bar chart
    QBarSet *stockSet = new QBarSet("Stock Quantity");
    stockSet->setColor(QColor("#3498db"));
    
    for (const QString &category : categories) {
        *stockSet << categoryQuantities[category];
    }
      m_stockBarSeries->append(stockSet);
    
    // Update chart axes
    if (m_stockLevelsBarChart && m_stockLevelsBarChart->chart() && 
        !m_stockLevelsBarChart->chart()->axes(Qt::Horizontal).isEmpty()) {
        if (QBarCategoryAxis *axisX = qobject_cast<QBarCategoryAxis*>(m_stockLevelsBarChart->chart()->axes(Qt::Horizontal).first())) {
            axisX->setCategories(categories);
        }
    }
    
    // Update value distribution bar chart
    QBarSet *valueSet = new QBarSet("Total Value");
    valueSet->setColor(QColor("#27ae60"));
    
    for (const QString &category : categories) {
        *valueSet << categoryValues[category];
    }
      m_valueBarSeries->append(valueSet);
    
    // Update value chart axes
    if (m_valueDistributionChart && m_valueDistributionChart->chart() && 
        !m_valueDistributionChart->chart()->axes(Qt::Horizontal).isEmpty()) {
        if (QBarCategoryAxis *axisX = qobject_cast<QBarCategoryAxis*>(m_valueDistributionChart->chart()->axes(Qt::Horizontal).first())) {
            axisX->setCategories(categories);
        }
    }
    
    qDebug() << "updateCharts: Chart update completed";
}

void MaterialWidget::setupReportsWidget()
{
    m_reportsWidget = new QWidget();
    
    // Create scroll area for reports
    QScrollArea *scrollArea = new QScrollArea(m_reportsWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Main reports layout
    QVBoxLayout *mainLayout = new QVBoxLayout(m_reportsWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);
    
    // Scrollable content widget
    QWidget *scrollableContent = new QWidget();
    QVBoxLayout *reportsLayout = new QVBoxLayout(scrollableContent);
    reportsLayout->setContentsMargins(20, 20, 20, 20);
    reportsLayout->setSpacing(20);
    
    // Reports header
    QLabel *reportsTitle = new QLabel("Materials Reports");
    reportsTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    reportsLayout->addWidget(reportsTitle);
    
    // Report generation section
    QWidget *reportGenerationWidget = new QWidget();
    QVBoxLayout *reportGenLayout = new QVBoxLayout(reportGenerationWidget);
    
    QLabel *reportGenTitle = new QLabel("Generate Reports");
    reportGenTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    reportGenLayout->addWidget(reportGenTitle);
    
    // Report types grid
    QWidget *reportTypesWidget = new QWidget();
    QGridLayout *reportTypesLayout = new QGridLayout(reportTypesWidget);
    reportTypesLayout->setSpacing(15);
    
    // Report type buttons
    QString reportButtonStyle = 
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 15px; "
        "border-radius: 8px; "
        "font-weight: bold; "
        "font-size: 14px; "
        "min-height: 40px; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "} "
        "QPushButton:pressed { "
        "background-color: #21618c; "
        "}";
    
    QPushButton *inventoryReportBtn = new QPushButton("📊 Inventory Summary Report");
    QPushButton *lowStockReportBtn = new QPushButton("⚠️ Low Stock Report");
    QPushButton *valueReportBtn = new QPushButton("💰 Inventory Value Report");
    QPushButton *categoryReportBtn = new QPushButton("📁 Category Analysis Report");
    QPushButton *supplierReportBtn = new QPushButton("🏭 Supplier Report");
    QPushButton *customReportBtn = new QPushButton("⚙️ Custom Report");
    
    inventoryReportBtn->setStyleSheet(reportButtonStyle);
    lowStockReportBtn->setStyleSheet(reportButtonStyle);
    valueReportBtn->setStyleSheet(reportButtonStyle);
    categoryReportBtn->setStyleSheet(reportButtonStyle);
    supplierReportBtn->setStyleSheet(reportButtonStyle);
    customReportBtn->setStyleSheet(reportButtonStyle);
    
    // Arrange in 2x3 grid
    reportTypesLayout->addWidget(inventoryReportBtn, 0, 0);
    reportTypesLayout->addWidget(lowStockReportBtn, 0, 1);
    reportTypesLayout->addWidget(valueReportBtn, 1, 0);
    reportTypesLayout->addWidget(categoryReportBtn, 1, 1);
    reportTypesLayout->addWidget(supplierReportBtn, 2, 0);
    reportTypesLayout->addWidget(customReportBtn, 2, 1);
    
    reportGenLayout->addWidget(reportTypesWidget);
    reportsLayout->addWidget(reportGenerationWidget);
    
    // Report filters section
    QWidget *filtersWidget = new QWidget();
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersWidget);
    
    QLabel *filtersTitle = new QLabel("Report Filters");
    filtersTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    filtersLayout->addWidget(filtersTitle);
    
    QWidget *filtersFormWidget = new QWidget();
    QGridLayout *filtersFormLayout = new QGridLayout(filtersFormWidget);
    filtersFormLayout->setSpacing(10);
    
    // Date range filter
    QLabel *dateRangeLabel = new QLabel("Date Range:");
    QComboBox *dateRangeCombo = new QComboBox();
    dateRangeCombo->addItems({"Last 7 days", "Last 30 days", "Last 3 months", "Last 6 months", "Last year", "All time", "Custom range"});
    
    // Category filter
    QLabel *categoryLabel = new QLabel("Category:");
    QComboBox *categoryCombo = new QComboBox();
    categoryCombo->addItem("All Categories");
    // Add categories from model
    if (m_model) {
        categoryCombo->addItems(m_model->getCategories());
    }
    
    // Stock level filter
    QLabel *stockLevelLabel = new QLabel("Stock Level:");
    QComboBox *stockLevelCombo = new QComboBox();
    stockLevelCombo->addItems({"All Items", "In Stock", "Low Stock", "Out of Stock"});
    
    // Price range filter
    QLabel *priceRangeLabel = new QLabel("Price Range:");
    QComboBox *priceRangeCombo = new QComboBox();
    priceRangeCombo->addItems({"All Prices", "$0 - $100", "$100 - $500", "$500 - $1000", "$1000+", "Custom range"});
    
    filtersFormLayout->addWidget(dateRangeLabel, 0, 0);
    filtersFormLayout->addWidget(dateRangeCombo, 0, 1);
    filtersFormLayout->addWidget(categoryLabel, 1, 0);
    filtersFormLayout->addWidget(categoryCombo, 1, 1);
    filtersFormLayout->addWidget(stockLevelLabel, 2, 0);
    filtersFormLayout->addWidget(stockLevelCombo, 2, 1);
    filtersFormLayout->addWidget(priceRangeLabel, 3, 0);
    filtersFormLayout->addWidget(priceRangeCombo, 3, 1);
    
    filtersLayout->addWidget(filtersFormWidget);
    reportsLayout->addWidget(filtersWidget);
    
    // Export options section
    QWidget *exportWidget = new QWidget();
    QVBoxLayout *exportLayout = new QVBoxLayout(exportWidget);
    
    QLabel *exportTitle = new QLabel("Export Options");
    exportTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    exportLayout->addWidget(exportTitle);
    
    QHBoxLayout *exportButtonsLayout = new QHBoxLayout();
    
    QPushButton *exportPdfBtn = new QPushButton("📄 Export to PDF");
    QPushButton *exportCsvBtn = new QPushButton("📊 Export to CSV");
    QPushButton *exportExcelBtn = new QPushButton("📈 Export to Excel");
    QPushButton *printBtn = new QPushButton("🖨️ Print Report");
    
    QString exportButtonStyle = 
        "QPushButton { "
        "background-color: #27ae60; "
        "color: white; "
        "border: none; "
        "padding: 10px 15px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "margin: 5px; "
        "} "
        "QPushButton:hover { "
        "background-color: #229954; "
        "} "
        "QPushButton:pressed { "
        "background-color: #1e8449; "
        "}";
    
    exportPdfBtn->setStyleSheet(exportButtonStyle);
    exportCsvBtn->setStyleSheet(exportButtonStyle);
    exportExcelBtn->setStyleSheet(exportButtonStyle);
    printBtn->setStyleSheet(exportButtonStyle);
    
    exportButtonsLayout->addWidget(exportPdfBtn);
    exportButtonsLayout->addWidget(exportCsvBtn);
    exportButtonsLayout->addWidget(exportExcelBtn);
    exportButtonsLayout->addWidget(printBtn);
    exportButtonsLayout->addStretch();
    
    exportLayout->addLayout(exportButtonsLayout);
    reportsLayout->addWidget(exportWidget);
    
    // Recent reports section
    QWidget *recentReportsWidget = new QWidget();
    QVBoxLayout *recentReportsLayout = new QVBoxLayout(recentReportsWidget);
    
    QLabel *recentReportsTitle = new QLabel("Recent Reports");
    recentReportsTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    recentReportsLayout->addWidget(recentReportsTitle);
    
    QListWidget *recentReportsList = new QListWidget();
    recentReportsList->setMaximumHeight(150);
    recentReportsList->setStyleSheet(
        "QListWidget { "
        "border: 1px solid #bdc3c7; "
        "border-radius: 5px; "
        "background-color: white; "
        "} "
        "QListWidget::item { "
        "padding: 8px; "
        "border-bottom: 1px solid #ecf0f1; "
        "} "
        "QListWidget::item:hover { "
        "background-color: #f8f9fa; "
        "}"
    );
    
    // Add some sample recent reports
    recentReportsList->addItem("📊 Inventory Summary - " + QDateTime::currentDateTime().addDays(-1).toString("yyyy-MM-dd hh:mm"));
    recentReportsList->addItem("⚠️ Low Stock Alert - " + QDateTime::currentDateTime().addDays(-3).toString("yyyy-MM-dd hh:mm"));
    recentReportsList->addItem("💰 Value Analysis - " + QDateTime::currentDateTime().addDays(-7).toString("yyyy-MM-dd hh:mm"));
    
    recentReportsLayout->addWidget(recentReportsList);
    reportsLayout->addWidget(recentReportsWidget);
    
    reportsLayout->addStretch();
    
    // Set the scrollable content to the scroll area
    scrollArea->setWidget(scrollableContent);
    
    // Connect report generation buttons
    connect(inventoryReportBtn, &QPushButton::clicked, this, [this]() {
        generateInventoryReport();
    });
    connect(lowStockReportBtn, &QPushButton::clicked, this, [this]() {
        generateLowStockReport();
    });
    connect(valueReportBtn, &QPushButton::clicked, this, [this]() {
        generateValueReport();
    });
    connect(categoryReportBtn, &QPushButton::clicked, this, [this]() {
        generateCategoryReport();
    });
    connect(supplierReportBtn, &QPushButton::clicked, this, [this]() {
        generateSupplierReport();
    });
    connect(customReportBtn, &QPushButton::clicked, this, [this]() {
        generateCustomReport();
    });
    
    // Connect export buttons
    connect(exportPdfBtn, &QPushButton::clicked, this, [this]() {
        exportReportToPDF();
    });
    connect(exportCsvBtn, &QPushButton::clicked, this, [this]() {
        exportReportToCSV();
    });
    connect(exportExcelBtn, &QPushButton::clicked, this, [this]() {
        exportReportToExcel();
    });    connect(printBtn, &QPushButton::clicked, this, [this]() {
        printReport();
    });
}

void MaterialWidget::setupSettingsWidget()
{
    m_settingsWidget = new QWidget();
    
    // Create scroll area for settings
    QScrollArea *scrollArea = new QScrollArea(m_settingsWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Main settings layout
    QVBoxLayout *mainLayout = new QVBoxLayout(m_settingsWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);
    
    // Scrollable content widget
    QWidget *scrollableContent = new QWidget();
    QVBoxLayout *settingsLayout = new QVBoxLayout(scrollableContent);
    settingsLayout->setContentsMargins(20, 20, 20, 20);
    settingsLayout->setSpacing(20);
      // Settings header
    QLabel *settingsTitle = new QLabel("Materials Management Settings");
    settingsTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    settingsLayout->addWidget(settingsTitle);
    
    // General Settings Section
    QGroupBox *generalGroup = new QGroupBox("General Settings");
    generalGroup->setStyleSheet(
        "QGroupBox { "
        "font-weight: bold; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 8px; "
        "margin: 10px 0px; "
        "padding-top: 10px; "
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 10px; "
        "padding: 0 5px 0 5px; "
        "color: #2c3e50; "
        "}"
    );
    
    QFormLayout *generalFormLayout = new QFormLayout(generalGroup);
    
    // Auto-refresh interval
    QSpinBox *refreshIntervalSpinBox = new QSpinBox();
    refreshIntervalSpinBox->setRange(1, 60);
    refreshIntervalSpinBox->setValue(5);
    refreshIntervalSpinBox->setSuffix(" minutes");
    generalFormLayout->addRow("Auto-refresh interval:", refreshIntervalSpinBox);
    
    // Default view
    QComboBox *defaultViewCombo = new QComboBox();
    defaultViewCombo->addItems({"Materials List", "Dashboard", "Suppliers"});
    generalFormLayout->addRow("Default view:", defaultViewCombo);
    
    settingsLayout->addWidget(generalGroup);
    
    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *saveBtn = new QPushButton("💾 Save Settings");
    QPushButton *resetBtn = new QPushButton("🔄 Reset to Defaults");
    
    QString buttonStyle = 
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 10px 15px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "margin: 5px; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "}";
    
    saveBtn->setStyleSheet(buttonStyle);
    resetBtn->setStyleSheet(buttonStyle.replace("#3498db", "#e74c3c").replace("#2980b9", "#c0392b"));
    
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(resetBtn);
    buttonLayout->addStretch();
    
    settingsLayout->addLayout(buttonLayout);
    settingsLayout->addStretch();
    
    // Connect settings buttons
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Settings", "Settings saved successfully!");
    });
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Settings", "Settings reset to defaults!");
    });
    
    // Set the scrollable content to the scroll area
    scrollArea->setWidget(scrollableContent);
}

// Report generation methods
void MaterialWidget::generateInventoryReport()
{
    QMessageBox::information(this, "Inventory Report", 
        "Generating comprehensive inventory summary report...\n\n"
        "This report will include:\n"
        "• Complete materials listing\n"
        "• Stock levels and values\n"
        "• Category breakdown\n"
        "• Supplier information\n\n"
        "Report generation functionality will be implemented based on your requirements.");
}

void MaterialWidget::generateLowStockReport()
{
    if (!m_model) {
        QMessageBox::warning(this, "Error", "No data model available.");
        return;
    }
    
    QString reportContent = "LOW STOCK ALERT REPORT\n";
    reportContent += "Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n\n";
    
    bool foundLowStock = false;
    int rowCount = m_model->rowCount();
    
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex nameIndex = m_model->index(row, 1);
        QModelIndex quantityIndex = m_model->index(row, 3);
        QModelIndex reorderIndex = m_model->index(row, 6);
        
        if (nameIndex.isValid() && quantityIndex.isValid() && reorderIndex.isValid()) {
            QString name = m_model->data(nameIndex).toString();
            int quantity = m_model->data(quantityIndex).toInt();
            int reorderPoint = m_model->data(reorderIndex).toInt();
            
            if (quantity <= reorderPoint) {
                foundLowStock = true;
                reportContent += QString("⚠️ %1: %2 units (Reorder at: %3)\n")
                    .arg(name).arg(quantity).arg(reorderPoint);
            }
        }
    }
    
    if (!foundLowStock) {
        reportContent += "✅ No items are currently at or below reorder points.\n";
    }
    
    // Display report in a dialog
    QDialog reportDialog(this);
    reportDialog.setWindowTitle("Low Stock Report");
    reportDialog.resize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&reportDialog);
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setPlainText(reportContent);
    textEdit->setReadOnly(true);
    
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &reportDialog, &QDialog::accept);
    
    layout->addWidget(textEdit);
    layout->addWidget(closeBtn);
    
    reportDialog.exec();
}

void MaterialWidget::generateValueReport()
{
    if (!m_model) {
        QMessageBox::warning(this, "Error", "No data model available.");
        return;
    }
    
    QString reportContent = "INVENTORY VALUE REPORT\n";
    reportContent += "Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n\n";
    
    double totalValue = 0.0;
    int totalItems = 0;
    QMap<QString, double> categoryValues;
    int rowCount = m_model->rowCount();
    
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex nameIndex = m_model->index(row, 1);
        QModelIndex categoryIndex = m_model->index(row, 2);
        QModelIndex quantityIndex = m_model->index(row, 3);
        QModelIndex priceIndex = m_model->index(row, 5);
        
        if (nameIndex.isValid() && categoryIndex.isValid() && 
            quantityIndex.isValid() && priceIndex.isValid()) {
            
            QString category = m_model->data(categoryIndex).toString();
            int quantity = m_model->data(quantityIndex).toInt();
            double price = m_model->data(priceIndex).toDouble();
            double itemValue = quantity * price;
            
            totalValue += itemValue;
            totalItems += quantity;
            categoryValues[category] += itemValue;
        }
    }
    
    reportContent += QString("Total Inventory Value: $%1\n").arg(totalValue, 0, 'f', 2);
    reportContent += QString("Total Items: %1\n").arg(totalItems);
    reportContent += QString("Average Item Value: $%1\n\n").arg(totalItems > 0 ? totalValue / totalItems : 0.0, 0, 'f', 2);
    
    reportContent += "VALUE BY CATEGORY:\n";
    for (auto it = categoryValues.begin(); it != categoryValues.end(); ++it) {
        double percentage = totalValue > 0 ? (it.value() / totalValue) * 100 : 0;
        reportContent += QString("• %1: $%2 (%3%)\n")
            .arg(it.key()).arg(it.value(), 0, 'f', 2).arg(percentage, 0, 'f', 1);
    }
    
    // Display report
    QDialog reportDialog(this);
    reportDialog.setWindowTitle("Inventory Value Report");
    reportDialog.resize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&reportDialog);
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setPlainText(reportContent);
    textEdit->setReadOnly(true);
    
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &reportDialog, &QDialog::accept);
    
    layout->addWidget(textEdit);
    layout->addWidget(closeBtn);
    
    reportDialog.exec();
}

void MaterialWidget::generateCategoryReport()
{
    if (!m_model) {
        QMessageBox::warning(this, "Error", "No data model available.");
        return;
    }
    
    QString reportContent = "CATEGORY ANALYSIS REPORT\n";
    reportContent += "Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n\n";
    
    QMap<QString, int> categoryCount;
    QMap<QString, double> categoryValue;
    int rowCount = m_model->rowCount();
    
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex categoryIndex = m_model->index(row, 2);
        QModelIndex quantityIndex = m_model->index(row, 3);
        QModelIndex priceIndex = m_model->index(row, 5);
        
        if (categoryIndex.isValid() && quantityIndex.isValid() && priceIndex.isValid()) {
            QString category = m_model->data(categoryIndex).toString();
            int quantity = m_model->data(quantityIndex).toInt();
            double price = m_model->data(priceIndex).toDouble();
            
            categoryCount[category] += quantity;
            categoryValue[category] += quantity * price;
        }
    }
    
    reportContent += QString("Total Categories: %1\n\n").arg(categoryCount.size());
    
    for (auto it = categoryCount.begin(); it != categoryCount.end(); ++it) {
        QString category = it.key();
        int count = it.value();
        double value = categoryValue[category];
        
        reportContent += QString("📁 %1:\n").arg(category);
        reportContent += QString("   Items: %1\n").arg(count);
        reportContent += QString("   Value: $%1\n\n").arg(value, 0, 'f', 2);
    }
    
    // Display report
    QDialog reportDialog(this);
    reportDialog.setWindowTitle("Category Analysis Report");
    reportDialog.resize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&reportDialog);
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setPlainText(reportContent);
    textEdit->setReadOnly(true);
    
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &reportDialog, &QDialog::accept);
    
    layout->addWidget(textEdit);
    layout->addWidget(closeBtn);
    
    reportDialog.exec();
}

void MaterialWidget::generateSupplierReport()
{
    QString reportContent = "SUPPLIER REPORT\n";
    reportContent += "Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n\n";
    
    if (m_supplierWidget) {
        QList<Supplier> suppliers = getActiveSuppliers();
        reportContent += QString("Total Active Suppliers: %1\n\n").arg(suppliers.size());
        
        for (const Supplier &supplier : suppliers) {
            reportContent += QString("🏭 %1\n").arg(supplier.name);
            reportContent += QString("   Contact: %1\n").arg(supplier.contactPerson);
            reportContent += QString("   Email: %1\n").arg(supplier.email);
            reportContent += QString("   Phone: %1\n").arg(supplier.phone);
            reportContent += QString("   City: %1\n\n").arg(supplier.city);
        }
    } else {
        reportContent += "No supplier data available.\n";
    }
    
    // Display report
    QDialog reportDialog(this);
    reportDialog.setWindowTitle("Supplier Report");
    reportDialog.resize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&reportDialog);
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setPlainText(reportContent);
    textEdit->setReadOnly(true);
    
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &reportDialog, &QDialog::accept);
    
    layout->addWidget(textEdit);
    layout->addWidget(closeBtn);
    
    reportDialog.exec();
}

void MaterialWidget::generateCustomReport()
{
    QMessageBox::information(this, "Custom Report", 
        "Custom Report Builder\n\n"
        "This feature will allow you to:\n"
        "• Select specific data fields\n"
        "• Apply custom filters\n"
        "• Choose output format\n"
        "• Schedule automated reports\n\n"
        "Please specify your custom report requirements for implementation.");
}

void MaterialWidget::exportReportToPDF()
{
    QMessageBox::information(this, "Export to PDF", 
        "PDF Export functionality will be implemented.\n\n"
        "This will generate professional PDF reports with:\n"
        "• Company branding\n"
        "• Charts and graphs\n"
        "• Formatted tables\n"
        "• Print-ready layout");
}

void MaterialWidget::exportReportToCSV()
{
    // Reuse existing CSV export functionality
    exportToCSV();
}

void MaterialWidget::exportReportToExcel()
{
    QMessageBox::information(this, "Export to Excel", 
        "Excel Export functionality will be implemented.\n\n"
        "This will create Excel files with:\n"
        "• Multiple worksheets\n"
        "• Formatted data tables\n"
        "• Charts and pivot tables\n"
        "• Formulas and calculations");
}

void MaterialWidget::printReport()
{
    QMessageBox::information(this, "Print Report", 
        "Print functionality will be implemented.\n\n"
        "This will provide:\n"
        "• Print preview\n"
        "• Page layout options\n"
        "• Multiple printer support\n"
        "• Print job management");
}

QList<Supplier> MaterialWidget::getActiveSuppliers() const
{
    if (m_supplierWidget) {
        return m_supplierWidget->getActiveSuppliers();
    }
    return QList<Supplier>();
}

QString MaterialWidget::getSupplierName(int supplierId) const
{
    if (m_supplierWidget) {
        return m_supplierWidget->getSupplierName(supplierId);
    }
    return QString();
}

// Right panel management methods
void MaterialWidget::toggleRightPanel()
{
    if (m_rightPanelVisible) {
        hideRightPanel();
    } else {
        showRightPanel();
    }
}

void MaterialWidget::showRightPanel()
{
    if (!m_rightPanelVisible && m_rightPanel && m_rightPanelAnimation) {
        // Show the panel first
        m_rightPanel->show();
        
        // Animate from 0 to 300 width
        m_rightPanelAnimation->setStartValue(0);
        m_rightPanelAnimation->setEndValue(300);
        m_rightPanelAnimation->start();
        m_rightPanelVisible = true;
        
        // Update toggle button icon
        if (m_togglePanelButton) {
            m_togglePanelButton->setText("⮜");
            m_togglePanelButton->setToolTip("Hide Material Details Panel");
        }
    }
}

void MaterialWidget::hideRightPanel()
{
    if (m_rightPanelVisible && m_rightPanel && m_rightPanelAnimation) {
        // Animate from 300 to 0 width
        m_rightPanelAnimation->setStartValue(300);
        m_rightPanelAnimation->setEndValue(0);        // Hide the panel when animation finishes
        connect(m_rightPanelAnimation, &QPropertyAnimation::finished, this, [this]() {
            if (!m_rightPanelVisible && m_rightPanel) {
                m_rightPanel->hide();
            }
        });
        
        m_rightPanelAnimation->start();
        m_rightPanelVisible = false;
        
        // Update toggle button icon
        if (m_togglePanelButton) {
            m_togglePanelButton->setText("⮞");
            m_togglePanelButton->setToolTip("Show Material Details Panel");
        }
    }
}

void MaterialWidget::setDatabaseManager(DatabaseManager *dbManager)
{
    if (m_model) {
        m_model->setDatabaseManager(dbManager);
        qDebug() << "Database manager connected to MaterialWidget";
    }
}


