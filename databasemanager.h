#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVector>
#include "client.h" // Pour la structure Client

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase();
    void closeDatabase();
    bool initializeDatabase();
    
    // Client CRUD operations
    bool addClient(const Client &client);
    bool updateClient(int id, const Client &client);
    bool deleteClient(int id);
    QVector<Client> getAllClients();
    Client getClientById(int id);
    QVector<Client> searchClients(const QString &searchText);
    
    // Get the last inserted row ID
    int getLastInsertId();

private:
    QSqlDatabase m_db;
    bool isConnected;
};

#endif // DATABASEMANAGER_H