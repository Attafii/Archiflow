#ifndef CONTRACTIMPORTDIALOG_H
#define CONTRACTIMPORTDIALOG_H

#include "ui/basedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QGroupBox>
#include <QFileDialog>
#include <QTabWidget>

class Contract;
class IContractImporter;
class ContractDatabaseManager;

/**
 * @brief Dialog for importing contracts from various file formats
 * 
 * This dialog provides a comprehensive interface for:
 * - File selection and format detection
 * - Field mapping configuration
 * - Data preview and validation
 * - Import progress monitoring
 * - Error handling and reporting
 */
class ContractImportDialog : public BaseDialog
{
    Q_OBJECT

public:
    explicit ContractImportDialog(QWidget *parent = nullptr);
    ~ContractImportDialog();

    // Configuration
    void setImporter(IContractImporter *importer);
    void setDatabaseManager(ContractDatabaseManager *dbManager);
    
    // Import operations
    void startImport();
    void cancelImport();
    void resetImport();

public slots:
    void selectFile();
    void previewFile();
    void configureMapping();
    void validateData();
    void executeImport();

signals:
    void importStarted();
    void importProgress(int current, int total);
    void importCompleted(int successful, int failed);
    void importCancelled();
    void fileSelected(const QString &filePath);
    void mappingConfigured();
    void dataValidated(bool isValid);
    void contractsImported(int count, int total, const QString &summary);
    void errorOccurred(const QString &error);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFileChanged();
    void onFormatChanged();
    void onFieldMappingChanged();
    void onImportSettingsChanged();
    void onPreviewRequested();
    void onValidateRequested();
    void onImportRequested();
    void onCancelRequested();
    void onResetRequested();
    void updateProgress();

private:
    void setupUi();
    void setupFileSelection();
    void setupFieldMapping();
    void setupDataPreview();
    void setupImportSettings();
    void setupProgressArea();
    void setupConnections();
    void applyArchiFlowStyling();

    // File operations
    void detectFileFormat();
    void loadFieldOptions();
    void analyzeFile();
    void validateFileFormat();

    // Preview operations
    void loadPreviewData();
    void updatePreviewTable();
    void highlightMappingIssues();
    void showPreviewErrors();

    // Mapping operations
    void autoMapFields();
    void clearMapping();
    void saveMapping();
    void loadMapping();
    void validateMapping();

    // Import operations
    void prepareImport();
    void processImport();
    void handleImportErrors();
    void generateImportReport();

    // UI helpers
    void updateButtonStates();
    void showMessage(const QString &message, bool isError = false);
    void enableControls(bool enabled);
    void resetProgress();
    void updateMappingTable();
    void updateSettingsPanel();    // Validation helpers
    bool validateSelectedFile();
    bool validateFieldMapping();
    bool validateImportSettings();
    QStringList getValidationErrors();
    
    // Utility helpers
    QString formatFileSize(qint64 size);
    void displayImportResults(int successful, int failed, const QStringList &errors);

    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // File Selection Tab
    QWidget *m_fileTab;
    QVBoxLayout *m_fileLayout;
    QGroupBox *m_fileSelectionGroup;
    QLineEdit *m_filePathEdit;
    QPushButton *m_browseButton;
    QLabel *m_fileInfoLabel;
    QComboBox *m_formatCombo;
    QPushButton *m_analyzeButton;
    
    // Field Mapping Tab
    QWidget *m_mappingTab;
    QVBoxLayout *m_mappingLayout;
    QGroupBox *m_mappingGroup;
    QTableWidget *m_mappingTable;
    QPushButton *m_autoMapButton;
    QPushButton *m_clearMappingButton;
    QPushButton *m_saveMappingButton;
    QPushButton *m_loadMappingButton;
    QLabel *m_mappingStatusLabel;
    
    // Preview Tab
    QWidget *m_previewTab;
    QVBoxLayout *m_previewLayout;
    QGroupBox *m_previewGroup;
    QTableWidget *m_previewTable;
    QLabel *m_previewInfoLabel;
    QPushButton *m_refreshPreviewButton;
    QTextEdit *m_previewErrorsText;
    
    // Settings Tab
    QWidget *m_settingsTab;
    QVBoxLayout *m_settingsLayout;
    QGroupBox *m_importOptionsGroup;
    QCheckBox *m_skipDuplicatesCheck;
    QCheckBox *m_validateDataCheck;
    QCheckBox *m_createBackupCheck;
    QCheckBox *m_continueOnErrorCheck;
    QComboBox *m_duplicateHandlingCombo;
    QLineEdit *m_batchSizeEdit;
    
    // Progress and Results
    QWidget *m_progressWidget;
    QVBoxLayout *m_progressLayout;
    QGroupBox *m_progressGroup;
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    QLabel *m_statusLabel;
    QPushButton *m_startImportButton;
    QPushButton *m_cancelImportButton;
    QPushButton *m_resetImportButton;
    
    // Results area
    QGroupBox *m_resultsGroup;
    QLabel *m_successCountLabel;
    QLabel *m_errorCountLabel;
    QLabel *m_warningCountLabel;
    QTextEdit *m_resultSummaryText;
    QPushButton *m_exportReportButton;
    QPushButton *m_viewErrorsButton;
    
    // Bottom buttons
    QWidget *m_buttonWidget;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_validateButton;
    QPushButton *m_importButton;
    QPushButton *m_closeButton;
    
    // Data and state
    IContractImporter *m_importer;
    ContractDatabaseManager *m_dbManager;
    QString m_selectedFilePath;
    QString m_detectedFormat;
    QStringList m_sourceFields;
    QStringList m_targetFields;
    QVariantMap m_fieldMapping;
    QVariantMap m_importSettings;
    
    // Import state
    bool m_importInProgress;
    bool m_importCancelled;
    int m_totalRecords;
    int m_processedRecords;
    int m_successfulImports;
    int m_failedImports;
    QStringList m_importErrors;
    QStringList m_importWarnings;
    
    // Preview data
    QList<QVariantMap> m_previewData;
    int m_maxPreviewRows;
    
    // Constants
    static constexpr int DEFAULT_BATCH_SIZE = 100;
    static constexpr int MAX_PREVIEW_ROWS = 50;
    static constexpr int PROGRESS_UPDATE_INTERVAL = 100; // ms
    
    // Field mapping table columns
    enum MappingColumn {
        SourceFieldColumn = 0,
        TargetFieldColumn,
        RequiredColumn,
        SampleDataColumn,
        StatusColumn,
        MappingColumnCount
    };
};

#endif // CONTRACTIMPORTDIALOG_H
