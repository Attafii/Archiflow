#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QDate>
#include <QTimer>
#include "databasemanager.h"
#include "statisticsdialog.h"
#include "chatwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addButton_clicked();
    void on_editButton_clicked();
    void on_deleteButton_clicked();
    void openChatWindow();
    void on_saveButton_clicked();
    void on_statisticsButton_clicked();
    void on_contractTableWidget_itemClicked(QTableWidgetItem *item);
    void on_searchLineEdit_textChanged(const QString &arg1);
    void on_filterComboBox_currentIndexChanged(int index);
    void on_actionContractList_triggered();
    void on_actionExpiringContracts_triggered();
    void on_actionValueSummary_triggered();
    void on_exportPdfButton_clicked();
    void on_actionExportPdf_triggered();

private:
    Ui::MainWindow *ui;
    DatabaseManager *dbManager;
    QTimer *expirationTimer;
    void checkExpiringContracts();
    void handleContractExpiration(const QString &contractId, const QString &clientName, const QDate &endDate);
    void clearDetails();
    void loadContractDetails(int row);
    void updateContractRow(int row);
    QString formatValue(double value);
    QString generateContractId();
    void setupDateEdits();
    void filterContracts();
    void loadContractsFromDatabase();
    void setupFilterWidgets();
    void exportContractsToPdf();
};
#endif // MAINWINDOW_H
