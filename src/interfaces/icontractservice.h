#ifndef ICONTRACTSERVICE_H
#define ICONTRACTSERVICE_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

class Contract;

/**
 * @brief Interface for contract service operations
 * 
 * This interface defines the contract for all contract-related business logic
 * and data operations, ensuring consistency across different implementations.
 */
class IContractService
{
public:
    virtual ~IContractService() = default;

    // Core CRUD operations
    virtual QString addContract(Contract *contract) = 0;
    virtual bool updateContract(Contract *contract) = 0;
    virtual bool deleteContract(const QString &contractId) = 0;
    virtual Contract* getContract(const QString &contractId) = 0;
    virtual QList<Contract*> getAllContracts() = 0;

    // Search and filter operations
    virtual QList<Contract*> searchContracts(const QString &searchTerm) = 0;
    virtual QList<Contract*> getContractsByStatus(const QString &status) = 0;
    virtual QList<Contract*> getContractsByClient(const QString &clientName) = 0;
    virtual QList<Contract*> getContractsByDateRange(const QDate &startDate, const QDate &endDate) = 0;
    virtual QList<Contract*> getActiveContracts() = 0;
    virtual QList<Contract*> getExpiringContracts(int daysFromNow = 30) = 0;

    // Statistics and analytics
    virtual QJsonObject getContractStatistics() = 0;
    virtual QJsonArray getStatusDistribution() = 0;
    virtual QJsonArray getMonthlyContractCounts() = 0;
    virtual double getTotalContractValue() = 0;
    virtual double getAverageContractValue() = 0;
    virtual int getContractCount() = 0;

    // Validation
    virtual bool validateContract(const Contract *contract, QString &errorMessage) = 0;
    virtual bool isContractIdUnique(const QString &contractId) = 0;

    // Business logic
    virtual QStringList getValidStatuses() = 0;
    virtual bool canDeleteContract(const QString &contractId) = 0;
    virtual bool isContractExpired(const QString &contractId) = 0;
    virtual int getDaysUntilExpiry(const QString &contractId) = 0;

    // Export/Import
    virtual QJsonArray exportContracts() = 0;
    virtual bool importContracts(const QJsonArray &contracts, QString &errorMessage) = 0;
};

#endif // ICONTRACTSERVICE_H
