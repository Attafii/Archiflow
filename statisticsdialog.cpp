#include "statisticsdialog.h"
#include "ui_statisticsdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QLocale>

StatisticsDialog::StatisticsDialog(DatabaseManager *dbManager, QWidget *parent)
    : QDialog(parent), dbManager(dbManager), ui(new Ui::StatisticsDialog)
{
    ui->setupUi(this);
    setWindowTitle("Contract Management");
    setMinimumSize(800, 600);
    
    // Set modern style with custom colors
    QString styleSheet = "QDialog { background-color: #3D485A; color: white; }"
                        "QLabel { color: white; font-family: 'Poppins'; }"
                        "QGroupBox { color: white; font-family: 'Poppins'; border: 1px solid #E3C6B0; border-radius: 5px; margin-top: 1em; }"
                        "QGroupBox::title { color: #E3C6B0; }"
                        "QChartView { background-color: white; border-radius: 10px; margin: 10px; }";
    this->setStyleSheet(styleSheet);
    
    setupUI();
    updateStatistics();
}

StatisticsDialog::~StatisticsDialog()
{
    delete ui;
}

void StatisticsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title Section
    QLabel *titleLabel = new QLabel("Contract Management", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-family: 'Poppins'; font-size: 24px; font-weight: bold; color: #E3C6B0; margin: 10px;");
    mainLayout->addWidget(titleLabel);

    // Add your table and buttons here
    // ...

    // Statistics Section at the bottom
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);

    // Create statistics labels with modern card-like style
    QString cardStyle = "QLabel { background-color: #3D485A; padding: 15px; border-radius: 10px; font-family: 'Poppins'; color: #E3C6B0; }";

    // Total Contracts
    QLabel *totalContractsLabel = new QLabel(this);
    totalContractsLabel->setAlignment(Qt::AlignCenter);
    totalContractsLabel->setStyleSheet(cardStyle);
    totalContractsLabel->setMinimumWidth(180);

    // Total Value
    QLabel *totalValueLabel = new QLabel(this);
    totalValueLabel->setAlignment(Qt::AlignCenter);
    totalValueLabel->setStyleSheet(cardStyle);
    totalValueLabel->setMinimumWidth(180);

    // Average Price
    QLabel *avgPriceLabel = new QLabel(this);
    avgPriceLabel->setAlignment(Qt::AlignCenter);
    avgPriceLabel->setStyleSheet(cardStyle);
    avgPriceLabel->setMinimumWidth(180);

    // Average Length
    QLabel *avgLengthLabel = new QLabel(this);
    avgLengthLabel->setAlignment(Qt::AlignCenter);
    avgLengthLabel->setStyleSheet(cardStyle);
    avgLengthLabel->setMinimumWidth(180);

    statsLayout->addWidget(totalContractsLabel);
    statsLayout->addWidget(totalValueLabel);
    statsLayout->addWidget(avgPriceLabel);
    statsLayout->addWidget(avgLengthLabel);

    mainLayout->addLayout(statsLayout);

    // Store widgets for later updates
    m_totalContractsLabel = totalContractsLabel;
    m_totalValueLabel = totalValueLabel;
    m_avgPriceLabel = avgPriceLabel;
    m_avgLengthLabel = avgLengthLabel;
}

void StatisticsDialog::updateStatistics()
{
    // Get total contracts and value
    QMap<QString, int> distribution = dbManager->getContractStatusDistribution();
    int totalCount = 0;
    for (auto value : distribution.values()) {
        totalCount += value;
    }
    double totalValue = dbManager->getTotalContractValue();
    
    // Calculate average price
    double avgPrice = totalCount > 0 ? totalValue / totalCount : 0;
    
    // Calculate average contract length
    QSqlQuery query = dbManager->getAllContracts();
    int totalDays = 0;
    int contractsWithDates = 0;
    
    while (query.next()) {
        QDate startDate = query.value("start_date").toDate();
        QDate endDate = query.value("end_date").toDate();
        if (startDate.isValid() && endDate.isValid()) {
            totalDays += startDate.daysTo(endDate);
            contractsWithDates++;
        }
    }
    
    double avgLength = contractsWithDates > 0 ? static_cast<double>(totalDays) / contractsWithDates : 0;
    
    // Update statistics labels with modern formatting
    m_totalContractsLabel->setText(QString("Total Contracts\n%1").arg(totalCount));
    m_totalValueLabel->setText(QString("Total Value\n$%1").arg(totalValue, 0, 'f', 2));
    m_avgPriceLabel->setText(QString("Average Price\n$%1").arg(avgPrice, 0, 'f', 2));
    m_avgLengthLabel->setText(QString("Average Length\n%1 days").arg(static_cast<int>(avgLength)));
}

void StatisticsDialog::updateStatusDistributionChart()
{
    QMap<QString, int> distribution = dbManager->getContractStatusDistribution();
    
    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.35);

    QStringList colors = {"#E3C6B0", "#3D485A", "#A58F7B", "#566478", "#C4A491"};
    int colorIndex = 0;

    for (auto it = distribution.constBegin(); it != distribution.constEnd(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: %2").arg(it.key()).arg(it.value()));
        
        if (colorIndex < colors.size()) {
            slice->setColor(QColor(colors[colorIndex]));
            colorIndex++;
        }
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Contract Status Distribution");
    chart->setTitleFont(QFont("Poppins", 12, QFont::Bold));
    chart->setBackgroundBrush(QBrush(QColor("white")));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Poppins", 10));
    chart->setTitleBrush(QBrush(QColor("#3D485A")));
    chart->setAnimationOptions(QChart::AllAnimations);

    m_statusChartView->setChart(chart);
}

void StatisticsDialog::updateValueTrendChart()
{
    // Get data for the last 12 months
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addMonths(-11);
    QMap<QDate, double> trend = dbManager->getContractValueTrend(startDate, endDate);

    QBarSeries *series = new QBarSeries();
    QBarSet *valueSet = new QBarSet("Contract Value");
    QStringList categories;

    // Fill in missing months with zero values
    for (QDate date = startDate; date <= endDate; date = date.addMonths(1)) {
        QString monthYear = date.toString("MMM yyyy");
        categories << monthYear;
        
        double value = 0;
        for (auto it = trend.constBegin(); it != trend.constEnd(); ++it) {
            if (it.key().month() == date.month() && it.key().year() == date.year()) {
                value = it.value();
                break;
            }
        }
        *valueSet << value;
    }

    series->append(valueSet);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Contract Value Trend");
    chart->setTitleFont(QFont("Poppins", 12, QFont::Bold));
    chart->setBackgroundBrush(QBrush(QColor("white")));
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTitleBrush(QBrush(QColor("#3D485A")));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsFont(QFont("Poppins", 9));
    axisX->setLabelsBrush(QBrush(QColor("#3D485A")));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value ($)");
    axisY->setTitleFont(QFont("Poppins", 10));
    axisY->setLabelsFont(QFont("Poppins", 9));
    axisY->setTitleBrush(QBrush(QColor("#3D485A")));
    axisY->setLabelsBrush(QBrush(QColor("#3D485A")));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    m_valueTrendChartView->setChart(chart);
}