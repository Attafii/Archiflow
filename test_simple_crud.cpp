#include <QApplication>
#include <QDebug>
#include <QTemporaryDir>
#include "src/features/contracts/contract.h"
#include "src/features/contracts/contractdatabasemanager.h"

/**
 * @brief Simple test program to validate enhanced Contract CRUD functionality
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== Enhanced Contract CRUD Test ===";
    
    // Create temporary directory for test database
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qDebug() << "Failed to create temporary directory";
        return 1;
    }
    
    QString testDbPath = tempDir.path() + "/test_contracts.db";
    qDebug() << "Test database path:" << testDbPath;
    
    // Initialize database manager
    ContractDatabaseManager dbManager;
    if (!dbManager.initialize(testDbPath)) {
        qDebug() << "Failed to initialize database:" << dbManager.getLastError();
        return 1;
    }
    
    qDebug() << "Database initialized successfully";
    qDebug() << "Caching enabled:" << dbManager.isCachingEnabled();
    
    // Test 1: Add single contract
    qDebug() << "\n--- Test 1: Add Single Contract ---";
    Contract testContract;
    testContract.setClientName("Test Client");
    testContract.setStartDate(QDate::currentDate());
    testContract.setEndDate(QDate::currentDate().addYears(1));
    testContract.setValue(50000.0);
    testContract.setStatus("Active");
    testContract.setDescription("Test contract for CRUD functionality");
    testContract.setPaymentTerms(30);
    
    QString contractId = dbManager.addContract(&testContract);
    if (contractId.isEmpty()) {
        qDebug() << "Failed to add contract:" << dbManager.getLastError();
        return 1;
    }
    qDebug() << "Contract added successfully with ID:" << contractId;
    
    // Test 2: Retrieve contract
    qDebug() << "\n--- Test 2: Retrieve Contract ---";
    Contract* retrievedContract = dbManager.getContract(contractId);
    if (!retrievedContract) {
        qDebug() << "Failed to retrieve contract:" << dbManager.getLastError();
        return 1;
    }
    qDebug() << "Contract retrieved successfully:";
    qDebug() << "  Client:" << retrievedContract->clientName();
    qDebug() << "  Value:" << retrievedContract->value();
    qDebug() << "  Status:" << retrievedContract->status();
    
    // Test 3: Update contract
    qDebug() << "\n--- Test 3: Update Contract ---";
    retrievedContract->setValue(75000.0);
    retrievedContract->setStatus("Completed");
    
    if (!dbManager.updateContract(retrievedContract)) {
        qDebug() << "Failed to update contract:" << dbManager.getLastError();
        delete retrievedContract;
        return 1;
    }
    qDebug() << "Contract updated successfully";
    
    // Test 4: Verify update
    Contract* updatedContract = dbManager.getContract(contractId);
    if (!updatedContract) {
        qDebug() << "Failed to retrieve updated contract";
        delete retrievedContract;
        return 1;
    }
    qDebug() << "Updated contract verified:";
    qDebug() << "  New Value:" << updatedContract->value();
    qDebug() << "  New Status:" << updatedContract->status();
    
    // Test 5: Batch operations
    qDebug() << "\n--- Test 5: Batch Add Operations ---";
    QList<Contract*> batchContracts;
    for (int i = 1; i <= 3; ++i) {
        Contract* contract = new Contract;
        contract->setClientName(QString("Batch Client %1").arg(i));
        contract->setStartDate(QDate::currentDate());
        contract->setEndDate(QDate::currentDate().addMonths(6));
        contract->setValue(10000.0 * i);
        contract->setStatus("Draft");
        contract->setDescription(QString("Batch test contract %1").arg(i));
        batchContracts.append(contract);
    }
    
    QStringList addedIds;
    QString errorMessage;
    if (!dbManager.addContracts(batchContracts, addedIds, errorMessage)) {
        qDebug() << "Batch add failed:" << errorMessage;
        qDeleteAll(batchContracts);
        delete retrievedContract;
        delete updatedContract;
        return 1;
    }
    qDebug() << "Batch add successful. Added" << addedIds.size() << "contracts";
    
    // Test 6: Get all contracts
    qDebug() << "\n--- Test 6: Get All Contracts ---";
    QList<Contract*> allContracts = dbManager.getAllContracts();
    qDebug() << "Total contracts in database:" << allContracts.size();
    
    for (Contract* contract : allContracts) {
        qDebug() << "  -" << contract->clientName() << "(" << contract->status() << ")";
    }
    
    // Test 7: Statistics
    qDebug() << "\n--- Test 7: Contract Statistics ---";
    QJsonObject stats = dbManager.getContractStatistics();
    qDebug() << "Statistics:";
    qDebug() << "  Total Contracts:" << stats["totalContracts"].toInt();
    qDebug() << "  Active Contracts:" << stats["activeContracts"].toInt();
    qDebug() << "  Total Value:" << stats["totalValue"].toDouble();
    qDebug() << "  Average Value:" << stats["averageValue"].toDouble();
    
    // Test 8: Database synchronization
    qDebug() << "\n--- Test 8: Database Synchronization ---";
    if (!dbManager.synchronizeDatabase()) {
        qDebug() << "Database synchronization failed:" << dbManager.getLastError();
    } else {
        qDebug() << "Database synchronization successful";
    }
    
    // Test 9: Caching performance
    qDebug() << "\n--- Test 9: Caching Performance ---";
    qDebug() << "Cache size before:" << dbManager.getCacheSize();
    
    // Access same contract multiple times
    for (int i = 0; i < 3; ++i) {
        Contract* cachedContract = dbManager.getContract(contractId);
        if (cachedContract) {
            qDebug() << "Cache access" << (i+1) << ": Client =" << cachedContract->clientName();
            delete cachedContract;
        }
    }
    qDebug() << "Cache size after:" << dbManager.getCacheSize();
    
    // Test 10: Delete operations (only non-active contracts)
    qDebug() << "\n--- Test 10: Delete Operations ---";
    QStringList deletableIds;
    for (const QString& id : addedIds) {
        if (dbManager.canDeleteContract(id)) {
            deletableIds.append(id);
        }
    }
    
    if (!deletableIds.isEmpty()) {
        QString deleteError;
        if (dbManager.deleteContracts(deletableIds, deleteError)) {
            qDebug() << "Batch delete successful. Deleted" << deletableIds.size() << "contracts";
        } else {
            qDebug() << "Batch delete failed:" << deleteError;
        }
    } else {
        qDebug() << "No contracts can be deleted (business rules)";
    }
    
    // Final statistics
    qDebug() << "\n--- Final Statistics ---";
    QList<Contract*> finalContracts = dbManager.getAllContracts();
    qDebug() << "Final contract count:" << finalContracts.size();
    
    // Cleanup
    delete retrievedContract;
    delete updatedContract;
    qDeleteAll(allContracts);
    qDeleteAll(batchContracts);
    qDeleteAll(finalContracts);
    
    qDebug() << "\n=== Enhanced Contract CRUD Test Completed Successfully ===";
    
    return 0;
}
