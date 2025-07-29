#include <QApplication>
#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QtCharts::QPieSeries *series = new QtCharts::QPieSeries();
    series->append("Test", 10);
    
    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    
    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
    chartView->show();
    
    return app.exec();
}
