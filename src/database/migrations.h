#ifndef MIGRATIONS_H
#define MIGRATIONS_H

#include <QObject>
#include <QStringList>
#include <functional>

class DatabaseManager;

/**
 * @brief The Migrations class - Handles database schema migrations
 * 
 * This class manages database schema versioning and migrations,
 * ensuring the database structure is always up-to-date.
 */
class Migrations : public QObject
{
    Q_OBJECT

public:
    explicit Migrations(DatabaseManager *databaseManager, QObject *parent = nullptr);

    // Migration management
    bool createInitialSchema();
    bool runMigrations();
    
    // Migration registration
    void addMigration(int version, const QString &description, std::function<bool()> migration);

private:
    struct Migration {
        int version;
        QString description;
        std::function<bool()> execute;
    };

    bool executeMigration(const Migration &migration);
    void registerCoreMigrations();
    
    DatabaseManager *m_databaseManager;
    QList<Migration> m_migrations;
};

#endif // MIGRATIONS_H
