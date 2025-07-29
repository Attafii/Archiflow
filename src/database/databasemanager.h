#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

class Migrations;

/**
 * @brief The DatabaseManager class - Manages SQLite database operations
 * 
 * This class handles database connection, initialization, schema management,
 * and provides a centralized interface for all database operations.
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // Database lifecycle
    bool initialize(const QString &databasePath);
    void close();

    // Connection management
    bool isConnected() const;
    QSqlDatabase database() const;

    // Query execution
    QSqlQuery executeQuery(const QString &query, const QVariantList &params = QVariantList());
    bool executeNonQuery(const QString &query, const QVariantList &params = QVariantList());

    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Schema management
    bool createTables();
    bool runMigrations();
    int currentSchemaVersion() const;    // Utility methods
    QString lastError() const;
    bool tableExists(const QString &tableName) const;
    QStringList tableNames() const;
      // Database reset and maintenance
    bool resetDatabase();
    bool clearAllTables();
    bool recreateSchema();
    bool ensureDefaultData();

signals:
    void connected();
    void disconnected();
    void error(const QString &errorMessage);

private slots:
    void onDatabaseError();

private:
    bool setupConnection(const QString &databasePath);
    bool createSchemaVersionTable();
    void setSchemaVersion(int version);

    QSqlDatabase m_database;
    std::unique_ptr<Migrations> m_migrations;
    QString m_lastError;
    bool m_connected;

    static const QString CONNECTION_NAME;
};

#endif // DATABASEMANAGER_H
