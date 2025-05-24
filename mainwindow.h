#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QtCharts>
#include <QLabel>
#include <QSlider>
#include "QRangeSlider.h"
#include <QDateEdit>
#include <QCheckBox>
#include "materialmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Material management slots
    void on_tableViewMaterials_clicked(const QModelIndex &index);
    void on_pushButtonNew_clicked();
    void on_pushButtonSave_clicked();
    void on_pushButtonDelete_clicked();
    void on_pushButtonClearFilters_clicked();
    void on_lineEditSearch_textChanged(const QString &text);
    void on_comboBoxCategory_currentTextChanged(const QString &text);
    
    // Advanced filtering slots
    void on_sliderPriceMin_valueChanged(int value);
    void on_sliderPriceMax_valueChanged(int value);
    void on_sliderQuantityMin_valueChanged(int value);
    void on_sliderQuantityMax_valueChanged(int value);
    void on_dateEditFrom_dateChanged(const QDate &date);
    void on_dateEditTo_dateChanged(const QDate &date);
    void on_checkBoxLowStock_toggled(bool checked);
    
    // Navigation slots
    void on_btnDashboard_clicked();
    void on_btnMaterials_clicked();
    void on_btnSuppliers_clicked();
    void on_btnUsers_clicked();
    void on_btnReports_clicked();
    void on_btnSettings_clicked();
    
    // Menu actions
    void on_actionNewMaterial_triggered();
    void on_actionExit_triggered();
    void on_actionImportMaterials_triggered();
    void on_actionExportMaterials_triggered();
    void on_actionBackupDatabase_triggered();
    void on_actionRestoreDatabase_triggered();
    void on_actionSettings_triggered();

private:
    Ui::MainWindow *ui;
    QSqlDatabase m_db;
    MaterialModel *m_materialModel;
    
    // Charts
    QChartView *m_quantityChartView;
    QChartView *m_valueChartView;
    QChartView *m_trendChartView;
    QChartView *m_predictionChartView;
    
    // Filter widgets
    QLabel *m_priceRangeLabel;
    QLabel *m_quantityRangeLabel;
    QSlider *m_priceMinSlider;
    QSlider *m_priceMaxSlider;
    QSlider *m_quantityMinSlider;
    QSlider *m_quantityMaxSlider;
    QDateEdit *m_dateEditFrom;
    QDateEdit *m_dateEditTo;
    QCheckBox *m_lowStockCheckBox;
    
    // Status indicators
    QLabel *m_statusLabel;
    QLabel *m_totalItemsLabel;
    QLabel *m_totalValueLabel;
    
    // Helper methods
    void setupDatabase();
    void setupMaterialModel();
    void setupAdvancedFilters();
    void updateCategoryComboBox();
    void clearForm();
    void loadMaterialDetails(int materialId);
    void applyFilters();
    
    // Chart methods
    void setupCharts();
    void updateCharts();
    void setupPredictionModel();
    
    // Low stock notification system
    void checkLowStockItems(int threshold = -1);
    
    // Navigation methods
    void setupNavigation();
    void loadSuppliersPage();
    void loadUsersPage();
    void loadReportsPage();
    void loadSettingsPage();
    
    // Data management methods
    bool backupDatabase(const QString &filePath);
    bool restoreDatabase(const QString &filePath);
    void validateImportData(const QStringList &data, QStringList &errors);
    
    // Accessibility helpers
    void setupAccessibility();
    void announceError(const QString &message);
    void announceSuccess(const QString &message);
    
    // File operations
    void setupFileOperationsToolbar();
    
    // Keyboard shortcuts
    void setupShortcuts();
    
    // Barcode/QR scanning
    void setupBarcodeScanner();
    void processBarcodeResult(const QString &barcode);
};
#endif // MAINWINDOW_H
