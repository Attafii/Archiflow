#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QDir::currentPath() + "/contracts.db";
    db.setDatabaseName(dbPath);
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::initializeDatabase()
{
    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        return false;
    }
    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;
    QString createTableQuery = 
        "CREATE TABLE IF NOT EXISTS contracts ("
        "contract_id TEXT PRIMARY KEY,"
        "client_name TEXT NOT NULL,"
        "start_date DATE NOT NULL,"
        "end_date DATE NOT NULL,"
        "value REAL NOT NULL,"
        "status TEXT NOT NULL,"
        "description TEXT,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::addContract(const QString &contractId, const QString &clientName,
                                const QDate &startDate, const QDate &endDate,
                                double value, const QString &status,
                                const QString &description)
{
    QSqlQuery query;
    query.prepare("INSERT INTO contracts (contract_id, client_name, start_date, "
                 "end_date, value, status, description) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(contractId);
    query.addBindValue(clientName);
    query.addBindValue(startDate.toString(Qt::ISODate));
    query.addBindValue(endDate.toString(Qt::ISODate));
    query.addBindValue(value);
    query.addBindValue(status);
    query.addBindValue(description);

    if (!query.exec()) {
        qDebug() << "Error adding contract:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateContract(const QString &contractId, const QString &clientName,
                                   const QDate &startDate, const QDate &endDate,
                                   double value, const QString &status,
                                   const QString &description)
{
    QSqlQuery query;
    query.prepare("UPDATE contracts SET client_name = ?, start_date = ?, "
                 "end_date = ?, value = ?, status = ?, description = ? "
                 "WHERE contract_id = ?");
    
    query.addBindValue(clientName);
    query.addBindValue(startDate.toString(Qt::ISODate));
    query.addBindValue(endDate.toString(Qt::ISODate));
    query.addBindValue(value);
    query.addBindValue(status);
    query.addBindValue(description);
    query.addBindValue(contractId);

    if (!query.exec()) {
        qDebug() << "Error updating contract:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteContract(const QString &contractId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM contracts WHERE contract_id = ?");
    query.addBindValue(contractId);

    if (!query.exec()) {
        qDebug() << "Error deleting contract:" << query.lastError().text();
        return false;
    }
    return true;
}

QSqlQuery DatabaseManager::getAllContracts()
{
    QSqlQuery query;
    query.prepare("SELECT * FROM contracts ORDER BY created_at DESC");
    if (!query.exec()) {
        qDebug() << "Error fetching contracts:" << query.lastError().text();
    }
    return query;
}

QSqlQuery DatabaseManager::getFilteredContracts(const QString &searchTerm, const QString &status,
                                              const QDate &startDate, const QDate &endDate,
                                              double minValue, double maxValue)
{
    QString queryStr = "SELECT * FROM contracts WHERE 1=1";
    QStringList conditions;
    QSqlQuery query;

    if (!searchTerm.isEmpty()) {
        conditions << "(contract_id LIKE ? OR client_name LIKE ? OR description LIKE ?)";
    }
    if (!status.isEmpty() && status != "All Contracts") {
        conditions << "status = ?";
    }
    if (startDate.isValid()) {
        conditions << "start_date >= ?";
    }
    if (endDate.isValid()) {
        conditions << "end_date <= ?";
    }
    if (minValue > 0) {
        conditions << "value >= ?";
    }
    if (maxValue > 0) {
        conditions << "value <= ?";
    }

    if (!conditions.isEmpty()) {
        queryStr += " AND " + conditions.join(" AND ");
    }
    queryStr += " ORDER BY created_at DESC";

    query.prepare(queryStr);

    if (!searchTerm.isEmpty()) {
        QString pattern = "%" + searchTerm + "%";
        query.addBindValue(pattern);
        query.addBindValue(pattern);
        query.addBindValue(pattern);
    }
    if (!status.isEmpty() && status != "All Contracts") {
        query.addBindValue(status);
    }
    if (startDate.isValid()) {
        query.addBindValue(startDate.toString(Qt::ISODate));
    }
    if (endDate.isValid()) {
        query.addBindValue(endDate.toString(Qt::ISODate));
    }
    if (minValue > 0) {
        query.addBindValue(minValue);
    }
    if (maxValue > 0) {
        query.addBindValue(maxValue);
    }

    if (!query.exec()) {
        qDebug() << "Error filtering contracts:" << query.lastError().text();
    }
    return query;
}

double DatabaseManager::getTotalContractValue()
{
    QSqlQuery query;
    query.prepare("SELECT SUM(value) FROM contracts");
    
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

QSqlQuery DatabaseManager::getExpiringContracts(int daysThreshold)
{
    QSqlQuery query;
    QString queryStr = "SELECT * FROM contracts WHERE status = 'Active' AND "
                      "date(end_date) <= date('now', '+' || ? || ' days') AND "
                      "date(end_date) >= date('now')";
    
    query.prepare(queryStr);
    query.addBindValue(daysThreshold);
    
    if (!query.exec()) {
        qDebug() << "Error fetching expiring contracts:" << query.lastError().text();
    }
    return query;
}

int DatabaseManager::getContractCountByStatus(const QString &status)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM contracts WHERE status = ?");
    query.addBindValue(status);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QMap<QString, int> DatabaseManager::getContractStatusDistribution()
{
    QMap<QString, int> distribution;
    QSqlQuery query;
    query.prepare("SELECT status, COUNT(*) FROM contracts GROUP BY status");
    
    if (query.exec()) {
        while (query.next()) {
            distribution.insert(query.value(0).toString(), query.value(1).toInt());
        }
    }
    return distribution;
}

QMap<QDate, double> DatabaseManager::getContractValueTrend(const QDate &startDate, const QDate &endDate)
{
    QMap<QDate, double> trend;
    QSqlQuery query;
    query.prepare("SELECT start_date, SUM(value) FROM contracts "
                 "WHERE start_date BETWEEN ? AND ? "
                 "GROUP BY start_date ORDER BY start_date");
    
    query.addBindValue(startDate.toString(Qt::ISODate));
    query.addBindValue(endDate.toString(Qt::ISODate));
    
    if (query.exec()) {
        while (query.next()) {
            QDate date = QDate::fromString(query.value(0).toString(), Qt::ISODate);
            trend.insert(date, query.value(1).toDouble());
        }
    }
    return trend;
}