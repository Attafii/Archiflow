#include "employeewidget.h"
#include "employee.h"
#include "employeedatabasemanager.h"
#include "employeedialog.h"
#include "../../utils/mapboxhandler.h"
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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDateTime>

EmployeeWidget::EmployeeWidget(QWidget *parent)
    : QWidget(parent)
    , m_dbManager(nullptr)
    , m_mapHandler(nullptr)
    , m_selectedEmployee(nullptr)
    , m_searchTimer(new QTimer(this))
    , m_isLoading(false)
    , m_currentTab(0)
    , m_totalEmployees(0)
    , m_activeDepartments(0)
    , m_averageSalary(0.0)
{
    qDebug() << "EmployeeWidget: Initializing...";
    
    // Initialize database manager
    m_dbManager = new EmployeeDatabaseManager(this);
    if (!m_dbManager->initialize()) {
        qWarning() << "Failed to initialize employee database:" << m_dbManager->lastError();
    }
    
    // Setup search timer
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300);
    
    setupUI();
    setupConnections();
    
    // Load initial data
    refreshEmployees();
    updateDashboard();
    
    qDebug() << "EmployeeWidget: Initialization complete";
}

EmployeeWidget::~EmployeeWidget()
{
    // Clear existing data
    for (Employee* employee : m_employees) {
        employee->deleteLater();
    }
    m_employees.clear();
    m_filteredEmployees.clear();
}

void EmployeeWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    
    setupDashboardTab();
    setupEmployeeTab();
    setupCalendarTab();
    setupMapTab();
    
    m_mainLayout->addWidget(m_tabWidget);
    
    applyTheme();
}

void EmployeeWidget::setupDashboardTab()
{
    m_dashboardTab = new QWidget();
    m_dashboardLayout = new QVBoxLayout(m_dashboardTab);
    
    // Create main splitter
    m_dashboardSplitter = new QSplitter(Qt::Horizontal, m_dashboardTab);
    
    // Left panel - Statistics
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    // Statistics group
    m_statsGroup = new QGroupBox("Employee Statistics", leftPanel);
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    
    m_totalEmployeesLabel = new QLabel("Total Employees: 0");
    m_activeDepartmentsLabel = new QLabel("Active Departments: 0");
    m_avgSalaryLabel = new QLabel("Average Salary: $0.00");
    m_employmentDistributionLabel = new QLabel("Employment Types:");
    
    statsLayout->addWidget(m_totalEmployeesLabel);
    statsLayout->addWidget(m_activeDepartmentsLabel);
    statsLayout->addWidget(m_avgSalaryLabel);
    statsLayout->addWidget(m_employmentDistributionLabel);
    statsLayout->addStretch();
    
    // Department distribution
    m_departmentDistGroup = new QGroupBox("Department Distribution", leftPanel);
    QVBoxLayout *deptLayout = new QVBoxLayout(m_departmentDistGroup);
    
    m_departmentDistText = new QTextEdit();
    m_departmentDistText->setReadOnly(true);
    m_departmentDistText->setMaximumHeight(200);
    deptLayout->addWidget(m_departmentDistText);
    
    leftLayout->addWidget(m_statsGroup);
    leftLayout->addWidget(m_departmentDistGroup);
    leftLayout->addStretch();
    
    // Right panel - Recent Activity
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    m_recentActivityGroup = new QGroupBox("Recent Activity", rightPanel);
    QVBoxLayout *activityLayout = new QVBoxLayout(m_recentActivityGroup);
    
    m_recentActivityList = new QListWidget();
    activityLayout->addWidget(m_recentActivityList);
    
    rightLayout->addWidget(m_recentActivityGroup);
    
    // Add panels to splitter
    m_dashboardSplitter->addWidget(leftPanel);
    m_dashboardSplitter->addWidget(rightPanel);
    m_dashboardSplitter->setSizes({300, 400});
    
    m_dashboardLayout->addWidget(m_dashboardSplitter);
    
    m_tabWidget->addTab(m_dashboardTab, "📊 Dashboard");
}

void EmployeeWidget::setupEmployeeTab()
{
    m_employeeTab = new QWidget();
    m_employeeLayout = new QVBoxLayout(m_employeeTab);
    
    // Create main splitter
    m_employeeSplitter = new QSplitter(Qt::Horizontal, m_employeeTab);
    
    // Left panel - Employee list and controls
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    // Search and filters group
    m_searchGroup = new QGroupBox("Search & Filters", leftPanel);
    QVBoxLayout *searchLayout = new QVBoxLayout(m_searchGroup);
    
    // Search bar
    QHBoxLayout *searchBarLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel("Search:");
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Search employees by name, email, department...");
    searchBarLayout->addWidget(searchLabel);
    searchBarLayout->addWidget(m_searchEdit);
    
    // Filter controls
    QHBoxLayout *filtersLayout1 = new QHBoxLayout();
    QLabel *deptLabel = new QLabel("Department:");
    m_departmentFilter = new QComboBox();
    m_departmentFilter->addItem("All Departments", "");
    QLabel *posLabel = new QLabel("Position:");
    m_positionFilter = new QComboBox();
    m_positionFilter->addItem("All Positions", "");
    filtersLayout1->addWidget(deptLabel);
    filtersLayout1->addWidget(m_departmentFilter);
    filtersLayout1->addWidget(posLabel);
    filtersLayout1->addWidget(m_positionFilter);
    
    QHBoxLayout *filtersLayout2 = new QHBoxLayout();
    QLabel *statusLabel = new QLabel("Status:");
    m_statusFilter = new QComboBox();
    m_statusFilter->addItem("All Statuses", "");
    m_statusFilter->addItem("Active", "Active");
    m_statusFilter->addItem("Inactive", "Inactive");
    m_statusFilter->addItem("On Leave", "On Leave");
    QLabel *typeLabel = new QLabel("Type:");
    m_employmentTypeFilter = new QComboBox();
    m_employmentTypeFilter->addItem("All Types", "");
    m_employmentTypeFilter->addItem("Full-time", "Full-time");
    m_employmentTypeFilter->addItem("Part-time", "Part-time");
    m_employmentTypeFilter->addItem("Contract", "Contract");
    m_employmentTypeFilter->addItem("Temporary", "Temporary");
    m_clearFiltersButton = new QPushButton("Clear Filters");
    filtersLayout2->addWidget(statusLabel);
    filtersLayout2->addWidget(m_statusFilter);
    filtersLayout2->addWidget(typeLabel);
    filtersLayout2->addWidget(m_employmentTypeFilter);
    filtersLayout2->addWidget(m_clearFiltersButton);
    filtersLayout2->addStretch();
    
    searchLayout->addLayout(searchBarLayout);
    searchLayout->addLayout(filtersLayout1);
    searchLayout->addLayout(filtersLayout2);
    
    // Employee table
    m_employeeTable = new QTableWidget();
    m_employeeTable->setColumnCount(8);
    QStringList headers = {"Name", "Department", "Position", "Email", "Phone", "Status", "Employment Type", "Start Date"};
    m_employeeTable->setHorizontalHeaderLabels(headers);
    m_employeeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_employeeTable->setAlternatingRowColors(true);
    m_employeeTable->setSortingEnabled(true);
    m_employeeTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Resize table columns
    QHeaderView *header = m_employeeTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(0, 150); // Name
    header->resizeSection(1, 120); // Department
    header->resizeSection(2, 120); // Position
    header->resizeSection(3, 180); // Email
    header->resizeSection(4, 120); // Phone
    header->resizeSection(5, 80);  // Status
    header->resizeSection(6, 120); // Employment Type
    
    // Action buttons group
    m_actionsGroup = new QGroupBox("Actions", leftPanel);
    QGridLayout *actionsLayout = new QGridLayout(m_actionsGroup);
    
    m_addButton = new QPushButton("➕ Add Employee");
    m_editButton = new QPushButton("✏️ Edit Employee");
    m_deleteButton = new QPushButton("🗑️ Delete Employee");
    m_viewButton = new QPushButton("👁️ View Details");
    m_exportButton = new QPushButton("📤 Export");
    m_importButton = new QPushButton("📥 Import");
    m_refreshButton = new QPushButton("🔄 Refresh");
    
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_viewButton->setEnabled(false);
    
    actionsLayout->addWidget(m_addButton, 0, 0);
    actionsLayout->addWidget(m_editButton, 0, 1);
    actionsLayout->addWidget(m_deleteButton, 0, 2);
    actionsLayout->addWidget(m_viewButton, 1, 0);
    actionsLayout->addWidget(m_exportButton, 1, 1);
    actionsLayout->addWidget(m_importButton, 1, 2);
    actionsLayout->addWidget(m_refreshButton, 2, 0, 1, 3);
    
    leftLayout->addWidget(m_searchGroup);
    leftLayout->addWidget(m_employeeTable, 1);
    leftLayout->addWidget(m_actionsGroup);
    
    // Right panel - Employee details
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    m_detailsGroup = new QGroupBox("Employee Details", rightPanel);
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsGroup);
    
    m_detailsText = new QTextEdit();
    m_detailsText->setReadOnly(true);
    m_detailsText->setPlaceholderText("Select an employee to view details...");
    detailsLayout->addWidget(m_detailsText);
    
    rightLayout->addWidget(m_detailsGroup);
    
    // Add panels to splitter
    m_employeeSplitter->addWidget(leftPanel);
    m_employeeSplitter->addWidget(rightPanel);
    m_employeeSplitter->setSizes({600, 300});
    
    m_employeeLayout->addWidget(m_employeeSplitter);
    
    // Setup context menu
    setupContextMenu();
    
    m_tabWidget->addTab(m_employeeTab, "👥 Employees");
}

void EmployeeWidget::setupCalendarTab()
{
    m_calendarTab = new QWidget();
    m_calendarLayout = new QVBoxLayout(m_calendarTab);
    
    // Create main splitter
    m_calendarSplitter = new QSplitter(Qt::Horizontal, m_calendarTab);
    
    // Left panel - Calendar
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    // Date filter group
    m_dateGroup = new QGroupBox("Date Range", leftPanel);
    QHBoxLayout *dateLayout = new QHBoxLayout(m_dateGroup);
    
    QLabel *startLabel = new QLabel("From:");
    m_startDateEdit = new QDateEdit(QDate::currentDate().addDays(-30));
    m_startDateEdit->setCalendarPopup(true);
    QLabel *endLabel = new QLabel("To:");
    m_endDateEdit = new QDateEdit(QDate::currentDate().addDays(30));
    m_endDateEdit->setCalendarPopup(true);
    m_filterByDateButton = new QPushButton("Filter");
    
    dateLayout->addWidget(startLabel);
    dateLayout->addWidget(m_startDateEdit);
    dateLayout->addWidget(endLabel);
    dateLayout->addWidget(m_endDateEdit);
    dateLayout->addWidget(m_filterByDateButton);
    dateLayout->addStretch();
    
    // Calendar widget
    m_calendar = new QCalendarWidget();
    m_calendar->setGridVisible(true);
    m_calendar->setNavigationBarVisible(true);
    
    leftLayout->addWidget(m_dateGroup);
    leftLayout->addWidget(m_calendar);
    
    // Right panel - Events
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    m_eventsGroup = new QGroupBox("Employee Events", rightPanel);
    QVBoxLayout *eventsLayout = new QVBoxLayout(m_eventsGroup);
    
    m_eventsList = new QListWidget();
    m_eventsList->setAlternatingRowColors(true);
    eventsLayout->addWidget(m_eventsList);
    
    // Event action buttons
    QHBoxLayout *eventButtonsLayout = new QHBoxLayout();
    m_addEventButton = new QPushButton("➕ Add Event");
    m_editEventButton = new QPushButton("✏️ Edit Event");
    m_deleteEventButton = new QPushButton("🗑️ Delete Event");
    
    m_editEventButton->setEnabled(false);
    m_deleteEventButton->setEnabled(false);
    
    eventButtonsLayout->addWidget(m_addEventButton);
    eventButtonsLayout->addWidget(m_editEventButton);
    eventButtonsLayout->addWidget(m_deleteEventButton);
    eventButtonsLayout->addStretch();
    
    eventsLayout->addLayout(eventButtonsLayout);
    
    rightLayout->addWidget(m_eventsGroup);
    
    // Add panels to splitter
    m_calendarSplitter->addWidget(leftPanel);
    m_calendarSplitter->addWidget(rightPanel);
    m_calendarSplitter->setSizes({400, 300});
    
    m_calendarLayout->addWidget(m_calendarSplitter);
    
    m_tabWidget->addTab(m_calendarTab, "📅 Calendar");
}

void EmployeeWidget::setupMapTab()
{
    m_mapTab = new QWidget();
    m_mapLayout = new QVBoxLayout(m_mapTab);
    
    // Map controls
    QHBoxLayout *mapControlsLayout = new QHBoxLayout();
    m_showAllLocationsBtn = new QPushButton("🌍 Show All Employee Locations");
    m_showAllLocationsBtn->setToolTip("Display all employees with addresses on the map");
    m_refreshMapBtn = new QPushButton("🔄 Refresh Map");
    m_refreshMapBtn->setToolTip("Refresh employee locations on the map");
    
    mapControlsLayout->addWidget(m_showAllLocationsBtn);
    mapControlsLayout->addWidget(m_refreshMapBtn);
    mapControlsLayout->addStretch();
    
    m_mapLayout->addLayout(mapControlsLayout);
    
    // Initialize MapboxHandler
    if (!m_mapHandler) {
        m_mapHandler = new MapboxHandler(this);
    }
    
    // Map widget placeholder
    m_mapWidget = m_mapHandler->createMapWidget();
    if (m_mapWidget) {
        m_mapLayout->addWidget(m_mapWidget);
    } else {
        // Fallback if map widget creation fails
        QLabel *mapPlaceholder = new QLabel("Map integration not available.\nPlease check your Mapbox API configuration.");
        mapPlaceholder->setAlignment(Qt::AlignCenter);
        mapPlaceholder->setStyleSheet("border: 1px dashed #ccc; padding: 20px; background-color: #f9f9f9;");
        m_mapLayout->addWidget(mapPlaceholder);
    }
    
    m_tabWidget->addTab(m_mapTab, "🗺️ Map");
}

void EmployeeWidget::setupConnections()
{
    // Search timer
    connect(m_searchTimer, &QTimer::timeout, this, &EmployeeWidget::performSearch);
    
    // Search and filters
    connect(m_searchEdit, &QLineEdit::textChanged, this, &EmployeeWidget::onSearchTextChanged);
    connect(m_departmentFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EmployeeWidget::onDepartmentFilterChanged);
    connect(m_positionFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EmployeeWidget::onPositionFilterChanged);
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EmployeeWidget::onStatusFilterChanged);
    connect(m_employmentTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EmployeeWidget::onEmploymentTypeFilterChanged);
    connect(m_clearFiltersButton, &QPushButton::clicked, this, [this]() {
        m_searchEdit->clear();
        m_departmentFilter->setCurrentIndex(0);
        m_positionFilter->setCurrentIndex(0);
        m_statusFilter->setCurrentIndex(0);
        m_employmentTypeFilter->setCurrentIndex(0);
        performSearch();
    });
    
    // Table interactions
    connect(m_employeeTable, &QTableWidget::itemSelectionChanged,
            this, &EmployeeWidget::onEmployeeSelectionChanged);
    connect(m_employeeTable, &QTableWidget::cellDoubleClicked,
            this, &EmployeeWidget::onEmployeeDoubleClicked);
    connect(m_employeeTable, &QTableWidget::customContextMenuRequested,
            this, &EmployeeWidget::onContextMenuRequested);
    
    // Action buttons
    connect(m_addButton, &QPushButton::clicked, this, &EmployeeWidget::addEmployee);
    connect(m_editButton, &QPushButton::clicked, this, &EmployeeWidget::editEmployee);
    connect(m_deleteButton, &QPushButton::clicked, this, &EmployeeWidget::deleteEmployee);
    connect(m_viewButton, &QPushButton::clicked, this, &EmployeeWidget::viewEmployee);
    connect(m_exportButton, &QPushButton::clicked, this, &EmployeeWidget::exportEmployees);
    connect(m_importButton, &QPushButton::clicked, this, &EmployeeWidget::importEmployees);
    connect(m_refreshButton, &QPushButton::clicked, this, &EmployeeWidget::refreshEmployees);
    
    // Map connections
    connect(m_showAllLocationsBtn, &QPushButton::clicked, this, &EmployeeWidget::onShowAllLocations);
    connect(m_refreshMapBtn, &QPushButton::clicked, this, &EmployeeWidget::onRefreshMap);
    
    // Calendar connections
    connect(m_calendar, &QCalendarWidget::selectionChanged,
            this, [this]() { onCalendarDateChanged(m_calendar->selectedDate()); });
    connect(m_filterByDateButton, &QPushButton::clicked,
            this, &EmployeeWidget::showCalendarEvents);
    connect(m_addEventButton, &QPushButton::clicked,
            this, &EmployeeWidget::addCalendarEvent);
    connect(m_editEventButton, &QPushButton::clicked,
            this, &EmployeeWidget::editCalendarEvent);
    connect(m_deleteEventButton, &QPushButton::clicked, [this]() {
        // TODO: Implement event deletion
        QMessageBox::information(this, "Calendar Events", "Event deletion will be implemented in a future update.");
    });
      // Map connections
    connect(m_showAllLocationsBtn, &QPushButton::clicked, this, &EmployeeWidget::onShowAllLocations);
    connect(m_refreshMapBtn, &QPushButton::clicked, this, &EmployeeWidget::onRefreshMap);
    
    // Tab changes
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &EmployeeWidget::onTabChanged);
}

void EmployeeWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_editAction = m_contextMenu->addAction("✏️ Edit Employee");
    m_viewAction = m_contextMenu->addAction("👁️ View Details");
    m_contextMenu->addSeparator();
    QAction *showMapAction = m_contextMenu->addAction("🗺️ Show on Map");
    m_contextMenu->addSeparator();
    m_deleteAction = m_contextMenu->addAction("🗑️ Delete Employee");
    m_contextMenu->addSeparator();
    m_exportAction = m_contextMenu->addAction("📤 Export Selected");
    
    connect(m_editAction, &QAction::triggered, this, &EmployeeWidget::editEmployee);
    connect(m_viewAction, &QAction::triggered, this, &EmployeeWidget::viewEmployee);
    connect(showMapAction, &QAction::triggered, this, &EmployeeWidget::showSelectedEmployeeOnMap);
    connect(m_deleteAction, &QAction::triggered, this, &EmployeeWidget::deleteEmployee);
    connect(m_exportAction, &QAction::triggered, this, &EmployeeWidget::exportEmployees);
}

void EmployeeWidget::applyTheme()
{
    setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #c0c0c0;
            background-color: #f5f5f5;
        }
        QTabBar::tab {
            background-color: #e0e0e0;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #4a90e2;
            color: white;
        }
        QTableWidget {
            gridline-color: #d0d0d0;
            background-color: white;
            alternate-background-color: #f8f8f8;
        }
        QTableWidget::item:selected {
            background-color: #4a90e2;
            color: white;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
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
        QLineEdit, QComboBox {
            padding: 6px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        QLineEdit:focus, QComboBox:focus {
            border-color: #4a90e2;
        }
    )");
}

// Data management methods
void EmployeeWidget::refreshEmployees()
{
    if (!m_dbManager) {
        qWarning() << "Database manager not set";
        return;
    }
    
    m_isLoading = true;
    loadEmployees();
    updateEmployeeFilters();
    populateEmployeeTable();
    updateDashboard();
    m_isLoading = false;
}

void EmployeeWidget::loadEmployees()
{
    // Clear existing data
    for (Employee* employee : m_employees) {
        employee->deleteLater();
    }
    m_employees.clear();
    m_filteredEmployees.clear();
    
    // Load from database
    m_employees = m_dbManager->getAllEmployees();
    m_filteredEmployees = m_employees;
    
    qDebug() << "Loaded" << m_employees.size() << "employees";
}

void EmployeeWidget::populateEmployeeTable()
{
    m_employeeTable->setRowCount(0);
    
    for (int i = 0; i < m_filteredEmployees.size(); ++i) {
        const Employee* employee = m_filteredEmployees[i];
        
        m_employeeTable->insertRow(i);
        
        // Name
        m_employeeTable->setItem(i, 0, new QTableWidgetItem(
            employee->firstName() + " " + employee->lastName()));
        
        // Department
        m_employeeTable->setItem(i, 1, new QTableWidgetItem(employee->department()));
        
        // Position
        m_employeeTable->setItem(i, 2, new QTableWidgetItem(employee->position()));
        
        // Email
        m_employeeTable->setItem(i, 3, new QTableWidgetItem(employee->email()));
        
        // Phone
        m_employeeTable->setItem(i, 4, new QTableWidgetItem(employee->phone()));
          // Status
        m_employeeTable->setItem(i, 5, new QTableWidgetItem(employee->statusString()));
        
        // Employment Type
        m_employeeTable->setItem(i, 6, new QTableWidgetItem(employee->employmentType()));
        
        // Start Date
        m_employeeTable->setItem(i, 7, new QTableWidgetItem(
            employee->startDate().toString("yyyy-MM-dd")));
          // Store employee CIN in first column for reference
        m_employeeTable->item(i, 0)->setData(Qt::UserRole, employee->cin());
    }
    
    // Update table appearance
    m_employeeTable->resizeColumnsToContents();
    m_employeeTable->sortItems(0); // Sort by name
}

void EmployeeWidget::updateEmployeeFilters()
{
    // Clear and populate department filter
    QString currentDept = m_departmentFilter->currentData().toString();
    m_departmentFilter->clear();
    m_departmentFilter->addItem("All Departments", "");
    
    QSet<QString> departments;
    for (const Employee* employee : m_employees) {
        if (!employee->department().isEmpty()) {
            departments.insert(employee->department());
        }
    }
    
    for (const QString &dept : departments) {
        m_departmentFilter->addItem(dept, dept);
    }
    
    // Restore selection
    int deptIndex = m_departmentFilter->findData(currentDept);
    if (deptIndex >= 0) {
        m_departmentFilter->setCurrentIndex(deptIndex);
    }
    
    // Clear and populate position filter
    QString currentPos = m_positionFilter->currentData().toString();
    m_positionFilter->clear();
    m_positionFilter->addItem("All Positions", "");
    
    QSet<QString> positions;
    for (const Employee* employee : m_employees) {
        if (!employee->position().isEmpty()) {
            positions.insert(employee->position());
        }
    }
    
    for (const QString &pos : positions) {
        m_positionFilter->addItem(pos, pos);
    }
    
    // Restore selection
    int posIndex = m_positionFilter->findData(currentPos);
    if (posIndex >= 0) {
        m_positionFilter->setCurrentIndex(posIndex);
    }
}

void EmployeeWidget::clearSelection()
{
    m_employeeTable->clearSelection();
    m_selectedEmployee = nullptr;
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_viewButton->setEnabled(false);
    m_detailsText->clear();
    m_detailsText->setPlaceholderText("Select an employee to view details...");
}

void EmployeeWidget::setDatabaseManager(EmployeeDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
}

// CRUD operations
void EmployeeWidget::addEmployee()
{
    EmployeeDialog dialog(this);
    dialog.setWindowTitle("Add New Employee");
    
    if (dialog.exec() == QDialog::Accepted) {
        Employee* employee = dialog.employee();
        if (employee && m_dbManager->addEmployee(*employee)) {
            refreshEmployees();
            emit employeeModified();
            
            // Add to recent activity
            m_recentActivityList->insertItem(0, 
                QString("✅ Added employee: %1 %2")
                .arg(employee->firstName())
                .arg(employee->lastName()));
                
            QMessageBox::information(this, "Success", 
                "Employee added successfully!");
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to add employee: " + m_dbManager->lastError());
        }
    }
}

void EmployeeWidget::editEmployee()
{
    if (!m_selectedEmployee) return;
    
    EmployeeDialog dialog(m_selectedEmployee, this);
    dialog.setWindowTitle("Edit Employee");
    
    if (dialog.exec() == QDialog::Accepted) {
        Employee* updatedEmployee = dialog.employee();
        
        if (updatedEmployee && m_dbManager->updateEmployee(*updatedEmployee)) {
            refreshEmployees();
            emit employeeModified();
            
            // Add to recent activity
            m_recentActivityList->insertItem(0, 
                QString("✏️ Updated employee: %1 %2")
                .arg(updatedEmployee->firstName())
                .arg(updatedEmployee->lastName()));
                
            QMessageBox::information(this, "Success", 
                "Employee updated successfully!");
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to update employee: " + m_dbManager->lastError());
        }
    }
}

void EmployeeWidget::deleteEmployee()
{
    if (!m_selectedEmployee) return;
    
    int ret = QMessageBox::question(this, "Confirm Deletion",
        QString("Are you sure you want to delete employee '%1 %2'?\n\n"
                "This action cannot be undone.")
        .arg(m_selectedEmployee->firstName())
        .arg(m_selectedEmployee->lastName()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        QString employeeName = m_selectedEmployee->firstName() + " " + m_selectedEmployee->lastName();
        
        if (m_dbManager->deleteEmployee(m_selectedEmployee->cin())) {
            refreshEmployees();
            clearSelection();
            emit employeeModified();
            
            // Add to recent activity
            m_recentActivityList->insertItem(0, 
                QString("🗑️ Deleted employee: %1").arg(employeeName));
                
            QMessageBox::information(this, "Success", 
                "Employee deleted successfully!");
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to delete employee: " + m_dbManager->lastError());
        }
    }
}

void EmployeeWidget::viewEmployee()
{
    if (!m_selectedEmployee) return;
    
    EmployeeDialog dialog(m_selectedEmployee, this);
    dialog.setWindowTitle("View Employee Details");
    dialog.setReadOnly(true); // Make dialog read-only for viewing
    
    dialog.exec();
}

void EmployeeWidget::exportEmployees()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Export Employees", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/employees.json",
        "JSON Files (*.json)");
    
    if (fileName.isEmpty()) return;
    
    QJsonArray employeeArray;
    const QList<Employee*> &employeesToExport = m_employeeTable->selectedItems().isEmpty() 
        ? m_filteredEmployees : getSelectedEmployees();
    
    for (const Employee* employee : employeesToExport) {
        employeeArray.append(employee->toJson());
    }
    
    QJsonDocument doc(employeeArray);
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        QMessageBox::information(this, "Export Complete", 
            QString("Exported %1 employees to %2")
            .arg(employeesToExport.size())
            .arg(fileName));
    } else {
        QMessageBox::critical(this, "Export Failed", 
            "Could not write to file: " + fileName);
    }
}

void EmployeeWidget::importEmployees()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "Import Employees", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "JSON Files (*.json)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Import Failed", 
            "Could not read file: " + fileName);
        return;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, "Import Failed", 
            "Invalid JSON format: " + error.errorString());
        return;
    }
    
    QJsonArray employeeArray = doc.array();
    int imported = 0, skipped = 0;
    
    for (const QJsonValue &value : employeeArray) {
        Employee employee;
        if (employee.fromJson(value.toObject())) {
            if (m_dbManager->addEmployee(employee)) {
                imported++;
            } else {
                skipped++;
            }
        } else {
            skipped++;
        }
    }
    
    refreshEmployees();
    emit employeeModified();
    
    QMessageBox::information(this, "Import Complete", 
        QString("Imported: %1 employees\nSkipped: %2 employees")
        .arg(imported).arg(skipped));
}

QList<Employee*> EmployeeWidget::getSelectedEmployees() const
{
    QList<Employee*> selected;
    QList<QTableWidgetItem*> selectedItems = m_employeeTable->selectedItems();
    
    QSet<int> selectedRows;
    for (QTableWidgetItem* item : selectedItems) {
        selectedRows.insert(item->row());
    }
      for (int row : selectedRows) {
        QTableWidgetItem* nameItem = m_employeeTable->item(row, 0);
        if (nameItem) {
            QString employeeCin = nameItem->data(Qt::UserRole).toString();
            for (Employee* employee : m_filteredEmployees) {
                if (employee->cin() == employeeCin) {
                    selected.append(employee);
                    break;
                }
            }
        }
    }
    
    return selected;
}

// Search and filtering
void EmployeeWidget::onSearchTextChanged()
{
    if (!m_isLoading) {
        m_searchTimer->start();
    }
}

void EmployeeWidget::onDepartmentFilterChanged()
{
    if (!m_isLoading) {
        performSearch();
    }
}

void EmployeeWidget::onPositionFilterChanged()
{
    if (!m_isLoading) {
        performSearch();
    }
}

void EmployeeWidget::onStatusFilterChanged()
{
    if (!m_isLoading) {
        performSearch();
    }
}

void EmployeeWidget::onEmploymentTypeFilterChanged()
{
    if (!m_isLoading) {
        performSearch();
    }
}

void EmployeeWidget::performSearch()
{
    QString searchText = m_searchEdit->text().toLower();
    QString departmentFilter = m_departmentFilter->currentData().toString();
    QString positionFilter = m_positionFilter->currentData().toString();
    QString statusFilter = m_statusFilter->currentData().toString();
    QString typeFilter = m_employmentTypeFilter->currentData().toString();
    
    m_filteredEmployees.clear();
    
    for (Employee* employee : m_employees) {
        bool matches = true;
        
        // Text search
        if (!searchText.isEmpty()) {
            QString fullName = (employee->firstName() + " " + employee->lastName()).toLower();
            QString email = employee->email().toLower();
            QString department = employee->department().toLower();
            QString position = employee->position().toLower();
            
            matches = matches && (fullName.contains(searchText) ||
                                email.contains(searchText) ||
                                department.contains(searchText) ||
                                position.contains(searchText));
        }
        
        // Department filter
        if (!departmentFilter.isEmpty()) {
            matches = matches && (employee->department() == departmentFilter);
        }
        
        // Position filter
        if (!positionFilter.isEmpty()) {
            matches = matches && (employee->position() == positionFilter);
        }
          // Status filter
        if (!statusFilter.isEmpty()) {
            matches = matches && (employee->statusString() == statusFilter);
        }
        
        // Employment type filter
        if (!typeFilter.isEmpty()) {
            matches = matches && (employee->employmentType() == typeFilter);
        }
        
        if (matches) {
            m_filteredEmployees.append(employee);
        }
    }
    
    populateEmployeeTable();
    clearSelection();
}

// Table interactions
void EmployeeWidget::onEmployeeSelectionChanged()
{
    QList<QTableWidgetItem*> selectedItems = m_employeeTable->selectedItems();
    
    if (selectedItems.isEmpty()) {
        clearSelection();
        return;
    }
      // Get the employee CIN from the first column of the selected row
    int row = selectedItems.first()->row();
    QTableWidgetItem* nameItem = m_employeeTable->item(row, 0);
    
    if (!nameItem) {
        clearSelection();
        return;
    }
    
    QString employeeCin = nameItem->data(Qt::UserRole).toString();
    
    // Find the employee in our list
    m_selectedEmployee = nullptr;
    for (Employee* employee : m_filteredEmployees) {
        if (employee->cin() == employeeCin) {
            m_selectedEmployee = employee;
            break;
        }
    }
    
    if (m_selectedEmployee) {
        m_editButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
        m_viewButton->setEnabled(true);
        
        // Update details panel
        updateEmployeeDetails();
        
        emit employeeSelectionChanged(*m_selectedEmployee);
    } else {
        clearSelection();
    }
}

void EmployeeWidget::updateEmployeeDetails()
{
    if (!m_selectedEmployee) {
        m_detailsText->clear();
        return;
    }
      QString details = QString(
        "<h3>%1 %2</h3>"
        "<p><strong>Employee CIN:</strong> %3</p>"
        "<p><strong>Department:</strong> %4</p>"
        "<p><strong>Position:</strong> %5</p>"
        "<p><strong>Email:</strong> <a href=\"mailto:%6\">%6</a></p>"
        "<p><strong>Phone:</strong> %7</p>"
        "<p><strong>Status:</strong> %8</p>"
        "<p><strong>Employment Type:</strong> %9</p>"
        "<p><strong>Start Date:</strong> %10</p>"
        "<p><strong>Address:</strong> %11</p>"
        "<p><strong>Emergency Contact:</strong> %12</p>"
        "<p><strong>Emergency Phone:</strong> %13</p>"
    ).arg(m_selectedEmployee->firstName())
     .arg(m_selectedEmployee->lastName())
     .arg(m_selectedEmployee->cin())
     .arg(m_selectedEmployee->department())
     .arg(m_selectedEmployee->position())
     .arg(m_selectedEmployee->email())
     .arg(m_selectedEmployee->phoneNumber())
     .arg(m_selectedEmployee->statusString())
     .arg(m_selectedEmployee->employmentType())
     .arg(m_selectedEmployee->hireDate().toString("MMMM dd, yyyy"))
     .arg(m_selectedEmployee->address())
     .arg(m_selectedEmployee->emergencyContact())
     .arg(m_selectedEmployee->emergencyPhone());
    
    if (m_selectedEmployee->salary() > 0) {
        details += QString("<p><strong>Salary:</strong> $%1</p>")
                  .arg(m_selectedEmployee->salary(), 0, 'f', 2);
    }
    
    if (!m_selectedEmployee->notes().isEmpty()) {
        details += QString("<p><strong>Notes:</strong></p><p>%1</p>")
                  .arg(m_selectedEmployee->notes().replace("\n", "<br>"));
    }
    
    m_detailsText->setHtml(details);
}

void EmployeeWidget::onEmployeeDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    // Select the row and edit the employee
    m_employeeTable->selectRow(row);
    editEmployee();
}

void EmployeeWidget::onContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem* item = m_employeeTable->itemAt(pos);
    if (item) {
        m_employeeTable->selectRow(item->row());
        m_contextMenu->exec(m_employeeTable->mapToGlobal(pos));
    }
}

// Dashboard functionality
void EmployeeWidget::updateDashboard()
{
    calculateEmployeeStats();
    updateStatistics();
    updateDepartmentDistribution();
    updateRecentActivity();
}

void EmployeeWidget::calculateEmployeeStats()
{
    m_totalEmployees = m_employees.size();
    
    QSet<QString> departments;
    double totalSalary = 0.0;
    int salaryCount = 0;
    m_departmentCounts.clear();
    m_employmentTypeCounts.clear();
    
    for (const Employee* employee : m_employees) {
        // Departments
        if (!employee->department().isEmpty()) {
            departments.insert(employee->department());
            m_departmentCounts[employee->department()]++;
        }
        
        // Employment types
        if (!employee->employmentType().isEmpty()) {
            m_employmentTypeCounts[employee->employmentType()]++;
        }
        
        // Salary calculation
        if (employee->salary() > 0) {
            totalSalary += employee->salary();
            salaryCount++;
        }
    }
    
    m_activeDepartments = departments.size();
    m_averageSalary = salaryCount > 0 ? totalSalary / salaryCount : 0.0;
}

void EmployeeWidget::updateStatistics()
{
    m_totalEmployeesLabel->setText(QString("Total Employees: %1").arg(m_totalEmployees));
    m_activeDepartmentsLabel->setText(QString("Active Departments: %1").arg(m_activeDepartments));
    m_avgSalaryLabel->setText(QString("Average Salary: $%1").arg(m_averageSalary, 0, 'f', 2));
    
    // Employment type distribution
    QString empTypeText = "Employment Types:\n";
    for (auto it = m_employmentTypeCounts.begin(); it != m_employmentTypeCounts.end(); ++it) {
        empTypeText += QString("• %1: %2\n").arg(it.key()).arg(it.value());
    }
    m_employmentDistributionLabel->setText(empTypeText.trimmed());
}

void EmployeeWidget::updateDepartmentDistribution()
{
    QString distText;
    for (auto it = m_departmentCounts.begin(); it != m_departmentCounts.end(); ++it) {
        double percentage = m_totalEmployees > 0 ? (double(it.value()) / m_totalEmployees) * 100.0 : 0.0;
        distText += QString("%1: %2 (%3%)\n")
                   .arg(it.key())
                   .arg(it.value())
                   .arg(percentage, 0, 'f', 1);
    }
    m_departmentDistText->setPlainText(distText);
}

void EmployeeWidget::updateRecentActivity()
{
    // Keep only the last 10 items
    while (m_recentActivityList->count() > 10) {
        delete m_recentActivityList->takeItem(10);
    }
    
    // Add current timestamp for reference
    if (m_recentActivityList->count() == 0) {
        m_recentActivityList->addItem(QString("📊 Dashboard updated at %1")
                                     .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    }
}

// Calendar functionality
void EmployeeWidget::onCalendarDateChanged(const QDate &date)
{
    Q_UNUSED(date);
    showCalendarEvents();
}

void EmployeeWidget::showCalendarEvents()
{
    // Clear current events
    m_eventsList->clear();
    
    QDate selectedDate = m_calendar->selectedDate();
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();
      // Add sample events (in a real implementation, these would come from the database)
    for (const Employee* employee : m_employees) {
        // Add birthdays, work anniversaries, etc.
        if (employee->hireDate().date().month() == selectedDate.month() &&
            employee->hireDate().date().day() == selectedDate.day()) {
            
            int yearsOfService = employee->hireDate().date().daysTo(selectedDate) / 365;
            if (yearsOfService > 0) {
                m_eventsList->addItem(QString("🎉 %1 %2 - %3 years of service")
                                     .arg(employee->firstName())
                                     .arg(employee->lastName())
                                     .arg(yearsOfService));
            }
        }
    }
    
    // Add placeholder events
    if (m_eventsList->count() == 0) {
        m_eventsList->addItem("📅 No events scheduled for this date");
        m_eventsList->item(0)->setFlags(m_eventsList->item(0)->flags() & ~Qt::ItemIsSelectable);
    }
}

void EmployeeWidget::addCalendarEvent()
{
    // TODO: Implement event creation dialog
    QMessageBox::information(this, "Calendar Events", 
        "Event creation will be implemented in a future update.");
}

void EmployeeWidget::editCalendarEvent()
{
    // TODO: Implement event editing dialog
    QMessageBox::information(this, "Calendar Events", 
        "Event editing will be implemented in a future update.");
}

// Tab management
void EmployeeWidget::onTabChanged(int index)
{
    m_currentTab = index;
    
    if (index == 0) { // Dashboard tab
        updateDashboard();
    } else if (index == 2) { // Calendar tab
        showCalendarEvents();
    } else if (index == 3) { // Map tab
        onRefreshMap();
    }
}

// Map functionality
void EmployeeWidget::showAllEmployeesOnMap()
{
    if (!m_mapHandler) {
        m_mapHandler = new MapboxHandler(this);
    }
    
    int locationsAdded = 0;
    for (const Employee* employee : m_employees) {
        if (!employee->address().isEmpty()) {
            // For now, we'll show a simple message. In a full implementation,
            // we would geocode addresses and add markers
            locationsAdded++;
        }
    }
    
    QMessageBox::information(this, tr("Employee Locations"), 
                           tr("Found %1 employees with addresses.\nMap functionality will display their locations.").arg(locationsAdded));
}

void EmployeeWidget::showSelectedEmployeeOnMap()
{
    if (!m_selectedEmployee) {
        QMessageBox::warning(this, tr("No Selection"), 
                           tr("Please select an employee to show on the map."));
        return;
    }
    
    if (m_selectedEmployee->address().isEmpty()) {
        QMessageBox::warning(this, tr("No Address"), 
                           tr("The selected employee has no address information."));
        return;
    }
    
    if (!m_mapHandler) {
        m_mapHandler = new MapboxHandler(this);
    }
    
    // Switch to map tab
    m_tabWidget->setCurrentIndex(3); // Map tab index
    
    // Show employee location (in a full implementation, geocode and show)
    QMessageBox::information(this, tr("Employee Location"), 
                           tr("Showing location for %1\nAddress: %2")
                           .arg(m_selectedEmployee->fullName())
                           .arg(m_selectedEmployee->address()));
}

void EmployeeWidget::onShowAllLocations()
{
    showAllEmployeesOnMap();
}

void EmployeeWidget::onRefreshMap()
{
    if (!m_mapHandler) {
        m_mapHandler = new MapboxHandler(this);
    }
    
    // Count employees with addresses
    int employeesWithAddresses = 0;
    for (const Employee* employee : m_employees) {
        if (!employee->address().isEmpty()) {
            employeesWithAddresses++;
        }
    }
    
    // Update UI or show status
    m_showAllLocationsBtn->setText(QString("🌍 Show All Locations (%1)").arg(employeesWithAddresses));
}

void EmployeeWidget::updateCalendarView()
{
    // Refresh the calendar view when data changes
    showCalendarEvents();
}
