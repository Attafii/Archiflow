#include "databasemanager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    // Ensure database directory exists
    QDir dir;
    QString dbPath = QDir::currentPath() + "/data";
    if (!dir.exists(dbPath)) {
        dir.mkpath(dbPath);
    }

    // Initialize database connection
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath + "/materials.db");

    if (!m_db.open()) {
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    // Create tables if they don't exist
    if (!createTables()) {
        qCritical() << "Failed to create tables";
        return false;
    }

    m_initialized = true;
    qDebug() << "Database initialized successfully";
    return true;
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}

bool DatabaseManager::isInitialized() const
{
    return m_initialized;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;
    
    // Create materials table
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS materials ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    name TEXT NOT NULL,"
        "    quantity REAL NOT NULL,"
        "    unit TEXT NOT NULL,"
        "    price REAL NOT NULL,"
        "    description TEXT"
        ");"
    );
    
    if (!success) {
        qCritical() << "Failed to create materials table:" << query.lastError().text();
        return false;
    }
    
    return true;
}