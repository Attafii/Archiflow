#include "csvimportdialog.h"
#include <QApplication>
#include <QTextStream>
#include <QDateTime>
#include <QDate>
#include <QDebug>
#include <QThread>
#include <QDir>

// Styling constants matching the main application
const QString CsvImportDialog::DIALOG_STYLE = R"(
    QDialog {
        background-color: #3D485A;
        color: #E3C6B0;
        font-family: 'Poppins', 'Segoe UI', sans-serif;
        font-size: 14px;
    }
)";

const QString CsvImportDialog::BUTTON_STYLE = R"(
    QPushButton {
        background-color: #E3C6B0;
        color: #3D485A;
        border: 2px solid #C4A491;
        border-radius: 8px;
        padding: 10px 20px;
        font-weight: bold;
        font-size: 14px;
        min-width: 120px;
    }
    QPushButton:hover {
        background-color: #C4A491;
        border-color: #A68B73;
    }
    QPushButton:pressed {
        background-color: #A68B73;
    }
    QPushButton:disabled {
        background-color: #5A6B7D;
        color: #8B9AAD;
        border-color: #5A6B7D;
    }
)";

const QString CsvImportDialog::INPUT_STYLE = R"(
    QLineEdit {
        background-color: #4A5568;
        color: #E3C6B0;
        border: 2px solid #5A6B7D;
        border-radius: 6px;
        padding: 8px 12px;
        font-size: 14px;
        selection-background-color: #C4A491;
    }
    QLineEdit:focus {
        border-color: #E3C6B0;
        background-color: #525F6F;
    }
)";

const QString CsvImportDialog::INSTRUCTIONS_STYLE = R"(
    QTextEdit {
        background-color: #4A5568;
        color: #E3C6B0;
        border: 2px solid #5A6B7D;
        border-radius: 6px;
        padding: 12px;
        font-size: 13px;
        font-family: 'Consolas', 'Courier New', monospace;
    }
)";

CsvImportDialog::CsvImportDialog(DatabaseManager* dbManager, QWidget *parent)
    : QDialog(parent)
    , m_dbManager(dbManager)
{
    setupUI();
    setWindowTitle("Import Contracts from CSV");
    setWindowIcon(QIcon(":/icons/import.png"));
    resize(600, 500);
    
    // Center the dialog
    if (parent) {
        move(parent->geometry().center() - rect().center());
    }
}

CsvImportDialog::~CsvImportDialog()
{
}

void CsvImportDialog::setupUI()
{
    setStyleSheet(DIALOG_STYLE);
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Title
    m_titleLabel = new QLabel("📊 Import Contracts from CSV File");
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #E3C6B0; margin-bottom: 10px;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);
    
    // Instructions
    m_instructionsText = new QTextEdit();
    m_instructionsText->setStyleSheet(INSTRUCTIONS_STYLE);
    m_instructionsText->setMaximumHeight(200);
    m_instructionsText->setReadOnly(true);
    m_instructionsText->setHtml(R"(
<b>📋 Required CSV Structure:</b><br><br>
The CSV file must contain the following columns (in this exact order):<br><br>
<b>client_name,start_date,end_date,value,status,description</b><br><br>
<b>📝 Example:</b><br>
ABC Corporation,2025-01-15,2026-01-15,50000.00,Active,Software development contract<br>
XYZ Ltd,2025-02-01,2025-12-31,25000.00,Draft,Consulting services agreement<br>
Tech Solutions Inc,2025-03-01,2027-03-01,75000.00,Active,Long-term support contract<br><br>
<b>📌 Notes:</b><br>
• Date format: YYYY-MM-DD<br>
• Value: Numeric (decimals allowed)<br>
• Status: Active, Draft, Completed, Expired<br>
• Description: Any text (optional)<br>
• Contract IDs will be auto-generated
    )");
    m_mainLayout->addWidget(m_instructionsText);
    
    // File selection
    QLabel* fileLabel = new QLabel("📁 Select CSV File:");
    fileLabel->setStyleSheet("font-weight: bold; color: #E3C6B0;");
    m_mainLayout->addWidget(fileLabel);
    
    m_fileLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit();
    m_filePathEdit->setStyleSheet(INPUT_STYLE);
    m_filePathEdit->setPlaceholderText("Choose a CSV file to import...");
    m_filePathEdit->setReadOnly(true);
    
    m_browseButton = new QPushButton("Browse...");
    m_browseButton->setStyleSheet(BUTTON_STYLE);
    
    m_fileLayout->addWidget(m_filePathEdit, 1);
    m_fileLayout->addWidget(m_browseButton);
    m_mainLayout->addLayout(m_fileLayout);
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid #5A6B7D;
            border-radius: 6px;
            background-color: #4A5568;
            text-align: center;
            color: #E3C6B0;
        }
        QProgressBar::chunk {
            background-color: #E3C6B0;
            border-radius: 4px;
        }
    )");
    m_progressBar->setVisible(false);
    m_mainLayout->addWidget(m_progressBar);
    
    // Status label
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("color: #E3C6B0; font-weight: bold;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setVisible(false);
    m_mainLayout->addWidget(m_statusLabel);
    
    // Buttons
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_importButton = new QPushButton("🚀 Import Contracts");
    m_importButton->setStyleSheet(BUTTON_STYLE);
    m_importButton->setEnabled(false);
    
    m_cancelButton = new QPushButton("❌ Cancel");
    m_cancelButton->setStyleSheet(BUTTON_STYLE);
    
    m_buttonLayout->addWidget(m_importButton);
    m_buttonLayout->addWidget(m_cancelButton);
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_browseButton, &QPushButton::clicked, this, &CsvImportDialog::browseFile);
    connect(m_importButton, &QPushButton::clicked, this, &CsvImportDialog::importContracts);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void CsvImportDialog::browseFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select CSV File",
        QDir::homePath(),
        "CSV Files (*.csv);;All Files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        m_filePathEdit->setText(fileName);
        m_importButton->setEnabled(validateCsvFile(fileName));
    }
}

bool CsvImportDialog::validateCsvFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open the selected file.");
        return false;
    }
    
    QTextStream in(&file);
    if (in.atEnd()) {
        QMessageBox::warning(this, "Error", "The selected file is empty.");
        return false;
    }
    
    // Read first line to check structure
    QString firstLine = in.readLine().trimmed();
    QStringList headers = firstLine.split(',');
    
    // Check if it has the minimum required columns
    if (headers.size() < 6) {
        QMessageBox::warning(this, "Invalid Format", 
                           "The CSV file must have at least 6 columns:\n"
                           "client_name,start_date,end_date,value,status,description");
        return false;
    }
    
    return true;
}

void CsvImportDialog::importContracts()
{
    QString filePath = m_filePathEdit->text();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a CSV file first.");
        return;
    }
    
    // Show progress
    m_progressBar->setVisible(true);
    m_statusLabel->setVisible(true);
    m_statusLabel->setText("📊 Processing contracts...");
    m_importButton->setEnabled(false);
    m_browseButton->setEnabled(false);
    
    QApplication::processEvents();
    
    int successCount = processContracts(filePath);
    
    if (successCount > 0) {
        m_statusLabel->setText(QString("✅ Successfully imported %1 contracts!").arg(successCount));
        QMessageBox::information(this, "Import Complete", 
                               QString("Successfully imported %1 contracts from the CSV file.").arg(successCount));
        accept(); // Close dialog
    } else {
        m_statusLabel->setText("❌ Import failed!");
        QMessageBox::warning(this, "Import Failed", "No contracts were imported. Please check the file format.");
    }
    
    // Reset UI
    m_progressBar->setVisible(false);
    m_importButton->setEnabled(true);
    m_browseButton->setEnabled(true);
}

int CsvImportDialog::processContracts(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }
    
    QTextStream in(&file);
    QStringList lines;
    
    // Read all lines
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    }
    
    if (lines.isEmpty()) {
        return 0;
    }
    
    // Skip header if it looks like a header
    int startIndex = 0;
    if (!lines.isEmpty() && lines.first().toLower().contains("client_name")) {
        startIndex = 1;
    }
    
    int totalLines = lines.size() - startIndex;
    int successCount = 0;
    
    m_progressBar->setMaximum(totalLines);
    m_progressBar->setValue(0);
    
    for (int i = startIndex; i < lines.size(); ++i) {
        QString line = lines[i];
        QStringList fields = line.split(',');
        
        // Ensure we have enough fields
        if (fields.size() < 6) {
            qDebug() << "Skipping line (insufficient fields):" << line;
            continue;
        }
        
        // Extract fields
        QString clientName = fields[0].trimmed().remove('"');
        QString startDateStr = fields[1].trimmed().remove('"');
        QString endDateStr = fields[2].trimmed().remove('"');
        QString valueStr = fields[3].trimmed().remove('"');
        QString status = fields[4].trimmed().remove('"');
        QString description = fields[5].trimmed().remove('"');
        
        // Validate and convert data
        QDate startDate = QDate::fromString(startDateStr, "yyyy-MM-dd");
        QDate endDate = QDate::fromString(endDateStr, "yyyy-MM-dd");
        bool valueOk;
        double value = valueStr.toDouble(&valueOk);
        
        if (clientName.isEmpty() || !startDate.isValid() || !endDate.isValid() || !valueOk) {
            qDebug() << "Skipping line (invalid data):" << line;
            m_progressBar->setValue(i - startIndex + 1);
            QApplication::processEvents();
            continue;
        }
        
        // Generate unique contract ID
        QString contractId = QString("CONT%1%2").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"))
                                                .arg(QString::number(i), 3, '0');
        
        // Set default values if empty
        if (status.isEmpty()) status = "Draft";
        if (description.isEmpty()) description = QString("Imported contract for %1").arg(clientName);
        
        // Add to database
        bool success = m_dbManager->addContract(contractId, clientName, startDate, endDate, value, status, description);
        
        if (success) {
            successCount++;
            qDebug() << "Imported contract:" << contractId << "for" << clientName;
        } else {
            qDebug() << "Failed to import contract for:" << clientName;
        }
        
        // Update progress
        m_progressBar->setValue(i - startIndex + 1);
        m_statusLabel->setText(QString("📊 Processed %1 of %2 contracts...").arg(i - startIndex + 1).arg(totalLines));
        QApplication::processEvents();
        
        // Small delay to show progress
        QThread::msleep(50);
    }
    
    return successCount;
}
