#include "contractdatabasemanager.h"
#include "contract.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlRecord>
#include <QDateTime>

ContractDatabaseManager::ContractDatabaseManager(QObject *parent)
    : QObject(parent)
    , m_isInitialized(false)
    , m_cachingEnabled(true)
    , m_cacheTimestamp(QDateTime::currentDateTime())
{
}

ContractDatabaseManager::~ContractDatabaseManager()
{
    clearCache();
    shutdown();
}

bool ContractDatabaseManager::initialize(const QString &databasePath)
{
    qDebug() << "ContractDatabaseManager::initialize called with path:" << databasePath;
    
    if (m_isInitialized) {
        qDebug() << "Database already initialized";
        return true;
    }

    // Determine database path
    if (databasePath.isEmpty()) {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(appDataPath);
        m_databasePath = appDataPath + "/archiflow_contracts.db";
    } else {
        m_databasePath = databasePath;
    }

    qDebug() << "Database path determined as:" << m_databasePath;

    // Setup database connection
    m_database = QSqlDatabase::addDatabase("QSQLITE", "contracts_connection");
    m_database.setDatabaseName(m_databasePath);

    qDebug() << "Attempting to open database...";
    if (!m_database.open()) {
        m_lastError = QString("Failed to open database: %1").arg(m_database.lastError().text());
        qWarning() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    qDebug() << "Database opened successfully";

    // Create tables
    qDebug() << "Creating database tables...";
    if (!createTables()) {
        qWarning() << "Failed to create database tables";
        return false;
    }

    m_isInitialized = true;
    qDebug() << "Contract database initialized successfully at:" << m_databasePath;
    return true;
}

void ContractDatabaseManager::shutdown()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
    m_isInitialized = false;
}

bool ContractDatabaseManager::createTables()
{
    QSqlQuery query(m_database);
    
    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS contracts (
            id TEXT PRIMARY KEY,
            client_name TEXT NOT NULL,
            start_date TEXT NOT NULL,
            end_date TEXT NOT NULL,
            value REAL NOT NULL DEFAULT 0.0,
            status TEXT NOT NULL DEFAULT 'Draft',
            description TEXT,
            payment_terms INTEGER DEFAULT 30,
            has_non_compete_clause BOOLEAN DEFAULT FALSE,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    query.prepare(createTableSQL);
    if (!executeQuery(query, "create contracts table")) {
        return false;
    }

    // Create indexes for better performance
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_contracts_client_name ON contracts(client_name)",
        "CREATE INDEX IF NOT EXISTS idx_contracts_status ON contracts(status)",
        "CREATE INDEX IF NOT EXISTS idx_contracts_start_date ON contracts(start_date)",
        "CREATE INDEX IF NOT EXISTS idx_contracts_end_date ON contracts(end_date)"
    };

    for (const QString &indexSQL : indexes) {
        query.prepare(indexSQL);
        if (!executeQuery(query, "create index")) {
            qWarning() << "Failed to create index:" << indexSQL;
        }
    }

    return true;
}

QString ContractDatabaseManager::addContract(Contract *contract)
{
    qDebug() << "ContractDatabaseManager::addContract called";
    
    if (!contract) {
        m_lastError = "Contract object is null";
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return QString();
    }
    
    if (!m_isInitialized || !m_database.isOpen()) {
        m_lastError = "Database not initialized or not connected";
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return QString();
    }
    
    // Validate contract before adding
    QString validationError;
    if (!validateContract(contract, validationError)) {
        m_lastError = QString("Contract validation failed: %1").arg(validationError);
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return QString();
    }
    
    qDebug() << "Adding contract - Client:" << contract->clientName() 
             << "Value:" << contract->value() 
             << "Status:" << contract->status();

    // Generate unique ID if not set
    if (contract->id().isEmpty()) {
        QString newId;
        do {
            newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        } while (!isContractIdUnique(newId));
        contract->setId(newId);
        qDebug() << "Generated new contract ID:" << contract->id();
    } else {
        // Check if existing ID is unique
        if (!isContractIdUnique(contract->id())) {
            m_lastError = QString("Contract ID '%1' already exists").arg(contract->id());
            qDebug() << m_lastError;
            emit databaseError(m_lastError);
            return QString();
        }
        qDebug() << "Using existing contract ID:" << contract->id();
    }

    // Begin transaction for data integrity
    if (!m_database.transaction()) {
        m_lastError = QString("Failed to start transaction: %1").arg(m_database.lastError().text());
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return QString();
    }

    QSqlQuery query(m_database);
    QString sql = R"(
        INSERT INTO contracts (
            id, client_name, start_date, end_date, value, status,
            description, payment_terms, has_non_compete_clause,
            created_at, updated_at
        ) VALUES (
            :id, :client_name, :start_date, :end_date, :value, :status,
            :description, :payment_terms, :has_non_compete_clause,
            CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
        )
    )";

    query.prepare(sql);
    bindContractToQuery(query, contract);

    qDebug() << "Executing contract insert query...";
    if (executeQuery(query, "add contract")) {
        if (m_database.commit()) {
            qDebug() << "Contract added successfully to database with ID:" << contract->id();
            emit contractAdded(contract->id());
            return contract->id();
        } else {
            m_lastError = QString("Failed to commit transaction: %1").arg(m_database.lastError().text());
            qDebug() << m_lastError;
            emit databaseError(m_lastError);
        }
    }

    // Rollback on failure
    m_database.rollback();
    qDebug() << "Failed to add contract to database, transaction rolled back";
    return QString();
}

bool ContractDatabaseManager::updateContract(Contract *contract)
{
    if (!contract) {
        m_lastError = "Contract object is null";
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }
    
    if (!m_isInitialized || !m_database.isOpen()) {
        m_lastError = "Database not initialized or not connected";
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }
    
    if (contract->id().isEmpty()) {
        m_lastError = "Contract ID is empty, cannot update";
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    // Validate contract before updating
    QString validationError;
    if (!validateContract(contract, validationError)) {
        m_lastError = QString("Contract validation failed: %1").arg(validationError);
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    // Check if contract exists
    if (!getContract(contract->id())) {
        m_lastError = QString("Contract with ID '%1' does not exist").arg(contract->id());
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    // Begin transaction for data integrity
    if (!m_database.transaction()) {
        m_lastError = QString("Failed to start transaction: %1").arg(m_database.lastError().text());
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    QSqlQuery query(m_database);
    QString sql = R"(
        UPDATE contracts SET
            client_name = :client_name,
            start_date = :start_date,
            end_date = :end_date,
            value = :value,
            status = :status,
            description = :description,
            payment_terms = :payment_terms,
            has_non_compete_clause = :has_non_compete_clause,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = :id
    )";

    query.prepare(sql);
    bindContractToQuery(query, contract);

    qDebug() << "Updating contract with ID:" << contract->id();
    if (executeQuery(query, "update contract")) {
        if (query.numRowsAffected() > 0) {
            if (m_database.commit()) {
                qDebug() << "Contract updated successfully in database";
                emit contractUpdated(contract->id());
                return true;
            } else {
                m_lastError = QString("Failed to commit transaction: %1").arg(m_database.lastError().text());
                qDebug() << m_lastError;
                emit databaseError(m_lastError);
            }
        } else {
            m_lastError = QString("No contract found with ID '%1' to update").arg(contract->id());
            qDebug() << m_lastError;
            emit databaseError(m_lastError);
        }
    }

    // Rollback on failure
    m_database.rollback();
    qDebug() << "Failed to update contract, transaction rolled back";
    return false;
}

bool ContractDatabaseManager::deleteContract(const QString &contractId)
{
    if (contractId.isEmpty()) {
        m_lastError = "Contract ID is empty";
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }
    
    if (!m_isInitialized || !m_database.isOpen()) {
        m_lastError = "Database not initialized or not connected";
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    // Check if contract exists before attempting to delete
    Contract* existingContract = getContract(contractId);
    if (!existingContract) {
        m_lastError = QString("Contract with ID '%1' does not exist").arg(contractId);
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    // Check business rules for deletion
    if (!canDeleteContract(contractId)) {
        m_lastError = QString("Contract '%1' cannot be deleted (business rules violation)").arg(contractId);
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        delete existingContract;
        return false;
    }

    delete existingContract; // Clean up

    // Begin transaction for data integrity
    if (!m_database.transaction()) {
        m_lastError = QString("Failed to start transaction: %1").arg(m_database.lastError().text());
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM contracts WHERE id = :id");
    query.bindValue(":id", contractId);

    qDebug() << "Deleting contract with ID:" << contractId;
    if (executeQuery(query, "delete contract")) {
        if (query.numRowsAffected() > 0) {
            if (m_database.commit()) {
                qDebug() << "Contract deleted successfully from database";
                emit contractDeleted(contractId);
                return true;
            } else {
                m_lastError = QString("Failed to commit transaction: %1").arg(m_database.lastError().text());
                qDebug() << m_lastError;
                emit databaseError(m_lastError);
            }
        } else {
            m_lastError = QString("No contract found with ID '%1' to delete").arg(contractId);
            qDebug() << m_lastError;
            emit databaseError(m_lastError);
        }
    }

    // Rollback on failure
    m_database.rollback();
    qDebug() << "Failed to delete contract, transaction rolled back";
    return false;
}

Contract* ContractDatabaseManager::getContract(const QString &contractId)
{
    if (contractId.isEmpty()) {
        qDebug() << "Contract ID is empty";
        return nullptr;
    }
    
    if (!m_isInitialized || !m_database.isOpen()) {
        qDebug() << "Database not initialized or not connected";
        return nullptr;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM contracts WHERE id = :id");
    query.bindValue(":id", contractId);

    if (executeQuery(query, "get contract")) {
        if (query.next()) {
            Contract* contract = createContractFromQuery(query);
            qDebug() << "Retrieved contract with ID:" << contractId;
            return contract;
        } else {
            qDebug() << "No contract found with ID:" << contractId;
        }
    } else {
        qDebug() << "Failed to execute query for contract ID:" << contractId;
    }

    return nullptr;
}

QList<Contract*> ContractDatabaseManager::getAllContracts()
{
    qDebug() << "ContractDatabaseManager::getAllContracts called";
    QList<Contract*> contracts;

    if (!m_isInitialized) {
        qDebug() << "Database not initialized in getAllContracts!";
        return contracts;
    }

    QSqlQuery query(m_database);
    QString sql = "SELECT * FROM contracts ORDER BY created_at DESC";

    qDebug() << "Executing getAllContracts query...";
    if (executeQuery(query, sql)) {
        int count = 0;
        while (query.next()) {
            Contract *contract = createContractFromQuery(query);
            if (contract) {
                contracts.append(contract);
                count++;
                qDebug() << "Loaded contract" << count << "- ID:" << contract->id() 
                         << "Client:" << contract->clientName();
            }
        }
        qDebug() << "Successfully loaded" << contracts.size() << "contracts from database";
    } else {
        qDebug() << "Failed to execute getAllContracts query";
    }

    return contracts;
}

QList<Contract*> ContractDatabaseManager::searchContracts(const QString &searchTerm)
{
    QList<Contract*> contracts;

    if (searchTerm.isEmpty() || !m_isInitialized) {
        return getAllContracts();
    }

    QSqlQuery query(m_database);
    QString sql = R"(
        SELECT * FROM contracts 
        WHERE client_name LIKE :term 
           OR description LIKE :term 
           OR status LIKE :term
        ORDER BY created_at DESC
    )";

    query.prepare(sql);
    query.bindValue(":term", "%" + searchTerm + "%");

    if (executeQuery(query, "search contracts")) {
        while (query.next()) {
            Contract *contract = createContractFromQuery(query);
            if (contract) {
                contracts.append(contract);
            }
        }
    }

    return contracts;
}

QList<Contract*> ContractDatabaseManager::getContractsByStatus(const QString &status)
{
    QList<Contract*> contracts;

    if (status.isEmpty() || !m_isInitialized) {
        return contracts;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM contracts WHERE status = :status ORDER BY created_at DESC");
    query.bindValue(":status", status);

    if (executeQuery(query, "get contracts by status")) {
        while (query.next()) {
            Contract *contract = createContractFromQuery(query);
            if (contract) {
                contracts.append(contract);
            }
        }
    }

    return contracts;
}

QList<Contract*> ContractDatabaseManager::getContractsByDateRange(const QDate &startDate, const QDate &endDate)
{
    QList<Contract*> contracts;

    if (!startDate.isValid() || !endDate.isValid() || !m_isInitialized) {
        return contracts;
    }

    QSqlQuery query(m_database);
    QString sql = R"(
        SELECT * FROM contracts 
        WHERE start_date >= :start_date AND end_date <= :end_date
        ORDER BY start_date
    )";

    query.prepare(sql);
    query.bindValue(":start_date", startDate.toString(Qt::ISODate));
    query.bindValue(":end_date", endDate.toString(Qt::ISODate));

    if (executeQuery(query, "get contracts by date range")) {
        while (query.next()) {
            Contract *contract = createContractFromQuery(query);
            if (contract) {
                contracts.append(contract);
            }
        }
    }

    return contracts;
}

QList<Contract*> ContractDatabaseManager::getExpiringContracts(int daysFromNow)
{
    QList<Contract*> contracts;

    if (!m_isInitialized) {
        return contracts;
    }

    QDate currentDate = QDate::currentDate();
    QDate futureDate = currentDate.addDays(daysFromNow);

    QSqlQuery query(m_database);
    QString sql = R"(
        SELECT * FROM contracts 
        WHERE end_date >= :current_date AND end_date <= :future_date
        AND status = 'Active'
        ORDER BY end_date
    )";

    query.prepare(sql);
    query.bindValue(":current_date", currentDate.toString(Qt::ISODate));
    query.bindValue(":future_date", futureDate.toString(Qt::ISODate));

    if (executeQuery(query, "get expiring contracts")) {
        while (query.next()) {
            Contract *contract = createContractFromQuery(query);
            if (contract) {
                contracts.append(contract);
            }
        }
    }

    return contracts;
}

int ContractDatabaseManager::getTotalContracts()
{
    if (!m_isInitialized) return 0;

    QSqlQuery query(m_database);
    if (executeQuery(query, "SELECT COUNT(*) FROM contracts") && query.next()) {
        return query.value(0).toInt();
    }    return 0;
}

int ContractDatabaseManager::getExpiredContracts()
{
    if (!m_isInitialized) return 0;

    QSqlQuery query(m_database);
    QString sql = "SELECT COUNT(*) FROM contracts WHERE end_date < :current_date";
    query.prepare(sql);
    query.bindValue(":current_date", QDate::currentDate().toString(Qt::ISODate));    if (executeQuery(query, "get expired contracts count") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

double ContractDatabaseManager::getTotalContractValue()
{
    if (!m_isInitialized) return 0.0;

    QSqlQuery query(m_database);
    if (executeQuery(query, "SELECT SUM(value) FROM contracts") && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double ContractDatabaseManager::getActiveContractValue()
{
    if (!m_isInitialized || !m_database.isOpen()) {
        return 0.0;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT SUM(value) FROM contracts WHERE status = ?");
    query.addBindValue("Active");

    if (executeQuery(query, "get active contract value") && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

bool ContractDatabaseManager::isDatabaseConnected() const
{
    return m_database.isOpen() && m_isInitialized;
}

QString ContractDatabaseManager::getLastError() const
{
    return m_lastError;
}

void ContractDatabaseManager::onDatabaseError(const QString &error)
{
    m_lastError = error;
    qWarning() << "Contract Database Error:" << error;
}

Contract* ContractDatabaseManager::createContractFromQuery(const QSqlQuery &query)
{
    Contract *contract = new Contract(this);
    
    contract->setId(query.value("id").toString());
    contract->setClientName(query.value("client_name").toString());
    contract->setStartDate(QDate::fromString(query.value("start_date").toString(), Qt::ISODate));
    contract->setEndDate(QDate::fromString(query.value("end_date").toString(), Qt::ISODate));
    contract->setValue(query.value("value").toDouble());
    contract->setStatus(query.value("status").toString());
    contract->setDescription(query.value("description").toString());
    contract->setPaymentTerms(query.value("payment_terms").toInt());
    contract->setHasNonCompeteClause(query.value("has_non_compete_clause").toBool());

    return contract;
}

void ContractDatabaseManager::bindContractToQuery(QSqlQuery &query, Contract *contract)
{
    query.bindValue(":id", contract->id());
    query.bindValue(":client_name", contract->clientName());
    query.bindValue(":start_date", contract->startDate().toString(Qt::ISODate));
    query.bindValue(":end_date", contract->endDate().toString(Qt::ISODate));
    query.bindValue(":value", contract->value());
    query.bindValue(":status", contract->status());
    query.bindValue(":description", contract->description());
    query.bindValue(":payment_terms", contract->paymentTerms());
    query.bindValue(":has_non_compete_clause", contract->hasNonCompeteClause());
}

bool ContractDatabaseManager::executeQuery(QSqlQuery &query, const QString &operation)
{
    if (!query.exec()) {
        m_lastError = QString("Failed to execute %1: %2").arg(operation, query.lastError().text());
        qWarning() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }
    return true;
}

// IContractService interface implementation

QList<Contract*> ContractDatabaseManager::getContractsByClient(const QString &clientName)
{
    QList<Contract*> contracts;
    
    if (!m_isInitialized || !m_database.isOpen()) {
        return contracts;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM contracts WHERE client_name LIKE ?");
    query.addBindValue(QString("%%1%").arg(clientName));

    if (executeQuery(query, "get contracts by client")) {
        while (query.next()) {
            Contract *contract = createContractFromQuery(query);
            if (contract) {
                contracts.append(contract);
            }
        }
    }

    return contracts;
}

QList<Contract*> ContractDatabaseManager::getActiveContracts()
{
    return getContractsByStatus("Active");
}

QJsonObject ContractDatabaseManager::getContractStatistics()
{
    QJsonObject stats;
    
    if (!m_isInitialized || !m_database.isOpen()) {
        return stats;
    }

    // Total contracts
    stats["totalContracts"] = getTotalContracts();
    
    // Active contracts
    stats["activeContracts"] = getActiveContractsCount();
    
    // Expired contracts
    stats["expiredContracts"] = getExpiredContracts();
    
    // Total value
    stats["totalValue"] = getTotalContractValue();
    
    // Average value
    stats["averageValue"] = getAverageContractValue();
    
    // Contracts by status
    QJsonObject statusCounts;
    QStringList statuses = getValidStatuses();
    for (const QString &status : statuses) {
        statusCounts[status] = getContractsByStatusCount(status);
    }
    stats["contractsByStatus"] = statusCounts;
    
    // Expiring soon (30 days)
    stats["expiringSoon"] = getExpiringContracts(30).count();

    return stats;
}

QJsonArray ContractDatabaseManager::getStatusDistribution()
{
    QJsonArray distribution;
    
    if (!m_isInitialized || !m_database.isOpen()) {
        return distribution;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT status, COUNT(*) as count FROM contracts GROUP BY status");

    if (executeQuery(query, "get status distribution")) {
        while (query.next()) {
            QJsonObject statusInfo;
            statusInfo["status"] = query.value("status").toString();
            statusInfo["count"] = query.value("count").toInt();
            distribution.append(statusInfo);
        }
    }

    return distribution;
}

QJsonArray ContractDatabaseManager::getMonthlyContractCounts()
{
    QJsonArray monthlyCounts;
    
    if (!m_isInitialized || !m_database.isOpen()) {
        return monthlyCounts;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT 
            strftime('%Y-%m', start_date) as month,
            COUNT(*) as count
        FROM contracts 
        GROUP BY strftime('%Y-%m', start_date)
        ORDER BY month DESC
        LIMIT 12
    )");

    if (executeQuery(query, "get monthly contract counts")) {
        while (query.next()) {
            QJsonObject monthData;
            monthData["month"] = query.value("month").toString();
            monthData["count"] = query.value("count").toInt();
            monthlyCounts.append(monthData);
        }
    }

    return monthlyCounts;
}

double ContractDatabaseManager::getAverageContractValue()
{
    if (!m_isInitialized || !m_database.isOpen()) {
        return 0.0;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT AVG(value) FROM contracts");

    if (executeQuery(query, "get average contract value") && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

int ContractDatabaseManager::getContractCount()
{
    return getTotalContracts();
}

bool ContractDatabaseManager::validateContract(const Contract *contract, QString &errorMessage)
{
    if (!contract) {
        errorMessage = "Contract is null";
        return false;
    }

    if (contract->clientName().trimmed().isEmpty()) {
        errorMessage = "Client name cannot be empty";
        return false;
    }

    if (!contract->startDate().isValid()) {
        errorMessage = "Start date is invalid";
        return false;
    }

    if (!contract->endDate().isValid()) {
        errorMessage = "End date is invalid";
        return false;
    }

    if (contract->endDate() <= contract->startDate()) {
        errorMessage = "End date must be after start date";
        return false;
    }

    if (contract->value() < 0) {
        errorMessage = "Contract value cannot be negative";
        return false;
    }

    if (!getValidStatuses().contains(contract->status())) {
        errorMessage = "Invalid contract status";
        return false;
    }

    if (contract->paymentTerms() < 0) {
        errorMessage = "Payment terms cannot be negative";
        return false;
    }

    return true;
}

bool ContractDatabaseManager::isContractIdUnique(const QString &contractId)
{
    if (!m_isInitialized || !m_database.isOpen()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM contracts WHERE id = ?");
    query.addBindValue(contractId);

    if (executeQuery(query, "check contract ID uniqueness") && query.next()) {
        return query.value(0).toInt() == 0;
    }

    return false;
}

QStringList ContractDatabaseManager::getValidStatuses()
{
    return {"Draft", "Active", "Completed", "Cancelled", "Expired"};
}

bool ContractDatabaseManager::canDeleteContract(const QString &contractId)
{
    // Business rule: Can delete if contract is not active
    Contract *contract = getContract(contractId);
    if (contract) {
        bool canDelete = contract->status() != "Active";
        delete contract;
        return canDelete;
    }
    return false;
}

bool ContractDatabaseManager::isContractExpired(const QString &contractId)
{
    Contract *contract = getContract(contractId);
    if (contract) {
        bool expired = contract->endDate() < QDate::currentDate();
        delete contract;
        return expired;
    }
    return false;
}

int ContractDatabaseManager::getDaysUntilExpiry(const QString &contractId)
{
    Contract *contract = getContract(contractId);
    if (contract) {
        int days = QDate::currentDate().daysTo(contract->endDate());
        delete contract;
        return days;
    }
    return -1;
}

QJsonArray ContractDatabaseManager::exportContracts()
{
    QJsonArray contractsArray;
    
    QList<Contract*> contracts = getAllContracts();
    for (Contract *contract : contracts) {
        QJsonObject contractJson;
        contractJson["id"] = contract->id();
        contractJson["clientName"] = contract->clientName();
        contractJson["startDate"] = contract->startDate().toString(Qt::ISODate);
        contractJson["endDate"] = contract->endDate().toString(Qt::ISODate);
        contractJson["value"] = contract->value();
        contractJson["status"] = contract->status();
        contractJson["description"] = contract->description();
        contractJson["paymentTerms"] = contract->paymentTerms();
        contractJson["hasNonCompeteClause"] = contract->hasNonCompeteClause();
        
        contractsArray.append(contractJson);
        delete contract;
    }
    
    return contractsArray;
}

bool ContractDatabaseManager::importContracts(const QJsonArray &contracts, QString &errorMessage)
{
    if (!m_isInitialized || !m_database.isOpen()) {
        errorMessage = "Database not initialized";
        return false;
    }

    bool overallSuccess = true;
    int successCount = 0;
    int errorCount = 0;

    for (const QJsonValue &value : contracts) {
        if (!value.isObject()) {
            errorCount++;
            continue;
        }

        QJsonObject contractObj = value.toObject();
        
        Contract *contract = new Contract(this);
        contract->setId(contractObj["id"].toString());
        contract->setClientName(contractObj["clientName"].toString());
        contract->setStartDate(QDate::fromString(contractObj["startDate"].toString(), Qt::ISODate));
        contract->setEndDate(QDate::fromString(contractObj["endDate"].toString(), Qt::ISODate));
        contract->setValue(contractObj["value"].toDouble());
        contract->setStatus(contractObj["status"].toString());
        contract->setDescription(contractObj["description"].toString());
        contract->setPaymentTerms(contractObj["paymentTerms"].toInt());
        contract->setHasNonCompeteClause(contractObj["hasNonCompeteClause"].toBool());

        QString validationError;
        if (!validateContract(contract, validationError)) {
            errorMessage += QString("Contract %1: %2\n").arg(contract->id(), validationError);
            errorCount++;
            delete contract;
            continue;
        }

        QString contractId = addContract(contract);
        if (contractId.isEmpty()) {
            errorMessage += QString("Failed to add contract %1\n").arg(contract->id());
            errorCount++;
            overallSuccess = false;
        } else {
            successCount++;
        }
        
        delete contract;
    }

    if (errorCount > 0) {
        errorMessage = QString("Import completed with %1 successes and %2 errors:\n%3")
                      .arg(successCount).arg(errorCount).arg(errorMessage);
        overallSuccess = false;
    }

    return overallSuccess;
}

// Legacy methods for backward compatibility

int ContractDatabaseManager::getActiveContractsCount()
{
    return getContractsByStatus("Active").count();
}

int ContractDatabaseManager::getContractsByStatusCount(const QString &status)
{
    return getContractsByStatus(status).count();
}

QList<Contract*> ContractDatabaseManager::getContractsInDateRange(const QDate &startDate, const QDate &endDate)
{
    return getContractsByDateRange(startDate, endDate);
}

// Enhanced CRUD operations with better error handling

bool ContractDatabaseManager::addContracts(const QList<Contract*> &contracts, QStringList &addedIds, QString &errorMessage)
{
    if (!m_isInitialized || !m_database.isOpen()) {
        errorMessage = "Database not initialized or not connected";
        return false;
    }

    if (contracts.isEmpty()) {
        errorMessage = "No contracts provided to add";
        return false;
    }

    // Begin transaction for batch operation
    if (!m_database.transaction()) {
        errorMessage = QString("Failed to start transaction: %1").arg(m_database.lastError().text());
        return false;
    }

    bool overallSuccess = true;
    int successCount = 0;
    int errorCount = 0;
    QStringList errors;

    for (Contract* contract : contracts) {
        if (!contract) {
            errors << "Null contract object encountered";
            errorCount++;
            continue;
        }

        QString validationError;
        if (!validateContract(contract, validationError)) {
            errors << QString("Contract '%1': %2").arg(contract->clientName(), validationError);
            errorCount++;
            continue;
        }

        // Generate unique ID if needed
        if (contract->id().isEmpty()) {
            QString newId;
            do {
                newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
            } while (!isContractIdUnique(newId));
            contract->setId(newId);
        } else if (!isContractIdUnique(contract->id())) {
            errors << QString("Contract ID '%1' already exists").arg(contract->id());
            errorCount++;
            continue;
        }

        QSqlQuery query(m_database);
        QString sql = R"(
            INSERT INTO contracts (
                id, client_name, start_date, end_date, value, status,
                description, payment_terms, has_non_compete_clause,
                created_at, updated_at
            ) VALUES (
                :id, :client_name, :start_date, :end_date, :value, :status,
                :description, :payment_terms, :has_non_compete_clause,
                CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
            )
        )";

        query.prepare(sql);
        bindContractToQuery(query, contract);

        if (executeQuery(query, "batch add contract")) {
            addedIds << contract->id();
            successCount++;
            
            // Clear cache entry if caching is enabled
            if (m_cachingEnabled) {
                m_contractCache.remove(contract->id());
            }
        } else {
            errors << QString("Failed to add contract '%1': %2").arg(contract->clientName(), query.lastError().text());
            errorCount++;
            overallSuccess = false;
        }
    }

    if (overallSuccess && errorCount == 0) {
        if (m_database.commit()) {
            // Emit signals for successfully added contracts
            for (const QString &contractId : addedIds) {
                emit contractAdded(contractId);
            }
            qDebug() << "Batch add completed successfully:" << successCount << "contracts added";
            return true;
        } else {
            errorMessage = QString("Failed to commit transaction: %1").arg(m_database.lastError().text());
            overallSuccess = false;
        }
    }

    // Rollback on any failure
    m_database.rollback();
    errorMessage = QString("Batch add completed with %1 successes and %2 errors:\n%3")
                  .arg(successCount).arg(errorCount).arg(errors.join("\n"));
    
    return false;
}

bool ContractDatabaseManager::updateContracts(const QList<Contract*> &contracts, QString &errorMessage)
{
    if (!m_isInitialized || !m_database.isOpen()) {
        errorMessage = "Database not initialized or not connected";
        return false;
    }

    if (contracts.isEmpty()) {
        errorMessage = "No contracts provided to update";
        return false;
    }

    // Begin transaction for batch operation
    if (!m_database.transaction()) {
        errorMessage = QString("Failed to start transaction: %1").arg(m_database.lastError().text());
        return false;
    }

    bool overallSuccess = true;
    int successCount = 0;
    int errorCount = 0;
    QStringList errors;
    QStringList updatedIds;

    for (Contract* contract : contracts) {
        if (!contract || contract->id().isEmpty()) {
            errors << "Invalid contract object or empty ID";
            errorCount++;
            continue;
        }

        QString validationError;
        if (!validateContract(contract, validationError)) {
            errors << QString("Contract '%1': %2").arg(contract->id(), validationError);
            errorCount++;
            continue;
        }

        QSqlQuery query(m_database);
        QString sql = R"(
            UPDATE contracts SET
                client_name = :client_name,
                start_date = :start_date,
                end_date = :end_date,
                value = :value,
                status = :status,
                description = :description,
                payment_terms = :payment_terms,
                has_non_compete_clause = :has_non_compete_clause,
                updated_at = CURRENT_TIMESTAMP
            WHERE id = :id
        )";

        query.prepare(sql);
        bindContractToQuery(query, contract);

        if (executeQuery(query, "batch update contract")) {
            if (query.numRowsAffected() > 0) {
                updatedIds << contract->id();
                successCount++;
                
                // Clear cache entry if caching is enabled
                if (m_cachingEnabled) {
                    m_contractCache.remove(contract->id());
                }
            } else {
                errors << QString("No contract found with ID '%1' to update").arg(contract->id());
                errorCount++;
                overallSuccess = false;
            }
        } else {
            errors << QString("Failed to update contract '%1': %2").arg(contract->id(), query.lastError().text());
            errorCount++;
            overallSuccess = false;
        }
    }

    if (overallSuccess && errorCount == 0) {
        if (m_database.commit()) {
            // Emit signals for successfully updated contracts
            for (const QString &contractId : updatedIds) {
                emit contractUpdated(contractId);
            }
            qDebug() << "Batch update completed successfully:" << successCount << "contracts updated";
            return true;
        } else {
            errorMessage = QString("Failed to commit transaction: %1").arg(m_database.lastError().text());
            overallSuccess = false;
        }
    }

    // Rollback on any failure
    m_database.rollback();
    errorMessage = QString("Batch update completed with %1 successes and %2 errors:\n%3")
                  .arg(successCount).arg(errorCount).arg(errors.join("\n"));
    
    return false;
}

bool ContractDatabaseManager::deleteContracts(const QStringList &contractIds, QString &errorMessage)
{
    if (!m_isInitialized || !m_database.isOpen()) {
        errorMessage = "Database not initialized or not connected";
        return false;
    }

    if (contractIds.isEmpty()) {
        errorMessage = "No contract IDs provided to delete";
        return false;
    }

    // Begin transaction for batch operation
    if (!m_database.transaction()) {
        errorMessage = QString("Failed to start transaction: %1").arg(m_database.lastError().text());
        return false;
    }

    bool overallSuccess = true;
    int successCount = 0;
    int errorCount = 0;
    QStringList errors;
    QStringList deletedIds;

    for (const QString &contractId : contractIds) {
        if (contractId.isEmpty()) {
            errors << "Empty contract ID";
            errorCount++;
            continue;
        }

        // Check if contract exists and can be deleted
        if (!getContract(contractId)) {
            errors << QString("Contract with ID '%1' does not exist").arg(contractId);
            errorCount++;
            continue;
        }

        if (!canDeleteContract(contractId)) {
            errors << QString("Contract '%1' cannot be deleted (business rules violation)").arg(contractId);
            errorCount++;
            continue;
        }

        QSqlQuery query(m_database);
        query.prepare("DELETE FROM contracts WHERE id = :id");
        query.bindValue(":id", contractId);

        if (executeQuery(query, "batch delete contract")) {
            if (query.numRowsAffected() > 0) {
                deletedIds << contractId;
                successCount++;
                
                // Clear cache entry if caching is enabled
                if (m_cachingEnabled) {
                    m_contractCache.remove(contractId);
                }
            } else {
                errors << QString("No contract found with ID '%1' to delete").arg(contractId);
                errorCount++;
                overallSuccess = false;
            }
        } else {
            errors << QString("Failed to delete contract '%1': %2").arg(contractId, query.lastError().text());
            errorCount++;
            overallSuccess = false;
        }
    }

    if (overallSuccess && errorCount == 0) {
        if (m_database.commit()) {
            // Emit signals for successfully deleted contracts
            for (const QString &contractId : deletedIds) {
                emit contractDeleted(contractId);
            }
            qDebug() << "Batch delete completed successfully:" << successCount << "contracts deleted";
            return true;
        } else {
            errorMessage = QString("Failed to commit transaction: %1").arg(m_database.lastError().text());
            overallSuccess = false;
        }
    }

    // Rollback on any failure
    m_database.rollback();
    errorMessage = QString("Batch delete completed with %1 successes and %2 errors:\n%3")
                  .arg(successCount).arg(errorCount).arg(errors.join("\n"));
    
    return false;
}

QList<Contract*> ContractDatabaseManager::getContracts(const QStringList &contractIds)
{
    QList<Contract*> contracts;
    
    if (!m_isInitialized || !m_database.isOpen()) {
        qDebug() << "Database not initialized or not connected";
        return contracts;
    }

    if (contractIds.isEmpty()) {
        qDebug() << "No contract IDs provided";
        return contracts;
    }    QStringList queryIds;

    // Check cache first if caching is enabled
    if (m_cachingEnabled) {
        QStringList uncachedIds;
        for (const QString &contractId : contractIds) {
            if (m_contractCache.contains(contractId)) {
                Contract* cachedContract = new Contract(*m_contractCache[contractId], this);
                contracts.append(cachedContract);
            } else {
                uncachedIds.append(contractId);
            }
        }
        
        // If all contracts were found in cache, return
        if (uncachedIds.isEmpty()) {
            return contracts;
        }
        
        // Only query for uncached contracts
        queryIds = uncachedIds;
    } else {
        queryIds = contractIds;
    }// Build query for multiple IDs
    QStringList placeholders;
    for (int i = 0; i < queryIds.size(); ++i) {
        placeholders.append("?");
    }
    QString sql = QString("SELECT * FROM contracts WHERE id IN (%1) ORDER BY created_at DESC").arg(placeholders.join(","));
    
    QSqlQuery query(m_database);
    query.prepare(sql);
    
    for (const QString &contractId : queryIds) {
        query.addBindValue(contractId);
    }

    if (executeQuery(query, "get multiple contracts")) {
        while (query.next()) {
            Contract* contract = createContractFromQuery(query);
            if (contract) {
                contracts.append(contract);
                  // Cache the contract if caching is enabled
                if (m_cachingEnabled) {
                    m_contractCache[contract->id()] = new Contract(*contract, this);
                }
            }
        }
    }

    qDebug() << "Retrieved" << contracts.size() << "contracts out of" << queryIds.size() << "requested";
    return contracts;
}

// Database synchronization and maintenance

bool ContractDatabaseManager::synchronizeDatabase()
{
    if (!m_isInitialized || !m_database.isOpen()) {
        m_lastError = "Database not initialized or not connected";
        return false;
    }

    qDebug() << "Starting database synchronization...";

    // Begin transaction
    if (!m_database.transaction()) {
        m_lastError = QString("Failed to start synchronization transaction: %1").arg(m_database.lastError().text());
        return false;
    }

    try {
        // Update expired contracts
        QSqlQuery updateExpiredQuery(m_database);
        updateExpiredQuery.prepare(R"(
            UPDATE contracts 
            SET status = 'Expired', updated_at = CURRENT_TIMESTAMP 
            WHERE status = 'Active' AND end_date < :current_date
        )");
        updateExpiredQuery.bindValue(":current_date", QDate::currentDate().toString(Qt::ISODate));

        if (!executeQuery(updateExpiredQuery, "update expired contracts")) {
            throw std::runtime_error("Failed to update expired contracts");
        }

        // Clean up orphaned records (if any)
        QSqlQuery cleanupQuery(m_database);
        cleanupQuery.prepare("DELETE FROM contracts WHERE id IS NULL OR client_name IS NULL OR client_name = ''");
        
        if (!executeQuery(cleanupQuery, "cleanup orphaned records")) {
            throw std::runtime_error("Failed to cleanup orphaned records");
        }

        // Analyze database for performance
        QSqlQuery analyzeQuery(m_database);
        analyzeQuery.prepare("ANALYZE contracts");
        
        if (!executeQuery(analyzeQuery, "analyze database")) {
            qWarning() << "Failed to analyze database, but continuing...";
        }

        // Commit transaction
        if (!m_database.commit()) {
            throw std::runtime_error(QString("Failed to commit synchronization: %1").arg(m_database.lastError().text()).toStdString());
        }

        // Clear cache to ensure fresh data
        clearCache();

        qDebug() << "Database synchronization completed successfully";
        return true;

    } catch (const std::exception &e) {
        m_database.rollback();
        m_lastError = QString("Database synchronization failed: %1").arg(e.what());
        qDebug() << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }
}

bool ContractDatabaseManager::optimizeDatabase()
{
    if (!m_isInitialized || !m_database.isOpen()) {
        m_lastError = "Database not initialized or not connected";
        return false;
    }

    qDebug() << "Starting database optimization...";

    QSqlQuery query(m_database);
    
    // Vacuum database to reclaim space
    if (!executeQuery(query, "VACUUM")) {
        qWarning() << "Failed to vacuum database";
    }

    // Reindex for better performance
    if (!executeQuery(query, "REINDEX")) {
        qWarning() << "Failed to reindex database";
    }

    qDebug() << "Database optimization completed";
    return true;
}

bool ContractDatabaseManager::backupDatabase(const QString &backupPath)
{
    if (!m_isInitialized || !m_database.isOpen()) {
        m_lastError = "Database not initialized or not connected";
        return false;
    }

    if (backupPath.isEmpty()) {
        m_lastError = "Backup path is empty";
        return false;
    }

    qDebug() << "Creating database backup at:" << backupPath;

    // Close current connection temporarily
    QString originalPath = m_databasePath;
    m_database.close();

    // Copy database file
    if (QFile::copy(originalPath, backupPath)) {
        qDebug() << "Database backup created successfully";
        
        // Reopen database
        m_database.open();
        return true;
    } else {
        m_lastError = QString("Failed to create backup at: %1").arg(backupPath);
        qDebug() << m_lastError;
        
        // Reopen database
        m_database.open();
        return false;
    }
}

bool ContractDatabaseManager::restoreDatabase(const QString &backupPath)
{
    if (backupPath.isEmpty() || !QFile::exists(backupPath)) {
        m_lastError = QString("Backup file does not exist: %1").arg(backupPath);
        return false;
    }

    qDebug() << "Restoring database from:" << backupPath;

    // Close current connection
    shutdown();

    // Replace current database with backup
    if (QFile::remove(m_databasePath) && QFile::copy(backupPath, m_databasePath)) {
        // Reinitialize database
        if (initialize(m_databasePath)) {
            qDebug() << "Database restored successfully";
            return true;
        } else {
            m_lastError = "Failed to reinitialize database after restore";
        }
    } else {
        m_lastError = QString("Failed to restore database from: %1").arg(backupPath);
    }

    qDebug() << m_lastError;
    return false;
}

void ContractDatabaseManager::clearCache()
{
    if (m_cachingEnabled) {
        qDeleteAll(m_contractCache.values());
        m_contractCache.clear();
        m_cacheTimestamp = QDateTime::currentDateTime();
        qDebug() << "Contract cache cleared";
    }
}

// Performance and caching

void ContractDatabaseManager::enableCaching(bool enable)
{
    if (m_cachingEnabled != enable) {
        m_cachingEnabled = enable;
        if (!enable) {
            clearCache();
        }
        qDebug() << "Contract caching" << (enable ? "enabled" : "disabled");
    }
}

bool ContractDatabaseManager::isCachingEnabled() const
{
    return m_cachingEnabled;
}

void ContractDatabaseManager::refreshCache()
{
    if (m_cachingEnabled) {
        clearCache();
        // Optionally preload frequently accessed contracts
        qDebug() << "Contract cache refreshed";
    }
}

int ContractDatabaseManager::getCacheSize() const
{
    return m_cachingEnabled ? m_contractCache.size() : 0;
}
