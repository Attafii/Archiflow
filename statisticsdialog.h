#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDate>
#include <QChartView>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class StatisticsDialog; }
QT_END_NAMESPACE

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~StatisticsDialog();

private:
    Ui::StatisticsDialog *ui;
    DatabaseManager *dbManager;
    QLabel *m_totalValueLabel;
    QLabel *m_totalContractsLabel;
    QLabel *m_avgPriceLabel;
    QLabel *m_avgLengthLabel;
    QChartView *m_statusChartView;
    QChartView *m_valueTrendChartView;
    
    void setupUI();
    void updateStatistics();
    void updateStatusDistributionChart();
    void updateValueTrendChart();
};


#endif // STATISTICSDIALOG_H