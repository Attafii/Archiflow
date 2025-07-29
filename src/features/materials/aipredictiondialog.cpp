#include "aipredictiondialog.h"
#include "../../database/databaseservice.h"
#include "../../utils/environmentloader.h"
#include <QApplication>
#include <QRandomGenerator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPainter>
#include <QMovie>
#include <QTimer>
#include <QScrollArea>
#include <QSplitter>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QDateTimeAxis>
#include <QDateTime>

AIPredictionDialog::AIPredictionDialog(QWidget *parent)
    : QDialog(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_groqApiKey(QString())
    , m_databaseService(new DatabaseService(this))
    , m_isLoading(false)
    , m_retryCount(0)
    , m_currentModelIndex(0)
{
    // Initialize available models in order of preference
    m_availableModels << "llama3-8b-8192" 
                     << "llama3-70b-8192" 
                     << "mixtral-8x7b-32768"
                     << "gemma-7b-it";
    
    setupUI();
    setupConnections();
    loadGroqApiKey();
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, false); // Prevent auto-deletion
}

AIPredictionDialog::~AIPredictionDialog()
{
}

void AIPredictionDialog::setupUI()
{
    setWindowTitle("AI Prediction Analytics - ArchiFlow");
    setMinimumSize(1200, 800);
    resize(1400, 900);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    setupHeader(mainLayout);

    // Content area with splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setStyleSheet("QSplitter::handle { background-color: #3d4043; width: 2px; }");

    // Left panel - Controls
    setupControlPanel(splitter);

    // Right panel - Results
    setupResultsPanel(splitter);

    // Set splitter proportions
    splitter->setSizes({400, 800});
    mainLayout->addWidget(splitter);

    // Apply styling
    applyTheming();
}

void AIPredictionDialog::setupHeader(QVBoxLayout *mainLayout)
{
    QWidget *headerWidget = new QWidget;
    headerWidget->setFixedHeight(80);
    headerWidget->setObjectName("headerWidget");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(30, 0, 30, 0);

    // Title and subtitle
    QVBoxLayout *titleLayout = new QVBoxLayout;
    
    QLabel *titleLabel = new QLabel("AI Prediction Analytics");
    titleLabel->setObjectName("titleLabel");
    
    QLabel *subtitleLabel = new QLabel("Forecast demand, predict costs, and analyze trends with AI");
    subtitleLabel->setObjectName("subtitleLabel");
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    titleLayout->addStretch();

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // Status indicator
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setObjectName("statusLabel");
    headerLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(headerWidget);
}

void AIPredictionDialog::setupControlPanel(QSplitter *splitter)
{
    QWidget *controlPanel = new QWidget;
    controlPanel->setObjectName("controlPanel");
    controlPanel->setMinimumWidth(350);
    controlPanel->setMaximumWidth(450);

    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setContentsMargins(20, 20, 20, 20);
    controlLayout->setSpacing(20);

    // Prediction Type Selection
    QGroupBox *typeGroup = new QGroupBox("Prediction Type");
    typeGroup->setObjectName("predictionGroup");
    QVBoxLayout *typeLayout = new QVBoxLayout(typeGroup);

    m_predictionTypeCombo = new QComboBox;
    m_predictionTypeCombo->addItems({
        "Demand Forecasting",
        "Cost Prediction", 
        "Trend Analysis",
        "Budget Planning",
        "Resource Optimization"
    });
    typeLayout->addWidget(m_predictionTypeCombo);

    controlLayout->addWidget(typeGroup);

    // Parameters
    QGroupBox *paramGroup = new QGroupBox("Parameters");
    paramGroup->setObjectName("predictionGroup");
    QFormLayout *paramLayout = new QFormLayout(paramGroup);

    m_timeHorizonSpin = new QSpinBox;
    m_timeHorizonSpin->setRange(1, 24);
    m_timeHorizonSpin->setValue(6);
    m_timeHorizonSpin->setSuffix(" months");
    paramLayout->addRow("Time Horizon:", m_timeHorizonSpin);

    m_confidenceCombo = new QComboBox;
    m_confidenceCombo->addItems({"90%", "95%", "99%"});
    m_confidenceCombo->setCurrentText("95%");
    paramLayout->addRow("Confidence Level:", m_confidenceCombo);

    m_categoryCombo = new QComboBox;
    m_categoryCombo->addItem("All Categories");
    loadMaterialCategories();
    paramLayout->addRow("Material Category:", m_categoryCombo);

    controlLayout->addWidget(paramGroup);

    // Quick Insights
    QGroupBox *insightsGroup = new QGroupBox("Quick Insights");
    insightsGroup->setObjectName("predictionGroup");
    QVBoxLayout *insightsLayout = new QVBoxLayout(insightsGroup);    for (const QString insight : {
        "Market Trends", "Seasonal Patterns", "Cost Volatility", 
        "Supply Chain Risk", "Demand Spikes"}) {
        QPushButton *btn = new QPushButton(insight);
        btn->setObjectName("quickInsightBtn");
        btn->setCheckable(true);
        m_quickInsightButtons.append(btn);
        insightsLayout->addWidget(btn);
    }

    controlLayout->addWidget(insightsGroup);

    // Generate Button
    m_generateButton = new QPushButton("Generate Prediction");
    m_generateButton->setObjectName("generateButton");
    m_generateButton->setMinimumHeight(45);
    controlLayout->addWidget(m_generateButton);

    controlLayout->addStretch();

    splitter->addWidget(controlPanel);
}

void AIPredictionDialog::setupResultsPanel(QSplitter *splitter)
{
    QWidget *resultsPanel = new QWidget;
    resultsPanel->setObjectName("resultsPanel");

    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsPanel);
    resultsLayout->setContentsMargins(20, 20, 20, 20);
    resultsLayout->setSpacing(15);

    // Results header
    QHBoxLayout *resultsHeaderLayout = new QHBoxLayout;
    
    QLabel *resultsTitle = new QLabel("Prediction Results");
    resultsTitle->setObjectName("resultsTitle");
    resultsHeaderLayout->addWidget(resultsTitle);
    
    resultsHeaderLayout->addStretch();
    
    // Export button
    m_exportButton = new QPushButton("Export Results");
    m_exportButton->setObjectName("exportButton");
    m_exportButton->setEnabled(false);
    resultsHeaderLayout->addWidget(m_exportButton);

    resultsLayout->addLayout(resultsHeaderLayout);

    // Tab widget for different views
    m_resultsTabWidget = new QTabWidget;
    m_resultsTabWidget->setObjectName("resultsTabWidget");

    // Chart view tab
    setupChartTab();
    
    // Table view tab
    setupTableTab();
    
    // Insights tab
    setupInsightsTab();

    resultsLayout->addWidget(m_resultsTabWidget);

    // Loading overlay
    setupLoadingOverlay(resultsPanel);

    splitter->addWidget(resultsPanel);
}

void AIPredictionDialog::setupChartTab()
{
    QWidget *chartWidget = new QWidget;
    QVBoxLayout *chartLayout = new QVBoxLayout(chartWidget);

    // Chart view
    m_chartView = new QChartView;
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setObjectName("chartView");
    
    // Create default empty chart
    QChart *chart = new QChart();
    chart->setTitle("Select prediction type to generate forecast");
    chart->setTheme(QChart::ChartThemeDark);
    chart->setBackgroundBrush(QBrush(QColor("#2d2d30")));
    chart->setTitleBrush(QBrush(QColor("#ffffff")));
    m_chartView->setChart(chart);

    chartLayout->addWidget(m_chartView);

    m_resultsTabWidget->addTab(chartWidget, "📊 Chart View");
}

void AIPredictionDialog::setupTableTab()
{
    QWidget *tableWidget = new QWidget;
    QVBoxLayout *tableLayout = new QVBoxLayout(tableWidget);

    m_resultsTable = new QTableWidget;
    m_resultsTable->setObjectName("resultsTable");
    m_resultsTable->setAlternatingRowColors(true);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);

    tableLayout->addWidget(m_resultsTable);

    m_resultsTabWidget->addTab(tableWidget, "📋 Data Table");
}

void AIPredictionDialog::setupInsightsTab()
{
    QWidget *insightsWidget = new QWidget;
    QVBoxLayout *insightsLayout = new QVBoxLayout(insightsWidget);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setObjectName("insightsScrollArea");

    m_insightsTextEdit = new QTextEdit;
    m_insightsTextEdit->setObjectName("insightsTextEdit");
    m_insightsTextEdit->setReadOnly(true);
    m_insightsTextEdit->setPlaceholderText("AI insights will appear here after generating predictions...");

    scrollArea->setWidget(m_insightsTextEdit);
    insightsLayout->addWidget(scrollArea);

    m_resultsTabWidget->addTab(insightsWidget, "🧠 AI Insights");
}

void AIPredictionDialog::setupLoadingOverlay(QWidget *parent)
{
    m_loadingOverlay = new QWidget(parent);
    m_loadingOverlay->setStyleSheet("background-color: rgba(45, 45, 48, 180);");
    m_loadingOverlay->hide();

    QVBoxLayout *overlayLayout = new QVBoxLayout(m_loadingOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);

    QLabel *loadingLabel = new QLabel("Generating AI Predictions...");
    loadingLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    loadingLabel->setAlignment(Qt::AlignCenter);

    m_loadingSpinner = new QLabel;
    m_loadingSpinner->setAlignment(Qt::AlignCenter);
    m_loadingSpinner->setFixedSize(64, 64);

    // Create spinning animation
    QMovie *movie = new QMovie(":/icons/loading.gif");
    if (!movie->isValid()) {
        // Fallback: create a simple rotating text
        m_loadingSpinner->setText("⟳");
        m_loadingSpinner->setStyleSheet("color: #007ACC; font-size: 32px;");
        
        QTimer *rotationTimer = new QTimer(this);
        connect(rotationTimer, &QTimer::timeout, [this]() {
            static int rotation = 0;
            rotation = (rotation + 30) % 360;
            m_loadingSpinner->setStyleSheet(QString("color: #007ACC; font-size: 32px; transform: rotate(%1deg);").arg(rotation));
        });
        rotationTimer->start(100);
    } else {
        m_loadingSpinner->setMovie(movie);
        movie->start();
    }

    overlayLayout->addWidget(m_loadingSpinner);
    overlayLayout->addWidget(loadingLabel);
}

void AIPredictionDialog::setupConnections()
{
    connect(m_generateButton, &QPushButton::clicked, this, &AIPredictionDialog::generatePrediction);
    connect(m_exportButton, &QPushButton::clicked, this, &AIPredictionDialog::exportResults);
    connect(m_predictionTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &AIPredictionDialog::onPredictionTypeChanged);
    
    // Connect quick insight buttons
    for (QPushButton *btn : m_quickInsightButtons) {
        connect(btn, &QPushButton::toggled, this, &AIPredictionDialog::onQuickInsightToggled);
    }
}

void AIPredictionDialog::applyTheming()
{
    setStyleSheet(R"(
        AIPredictionDialog {
            background-color: #2d2d30;
            color: #ffffff;
        }
        
        #headerWidget {
            background-color: #3c3c3c;
            border-bottom: 2px solid #007ACC;
        }
        
        #titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #ffffff;
        }
        
        #subtitleLabel {
            font-size: 14px;
            color: #cccccc;
            margin-top: 5px;
        }
        
        #statusLabel {
            color: #4EC9B0;
            font-weight: bold;
            padding: 5px 10px;
            background-color: rgba(78, 201, 176, 0.1);
            border-radius: 12px;
        }
        
        #controlPanel {
            background-color: #252526;
            border-right: 1px solid #3d4043;
        }
        
        #resultsPanel {
            background-color: #1e1e1e;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #3d4043;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            color: #ffffff;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px 0 8px;
            color: #007ACC;
        }
        
        #predictionGroup {
            background-color: #2d2d30;
        }
        
        QComboBox, QSpinBox {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 6px;
            color: #ffffff;
            min-height: 20px;
        }
        
        QComboBox:hover, QSpinBox:hover {
            border-color: #007ACC;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #cccccc;
            margin-right: 5px;
        }
        
        #quickInsightBtn {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 6px;
            padding: 8px 12px;
            color: #ffffff;
            text-align: left;
            margin: 2px 0;
        }
        
        #quickInsightBtn:hover {
            background-color: #4a4a4a;
            border-color: #007ACC;
        }
        
        #quickInsightBtn:checked {
            background-color: #007ACC;
            border-color: #007ACC;
            color: #ffffff;
        }
        
        #generateButton {
            background-color: #007ACC;
            border: none;
            border-radius: 6px;
            color: #ffffff;
            font-weight: bold;
            font-size: 14px;
        }
        
        #generateButton:hover {
            background-color: #1e88e5;
        }
        
        #generateButton:pressed {
            background-color: #0d5aa7;
        }
        
        #generateButton:disabled {
            background-color: #555555;
            color: #999999;
        }
        
        #resultsTitle {
            font-size: 18px;
            font-weight: bold;
            color: #ffffff;
        }
        
        #exportButton {
            background-color: #4EC9B0;
            border: none;
            border-radius: 4px;
            color: #000000;
            font-weight: bold;
            padding: 8px 16px;
        }
        
        #exportButton:hover {
            background-color: #5fd4b8;
        }
        
        #exportButton:disabled {
            background-color: #555555;
            color: #999999;
        }
        
        #resultsTabWidget::pane {
            border: 1px solid #3d4043;
            background-color: #2d2d30;
        }
        
        #resultsTabWidget::tab-bar {
            alignment: left;
        }
        
        QTabBar::tab {
            background-color: #3c3c3c;
            color: #cccccc;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        
        QTabBar::tab:selected {
            background-color: #007ACC;
            color: #ffffff;
        }
        
        QTabBar::tab:hover:!selected {
            background-color: #4a4a4a;
        }
        
        #chartView {
            background-color: #2d2d30;
            border: 1px solid #3d4043;
            border-radius: 4px;
        }
        
        #resultsTable {
            background-color: #2d2d30;
            alternate-background-color: #3c3c3c;
            color: #ffffff;
            gridline-color: #555555;
            border: 1px solid #3d4043;
        }
        
        #resultsTable::item {
            padding: 8px;
            border-bottom: 1px solid #555555;
        }
        
        #resultsTable::item:selected {
            background-color: #007ACC;
        }
        
        QHeaderView::section {
            background-color: #3c3c3c;
            color: #ffffff;
            padding: 8px;
            border: 1px solid #555555;
            font-weight: bold;
        }
        
        #insightsScrollArea {
            border: 1px solid #3d4043;
            border-radius: 4px;
        }
        
        #insightsTextEdit {
            background-color: #2d2d30;
            color: #ffffff;
            border: none;
            font-family: 'Segoe UI', sans-serif;
            font-size: 13px;
            line-height: 1.5;
        }
    )");
}

void AIPredictionDialog::loadGroqApiKey()
{
    m_groqApiKey = EnvironmentLoader::getEnv("GROQ_API_KEY");
    
    if (m_groqApiKey.isEmpty()) {
        m_groqApiKey = qgetenv("GROQ_API_KEY");
    }
    
    if (m_groqApiKey.isEmpty()) {
        m_statusLabel->setText("⚠️ API Key Missing");
        m_statusLabel->setStyleSheet("color: #f48771; background-color: rgba(244, 135, 113, 0.1);");
        m_generateButton->setEnabled(false);
        m_generateButton->setText("Configure API Key");
    }
}

void AIPredictionDialog::loadMaterialCategories()
{
    QStringList categories = m_databaseService->getValidCategories();
    for (const QString &category : categories) {
        m_categoryCombo->addItem(category);
    }
}

void AIPredictionDialog::generatePrediction()
{
    if (m_groqApiKey.isEmpty()) {
        showApiKeyDialog();
        return;
    }

    if (m_isLoading) {
        return;
    }

    setLoadingState(true);
    
    // Reset retry mechanism
    m_retryCount = 0;
    m_currentModelIndex = 0;
    
    // Gather parameters
    QString predictionType = m_predictionTypeCombo->currentText();
    int timeHorizon = m_timeHorizonSpin->value();
    QString confidence = m_confidenceCombo->currentText();
    QString category = m_categoryCombo->currentText();
    
    QStringList selectedInsights;
    for (QPushButton *btn : m_quickInsightButtons) {
        if (btn->isChecked()) {
            selectedInsights << btn->text();
        }
    }

    // Get relevant data from database
    QString contextData = gatherContextData(category);
    
    // Build prompt and store it for potential retries
    m_currentPrompt = buildPredictionPrompt(predictionType, timeHorizon, confidence, category, selectedInsights, contextData);
    
    // Send to Groq API
    sendPredictionRequest(m_currentPrompt);
}

void AIPredictionDialog::tryNextModel()
{
    m_currentModelIndex++;
    m_retryCount++;
    
    if (m_currentModelIndex >= m_availableModels.size()) {
        // We've tried all models, show error
        QMessageBox::warning(this, "API Error", 
            "Failed to generate prediction with all available models. Please check your internet connection and try again later.");
        setLoadingState(false);
        return;
    }
    
    qDebug() << "Retrying with next model:" << m_availableModels[m_currentModelIndex] 
             << "(attempt" << (m_retryCount + 1) << "of" << m_availableModels.size() << ")";
    
    // Update status to show retry
    if (m_statusLabel) {
        m_statusLabel->setText(QString("🔄 Retrying with %1... (Attempt %2)")
                              .arg(m_availableModels[m_currentModelIndex])
                              .arg(m_retryCount + 1));
    }
    
    setLoadingState(true);
    sendPredictionRequest(m_currentPrompt);
}

QString AIPredictionDialog::gatherContextData(const QString &category)
{
    QStringList contextParts;
    
    // Get materials data
    QJsonArray materialsArray;
    if (category == "All Categories") {
        materialsArray = m_databaseService->getAllMaterials();
    } else {
        materialsArray = m_databaseService->getMaterialsByCategory(category);
    }
    
    contextParts << QString("Total materials in scope: %1").arg(materialsArray.size());
    
    if (!materialsArray.isEmpty()) {
        // Calculate statistics
        double totalValue = 0;
        double minPrice = -1;
        double maxPrice = -1;
        QMap<QString, int> categoryCount;
        QMap<QString, int> supplierCount;
        
        for (int i = 0; i < materialsArray.size(); i++) {
            QJsonObject material = materialsArray[i].toObject();
            
            double unitPrice = material["unitPrice"].toDouble();
            int quantity = material["quantity"].toInt();
            QString materialCategory = material["category"].toString();
            QString supplier = material["supplier"].toString();
            
            totalValue += unitPrice * quantity;
            
            if (minPrice < 0 || unitPrice < minPrice) {
                minPrice = unitPrice;
            }
            if (maxPrice < 0 || unitPrice > maxPrice) {
                maxPrice = unitPrice;
            }
            
            categoryCount[materialCategory]++;
            supplierCount[supplier]++;
        }
        
        if (minPrice >= 0 && maxPrice >= 0) {
            contextParts << QString("Total inventory value: $%1").arg(totalValue, 0, 'f', 2);
            contextParts << QString("Price range: $%1 - $%2").arg(minPrice, 0, 'f', 2).arg(maxPrice, 0, 'f', 2);
        }
        contextParts << QString("Categories: %1").arg(categoryCount.keys().join(", "));
        contextParts << QString("Suppliers: %1").arg(supplierCount.keys().join(", "));
        
        // Sample materials for context
        contextParts << "\nSample materials data:";
        int sampleCount = qMin(5, materialsArray.size());
        for (int i = 0; i < sampleCount; i++) {
            QJsonObject material = materialsArray[i].toObject();
            contextParts << QString("- %1 (%2): $%3/unit, Qty: %4, Supplier: %5")
                .arg(material["name"].toString())
                .arg(material["category"].toString())
                .arg(material["unitPrice"].toDouble(), 0, 'f', 2)
                .arg(material["quantity"].toInt())
                .arg(material["supplier"].toString());
        }
    }
    
    return contextParts.join("\n");
}

QString AIPredictionDialog::buildPredictionPrompt(const QString &predictionType, int timeHorizon, 
    const QString &confidence, const QString &category, const QStringList &insights, const QString &contextData)
{
    QString prompt = QString(R"(
You are an AI expert in construction materials management and predictive analytics. 

TASK: Generate a %1 for a construction materials inventory system.

PARAMETERS:
- Time Horizon: %2 months
- Confidence Level: %3
- Material Category: %4
- Focus Areas: %5

CURRENT DATA CONTEXT:
%6

INSTRUCTIONS:
1. Provide specific, actionable predictions based on the data provided
2. Include numerical forecasts where applicable
3. Identify key trends, risks, and opportunities
4. Suggest optimization strategies
5. Format your response as JSON with the following structure:

{
  "prediction_type": "%1",
  "summary": "Brief overview of key findings",
  "forecast_data": [
    {
      "period": "Month 1", 
      "value": numeric_value,
      "confidence": confidence_percentage,
      "notes": "explanation"
    }
    // ... more periods
  ],
  "key_insights": [
    "insight 1",
    "insight 2",
    // ... more insights
  ],
  "recommendations": [
    {
      "priority": "High/Medium/Low",
      "action": "specific recommendation",
      "impact": "expected outcome",
      "timeline": "implementation timeframe"
    }
    // ... more recommendations
  ],
  "risk_factors": [
    {
      "risk": "risk description",
      "probability": "High/Medium/Low",
      "impact": "potential consequences",
      "mitigation": "suggested mitigation strategy"
    }
    // ... more risks
  ],
  "charts": {
    "chart_type": "line|bar|pie",
    "title": "Chart Title",
    "x_axis": "X-axis label",
    "y_axis": "Y-axis label",
    "data_points": [
      {"label": "Label1", "value": numeric_value},
      {"label": "Label2", "value": numeric_value}
      // ... more data points
    ]
  }
}

Ensure all numerical values are realistic and based on construction industry standards and the provided data context.
)")
    .arg(predictionType)
    .arg(timeHorizon)
    .arg(confidence)
    .arg(category)
    .arg(insights.join(", "))
    .arg(contextData);

    return prompt;
}

void AIPredictionDialog::sendPredictionRequest(const QString &prompt)
{
    QString currentModel = m_availableModels[m_currentModelIndex];
    qDebug() << "Groq API request sent with model:" << currentModel;
    qDebug() << "API endpoint: https://api.groq.com/openai/v1/chat/completions";
    
    QUrl url("https://api.groq.com/openai/v1/chat/completions");
    QNetworkRequest request(url);
    
    // Set proper headers for Groq API
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_groqApiKey).toUtf8());
    request.setRawHeader("User-Agent", "ArchiFlow/1.0");
    
    QJsonObject json;
    // Use the current model from the list
    json["model"] = currentModel;
    json["temperature"] = 0.3;
    json["max_tokens"] = 4000;
    json["top_p"] = 1.0;
    json["stream"] = false;
    
    QJsonArray messages;
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "You are an expert AI assistant specialized in construction materials management and prediction analytics. Provide accurate, detailed analysis based on the data provided. Always respond with valid JSON format when requested.";
    messages.append(systemMessage);
    
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = prompt;
    messages.append(userMessage);
    
    json["messages"] = messages;
    
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();
    
    qDebug() << "Request model:" << currentModel;
    qDebug() << "Request size:" << data.size() << "bytes";
    
    QNetworkReply *reply = m_networkManager->post(request, data);
    
    // Set timeout for the request
    QTimer::singleShot(30000, reply, [reply]() {
        if (reply->isRunning()) {
            qDebug() << "Request timeout, aborting";
            reply->abort();
        }
    });
    
    // Use Qt::QueuedConnection to ensure safe handling
    connect(reply, &QNetworkReply::finished, this, &AIPredictionDialog::handlePredictionResponse, Qt::QueuedConnection);
    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](QNetworkReply::NetworkError error) {
        qDebug() << "Network error occurred:" << error;
        qDebug() << "Error string:" << reply->errorString();
        qDebug() << "HTTP status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        QByteArray errorResponse = reply->readAll();
        qDebug() << "Error response:" << errorResponse;
        
        setLoadingState(false);
        
        // Try next model if available and this is a model-related error
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if ((httpStatus == 400 || httpStatus == 404) && m_retryCount < 3 && (m_currentModelIndex + 1) < m_availableModels.size()) {
            qDebug() << "Trying next model due to error";
            tryNextModel();
            return;
        }
        
        QString errorMsg = QString("Network error: %1").arg(reply->errorString());
        QMessageBox::warning(this, "API Error", errorMsg);
    }, Qt::QueuedConnection);
}

void AIPredictionDialog::handlePredictionResponse()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << "No reply object in handlePredictionResponse";
        setLoadingState(false);
        return;
    }
    
    // Ensure proper cleanup
    reply->deleteLater();
    setLoadingState(false);
    
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Groq API response received. Error:" << reply->error();
    qDebug() << "HTTP Status Code:" << httpStatus;
    
    if (reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        qDebug() << "API Error:" << reply->errorString();
        qDebug() << "Error Response:" << errorData;
        
        // Check if we should retry with next model
        if ((httpStatus == 400 || httpStatus == 404 || httpStatus == 500 || httpStatus == 503) 
            && m_retryCount < 3 && (m_currentModelIndex + 1) < m_availableModels.size()) {
            qDebug() << "Retrying with next model due to HTTP error:" << httpStatus;
            tryNextModel();
            return;
        }
        
        QString errorMessage = reply->errorString();
        
        // Try to parse error response for more details
        if (!errorData.isEmpty()) {
            QJsonParseError parseError;
            QJsonDocument errorDoc = QJsonDocument::fromJson(errorData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                QJsonObject errorObj = errorDoc.object();
                if (errorObj.contains("error")) {
                    QJsonObject error = errorObj["error"].toObject();
                    if (error.contains("message")) {
                        errorMessage = error["message"].toString();
                    }
                }
            }
        }
        
        QMessageBox::warning(this, "API Error", 
            QString("Failed to get prediction: %1\n\nPlease check:\n- Your internet connection\n- API key is valid\n- Groq service is available").arg(errorMessage));
        return;
    }
    
    QByteArray responseData = reply->readAll();
    qDebug() << "Response data size:" << responseData.size();
    qDebug() << "Response content:" << QString::fromUtf8(responseData.left(500)) << "..."; // Log first 500 chars
    
    if (responseData.isEmpty()) {
        // Retry with next model if response is empty
        if (m_retryCount < 3 && (m_currentModelIndex + 1) < m_availableModels.size()) {
            qDebug() << "Empty response, trying next model";
            tryNextModel();
            return;
        }
        QMessageBox::warning(this, "API Error", "Empty response from Groq API");
        return;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << parseError.errorString();
        qDebug() << "Raw response:" << responseData;
        
        // Retry with next model if JSON is invalid
        if (m_retryCount < 3 && (m_currentModelIndex + 1) < m_availableModels.size()) {
            qDebug() << "JSON parse error, trying next model";
            tryNextModel();
            return;
        }
        
        QMessageBox::warning(this, "API Error", 
            QString("Invalid JSON response: %1").arg(parseError.errorString()));
        return;
    }
    
    QJsonObject response = doc.object();
    
    if (response.contains("error")) {
        QJsonObject error = response["error"].toObject();
        QString errorMessage = error["message"].toString();
        qDebug() << "API returned error:" << errorMessage;
        
        // Retry with next model for certain error types
        QString errorType = error["type"].toString();
        if ((errorType == "invalid_request_error" || errorMessage.contains("model")) 
            && m_retryCount < 3 && (m_currentModelIndex + 1) < m_availableModels.size()) {
            qDebug() << "Model-related error, trying next model";
            tryNextModel();
            return;
        }
        
        QMessageBox::warning(this, "API Error", 
            QString("Groq API Error: %1").arg(errorMessage));
        return;
    }
    
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            QJsonObject message = choice["message"].toObject();
            QString content = message["content"].toString();
            
            qDebug() << "Parsing prediction results...";
            // Parse and display the prediction results
            parsePredictionResults(content);
        } else {
            QMessageBox::warning(this, "API Error", "No choices in API response");
        }
    } else {
        qDebug() << "Response object keys:" << response.keys();
        qDebug() << "Full response:" << doc.toJson(QJsonDocument::Indented);
        QMessageBox::warning(this, "API Error", "Invalid response format from Groq API");
    }
}

void AIPredictionDialog::parsePredictionResults(const QString &jsonResponse)
{
    // Try to extract JSON from the response
    QString cleanJson = jsonResponse;
    int jsonStart = cleanJson.indexOf('{');
    int jsonEnd = cleanJson.lastIndexOf('}');
    
    if (jsonStart >= 0 && jsonEnd > jsonStart) {
        cleanJson = cleanJson.mid(jsonStart, jsonEnd - jsonStart + 1);
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(cleanJson.toUtf8(), &error);
      if (error.error != QJsonParseError::NoError) {
        // If JSON parsing fails, display raw response in insights
        QString formattedResponse = jsonResponse;
        formattedResponse.replace("\n", "<br>");
        m_insightsTextEdit->setHtml(QString("<h3>AI Prediction Results</h3><p>%1</p>")
            .arg(formattedResponse));
        m_resultsTabWidget->setCurrentIndex(2); // Switch to insights tab
        m_exportButton->setEnabled(true);
        return;
    }
    
    QJsonObject result = doc.object();
    
    // Update chart
    updateChart(result);
    
    // Update table
    updateTable(result);
    
    // Update insights
    updateInsights(result);
    
    m_exportButton->setEnabled(true);
    m_statusLabel->setText("✅ Prediction Complete");
    m_statusLabel->setStyleSheet("color: #4EC9B0; background-color: rgba(78, 201, 176, 0.1);");
}

void AIPredictionDialog::updateChart(const QJsonObject &result)
{
    // Delete existing chart to prevent memory leaks
    if (m_chartView->chart()) {
        delete m_chartView->chart();
    }
    
    QChart *chart = new QChart();
    chart->setTheme(QChart::ChartThemeDark);
    chart->setBackgroundBrush(QBrush(QColor("#2d2d30")));
    chart->setTitleBrush(QBrush(QColor("#ffffff")));
    
    QString title = "Prediction Results";
    if (result.contains("prediction_type")) {
        title = result["prediction_type"].toString();
    }
    chart->setTitle(title);
    
    try {
        // Check if we have chart data
        if (result.contains("charts")) {
            QJsonObject chartData = result["charts"].toObject();
            QString chartType = chartData["chart_type"].toString().toLower();
            
            if (chartType == "line" || chartType.isEmpty()) {
                createLineChart(chart, result);
            } else if (chartType == "bar") {
                createBarChart(chart, result);
            } else {
                createSampleChart(chart);
            }
        } else if (result.contains("forecast_data")) {
            // Create line chart from forecast data
            createLineChart(chart, result);
        } else {
            // Create a simple chart with sample data
            createSampleChart(chart);
        }
        
        m_chartView->setChart(chart);
        qDebug() << "Chart updated successfully";
        
    } catch (const std::exception &e) {
        qDebug() << "Exception in chart creation:" << e.what();
        delete chart;
        // Create a simple fallback chart
        QChart *fallbackChart = new QChart();
        fallbackChart->setTitle("Chart Creation Failed");
        m_chartView->setChart(fallbackChart);
    }
}

void AIPredictionDialog::createLineChart(QChart *chart, const QJsonObject &result)
{
    QLineSeries *series = new QLineSeries();
    series->setName("Forecast");
    series->setColor(QColor("#007ACC"));
    
    QStringList categories;
    
    if (result.contains("forecast_data")) {
        QJsonArray forecastData = result["forecast_data"].toArray();
        for (int i = 0; i < forecastData.size(); i++) {
            QJsonObject point = forecastData[i].toObject();
            QString period = point["period"].toString();
            double value = point["value"].toDouble();
            
            series->append(i, value);
            categories << period;
        }
    } else if (result.contains("charts")) {
        QJsonObject chartData = result["charts"].toObject();
        QJsonArray dataPoints = chartData["data_points"].toArray();
        for (int i = 0; i < dataPoints.size(); i++) {
            QJsonObject point = dataPoints[i].toObject();
            QString label = point["label"].toString();
            double value = point["value"].toDouble();
            
            series->append(i, value);
            categories << label;
        }
    }
    
    chart->addSeries(series);
    
    // Setup axes
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value");
    axisY->setLabelFormat("%.2f");
    axisY->setLabelsColor(QColor("#ffffff"));
    axisY->setTitleBrush(QBrush(QColor("#ffffff")));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(QColor("#ffffff"));
    axisX->setTitleBrush(QBrush(QColor("#ffffff")));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
}

void AIPredictionDialog::createBarChart(QChart *chart, const QJsonObject &result)
{
    QBarSeries *series = new QBarSeries();
    QBarSet *set = new QBarSet("Values");
    set->setColor(QColor("#007ACC"));
    
    QStringList categories;
    
    if (result.contains("charts")) {
        QJsonObject chartData = result["charts"].toObject();
        QJsonArray dataPoints = chartData["data_points"].toArray();
        for (const QJsonValue &value : dataPoints) {
            QJsonObject point = value.toObject();
            QString label = point["label"].toString();
            double val = point["value"].toDouble();
            
            *set << val;
            categories << label;
        }
    }
    
    series->append(set);
    chart->addSeries(series);
    
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(QColor("#ffffff"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelsColor(QColor("#ffffff"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
}

void AIPredictionDialog::createSampleChart(QChart *chart)
{
    QLineSeries *series = new QLineSeries();
    series->setName("Sample Forecast");
    series->setColor(QColor("#007ACC"));
      // Generate sample data
    QRandomGenerator *generator = QRandomGenerator::global();
    for (int i = 0; i < 6; i++) {
        series->append(i, 1000 + (generator->bounded(500)));
    }
    
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setLabelsColor(QColor("#ffffff"));
    chart->axes(Qt::Vertical).first()->setLabelsColor(QColor("#ffffff"));
}

void AIPredictionDialog::updateTable(const QJsonObject &result)
{
    m_resultsTable->clear();
    
    if (result.contains("forecast_data")) {
        QJsonArray forecastData = result["forecast_data"].toArray();
        
        m_resultsTable->setRowCount(forecastData.size());
        m_resultsTable->setColumnCount(4);
        
        QStringList headers = {"Period", "Predicted Value", "Confidence", "Notes"};
        m_resultsTable->setHorizontalHeaderLabels(headers);
        
        for (int i = 0; i < forecastData.size(); i++) {
            QJsonObject point = forecastData[i].toObject();
            
            m_resultsTable->setItem(i, 0, new QTableWidgetItem(point["period"].toString()));
            m_resultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(point["value"].toDouble(), 'f', 2)));
            m_resultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(point["confidence"].toDouble()) + "%"));
            m_resultsTable->setItem(i, 3, new QTableWidgetItem(point["notes"].toString()));
        }
    } else {
        // Show summary table
        m_resultsTable->setRowCount(1);
        m_resultsTable->setColumnCount(2);
        m_resultsTable->setHorizontalHeaderLabels({"Metric", "Value"});
        m_resultsTable->setItem(0, 0, new QTableWidgetItem("Status"));
        m_resultsTable->setItem(0, 1, new QTableWidgetItem("Prediction generated successfully"));
    }
    
    m_resultsTable->resizeColumnsToContents();
}

void AIPredictionDialog::updateInsights(const QJsonObject &result)
{
    QString html = "<div style='font-family: Segoe UI; color: #ffffff; line-height: 1.6;'>";
    
    // Summary
    if (result.contains("summary")) {
        html += QString("<h2 style='color: #007ACC; margin-bottom: 15px;'>📊 Executive Summary</h2>");
        html += QString("<p style='margin-bottom: 20px; background-color: #3c3c3c; padding: 15px; border-radius: 8px;'>%1</p>")
            .arg(result["summary"].toString());
    }
    
    // Key Insights
    if (result.contains("key_insights")) {
        html += "<h3 style='color: #4EC9B0; margin-bottom: 10px;'>🧠 Key Insights</h3>";
        html += "<ul style='margin-bottom: 20px;'>";
        QJsonArray insights = result["key_insights"].toArray();
        for (const QJsonValue &insight : insights) {
            html += QString("<li style='margin-bottom: 8px; padding: 5px; background-color: rgba(78, 201, 176, 0.1); border-left: 3px solid #4EC9B0; padding-left: 10px;'>%1</li>")
                .arg(insight.toString());
        }
        html += "</ul>";
    }
    
    // Recommendations
    if (result.contains("recommendations")) {
        html += "<h3 style='color: #DCDCAA; margin-bottom: 10px;'>💡 Recommendations</h3>";
        QJsonArray recommendations = result["recommendations"].toArray();
        for (const QJsonValue &rec : recommendations) {
            QJsonObject recommendation = rec.toObject();
            QString priority = recommendation["priority"].toString();
            QString priorityColor = (priority == "High") ? "#f48771" : (priority == "Medium") ? "#DCDCAA" : "#4EC9B0";
            
            html += QString(R"(
                <div style='margin-bottom: 15px; padding: 15px; background-color: #3c3c3c; border-radius: 8px; border-left: 4px solid %1;'>
                    <div style='font-weight: bold; color: %1; margin-bottom: 5px;'>%2 Priority</div>
                    <div style='font-weight: bold; margin-bottom: 5px;'>Action: %3</div>
                    <div style='color: #cccccc; margin-bottom: 5px;'>Impact: %4</div>
                    <div style='color: #9cdcfe; font-size: 12px;'>Timeline: %5</div>
                </div>
            )")
                .arg(priorityColor)
                .arg(priority)
                .arg(recommendation["action"].toString())
                .arg(recommendation["impact"].toString())
                .arg(recommendation["timeline"].toString());
        }
    }
    
    // Risk Factors
    if (result.contains("risk_factors")) {
        html += "<h3 style='color: #f48771; margin-bottom: 10px;'>⚠️ Risk Factors</h3>";
        QJsonArray risks = result["risk_factors"].toArray();
        for (const QJsonValue &riskVal : risks) {
            QJsonObject risk = riskVal.toObject();
            QString probability = risk["probability"].toString();
            QString probColor = (probability == "High") ? "#f48771" : (probability == "Medium") ? "#DCDCAA" : "#4EC9B0";
            
            html += QString(R"(
                <div style='margin-bottom: 15px; padding: 15px; background-color: rgba(244, 135, 113, 0.1); border-radius: 8px; border-left: 4px solid #f48771;'>
                    <div style='font-weight: bold; margin-bottom: 5px;'>%1</div>
                    <div style='margin-bottom: 5px;'><span style='color: %2; font-weight: bold;'>%3 Probability</span> - %4</div>
                    <div style='color: #cccccc; font-size: 12px;'><strong>Mitigation:</strong> %5</div>
                </div>
            )")
                .arg(risk["risk"].toString())
                .arg(probColor)
                .arg(probability)
                .arg(risk["impact"].toString())
                .arg(risk["mitigation"].toString());
        }
    }
    
    html += "</div>";
    m_insightsTextEdit->setHtml(html);
}

void AIPredictionDialog::refreshData()
{
    qDebug() << "Refreshing prediction data...";
    loadMaterialCategories();
    clearResults();
}

void AIPredictionDialog::exportResults()
{
    qDebug() << "Exporting prediction results...";
    // TODO: Implement export functionality
    QMessageBox::information(this, "Export", "Export functionality will be implemented in a future version.");
}

void AIPredictionDialog::onTabChanged(int index)
{
    qDebug() << "Tab changed to index:" << index;
    // Handle tab change logic if needed
}

void AIPredictionDialog::onParameterChanged()
{
    qDebug() << "Parameters changed, clearing previous results";
    clearResults();
}

void AIPredictionDialog::onPredictionCompleted(const QString &response)
{
    qDebug() << "Prediction completed";
    setLoadingState(false);
    parsePredictionResults(response);
}

void AIPredictionDialog::onPredictionError(const QString &error)
{
    qDebug() << "Prediction error:" << error;
    setLoadingState(false);
    QMessageBox::warning(this, "Prediction Error", error);
}

void AIPredictionDialog::updateCharts()
{
    qDebug() << "Updating charts with current data";
    // Refresh charts with current results
    if (!m_currentResults.isEmpty()) {
        updateChart(m_currentResults);
    }
}

void AIPredictionDialog::animateResults()
{
    qDebug() << "Animating results display";
    // TODO: Implement result animation
}

void AIPredictionDialog::setLoadingState(bool loading)
{
    m_isLoading = loading;
    m_generateButton->setEnabled(!loading);
    m_loadingOverlay->setVisible(loading);
    
    if (loading) {
        m_statusLabel->setText("🔄 Generating...");
        m_statusLabel->setStyleSheet("color: #DCDCAA; background-color: rgba(220, 220, 170, 0.1);");
        
        // Resize overlay to match parent
        if (m_loadingOverlay->parent()) {
            QWidget *parent = qobject_cast<QWidget*>(m_loadingOverlay->parent());
            if (parent) {
                m_loadingOverlay->resize(parent->size());
            }
        }
    }
}

void AIPredictionDialog::clearResults()
{
    // Clear chart
    QChart *emptyChart = new QChart();
    emptyChart->setTitle("Select prediction type to generate forecast");
    emptyChart->setTheme(QChart::ChartThemeDark);
    emptyChart->setBackgroundBrush(QBrush(QColor("#2d2d30")));
    emptyChart->setTitleBrush(QBrush(QColor("#ffffff")));
    m_chartView->setChart(emptyChart);
    
    // Clear table
    m_resultsTable->clear();
    m_resultsTable->setRowCount(0);
    
    // Clear insights
    m_insightsTextEdit->clear();
    m_insightsTextEdit->setPlaceholderText("AI insights will appear here after generating predictions...");
    
    m_exportButton->setEnabled(false);
}

void AIPredictionDialog::showApiKeyDialog()
{
    // TODO: Implement API key configuration dialog (similar to AI Assistant)
    QMessageBox::information(this, "API Key Required", 
        "Please configure your Groq API key in the .env file or environment variables.");
}

void AIPredictionDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (m_loadingOverlay && m_loadingOverlay->isVisible()) {
        m_loadingOverlay->resize(size());
    }
}

void AIPredictionDialog::onPredictionTypeChanged(int index)
{
    Q_UNUSED(index)
    // Clear any existing results when prediction type changes
    clearResults();
    
    // Update status to reflect the change
    if (m_statusLabel) {
        m_statusLabel->setText("Ready - Prediction type changed");
    }
    
    qDebug() << "Prediction type changed to index:" << index;
}

void AIPredictionDialog::onQuickInsightToggled(bool checked)
{
    QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
    if (!senderBtn) return;
    
    // Update button style based on toggle state
    if (checked) {
        senderBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #007ACC;"
            "  color: white;"
            "  border: 1px solid #007ACC;"
            "}"
        );
    } else {
        senderBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: transparent;"
            "  color: #007ACC;"
            "  border: 1px solid #007ACC;"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(0, 122, 204, 0.1);"
            "}"
        );
    }
    
    qDebug() << "Quick insight toggled:" << senderBtn->text() << "checked:" << checked;
}
