#include "contractexportmanager.h"
#include "contract.h"
#include "../../interfaces/icontractservice.h"
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QStringConverter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QXmlStreamWriter>
#include <QProgressDialog>
#include <QMessageBox>
#include <QLocale>
#include <QDateTime>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCharFormat>

ContractExportManager::ContractExportManager(QObject *parent)
    : QObject(parent)
    , m_contractService(nullptr)
    , m_dateFormat("dd/MM/yyyy")
    , m_currencyFormat("$#,##0.00")
    , m_includeStatistics(true)
    , m_exportTitle("Contract Export Report")
    , m_exportAuthor("ArchiFlow Application")
    , m_exportSubject("Contract Management Export")
    , m_lastExportCount(0)
    , m_progressTimer(new QTimer(this))
    , m_progressDialog(nullptr)
    , m_currentProgress(0)
    , m_totalProgress(0)
{
    // Set default headers
    m_exportHeaders << "Client Name" << "Start Date" << "End Date" 
                   << "Value" << "Status" << "Payment Terms" << "Description";

    // Set default column widths (for PDF export)
    m_columnWidths << 120 << 80 << 80 << 100 << 80 << 80 << 200;

    connect(m_progressTimer, &QTimer::timeout, this, &ContractExportManager::updateProgress);
}

ContractExportManager::~ContractExportManager()
{
    if (m_progressDialog) {
        delete m_progressDialog;
    }
}

void ContractExportManager::setContractService(IContractService *service)
{
    m_contractService = service;
}

void ContractExportManager::setContracts(const QList<Contract*> &contracts)
{
    m_allContracts = contracts;
}

void ContractExportManager::setSelectedContracts(const QList<Contract*> &selected)
{
    m_selectedContracts = selected;
}

void ContractExportManager::setFilteredContracts(const QList<Contract*> &filtered)
{
    m_filteredContracts = filtered;
}

bool ContractExportManager::exportContracts(const QString &filePath, 
                                           ExportFormat format, 
                                           ExportScope scope)
{
    m_lastError.clear();
    m_lastExportCount = 0;

    // Validate export path
    if (!validateExportPath(filePath, format)) {
        return false;
    }

    // Get contracts for the specified scope
    QList<Contract*> contracts = getContractsForScope(scope);
    if (contracts.isEmpty()) {
        m_lastError = "No contracts available for export";
        return false;
    }

    // Set up progress tracking
    m_totalProgress = contracts.count();
    m_currentProgress = 0;

    // Create progress dialog
    if (m_progressDialog) {
        delete m_progressDialog;
    }
    m_progressDialog = new QProgressDialog("Exporting contracts...", "Cancel", 0, m_totalProgress);
    m_progressDialog->setWindowTitle("Export Progress");
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->show();

    emit exportStarted();

    bool success = false;
    switch (format) {
        case CSV:
            success = exportToCSV(filePath, contracts);
            break;
        case PDF:
            success = exportToPDF(filePath, contracts);
            break;
        case Excel:
            success = exportToExcel(filePath, contracts);
            break;
        case JSON:
            success = exportToJSON(filePath, contracts);
            break;
        case XML:
            success = exportToXML(filePath, contracts);
            break;
    }

    // Clean up progress dialog
    if (m_progressDialog) {
        m_progressDialog->close();
        delete m_progressDialog;
        m_progressDialog = nullptr;
    }

    emit exportCompleted(success, success ? "Export completed successfully" : m_lastError);
    return success;
}

bool ContractExportManager::exportContractsWithTemplate(const QString &templatePath,
                                                       const QString &outputPath,
                                                       ExportScope scope)
{
    Q_UNUSED(templatePath)
    Q_UNUSED(outputPath)
    Q_UNUSED(scope)
    
    // This is a simplified template implementation
    // In a full implementation, you would parse template files
    m_lastError = "Template-based export not yet implemented";
    return false;
}

void ContractExportManager::setExportHeaders(const QStringList &headers)
{
    m_exportHeaders = headers;
}

void ContractExportManager::setDateFormat(const QString &format)
{
    m_dateFormat = format;
}

void ContractExportManager::setCurrencyFormat(const QString &format)
{
    m_currencyFormat = format;
}

void ContractExportManager::setIncludeStatistics(bool include)
{
    m_includeStatistics = include;
}

QString ContractExportManager::getLastError() const
{
    return m_lastError;
}

int ContractExportManager::getLastExportCount() const
{
    return m_lastExportCount;
}

QString ContractExportManager::generateExportSummary() const
{
    return QString("Exported %1 contracts successfully").arg(m_lastExportCount);
}

bool ContractExportManager::validateExportPath(const QString &path, ExportFormat format) const
{
    QFileInfo fileInfo(path);
    
    // Check if directory exists
    if (!fileInfo.dir().exists()) {
        const_cast<ContractExportManager*>(this)->m_lastError = "Export directory does not exist";
        return false;
    }

    // Check file extension
    QString expectedExt = getRecommendedExtension(format);
    if (!fileInfo.suffix().toLower().endsWith(expectedExt.mid(1))) {
        const_cast<ContractExportManager*>(this)->m_lastError = 
            QString("File should have %1 extension").arg(expectedExt);
        return false;
    }

    return true;
}

QStringList ContractExportManager::getSupportedFormats() const
{
    return QStringList() << "CSV" << "PDF" << "Excel" << "JSON" << "XML";
}

QString ContractExportManager::getRecommendedExtension(ExportFormat format) const
{
    switch (format) {
        case CSV: return ".csv";
        case PDF: return ".pdf";
        case Excel: return ".xlsx";
        case JSON: return ".json";
        case XML: return ".xml";
        default: return ".txt";
    }
}

void ContractExportManager::setCompanyLogo(const QString &logoPath)
{
    m_companyLogo = logoPath;
}

void ContractExportManager::setExportMetadata(const QString &title, const QString &author, const QString &subject)
{
    m_exportTitle = title;
    m_exportAuthor = author;
    m_exportSubject = subject;
}

void ContractExportManager::setColumnWidths(const QList<int> &widths)
{
    m_columnWidths = widths;
}

bool ContractExportManager::exportStatisticsOnly(const QString &filePath, ExportFormat format)
{
    QList<Contract*> contracts = getContractsForScope(AllContracts);
    
    if (format == CSV) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_lastError = "Cannot open file for writing: " + file.errorString();
            return false;        }

        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        
        // Write statistics header
        stream << "Contract Statistics Report\n";
        stream << "Generated on: " << QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") << "\n\n";
        
        generateStatisticsSection(stream, contracts);
        return true;
    }
    
    return false;
}

void ContractExportManager::updateProgress()
{
    if (m_progressDialog) {
        m_progressDialog->setValue(m_currentProgress);
        QApplication::processEvents();
    }
}

bool ContractExportManager::exportToCSV(const QString &filePath, const QList<Contract*> &contracts)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Cannot open file for writing: " + file.errorString();
        return false;
    }    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // Write header
    stream << m_exportHeaders.join(",") << "\n";

    // Write contract data
    for (Contract *contract : contracts) {
        if (m_progressDialog && m_progressDialog->wasCanceled()) {
            m_lastError = "Export cancelled by user";
            return false;
        }

        QStringList row;
        row << contract->clientName()
            << formatDate(contract->startDate())
            << formatDate(contract->endDate())
            << formatCurrency(contract->value())
            << contract->status()
            << QString::number(contract->paymentTerms())
            << QString("\"%1\"").arg(contract->description().replace("\"", "\"\""));

        stream << row.join(",") << "\n";
        
        m_currentProgress++;
        emit exportProgress(m_currentProgress, m_totalProgress, contract->clientName());
        updateProgress();
    }

    // Add statistics if requested
    if (m_includeStatistics) {
        stream << "\n";
        generateStatisticsSection(stream, contracts);
    }

    m_lastExportCount = contracts.count();
    return true;
}

bool ContractExportManager::exportToPDF(const QString &filePath, const QList<Contract*> &contracts)
{
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Landscape);

    QTextDocument document;
    QTextCursor cursor(&document);

    // Set up formats
    QTextCharFormat titleFormat;
    titleFormat.setFontPointSize(16);
    titleFormat.setFontWeight(QFont::Bold);

    QTextCharFormat headerFormat;
    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setBackground(QColor("#E3C6B0"));

    // Add title
    cursor.insertText(m_exportTitle + "\n", titleFormat);
    cursor.insertText("Generated on: " + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") + "\n\n");

    // Create table
    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setCellPadding(4);
    tableFormat.setCellSpacing(0);

    QTextTable *table = cursor.insertTable(contracts.count() + 1, m_exportHeaders.count(), tableFormat);

    // Add headers
    for (int col = 0; col < m_exportHeaders.count(); ++col) {
        QTextTableCell cell = table->cellAt(0, col);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.setCharFormat(headerFormat);
        cellCursor.insertText(m_exportHeaders.at(col));
    }

    // Add contract data
    for (int row = 0; row < contracts.count(); ++row) {
        if (m_progressDialog && m_progressDialog->wasCanceled()) {
            m_lastError = "Export cancelled by user";
            return false;
        }

        Contract *contract = contracts.at(row);
        
        QStringList data;
        data << contract->clientName()
             << formatDate(contract->startDate())
             << formatDate(contract->endDate())
             << formatCurrency(contract->value())
             << contract->status()
             << QString::number(contract->paymentTerms())
             << contract->description();

        for (int col = 0; col < data.count(); ++col) {
            QTextTableCell cell = table->cellAt(row + 1, col);
            QTextCursor cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(data.at(col));
        }

        m_currentProgress++;
        emit exportProgress(m_currentProgress, m_totalProgress, contract->clientName());
        updateProgress();
    }

    // Add statistics if requested
    if (m_includeStatistics) {
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\n\nContract Statistics:\n");
        
        // Generate basic statistics
        double totalValue = 0;
        QMap<QString, int> statusCounts;
        for (Contract *contract : contracts) {
            totalValue += contract->value();
            statusCounts[contract->status()]++;
        }

        cursor.insertText(QString("Total Contracts: %1\n").arg(contracts.count()));
        cursor.insertText(QString("Total Value: %1\n").arg(formatCurrency(totalValue)));
        cursor.insertText(QString("Average Value: %1\n").arg(formatCurrency(totalValue / contracts.count())));
        
        cursor.insertText("\nStatus Distribution:\n");
        for (auto it = statusCounts.begin(); it != statusCounts.end(); ++it) {
            cursor.insertText(QString("%1: %2\n").arg(it.key()).arg(it.value()));
        }
    }

    document.print(&printer);
    m_lastExportCount = contracts.count();
    return true;
}

bool ContractExportManager::exportToExcel(const QString &filePath, const QList<Contract*> &contracts)
{
    // This would require an Excel library like QXlsx
    // For now, export as enhanced CSV that can be opened in Excel
    QString csvPath = filePath;
    csvPath.replace(".xlsx", ".csv");
    return exportToCSV(csvPath, contracts);
}

bool ContractExportManager::exportToJSON(const QString &filePath, const QList<Contract*> &contracts)
{
    QJsonObject exportData;
    exportData["title"] = m_exportTitle;
    exportData["author"] = m_exportAuthor;
    exportData["generated"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    exportData["count"] = contracts.count();

    QJsonArray contractsArray;
    for (Contract *contract : contracts) {
        if (m_progressDialog && m_progressDialog->wasCanceled()) {
            m_lastError = "Export cancelled by user";
            return false;
        }

        QJsonObject contractObj;
        contractObj["id"] = contract->id();
        contractObj["clientName"] = contract->clientName();
        contractObj["startDate"] = contract->startDate().toString(Qt::ISODate);
        contractObj["endDate"] = contract->endDate().toString(Qt::ISODate);
        contractObj["value"] = contract->value();
        contractObj["status"] = contract->status();
        contractObj["paymentTerms"] = contract->paymentTerms();
        contractObj["description"] = contract->description();
        contractObj["hasNonCompeteClause"] = contract->hasNonCompeteClause();

        contractsArray.append(contractObj);

        m_currentProgress++;
        emit exportProgress(m_currentProgress, m_totalProgress, contract->clientName());
        updateProgress();
    }

    exportData["contracts"] = contractsArray;

    // Add statistics if requested
    if (m_includeStatistics) {
        QJsonObject stats;
        double totalValue = 0;
        QMap<QString, int> statusCounts;
        
        for (Contract *contract : contracts) {
            totalValue += contract->value();
            statusCounts[contract->status()]++;
        }

        stats["totalContracts"] = contracts.count();
        stats["totalValue"] = totalValue;
        stats["averageValue"] = totalValue / contracts.count();

        QJsonObject statusDistribution;
        for (auto it = statusCounts.begin(); it != statusCounts.end(); ++it) {
            statusDistribution[it.key()] = it.value();
        }
        stats["statusDistribution"] = statusDistribution;
        
        exportData["statistics"] = stats;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = "Cannot open file for writing: " + file.errorString();
        return false;
    }

    QJsonDocument document(exportData);
    file.write(document.toJson());
    
    m_lastExportCount = contracts.count();
    return true;
}

bool ContractExportManager::exportToXML(const QString &filePath, const QList<Contract*> &contracts)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = "Cannot open file for writing: " + file.errorString();
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    writer.writeStartElement("ContractExport");
    writer.writeAttribute("title", m_exportTitle);
    writer.writeAttribute("author", m_exportAuthor);
    writer.writeAttribute("generated", QDateTime::currentDateTime().toString(Qt::ISODate));
    writer.writeAttribute("count", QString::number(contracts.count()));

    writer.writeStartElement("Contracts");

    for (Contract *contract : contracts) {
        if (m_progressDialog && m_progressDialog->wasCanceled()) {
            m_lastError = "Export cancelled by user";
            return false;
        }

        writer.writeStartElement("Contract");
        writer.writeAttribute("id", contract->id());
        
        writer.writeTextElement("ClientName", contract->clientName());
        writer.writeTextElement("StartDate", contract->startDate().toString(Qt::ISODate));
        writer.writeTextElement("EndDate", contract->endDate().toString(Qt::ISODate));
        writer.writeTextElement("Value", QString::number(contract->value()));
        writer.writeTextElement("Status", contract->status());
        writer.writeTextElement("PaymentTerms", QString::number(contract->paymentTerms()));
        writer.writeTextElement("Description", contract->description());
        writer.writeTextElement("HasNonCompeteClause", contract->hasNonCompeteClause() ? "true" : "false");
        
        writer.writeEndElement(); // Contract

        m_currentProgress++;
        emit exportProgress(m_currentProgress, m_totalProgress, contract->clientName());
        updateProgress();
    }

    writer.writeEndElement(); // Contracts

    // Add statistics if requested
    if (m_includeStatistics) {
        writer.writeStartElement("Statistics");
        
        double totalValue = 0;
        QMap<QString, int> statusCounts;
        for (Contract *contract : contracts) {
            totalValue += contract->value();
            statusCounts[contract->status()]++;
        }

        writer.writeTextElement("TotalContracts", QString::number(contracts.count()));
        writer.writeTextElement("TotalValue", QString::number(totalValue));
        writer.writeTextElement("AverageValue", QString::number(totalValue / contracts.count()));

        writer.writeStartElement("StatusDistribution");
        for (auto it = statusCounts.begin(); it != statusCounts.end(); ++it) {
            writer.writeStartElement("Status");
            writer.writeAttribute("name", it.key());
            writer.writeAttribute("count", QString::number(it.value()));
            writer.writeEndElement();
        }
        writer.writeEndElement(); // StatusDistribution
        
        writer.writeEndElement(); // Statistics
    }

    writer.writeEndElement(); // ContractExport
    writer.writeEndDocument();

    m_lastExportCount = contracts.count();
    return true;
}

QList<Contract*> ContractExportManager::getContractsForScope(ExportScope scope)
{
    switch (scope) {
        case AllContracts:
            return m_allContracts;
        case FilteredContracts:
            return m_filteredContracts.isEmpty() ? m_allContracts : m_filteredContracts;
        case SelectedContracts:
            return m_selectedContracts;
        default:
            return m_allContracts;
    }
}

QString ContractExportManager::formatValue(const QVariant &value, const QString &type) const
{
    if (type == "date") {
        return formatDate(value.toDate());
    } else if (type == "currency") {
        return formatCurrency(value.toDouble());
    }
    return value.toString();
}

QString ContractExportManager::formatDate(const QDate &date) const
{
    return date.toString(m_dateFormat);
}

QString ContractExportManager::formatCurrency(double value) const
{
    QLocale locale;
    return locale.toCurrencyString(value);
}

void ContractExportManager::generateStatisticsSection(QTextStream &stream, const QList<Contract*> &contracts)
{
    stream << "Contract Statistics:\n";
    stream << "Total Contracts," << contracts.count() << "\n";
    
    double totalValue = 0;
    QMap<QString, int> statusCounts;
    int expiredCount = 0;
    QDate currentDate = QDate::currentDate();
    
    for (Contract *contract : contracts) {
        totalValue += contract->value();
        statusCounts[contract->status()]++;
        if (contract->endDate() < currentDate) {
            expiredCount++;
        }
    }
    
    stream << "Total Value," << formatCurrency(totalValue) << "\n";
    stream << "Average Value," << formatCurrency(totalValue / contracts.count()) << "\n";
    stream << "Expired Contracts," << expiredCount << "\n\n";
    
    stream << "Status Distribution:\n";
    for (auto it = statusCounts.begin(); it != statusCounts.end(); ++it) {
        stream << it.key() << "," << it.value() << "\n";
    }
}
