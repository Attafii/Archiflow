#include "clientdatabasemanager.h"
#include "client.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QUuid>
#include <QDebug>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QtMath>

const QString ClientDatabaseManager::DATABASE_VERSION = "1.0";
const QString ClientDatabaseManager::CLIENTS_TABLE = "clients";

ClientDatabaseManager::ClientDatabaseManager(QObject *parent)
    : QObject(parent)
    , m_connectionName(generateConnectionName())
{
}

ClientDatabaseManager::~ClientDatabaseManager()
{
    close();
}

bool ClientDatabaseManager::initialize(const QString &databasePath)
{
    QMutexLocker locker(&m_mutex);
    
    // Setup database connection
    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    
    QString dbPath = databasePath;
    if (dbPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        dbPath = dataDir + "/clients.db";
    }
    
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        setLastError("Failed to open database: " + m_database.lastError().text());
        return false;
    }
    
    // Create tables if they don't exist
    if (!createTables()) {
        setLastError("Failed to create database tables");
        return false;
    }
    
    qDebug() << "ClientDatabaseManager: Database initialized at" << dbPath;
    return true;
}

bool ClientDatabaseManager::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_database.isOpen();
}

void ClientDatabaseManager::close()
{
    QMutexLocker locker(&m_mutex);
    if (m_database.isOpen()) {
        m_database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

QString ClientDatabaseManager::lastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

bool ClientDatabaseManager::createTables()
{
    return createClientsTable() && createIndexes();
}

bool ClientDatabaseManager::createClientsTable()
{
    QSqlQuery query(m_database);
    QString sql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            company_name TEXT,
            email TEXT,
            phone_number TEXT,
            address_street TEXT,
            address_city TEXT,
            address_state TEXT,
            address_zipcode TEXT,
            address_country TEXT,
            latitude REAL DEFAULT 0.0,
            longitude REAL DEFAULT 0.0,
            notes TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )").arg(CLIENTS_TABLE);
    
    if (!query.exec(sql)) {
        setLastError("Failed to create clients table: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool ClientDatabaseManager::createIndexes()
{
    QSqlQuery query(m_database);
    
    QStringList indexQueries = {
        QString("CREATE INDEX IF NOT EXISTS idx_clients_name ON %1(name)").arg(CLIENTS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_clients_email ON %1(email)").arg(CLIENTS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_clients_company ON %1(company_name)").arg(CLIENTS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_clients_city ON %1(address_city)").arg(CLIENTS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_clients_country ON %1(address_country)").arg(CLIENTS_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_clients_location ON %1(latitude, longitude)").arg(CLIENTS_TABLE)
    };
    
    for (const QString &sql : indexQueries) {
        if (!query.exec(sql)) {
            setLastError("Failed to create index: " + query.lastError().text());
            return false;
        }
    }
    
    return true;
}

// Client operations
bool ClientDatabaseManager::addClient(const ClientContact *client)
{
    if (!client) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString(R"(
        INSERT INTO %1 (id, name, company_name, email, phone_number, 
                       address_street, address_city, address_state, 
                       address_zipcode, address_country, latitude, longitude, 
                       notes, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )").arg(CLIENTS_TABLE));
    
    query.addBindValue(client->id());
    query.addBindValue(client->name());
    query.addBindValue(client->companyName());
    query.addBindValue(client->email());
    query.addBindValue(client->phoneNumber());
    query.addBindValue(client->addressStreet());
    query.addBindValue(client->addressCity());
    query.addBindValue(client->addressState());
    query.addBindValue(client->addressZipcode());
    query.addBindValue(client->addressCountry());
    query.addBindValue(client->latitude());
    query.addBindValue(client->longitude());
    query.addBindValue(client->notes());
    query.addBindValue(client->createdAt());
    query.addBindValue(client->updatedAt());
    
    if (!query.exec()) {
        setLastError("Failed to add client: " + query.lastError().text());
        return false;
    }
    
    emit clientAdded(client->id());
    emit dataChanged();
    return true;
}

bool ClientDatabaseManager::updateClient(const ClientContact *client)
{
    if (!client) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString(R"(
        UPDATE %1 SET name = ?, company_name = ?, email = ?, phone_number = ?,
                     address_street = ?, address_city = ?, address_state = ?,
                     address_zipcode = ?, address_country = ?, latitude = ?,
                     longitude = ?, notes = ?, updated_at = ?
        WHERE id = ?
    )").arg(CLIENTS_TABLE));
    
    query.addBindValue(client->name());
    query.addBindValue(client->companyName());
    query.addBindValue(client->email());
    query.addBindValue(client->phoneNumber());
    query.addBindValue(client->addressStreet());
    query.addBindValue(client->addressCity());
    query.addBindValue(client->addressState());
    query.addBindValue(client->addressZipcode());
    query.addBindValue(client->addressCountry());
    query.addBindValue(client->latitude());
    query.addBindValue(client->longitude());
    query.addBindValue(client->notes());
    query.addBindValue(client->updatedAt());
    query.addBindValue(client->id());
    
    if (!query.exec()) {
        setLastError("Failed to update client: " + query.lastError().text());
        return false;
    }
    
    emit clientUpdated(client->id());
    emit dataChanged();
    return true;
}

bool ClientDatabaseManager::deleteClient(const QString &clientId)
{
    if (clientId.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(CLIENTS_TABLE));
    query.addBindValue(clientId);
    
    if (!query.exec()) {
        setLastError("Failed to delete client: " + query.lastError().text());
        return false;
    }
    
    emit clientDeleted(clientId);
    emit dataChanged();
    return true;
}

ClientContact* ClientDatabaseManager::getClient(const QString &clientId)
{
    if (clientId.isEmpty()) return nullptr;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT * FROM %1 WHERE id = ?").arg(CLIENTS_TABLE));
    query.addBindValue(clientId);
    
    if (!query.exec() || !query.next()) {
        return nullptr;
    }
    
    return clientFromQuery(query);
}

QList<ClientContact*> ClientDatabaseManager::getAllClients()
{
    QMutexLocker locker(&m_mutex);
    QList<ClientContact*> clients;
    
    QSqlQuery query(m_database);    query.prepare(QString("SELECT * FROM %1 ORDER BY name").arg(CLIENTS_TABLE));
    
    if (!query.exec()) {
        setLastError("Failed to get clients: " + query.lastError().text());
        return clients;
    }
    
    while (query.next()) {
        if (ClientContact *client = clientFromQuery(query)) {
            clients.append(client);
        }
    }
    
    return clients;
}

QList<ClientContact*> ClientDatabaseManager::searchClients(const QString &searchTerm)
{
    QMutexLocker locker(&m_mutex);
    QList<ClientContact*> clients;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        SELECT * FROM %1 
        WHERE name LIKE ? OR company_name LIKE ? OR email LIKE ? 
           OR address_city LIKE ? OR address_country LIKE ?
        ORDER BY name
    )").arg(CLIENTS_TABLE));
    
    QString term = "%" + searchTerm + "%";
    query.addBindValue(term);
    query.addBindValue(term);
    query.addBindValue(term);
    query.addBindValue(term);
    query.addBindValue(term);
      if (!query.exec()) {
        setLastError("Failed to search clients: " + query.lastError().text());
        return clients;
    }
    
    while (query.next()) {
        if (ClientContact *client = clientFromQuery(query)) {
            clients.append(client);
        }
    }
    
    return clients;
}

bool ClientDatabaseManager::clientExists(const QString &clientId)
{
    if (clientId.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE id = ?").arg(CLIENTS_TABLE));
    query.addBindValue(clientId);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toInt() > 0;
}

bool ClientDatabaseManager::emailExists(const QString &email, const QString &excludeClientId)
{
    if (email.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    if (excludeClientId.isEmpty()) {
        query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE email = ?").arg(CLIENTS_TABLE));
        query.addBindValue(email);
    } else {
        query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE email = ? AND id != ?").arg(CLIENTS_TABLE));
        query.addBindValue(email);
        query.addBindValue(excludeClientId);
    }
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toInt() > 0;
}

QList<ClientContact*> ClientDatabaseManager::getClientsByLocation(double latitude, double longitude, double radius)
{
    QMutexLocker locker(&m_mutex);
    QList<ClientContact*> clients;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        SELECT * FROM %1 
        WHERE latitude != 0 OR longitude != 0
        ORDER BY name
    )").arg(CLIENTS_TABLE));
    
    if (!query.exec()) {
        setLastError("Failed to get clients by location: " + query.lastError().text());
        return clients;
    }
      while (query.next()) {
        if (ClientContact *client = clientFromQuery(query)) {
            // Calculate distance using Haversine formula
            double clientLat = client->latitude();
            double clientLng = client->longitude();
            
            double dLat = qDegreesToRadians(clientLat - latitude);
            double dLng = qDegreesToRadians(clientLng - longitude);
            
            double a = qSin(dLat/2) * qSin(dLat/2) + 
                      qCos(qDegreesToRadians(latitude)) * qCos(qDegreesToRadians(clientLat)) * 
                      qSin(dLng/2) * qSin(dLng/2);
            double c = 2 * qAtan2(qSqrt(a), qSqrt(1-a));
            double distance = 6371 * c; // Earth's radius in km
            
            if (distance <= radius) {
                clients.append(client);
            } else {
                delete client;
            }
        }
    }
    
    return clients;
}

QList<ClientContact*> ClientDatabaseManager::getClientsWithCoordinates()
{
    QMutexLocker locker(&m_mutex);
    QList<ClientContact*> clients;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        SELECT * FROM %1 
        WHERE (latitude != 0 OR longitude != 0)
        ORDER BY name
    )").arg(CLIENTS_TABLE));
    
    if (!query.exec()) {
        setLastError("Failed to get clients with coordinates: " + query.lastError().text());
        return clients;    }
    
    while (query.next()) {
        if (ClientContact *client = clientFromQuery(query)) {
            clients.append(client);
        }
    }
    
    return clients;
}

// Statistics
int ClientDatabaseManager::getTotalClientCount()
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1").arg(CLIENTS_TABLE));
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value(0).toInt();
}

int ClientDatabaseManager::getClientCountByCity(const QString &city)
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE address_city = ?").arg(CLIENTS_TABLE));
    query.addBindValue(city);
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value(0).toInt();
}

int ClientDatabaseManager::getClientCountByCountry(const QString &country)
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE address_country = ?").arg(CLIENTS_TABLE));
    query.addBindValue(country);
    
    if (!query.exec() || !query.next()) {
        return 0;
    }
    
    return query.value(0).toInt();
}

QStringList ClientDatabaseManager::getAllCities()
{
    QMutexLocker locker(&m_mutex);
    QStringList cities;
    
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT DISTINCT address_city FROM %1 WHERE address_city != '' ORDER BY address_city").arg(CLIENTS_TABLE));
    
    if (!query.exec()) {
        return cities;
    }
    
    while (query.next()) {
        cities << query.value(0).toString();
    }
    
    return cities;
}

QStringList ClientDatabaseManager::getAllCountries()
{
    QMutexLocker locker(&m_mutex);
    QStringList countries;
    
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT DISTINCT address_country FROM %1 WHERE address_country != '' ORDER BY address_country").arg(CLIENTS_TABLE));
    
    if (!query.exec()) {
        return countries;
    }
    
    while (query.next()) {
        countries << query.value(0).toString();
    }
    
    return countries;
}

// Transaction management
bool ClientDatabaseManager::beginTransaction()
{
    return m_database.transaction();
}

bool ClientDatabaseManager::commitTransaction()
{
    return m_database.commit();
}

bool ClientDatabaseManager::rollbackTransaction()
{
    return m_database.rollback();
}

bool ClientDatabaseManager::vacuum()
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    return query.exec("VACUUM");
}

bool ClientDatabaseManager::backup(const QString &backupPath)
{
    QMutexLocker locker(&m_mutex);
    return QFile::copy(m_database.databaseName(), backupPath);
}

bool ClientDatabaseManager::restore(const QString &backupPath)
{
    close();
    
    QMutexLocker locker(&m_mutex);
    QString currentPath = m_database.databaseName();
    
    if (QFile::exists(currentPath)) {
        QFile::remove(currentPath);
    }
    
    bool success = QFile::copy(backupPath, currentPath);
    
    if (success) {
        initialize(currentPath);
    }
    
    return success;
}

void ClientDatabaseManager::refresh()
{
    emit dataChanged();
}

// Helper methods
ClientContact* ClientDatabaseManager::clientFromQuery(const QSqlQuery &query)
{
    ClientContact *client = new ClientContact();
    
    client->setId(query.value("id").toString());
    client->setName(query.value("name").toString());
    client->setCompanyName(query.value("company_name").toString());
    client->setEmail(query.value("email").toString());
    client->setPhoneNumber(query.value("phone_number").toString());
    client->setAddressStreet(query.value("address_street").toString());
    client->setAddressCity(query.value("address_city").toString());
    client->setAddressState(query.value("address_state").toString());
    client->setAddressZipcode(query.value("address_zipcode").toString());
    client->setAddressCountry(query.value("address_country").toString());
    client->setLatitude(query.value("latitude").toDouble());
    client->setLongitude(query.value("longitude").toDouble());
    client->setNotes(query.value("notes").toString());
    client->setCreatedAt(query.value("created_at").toDateTime());
    client->setUpdatedAt(query.value("updated_at").toDateTime());
    
    return client;
}

void ClientDatabaseManager::setLastError(const QString &error)
{
    m_lastError = error;
    qWarning() << "ClientDatabaseManager Error:" << error;
    emit errorOccurred(error);
}

QString ClientDatabaseManager::generateConnectionName()
{
    return QString("ClientDB_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}
