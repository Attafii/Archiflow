#include "databasemanager.h"
#include "migrations.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

const QString DatabaseManager::CONNECTION_NAME = "ArchiFlowDB";

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_connected(false)
{
    m_migrations = std::make_unique<Migrations>(this);
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::initialize(const QString &databasePath)
{
    if (m_connected) {
        return true;
    }

    qDebug() << "Initializing database at:" << databasePath;

    // Ensure database directory exists
    QFileInfo dbInfo(databasePath);
    QDir dbDir = dbInfo.absoluteDir();
    if (!dbDir.exists()) {
        dbDir.mkpath(".");
    }

    if (!setupConnection(databasePath)) {
        return false;
    }

    if (!createSchemaVersionTable()) {
        return false;
    }    if (!runMigrations()) {
        return false;
    }

    // Ensure default data is present after migrations
    if (!ensureDefaultData()) {
        qWarning() << "Failed to ensure default data, but continuing...";
    }

    m_connected = true;
    emit connected();
    
    qDebug() << "Database initialized successfully";
    return true;
}

void DatabaseManager::close()
{
    if (!m_connected) {
        return;
    }

    qDebug() << "Closing database connection";
    
    if (m_database.isOpen()) {
        m_database.close();
    }
    
    QSqlDatabase::removeDatabase(CONNECTION_NAME);
    m_connected = false;
    
    emit disconnected();
}

bool DatabaseManager::isConnected() const
{
    return m_connected && m_database.isOpen();
}

QSqlDatabase DatabaseManager::database() const
{
    return m_database;
}

QSqlQuery DatabaseManager::executeQuery(const QString &query, const QVariantList &params)
{
    QSqlQuery sqlQuery(m_database);
    sqlQuery.prepare(query);
    
    for (const QVariant &param : params) {
        sqlQuery.addBindValue(param);
    }
    
    if (!sqlQuery.exec()) {
        m_lastError = sqlQuery.lastError().text();
        qWarning() << "Query execution failed:" << m_lastError;
        qWarning() << "Query:" << query;
        emit error(m_lastError);
    }
    
    return sqlQuery;
}

bool DatabaseManager::executeNonQuery(const QString &query, const QVariantList &params)
{
    QSqlQuery result = executeQuery(query, params);
    return !result.lastError().isValid();
}

bool DatabaseManager::beginTransaction()
{
    if (!m_database.transaction()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "Failed to begin transaction:" << m_lastError;
        emit error(m_lastError);
        return false;
    }
    return true;
}

bool DatabaseManager::commitTransaction()
{
    if (!m_database.commit()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "Failed to commit transaction:" << m_lastError;
        emit error(m_lastError);
        return false;
    }
    return true;
}

bool DatabaseManager::rollbackTransaction()
{
    if (!m_database.rollback()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "Failed to rollback transaction:" << m_lastError;
        emit error(m_lastError);
        return false;
    }
    return true;
}

bool DatabaseManager::createTables()
{
    return m_migrations->createInitialSchema();
}

bool DatabaseManager::runMigrations()
{
    return m_migrations->runMigrations();
}

int DatabaseManager::currentSchemaVersion() const
{
    QSqlQuery query = const_cast<DatabaseManager*>(this)->executeQuery(
        "SELECT version FROM schema_version ORDER BY id DESC LIMIT 1"
    );
    
    if (query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::tableExists(const QString &tableName) const
{
    QStringList tables = tableNames();
    return tables.contains(tableName, Qt::CaseInsensitive);
}

QStringList DatabaseManager::tableNames() const
{
    return m_database.tables();
}

void DatabaseManager::onDatabaseError()
{
    if (m_database.lastError().isValid()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "Database error:" << m_lastError;
        emit error(m_lastError);
    }
}

bool DatabaseManager::setupConnection(const QString &databasePath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    m_database.setDatabaseName(databasePath);
    
    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qCritical() << "Failed to open database:" << m_lastError;
        emit error(m_lastError);
        return false;
    }
    
    // Enable foreign key constraints
    executeNonQuery("PRAGMA foreign_keys = ON");
    
    // Set journal mode for better performance
    executeNonQuery("PRAGMA journal_mode = WAL");
    
    return true;
}

bool DatabaseManager::createSchemaVersionTable()
{
    const QString createTable = R"(
        CREATE TABLE IF NOT EXISTS schema_version (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            version INTEGER NOT NULL,
            applied_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return executeNonQuery(createTable);
}

bool DatabaseManager::resetDatabase()
{
    if (!m_connected) {
        m_lastError = "Database not connected";
        return false;
    }
    
    qDebug() << "Resetting database - dropping all tables and recreating schema";
    
    if (!beginTransaction()) {
        return false;
    }
    
    // Clear all tables
    if (!clearAllTables()) {
        rollbackTransaction();
        return false;
    }
    
    // Recreate schema
    if (!recreateSchema()) {
        rollbackTransaction();
        return false;
    }
    
    if (!commitTransaction()) {
        return false;
    }
    
    qDebug() << "Database reset completed successfully";
    return true;
}

bool DatabaseManager::clearAllTables()
{
    qDebug() << "Clearing all database tables";
    
    // Get list of all tables (excluding sqlite_* system tables)
    QSqlQuery query = executeQuery(
        "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'"
    );
    
    QStringList tables;
    while (query.next()) {
        tables << query.value(0).toString();
    }
    
    // Disable foreign key constraints temporarily
    if (!executeNonQuery("PRAGMA foreign_keys = OFF")) {
        return false;
    }
    
    // Drop all tables
    for (const QString &tableName : tables) {
        if (!executeNonQuery(QString("DROP TABLE IF EXISTS %1").arg(tableName))) {
            // Re-enable foreign keys before returning
            executeNonQuery("PRAGMA foreign_keys = ON");
            return false;
        }
        qDebug() << "Dropped table:" << tableName;
    }
    
    // Re-enable foreign key constraints
    if (!executeNonQuery("PRAGMA foreign_keys = ON")) {
        return false;
    }
    
    return true;
}

bool DatabaseManager::recreateSchema()
{
    qDebug() << "Recreating database schema";
    
    // Create schema version table
    if (!createSchemaVersionTable()) {
        return false;
    }
    
    // Run all migrations from the beginning
    if (!runMigrations()) {
        return false;
    }
    
    // Ensure default data is present after recreation
    if (!ensureDefaultData()) {
        qWarning() << "Failed to ensure default data after schema recreation";
        return false;
    }
    
    return true;
}

bool DatabaseManager::ensureDefaultData()
{
    if (!m_connected) {
        m_lastError = "Database not connected";
        return false;
    }
      qDebug() << "Ensuring default materials are present in database";
    
    // Check if we already have materials
    QSqlQuery checkQuery = executeQuery("SELECT COUNT(*) FROM materials");
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "Materials already exist, skipping default data insertion";
        return true;
    }
    
    // Check if we have suppliers (needed for foreign key references)
    QSqlQuery supplierCheck = executeQuery("SELECT COUNT(*) FROM suppliers");
    if (!supplierCheck.next() || supplierCheck.value(0).toInt() == 0) {
        qDebug() << "No suppliers found, cannot insert materials with supplier references";
        return false;
    }
      // Insert default materials (without specifying IDs, let SQLite auto-generate them)
    QString insertMaterials = R"(
        INSERT INTO materials (name, description, category, quantity, unit, price, 
                             supplier_id, barcode, location, minimum_stock, maximum_stock, 
                             reorder_point, status, created_by, updated_by) VALUES        ('Concrete', 'Standard concrete mix for foundations', 'Construction Materials', 100, 'm³', 150.0, 1, 'DEFAULT001', 'Warehouse A', 20, 200, 30, 'active', 'System', 'System'),
        ('Steel Rebar', 'Reinforcement steel bars 16mm', 'Metal', 50, 'tons', 800.0, 2, 'DEFAULT002', 'Warehouse B', 10, 100, 15, 'active', 'System', 'System'),
        ('Portland Cement', 'High-grade portland cement bags', 'Construction Materials', 5, 'bags', 25.0, 1, 'DEFAULT003', 'Storage Room 1', 20, 500, 25, 'active', 'System', 'System'),
        ('Red Bricks', 'Standard clay bricks for construction', 'Masonry', 10000, 'pcs', 0.5, 1, 'DEFAULT004', 'Yard A', 5000, 50000, 7500, 'active', 'System', 'System'),
        ('Exterior Paint', 'Weather-resistant exterior paint', 'Finishes', 2, 'gallons', 45.0, 3, 'DEFAULT005', 'Storage Room 2', 10, 100, 15, 'active', 'System', 'System'),
        ('Ceramic Tiles', 'Premium ceramic floor tiles 60x60cm', 'Finishes', 500, 'm²', 35.0, 3, 'DEFAULT006', 'Warehouse C', 100, 1000, 150, 'active', 'System', 'System'),
        ('Thermal Insulation', 'Rockwool thermal insulation panels', 'Insulation', 200, 'm²', 15.0, 1, 'DEFAULT007', 'Warehouse A', 50, 500, 75, 'active', 'System', 'System'),
        ('Pine Lumber', 'Construction grade pine lumber 2x4x8ft', 'Wood', 300, 'pieces', 12.50, 1, 'DEFAULT008', 'Lumber Yard', 100, 1000, 150, 'active', 'System', 'System'),
        ('PVC Pipe', 'PVC pipe 4 inch diameter for drainage', 'Plumbing', 8, 'meters', 18.0, 2, 'DEFAULT009', 'Storage Room 3', 20, 200, 25, 'active', 'System', 'System'),
        ('Electrical Wire', '12 AWG copper electrical wire', 'Electrical', 500, 'feet', 2.25, 2, 'DEFAULT010', 'Electrical Storage', 200, 2000, 300, 'active', 'System', 'System')
    )";
    
    bool success = executeNonQuery(insertMaterials);
    
    if (success) {
        qDebug() << "Default materials inserted successfully";
    } else {
        qDebug() << "Failed to insert default materials:" << m_lastError;
    }
    
    return success;
}

void DatabaseManager::setSchemaVersion(int version)
{
    executeNonQuery("INSERT INTO schema_version (version) VALUES (?)", {version});
}
