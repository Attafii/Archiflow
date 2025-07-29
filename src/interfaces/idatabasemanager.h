#ifndef IDATABASEMANAGER_H
#define IDATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QSqlQuery>
#include <QSqlDatabase>

/**
 * @brief Interface for database management operations
 * 
 * This interface defines the contract for all database-related operations,
 * allowing for different database implementations while maintaining consistency.
 */
class IDatabaseManager
{
public:
    virtual ~IDatabaseManager() = default;

    // Database lifecycle
    virtual bool initialize(const QString &databasePath) = 0;
    virtual void shutdown() = 0;

    // Connection management
    virtual bool isConnected() const = 0;
    virtual QSqlDatabase database() const = 0;

    // Query execution
    virtual QSqlQuery executeQuery(const QString &query, const QVariantList &params = QVariantList()) = 0;
    virtual bool executeNonQuery(const QString &query, const QVariantList &params = QVariantList()) = 0;

    // Transaction management
    virtual bool beginTransaction() = 0;
    virtual bool commitTransaction() = 0;
    virtual bool rollbackTransaction() = 0;

    // Schema management
    virtual bool createTables() = 0;
    virtual bool tableExists(const QString &tableName) const = 0;
    virtual QStringList tableNames() const = 0;

    // Error handling
    virtual QString lastError() const = 0;

    // Utility methods
    virtual int currentSchemaVersion() const = 0;
    virtual bool resetDatabase() = 0;
    virtual bool ensureDefaultData() = 0;
};

/**
 * @brief QObject-based interface for database management with signals
 * 
 * This interface extends IDatabaseManager with Qt's signal-slot mechanism
 * for event-driven database operations.
 */
class IDatabaseManagerSignals : public QObject, public IDatabaseManager
{
    Q_OBJECT

public:
    explicit IDatabaseManagerSignals(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IDatabaseManagerSignals() = default;

signals:
    void connected();
    void disconnected();
    void error(const QString &error);
    void tableCreated(const QString &tableName);
    void dataChanged();
};

#endif // IDATABASEMANAGER_H
