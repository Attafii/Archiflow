#ifndef DOCUMENTPROCESSOR_H
#define DOCUMENTPROCESSOR_H

#include <QString>
#include <QStringList>

/**
 * @brief Utility class for processing different document formats
 */
class DocumentProcessor
{
public:
    /**
     * @brief Check if the file format is supported
     * @param filePath Path to the file
     * @return true if supported
     */
    static bool isSupportedFormat(const QString &filePath);
    
    /**
     * @brief Get list of supported file extensions
     * @return List of supported extensions
     */
    static QStringList getSupportedExtensions();
    
    /**
     * @brief Extract text content from a document
     * @param filePath Path to the document
     * @return Extracted text content
     */
    static QString extractText(const QString &filePath);
    
    /**
     * @brief Extract text from PDF file
     * @param filePath Path to PDF file
     * @return Extracted text
     */
    static QString extractFromPdf(const QString &filePath);
    
    /**
     * @brief Extract text from CSV file
     * @param filePath Path to CSV file
     * @return Formatted CSV content
     */
    static QString extractFromCsv(const QString &filePath);
    
    /**
     * @brief Extract text from Excel file (.xlsx, .xls)
     * @param filePath Path to Excel file
     * @return Formatted Excel content
     */
    static QString extractFromExcel(const QString &filePath);
    
    /**
     * @brief Get file format from extension
     * @param filePath Path to the file
     * @return File format description
     */
    static QString getFileFormat(const QString &filePath);
    
    /**
     * @brief Get file size in human readable format
     * @param filePath Path to the file
     * @return File size string
     */
    static QString getFileSize(const QString &filePath);

private:
    static QString formatCsvContent(const QString &content, int maxRows = 50);
    static QString formatTableContent(const QStringList &headers, const QList<QStringList> &rows, int maxRows = 50);
};

#endif // DOCUMENTPROCESSOR_H
