#ifndef CONTRACTDATABASEMANAGER_H
#define CONTRACTDATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QStringList>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>
#include "../../interfaces/icontractservice.h"

class Contract;

/**
 * @brief The ContractDatabaseManager class handles all database operations for contracts
 * 
 * This class provides a clean interface for CRUD operations on contracts,
 * search functionality, and statistics calculations using SQLite database.
 * Implements IContractService interface for consistent service abstraction.
 */
class ContractDatabaseManager : public QObject, public IContractService
{
    Q_OBJECT

public:
    explicit ContractDatabaseManager(QObject *parent = nullptr);
    ~ContractDatabaseManager();

    // Database initialization
    bool initialize(const QString &databasePath = QString());    void shutdown();

    // IContractService implementation
    QString addContract(Contract *contract) override;
    bool updateContract(Contract *contract) override;
    bool deleteContract(const QString &contractId) override;
    Contract* getContract(const QString &contractId) override;
    QList<Contract*> getAllContracts() override;

    // Search and Filter
    QList<Contract*> searchContracts(const QString &searchTerm) override;
    QList<Contract*> getContractsByStatus(const QString &status) override;
    QList<Contract*> getContractsByClient(const QString &clientName) override;
    QList<Contract*> getContractsByDateRange(const QDate &startDate, const QDate &endDate) override;
    QList<Contract*> getActiveContracts() override;
    QList<Contract*> getExpiringContracts(int daysFromNow = 30) override;

    // Statistics and analytics
    QJsonObject getContractStatistics() override;
    QJsonArray getStatusDistribution() override;
    QJsonArray getMonthlyContractCounts() override;
    double getTotalContractValue() override;
    double getAverageContractValue() override;
    int getContractCount() override;

    // Validation
    bool validateContract(const Contract *contract, QString &errorMessage) override;
    bool isContractIdUnique(const QString &contractId) override;

    // Business logic
    QStringList getValidStatuses() override;
    bool canDeleteContract(const QString &contractId) override;
    bool isContractExpired(const QString &contractId) override;
    int getDaysUntilExpiry(const QString &contractId) override;

    // Export/Import
    QJsonArray exportContracts() override;
    bool importContracts(const QJsonArray &contracts, QString &errorMessage) override;

    // Legacy methods (for backward compatibility)
    int getTotalContracts();
    int getActiveContractsCount();
    int getExpiredContracts();
    int getContractsByStatusCount(const QString &status);
    double getActiveContractValue();
    QList<Contract*> getContractsInDateRange(const QDate &startDate, const QDate &endDate);

    // Database utilities
    bool isDatabaseConnected() const;
    QString getLastError() const;
    
    // Enhanced CRUD operations with better error handling
    bool addContracts(const QList<Contract*> &contracts, QStringList &addedIds, QString &errorMessage);
    bool updateContracts(const QList<Contract*> &contracts, QString &errorMessage);
    bool deleteContracts(const QStringList &contractIds, QString &errorMessage);
    QList<Contract*> getContracts(const QStringList &contractIds);
    
    // Database synchronization and maintenance
    bool synchronizeDatabase();
    bool optimizeDatabase();
    bool backupDatabase(const QString &backupPath);
    bool restoreDatabase(const QString &backupPath);
    void clearCache();
    
    // Performance and caching
    void enableCaching(bool enable = true);
    bool isCachingEnabled() const;
    void refreshCache();
    int getCacheSize() const;

signals:
    void contractAdded(const QString &contractId);
    void contractUpdated(const QString &contractId);
    void contractDeleted(const QString &contractId);
    void databaseError(const QString &error);

private slots:
    void onDatabaseError(const QString &error);

private:
    bool createTables();
    Contract* createContractFromQuery(const QSqlQuery &query);
    void bindContractToQuery(QSqlQuery &query, Contract *contract);
    bool executeQuery(QSqlQuery &query, const QString &operation);

    QSqlDatabase m_database;
    QString m_databasePath;
    QString m_lastError;
    bool m_isInitialized;
    
    // Caching for performance
    mutable QHash<QString, Contract*> m_contractCache;
    bool m_cachingEnabled;
    QDateTime m_cacheTimestamp;
};

#endif // CONTRACTDATABASEMANAGER_H
