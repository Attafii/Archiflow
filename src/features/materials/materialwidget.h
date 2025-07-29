#ifndef MATERIALWIDGET_H
#define MATERIALWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QGroupBox>
#include <QSplitter>
#include <QFrame>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <QTabWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include "suppliermodel.h"  // For Supplier struct

class MaterialModel;
struct Material;
class GroqClient;
class AIAssistantDialog;
class AIPredictionDialog;
class SupplierWidget;

/**
 * @brief The MaterialWidget class provides the main interface for materials management
 * 
 * Features:
 * - Modern dashboard with statistics cards
 * - Advanced search and filtering
 * - Data table with sorting and editing
 * - Action buttons for CRUD operations
 * - Real-time updates and animations
 */
class MaterialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MaterialWidget(QWidget *parent = nullptr);
    
    // Public interface
    void refreshData();
    void selectMaterial(int materialId);
      // Supplier access methods
    QList<Supplier> getActiveSuppliers() const;
    QString getSupplierName(int supplierId) const;
    
    // Database integration
    void setDatabaseManager(class DatabaseManager *dbManager);

public slots:
    void importFromCSV();
    void exportToCSV();
    void showMaterialDetails();
    void openAIAssistant();
    void openAIPrediction();    void saveMaterialFromDetail();
    void clearDetailForm();
    void populateDetailForm(const Material &material);
    void resetDatabase();    void showDashboard();
    void showMaterialsList();
    void showSuppliers();
    void showReports();
    void showSettings();

private slots:
    void onSearchTextChanged(const QString &text);
    void onCategoryFilterChanged(const QString &category);
    void onTableSelectionChanged();
    void addMaterial();
    void editMaterial();
    void deleteMaterial();
    void addMaterialFromDetail();
    void updateDashboardStats();
    void onDashboardCardClicked();
    
    // Report generation slots
    void generateInventoryReport();
    void generateLowStockReport();
    void generateValueReport();
    void generateCategoryReport();
    void generateSupplierReport();
    void generateCustomReport();
    void exportReportToPDF();
    void exportReportToCSV();
    void exportReportToExcel();
    void printReport();

    // Right panel management slots
    void toggleRightPanel();
    void showRightPanel();
    void hideRightPanel();

private:
    void setupUI();
    void setupFilters();
    void setupTable();
    void setupActions();    void setupConnections();
    void setupMaterialDetailsForm(QWidget *parent, QVBoxLayout *layout);    void setupDashboard();
    void setupReportsWidget();
    void setupSettingsWidget();
    void createDashboardCard(const QString &title, const QString &value, const QString &subtitle, QGridLayout *layout, int row, int col);
    void updateRecentActivity();
    QWidget* createStatCard(const QString &title, const QString &value, const QString &color);
    void updateStatCard(QWidget *card, const QString &newValue);
    
    // Chart methods
    void setupCharts();
    void updateCharts();
    QChartView* createCategoryPieChart();
    QChartView* createStockLevelsBarChart();
    QChartView* createValueDistributionChart();
    
    // AI Assistant methods
    void initializeAIAssistant();
    void showAISetupDialog();
      // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Tab widgets
    QWidget *m_materialsTab;
    QWidget *m_suppliersTab;
    
    // Filters section
    QWidget *m_filtersWidget;
    QLineEdit *m_searchEdit;
    QComboBox *m_categoryFilter;
    QPushButton *m_clearFiltersButton;    // Table section
    QWidget *m_tableWidget;
    QVBoxLayout *m_tableLayout;
    QTableView *m_tableView;
    
    // Data
    MaterialModel *m_model;
    QSortFilterProxyModel *m_proxyModel;
    
    // Actions section
    QWidget *m_actionsWidget;
    QHBoxLayout *m_actionsLayout;
    QPushButton *m_importButton;
    QPushButton *m_exportButton;
    QPushButton *m_resetDbButton;
    QPushButton *m_scanButton;
    QPushButton *m_aiAssistantButton;
    QPushButton *m_aiPredictionsButton;
    
    // CRUD buttons
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_viewButton;
    
    // Detail panel controls
    QLineEdit *m_detailIdEdit;
    QLineEdit *m_detailNameEdit;
    QTextEdit *m_detailDescEdit;
    QComboBox *m_detailCategoryCombo;
    QSpinBox *m_detailQuantitySpinBox;
    QComboBox *m_detailUnitCombo;    QDoubleSpinBox *m_detailPriceSpinBox;
    QPushButton *m_detailNewBtn;
    QPushButton *m_detailSaveBtn;
    QPushButton *m_detailDeleteBtn;    // Dashboard components
    QWidget *m_dashboardWidget;
    QWidget *m_materialsListWidget;
    QWidget *m_reportsWidget;
    QWidget *m_settingsWidget;
    QWidget *m_centerContent;
    QPushButton *m_dashboardBtn;
    QPushButton *m_materialsBtn;
    QPushButton *m_suppliersBtn;
    QPushButton *m_reportsBtn;
    QPushButton *m_settingsBtn;
    
    QWidget *m_totalMaterialsCard;
    QWidget *m_totalValueCard;
    QWidget *m_lowStockCard;
    QWidget *m_categoriesCard;
    QWidget *m_avgPriceCard;
    QWidget *m_mostStockedCard;
    QWidget *m_leastStockedCard;
    QWidget *m_totalCostCard;
    QListWidget *m_recentActivityList;
    QWidget *m_recentActivityWidget;
    QWidget *m_quickStatsWidget;
    
    // Charts
    QChartView *m_categoryPieChart;
    QChartView *m_stockLevelsBarChart;
    QChartView *m_valueDistributionChart;
    QPieSeries *m_categoryPieSeries;
    QBarSeries *m_stockBarSeries;
    QBarSeries *m_valueBarSeries;
    
    // AI Assistant
    GroqClient *m_groqClient;
    AIAssistantDialog *m_aiDialog;
    AIPredictionDialog *m_aiPredictionDialog;
    
    // Supplier management
    SupplierWidget *m_supplierWidget;
    
    // Right panel management
    QWidget *m_rightPanel;
    QPushButton *m_togglePanelButton;
    QPropertyAnimation *m_rightPanelAnimation;
    bool m_rightPanelVisible;
};

#endif // MATERIALWIDGET_H
