#include "databasemanager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDebug>
#include <QDir>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , isConnected(false)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase()
{
    // Check if database is already open
    if (isConnected) {
        // Vérifier si la connexion est toujours valide
        if (m_db.isOpen()) {
            return true;
        } else {
            // La connexion a été perdue, réinitialiser l'état
            isConnected = false;
        }
    }

    // Set up the database connection
    if (!QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
    } else {
        m_db = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    }
    
    // Set the database file path
    QString dbPath = QDir::currentPath() + "/clients.db";
    m_db.setDatabaseName(dbPath);
    
    // Open the database
    if (!m_db.open()) {
        qDebug() << "Error opening database:" << m_db.lastError().text();
        return false;
    }
    
    isConnected = true;
    qDebug() << "Database opened successfully at:" << dbPath;
    
    // Initialize the database tables if they don't exist
    if (!initializeDatabase()) {
        closeDatabase();
        return false;
    }
    
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (isConnected) {
        // Vérifier si la connexion est toujours active avant de la fermer
        if (m_db.isOpen()) {
            m_db.close();
            qDebug() << "Database connection closed";
        }
        isConnected = false;
    }
}

bool DatabaseManager::initializeDatabase()
{
    QSqlQuery query;
    
    // Create clients table if it doesn't exist
    QString createTableQuery = 
        "CREATE TABLE IF NOT EXISTS clients ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "project TEXT NOT NULL, "
        "location TEXT, "
        "postcode TEXT, "
        "phone TEXT, "
        "email TEXT, "
        "service_area TEXT, "
        "description TEXT, "
        "latitude REAL, "
        "longitude REAL, "
        "cost_estimation REAL DEFAULT 0.0, "
        "estimation_details TEXT"
        ");";
    
    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating clients table:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::addClient(const Client &client)
{
    // S'assurer que la base de données est ouverte avant d'effectuer des opérations
    if (!isConnected || !m_db.isOpen()) {
        if (!openDatabase()) {
            return false;
        }
    }
    
    QSqlQuery query;
    query.prepare("INSERT INTO clients (name, project, location, postcode, phone, email, service_area, description, latitude, longitude, cost_estimation, estimation_details) "
                 "VALUES (:name, :project, :location, :postcode, :phone, :email, :service_area, :description, :latitude, :longitude, :cost_estimation, :estimation_details)");
    query.bindValue(":name", client.name);
    query.bindValue(":project", client.project);
    query.bindValue(":location", client.location);
    query.bindValue(":postcode", client.postcode);
    query.bindValue(":phone", client.phone);
    query.bindValue(":email", client.email);
    query.bindValue(":service_area", client.serviceArea);
    query.bindValue(":description", client.description);
    query.bindValue(":latitude", client.latitude);
    query.bindValue(":longitude", client.longitude);
    query.bindValue(":cost_estimation", client.costEstimation);
    query.bindValue(":estimation_details", client.estimationDetails);
    
    if (!query.exec()) {
        qDebug() << "Error adding client:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::updateClient(int id, const Client &client)
{
    // S'assurer que la base de données est ouverte avant d'effectuer des opérations
    if (!isConnected || !m_db.isOpen()) {
        if (!openDatabase()) {
            return false;
        }
    }
    
    QSqlQuery query;
    query.prepare("UPDATE clients SET "
                 "name = :name, "
                 "project = :project, "
                 "location = :location, "
                 "postcode = :postcode, "
                 "phone = :phone, "
                 "email = :email, "
                 "service_area = :service_area, "
                 "description = :description, "
                 "latitude = :latitude, "
                 "longitude = :longitude, "
                 "cost_estimation = :cost_estimation, "
                 "estimation_details = :estimation_details "
                 "WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":name", client.name);
    query.bindValue(":project", client.project);
    query.bindValue(":location", client.location);
    query.bindValue(":postcode", client.postcode);
    query.bindValue(":phone", client.phone);
    query.bindValue(":email", client.email);
    query.bindValue(":service_area", client.serviceArea);
    query.bindValue(":description", client.description);
    query.bindValue(":latitude", client.latitude);
    query.bindValue(":longitude", client.longitude);
    query.bindValue(":cost_estimation", client.costEstimation);
    query.bindValue(":estimation_details", client.estimationDetails);
    
    if (!query.exec()) {
        qDebug() << "Error updating client:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::deleteClient(int id)
{
    // S'assurer que la base de données est ouverte avant d'effectuer des opérations
    if (!isConnected || !m_db.isOpen()) {
        if (!openDatabase()) {
            return false;
        }
    }
    
    QSqlQuery query;
    query.prepare("DELETE FROM clients WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        qDebug() << "Error deleting client:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QVector<Client> DatabaseManager::getAllClients()
{
    QVector<Client> clients;
    
    // S'assurer que la base de données est ouverte avant d'effectuer des opérations
    if (!isConnected || !m_db.isOpen()) {
        if (!openDatabase()) {
            return clients;
        }
    }
    
    QSqlQuery query("SELECT id, name, project, location, postcode, phone, email, service_area, description, latitude, longitude, cost_estimation, estimation_details FROM clients");
    while (query.next()) {
        Client client;
        client.id = query.value(0).toInt();
        client.name = query.value(1).toString();
        client.project = query.value(2).toString();
        client.location = query.value(3).toString();
        client.postcode = query.value(4).toString();
        client.phone = query.value(5).toString();
        client.email = query.value(6).toString();
        client.serviceArea = query.value(7).toString();
        client.description = query.value(8).toString();
        client.latitude = query.value(9).toDouble();
        client.longitude = query.value(10).toDouble();
        client.costEstimation = query.value(11).toDouble();
        client.estimationDetails = query.value(12).toString();
        clients.append(client);
    }
    
    return clients;
}

Client DatabaseManager::getClientById(int id)
{
    Client client;
    
    // S'assurer que la base de données est ouverte avant d'effectuer des opérations
    if (!isConnected || !m_db.isOpen()) {
        if (!openDatabase()) {
            return client;
        }
    }
    
    QSqlQuery query;
    query.prepare("SELECT id, name, project, location, postcode, phone, email, service_area, description, latitude, longitude, cost_estimation, estimation_details FROM clients WHERE id = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        client.id = query.value(0).toInt();
        client.name = query.value(1).toString();
        client.project = query.value(2).toString();
        client.location = query.value(3).toString();
        client.postcode = query.value(4).toString();
        client.phone = query.value(5).toString();
        client.email = query.value(6).toString();
        client.serviceArea = query.value(7).toString();
        client.description = query.value(8).toString();
        client.latitude = query.value(9).toDouble();
        client.longitude = query.value(10).toDouble();
        client.costEstimation = query.value(11).toDouble();
        client.estimationDetails = query.value(12).toString();
    }
    return client;
}

QVector<Client> DatabaseManager::searchClients(const QString &searchText)
{
    QVector<Client> clients;
    
    // S'assurer que la base de données est ouverte avant d'effectuer des opérations
    if (!isConnected || !m_db.isOpen()) {
        if (!openDatabase()) {
            return clients;
        }
    }
    
    QSqlQuery query;
    query.prepare("SELECT id, name, project, location, postcode, phone, email, service_area, description "
                 "FROM clients WHERE "
                 "name LIKE :search OR "
                 "project LIKE :search OR "
                 "location LIKE :search OR "
                 "postcode LIKE :search OR "
                 "phone LIKE :search OR "
                 "email LIKE :search");
    
    query.bindValue(":search", "%" + searchText + "%");
    
    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.id = query.value(0).toInt();
            client.name = query.value(1).toString();
            client.project = query.value(2).toString();
            client.location = query.value(3).toString();
            client.postcode = query.value(4).toString();
            client.phone = query.value(5).toString();
            client.email = query.value(6).toString();
            client.serviceArea = query.value(7).toString();
            client.description = query.value(8).toString();
            
            clients.append(client);
        }
    }
    
    return clients;
}

int DatabaseManager::getLastInsertId()
{
    // S'assurer que la base de données est ouverte avant d'effectuer des opérations
    if (!isConnected || !m_db.isOpen()) {
        if (!openDatabase()) {
            return -1;
        }
    }
    
    QSqlQuery query("SELECT last_insert_rowid()");
    if (query.next()) {
        return query.value(0).toInt();
    }
    
    return -1;
}