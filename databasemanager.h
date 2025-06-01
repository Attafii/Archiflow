#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QString>
#include <QVariant>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool initializeDatabase();
    
    // CRUD Operations
    bool addContract(const QString &contractId, const QString &clientName,
                    const QDate &startDate, const QDate &endDate,
                    double value, const QString &status,
                    const QString &description);
    
    bool updateContract(const QString &contractId, const QString &clientName,
                      const QDate &startDate, const QDate &endDate,
                      double value, const QString &status,
                      const QString &description);
    
    bool deleteContract(const QString &contractId);
    
    QSqlQuery getAllContracts();
    QSqlQuery getFilteredContracts(const QString &searchTerm, const QString &status,
                                  const QDate &startDate, const QDate &endDate,
                                  double minValue, double maxValue);
    
    // Statistics
    double getTotalContractValue();
    int getContractCountByStatus(const QString &status);
    QMap<QString, int> getContractStatusDistribution();
    QMap<QDate, double> getContractValueTrend(const QDate &startDate, const QDate &endDate);
    
    // Contract Expiration Management
    QSqlQuery getExpiringContracts(int daysThreshold);

private:
    QSqlDatabase db;
    bool createTables();
};

#endif // DATABASEMANAGER_H