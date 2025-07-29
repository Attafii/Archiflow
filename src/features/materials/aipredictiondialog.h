#ifndef AIPREDICTIONDIALOG_H
#define AIPREDICTIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QValueAxis>
#include <QCategoryAxis>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QSplitter>
#include <QFormLayout>
#include "groqclient.h"

class DatabaseService;

/**
 * @brief AI-powered prediction and analytics dialog for materials management
 * 
 * This dialog provides intelligent predictions for:
 * - Demand forecasting
 * - Cost predictions
 * - Inventory optimization
 * - Stock level recommendations
 * - Seasonal analysis
 */
class AIPredictionDialog : public QDialog
{
    Q_OBJECT

public:
    enum PredictionType {
        DemandForecast,
        CostPrediction,
        InventoryOptimization,
        SeasonalAnalysis,
        SupplierAnalysis
    };    explicit AIPredictionDialog(QWidget *parent = nullptr);
    ~AIPredictionDialog();

    void setGroqClient(GroqClient *client);
    void setDatabaseService(DatabaseService *service);
    void setMaterialContext(const QJsonObject &context);

public slots:
    void generatePrediction();
    void refreshData();
    void exportResults();

private slots:
    void onTabChanged(int index);
    void onPredictionTypeChanged(int index);
    void onParameterChanged();
    void onPredictionCompleted(const QString &response);
    void onPredictionError(const QString &error);
    void updateCharts();
    void animateResults();
    void onQuickInsightToggled(bool checked);

private:
    void setupUI();
    void setupHeader(QVBoxLayout *mainLayout);
    void setupControlPanel(QSplitter *splitter);
    void setupResultsPanel(QSplitter *splitter);
    void setupChartTab();
    void setupTableTab();
    void setupInsightsTab();
    void setupLoadingOverlay(QWidget *parent);
    void setupConnections();
    void applyTheming();
    void loadGroqApiKey();
    void loadMaterialCategories();
    
    QString gatherContextData(const QString &category);
    QString buildPredictionPrompt(const QString &predictionType, int timeHorizon,
                                  const QString &confidence, const QString &category,
                                  const QStringList &selectedInsights, const QString &contextData);
    void sendPredictionRequest(const QString &prompt);
    void handlePredictionResponse();
    void parsePredictionResults(const QString &jsonResponse);
    
    void tryNextModel();
    
    void updateChart(const QJsonObject &result);
    void createLineChart(QChart *chart, const QJsonObject &result);
    void createBarChart(QChart *chart, const QJsonObject &result);
    void createSampleChart(QChart *chart);
    void updateTable(const QJsonObject &result);
    void updateInsights(const QJsonObject &result);
    
    void setLoadingState(bool loading);
    void clearResults();
    void showApiKeyDialog();
    
    void updateStatus(const QString &status);
    void updateChartWithData(const QJsonArray &data);
    void updateTableWithData(const QJsonArray &data);
    void makeGroqRequest(const QString &prompt);
    QString buildPrompt();
    QJsonArray getMaterialsData();
    
    void setupTabs();
    void setupDemandForecastTab();
    void setupCostPredictionTab();
    void setupInventoryOptimizationTab();
    void setupSeasonalAnalysisTab();
    void setupSupplierAnalysisTab();
    
    void generateDemandForecast();
    void generateCostPrediction();
    void generateInventoryOptimization();
    void generateSeasonalAnalysis();
    void generateSupplierAnalysis();
    
    void updateDemandChart(const QJsonArray &data);
    void updateCostChart(const QJsonArray &data);
    void updateInventoryChart(const QJsonArray &data);
    void updateSeasonalChart(const QJsonArray &data);
    void updateSupplierChart(const QJsonArray &data);
    
    void showLoadingAnimation();
    void hideLoadingAnimation();
    void displayPredictionResults(const QString &results);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    // Core components
    QNetworkAccessManager *m_networkManager;
    QString m_groqApiKey;
    DatabaseService *m_databaseService;
    bool m_isLoading;
    
    // Retry mechanism
    int m_retryCount;
    QString m_currentPrompt;
    QStringList m_availableModels;
    int m_currentModelIndex;
    
    // Header components
    QLabel *m_statusLabel;
    
    // Control panel components
    QComboBox *m_predictionTypeCombo;
    QSpinBox *m_timeHorizonSpin;
    QComboBox *m_confidenceCombo;
    QComboBox *m_categoryCombo;
    QList<QPushButton*> m_quickInsightButtons;
    QPushButton *m_generateButton;
      // Results panel components
    QPushButton *m_exportButton;
    QTabWidget *m_resultsTabWidget;
    QChartView *m_chartView;
    QTableWidget *m_resultsTable;
    QTextEdit *m_insightsTextEdit;
    
    // Loading overlay components
    QWidget *m_loadingOverlay;
    QLabel *m_loadingSpinner;
    
    // Legacy UI Components (kept for compatibility)
    GroqClient *m_groqClient;
    QJsonObject m_materialContext;
    
    // Animation and effects
    QPropertyAnimation *m_fadeAnimation;
    QGraphicsOpacityEffect *m_opacityEffect;
    QTimer *m_loadingTimer;
    
    // Current state
    PredictionType m_currentPredictionType;
    bool m_isGenerating;
    QJsonObject m_currentResults;
};

#endif // AIPREDICTIONDIALOG_H
