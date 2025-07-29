#ifndef CONTRACTSTATISTICSWIDGET_H
#define CONTRACTSTATISTICSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>
#include <QTimer>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>

// Forward declarations for Qt Charts
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QCategoryAxis>

// Forward declarations
class ContractDatabaseManager;
class Contract;

/**
 * @brief The ContractStatisticsWidget class provides comprehensive analytics for contract management
 * 
 * This widget displays various statistics including:
 * - Total contracts and their status distribution
 * - Revenue analytics and trends
 * - Expiring contracts alerts
 * - Performance metrics and KPIs
 * - Interactive charts and visualizations
 */
class ContractStatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContractStatisticsWidget(QWidget *parent = nullptr);
    ~ContractStatisticsWidget();    // Data management
    void setDatabaseManager(ContractDatabaseManager *dbManager);
    void refreshStatistics();
    void setDateRange(const QDate &startDate, const QDate &endDate);

    // Statistics data structure
    struct ContractStatistics {
        // Basic counts
        int totalContracts = 0;
        int activeContracts = 0;
        int completedContracts = 0;
        int expiredContracts = 0;
        int draftContracts = 0;
        int pendingContracts = 0;
        
        // Financial metrics
        double totalValue = 0.0;
        double activeValue = 0.0;
        double completedValue = 0.0;
        double averageContractValue = 0.0;
        double monthlyRevenue = 0.0;
        double projectedRevenue = 0.0;
        
        // Time-based metrics
        int expiringIn30Days = 0;
        int expiringIn90Days = 0;
        double averageDuration = 0.0; // in days
        
        // Performance metrics
        double completionRate = 0.0; // percentage
        double renewalRate = 0.0; // percentage
        int overdueContracts = 0;
    };

public slots:
    void updateStatistics();
    void onContractAdded(const QString &contractId);
    void onContractUpdated(const QString &contractId);
    void onContractDeleted(const QString &contractId);
    void onDateRangeChanged();
    void onRefreshRequested();
    void exportReport();

signals:
    void statisticsUpdated();
    void errorOccurred(const QString &error);
    void reportExported(const QString &filePath);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onTimeRangeChanged();
    void onChartTypeChanged();
    void onExportClicked();
    void updateCharts();

private:
    void setupUi();
    void setupControlPanel();
    void setupStatisticsCards();
    void setupCharts();
    void setupConnections();
    void applyArchiFlowStyling();

    // Data operations
    ContractStatistics calculateStatistics();
    void loadStatisticsData();
    
    // UI helpers
    QWidget* createStatCard(const QString &title, const QString &value, 
                           const QString &subtitle, const QColor &color);
    QWidget* createKpiCard(const QString &title, double value, const QString &unit,
                          double target = 0.0, bool showProgress = false);
    void updateStatCard(QWidget *card, const QString &value, const QString &subtitle);
    void updateKpiCard(QWidget *card, double value, double target = 0.0);
    
    // Chart operations
    void createStatusDistributionChart();
    void createRevenueChart();
    void createTrendsChart();
    void createExpirationChart();
    void updateChartData();
    
    // Export operations
    void exportToPdf();
    void exportToExcel();
    void exportToImage();
    
    // Formatting helpers
    QString formatCurrency(double value) const;
    QString formatPercentage(double value) const;
    QString formatNumber(int value) const;
    QString formatDuration(double days) const;

    // UI Components
    QVBoxLayout *m_mainLayout;
    
    // Control panel
    QWidget *m_controlPanel;
    QComboBox *m_timeRangeCombo;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QComboBox *m_chartTypeCombo;
    QPushButton *m_refreshButton;
    QPushButton *m_exportButton;
    
    // Statistics cards section
    QWidget *m_statsCardsWidget;
    QGridLayout *m_statsCardsLayout;
    QWidget *m_totalContractsCard;
    QWidget *m_activeContractsCard;
    QWidget *m_totalValueCard;
    QWidget *m_averageValueCard;
    QWidget *m_completionRateCard;
    QWidget *m_expiringContractsCard;
    
    // KPI section
    QWidget *m_kpiWidget;
    QGridLayout *m_kpiLayout;
    QWidget *m_monthlyRevenueKpi;
    QWidget *m_renewalRateKpi;
    QWidget *m_avgDurationKpi;
    QWidget *m_overdueContractsKpi;
    
    // Charts section
    QWidget *m_chartsWidget;
    QHBoxLayout *m_chartsLayout;      // Chart views
    QChartView *m_statusChartView;
    QChartView *m_revenueChartView;
    QChartView *m_trendsChartView;
    QChartView *m_expirationChartView;
    
    // Chart objects
    QChart *m_statusChart;
    QChart *m_revenueChart;
    QChart *m_trendsChart;
    QChart *m_expirationChart;
    
    QPieSeries *m_statusPieSeries;
    QBarSeries *m_revenueBarSeries;
    QLineSeries *m_trendsLineSeries;
    QBarSeries *m_expirationBarSeries;
      // Data and state
    ContractDatabaseManager *m_dbManager;
    ContractStatistics m_currentStats;
    QDate m_startDate;
    QDate m_endDate;
    QTimer *m_refreshTimer;
    bool m_autoRefresh;
    QString m_currentTimeRange;
    QString m_currentChartType;
    
    // Constants
    static constexpr int REFRESH_INTERVAL = 30000; // 30 seconds
    static constexpr int CARD_MIN_WIDTH = 200;
    static constexpr int CARD_MIN_HEIGHT = 120;
    static constexpr int CHART_MIN_HEIGHT = 300;
};

#endif // CONTRACTSTATISTICSWIDGET_H
