#ifndef CLIENTDATABASEMANAGER_H
#define CLIENTDATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QMutex>

class ClientContact;

/**
 * @brief The ClientDatabaseManager class handles all database operations for clients
 * 
 * This class provides a complete database abstraction layer for ClientContact management
 * including CRUD operations, search, and reporting functionality.
 */
class ClientDatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit ClientDatabaseManager(QObject *parent = nullptr);
    ~ClientDatabaseManager();

    // Database connection
    bool initialize(const QString &databasePath = QString());
    bool isConnected() const;
    void close();
    QString lastError() const;

    // ClientContact operations
    bool addClient(const ClientContact *ClientContact);
    bool updateClient(const ClientContact *ClientContact);
    bool deleteClient(const QString &clientId);
    ClientContact* getClient(const QString &clientId);
    QList<ClientContact*> getAllClients();
    QList<ClientContact*> searchClients(const QString &searchTerm);
    bool clientExists(const QString &clientId);
    bool emailExists(const QString &email, const QString &excludeClientId = QString());

    // Location-based queries
    QList<ClientContact*> getClientsByLocation(double latitude, double longitude, double radius);
    QList<ClientContact*> getClientsWithCoordinates();

    // Statistics
    int getTotalClientCount();
    int getClientCountByCity(const QString &city);
    int getClientCountByCountry(const QString &country);
    QStringList getAllCities();
    QStringList getAllCountries();

    // Data management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool vacuum();
    bool backup(const QString &backupPath);
    bool restore(const QString &backupPath);

public slots:
    void refresh();

signals:
    void dataChanged();
    void clientAdded(const QString &clientId);
    void clientUpdated(const QString &clientId);
    void clientDeleted(const QString &clientId);
    void errorOccurred(const QString &error);

private:
    bool createTables();
    bool createClientsTable();
    bool createIndexes();
    
    ClientContact* clientFromQuery(const QSqlQuery &query);
    void setLastError(const QString &error);
    QString generateConnectionName();

    QSqlDatabase m_database;
    QString m_connectionName;
    QString m_lastError;
    mutable QMutex m_mutex;
    
    static const QString DATABASE_VERSION;
    static const QString CLIENTS_TABLE;
};

#endif // CLIENTDATABASEMANAGER_H
