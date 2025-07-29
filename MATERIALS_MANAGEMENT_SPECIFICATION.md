# Materials Management Feature - Complete Implementation Specification

> **Version:** 2.0  
> **Last Updated:** June 4, 2025  
> **Target Framework:** Qt 6.5+  
> **Language:** C++ 17+  
> **Database:** SQLite 3.x  

---

## 📋 Table of Contents

1. [Project Overview](#project-overview)
2. [Dependencies & Setup](#dependencies--setup)
3. [Database Architecture](#database-architecture)
4. [API Configuration](#api-configuration)
5. [Core Architecture](#core-architecture)
6. [Implementation Specifications](#implementation-specifications)
7. [Feature Requirements](#feature-requirements)
8. [Security & Performance](#security--performance)
9. [UI/UX Guidelines](#uiux-guidelines)
10. [Testing Strategy](#testing-strategy)
11. [Deployment Guide](#deployment-guide)
12. [Advanced Troubleshooting](#advanced-troubleshooting)

---

## 🎯 Project Overview

The Materials Management System is a comprehensive Qt-based application that provides intelligent inventory management with AI-powered assistance. It combines traditional CRUD operations with modern AI capabilities for enhanced user experience and productivity.

### Key Features
- **Smart CRUD Operations** with real-time validation
- **AI-Powered Assistant** using Groq API integration
- **Advanced Search & Filtering** with natural language processing
- **CSV Import/Export** with intelligent data mapping
- **Real-time Analytics** with interactive charts
- **Modern Material Design** UI with responsive layout

---

## 📦 Dependencies & Setup

### CMakeLists.txt Configuration

```cmake
cmake_minimum_required(VERSION 3.19)
project(Materials LANGUAGES CXX)

# Qt Configuration
find_package(Qt6 6.5 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Sql 
    Charts 
    PrintSupport 
    Network
    WebEngineWidgets  # For advanced PDF rendering
    Concurrent       # For background operations
)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler-specific flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Qt Setup
qt_standard_project_setup()

# Source Files
qt_add_executable(Materials
    WIN32 MACOSX_BUNDLE
    # Core Application
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    
    # Models
    materialmodel.cpp materialmodel.h
    
    # Database Layer
    database/databasemanager.cpp database/databasemanager.h
    
    # AI Integration
    groqclient.cpp groqclient.h
    materialai.cpp materialai.h
    materialchatbot.cpp materialchatbot.h
    aipredictionwidget.cpp aipredictionwidget.h
    
    # Custom Widgets
    QRangeSlider.cpp QRangeSlider.h
    
    # Controllers (Optional)
    controllers/materialcontroller.cpp controllers/materialcontroller.h
    controllers/searchcontroller.cpp controllers/searchcontroller.h
    
    # Utilities
    utils/csvimporter.cpp utils/csvimporter.h
    utils/pdfexporter.cpp utils/pdfexporter.h
    utils/validator.cpp utils/validator.h
    
    # Resources
    resources.qrc
)

# Linking
target_link_libraries(Materials PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Sql
    Qt::Charts
    Qt::PrintSupport
    Qt::Network
    Qt::WebEngineWidgets
    Qt::Concurrent
)

# Include Directories
target_include_directories(Materials PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/database
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Installation
include(GNUInstallDirs)
install(TARGETS Materials
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
```

### Required External Dependencies

```bash
# Qt 6.5+ Installation (Windows with vcpkg)
vcpkg install qt6[core,widgets,sql,charts,printsupport,network,webengine,concurrent]:x64-windows

# Alternative: Qt Online Installer
# Download from https://www.qt.io/download-qt-installer

# SQLite (usually included with Qt)
# OpenSSL for secure API communications
vcpkg install openssl:x64-windows

# Optional: For enhanced PDF generation
vcpkg install poppler:x64-windows
```

---

## 🗄️ Database Architecture

### SQLite Schema Design

#### Materials Table
```sql
CREATE TABLE IF NOT EXISTS materials (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT,
    category TEXT NOT NULL,
    quantity INTEGER NOT NULL DEFAULT 0,
    unit TEXT NOT NULL DEFAULT 'pcs',
    price REAL NOT NULL DEFAULT 0.0,
    supplier_id INTEGER,
    barcode TEXT,
    location TEXT,
    minimum_stock INTEGER DEFAULT 0,
    maximum_stock INTEGER DEFAULT 1000,
    reorder_point INTEGER DEFAULT 10,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    created_by TEXT DEFAULT 'system',
    updated_by TEXT DEFAULT 'system',
    status TEXT DEFAULT 'active' CHECK(status IN ('active', 'inactive', 'discontinued')),
    FOREIGN KEY (supplier_id) REFERENCES suppliers(id)
);
```

#### Categories Table
```sql
CREATE TABLE IF NOT EXISTS categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT,
    parent_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (parent_id) REFERENCES categories(id)
);
```

#### Suppliers Table
```sql
CREATE TABLE IF NOT EXISTS suppliers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    contact_person TEXT,
    email TEXT,
    phone TEXT,
    address TEXT,
    website TEXT,
    rating REAL DEFAULT 0.0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    status TEXT DEFAULT 'active' CHECK(status IN ('active', 'inactive'))
);
```

#### Inventory Transactions Table
```sql
CREATE TABLE IF NOT EXISTS inventory_transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    material_id INTEGER NOT NULL,
    transaction_type TEXT NOT NULL CHECK(transaction_type IN ('IN', 'OUT', 'ADJUSTMENT')),
    quantity INTEGER NOT NULL,
    unit_price REAL,
    total_value REAL,
    reference_document TEXT,
    notes TEXT,
    transaction_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    created_by TEXT DEFAULT 'system',
    FOREIGN KEY (material_id) REFERENCES materials(id)
);
```

#### AI Interactions Log Table
```sql
CREATE TABLE IF NOT EXISTS ai_interactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_query TEXT NOT NULL,
    ai_response TEXT,
    query_type TEXT, -- 'search', 'create', 'update', 'analysis'
    material_ids TEXT, -- JSON array of related material IDs
    processing_time_ms INTEGER,
    success BOOLEAN DEFAULT 1,
    error_message TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### Database Indexes for Performance
```sql
-- Performance indexes
CREATE INDEX IF NOT EXISTS idx_materials_name ON materials(name);
CREATE INDEX IF NOT EXISTS idx_materials_category ON materials(category);
CREATE INDEX IF NOT EXISTS idx_materials_quantity ON materials(quantity);
CREATE INDEX IF NOT EXISTS idx_materials_status ON materials(status);
CREATE INDEX IF NOT EXISTS idx_materials_created_at ON materials(created_at);
CREATE INDEX IF NOT EXISTS idx_transactions_material_id ON inventory_transactions(material_id);
CREATE INDEX IF NOT EXISTS idx_transactions_date ON inventory_transactions(transaction_date);
```

---

## 🔑 API Configuration

### Groq API Setup

#### Environment Variables
```bash
# Required Environment Variables
GROQ_API_KEY=your_groq_api_key_here
GROQ_BASE_URL=https://api.groq.com/openai/v1
GROQ_MODEL=llama-3.3-70b-versatile
GROQ_TIMEOUT=30000
GROQ_MAX_TOKENS=4096
GROQ_TEMPERATURE=0.7

# Optional Configuration
MATERIALS_DB_PATH=./data/materials.db
MATERIALS_LOG_LEVEL=INFO
MATERIALS_BACKUP_INTERVAL=3600
```

#### API Request Configuration
```cpp
// GroqClient Configuration Structure
struct GroqConfig {
    QString apiKey;
    QString baseUrl = "https://api.groq.com/openai/v1";
    QString model = "llama-3.3-70b-versatile";
    int timeout = 30000;
    int maxTokens = 4096;
    double temperature = 0.7;
    bool enableRetry = true;
    int maxRetries = 3;
    int retryDelay = 1000; // milliseconds
};
```

---

## 🏗️ Core Architecture

### Architecture Patterns

1. **Model-View-Controller (MVC)** - Clear separation of concerns
2. **Repository Pattern** - Database abstraction layer
3. **Observer Pattern** - Real-time UI updates
4. **Strategy Pattern** - Pluggable AI providers
5. **Factory Pattern** - Widget creation and management

### Directory Structure
```
Materials/
├── main.cpp                    # Application entry point
├── CMakeLists.txt             # Build configuration
├── resources.qrc             # Qt resources
├── mainwindow.{cpp,h,ui}      # Main application window
├── database/                  # Database layer
│   ├── databasemanager.{cpp,h}
│   └── migrations/           # Database version management
├── models/                   # Data models
│   ├── materialmodel.{cpp,h}
│   ├── categorymodel.{cpp,h}
│   └── suppliermodel.{cpp,h}
├── controllers/              # Business logic
│   ├── materialcontroller.{cpp,h}
│   ├── searchcontroller.{cpp,h}
│   └── importexportcontroller.{cpp,h}
├── views/                    # UI components
│   ├── materialview.{cpp,h}
│   ├── searchwidget.{cpp,h}
│   └── dialogs/
├── ai/                       # AI integration
│   ├── groqclient.{cpp,h}
│   ├── materialai.{cpp,h}
│   ├── materialchatbot.{cpp,h}
│   └── aipredictionwidget.{cpp,h}
├── utils/                    # Utility classes
│   ├── csvimporter.{cpp,h}
│   ├── pdfexporter.{cpp,h}
│   ├── validator.{cpp,h}
│   └── style.qss
├── widgets/                  # Custom widgets
│   └── QRangeSlider.{cpp,h}
├── icons/                    # Application icons
├── data/                     # Database and config files
└── tests/                    # Unit and integration tests
```

---

## 🚀 Implementation Specifications

### MaterialModel Class (QSqlTableModel Extended)

```cpp
class MaterialModel : public QSqlTableModel {
    Q_OBJECT

public:
    enum MaterialColumns {
        IdColumn = 0,
        NameColumn,
        DescriptionColumn,
        CategoryColumn,
        QuantityColumn,
        UnitColumn,
        PriceColumn,
        SupplierColumn,
        BarcodeColumn,
        LocationColumn,
        MinStockColumn,
        MaxStockColumn,
        ReorderPointColumn,
        CreatedAtColumn,
        UpdatedAtColumn,
        StatusColumn
    };

    enum MaterialRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        CategoryRole,
        QuantityRole,
        PriceRole,
        TotalValueRole,
        StockStatusRole,
        LastUpdatedRole
    };

    explicit MaterialModel(QObject *parent = nullptr);

    // QAbstractItemModel overrides
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Custom methods
    bool initialize();
    void refreshData();
    
    // Advanced filtering
    void setNameFilter(const QString &pattern);
    void setCategoryFilter(const QString &category);
    void setQuantityRange(int min, int max);
    void setPriceRange(double min, double max);
    void setStockStatusFilter(const QStringList &statuses);
    void clearAllFilters();
    
    // CRUD with validation
    bool addMaterial(const MaterialData &data, QString &errorMessage);
    bool updateMaterial(int id, const MaterialData &data, QString &errorMessage);
    bool deleteMaterial(int id, QString &errorMessage);
    bool canDeleteMaterial(int id, QString &reason);
    
    // Bulk operations
    bool importMaterials(const QList<MaterialData> &materials, QStringList &errors);
    bool exportMaterials(const QString &filePath, const QModelIndexList &selection = {});
    
    // Analytics
    QJsonObject getInventoryStatistics();
    QList<QPair<QString, double>> getCategoryDistribution();
    QList<QPair<QString, int>> getLowStockItems();
    QList<QPair<QString, double>> getTopValueItems(int limit = 10);
    
    // AI Integration helpers
    QJsonArray getMaterialsAsJson(const QModelIndexList &selection = {});
    QString getMaterialsSummary();
    QStringList getSimilarMaterials(const QString &name, int limit = 5);

signals:
    void dataChanged();
    void materialAdded(int id);
    void materialUpdated(int id);
    void materialDeleted(int id);
    void lowStockAlert(int materialId, const QString &materialName, int currentStock, int reorderPoint);
    void importProgress(int current, int total);
    void exportProgress(int current, int total);

private slots:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    bool validateMaterialData(const MaterialData &data, QString &errorMessage);
    void checkStockLevels();
    QString buildFilterString();
    
    // Filter state
    QString m_nameFilter;
    QString m_categoryFilter;
    int m_minQuantity = 0;
    int m_maxQuantity = INT_MAX;
    double m_minPrice = 0.0;
    double m_maxPrice = DBL_MAX;
    QStringList m_stockStatusFilter;
};

// Material data structure
struct MaterialData {
    int id = -1;
    QString name;
    QString description;
    QString category;
    int quantity = 0;
    QString unit = "pcs";
    double price = 0.0;
    int supplierId = -1;
    QString barcode;
    QString location;
    int minStock = 0;
    int maxStock = 1000;
    int reorderPoint = 10;
    QString status = "active";
    
    bool isValid() const {
        return !name.isEmpty() && !category.isEmpty() && 
               quantity >= 0 && price >= 0.0;
    }
    
    QJsonObject toJson() const;
    static MaterialData fromJson(const QJsonObject &json);
};
```

### GroqClient Class (Advanced AI Integration)

```cpp
class GroqClient : public QObject {
    Q_OBJECT

public:
    enum RequestType {
        ChatRequest,
        MaterialAnalysis,
        SearchSuggestion,
        InventoryPrediction,
        DataValidation,
        OptimizationSuggestion,
        AnomalyDetection
    };

    enum ResponseFormat {
        PlainText,
        StructuredJson,
        MaterialObject,
        ActionList
    };

    explicit GroqClient(QObject *parent = nullptr);
    ~GroqClient();

    // Configuration
    void setConfiguration(const GroqConfig &config);
    GroqConfig configuration() const;
    bool isConfigured() const;
    
    // Connection management
    void testConnection();
    bool isConnected() const;
    void setNetworkTimeout(int milliseconds);
    
    // Core AI methods
    QString sendSynchronousRequest(const QString &prompt, RequestType type = ChatRequest);
    void sendAsynchronousRequest(const QString &prompt, RequestType type = ChatRequest, 
                               ResponseFormat format = PlainText, const QJsonObject &context = {});
    
    // Material-specific AI operations
    void analyzeInventoryTrends(const QJsonArray &historicalData);
    void generateReorderSuggestions(const QJsonArray &currentInventory);
    void validateMaterialData(const QJsonObject &materialData);
    void suggestCategorization(const QString &materialName, const QString &description);
    void estimateProjectRequirements(const QString &projectDescription, double area);
    void detectPriceAnomalies(const QJsonArray &materialPrices);
    void optimizeStockLevels(const QJsonObject &usagePatterns);
    
    // Natural language processing
    void parseSearchQuery(const QString &naturalLanguageQuery);
    void convertToMaterialData(const QString &description);
    void generateMaterialDescription(const QJsonObject &materialSpecs);
    
    // Batch operations
    void processBatchRequests(const QList<QPair<QString, RequestType>> &requests);
    void cancelAllRequests();
    void cancelRequest(const QString &requestId);

signals:
    // Connection signals
    void connectionTestCompleted(bool success, const QString &message);
    void configurationChanged();
    
    // Response signals
    void responseReceived(const QString &requestId, const QJsonObject &response);
    void errorOccurred(const QString &requestId, const QString &error);
    void requestProgress(const QString &requestId, int percentage);
    void batchCompleted(const QJsonArray &results);
    
    // Specific response types
    void materialAnalysisReceived(const QJsonObject &analysis);
    void searchSuggestionsReceived(const QStringList &suggestions);
    void inventoryPredictionsReceived(const QJsonObject &predictions);
    void validationResultReceived(bool isValid, const QStringList &issues);
    void optimizationSuggestionsReceived(const QJsonArray &suggestions);
    void anomaliesDetected(const QJsonArray &anomalies);
    
    // Material creation assistance
    void materialDataGenerated(const MaterialData &data);
    void categorizationSuggested(const QString &category, double confidence);
    void projectRequirementsEstimated(const QJsonArray &requirements);

private slots:
    void handleNetworkReply();
    void handleNetworkError(QNetworkReply::NetworkError error);
    void handleSslErrors(const QList<QSslError> &errors);
    void onRequestTimeout();

private:
    struct PendingRequest {
        QString id;
        RequestType type;
        ResponseFormat format;
        QJsonObject context;
        QDateTime timestamp;
        QNetworkReply *reply;
        QTimer *timeoutTimer;
    };

    QString generateRequestId();
    QNetworkRequest createApiRequest(const QString &endpoint);
    QJsonObject createChatPayload(const QString &prompt, const QString &systemPrompt = QString());
    void processResponse(const QString &requestId, const QJsonObject &response);
    QString formatPromptForMaterials(const QString &userPrompt, RequestType type, const QJsonObject &context);
    
    QNetworkAccessManager *m_networkManager;
    GroqConfig m_config;
    QHash<QString, PendingRequest> m_pendingRequests;
    bool m_isConnected = false;
    QTimer *m_connectionTestTimer;
    
    // System prompts for different operations
    static const QString MATERIAL_ANALYSIS_PROMPT;
    static const QString SEARCH_ASSISTANT_PROMPT;
    static const QString VALIDATION_PROMPT;
    static const QString OPTIMIZATION_PROMPT;
};
```

### MaterialChatbot Class (Intelligent UI Assistant)

```cpp
class MaterialChatbot : public QWidget {
    Q_OBJECT

public:
    enum ChatMode {
        GeneralAssistance,
        MaterialCreation,
        InventorySearch,
        DataAnalysis,
        TroubleShooting
    };

    explicit MaterialChatbot(QWidget *parent = nullptr);
    ~MaterialChatbot();

    // Configuration
    void setMaterialModel(MaterialModel *model);
    void setGroqClient(GroqClient *client);
    void setChatMode(ChatMode mode);
    
    // Chat management
    void clearChat();
    void addMessage(const QString &message, bool isUser = true);
    void addSystemMessage(const QString &message);
    void showTypingIndicator();
    void hideTypingIndicator();
    
    // Quick actions
    void addQuickAction(const QString &text, const QString &command);
    void removeQuickAction(const QString &text);
    void clearQuickActions();
    
    // Context management
    void setCurrentMaterial(int materialId);
    void setSelectedMaterials(const QList<int> &materialIds);
    void clearContext();

public slots:
    void sendMessage(const QString &message);
    void handleVoiceInput(); // Future feature
    void exportChatHistory();
    void importChatHistory();

signals:
    void materialCreationRequested(const MaterialData &data);
    void searchRequested(const QString &query);
    void materialUpdateRequested(int id, const QJsonObject &updates);
    void actionRequested(const QString &action, const QJsonObject &parameters);
    void helpRequested(const QString &topic);

private slots:
    void onSendButtonClicked();
    void onMessageReceived(const QString &requestId, const QJsonObject &response);
    void onQuickActionClicked();
    void onClearChatClicked();
    void onModeChanged();

private:
    void setupUi();
    void setupQuickActions();
    void processAiResponse(const QJsonObject &response);
    void executeCommand(const QString &command, const QJsonObject &parameters);
    QString formatContextInformation();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_chatScrollArea;
    QWidget *m_chatWidget;
    QVBoxLayout *m_chatLayout;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
    QPushButton *m_voiceButton;
    QComboBox *m_modeComboBox;
    QFlowLayout *m_quickActionsLayout;
    QLabel *m_typingIndicator;
    QPushButton *m_clearButton;
    QPushButton *m_exportButton;
    
    // Data
    MaterialModel *m_materialModel = nullptr;
    GroqClient *m_groqClient = nullptr;
    ChatMode m_currentMode = GeneralAssistance;
    QList<QPair<QString, bool>> m_chatHistory; // message, isUser
    QHash<QString, QString> m_quickActions; // text -> command
    int m_currentMaterialId = -1;
    QList<int> m_selectedMaterialIds;
    
    // Animation
    QPropertyAnimation *m_typingAnimation;
    QTimer *m_autoScrollTimer;
};
```

---

## 📦 Feature Requirements Implementation

### CRUD Operations (Enhanced)

#### Create Material
```cpp
// Enhanced material creation with AI assistance
class MaterialCreationDialog : public QDialog {
    Q_OBJECT

public:
    explicit MaterialCreationDialog(QWidget *parent = nullptr);
    
    void setAiClient(GroqClient *client);
    void prefillFromDescription(const QString &description);
    MaterialData getMaterialData() const;

private slots:
    void onAiAssistClicked();
    void onCategoryChanged();
    void onValidateFields();
    void onBarcodeScanned();

private:
    void setupValidation();
    void validateInRealTime();
    
    // AI-powered suggestions
    void suggestCategory();
    void suggestPricing();
    void validateDescription();
    
    QLineEdit *m_nameEdit;
    QTextEdit *m_descriptionEdit;
    QComboBox *m_categoryCombo;
    QSpinBox *m_quantitySpinBox;
    QComboBox *m_unitCombo;
    QDoubleSpinBox *m_priceSpinBox;
    QPushButton *m_aiAssistButton;
    QLabel *m_validationLabel;
    
    GroqClient *m_aiClient = nullptr;
    QTimer *m_validationTimer;
};
```

#### Advanced Search & Filter
```cpp
class AdvancedSearchWidget : public QWidget {
    Q_OBJECT

public:
    explicit AdvancedSearchWidget(QWidget *parent = nullptr);
    
    void setMaterialModel(MaterialModel *model);
    void setAiClient(GroqClient *client);

signals:
    void filterChanged();
    void aiSearchRequested(const QString &query);

private slots:
    void onSearchTextChanged();
    void onCategoryFilterChanged();
    void onPriceRangeChanged();
    void onQuantityRangeChanged();
    void onAdvancedToggled();
    void onAiSearchClicked();
    void onClearFiltersClicked();
    void onSaveSearchClicked();
    void onLoadSearchClicked();

private:
    void setupNaturalLanguageSearch();
    void applyCombinedFilters();
    void saveSearchPreset(const QString &name);
    void loadSearchPreset(const QString &name);
    
    // Basic search
    QLineEdit *m_searchEdit;
    QComboBox *m_categoryFilter;
    
    // Advanced filters
    QGroupBox *m_advancedGroup;
    QRangeSlider *m_priceRange;
    QRangeSlider *m_quantityRange;
    QCheckBox *m_lowStockOnly;
    QCheckBox *m_activeOnly;
    QDateEdit *m_createdAfter;
    QDateEdit *m_createdBefore;
    
    // AI search
    QLineEdit *m_aiSearchEdit;
    QPushButton *m_aiSearchButton;
    QLabel *m_aiSuggestionsLabel;
    
    // Search management
    QComboBox *m_savedSearches;
    QPushButton *m_saveSearchButton;
    QPushButton *m_clearButton;
    
    MaterialModel *m_model = nullptr;
    GroqClient *m_aiClient = nullptr;
    QTimer *m_searchDelayTimer;
};
```

#### Import/Export (Enhanced)
```cpp
class ImportExportManager : public QObject {
    Q_OBJECT

public:
    enum ImportFormat {
        CSV,
        Excel,
        JSON,
        XML
    };

    enum ExportFormat {
        CSV,
        Excel,
        PDF,
        JSON,
        XML
    };

    explicit ImportExportManager(QObject *parent = nullptr);
    
    void setMaterialModel(MaterialModel *model);
    void setAiClient(GroqClient *client);

    // Import operations
    bool importFromFile(const QString &filePath, ImportFormat format, QString &errorMessage);
    bool validateImportData(const QList<MaterialData> &data, QStringList &errors);
    void previewImportData(const QString &filePath, ImportFormat format);
    
    // Export operations
    bool exportToFile(const QString &filePath, ExportFormat format, 
                     const QModelIndexList &selection = {}, QString &errorMessage);
    bool exportWithTemplate(const QString &templatePath, const QString &outputPath);
    
    // AI-powered features
    void suggestDataMapping(const QStringList &headers);
    void cleanupImportData(QList<MaterialData> &data);
    void generateExportSummary(const QModelIndexList &selection);

signals:
    void importProgress(int current, int total, const QString &currentItem);
    void importCompleted(int successful, int failed, const QStringList &errors);
    void exportProgress(int current, int total);
    void exportCompleted(bool success, const QString &message);
    void dataMappingReady(const QHash<QString, QString> &mapping);
    void previewReady(const QList<MaterialData> &previewData);

private:
    bool importCSV(const QString &filePath, QString &errorMessage);
    bool importExcel(const QString &filePath, QString &errorMessage);
    bool importJSON(const QString &filePath, QString &errorMessage);
    
    bool exportCSV(const QString &filePath, const QModelIndexList &selection);
    bool exportPDF(const QString &filePath, const QModelIndexList &selection);
    bool exportJSON(const QString &filePath, const QModelIndexList &selection);
    
    QStringList detectEncoding(const QString &filePath);
    QChar detectDelimiter(const QString &filePath);
    
    MaterialModel *m_model = nullptr;
    GroqClient *m_aiClient = nullptr;
};
```

---

## 🛡️ Security & Performance

### Security Considerations

#### API Key Management
```cpp
class SecureConfig : public QObject {
    Q_OBJECT

public:
    static SecureConfig& instance();
    
    // Secure storage
    void setApiKey(const QString &key);
    QString getApiKey() const;
    void clearApiKey();
    
    // Encryption
    void setEncryptionKey(const QByteArray &key);
    QByteArray encryptData(const QByteArray &data) const;
    QByteArray decryptData(const QByteArray &encryptedData) const;
    
    // Session management
    void startSession();
    void endSession();
    bool isSessionValid() const;
    
private:
    explicit SecureConfig(QObject *parent = nullptr);
    
    QByteArray m_encryptionKey;
    QString m_sessionToken;
    QDateTime m_sessionExpiry;
};
```

#### Input Validation
```cpp
class InputValidator {
public:
    static bool validateMaterialName(const QString &name, QString &error);
    static bool validatePrice(double price, QString &error);
    static bool validateQuantity(int quantity, QString &error);
    static bool validateCategory(const QString &category, QString &error);
    static bool validateBarcode(const QString &barcode, QString &error);
    
    // SQL injection prevention
    static QString sanitizeInput(const QString &input);
    static bool containsSqlInjection(const QString &input);
    
    // XSS prevention for AI responses
    static QString sanitizeHtmlContent(const QString &content);
    
    // API input validation
    static bool validateApiResponse(const QJsonObject &response);
};
```

### Performance Optimization

#### Database Performance
```cpp
class DatabaseOptimizer {
public:
    static void createOptimalIndexes(QSqlDatabase &db);
    static void analyzePerformance(QSqlDatabase &db);
    static void optimizeQueries();
    static void enableWALMode(QSqlDatabase &db);
    static void configureCache(QSqlDatabase &db, int cacheSize = 2000);
    
    // Query optimization
    static QString optimizeFilterQuery(const QString &baseQuery, const QStringList &filters);
    static void prepareStatements(QSqlDatabase &db);
};
```

#### Memory Management
```cpp
class MemoryManager {
public:
    static void setupMemoryPools();
    static void clearUnusedCache();
    static void optimizeImageCache();
    static size_t getCurrentMemoryUsage();
    static void setMemoryLimit(size_t limitMB);
    
    // Model optimization
    static void enableLazyLoading(MaterialModel *model);
    static void configurePaging(MaterialModel *model, int pageSize = 1000);
};
```

---

## 🎨 UI/UX Guidelines

### Design System

#### Color Palette
```css
/* Primary Colors */
:root {
    --primary-dark: #3D485A;     /* Main background */
    --primary-light: #E3C6B0;    /* Text and accents */
    --accent-dark: #2A3340;      /* Secondary background */
    --accent-light: #D4B7A1;     /* Secondary text */
    
    /* Status Colors */
    --success: #4CAF50;          /* Success states */
    --warning: #FFA726;          /* Warning states */
    --error: #FF6B6B;            /* Error states */
    --info: #42A5F5;             /* Information */
    
    /* Gradients */
    --gradient-primary: linear-gradient(135deg, #3D485A 0%, #2A3340 100%);
    --gradient-accent: linear-gradient(135deg, #E3C6B0 0%, #D4B7A1 100%);
    
    /* Shadows */
    --shadow-light: 0 2px 4px rgba(0,0,0,0.1);
    --shadow-medium: 0 4px 8px rgba(0,0,0,0.15);
    --shadow-heavy: 0 8px 16px rgba(0,0,0,0.2);
}
```

#### Typography
```css
/* Font System */
.font-primary {
    font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif;
}

.font-mono {
    font-family: "Consolas", "Monaco", "Courier New", monospace;
}

/* Text Sizes */
.text-xs { font-size: 12px; }
.text-sm { font-size: 14px; }
.text-base { font-size: 16px; }
.text-lg { font-size: 18px; }
.text-xl { font-size: 20px; }
.text-2xl { font-size: 24px; }
.text-3xl { font-size: 30px; }
```

#### Component Specifications

```cpp
// Custom styling for enhanced user experience
class StyleManager {
public:
    static void applyMaterialDesign(QWidget *widget);
    static void applyDarkTheme(QApplication *app);
    static void setupAnimations(QWidget *widget);
    static void addElevation(QWidget *widget, int level = 1);
    static void addRippleEffect(QPushButton *button);
    
    // Responsive design
    static void makeResponsive(QWidget *widget);
    static void adjustForScreenSize(QWidget *widget, const QSize &screenSize);
    
    // Accessibility
    static void improveAccessibility(QWidget *widget);
    static void addKeyboardNavigation(QWidget *widget);
    static void setHighContrast(QWidget *widget, bool enabled);
};
```

---

## 🧪 Testing Strategy

### Unit Testing Framework
```cpp
class MaterialModelTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Model tests
    void testMaterialCreation();
    void testMaterialUpdate();
    void testMaterialDeletion();
    void testFilterFunctionality();
    void testValidationLogic();
    void testPerformanceWithLargeDataset();
    
    // AI integration tests
    void testGroqClientConnection();
    void testAiResponseParsing();
    void testAiRequestTimeout();
    void testAiErrorHandling();
    
    // Database tests
    void testDatabaseMigration();
    void testTransactionRollback();
    void testConcurrentAccess();
    void testDataIntegrity();
    
    // Import/Export tests
    void testCsvImport();
    void testPdfExport();
    void testDataValidation();
    void testLargeFileHandling();

private:
    QSqlDatabase m_testDb;
    MaterialModel *m_model;
    GroqClient *m_groqClient;
};
```

### Integration Testing
```cpp
class IntegrationTest : public QObject {
    Q_OBJECT

private slots:
    void testEndToEndWorkflow();
    void testAiMaterialCreation();
    void testBulkOperations();
    void testUserInterfaceFlow();
    void testErrorRecovery();
    void testPerformanceUnderLoad();
};
```

### Performance Benchmarks
```cpp
class PerformanceBenchmark : public QObject {
    Q_OBJECT

private slots:
    void benchmarkDatabaseQueries();
    void benchmarkUiRendering();
    void benchmarkAiResponseTime();
    void benchmarkMemoryUsage();
    void benchmarkStartupTime();

private:
    void measureExecutionTime(const std::function<void()> &function, const QString &testName);
    void profileMemoryUsage(const std::function<void()> &function, const QString &testName);
};
```

---

## 🚀 Deployment Guide

### Build Configuration

#### Release Build Script (Windows)
```batch
@echo off
echo Building Materials Management System...

REM Set environment variables
set QT_DIR=C:\Qt\6.5.0\msvc2022_64
set CMAKE_DIR=C:\Program Files\CMake\bin
set VCPKG_DIR=C:\vcpkg

REM Add to PATH
set PATH=%QT_DIR%\bin;%CMAKE_DIR%;%VCPKG_DIR%;%PATH%

REM Create build directory
if exist build rmdir /s /q build
mkdir build
cd build

REM Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release ^
         -DCMAKE_PREFIX_PATH=%QT_DIR% ^
         -DCMAKE_TOOLCHAIN_FILE=%VCPKG_DIR%\scripts\buildsystems\vcpkg.cmake

REM Build
cmake --build . --config Release --parallel

REM Deploy Qt dependencies
%QT_DIR%\bin\windeployqt.exe --release --qmldir .. Release\Materials.exe

echo Build completed successfully!
pause
```

#### Packaging Configuration
```cmake
# CPack configuration for installer creation
set(CPACK_PACKAGE_NAME "Materials Management System")
set(CPACK_PACKAGE_VERSION "2.0.0")
set(CPACK_PACKAGE_DESCRIPTION "AI-Powered Materials Management Solution")
set(CPACK_PACKAGE_VENDOR "Your Company")
set(CPACK_PACKAGE_CONTACT "support@yourcompany.com")

# Windows installer
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Materials Management System")
    set(CPACK_NSIS_PACKAGE_NAME "MaterialsManagement")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()

include(CPack)
```

### Database Migration System
```cpp
class DatabaseMigration {
public:
    static bool migrateToVersion(QSqlDatabase &db, int targetVersion);
    static int getCurrentVersion(QSqlDatabase &db);
    static bool backupDatabase(const QString &dbPath, const QString &backupPath);
    static bool restoreDatabase(const QString &backupPath, const QString &dbPath);
    
private:
    static bool runMigration(QSqlDatabase &db, int fromVersion, int toVersion);
    static QStringList getMigrationScripts(int fromVersion, int toVersion);
};
```

---

## 🔧 Advanced Troubleshooting

### Diagnostic Tools
```cpp
class DiagnosticManager : public QObject {
    Q_OBJECT

public:
    static DiagnosticManager& instance();
    
    // System diagnostics
    void runSystemCheck();
    void checkDatabaseHealth();
    void checkApiConnectivity();
    void checkMemoryUsage();
    void checkDiskSpace();
    
    // Performance monitoring
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    void generatePerformanceReport();
    
    // Error logging
    void logError(const QString &component, const QString &error, const QJsonObject &context = {});
    void exportLogs(const QString &filePath);
    void clearOldLogs(int daysToKeep = 30);
    
    // Automatic recovery
    void enableAutoRecovery(bool enabled);
    void recoverFromCorruptedDatabase();
    void resetToFactoryDefaults();

signals:
    void diagnosticCompleted(const QString &component, bool passed, const QString &details);
    void performanceAlert(const QString &metric, double value, double threshold);
    void errorLogged(const QString &component, const QString &error);
    void recoveryAttempted(const QString &component, bool successful);

private:
    void checkQtInstallation();
    void checkSqliteVersion();
    void checkNetworkConfiguration();
    void validateConfiguration();
    
    QTimer *m_performanceTimer;
    QJsonArray m_errorLog;
    bool m_autoRecoveryEnabled = true;
};
```

### Common Issues and Solutions

#### Database Issues
```cpp
// Database corruption recovery
bool DatabaseManager::repairDatabase() {
    QSqlDatabase db = QSqlDatabase::database();
    
    // Check integrity
    QSqlQuery query("PRAGMA integrity_check", db);
    if (query.exec() && query.next()) {
        QString result = query.value(0).toString();
        if (result != "ok") {
            // Attempt repair
            return performDatabaseRepair();
        }
    }
    
    return true;
}

// Connection pooling for high-load scenarios
class ConnectionPool {
public:
    static QSqlDatabase getConnection();
    static void returnConnection(QSqlDatabase db);
    static void setMaxConnections(int max);
    
private:
    static QQueue<QSqlDatabase> m_availableConnections;
    static QMutex m_connectionMutex;
    static int m_maxConnections;
};
```

#### AI Integration Issues
```cpp
// Robust error handling for AI requests
class AiErrorHandler {
public:
    static void handleApiError(const QString &error, const QJsonObject &context);
    static void implementFallbackResponse(const QString &requestType);
    static void cacheSuccessfulResponses(const QString &request, const QString &response);
    static QString getCachedResponse(const QString &request);
    
private:
    static QHash<QString, QString> m_responseCache;
    static QStringList m_fallbackResponses;
};
```

---

## 📝 Implementation Checklist

### Phase 1: Foundation (Week 1-2)
- [ ] Set up Qt project with CMake
- [ ] Configure database schema and migrations
- [ ] Implement basic MaterialModel class
- [ ] Create main window layout
- [ ] Set up unit testing framework
- [ ] Implement basic CRUD operations
- [ ] Add input validation
- [ ] Create database manager with connection pooling

### Phase 2: Core Features (Week 3-4)
- [ ] Implement advanced search and filtering
- [ ] Add CSV import/export functionality
- [ ] Create material creation/editing dialogs
- [ ] Implement data validation and error handling
- [ ] Add pagination for large datasets
- [ ] Create custom widgets (QRangeSlider, etc.)
- [ ] Implement styling and themes
- [ ] Add keyboard shortcuts and accessibility

### Phase 3: AI Integration (Week 5-6)
- [ ] Implement GroqClient with proper error handling
- [ ] Create MaterialChatbot widget
- [ ] Add AI-powered material creation
- [ ] Implement intelligent search suggestions
- [ ] Add natural language query processing
- [ ] Create AI prediction widget
- [ ] Implement response caching
- [ ] Add AI request rate limiting

### Phase 4: Advanced Features (Week 7-8)
- [ ] Add inventory analytics and charts
- [ ] Implement barcode scanning support
- [ ] Create supplier management
- [ ] Add inventory transaction logging
- [ ] Implement advanced reporting
- [ ] Add backup and restore functionality
- [ ] Create user preferences system
- [ ] Add multi-language support

### Phase 5: Testing & Optimization (Week 9-10)
- [ ] Complete unit test coverage
- [ ] Perform integration testing
- [ ] Conduct performance benchmarking
- [ ] Optimize database queries
- [ ] Implement memory management
- [ ] Add diagnostic tools
- [ ] Create user documentation
- [ ] Prepare deployment packages

### Phase 6: Deployment & Documentation (Week 11-12)
- [ ] Create installer packages
- [ ] Write deployment documentation
- [ ] Create user manual
- [ ] Set up error reporting system
- [ ] Implement automatic updates
- [ ] Create backup and migration tools
- [ ] Conduct user acceptance testing
- [ ] Finalize production deployment

---

## 📚 Additional Resources

### Development Guidelines
- Follow Qt coding conventions
- Use modern C++17 features where appropriate
- Implement RAII for resource management
- Use smart pointers for memory management
- Follow SOLID principles for class design
- Write comprehensive unit tests
- Document all public APIs
- Use consistent error handling patterns

### Performance Targets
- **Startup Time:** < 3 seconds
- **Search Response:** < 500ms for 10,000+ items
- **AI Response:** < 10 seconds (with 30s timeout)
- **Database Operations:** < 100ms for simple queries
- **Memory Usage:** < 500MB for normal operations
- **File Import:** Handle 100,000+ records efficiently

### Security Standards
- Store API keys securely using OS credential store
- Validate all user inputs before database operations
- Use prepared statements to prevent SQL injection
- Implement request rate limiting for AI API
- Log security events for audit purposes
- Encrypt sensitive data at rest
- Use HTTPS for all external communications

---

*This specification provides a comprehensive foundation for implementing a production-ready Materials Management System with AI integration. Regular updates and refinements should be made based on user feedback and changing requirements.*

**Last Updated:** June 4, 2025  
**Document Version:** 2.0  
**Next Review Date:** July 4, 2025
