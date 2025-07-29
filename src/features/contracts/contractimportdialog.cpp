#include "contractimportdialog.h"
#include "../../interfaces/icontractimporter.h"
#include "contractdatabasemanager.h"
#include "contract.h"
#include "utils/stylemanager.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTimer>
#include <QHeaderView>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QCloseEvent>

ContractImportDialog::ContractImportDialog(QWidget *parent)
    : BaseDialog(parent)
    , m_importer(nullptr)
    , m_dbManager(nullptr)
    , m_importInProgress(false)
    , m_importCancelled(false)
    , m_totalRecords(0)
    , m_processedRecords(0)
    , m_successfulImports(0)
    , m_failedImports(0)
    , m_maxPreviewRows(MAX_PREVIEW_ROWS)
{
    setWindowTitle("Import Contracts");
    setWindowIcon(QIcon(":/icons/import.png"));
    resize(800, 600);
    
    setupUi();
    setupConnections();
    applyArchiFlowStyling();
    
    // Initialize default settings
    m_importSettings["skipDuplicates"] = true;
    m_importSettings["validateData"] = true;
    m_importSettings["createBackup"] = true;
    m_importSettings["continueOnError"] = false;
    m_importSettings["batchSize"] = DEFAULT_BATCH_SIZE;
    m_importSettings["duplicateHandling"] = "Skip";
    
    updateButtonStates();
}

ContractImportDialog::~ContractImportDialog()
{
    if (m_importInProgress) {
        cancelImport();
    }
}

void ContractImportDialog::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // Create tab widget
    m_tabWidget = new QTabWidget;
    
    setupFileSelection();
    setupFieldMapping();
    setupDataPreview();
    setupImportSettings();
    setupProgressArea();
    
    // Add tabs
    m_tabWidget->addTab(m_fileTab, QIcon(":/icons/file.png"), "File Selection");
    m_tabWidget->addTab(m_mappingTab, QIcon(":/icons/mapping.png"), "Field Mapping");
    m_tabWidget->addTab(m_previewTab, QIcon(":/icons/preview.png"), "Data Preview");
    m_tabWidget->addTab(m_settingsTab, QIcon(":/icons/settings.png"), "Import Settings");
    
    m_mainLayout->addWidget(m_tabWidget);
    m_mainLayout->addWidget(m_progressWidget);
    
    // Bottom buttons
    m_buttonWidget = new QWidget;
    m_buttonLayout = new QHBoxLayout(m_buttonWidget);
    
    m_validateButton = new QPushButton("Validate");
    m_validateButton->setIcon(QIcon(":/icons/validate.png"));
    m_validateButton->setEnabled(false);
    
    m_importButton = new QPushButton("Import");
    m_importButton->setIcon(QIcon(":/icons/import.png"));
    m_importButton->setObjectName("primaryButton");
    m_importButton->setEnabled(false);
    
    m_closeButton = new QPushButton("Close");
    m_closeButton->setIcon(QIcon(":/icons/close.png"));
    
    m_buttonLayout->addWidget(m_validateButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_importButton);
    m_buttonLayout->addWidget(m_closeButton);
    
    m_mainLayout->addWidget(m_buttonWidget);
}

void ContractImportDialog::setupFileSelection()
{
    m_fileTab = new QWidget;
    m_fileLayout = new QVBoxLayout(m_fileTab);
    m_fileLayout->setSpacing(15);
    
    // File selection group
    m_fileSelectionGroup = new QGroupBox("Select Import File");
    QVBoxLayout *fileGroupLayout = new QVBoxLayout(m_fileSelectionGroup);
    
    // File path input
    QHBoxLayout *filePathLayout = new QHBoxLayout;
    QLabel *fileLabel = new QLabel("File Path:");
    m_filePathEdit = new QLineEdit;
    m_filePathEdit->setPlaceholderText("Click Browse to select a file...");
    m_filePathEdit->setReadOnly(true);
    
    m_browseButton = new QPushButton("Browse...");
    m_browseButton->setIcon(QIcon(":/icons/folder.png"));
    
    filePathLayout->addWidget(fileLabel);
    filePathLayout->addWidget(m_filePathEdit);
    filePathLayout->addWidget(m_browseButton);
    
    // File format selection
    QHBoxLayout *formatLayout = new QHBoxLayout;
    QLabel *formatLabel = new QLabel("File Format:");
    m_formatCombo = new QComboBox;
    m_formatCombo->addItems({"Auto-detect", "CSV", "Excel (XLSX)", "JSON", "XML"});
    
    m_analyzeButton = new QPushButton("Analyze File");
    m_analyzeButton->setIcon(QIcon(":/icons/analyze.png"));
    m_analyzeButton->setEnabled(false);
    
    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(m_formatCombo);
    formatLayout->addStretch();
    formatLayout->addWidget(m_analyzeButton);
    
    // File information
    m_fileInfoLabel = new QLabel("No file selected");
    m_fileInfoLabel->setObjectName("fileInfoLabel");
    m_fileInfoLabel->setWordWrap(true);
    
    fileGroupLayout->addLayout(filePathLayout);
    fileGroupLayout->addLayout(formatLayout);
    fileGroupLayout->addWidget(m_fileInfoLabel);
    
    // Supported formats info
    QGroupBox *formatInfoGroup = new QGroupBox("Supported Formats");
    QVBoxLayout *formatInfoLayout = new QVBoxLayout(formatInfoGroup);
    
    QLabel *formatInfoText = new QLabel(
        "• <b>CSV</b>: Comma-separated values with header row<br>"
        "• <b>Excel</b>: Microsoft Excel files (.xlsx)<br>"
        "• <b>JSON</b>: JavaScript Object Notation<br>"
        "• <b>XML</b>: Extensible Markup Language<br><br>"
        "<i>The system will automatically detect the format if 'Auto-detect' is selected.</i>"
    );
    formatInfoText->setWordWrap(true);
    formatInfoLayout->addWidget(formatInfoText);
    
    m_fileLayout->addWidget(m_fileSelectionGroup);
    m_fileLayout->addWidget(formatInfoGroup);
    m_fileLayout->addStretch();
}

void ContractImportDialog::setupFieldMapping()
{
    m_mappingTab = new QWidget;
    m_mappingLayout = new QVBoxLayout(m_mappingTab);
    m_mappingLayout->setSpacing(15);
    
    // Mapping group
    m_mappingGroup = new QGroupBox("Field Mapping Configuration");
    QVBoxLayout *mappingGroupLayout = new QVBoxLayout(m_mappingGroup);
    
    // Mapping controls
    QHBoxLayout *mappingControlsLayout = new QHBoxLayout;
    
    m_autoMapButton = new QPushButton("Auto Map");
    m_autoMapButton->setIcon(QIcon(":/icons/auto.png"));
    m_autoMapButton->setToolTip("Automatically map fields based on names");
    
    m_clearMappingButton = new QPushButton("Clear");
    m_clearMappingButton->setIcon(QIcon(":/icons/clear.png"));
    m_clearMappingButton->setToolTip("Clear all field mappings");
    
    m_saveMappingButton = new QPushButton("Save Mapping");
    m_saveMappingButton->setIcon(QIcon(":/icons/save.png"));
    m_saveMappingButton->setToolTip("Save current mapping for future use");
    
    m_loadMappingButton = new QPushButton("Load Mapping");
    m_loadMappingButton->setIcon(QIcon(":/icons/load.png"));
    m_loadMappingButton->setToolTip("Load a previously saved mapping");
    
    mappingControlsLayout->addWidget(m_autoMapButton);
    mappingControlsLayout->addWidget(m_clearMappingButton);
    mappingControlsLayout->addStretch();
    mappingControlsLayout->addWidget(m_saveMappingButton);
    mappingControlsLayout->addWidget(m_loadMappingButton);
    
    // Mapping table
    m_mappingTable = new QTableWidget;
    m_mappingTable->setColumnCount(MappingColumnCount);
    QStringList headers = {"Source Field", "Target Field", "Required", "Sample Data", "Status"};
    m_mappingTable->setHorizontalHeaderLabels(headers);
    m_mappingTable->horizontalHeader()->setStretchLastSection(true);
    m_mappingTable->setAlternatingRowColors(true);
    m_mappingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Mapping status
    m_mappingStatusLabel = new QLabel("Select a file to configure field mapping");
    m_mappingStatusLabel->setObjectName("mappingStatusLabel");
    
    mappingGroupLayout->addLayout(mappingControlsLayout);
    mappingGroupLayout->addWidget(m_mappingTable);
    mappingGroupLayout->addWidget(m_mappingStatusLabel);
    
    // Field requirements info
    QGroupBox *requirementsGroup = new QGroupBox("Required Fields");
    QVBoxLayout *requirementsLayout = new QVBoxLayout(requirementsGroup);
    
    QLabel *requiredFieldsText = new QLabel(
        "<b>Required Fields:</b><br>"
        "• Client Name<br>"
        "• Start Date<br>"
        "• End Date<br>"
        "• Contract Value<br>"
        "• Status<br><br>"
        "<b>Optional Fields:</b><br>"
        "• Description, Payment Terms, Notes, etc."
    );
    requiredFieldsText->setWordWrap(true);
    requirementsLayout->addWidget(requiredFieldsText);
    
    m_mappingLayout->addWidget(m_mappingGroup);
    m_mappingLayout->addWidget(requirementsGroup);
}

void ContractImportDialog::setupDataPreview()
{
    m_previewTab = new QWidget;
    m_previewLayout = new QVBoxLayout(m_previewTab);
    m_previewLayout->setSpacing(15);
    
    // Preview group
    m_previewGroup = new QGroupBox("Data Preview");
    QVBoxLayout *previewGroupLayout = new QVBoxLayout(m_previewGroup);
    
    // Preview controls
    QHBoxLayout *previewControlsLayout = new QHBoxLayout;
    
    m_previewInfoLabel = new QLabel("No data to preview");
    m_refreshPreviewButton = new QPushButton("Refresh Preview");
    m_refreshPreviewButton->setIcon(QIcon(":/icons/refresh.png"));
    m_refreshPreviewButton->setEnabled(false);
    
    previewControlsLayout->addWidget(m_previewInfoLabel);
    previewControlsLayout->addStretch();
    previewControlsLayout->addWidget(m_refreshPreviewButton);
    
    // Preview table
    m_previewTable = new QTableWidget;
    m_previewTable->setAlternatingRowColors(true);
    m_previewTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_previewTable->horizontalHeader()->setStretchLastSection(true);
    
    previewGroupLayout->addLayout(previewControlsLayout);
    previewGroupLayout->addWidget(m_previewTable);
    
    // Preview errors
    QGroupBox *errorsGroup = new QGroupBox("Preview Issues");
    QVBoxLayout *errorsLayout = new QVBoxLayout(errorsGroup);
    
    m_previewErrorsText = new QTextEdit;
    m_previewErrorsText->setMaximumHeight(100);
    m_previewErrorsText->setPlaceholderText("No issues detected");
    m_previewErrorsText->setReadOnly(true);
    
    errorsLayout->addWidget(m_previewErrorsText);
    
    m_previewLayout->addWidget(m_previewGroup);
    m_previewLayout->addWidget(errorsGroup);
}

void ContractImportDialog::setupImportSettings()
{
    m_settingsTab = new QWidget;
    m_settingsLayout = new QVBoxLayout(m_settingsTab);
    m_settingsLayout->setSpacing(15);
    
    // Import options group
    m_importOptionsGroup = new QGroupBox("Import Options");
    QGridLayout *optionsLayout = new QGridLayout(m_importOptionsGroup);
    
    m_skipDuplicatesCheck = new QCheckBox("Skip duplicate contracts");
    m_skipDuplicatesCheck->setChecked(true);
    m_skipDuplicatesCheck->setToolTip("Skip contracts that already exist in the database");
    
    m_validateDataCheck = new QCheckBox("Validate data before import");
    m_validateDataCheck->setChecked(true);
    m_validateDataCheck->setToolTip("Perform validation checks on the data");
    
    m_createBackupCheck = new QCheckBox("Create database backup");
    m_createBackupCheck->setChecked(true);
    m_createBackupCheck->setToolTip("Create a backup before importing");
    
    m_continueOnErrorCheck = new QCheckBox("Continue on errors");
    m_continueOnErrorCheck->setChecked(false);
    m_continueOnErrorCheck->setToolTip("Continue importing even if some records fail");
    
    // Duplicate handling
    QLabel *duplicateLabel = new QLabel("Duplicate Handling:");
    m_duplicateHandlingCombo = new QComboBox;
    m_duplicateHandlingCombo->addItems({"Skip", "Update", "Create New"});
    m_duplicateHandlingCombo->setToolTip("How to handle duplicate contracts");
    
    // Batch size
    QLabel *batchLabel = new QLabel("Batch Size:");
    m_batchSizeEdit = new QLineEdit(QString::number(DEFAULT_BATCH_SIZE));
    m_batchSizeEdit->setValidator(new QIntValidator(1, 10000, this));
    m_batchSizeEdit->setToolTip("Number of records to process at once");
    
    optionsLayout->addWidget(m_skipDuplicatesCheck, 0, 0, 1, 2);
    optionsLayout->addWidget(m_validateDataCheck, 1, 0, 1, 2);
    optionsLayout->addWidget(m_createBackupCheck, 2, 0, 1, 2);
    optionsLayout->addWidget(m_continueOnErrorCheck, 3, 0, 1, 2);
    optionsLayout->addWidget(duplicateLabel, 4, 0);
    optionsLayout->addWidget(m_duplicateHandlingCombo, 4, 1);
    optionsLayout->addWidget(batchLabel, 5, 0);
    optionsLayout->addWidget(m_batchSizeEdit, 5, 1);
    
    // Performance settings group
    QGroupBox *performanceGroup = new QGroupBox("Performance Settings");
    QVBoxLayout *performanceLayout = new QVBoxLayout(performanceGroup);
    
    QLabel *performanceText = new QLabel(
        "• <b>Batch Size:</b> Larger batches are faster but use more memory<br>"
        "• <b>Validation:</b> Disabling validation improves speed but may import invalid data<br>"
        "• <b>Continue on Error:</b> Allows partial imports when some records are invalid"
    );
    performanceText->setWordWrap(true);
    performanceLayout->addWidget(performanceText);
    
    m_settingsLayout->addWidget(m_importOptionsGroup);
    m_settingsLayout->addWidget(performanceGroup);
    m_settingsLayout->addStretch();
}

void ContractImportDialog::setupProgressArea()
{
    m_progressWidget = new QWidget;
    m_progressLayout = new QVBoxLayout(m_progressWidget);
    m_progressLayout->setSpacing(10);
    
    // Progress group
    m_progressGroup = new QGroupBox("Import Progress");
    QVBoxLayout *progressGroupLayout = new QVBoxLayout(m_progressGroup);
    
    // Progress controls
    QHBoxLayout *progressControlsLayout = new QHBoxLayout;
    
    m_startImportButton = new QPushButton("Start Import");
    m_startImportButton->setIcon(QIcon(":/icons/play.png"));
    m_startImportButton->setObjectName("successButton");
    m_startImportButton->setEnabled(false);
    
    m_cancelImportButton = new QPushButton("Cancel");
    m_cancelImportButton->setIcon(QIcon(":/icons/stop.png"));
    m_cancelImportButton->setObjectName("dangerButton");
    m_cancelImportButton->setEnabled(false);
    
    m_resetImportButton = new QPushButton("Reset");
    m_resetImportButton->setIcon(QIcon(":/icons/reset.png"));
    
    progressControlsLayout->addWidget(m_startImportButton);
    progressControlsLayout->addWidget(m_cancelImportButton);
    progressControlsLayout->addWidget(m_resetImportButton);
    progressControlsLayout->addStretch();
    
    // Progress indicators
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);
    
    m_progressLabel = new QLabel("Ready to import");
    m_statusLabel = new QLabel("");
    
    progressGroupLayout->addLayout(progressControlsLayout);
    progressGroupLayout->addWidget(m_progressBar);
    progressGroupLayout->addWidget(m_progressLabel);
    progressGroupLayout->addWidget(m_statusLabel);
    
    // Results group
    m_resultsGroup = new QGroupBox("Import Results");
    QGridLayout *resultsLayout = new QGridLayout(m_resultsGroup);
    
    m_successCountLabel = new QLabel("Successful: 0");
    m_successCountLabel->setObjectName("successLabel");
    
    m_errorCountLabel = new QLabel("Errors: 0");
    m_errorCountLabel->setObjectName("errorLabel");
    
    m_warningCountLabel = new QLabel("Warnings: 0");
    m_warningCountLabel->setObjectName("warningLabel");
    
    m_exportReportButton = new QPushButton("Export Report");
    m_exportReportButton->setIcon(QIcon(":/icons/export.png"));
    m_exportReportButton->setEnabled(false);
    
    m_viewErrorsButton = new QPushButton("View Errors");
    m_viewErrorsButton->setIcon(QIcon(":/icons/error.png"));
    m_viewErrorsButton->setEnabled(false);
    
    resultsLayout->addWidget(m_successCountLabel, 0, 0);
    resultsLayout->addWidget(m_errorCountLabel, 0, 1);
    resultsLayout->addWidget(m_warningCountLabel, 0, 2);
    resultsLayout->addWidget(m_exportReportButton, 1, 0);
    resultsLayout->addWidget(m_viewErrorsButton, 1, 1);
    
    // Result summary
    m_resultSummaryText = new QTextEdit;
    m_resultSummaryText->setMaximumHeight(80);
    m_resultSummaryText->setPlaceholderText("Import summary will appear here...");
    m_resultSummaryText->setReadOnly(true);
    resultsLayout->addWidget(m_resultSummaryText, 2, 0, 1, 3);
    
    m_progressLayout->addWidget(m_progressGroup);
    m_progressLayout->addWidget(m_resultsGroup);
    
    // Initially hide results
    m_resultsGroup->setVisible(false);
}

void ContractImportDialog::setupConnections()
{
    // File selection connections
    connect(m_browseButton, &QPushButton::clicked, this, &ContractImportDialog::selectFile);
    connect(m_filePathEdit, &QLineEdit::textChanged, this, &ContractImportDialog::onFileChanged);
    connect(m_formatCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &ContractImportDialog::onFormatChanged);
    connect(m_analyzeButton, &QPushButton::clicked, this, &ContractImportDialog::previewFile);
    
    // Mapping connections
    connect(m_autoMapButton, &QPushButton::clicked, this, &ContractImportDialog::autoMapFields);
    connect(m_clearMappingButton, &QPushButton::clicked, this, &ContractImportDialog::clearMapping);
    connect(m_saveMappingButton, &QPushButton::clicked, this, &ContractImportDialog::saveMapping);
    connect(m_loadMappingButton, &QPushButton::clicked, this, &ContractImportDialog::loadMapping);
    connect(m_mappingTable, &QTableWidget::cellChanged, this, &ContractImportDialog::onFieldMappingChanged);
    
    // Preview connections
    connect(m_refreshPreviewButton, &QPushButton::clicked, this, &ContractImportDialog::onPreviewRequested);
    
    // Settings connections
    connect(m_skipDuplicatesCheck, &QCheckBox::toggled, this, &ContractImportDialog::onImportSettingsChanged);
    connect(m_validateDataCheck, &QCheckBox::toggled, this, &ContractImportDialog::onImportSettingsChanged);
    connect(m_createBackupCheck, &QCheckBox::toggled, this, &ContractImportDialog::onImportSettingsChanged);
    connect(m_continueOnErrorCheck, &QCheckBox::toggled, this, &ContractImportDialog::onImportSettingsChanged);
    connect(m_duplicateHandlingCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &ContractImportDialog::onImportSettingsChanged);
    connect(m_batchSizeEdit, &QLineEdit::textChanged, this, &ContractImportDialog::onImportSettingsChanged);
    
    // Progress connections
    connect(m_startImportButton, &QPushButton::clicked, this, &ContractImportDialog::onImportRequested);
    connect(m_cancelImportButton, &QPushButton::clicked, this, &ContractImportDialog::onCancelRequested);
    connect(m_resetImportButton, &QPushButton::clicked, this, &ContractImportDialog::onResetRequested);
    
    // Button connections
    connect(m_validateButton, &QPushButton::clicked, this, &ContractImportDialog::onValidateRequested);
    connect(m_importButton, &QPushButton::clicked, this, &ContractImportDialog::executeImport);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
    
    // Results connections
    connect(m_exportReportButton, &QPushButton::clicked, this, &ContractImportDialog::generateImportReport);
    connect(m_viewErrorsButton, &QPushButton::clicked, this, &ContractImportDialog::showPreviewErrors);
}

void ContractImportDialog::setImporter(IContractImporter *importer)
{
    m_importer = importer;
    updateButtonStates();
}

void ContractImportDialog::setDatabaseManager(ContractDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
}

void ContractImportDialog::selectFile()
{
    QString filter = "All Supported Files (*.csv *.xlsx *.json *.xml);;";
    filter += "CSV Files (*.csv);;";
    filter += "Excel Files (*.xlsx);;";
    filter += "JSON Files (*.json);;";
    filter += "XML Files (*.xml);;";
    filter += "All Files (*)";
    
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString filePath = QFileDialog::getOpenFileName(this, "Select Import File", defaultPath, filter);
    
    if (!filePath.isEmpty()) {
        m_filePathEdit->setText(filePath);
        m_selectedFilePath = filePath;
        emit fileSelected(filePath);
        
        analyzeFile();
    }
}

void ContractImportDialog::analyzeFile()
{
    if (m_selectedFilePath.isEmpty() || !m_importer) {
        return;
    }
    
    // Detect file format
    detectFileFormat();
    
    // Get file information
    QFileInfo fileInfo(m_selectedFilePath);
    QString info = QString("File: %1\nSize: %2\nModified: %3\nFormat: %4")
        .arg(fileInfo.fileName())
        .arg(formatFileSize(fileInfo.size()))
        .arg(fileInfo.lastModified().toString())
        .arg(m_detectedFormat);
    
    m_fileInfoLabel->setText(info);
    
    // Load field options
    loadFieldOptions();
    
    // Update button states
    updateButtonStates();
    
    // Switch to mapping tab if file is valid
    if (validateSelectedFile()) {
        m_tabWidget->setCurrentIndex(1); // Mapping tab
    }
}

void ContractImportDialog::detectFileFormat()
{
    if (m_formatCombo->currentText() != "Auto-detect") {
        m_detectedFormat = m_formatCombo->currentText();
        return;
    }
    
    QFileInfo fileInfo(m_selectedFilePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "csv") {
        m_detectedFormat = "CSV";
    } else if (extension == "xlsx" || extension == "xls") {
        m_detectedFormat = "Excel";
    } else if (extension == "json") {
        m_detectedFormat = "JSON";
    } else if (extension == "xml") {
        m_detectedFormat = "XML";
    } else {
        m_detectedFormat = "Unknown";
    }
}

void ContractImportDialog::loadFieldOptions()
{
    if (!m_importer || m_selectedFilePath.isEmpty()) {
        return;
    }
    
    try {
        // Get source fields from file
        m_sourceFields = m_importer->getDetectedFields(m_selectedFilePath);
        
        // Get target fields (contract fields)
        m_targetFields = m_importer->getRequiredFields();
        m_targetFields.append(m_importer->getOptionalFields());
        
        // Update mapping table
        updateMappingTable();
        
        // Try auto-mapping
        autoMapFields();
        
    } catch (const std::exception &e) {
        showMessage(QString("Error analyzing file: %1").arg(e.what()), true);
    }
}

void ContractImportDialog::updateMappingTable()
{
    m_mappingTable->setRowCount(m_sourceFields.size());
    
    for (int i = 0; i < m_sourceFields.size(); ++i) {
        // Source field (read-only)
        QTableWidgetItem *sourceItem = new QTableWidgetItem(m_sourceFields[i]);
        sourceItem->setFlags(sourceItem->flags() & ~Qt::ItemIsEditable);
        m_mappingTable->setItem(i, SourceFieldColumn, sourceItem);
        
        // Target field (combo box)
        QComboBox *targetCombo = new QComboBox;
        targetCombo->addItem(""); // No mapping
        targetCombo->addItems(m_targetFields);
        m_mappingTable->setCellWidget(i, TargetFieldColumn, targetCombo);
        
        // Required indicator
        QTableWidgetItem *requiredItem = new QTableWidgetItem("No");
        requiredItem->setFlags(requiredItem->flags() & ~Qt::ItemIsEditable);
        m_mappingTable->setItem(i, RequiredColumn, requiredItem);
        
        // Sample data (placeholder)
        QTableWidgetItem *sampleItem = new QTableWidgetItem("...");
        sampleItem->setFlags(sampleItem->flags() & ~Qt::ItemIsEditable);
        m_mappingTable->setItem(i, SampleDataColumn, sampleItem);
        
        // Status
        QTableWidgetItem *statusItem = new QTableWidgetItem("Not Mapped");
        statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
        m_mappingTable->setItem(i, StatusColumn, statusItem);
    }
    
    m_mappingTable->resizeColumnsToContents();
}

void ContractImportDialog::autoMapFields()
{
    if (!m_importer || m_sourceFields.isEmpty()) {
        return;
    }
    
    try {
        QStringList suggestions = m_importer->getSuggestedMapping(m_sourceFields);
        
        for (int i = 0; i < m_sourceFields.size() && i < suggestions.size(); ++i) {
            QString suggestion = suggestions[i];
            if (!suggestion.isEmpty()) {
                QComboBox *targetCombo = qobject_cast<QComboBox*>(m_mappingTable->cellWidget(i, TargetFieldColumn));
                if (targetCombo) {
                    int index = targetCombo->findText(suggestion);
                    if (index >= 0) {
                        targetCombo->setCurrentIndex(index);
                        
                        // Update status
                        QTableWidgetItem *statusItem = m_mappingTable->item(i, StatusColumn);
                        if (statusItem) {
                            statusItem->setText("Mapped");
                            statusItem->setBackground(QColor("#d4edda"));
                        }
                    }
                }
            }
        }
        
        m_mappingStatusLabel->setText("Auto-mapping completed");
        
    } catch (const std::exception &e) {
        showMessage(QString("Error in auto-mapping: %1").arg(e.what()), true);
    }
}

void ContractImportDialog::clearMapping()
{
    for (int i = 0; i < m_mappingTable->rowCount(); ++i) {
        QComboBox *targetCombo = qobject_cast<QComboBox*>(m_mappingTable->cellWidget(i, TargetFieldColumn));
        if (targetCombo) {
            targetCombo->setCurrentIndex(0); // Empty selection
        }
        
        QTableWidgetItem *statusItem = m_mappingTable->item(i, StatusColumn);
        if (statusItem) {
            statusItem->setText("Not Mapped");
            statusItem->setBackground(QColor());
        }
    }
    
    m_mappingStatusLabel->setText("All mappings cleared");
}

void ContractImportDialog::onImportRequested()
{
    executeImport();
}

void ContractImportDialog::executeImport()
{
    if (!validateSelectedFile() || !validateFieldMapping()) {
        return;
    }
    
    // Confirm import
    int result = QMessageBox::question(this, "Confirm Import",
        QString("Are you sure you want to import contracts from:\n%1\n\nThis action cannot be undone.")
        .arg(m_selectedFilePath),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (result != QMessageBox::Yes) {
        return;
    }
    
    startImport();
}

void ContractImportDialog::startImport()
{
    if (!m_importer || !m_dbManager) {
        showMessage("Import service not available", true);
        return;
    }
    
    m_importInProgress = true;
    m_importCancelled = false;
    
    // Configure importer
    m_importer->setFieldMapping(m_fieldMapping);
    m_importer->setImportSettings(m_importSettings);
    
    // Setup progress tracking
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_progressLabel->setText("Starting import...");
    m_statusLabel->setText("Preparing data...");
    
    // Enable/disable controls
    m_startImportButton->setEnabled(false);
    m_cancelImportButton->setEnabled(true);
    m_tabWidget->setEnabled(false);
    m_importButton->setEnabled(false);
    
    // Start import process
    processImport();
}

void ContractImportDialog::processImport()
{
    if (!m_importer) {
        return;
    }
    
    try {
        // Import from file
        bool success = m_importer->importFromFile(m_selectedFilePath);
        
        if (success) {
            // Get results
            m_totalRecords = m_importer->getTotalRecords();
            m_successfulImports = m_importer->getSuccessfulImports();
            m_failedImports = m_importer->getFailedImports();
            m_importErrors = m_importer->getImportErrors();
            m_importWarnings = m_importer->getImportWarnings();
            
            // Update progress
            m_progressBar->setValue(100);
            m_progressLabel->setText("Import completed");
            m_statusLabel->setText(QString("Processed %1 records").arg(m_totalRecords));
              // Show results
            displayImportResults(m_successfulImports, m_failedImports, m_importErrors);
            
            emit importCompleted(m_successfulImports, m_failedImports);
            
        } else {
            QString error = m_importer->getLastError();
            showMessage(QString("Import failed: %1").arg(error), true);
        }
        
    } catch (const std::exception &e) {
        showMessage(QString("Import error: %1").arg(e.what()), true);
    }
    
    m_importInProgress = false;
    
    // Re-enable controls
    m_startImportButton->setEnabled(true);
    m_cancelImportButton->setEnabled(false);
    m_tabWidget->setEnabled(true);
    updateButtonStates();
}

void ContractImportDialog::displayImportResults(int successful, int failed, const QStringList &errors)
{
    // Update result labels
    m_successCountLabel->setText(QString("Successful: %1").arg(successful));
    m_errorCountLabel->setText(QString("Errors: %1").arg(failed));
    m_warningCountLabel->setText(QString("Warnings: %1").arg(m_importWarnings.size()));
    
    // Generate summary
    int totalRecords = successful + failed;
    QString summary = QString("Import completed successfully!\n"
                             "Total records: %1\n"
                             "Imported: %2\n"
                             "Failed: %3\n"
                             "Success rate: %4%")
        .arg(totalRecords)
        .arg(successful)
        .arg(failed)
        .arg(totalRecords > 0 ? (successful * 100 / totalRecords) : 0);
    
    m_resultSummaryText->setText(summary);
    
    // Show results group
    m_resultsGroup->setVisible(true);
    
    // Enable result buttons
    m_exportReportButton->setEnabled(true);
    m_viewErrorsButton->setEnabled(failed > 0 || !errors.isEmpty());
}

bool ContractImportDialog::validateSelectedFile()
{
    if (m_selectedFilePath.isEmpty()) {
        showMessage("Please select a file to import", true);
        return false;
    }
    
    if (!m_importer) {
        showMessage("Import service not available", true);
        return false;
    }
    
    if (!m_importer->validateFile(m_selectedFilePath)) {
        QString error = m_importer->getLastError();
        showMessage(QString("File validation failed: %1").arg(error), true);
        return false;
    }
    
    return true;
}

bool ContractImportDialog::validateFieldMapping()
{
    if (!m_importer) {
        return false;
    }
    
    // Check if required fields are mapped
    QStringList requiredFields = m_importer->getRequiredFields();
    QStringList mappedTargets;
    
    // Build field mapping
    m_fieldMapping.clear();
    for (int i = 0; i < m_mappingTable->rowCount(); ++i) {
        QComboBox *targetCombo = qobject_cast<QComboBox*>(m_mappingTable->cellWidget(i, TargetFieldColumn));
        if (targetCombo && !targetCombo->currentText().isEmpty()) {
            QString sourceField = m_mappingTable->item(i, SourceFieldColumn)->text();
            QString targetField = targetCombo->currentText();
            m_fieldMapping[sourceField] = targetField;
            mappedTargets.append(targetField);
        }
    }
    
    // Check for missing required fields
    QStringList missingFields;
    for (const QString &required : requiredFields) {
        if (!mappedTargets.contains(required)) {
            missingFields.append(required);
        }
    }
    
    if (!missingFields.isEmpty()) {
        QString message = QString("Required fields not mapped: %1").arg(missingFields.join(", "));
        showMessage(message, true);
        m_tabWidget->setCurrentIndex(1); // Switch to mapping tab
        return false;
    }
    
    return true;
}

void ContractImportDialog::updateButtonStates()
{
    bool hasFile = !m_selectedFilePath.isEmpty();
    bool hasImporter = (m_importer != nullptr);
    bool isReady = hasFile && hasImporter && !m_importInProgress;
    
    m_analyzeButton->setEnabled(hasFile);
    m_validateButton->setEnabled(isReady);
    m_importButton->setEnabled(isReady);
    m_startImportButton->setEnabled(isReady);
    
    // Mapping buttons
    m_autoMapButton->setEnabled(hasFile && hasImporter);
    m_clearMappingButton->setEnabled(hasFile);
    m_refreshPreviewButton->setEnabled(hasFile && hasImporter);
}

void ContractImportDialog::showMessage(const QString &message, bool isError)
{
    if (isError) {
        m_statusLabel->setText(QString("<span style='color: red;'>%1</span>").arg(message));
        QMessageBox::warning(this, "Import Error", message);
    } else {
        m_statusLabel->setText(message);
    }
}

QString ContractImportDialog::formatFileSize(qint64 size)
{
    const QStringList units = {"B", "KB", "MB", "GB"};
    double fileSize = size;
    int unitIndex = 0;
    
    while (fileSize >= 1024 && unitIndex < units.size() - 1) {
        fileSize /= 1024;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(QString::number(fileSize, 'f', 1)).arg(units[unitIndex]);
}

// Stub implementations for remaining slots
void ContractImportDialog::onFileChanged() { updateButtonStates(); }
void ContractImportDialog::onFormatChanged() { if (!m_selectedFilePath.isEmpty()) detectFileFormat(); }
void ContractImportDialog::onFieldMappingChanged() { validateMapping(); }
void ContractImportDialog::onImportSettingsChanged() { 
    m_importSettings["skipDuplicates"] = m_skipDuplicatesCheck->isChecked();
    m_importSettings["validateData"] = m_validateDataCheck->isChecked();
    m_importSettings["createBackup"] = m_createBackupCheck->isChecked();
    m_importSettings["continueOnError"] = m_continueOnErrorCheck->isChecked();
    m_importSettings["duplicateHandling"] = m_duplicateHandlingCombo->currentText();
    m_importSettings["batchSize"] = m_batchSizeEdit->text().toInt();
}
void ContractImportDialog::onPreviewRequested() { loadPreviewData(); }
void ContractImportDialog::onValidateRequested() { validateData(); }
void ContractImportDialog::onCancelRequested() { cancelImport(); }
void ContractImportDialog::onResetRequested() { resetImport(); }

void ContractImportDialog::previewFile() { onPreviewRequested(); }
void ContractImportDialog::configureMapping() { m_tabWidget->setCurrentIndex(1); }
void ContractImportDialog::validateData() { 
    if (validateSelectedFile() && validateFieldMapping()) {
        showMessage("Validation passed successfully", false);
    }
}
void ContractImportDialog::cancelImport() { 
    m_importCancelled = true;
    m_importInProgress = false;
    emit importCancelled();
}
void ContractImportDialog::resetImport() { 
    m_resultsGroup->setVisible(false);
    m_progressBar->setVisible(false);
    updateButtonStates();
}

void ContractImportDialog::loadPreviewData() { /* TODO: Implement */ }
void ContractImportDialog::saveMapping() { /* TODO: Implement mapping save */ }
void ContractImportDialog::loadMapping() { /* TODO: Implement mapping load */ }
void ContractImportDialog::validateMapping() { /* TODO: Implement mapping validation */ }
void ContractImportDialog::generateImportReport() { /* TODO: Implement report generation */ }
void ContractImportDialog::showPreviewErrors() { /* TODO: Implement error viewer */ }

void ContractImportDialog::applyArchiFlowStyling()
{
    QString styles = R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QLabel#fileInfoLabel {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 3px;
            padding: 10px;
            font-family: monospace;
        }
        
        QLabel#mappingStatusLabel {
            color: #6c757d;
            font-style: italic;
        }
        
        QLabel#successLabel { color: #28a745; font-weight: bold; }
        QLabel#errorLabel { color: #dc3545; font-weight: bold; }
        QLabel#warningLabel { color: #ffc107; font-weight: bold; }
    )";
    
    setStyleSheet(styles);
}

void ContractImportDialog::showEvent(QShowEvent *event)
{
    BaseDialog::showEvent(event);
    updateButtonStates();
}

void ContractImportDialog::closeEvent(QCloseEvent *event)
{
    if (m_importInProgress) {
        int result = QMessageBox::question(this, "Import in Progress",
            "An import is currently in progress. Do you want to cancel it and close?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
        if (result == QMessageBox::Yes) {
            cancelImport();
        } else {
            event->ignore();
            return;
        }
    }
    
    BaseDialog::closeEvent(event);
}

void ContractImportDialog::updateProgress()
{
    if (!m_importer) {
        return;
    }
      int current = m_importer->getProcessedRecords();
    int total = m_importer->getTotalRecords();
    
    if (total > 0) {
        int percentage = (current * 100) / total;
        m_progressBar->setValue(percentage);
        m_progressLabel->setText(QString("Processing: %1 of %2 (%3%)")
                                .arg(current)
                                .arg(total)
                                .arg(percentage));
    } else {
        m_progressBar->setValue(0);
        m_progressLabel->setText("Preparing import...");
    }
}
