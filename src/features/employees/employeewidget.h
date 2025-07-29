#ifndef EMPLOYEEWIDGET_H
#define EMPLOYEEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QSplitter>
#include <QGroupBox>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QProgressBar>
#include <QDateEdit>
#include <QCalendarWidget>

class Employee;
class EmployeeDatabaseManager;
class EmployeeDialog;
class MapboxHandler;

/**
 * @brief The EmployeeWidget class provides the main interface for employee management
 * 
 * This widget displays employees and provides comprehensive employee
 * management functionality including calendar integration and dashboard analytics.
 */
class EmployeeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeWidget(QWidget *parent = nullptr);
    ~EmployeeWidget();

    // Public interface
    void refreshEmployees();
    void clearSelection();
    void setDatabaseManager(EmployeeDatabaseManager *dbManager);

public slots:
    void addEmployee();
    void editEmployee();
    void deleteEmployee();
    void viewEmployee();
    void exportEmployees();
    void importEmployees();
    void showAllEmployeesOnMap();
    void showSelectedEmployeeOnMap();

private slots:
    // Search and filtering
    void onSearchTextChanged();
    void onDepartmentFilterChanged();
    void onPositionFilterChanged();
    void onStatusFilterChanged();
    void onEmploymentTypeFilterChanged();
    void performSearch();
    
    // Table interactions
    void onEmployeeSelectionChanged();
    void onEmployeeDoubleClicked(int row, int column);
    void onContextMenuRequested(const QPoint &pos);
    
    // Dashboard updates
    void updateDashboard();
    void updateStatistics();
    void updateRecentActivity();
    
    // Calendar functionality
    void onCalendarDateChanged(const QDate &date);
    void showCalendarEvents();
    void updateCalendarView();
    void addCalendarEvent();
    void editCalendarEvent();
    
    // Map updates
    void onShowAllLocations();
    void onRefreshMap();
    
    // Tab management
    void onTabChanged(int index);

signals:
    void employeeSelectionChanged(const Employee &employee);
    void employeeModified();

private:
    void setupUI();
    void setupConnections();
    void setupContextMenu();
    void setupDashboardTab();
    void setupEmployeeTab();
    void setupCalendarTab();
    void setupMapTab();
    void applyTheme();
    
    // Data management
    void loadEmployees();
    void populateEmployeeTable();
    void updateEmployeeFilters();
    void clearEmployeeTable();
      // Statistics helpers
    void calculateEmployeeStats();
    void updateDepartmentDistribution();
    void updateEmploymentTypeDistribution();
    void updateEmployeeDetails();
    QList<Employee*> getSelectedEmployees() const;
    
    // UI Components - Main Layout
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Dashboard Tab
    QWidget *m_dashboardTab;
    QVBoxLayout *m_dashboardLayout;
    QSplitter *m_dashboardSplitter;
    
    // Statistics widgets
    QGroupBox *m_statsGroup;
    QLabel *m_totalEmployeesLabel;
    QLabel *m_activeDepartmentsLabel;
    QLabel *m_avgSalaryLabel;
    QLabel *m_employmentDistributionLabel;
    
    // Recent activity
    QGroupBox *m_recentActivityGroup;
    QListWidget *m_recentActivityList;
    
    // Department distribution
    QGroupBox *m_departmentDistGroup;
    QTextEdit *m_departmentDistText;
    
    // Employee Tab
    QWidget *m_employeeTab;
    QVBoxLayout *m_employeeLayout;
    QSplitter *m_employeeSplitter;
    
    // Search and filters
    QGroupBox *m_searchGroup;
    QLineEdit *m_searchEdit;
    QComboBox *m_departmentFilter;
    QComboBox *m_positionFilter;
    QComboBox *m_statusFilter;
    QComboBox *m_employmentTypeFilter;
    QPushButton *m_clearFiltersButton;
    
    // Employee table
    QTableWidget *m_employeeTable;
    
    // Action buttons
    QGroupBox *m_actionsGroup;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_viewButton;
    QPushButton *m_exportButton;
    QPushButton *m_importButton;
    QPushButton *m_refreshButton;
    
    // Employee details panel
    QGroupBox *m_detailsGroup;
    QTextEdit *m_detailsText;
    
    // Calendar Tab
    QWidget *m_calendarTab;
    QVBoxLayout *m_calendarLayout;
    QSplitter *m_calendarSplitter;
    
    // Calendar widgets
    QCalendarWidget *m_calendar;
    QGroupBox *m_eventsGroup;
    QListWidget *m_eventsList;
    QPushButton *m_addEventButton;
    QPushButton *m_editEventButton;
    QPushButton *m_deleteEventButton;
    
    // Date filter
    QGroupBox *m_dateGroup;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QPushButton *m_filterByDateButton;
    
    // Map Tab
    QWidget *m_mapTab;
    QVBoxLayout *m_mapLayout;
    QWidget *m_mapWidget;
    QPushButton *m_showAllLocationsBtn;
    QPushButton *m_refreshMapBtn;
    
    // Data members
    EmployeeDatabaseManager *m_dbManager;
    MapboxHandler *m_mapHandler;
    QList<Employee*> m_employees;
    QList<Employee*> m_filteredEmployees;
    Employee *m_selectedEmployee;
    
    // Search timer
    QTimer *m_searchTimer;
    bool m_isLoading;
    int m_currentTab;
    
    // Context menu
    QMenu *m_contextMenu;
    QAction *m_editAction;
    QAction *m_deleteAction;
    QAction *m_viewAction;
    QAction *m_exportAction;
    
    // Statistics data
    int m_totalEmployees;
    int m_activeDepartments;
    double m_averageSalary;
    QMap<QString, int> m_departmentCounts;
    QMap<QString, int> m_employmentTypeCounts;
};

#endif // EMPLOYEEWIDGET_H
