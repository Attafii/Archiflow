#include <QtTest/QtTest>
#include <QDate>
#include "../databasemanager.h"

class TestDatabaseManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testExpiringContracts();
    void testExpiringContractsEdgeCases();

private:
    DatabaseManager* dbManager;
    QString testDbPath;
};

void TestDatabaseManager::initTestCase()
{
    // Set up a test database
    testDbPath = QDir::currentPath() + "/test_contracts.db";
    dbManager = new DatabaseManager();
    QVERIFY(dbManager->initializeDatabase());

    // Add test contracts
    QDate today = QDate::currentDate();
    QDate futureDate = today.addDays(10);
    QDate pastDate = today.addDays(-5);

    // Active contract expiring soon
    QVERIFY(dbManager->addContract("TEST001", "Client A", today, today.addDays(5),
                                  1000.0, "Active", "Test contract 1"));

    // Active contract not expiring soon
    QVERIFY(dbManager->addContract("TEST002", "Client B", today, futureDate,
                                  2000.0, "Active", "Test contract 2"));

    // Expired contract
    QVERIFY(dbManager->addContract("TEST003", "Client C", pastDate, pastDate,
                                  3000.0, "Expired", "Test contract 3"));

    // Inactive contract expiring soon
    QVERIFY(dbManager->addContract("TEST004", "Client D", today, today.addDays(3),
                                  4000.0, "Inactive", "Test contract 4"));
}

void TestDatabaseManager::cleanupTestCase()
{
    if (dbManager) {
        delete dbManager;
        dbManager = nullptr;
    }
    QFile::remove(testDbPath);
}

void TestDatabaseManager::testExpiringContracts()
{
    // Test contracts expiring within 7 days
    QSqlQuery query = dbManager->getExpiringContracts(7);
    int count = 0;
    QStringList foundIds;

    while (query.next()) {
        count++;
        QString contractId = query.value("contract_id").toString();
        QString status = query.value("status").toString();
        foundIds << contractId;

        // Verify only active contracts are returned
        QCOMPARE(status, QString("Active"));
    }

    // Should only find TEST001 (active and expiring within 7 days)
    QCOMPARE(count, 1);
    QVERIFY(foundIds.contains("TEST001"));
    QVERIFY(!foundIds.contains("TEST002")); // Not expiring soon
    QVERIFY(!foundIds.contains("TEST003")); // Expired
    QVERIFY(!foundIds.contains("TEST004")); // Inactive
}

void TestDatabaseManager::testExpiringContractsEdgeCases()
{
    // Test with 0 days threshold
    QSqlQuery zeroQuery = dbManager->getExpiringContracts(0);
    int zeroCount = 0;
    while (zeroQuery.next()) {
        zeroCount++;
    }
    QCOMPARE(zeroCount, 0);

    // Test with negative days threshold
    QSqlQuery negativeQuery = dbManager->getExpiringContracts(-1);
    int negativeCount = 0;
    while (negativeQuery.next()) {
        negativeCount++;
    }
    QCOMPARE(negativeCount, 0);

    // Test with large threshold
    QSqlQuery largeQuery = dbManager->getExpiringContracts(30);
    int largeCount = 0;
    while (largeQuery.next()) {
        QString status = largeQuery.value("status").toString();
        QCOMPARE(status, QString("Active")); // Verify only active contracts
        largeCount++;
    }
    QCOMPARE(largeCount, 2); // Should find both TEST001 and TEST002
}

QTEST_MAIN(TestDatabaseManager)
#include "test_databasemanager.moc"