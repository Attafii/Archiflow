#ifndef ICONTRACTIMPORTER_H
#define ICONTRACTIMPORTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class Contract;

/**
 * @brief Interface for contract import functionality
 * 
 * This interface defines the contract for importing contracts from various
 * file formats like CSV, Excel, JSON, and XML.
 */
class IContractImporter
{
public:
    virtual ~IContractImporter() = default;

    // Import operations
    virtual bool importFromFile(const QString &filePath) = 0;
    virtual bool importFromData(const QByteArray &data, const QString &format) = 0;
    virtual QList<Contract*> getImportedContracts() const = 0;
    
    // Validation and preview
    virtual bool validateFile(const QString &filePath) = 0;
    virtual QStringList getRequiredFields() const = 0;
    virtual QStringList getOptionalFields() const = 0;
    virtual QStringList getDetectedFields(const QString &filePath) = 0;
    virtual QList<QVariantMap> previewData(const QString &filePath, int maxRows = 10) = 0;
    
    // Field mapping
    virtual void setFieldMapping(const QVariantMap &mapping) = 0;
    virtual QVariantMap getFieldMapping() const = 0;
    virtual QStringList getSuggestedMapping(const QStringList &sourceFields) = 0;
    
    // Import settings
    virtual void setImportSettings(const QVariantMap &settings) = 0;
    virtual QVariantMap getImportSettings() const = 0;
    virtual void setSkipDuplicates(bool skip) = 0;
    virtual void setValidateData(bool validate) = 0;
    virtual void setCreateBackup(bool backup) = 0;
    
    // Progress and status
    virtual int getTotalRecords() const = 0;
    virtual int getProcessedRecords() const = 0;
    virtual int getSuccessfulImports() const = 0;
    virtual int getFailedImports() const = 0;
    virtual QStringList getImportErrors() const = 0;
    virtual QStringList getImportWarnings() const = 0;
    
    // Supported formats
    virtual QStringList getSupportedFormats() const = 0;
    virtual QString getFormatDescription(const QString &format) const = 0;
    virtual bool isFormatSupported(const QString &format) const = 0;
    
    // Status and capabilities
    virtual bool isReady() const = 0;
    virtual QString getLastError() const = 0;
    virtual void reset() = 0;
};

Q_DECLARE_INTERFACE(IContractImporter, "com.archiflow.IContractImporter/1.0")

#endif // ICONTRACTIMPORTER_H
