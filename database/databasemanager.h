#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QFile>
#include <QDir>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();
    bool initialize();
    QSqlDatabase database() const;
    bool isInitialized() const;

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    // Prevent copying
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    bool createTables();
    
    QSqlDatabase m_db;
    bool m_initialized;
};

#endif // DATABASEMANAGER_H