#ifndef ICONTRACTEXPORTER_H
#define ICONTRACTEXPORTER_H

#include <QString>
#include <QStringList>
#include <QModelIndexList>

/**
 * @brief Interface for contract export operations
 * 
 * This interface defines the contract for all export-related operations,
 * ensuring consistency across different export implementations.
 */
class IContractExporter
{
public:
    virtual ~IContractExporter() = default;

    enum ExportFormat {
        CSV,
        PDF,
        Excel,
        JSON,
        XML
    };

    enum ExportScope {
        AllContracts,
        FilteredContracts,
        SelectedContracts
    };

    // Core export operations
    virtual bool exportContracts(const QString &filePath, 
                                ExportFormat format, 
                                ExportScope scope = AllContracts) = 0;

    virtual bool exportContractsWithTemplate(const QString &templatePath,
                                            const QString &outputPath,
                                            ExportScope scope = AllContracts) = 0;

    // Export configuration
    virtual void setExportHeaders(const QStringList &headers) = 0;
    virtual void setDateFormat(const QString &format) = 0;
    virtual void setCurrencyFormat(const QString &format) = 0;
    virtual void setIncludeStatistics(bool include) = 0;

    // Export status and feedback
    virtual QString getLastError() const = 0;
    virtual int getLastExportCount() const = 0;
    virtual QString generateExportSummary() const = 0;

    // Export validation
    virtual bool validateExportPath(const QString &path, ExportFormat format) const = 0;
    virtual QStringList getSupportedFormats() const = 0;
    virtual QString getRecommendedExtension(ExportFormat format) const = 0;

signals:
    virtual void exportStarted() = 0;
    virtual void exportProgress(int current, int total, const QString &item) = 0;
    virtual void exportCompleted(bool success, const QString &message) = 0;
};

#endif // ICONTRACTEXPORTER_H
