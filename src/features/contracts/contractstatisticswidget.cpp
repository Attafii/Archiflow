#include "contractstatisticswidget.h"
#include "contractdatabasemanager.h"
#include "contract.h"
#include "utils/stylemanager.h"
#include <QApplication>
#include <QSplitter>
#include <QGroupBox>
#include <QLocale>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QStandardPaths>
#include <QPainter>
#include <QPrinter>
#include <QTextDocument>
#include <QDateTime>
#include <QRandomGenerator>
#include <QTime>

// Qt Charts includes
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QCategoryAxis>

// Qt Charts classes are included directly

ContractStatisticsWidget::ContractStatisticsWidget(QWidget *parent)
    : QWidget(parent)
    , m_dbManager(nullptr)
    , m_refreshTimer(new QTimer(this))
    , m_autoRefresh(true)
    , m_currentTimeRange("All Time")
    , m_currentChartType("Status Distribution")
{
    setupUi();
    setupConnections();
    applyArchiFlowStyling();
    
    // Set default date range (last 12 months)
    m_endDate = QDate::currentDate();
    m_startDate = m_endDate.addMonths(-12);
    m_startDateEdit->setDate(m_startDate);
    m_endDateEdit->setDate(m_endDate);
    
    // Configure auto-refresh timer
    m_refreshTimer->setInterval(REFRESH_INTERVAL);
    connect(m_refreshTimer, &QTimer::timeout, this, &ContractStatisticsWidget::updateStatistics);
    if (m_autoRefresh) {
        m_refreshTimer->start();
    }
}

ContractStatisticsWidget::~ContractStatisticsWidget()
{
    // Charts are owned by chart views, so they'll be cleaned up automatically
}

void ContractStatisticsWidget::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    setupControlPanel();
    setupStatisticsCards();
    setupCharts();
}

void ContractStatisticsWidget::setupControlPanel()
{
    m_controlPanel = new QWidget;
    m_controlPanel->setObjectName("controlPanel");
    
    QHBoxLayout *controlLayout = new QHBoxLayout(m_controlPanel);
    controlLayout->setSpacing(15);
    
    // Time range selection
    QLabel *timeRangeLabel = new QLabel("Time Range:");
    m_timeRangeCombo = new QComboBox;
    m_timeRangeCombo->addItems({"Last 30 Days", "Last 90 Days", "Last 6 Months", 
                               "Last 12 Months", "This Year", "All Time", "Custom"});
    m_timeRangeCombo->setCurrentText("Last 12 Months");
    
    // Date range inputs (initially hidden)
    QLabel *startLabel = new QLabel("From:");
    m_startDateEdit = new QDateEdit;
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDate(QDate::currentDate().addMonths(-12));
    m_startDateEdit->setVisible(false);
    startLabel->setVisible(false);
    
    QLabel *endLabel = new QLabel("To:");
    m_endDateEdit = new QDateEdit;
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDate(QDate::currentDate());
    m_endDateEdit->setVisible(false);
    endLabel->setVisible(false);
    
    // Chart type selection
    QLabel *chartLabel = new QLabel("Chart Type:");
    m_chartTypeCombo = new QComboBox;
    m_chartTypeCombo->addItems({"Status Distribution", "Revenue Trends", 
                               "Expiration Timeline", "All Charts"});
    
    // Action buttons
    m_refreshButton = new QPushButton("Refresh");
    m_refreshButton->setIcon(QIcon(":/icons/refresh.png"));
    m_refreshButton->setObjectName("primaryButton");
    
    m_exportButton = new QPushButton("Export Report");
    m_exportButton->setIcon(QIcon(":/icons/export.png"));
    
    // Layout
    controlLayout->addWidget(timeRangeLabel);
    controlLayout->addWidget(m_timeRangeCombo);
    controlLayout->addWidget(startLabel);
    controlLayout->addWidget(m_startDateEdit);
    controlLayout->addWidget(endLabel);
    controlLayout->addWidget(m_endDateEdit);
    controlLayout->addStretch();
    controlLayout->addWidget(chartLabel);
    controlLayout->addWidget(m_chartTypeCombo);
    controlLayout->addStretch();
    controlLayout->addWidget(m_refreshButton);
    controlLayout->addWidget(m_exportButton);
    
    m_mainLayout->addWidget(m_controlPanel);
}

void ContractStatisticsWidget::setupStatisticsCards()
{
    // Statistics cards container
    QGroupBox *statsGroup = new QGroupBox("Key Metrics");
    statsGroup->setObjectName("statsGroup");
    
    m_statsCardsLayout = new QGridLayout(statsGroup);
    m_statsCardsLayout->setSpacing(15);
    
    // Create statistics cards
    m_totalContractsCard = createStatCard("Total Contracts", "0", "All contracts", QColor("#3498db"));
    m_activeContractsCard = createStatCard("Active Contracts", "0", "Currently active", QColor("#27ae60"));
    m_totalValueCard = createStatCard("Total Value", "$0", "Combined value", QColor("#e74c3c"));
    m_averageValueCard = createStatCard("Average Value", "$0", "Per contract", QColor("#9b59b6"));
    m_completionRateCard = createStatCard("Completion Rate", "0%", "Success rate", QColor("#f39c12"));
    m_expiringContractsCard = createStatCard("Expiring Soon", "0", "Next 30 days", QColor("#e67e22"));
    
    // Add cards to layout
    m_statsCardsLayout->addWidget(m_totalContractsCard, 0, 0);
    m_statsCardsLayout->addWidget(m_activeContractsCard, 0, 1);
    m_statsCardsLayout->addWidget(m_totalValueCard, 0, 2);
    m_statsCardsLayout->addWidget(m_averageValueCard, 1, 0);
    m_statsCardsLayout->addWidget(m_completionRateCard, 1, 1);
    m_statsCardsLayout->addWidget(m_expiringContractsCard, 1, 2);
    
    m_mainLayout->addWidget(statsGroup);
    
    // KPI section
    QGroupBox *kpiGroup = new QGroupBox("Performance Indicators");
    kpiGroup->setObjectName("kpiGroup");
    
    m_kpiLayout = new QGridLayout(kpiGroup);
    m_kpiLayout->setSpacing(15);
    
    // Create KPI cards
    m_monthlyRevenueKpi = createKpiCard("Monthly Revenue", 0.0, "$", 100000.0, true);
    m_renewalRateKpi = createKpiCard("Renewal Rate", 0.0, "%", 80.0, true);
    m_avgDurationKpi = createKpiCard("Avg Duration", 0.0, " days", 365.0, false);
    m_overdueContractsKpi = createKpiCard("Overdue", 0.0, "", 0.0, false);
    
    m_kpiLayout->addWidget(m_monthlyRevenueKpi, 0, 0);
    m_kpiLayout->addWidget(m_renewalRateKpi, 0, 1);
    m_kpiLayout->addWidget(m_avgDurationKpi, 0, 2);
    m_kpiLayout->addWidget(m_overdueContractsKpi, 0, 3);
    
    m_mainLayout->addWidget(kpiGroup);
}

void ContractStatisticsWidget::setupCharts()
{
    // Charts container
    QGroupBox *chartsGroup = new QGroupBox("Analytics Charts");
    chartsGroup->setObjectName("chartsGroup");
    
    m_chartsLayout = new QHBoxLayout(chartsGroup);
    m_chartsLayout->setSpacing(15);
    
    // Create charts
    createStatusDistributionChart();
    createRevenueChart();
    createTrendsChart();
    createExpirationChart();
    
    // Add chart views to layout
    QSplitter *chartSplitter = new QSplitter(Qt::Horizontal);
    chartSplitter->addWidget(m_statusChartView);
    chartSplitter->addWidget(m_revenueChartView);
    chartSplitter->addWidget(m_trendsChartView);
    chartSplitter->addWidget(m_expirationChartView);
    chartSplitter->setSizes({250, 250, 250, 250});
    
    m_chartsLayout->addWidget(chartSplitter);
    m_mainLayout->addWidget(chartsGroup);
}

void ContractStatisticsWidget::createStatusDistributionChart()
{    // Create pie chart for contract status distribution    m_statusChart = new QChart();
    m_statusChart->setTitle("Contract Status Distribution");
    m_statusChart->setAnimationOptions(QChart::SeriesAnimations);

    m_statusPieSeries = new QPieSeries();
    m_statusChart->addSeries(m_statusPieSeries);
    m_statusChart->legend()->setAlignment(Qt::AlignBottom);
    
    m_statusChartView = new QChartView(m_statusChart);
    m_statusChartView->setRenderHint(QPainter::Antialiasing);
    m_statusChartView->setMinimumHeight(CHART_MIN_HEIGHT);
}

void ContractStatisticsWidget::createRevenueChart()
{    // Create bar chart for revenue analysis    m_revenueChart = new QChart();
    m_revenueChart->setTitle("Revenue by Status");
    m_revenueChart->setAnimationOptions(QChart::SeriesAnimations);

    m_revenueBarSeries = new QBarSeries();
    m_revenueChart->addSeries(m_revenueBarSeries);
    
    // Setup axes
    QStringList categories;    categories << "Draft" << "Active" << "Completed" << "Expired";
      QCategoryAxis *axisX = new QCategoryAxis();
    axisX->setTitleText("Contract Status");
    for (int i = 0; i < categories.size(); ++i) {
        axisX->append(categories[i], i);
    }
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Revenue ($)");
    axisY->setLabelFormat("$%.0f");
    
    m_revenueChart->addAxis(axisX, Qt::AlignBottom);
    m_revenueChart->addAxis(axisY, Qt::AlignLeft);
    m_revenueBarSeries->attachAxis(axisX);
    m_revenueBarSeries->attachAxis(axisY);
    
    m_revenueChartView = new QChartView(m_revenueChart);
    m_revenueChartView->setRenderHint(QPainter::Antialiasing);
    m_revenueChartView->setMinimumHeight(CHART_MIN_HEIGHT);
}

void ContractStatisticsWidget::createTrendsChart()
{    // Create line chart for trends over time    m_trendsChart = new QChart();
    m_trendsChart->setTitle("Contract Trends");
    m_trendsChart->setAnimationOptions(QChart::SeriesAnimations);

    m_trendsLineSeries = new QLineSeries();
    m_trendsLineSeries->setName("Monthly Contracts");
    m_trendsChart->addSeries(m_trendsLineSeries);      // Setup axes
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setTitleText("Date");
    axisX->setFormat("MMM yyyy");
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Number of Contracts");
    
    m_trendsChart->addAxis(axisX, Qt::AlignBottom);
    m_trendsChart->addAxis(axisY, Qt::AlignLeft);
    m_trendsLineSeries->attachAxis(axisX);
    m_trendsLineSeries->attachAxis(axisY);
    
    m_trendsChartView = new QChartView(m_trendsChart);
    m_trendsChartView->setRenderHint(QPainter::Antialiasing);
    m_trendsChartView->setMinimumHeight(CHART_MIN_HEIGHT);
}

void ContractStatisticsWidget::createExpirationChart()
{    // Create bar chart for contract expirations
    m_expirationChart = new QChart();
    m_expirationChart->setTitle("Contract Expirations");
    m_expirationChart->setAnimationOptions(QChart::SeriesAnimations);
    
    m_expirationBarSeries = new QBarSeries();
    m_expirationChart->addSeries(m_expirationBarSeries);
    
    // Setup axes
    QStringList categories;
    categories << "Next 30 Days" << "31-90 Days" << "91-180 Days" << "Beyond 180 Days";
      QCategoryAxis *axisX = new QCategoryAxis();
    axisX->setTitleText("Time Period");
    for (int i = 0; i < categories.size(); ++i) {
        axisX->append(categories[i], i);
    }
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Number of Contracts");
    
    m_expirationChart->addAxis(axisX, Qt::AlignBottom);
    m_expirationChart->addAxis(axisY, Qt::AlignLeft);
    m_expirationBarSeries->attachAxis(axisX);
    m_expirationBarSeries->attachAxis(axisY);
    
    m_expirationChartView = new QChartView(m_expirationChart);
    m_expirationChartView->setRenderHint(QPainter::Antialiasing);
    m_expirationChartView->setMinimumHeight(CHART_MIN_HEIGHT);
}

void ContractStatisticsWidget::setupConnections()
{
    // Control panel connections
    connect(m_timeRangeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &ContractStatisticsWidget::onTimeRangeChanged);
    connect(m_startDateEdit, &QDateEdit::dateChanged,
            this, &ContractStatisticsWidget::onDateRangeChanged);
    connect(m_endDateEdit, &QDateEdit::dateChanged,
            this, &ContractStatisticsWidget::onDateRangeChanged);
    connect(m_chartTypeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &ContractStatisticsWidget::onChartTypeChanged);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &ContractStatisticsWidget::onRefreshRequested);
    connect(m_exportButton, &QPushButton::clicked,
            this, &ContractStatisticsWidget::onExportClicked);
}

void ContractStatisticsWidget::applyArchiFlowStyling()
{
    // Apply ArchiFlow color scheme
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            border: 2px solid #3D485A;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: white;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #3D485A;
        }
        
        #controlPanel {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 6px;
            padding: 10px;
        }
        
        #primaryButton {
            background-color: #3D485A;
            color: #E3C6B0;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        
        #primaryButton:hover {
            background-color: #2C3E50;
        }
        
        QPushButton {
            background-color: #E3C6B0;
            color: #3D485A;
            border: 1px solid #D4B7A1;
            padding: 6px 12px;
            border-radius: 4px;
        }
        
        QPushButton:hover {
            background-color: #D4B7A1;
        }
        
        QComboBox, QDateEdit {
            padding: 4px 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: white;
        }
    )");
}

QWidget* ContractStatisticsWidget::createStatCard(const QString &title, const QString &value, 
                                                  const QString &subtitle, const QColor &color)
{
    QFrame *card = new QFrame;
    card->setFrameStyle(QFrame::StyledPanel);
    card->setMinimumSize(CARD_MIN_WIDTH, CARD_MIN_HEIGHT);
    card->setMaximumHeight(CARD_MIN_HEIGHT);
    
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(5);
    layout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 12px;").arg(color.name()));
    
    // Value
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setObjectName("valueLabel");
    valueLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    valueLabel->setAlignment(Qt::AlignCenter);
    
    // Subtitle
    QLabel *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setStyleSheet("color: #7f8c8d; font-size: 11px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    layout->addWidget(subtitleLabel);
    layout->addStretch();
    
    // Add colored border
    card->setStyleSheet(QString(R"(
        QFrame {
            background-color: white;
            border: 1px solid #ecf0f1;
            border-radius: 8px;
            border-left: 4px solid %1;
        }
        QFrame:hover {
            border: 1px solid %1;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
    )").arg(color.name()));
    
    return card;
}

QWidget* ContractStatisticsWidget::createKpiCard(const QString &title, double value, const QString &unit,
                                                 double target, bool showProgress)
{
    QFrame *card = new QFrame;
    card->setFrameStyle(QFrame::StyledPanel);
    card->setMinimumSize(CARD_MIN_WIDTH, CARD_MIN_HEIGHT);
    card->setMaximumHeight(CARD_MIN_HEIGHT);
    
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(8);
    layout->setContentsMargins(15, 10, 15, 10);
    
    // Title
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-weight: bold; color: #34495e; font-size: 12px;");
    
    // Value with unit
    QLabel *valueLabel = new QLabel(QString::number(value, 'f', 0) + unit);
    valueLabel->setObjectName("kpiValue");
    valueLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50;");
    valueLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    
    // Progress bar (if enabled)
    if (showProgress && target > 0) {
        QProgressBar *progressBar = new QProgressBar;
        progressBar->setObjectName("kpiProgress");
        progressBar->setRange(0, static_cast<int>(target));
        progressBar->setValue(static_cast<int>(value));
        progressBar->setTextVisible(false);
        progressBar->setMaximumHeight(6);
        
        QString progressColor = value >= target ? "#27ae60" : (value >= target * 0.7 ? "#f39c12" : "#e74c3c");
        progressBar->setStyleSheet(QString(R"(
            QProgressBar {
                border: none;
                border-radius: 3px;
                background-color: #ecf0f1;
            }
            QProgressBar::chunk {
                background-color: %1;
                border-radius: 3px;
            }
        )").arg(progressColor));
        
        layout->addWidget(progressBar);
        
        // Target label
        QLabel *targetLabel = new QLabel(QString("Target: %1%2").arg(target, 0, 'f', 0).arg(unit));
        targetLabel->setStyleSheet("color: #95a5a6; font-size: 10px;");
        targetLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(targetLabel);
    } else {
        layout->addStretch();
    }
    
    card->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border: 1px solid #ecf0f1;
            border-radius: 8px;
        }
        QFrame:hover {
            border: 1px solid #3498db;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
    )");
    
    return card;
}

void ContractStatisticsWidget::setDatabaseManager(ContractDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
    if (m_dbManager) {
        // Connect to database signals for auto-refresh
        connect(m_dbManager, &ContractDatabaseManager::contractAdded,
                this, &ContractStatisticsWidget::onContractAdded);
        connect(m_dbManager, &ContractDatabaseManager::contractUpdated,
                this, &ContractStatisticsWidget::onContractUpdated);
        connect(m_dbManager, &ContractDatabaseManager::contractDeleted,
                this, &ContractStatisticsWidget::onContractDeleted);
        
        // Initial load
        updateStatistics();
    }
}

void ContractStatisticsWidget::refreshStatistics()
{
    updateStatistics();
}

void ContractStatisticsWidget::updateStatistics()
{
    if (!m_dbManager) {
        return;
    }
    
    // Calculate statistics
    m_currentStats = calculateStatistics();
    
    // Update cards
    updateStatCard(m_totalContractsCard, formatNumber(m_currentStats.totalContracts), "All contracts");
    updateStatCard(m_activeContractsCard, formatNumber(m_currentStats.activeContracts), "Currently active");
    updateStatCard(m_totalValueCard, formatCurrency(m_currentStats.totalValue), "Combined value");
    updateStatCard(m_averageValueCard, formatCurrency(m_currentStats.averageContractValue), "Per contract");
    updateStatCard(m_completionRateCard, formatPercentage(m_currentStats.completionRate), "Success rate");
    updateStatCard(m_expiringContractsCard, formatNumber(m_currentStats.expiringIn30Days), "Next 30 days");
    
    // Update KPI cards
    updateKpiCard(m_monthlyRevenueKpi, m_currentStats.monthlyRevenue, 100000.0);
    updateKpiCard(m_renewalRateKpi, m_currentStats.renewalRate, 80.0);
    updateKpiCard(m_avgDurationKpi, m_currentStats.averageDuration);
    updateKpiCard(m_overdueContractsKpi, m_currentStats.overdueContracts);
    
    // Update charts
    updateChartData();
    
    emit statisticsUpdated();
}

ContractStatisticsWidget::ContractStatistics ContractStatisticsWidget::calculateStatistics()
{
    ContractStatistics stats;
    
    if (!m_dbManager) {
        return stats;
    }
    
    // Get all contracts within date range
    QList<Contract*> contracts = m_dbManager->getContractsInDateRange(m_startDate, m_endDate);
    
    stats.totalContracts = contracts.size();
    
    double totalValue = 0.0;
    QDate currentDate = QDate::currentDate();
    int totalDuration = 0;
    
    for (Contract *contract : contracts) {
        totalValue += contract->value();
        
        // Calculate duration
        int duration = contract->startDate().daysTo(contract->endDate());
        totalDuration += duration;
        
        // Count by status
        QString status = contract->status();
        if (status == "Active") {
            stats.activeContracts++;
            stats.activeValue += contract->value();
        } else if (status == "Completed") {
            stats.completedContracts++;
            stats.completedValue += contract->value();
        } else if (status == "Expired") {
            stats.expiredContracts++;
        } else if (status == "Draft") {
            stats.draftContracts++;
        } else if (status == "Pending") {
            stats.pendingContracts++;
        }
        
        // Check expiration
        int daysToExpiration = currentDate.daysTo(contract->endDate());
        if (daysToExpiration <= 30 && daysToExpiration >= 0) {
            stats.expiringIn30Days++;
        } else if (daysToExpiration <= 90 && daysToExpiration >= 0) {
            stats.expiringIn90Days++;
        }
        
        // Check overdue
        if (daysToExpiration < 0 && status != "Completed" && status != "Expired") {
            stats.overdueContracts++;
        }
    }
    
    stats.totalValue = totalValue;
    stats.averageContractValue = stats.totalContracts > 0 ? totalValue / stats.totalContracts : 0.0;
    stats.averageDuration = stats.totalContracts > 0 ? static_cast<double>(totalDuration) / stats.totalContracts : 0.0;
    stats.completionRate = stats.totalContracts > 0 ? 
        (static_cast<double>(stats.completedContracts) / stats.totalContracts) * 100.0 : 0.0;
    
    // Calculate monthly revenue (simple estimation)
    if (m_startDate.daysTo(m_endDate) > 0) {
        double months = m_startDate.daysTo(m_endDate) / 30.0;
        stats.monthlyRevenue = stats.activeValue / std::max(months, 1.0);
    }
    
    // Calculate renewal rate (placeholder - needs historical data)
    stats.renewalRate = 75.0; // Default placeholder
    
    // Clean up
    qDeleteAll(contracts);
    
    return stats;
}

void ContractStatisticsWidget::updateChartData()
{
    // Update status distribution chart
    m_statusPieSeries->clear();
    m_statusPieSeries->append("Active", m_currentStats.activeContracts);
    m_statusPieSeries->append("Completed", m_currentStats.completedContracts);
    m_statusPieSeries->append("Draft", m_currentStats.draftContracts);
    m_statusPieSeries->append("Expired", m_currentStats.expiredContracts);
    m_statusPieSeries->append("Pending", m_currentStats.pendingContracts);      // Update revenue chart
    m_revenueBarSeries->clear();
    QBarSet *revenueSet = new QBarSet("Revenue");
    revenueSet->append(m_currentStats.draftContracts * m_currentStats.averageContractValue * 0.5); // Estimated
    revenueSet->append(m_currentStats.activeValue);
    revenueSet->append(m_currentStats.completedValue);
    revenueSet->append(m_currentStats.expiredContracts * m_currentStats.averageContractValue * 0.3); // Estimated
    m_revenueBarSeries->append(revenueSet);
      // Update expiration chart
    m_expirationBarSeries->clear();
    QBarSet *expirationSet = new QBarSet("Contracts");
    expirationSet->append(m_currentStats.expiringIn30Days);
    expirationSet->append(m_currentStats.expiringIn90Days - m_currentStats.expiringIn30Days);
    expirationSet->append(0); // 91-180 days placeholder
    expirationSet->append(0); // Beyond 180 days placeholder
    m_expirationBarSeries->append(expirationSet);
      // Update trends chart (placeholder - needs historical data)
    m_trendsLineSeries->clear();
    QDateTime startDateTime = QDateTime(m_startDate, QTime(0, 0));
    for (int i = 0; i <= 12; ++i) {
        QDateTime date = startDateTime.addMonths(i);
        double randomFactor = QRandomGenerator::global()->generateDouble();
        m_trendsLineSeries->append(date.toMSecsSinceEpoch(), 
                                  m_currentStats.totalContracts * (0.7 + 0.3 * randomFactor));
    }
}

void ContractStatisticsWidget::onTimeRangeChanged()
{
    QString range = m_timeRangeCombo->currentText();
    QDate today = QDate::currentDate();
    
    if (range == "Last 30 Days") {
        m_startDate = today.addDays(-30);
        m_endDate = today;
    } else if (range == "Last 90 Days") {
        m_startDate = today.addDays(-90);
        m_endDate = today;
    } else if (range == "Last 6 Months") {
        m_startDate = today.addMonths(-6);
        m_endDate = today;
    } else if (range == "Last 12 Months") {
        m_startDate = today.addMonths(-12);
        m_endDate = today;
    } else if (range == "This Year") {
        m_startDate = QDate(today.year(), 1, 1);
        m_endDate = today;
    } else if (range == "All Time") {
        m_startDate = QDate(2000, 1, 1);
        m_endDate = today;
    } else if (range == "Custom") {
        // Show date inputs
        m_startDateEdit->setVisible(true);
        m_endDateEdit->setVisible(true);
        return;
    }
    
    // Hide date inputs for predefined ranges
    m_startDateEdit->setVisible(false);
    m_endDateEdit->setVisible(false);
    
    // Update date edits
    m_startDateEdit->setDate(m_startDate);
    m_endDateEdit->setDate(m_endDate);
    
    updateStatistics();
}

void ContractStatisticsWidget::onDateRangeChanged()
{
    m_startDate = m_startDateEdit->date();
    m_endDate = m_endDateEdit->date();
    
    if (m_startDate <= m_endDate) {
        updateStatistics();
    }
}

void ContractStatisticsWidget::onChartTypeChanged()
{
    QString chartType = m_chartTypeCombo->currentText();
    m_currentChartType = chartType;
    
    // Show/hide specific charts based on selection
    bool showStatus = (chartType == "Status Distribution" || chartType == "All Charts");
    bool showRevenue = (chartType == "Revenue Trends" || chartType == "All Charts");
    bool showTrends = (chartType == "Revenue Trends" || chartType == "All Charts");
    bool showExpiration = (chartType == "Expiration Timeline" || chartType == "All Charts");
    
    m_statusChartView->setVisible(showStatus);
    m_revenueChartView->setVisible(showRevenue);
    m_trendsChartView->setVisible(showTrends);
    m_expirationChartView->setVisible(showExpiration);
}

void ContractStatisticsWidget::onContractAdded(const QString &contractId)
{
    Q_UNUSED(contractId)
    if (m_autoRefresh) {
        updateStatistics();
    }
}

void ContractStatisticsWidget::onContractUpdated(const QString &contractId)
{
    Q_UNUSED(contractId)
    if (m_autoRefresh) {
        updateStatistics();
    }
}

void ContractStatisticsWidget::onContractDeleted(const QString &contractId)
{
    Q_UNUSED(contractId)
    if (m_autoRefresh) {
        updateStatistics();
    }
}

void ContractStatisticsWidget::onRefreshRequested()
{
    updateStatistics();
}

void ContractStatisticsWidget::onExportClicked()
{
    QStringList formats;
    formats << "PDF Report (*.pdf)" << "Excel Report (*.xlsx)" << "PNG Image (*.png)";
    
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Export Statistics Report", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + 
        QString("/Contract_Statistics_%1").arg(QDate::currentDate().toString("yyyy-MM-dd")),
        formats.join(";;"), 
        &selectedFilter);
    
    if (!fileName.isEmpty()) {
        if (selectedFilter.contains("PDF")) {
            exportToPdf();
        } else if (selectedFilter.contains("Excel")) {
            exportToExcel();
        } else if (selectedFilter.contains("PNG")) {
            exportToImage();
        }
    }
}

void ContractStatisticsWidget::exportToPdf()
{
    // Implementation would create a PDF report with all statistics
    // This is a placeholder for the full implementation
    QMessageBox::information(this, "Export", "PDF export functionality will be implemented.");
}

void ContractStatisticsWidget::exportToExcel()
{
    // Implementation would create an Excel file with statistics data
    // This is a placeholder for the full implementation
    QMessageBox::information(this, "Export", "Excel export functionality will be implemented.");
}

void ContractStatisticsWidget::exportToImage()
{
    // Implementation would capture and save charts as images
    // This is a placeholder for the full implementation
    QMessageBox::information(this, "Export", "Image export functionality will be implemented.");
}

void ContractStatisticsWidget::updateStatCard(QWidget *card, const QString &value, const QString &subtitle)
{
    Q_UNUSED(subtitle)
    
    QLabel *valueLabel = card->findChild<QLabel*>("valueLabel");
    if (valueLabel) {
        valueLabel->setText(value);
    }
}

void ContractStatisticsWidget::updateKpiCard(QWidget *card, double value, double target)
{
    QLabel *kpiValue = card->findChild<QLabel*>("kpiValue");
    QProgressBar *kpiProgress = card->findChild<QProgressBar*>("kpiProgress");
    
    if (kpiValue) {
        // Determine unit from the current text
        QString currentText = kpiValue->text();
        QString unit = "";
        if (currentText.contains("$")) unit = "$";
        else if (currentText.contains("%")) unit = "%";
        else if (currentText.contains("days")) unit = " days";
        
        kpiValue->setText(QString::number(value, 'f', 0) + unit);
    }
    
    if (kpiProgress && target > 0) {
        kpiProgress->setMaximum(static_cast<int>(target));
        kpiProgress->setValue(static_cast<int>(value));
    }
}

QString ContractStatisticsWidget::formatCurrency(double value) const
{
    QLocale locale;
    return locale.toCurrencyString(value, "$");
}

QString ContractStatisticsWidget::formatPercentage(double value) const
{
    return QString::number(value, 'f', 1) + "%";
}

QString ContractStatisticsWidget::formatNumber(int value) const
{
    QLocale locale;
    return locale.toString(value);
}

QString ContractStatisticsWidget::formatDuration(double days) const
{
    if (days < 30) {
        return QString::number(days, 'f', 0) + " days";
    } else if (days < 365) {
        return QString::number(days / 30.0, 'f', 1) + " months";
    } else {
        return QString::number(days / 365.0, 'f', 1) + " years";
    }
}

void ContractStatisticsWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_dbManager && !m_refreshTimer->isActive() && m_autoRefresh) {
        updateStatistics();
        m_refreshTimer->start();
    }
}

void ContractStatisticsWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Adjust chart sizes if needed
}

void ContractStatisticsWidget::setDateRange(const QDate &startDate, const QDate &endDate)
{
    m_startDate = startDate;
    m_endDate = endDate;
    m_startDateEdit->setDate(startDate);
    m_endDateEdit->setDate(endDate);
    m_timeRangeCombo->setCurrentText("Custom");
    updateStatistics();
}

void ContractStatisticsWidget::exportReport()
{
    onExportClicked();
}

void ContractStatisticsWidget::updateCharts()
{
    if (!m_dbManager) {
        return;
    }
    
    // Get contracts for the current date range
    QList<Contract*> contracts = m_dbManager->getContractsInDateRange(m_startDate, m_endDate);
    
    // Update all chart data
    updateChartData();
    
    // Clean up temporary contract objects
    qDeleteAll(contracts);
}
