#ifndef CONTRACTEXPORTMANAGER_H
#define CONTRACTEXPORTMANAGER_H

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QProgressDialog>
#include "interfaces/icontractexporter.h"

class Contract;
class IContractService;

/**
 * @brief The ContractExportManager class handles all contract export operations
 * 
 * This class provides comprehensive export functionality including CSV, PDF, Excel,
 * JSON, and XML formats with customizable templates and progress tracking.
 * Implements IContractExporter interface for consistent export behavior.
 */
class ContractExportManager : public QObject, public IContractExporter
{
    Q_OBJECT

public:
    explicit ContractExportManager(QObject *parent = nullptr);
    ~ContractExportManager();

    // Set data source
    void setContractService(IContractService *service);
    void setContracts(const QList<Contract*> &contracts);
    void setSelectedContracts(const QList<Contract*> &selected);
    void setFilteredContracts(const QList<Contract*> &filtered);

    // IContractExporter implementation
    bool exportContracts(const QString &filePath, 
                        ExportFormat format, 
                        ExportScope scope = AllContracts) override;

    bool exportContractsWithTemplate(const QString &templatePath,
                                    const QString &outputPath,
                                    ExportScope scope = AllContracts) override;

    // Export configuration
    void setExportHeaders(const QStringList &headers) override;
    void setDateFormat(const QString &format) override;
    void setCurrencyFormat(const QString &format) override;
    void setIncludeStatistics(bool include) override;

    // Export status and feedback
    QString getLastError() const override;
    int getLastExportCount() const override;
    QString generateExportSummary() const override;

    // Export validation
    bool validateExportPath(const QString &path, ExportFormat format) const override;
    QStringList getSupportedFormats() const override;
    QString getRecommendedExtension(ExportFormat format) const override;

    // Additional export features
    void setCompanyLogo(const QString &logoPath);
    void setExportMetadata(const QString &title, const QString &author, const QString &subject);
    void setColumnWidths(const QList<int> &widths);
    bool exportStatisticsOnly(const QString &filePath, ExportFormat format);

signals:
    void exportStarted() override;
    void exportProgress(int current, int total, const QString &item) override;
    void exportCompleted(bool success, const QString &message) override;

private slots:
    void updateProgress();

private:
    // Export format handlers
    bool exportToCSV(const QString &filePath, const QList<Contract*> &contracts);
    bool exportToPDF(const QString &filePath, const QList<Contract*> &contracts);
    bool exportToExcel(const QString &filePath, const QList<Contract*> &contracts);
    bool exportToJSON(const QString &filePath, const QList<Contract*> &contracts);
    bool exportToXML(const QString &filePath, const QList<Contract*> &contracts);

    // Helper methods
    QList<Contract*> getContractsForScope(ExportScope scope);
    QString formatValue(const QVariant &value, const QString &type) const;
    QString formatDate(const QDate &date) const;
    QString formatCurrency(double value) const;
    void generateStatisticsSection(QTextStream &stream, const QList<Contract*> &contracts);

    // Member variables
    IContractService *m_contractService;
    QList<Contract*> m_allContracts;
    QList<Contract*> m_selectedContracts;
    QList<Contract*> m_filteredContracts;

    // Export configuration
    QStringList m_exportHeaders;
    QString m_dateFormat;
    QString m_currencyFormat;
    bool m_includeStatistics;
    QString m_companyLogo;
    QString m_exportTitle;
    QString m_exportAuthor;
    QString m_exportSubject;
    QList<int> m_columnWidths;

    // Export status
    QString m_lastError;
    int m_lastExportCount;
    QTimer *m_progressTimer;
    QProgressDialog *m_progressDialog;
    int m_currentProgress;
    int m_totalProgress;
};

#endif // CONTRACTEXPORTMANAGER_H
