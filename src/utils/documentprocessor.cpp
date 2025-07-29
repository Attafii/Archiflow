#include "documentprocessor.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>

bool DocumentProcessor::isSupportedFormat(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    QStringList supportedExtensions = {"pdf", "csv", "xlsx", "xls", "txt"};
    return supportedExtensions.contains(extension);
}

QStringList DocumentProcessor::getSupportedExtensions()
{
    return {"*.pdf", "*.csv", "*.xlsx", "*.xls", "*.txt"};
}

QString DocumentProcessor::extractText(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "pdf") {
        return extractFromPdf(filePath);
    } else if (extension == "csv") {
        return extractFromCsv(filePath);
    } else if (extension == "xlsx" || extension == "xls") {
        return extractFromExcel(filePath);
    } else if (extension == "txt") {
        // Simple text file
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            return in.readAll();
        }
    }
    
    return QString("Unsupported file format: %1").arg(extension);
}

QString DocumentProcessor::extractFromPdf(const QString &filePath)
{
    // For now, return a placeholder since PDF parsing requires external libraries
    // In a real implementation, you would use libraries like Poppler or similar
    QFileInfo fileInfo(filePath);
    return QString("PDF Document: %1\n\n"
                  "Note: PDF text extraction is not implemented in this demo.\n"
                  "File size: %2\n"
                  "Please describe the content of this PDF document for analysis.")
           .arg(fileInfo.fileName())
           .arg(getFileSize(filePath));
}

QString DocumentProcessor::extractFromCsv(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString("Error: Could not open CSV file: %1").arg(filePath);
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    return formatCsvContent(content);
}

QString DocumentProcessor::extractFromExcel(const QString &filePath)
{
    // For now, return a placeholder since Excel parsing requires external libraries
    // In a real implementation, you would use libraries like QXlsx or similar
    QFileInfo fileInfo(filePath);
    return QString("Excel Document: %1\n\n"
                  "Note: Excel file parsing is not implemented in this demo.\n"
                  "File size: %2\n"
                  "Please describe the content of this Excel document for analysis.")
           .arg(fileInfo.fileName())
           .arg(getFileSize(filePath));
}

QString DocumentProcessor::getFileFormat(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "pdf") {
        return "PDF Document";
    } else if (extension == "csv") {
        return "CSV Spreadsheet";
    } else if (extension == "xlsx") {
        return "Excel Workbook (XLSX)";
    } else if (extension == "xls") {
        return "Excel Workbook (XLS)";
    } else if (extension == "txt") {
        return "Text Document";
    }
    
    return QString("Unknown Format (%1)").arg(extension.toUpper());
}

QString DocumentProcessor::getFileSize(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    qint64 size = fileInfo.size();
    
    if (size < 1024) {
        return QString("%1 bytes").arg(size);
    } else if (size < 1024 * 1024) {
        return QString("%1 KB").arg(size / 1024);
    } else {
        return QString("%1 MB").arg(size / (1024 * 1024));
    }
}

QString DocumentProcessor::formatCsvContent(const QString &content, int maxRows)
{
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return "Empty CSV file";
    }
    
    QString result;
    result += QString("CSV Document Analysis\n");
    result += QString("=====================\n\n");
    result += QString("Total rows: %1\n").arg(lines.size());
    
    if (!lines.isEmpty()) {
        // Parse header
        QString header = lines.first();
        QStringList headers = header.split(',');
        result += QString("Columns: %1\n").arg(headers.size());
        result += QString("Column names: %1\n\n").arg(headers.join(", "));
        
        // Show first few rows
        int rowsToShow = qMin(maxRows, lines.size());
        result += QString("Sample data (first %1 rows):\n").arg(rowsToShow);
        result += QString("----------------------------\n");
        
        for (int i = 0; i < rowsToShow; ++i) {
            result += QString("Row %1: %2\n").arg(i + 1).arg(lines[i]);
        }
        
        if (lines.size() > maxRows) {
            result += QString("\n... and %1 more rows\n").arg(lines.size() - maxRows);
        }
    }
    
    return result;
}

QString DocumentProcessor::formatTableContent(const QStringList &headers, const QList<QStringList> &rows, int maxRows)
{
    QString result;
    result += QString("Headers: %1\n").arg(headers.join(" | "));
    result += QString("%1\n").arg(QString("-").repeated(headers.join(" | ").length()));
    
    int rowsToShow = qMin(maxRows, rows.size());
    for (int i = 0; i < rowsToShow; ++i) {
        result += QString("%1\n").arg(rows[i].join(" | "));
    }
    
    if (rows.size() > maxRows) {
        result += QString("... and %1 more rows\n").arg(rows.size() - maxRows);
    }
    
    return result;
}
