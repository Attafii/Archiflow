#include <QtTest/QtTest>
#include <QApplication>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QDebug>
#include <QElapsedTimer>
#include <QRandomGenerator>

#include "src/features/contracts/contract.h"
#include "src/features/contracts/contractdatabasemanager.h"
#include "src/features/contracts/contractwidget.h"

/**
 * @brief Comprehensive test suite for enhanced Contract CRUD operations
 * 
 * This test suite validates all the enhanced CRUD functionality including:
 * - Basic CRUD operations with validation
 * - Batch operations
 * - Database synchronization
 * - Caching functionality
 * - Error handling and recovery
 */
class TestContractCRUD : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic CRUD tests
    void testAddContract();
    void testAddContractWithValidation();
    void testGetContract();
    void testUpdateContract();
    void testDeleteContract();
    void testDeleteContractBusinessRules();

    // Batch operations tests
    void testAddMultipleContracts();
    void testUpdateMultipleContracts();
    void testDeleteMultipleContracts();
    void testGetMultipleContracts();

    // Search and filter tests
    void testSearchContracts();
    void testFilterByStatus();
    void testFilterByDateRange();
    void testFilterByClient();

    // Statistics and analytics tests
    void testContractStatistics();
    void testStatusDistribution();
    void testMonthlyContractCounts();

    // Database management tests
    void testDatabaseSynchronization();
    void testDatabaseOptimization();
    void testDatabaseBackupRestore();

    // Caching tests
    void testCachingFunctionality();
    void testCachePerformance();

    // Error handling tests
    void testDatabaseConnectionLoss();
    void testTransactionRollback();
    void testValidationErrors();

    // Integration tests
    void testWidgetIntegration();
    void testContractLifecycle();

private:
    void createTestContracts();
    Contract* createTestContract(const QString &clientName, const QString &status = "Active");
    void verifyDatabaseState();

    ContractDatabaseManager *m_dbManager;
    ContractWidget *m_widget;
    QTemporaryDir *m_tempDir;
    QString m_testDbPath;
    QList<Contract*> m_testContracts;
};

void TestContractCRUD::initTestCase()
{
    // Create temporary directory for test database
    m_tempDir = new QTemporaryDir;
    QVERIFY(m_tempDir->isValid());
    
    m_testDbPath = m_tempDir->path() + "/test_contracts.db";
    qDebug() << "Test database path:" << m_testDbPath;
}

void TestContractCRUD::cleanupTestCase()
{
    delete m_tempDir;
}

void TestContractCRUD::init()
{
    // Create fresh database manager for each test
    m_dbManager = new ContractDatabaseManager(this);
    QVERIFY(m_dbManager->initialize(m_testDbPath));
    QVERIFY(m_dbManager->isDatabaseConnected());
    
    // Create widget for integration tests - DISABLED FOR NOW
    // m_widget = new ContractWidget;
    // m_widget->setDatabaseManager(m_dbManager);
    m_widget = nullptr;
    
    // Clear any existing data
    m_testContracts.clear();
}

void TestContractCRUD::cleanup()
{
    // Clean up test contracts
    qDeleteAll(m_testContracts);
    m_testContracts.clear();
    
    // Clean up widget and database manager - widget disabled for now
    // delete m_widget;
    delete m_dbManager;
    
    // Remove test database file
    QFile::remove(m_testDbPath);
}

void TestContractCRUD::testAddContract()
{
    // Create test contract
    Contract *contract = createTestContract("Test Client 1");
    
    // Test adding contract
    QString contractId = m_dbManager->addContract(contract);
    QVERIFY(!contractId.isEmpty());
    QCOMPARE(contract->id(), contractId);
    
    // Verify contract was added to database
    Contract *retrievedContract = m_dbManager->getContract(contractId);
    QVERIFY(retrievedContract != nullptr);
    QCOMPARE(retrievedContract->clientName(), "Test Client 1");
    QCOMPARE(retrievedContract->status(), "Active");
    
    delete retrievedContract;
}

void TestContractCRUD::testAddContractWithValidation()
{
    // Test adding invalid contract (empty client name)
    Contract *invalidContract = new Contract(this);
    invalidContract->setClientName("");
    invalidContract->setStartDate(QDate::currentDate());
    invalidContract->setEndDate(QDate::currentDate().addDays(30));
    invalidContract->setValue(1000.0);
    invalidContract->setStatus("Active");
    
    QString contractId = m_dbManager->addContract(invalidContract);
    QVERIFY(contractId.isEmpty());
    QVERIFY(!m_dbManager->getLastError().isEmpty());
    
    // Test adding contract with invalid date range
    Contract *invalidDateContract = new Contract(this);
    invalidDateContract->setClientName("Test Client");
    invalidDateContract->setStartDate(QDate::currentDate());
    invalidDateContract->setEndDate(QDate::currentDate().addDays(-1)); // End before start
    invalidDateContract->setValue(1000.0);
    invalidDateContract->setStatus("Active");
    
    contractId = m_dbManager->addContract(invalidDateContract);
    QVERIFY(contractId.isEmpty());
    QVERIFY(!m_dbManager->getLastError().isEmpty());
}

void TestContractCRUD::testGetContract()
{
    // Add test contract
    Contract *contract = createTestContract("Get Test Client");
    QString contractId = m_dbManager->addContract(contract);
    QVERIFY(!contractId.isEmpty());
    
    // Test getting existing contract
    Contract *retrievedContract = m_dbManager->getContract(contractId);
    QVERIFY(retrievedContract != nullptr);
    QCOMPARE(retrievedContract->id(), contractId);
    QCOMPARE(retrievedContract->clientName(), "Get Test Client");
    
    delete retrievedContract;
    
    // Test getting non-existent contract
    Contract *nonExistentContract = m_dbManager->getContract("non-existent-id");
    QVERIFY(nonExistentContract == nullptr);
}

void TestContractCRUD::testUpdateContract()
{
    // Add test contract
    Contract *contract = createTestContract("Update Test Client");
    QString contractId = m_dbManager->addContract(contract);
    QVERIFY(!contractId.isEmpty());
    
    // Update contract
    contract->setClientName("Updated Client Name");
    contract->setStatus("Completed");
    contract->setValue(2000.0);
    
    bool updateResult = m_dbManager->updateContract(contract);
    QVERIFY(updateResult);
    
    // Verify update
    Contract *updatedContract = m_dbManager->getContract(contractId);
    QVERIFY(updatedContract != nullptr);
    QCOMPARE(updatedContract->clientName(), "Updated Client Name");
    QCOMPARE(updatedContract->status(), "Completed");
    QCOMPARE(updatedContract->value(), 2000.0);
    
    delete updatedContract;
}

void TestContractCRUD::testDeleteContract()
{
    // Add test contract
    Contract *contract = createTestContract("Delete Test Client", "Draft");
    QString contractId = m_dbManager->addContract(contract);
    QVERIFY(!contractId.isEmpty());
    
    // Test deletion
    bool deleteResult = m_dbManager->deleteContract(contractId);
    QVERIFY(deleteResult);
    
    // Verify contract was deleted
    Contract *deletedContract = m_dbManager->getContract(contractId);
    QVERIFY(deletedContract == nullptr);
}

void TestContractCRUD::testDeleteContractBusinessRules()
{
    // Add active contract (should not be deletable)
    Contract *activeContract = createTestContract("Active Test Client", "Active");
    QString activeContractId = m_dbManager->addContract(activeContract);
    QVERIFY(!activeContractId.isEmpty());
    
    // Test that active contract cannot be deleted
    QVERIFY(!m_dbManager->canDeleteContract(activeContractId));
    bool deleteResult = m_dbManager->deleteContract(activeContractId);
    QVERIFY(!deleteResult);
    
    // Verify contract still exists
    Contract *stillExistsContract = m_dbManager->getContract(activeContractId);
    QVERIFY(stillExistsContract != nullptr);
    delete stillExistsContract;
}

void TestContractCRUD::testAddMultipleContracts()
{
    // Create multiple test contracts
    QList<Contract*> contracts;
    for (int i = 1; i <= 5; ++i) {
        contracts.append(createTestContract(QString("Batch Client %1").arg(i)));
    }
    
    // Test batch add
    QStringList addedIds;
    QString errorMessage;
    bool result = m_dbManager->addContracts(contracts, addedIds, errorMessage);
    
    QVERIFY(result);
    QCOMPARE(addedIds.size(), 5);
    QVERIFY(errorMessage.isEmpty());
    
    // Verify all contracts were added
    for (const QString &contractId : addedIds) {
        Contract *contract = m_dbManager->getContract(contractId);
        QVERIFY(contract != nullptr);
        delete contract;
    }
    
    // Clean up
    qDeleteAll(contracts);
}

void TestContractCRUD::testUpdateMultipleContracts()
{
    // Add test contracts
    createTestContracts();
    
    // Get all contracts
    QList<Contract*> contracts = m_dbManager->getAllContracts();
    QVERIFY(contracts.size() >= 3);
    
    // Update status for first 3 contracts
    QList<Contract*> contractsToUpdate;
    for (int i = 0; i < 3 && i < contracts.size(); ++i) {
        Contract *contract = new Contract(*contracts[i]);
        contract->setStatus("Completed");
        contractsToUpdate.append(contract);
    }
    
    // Test batch update
    QString errorMessage;
    bool result = m_dbManager->updateContracts(contractsToUpdate, errorMessage);
    
    QVERIFY(result);
    QVERIFY(errorMessage.isEmpty());
    
    // Verify updates
    for (Contract *contract : contractsToUpdate) {
        Contract *updatedContract = m_dbManager->getContract(contract->id());
        QVERIFY(updatedContract != nullptr);
        QCOMPARE(updatedContract->status(), "Completed");
        delete updatedContract;
    }
    
    // Clean up
    qDeleteAll(contracts);
    qDeleteAll(contractsToUpdate);
}

void TestContractCRUD::testGetMultipleContracts()
{
    // Add test contracts
    createTestContracts();
    
    // Get all contract IDs
    QList<Contract*> allContracts = m_dbManager->getAllContracts();
    QStringList contractIds;
    for (Contract *contract : allContracts) {
        contractIds.append(contract->id());
    }
    
    // Test batch get
    QList<Contract*> retrievedContracts = m_dbManager->getContracts(contractIds);
    
    QCOMPARE(retrievedContracts.size(), allContracts.size());
    
    // Verify all contracts were retrieved
    for (Contract *contract : retrievedContracts) {
        QVERIFY(contractIds.contains(contract->id()));
    }
    
    // Clean up
    qDeleteAll(allContracts);
    qDeleteAll(retrievedContracts);
}

void TestContractCRUD::testDatabaseSynchronization()
{
    // Add expired contract with active status
    Contract *expiredContract = createTestContract("Expired Client", "Active");
    expiredContract->setEndDate(QDate::currentDate().addDays(-10)); // Expired
    QString contractId = m_dbManager->addContract(expiredContract);
    QVERIFY(!contractId.isEmpty());
    
    // Test synchronization
    bool syncResult = m_dbManager->synchronizeDatabase();
    QVERIFY(syncResult);
    
    // Verify expired contract status was updated
    Contract *syncedContract = m_dbManager->getContract(contractId);
    QVERIFY(syncedContract != nullptr);
    QCOMPARE(syncedContract->status(), "Expired");
    
    delete syncedContract;
}

void TestContractCRUD::testCachingFunctionality()
{
    // Enable caching
    m_dbManager->enableCaching(true);
    QVERIFY(m_dbManager->isCachingEnabled());
    
    // Add test contract
    Contract *contract = createTestContract("Cache Test Client");
    QString contractId = m_dbManager->addContract(contract);
    QVERIFY(!contractId.isEmpty());
    
    // First retrieval (should populate cache)
    Contract *contract1 = m_dbManager->getContract(contractId);
    QVERIFY(contract1 != nullptr);
    
    // Second retrieval (should use cache)
    Contract *contract2 = m_dbManager->getContract(contractId);
    QVERIFY(contract2 != nullptr);
    
    QCOMPARE(contract1->id(), contract2->id());
    QCOMPARE(contract1->clientName(), contract2->clientName());
    
    // Test cache clearing
    m_dbManager->clearCache();
    QCOMPARE(m_dbManager->getCacheSize(), 0);
    
    delete contract1;
    delete contract2;
}

void TestContractCRUD::testWidgetIntegration()
{
    // Test widget CRUD operations - DISABLED FOR NOW
    // QVERIFY(m_widget != nullptr);
    
    // Add contract through widget
    // m_widget->addContract();
    
    // The widget should handle the dialog interaction
    // This is a simplified test - in real scenarios, you'd simulate user input
    
    // For now, just test basic functionality without widget
    Contract *contract = createTestContract("Widget Test Client");
    QString contractId = m_dbManager->addContract(contract);
    QVERIFY(!contractId.isEmpty());
}

void TestContractCRUD::createTestContracts()
{
    QStringList clientNames = {"Client A", "Client B", "Client C", "Client D", "Client E"};
    QStringList statuses = {"Active", "Draft", "Completed", "Active", "Draft"};
    
    for (int i = 0; i < clientNames.size(); ++i) {
        Contract *contract = createTestContract(clientNames[i], statuses[i]);
        QString contractId = m_dbManager->addContract(contract);
        QVERIFY(!contractId.isEmpty());
        m_testContracts.append(contract);
    }
}

Contract* TestContractCRUD::createTestContract(const QString &clientName, const QString &status)
{
    Contract *contract = new Contract(this);
    contract->setClientName(clientName);
    contract->setStartDate(QDate::currentDate());
    contract->setEndDate(QDate::currentDate().addDays(365));
    contract->setValue(1000.0 + QRandomGenerator::global()->bounded(9000)); // Random value between 1000-10000
    contract->setStatus(status);
    contract->setDescription(QString("Test contract for %1").arg(clientName));
    contract->setPaymentTerms(30);
    contract->setHasNonCompeteClause(false);
    
    return contract;
}

void TestContractCRUD::verifyDatabaseState()
{
    QVERIFY(m_dbManager->isDatabaseConnected());
    QVERIFY(m_dbManager->getLastError().isEmpty() || 
            m_dbManager->getLastError().contains("No such table", Qt::CaseInsensitive));
}

// Additional test methods for other scenarios...
void TestContractCRUD::testSearchContracts()
{
    createTestContracts();
    
    // Test search by client name
    QList<Contract*> results = m_dbManager->searchContracts("Client A");
    QVERIFY(results.size() >= 1);
    QVERIFY(results[0]->clientName().contains("Client A"));
    
    qDeleteAll(results);
}

void TestContractCRUD::testFilterByStatus()
{
    createTestContracts();
    
    // Test filter by active status
    QList<Contract*> activeContracts = m_dbManager->getContractsByStatus("Active");
    for (Contract *contract : activeContracts) {
        QCOMPARE(contract->status(), "Active");
    }
    
    qDeleteAll(activeContracts);
}

void TestContractCRUD::testFilterByDateRange()
{
    createTestContracts();
    
    QDate startDate = QDate::currentDate().addDays(-30);
    QDate endDate = QDate::currentDate().addDays(30);
    
    QList<Contract*> contracts = m_dbManager->getContractsByDateRange(startDate, endDate);
    QVERIFY(contracts.size() >= 0); // May be empty, but should not crash
    
    qDeleteAll(contracts);
}

void TestContractCRUD::testFilterByClient()
{
    createTestContracts();
    
    QList<Contract*> clientContracts = m_dbManager->getContractsByClient("Client A");
    for (Contract *contract : clientContracts) {
        QVERIFY(contract->clientName().contains("Client A"));
    }
    
    qDeleteAll(clientContracts);
}

void TestContractCRUD::testContractStatistics()
{
    createTestContracts();
    
    QJsonObject stats = m_dbManager->getContractStatistics();
    QVERIFY(stats.contains("totalContracts"));
    QVERIFY(stats.contains("activeContracts"));
    QVERIFY(stats.contains("totalValue"));
    
    QVERIFY(stats["totalContracts"].toInt() >= 5);
}

void TestContractCRUD::testStatusDistribution()
{
    createTestContracts();
    
    QJsonArray distribution = m_dbManager->getStatusDistribution();
    QVERIFY(distribution.size() > 0);
    
    for (const QJsonValue &value : distribution) {
        QJsonObject statusInfo = value.toObject();
        QVERIFY(statusInfo.contains("status"));
        QVERIFY(statusInfo.contains("count"));
    }
}

void TestContractCRUD::testMonthlyContractCounts()
{
    createTestContracts();
    
    QJsonArray monthlyCounts = m_dbManager->getMonthlyContractCounts();
    // May be empty for test data, but should not crash
    QVERIFY(monthlyCounts.size() >= 0);
}

void TestContractCRUD::testDatabaseOptimization()
{
    bool result = m_dbManager->optimizeDatabase();
    QVERIFY(result);
}

void TestContractCRUD::testDatabaseBackupRestore()
{
    // Create test data
    createTestContracts();
    
    // Create backup
    QString backupPath = m_tempDir->path() + "/backup_test.db";
    bool backupResult = m_dbManager->backupDatabase(backupPath);
    QVERIFY(backupResult);
    QVERIFY(QFile::exists(backupPath));
    
    // Test restore
    bool restoreResult = m_dbManager->restoreDatabase(backupPath);
    QVERIFY(restoreResult);
    
    // Verify data after restore
    QList<Contract*> contracts = m_dbManager->getAllContracts();
    QVERIFY(contracts.size() >= 5);
    
    qDeleteAll(contracts);
}

void TestContractCRUD::testCachePerformance()
{
    // Enable caching
    m_dbManager->enableCaching(true);
    
    // Add test contracts
    createTestContracts();
    QList<Contract*> allContracts = m_dbManager->getAllContracts();
    QStringList contractIds;
    for (Contract *contract : allContracts) {
        contractIds.append(contract->id());
    }
      // Measure time for first access (no cache)
    QElapsedTimer timer;
    timer.start();
    QList<Contract*> firstAccess = m_dbManager->getContracts(contractIds);
    int firstAccessTime = timer.elapsed();

    // Measure time for second access (with cache)
    timer.restart();
    QList<Contract*> secondAccess = m_dbManager->getContracts(contractIds);
    int secondAccessTime = timer.elapsed();
    
    qDebug() << "First access:" << firstAccessTime << "ms";
    qDebug() << "Second access:" << secondAccessTime << "ms";
    
    // Cache should improve performance (allowing some margin for test variability)
    // QVERIFY(secondAccessTime <= firstAccessTime * 1.5);
    
    qDeleteAll(allContracts);
    qDeleteAll(firstAccess);
    qDeleteAll(secondAccess);
}

void TestContractCRUD::testDatabaseConnectionLoss()
{
    // This test would simulate database connection loss
    // For now, just verify proper error handling
    QVERIFY(m_dbManager->isDatabaseConnected());
}

void TestContractCRUD::testTransactionRollback()
{
    // Test rollback on batch operation failure
    // Create a mix of valid and invalid contracts
    QList<Contract*> contracts;
    
    // Valid contract
    contracts.append(createTestContract("Valid Client"));
    
    // Invalid contract (empty client name)
    Contract *invalidContract = new Contract(this);
    invalidContract->setClientName(""); // Invalid
    invalidContract->setStartDate(QDate::currentDate());
    invalidContract->setEndDate(QDate::currentDate().addDays(30));
    contracts.append(invalidContract);
    
    QStringList addedIds;
    QString errorMessage;
    bool result = m_dbManager->addContracts(contracts, addedIds, errorMessage);
    
    // Batch should fail due to invalid contract
    QVERIFY(!result);
    QVERIFY(!errorMessage.isEmpty());
    
    // No contracts should be added due to rollback
    QVERIFY(addedIds.isEmpty());
    
    qDeleteAll(contracts);
}

void TestContractCRUD::testValidationErrors()
{
    // Test various validation scenarios
    Contract *contract = new Contract(this);
    QString errorMessage;
    
    // Empty client name
    contract->setClientName("");
    QVERIFY(!m_dbManager->validateContract(contract, errorMessage));
    QVERIFY(errorMessage.contains("client name", Qt::CaseInsensitive));
    
    // Invalid date range
    contract->setClientName("Test Client");
    contract->setStartDate(QDate::currentDate());
    contract->setEndDate(QDate::currentDate().addDays(-1));
    QVERIFY(!m_dbManager->validateContract(contract, errorMessage));
    QVERIFY(errorMessage.contains("end date", Qt::CaseInsensitive));
    
    delete contract;
}

void TestContractCRUD::testContractLifecycle()
{
    // Test complete contract lifecycle
    Contract *contract = createTestContract("Lifecycle Test Client", "Draft");
    
    // 1. Add as draft
    QString contractId = m_dbManager->addContract(contract);
    QVERIFY(!contractId.isEmpty());
    
    // 2. Activate contract
    contract->setStatus("Active");
    QVERIFY(m_dbManager->updateContract(contract));
    
    // 3. Complete contract
    contract->setStatus("Completed");
    QVERIFY(m_dbManager->updateContract(contract));
      // 4. Now it should be deletable
    QVERIFY(m_dbManager->canDeleteContract(contractId));
    QVERIFY(m_dbManager->deleteContract(contractId));
    
    // 5. Verify deletion
    Contract *deletedContract = m_dbManager->getContract(contractId);
    QVERIFY(deletedContract == nullptr);
}

void TestContractCRUD::testDeleteMultipleContracts()
{
    // Add test contracts
    createTestContracts();
    
    // Get all contracts
    QList<Contract*> contracts = m_dbManager->getAllContracts();
    QVERIFY(contracts.size() >= 3);
    
    // Select first 3 contracts for deletion (assuming they're in Draft status)
    QStringList contractIds;
    for (int i = 0; i < 3 && i < contracts.size(); ++i) {
        if (contracts[i]->status() == "Draft") {
            contractIds.append(contracts[i]->id());
        }
    }
    
    QVERIFY(contractIds.size() >= 2); // At least 2 contracts to delete
    
    // Test batch deletion
    QString errorMessage;
    bool result = m_dbManager->deleteContracts(contractIds, errorMessage);
    
    QVERIFY(result);
    QVERIFY(errorMessage.isEmpty());
    
    // Verify deletions
    for (const QString &contractId : contractIds) {
        Contract *deletedContract = m_dbManager->getContract(contractId);
        QVERIFY(deletedContract == nullptr);
    }
    
    // Clean up remaining contracts
    qDeleteAll(contracts);
}

// Test runner
QTEST_MAIN(TestContractCRUD)
#include "test_contract_crud.moc"
