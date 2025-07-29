#include "invoicedatabasemanager.h"
#include "invoice.h"
#include "invoiceitem.h"
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

const QString InvoiceDatabaseManager::DATABASE_VERSION = "1.0";
const QString InvoiceDatabaseManager::CLIENTS_TABLE = "invoice_clients";
const QString InvoiceDatabaseManager::INVOICES_TABLE = "invoices";
const QString InvoiceDatabaseManager::INVOICE_ITEMS_TABLE = "invoice_items";

InvoiceDatabaseManager::InvoiceDatabaseManager(QObject *parent)
    : QObject(parent)
    , m_connectionName(generateConnectionName())
{
}

InvoiceDatabaseManager::~InvoiceDatabaseManager()
{
    close();
}

bool InvoiceDatabaseManager::initialize(const QString &databasePath)
{
    QMutexLocker locker(&m_mutex);
    
    // Setup database connection
    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    
    QString dbPath = databasePath;
    if (dbPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        dbPath = dataDir + "/invoices.db";
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
    
    qDebug() << "InvoiceDatabaseManager: Database initialized at" << dbPath;
    return true;
}

bool InvoiceDatabaseManager::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_database.isOpen();
}

void InvoiceDatabaseManager::close()
{
    QMutexLocker locker(&m_mutex);
    if (m_database.isOpen()) {
        m_database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

QString InvoiceDatabaseManager::lastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

bool InvoiceDatabaseManager::createTables()
{
    return createClientsTable() && createInvoicesTable() && createInvoiceItemsTable() && createIndexes();
}

bool InvoiceDatabaseManager::createClientsTable()
{
    QSqlQuery query(m_database);
    QString sql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            company TEXT,
            address TEXT,
            email TEXT,
            phone TEXT,
            tax_id TEXT,
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

bool InvoiceDatabaseManager::createInvoicesTable()
{
    QSqlQuery query(m_database);
    QString sql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            id TEXT PRIMARY KEY,
            invoice_number TEXT UNIQUE NOT NULL,
            client_id TEXT NOT NULL,
            client_name TEXT NOT NULL,
            client_address TEXT,
            client_email TEXT,
            client_phone TEXT,
            invoice_date DATE NOT NULL,
            due_date DATE NOT NULL,
            subtotal REAL DEFAULT 0.0,
            tax_rate REAL DEFAULT 0.0,
            tax_amount REAL DEFAULT 0.0,
            total_amount REAL NOT NULL,
            status TEXT DEFAULT 'Draft',
            notes TEXT,
            currency TEXT DEFAULT 'USD',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (client_id) REFERENCES %2(id)
        )
    )").arg(INVOICES_TABLE).arg(CLIENTS_TABLE);
    
    if (!query.exec(sql)) {
        setLastError("Failed to create invoices table: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool InvoiceDatabaseManager::createInvoiceItemsTable()
{
    QSqlQuery query(m_database);
    QString sql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            id TEXT PRIMARY KEY,
            invoice_id TEXT NOT NULL,
            description TEXT NOT NULL,
            quantity INTEGER NOT NULL,
            unit TEXT DEFAULT 'pcs',
            unit_price REAL NOT NULL,
            total_price REAL NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (invoice_id) REFERENCES %2(id) ON DELETE CASCADE
        )
    )").arg(INVOICE_ITEMS_TABLE).arg(INVOICES_TABLE);
    
    if (!query.exec(sql)) {
        setLastError("Failed to create invoice items table: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool InvoiceDatabaseManager::createIndexes()
{
    QSqlQuery query(m_database);
    
    QStringList indexQueries = {
        QString("CREATE INDEX IF NOT EXISTS idx_invoices_client_id ON %1(client_id)").arg(INVOICES_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_invoices_status ON %1(status)").arg(INVOICES_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_invoices_date ON %1(invoice_date)").arg(INVOICES_TABLE),
        QString("CREATE INDEX IF NOT EXISTS idx_invoice_items_invoice_id ON %1(invoice_id)").arg(INVOICE_ITEMS_TABLE)
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
bool InvoiceDatabaseManager::addClient(const Client *client)
{
    if (!client) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString(R"(
        INSERT INTO %1 (id, name, company, address, email, phone, tax_id, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )").arg(CLIENTS_TABLE));
    
    query.addBindValue(client->id());
    query.addBindValue(client->name());
    query.addBindValue(client->company());
    query.addBindValue(client->address());
    query.addBindValue(client->email());
    query.addBindValue(client->phone());
    query.addBindValue(client->taxId());
    query.addBindValue(client->notes());
    
    if (!query.exec()) {
        setLastError("Failed to add client: " + query.lastError().text());
        return false;
    }
    
    emit clientAdded(client->id());
    emit dataChanged();
    return true;
}

bool InvoiceDatabaseManager::updateClient(const Client *client)
{
    if (!client) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString(R"(
        UPDATE %1 SET name = ?, company = ?, address = ?, email = ?, 
                     phone = ?, tax_id = ?, notes = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )").arg(CLIENTS_TABLE));
    
    query.addBindValue(client->name());
    query.addBindValue(client->company());
    query.addBindValue(client->address());
    query.addBindValue(client->email());
    query.addBindValue(client->phone());
    query.addBindValue(client->taxId());
    query.addBindValue(client->notes());
    query.addBindValue(client->id());
    
    if (!query.exec()) {
        setLastError("Failed to update client: " + query.lastError().text());
        return false;
    }
    
    emit clientUpdated(client->id());
    emit dataChanged();
    return true;
}

bool InvoiceDatabaseManager::deleteClient(const QString &clientId)
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

Client* InvoiceDatabaseManager::getClient(const QString &clientId)
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

QList<Client*> InvoiceDatabaseManager::getAllClients()
{
    QMutexLocker locker(&m_mutex);
    QList<Client*> clients;
    
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT * FROM %1 ORDER BY name").arg(CLIENTS_TABLE));
    
    if (!query.exec()) {
        setLastError("Failed to get clients: " + query.lastError().text());
        return clients;
    }
    
    while (query.next()) {
        if (Client *client = clientFromQuery(query)) {
            clients.append(client);
        }
    }
    
    return clients;
}

QList<Client*> InvoiceDatabaseManager::searchClients(const QString &searchTerm)
{
    QMutexLocker locker(&m_mutex);
    QList<Client*> clients;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        SELECT * FROM %1 
        WHERE name LIKE ? OR company LIKE ? OR email LIKE ? 
        ORDER BY name
    )").arg(CLIENTS_TABLE));
    
    QString term = "%" + searchTerm + "%";
    query.addBindValue(term);
    query.addBindValue(term);
    query.addBindValue(term);
    
    if (!query.exec()) {
        setLastError("Failed to search clients: " + query.lastError().text());
        return clients;
    }
    
    while (query.next()) {
        if (Client *client = clientFromQuery(query)) {
            clients.append(client);
        }
    }
    
    return clients;
}

bool InvoiceDatabaseManager::clientExists(const QString &clientId)
{
    if (clientId.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE id = ?").arg(CLIENTS_TABLE));
    query.addBindValue(clientId);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    
    return false;
}

// Invoice operations (basic implementations)
bool InvoiceDatabaseManager::addInvoice(const Invoice *invoice)
{
    if (!invoice) return false;
    
    QMutexLocker locker(&m_mutex);
    
    if (!beginTransaction()) return false;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        INSERT INTO %1 (id, invoice_number, client_id, client_name, client_address, 
                       client_email, client_phone, invoice_date, due_date, subtotal, 
                       tax_rate, tax_amount, total_amount, status, notes, currency)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )").arg(INVOICES_TABLE));
    
    query.addBindValue(invoice->id());
    query.addBindValue(invoice->invoiceNumber());
    query.addBindValue(invoice->clientId());
    query.addBindValue(invoice->clientName());
    query.addBindValue(invoice->clientAddress());
    query.addBindValue(invoice->clientEmail());
    query.addBindValue(invoice->clientPhone());
    query.addBindValue(invoice->invoiceDate());
    query.addBindValue(invoice->dueDate());
    query.addBindValue(invoice->subtotal());
    query.addBindValue(invoice->taxRate());
    query.addBindValue(invoice->taxAmount());
    query.addBindValue(invoice->totalAmount());
    query.addBindValue(invoice->status());
    query.addBindValue(invoice->notes());
    query.addBindValue(invoice->currency());
    
    if (!query.exec()) {
        rollbackTransaction();
        setLastError("Failed to add invoice: " + query.lastError().text());
        return false;
    }
    
    // Add invoice items
    for (const InvoiceItem *item : invoice->items()) {
        if (!addInvoiceItem(item)) {
            rollbackTransaction();
            return false;
        }
    }
    
    if (!commitTransaction()) return false;
    
    emit invoiceAdded(invoice->id());
    emit dataChanged();
    return true;
}

QList<Invoice*> InvoiceDatabaseManager::getAllInvoices()
{
    QMutexLocker locker(&m_mutex);
    QList<Invoice*> invoices;
    
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT * FROM %1 ORDER BY invoice_date DESC").arg(INVOICES_TABLE));
    
    if (!query.exec()) {
        setLastError("Failed to get invoices: " + query.lastError().text());
        return invoices;
    }
    
    while (query.next()) {
        if (Invoice *invoice = invoiceFromQuery(query)) {
            // Load items for this invoice
            QList<InvoiceItem*> items = getInvoiceItems(invoice->id());
            for (InvoiceItem *item : items) {
                invoice->addItem(item);
            }
            invoices.append(invoice);
        }
    }
    
    return invoices;
}

// Additional methods with basic implementations
bool InvoiceDatabaseManager::updateInvoice(const Invoice *invoice)
{
    if (!invoice) return false;
    
    QMutexLocker locker(&m_mutex);
    
    if (!beginTransaction()) {
        return false;
    }
    
    // Update invoice
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        UPDATE %1 SET invoice_number = ?, client_id = ?, client_name = ?, 
                     client_address = ?, client_email = ?, client_phone = ?,
                     invoice_date = ?, due_date = ?, subtotal = ?, tax_rate = ?,
                     tax_amount = ?, total_amount = ?, status = ?, notes = ?,
                     currency = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )").arg(INVOICES_TABLE));
    
    query.addBindValue(invoice->invoiceNumber());
    query.addBindValue(invoice->clientId());
    query.addBindValue(invoice->clientName());
    query.addBindValue(invoice->clientAddress());
    query.addBindValue(invoice->clientEmail());
    query.addBindValue(invoice->clientPhone());
    query.addBindValue(invoice->invoiceDate());
    query.addBindValue(invoice->dueDate());
    query.addBindValue(invoice->subtotal());
    query.addBindValue(invoice->taxRate());
    query.addBindValue(invoice->taxAmount());
    query.addBindValue(invoice->totalAmount());
    query.addBindValue(invoice->status());
    query.addBindValue(invoice->notes());
    query.addBindValue(invoice->currency());
    query.addBindValue(invoice->id());
    
    if (!query.exec()) {
        setLastError("Failed to update invoice: " + query.lastError().text());
        rollbackTransaction();
        return false;
    }
    
    // Delete existing items
    if (!deleteInvoiceItems(invoice->id())) {
        rollbackTransaction();
        return false;
    }
    
    // Add updated items
    for (const InvoiceItem *item : invoice->items()) {
        if (!addInvoiceItem(item)) {
            rollbackTransaction();
            return false;
        }
    }
    
    if (!commitTransaction()) {
        return false;
    }
    
    emit invoiceUpdated(invoice->id());
    emit dataChanged();
    return true;
}

bool InvoiceDatabaseManager::deleteInvoice(const QString &invoiceId)
{
    if (invoiceId.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    
    if (!beginTransaction()) {
        return false;
    }
    
    // Delete invoice items first (foreign key constraint)
    if (!deleteInvoiceItems(invoiceId)) {
        rollbackTransaction();
        return false;
    }
    
    // Delete invoice
    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(INVOICES_TABLE));
    query.addBindValue(invoiceId);
    
    if (!query.exec()) {
        setLastError("Failed to delete invoice: " + query.lastError().text());
        rollbackTransaction();
        return false;
    }
    
    if (!commitTransaction()) {
        return false;
    }
    
    emit invoiceDeleted(invoiceId);
    emit dataChanged();
    return true;
}
Invoice* InvoiceDatabaseManager::getInvoice(const QString &invoiceId)
{
    if (invoiceId.isEmpty()) return nullptr;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT * FROM %1 WHERE id = ?").arg(INVOICES_TABLE));
    query.addBindValue(invoiceId);
    
    if (!query.exec() || !query.next()) {
        return nullptr;
    }
    
    Invoice *invoice = invoiceFromQuery(query);
    if (invoice) {
        // Load invoice items
        QList<InvoiceItem*> items = getInvoiceItems(invoiceId);
        for (InvoiceItem *item : items) {
            invoice->addItem(item);
        }
    }
    
    return invoice;
}
QList<Invoice*> InvoiceDatabaseManager::getInvoicesByClient(const QString &clientId)
{
    QMutexLocker locker(&m_mutex);
    QList<Invoice*> invoices;
    
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT * FROM %1 WHERE client_id = ? ORDER BY invoice_date DESC").arg(INVOICES_TABLE));
    query.addBindValue(clientId);
    
    if (!query.exec()) {
        setLastError("Failed to get invoices by client: " + query.lastError().text());
        return invoices;
    }
    
    while (query.next()) {
        if (Invoice *invoice = invoiceFromQuery(query)) {
            // Load invoice items
            QList<InvoiceItem*> items = getInvoiceItems(invoice->id());
            for (InvoiceItem *item : items) {
                invoice->addItem(item);
            }
            invoices.append(invoice);
        }
    }
    
    return invoices;
}

QList<Invoice*> InvoiceDatabaseManager::getInvoicesByStatus(const QString &status)
{
    QMutexLocker locker(&m_mutex);
    QList<Invoice*> invoices;
    
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT * FROM %1 WHERE status = ? ORDER BY invoice_date DESC").arg(INVOICES_TABLE));
    query.addBindValue(status);
    
    if (!query.exec()) {
        setLastError("Failed to get invoices by status: " + query.lastError().text());
        return invoices;
    }
    
    while (query.next()) {
        if (Invoice *invoice = invoiceFromQuery(query)) {
            // Load invoice items
            QList<InvoiceItem*> items = getInvoiceItems(invoice->id());
            for (InvoiceItem *item : items) {
                invoice->addItem(item);
            }
            invoices.append(invoice);
        }
    }
    
    return invoices;
}

QList<Invoice*> InvoiceDatabaseManager::getInvoicesByDateRange(const QDate &startDate, const QDate &endDate)
{
    QMutexLocker locker(&m_mutex);
    QList<Invoice*> invoices;
    
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT * FROM %1 WHERE invoice_date BETWEEN ? AND ? ORDER BY invoice_date DESC").arg(INVOICES_TABLE));
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    
    if (!query.exec()) {
        setLastError("Failed to get invoices by date range: " + query.lastError().text());
        return invoices;
    }
    
    while (query.next()) {
        if (Invoice *invoice = invoiceFromQuery(query)) {
            // Load invoice items
            QList<InvoiceItem*> items = getInvoiceItems(invoice->id());
            for (InvoiceItem *item : items) {
                invoice->addItem(item);
            }
            invoices.append(invoice);
        }
    }
    
    return invoices;
}

QList<Invoice*> InvoiceDatabaseManager::searchInvoices(const QString &searchTerm)
{
    QMutexLocker locker(&m_mutex);
    QList<Invoice*> invoices;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        SELECT * FROM %1 
        WHERE invoice_number LIKE ? OR client_name LIKE ? OR notes LIKE ?
        ORDER BY invoice_date DESC
    )").arg(INVOICES_TABLE));
    
    QString term = "%" + searchTerm + "%";
    query.addBindValue(term);
    query.addBindValue(term);
    query.addBindValue(term);
    
    if (!query.exec()) {
        setLastError("Failed to search invoices: " + query.lastError().text());
        return invoices;
    }
    
    while (query.next()) {
        if (Invoice *invoice = invoiceFromQuery(query)) {
            // Load invoice items
            QList<InvoiceItem*> items = getInvoiceItems(invoice->id());
            for (InvoiceItem *item : items) {
                invoice->addItem(item);
            }
            invoices.append(invoice);
        }
    }
    
    return invoices;
}

bool InvoiceDatabaseManager::invoiceExists(const QString &invoiceId)
{
    if (invoiceId.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE id = ?").arg(INVOICES_TABLE));
    query.addBindValue(invoiceId);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toInt() > 0;
}

bool InvoiceDatabaseManager::invoiceNumberExists(const QString &invoiceNumber)
{
    if (invoiceNumber.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE invoice_number = ?").arg(INVOICES_TABLE));
    query.addBindValue(invoiceNumber);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toInt() > 0;
}

bool InvoiceDatabaseManager::addInvoiceItem(const InvoiceItem *item)
{
    if (!item) return false;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        INSERT INTO %1 (id, invoice_id, description, quantity, unit, unit_price, total_price)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )").arg(INVOICE_ITEMS_TABLE));
    
    query.addBindValue(item->id());
    query.addBindValue(item->invoiceId());
    query.addBindValue(item->description());
    query.addBindValue(item->quantity());
    query.addBindValue(item->unit());
    query.addBindValue(item->unitPrice());
    query.addBindValue(item->totalPrice());
    
    if (!query.exec()) {
        setLastError("Failed to add invoice item: " + query.lastError().text());
        return false;
    }
    
    return true;
}

QList<InvoiceItem*> InvoiceDatabaseManager::getInvoiceItems(const QString &invoiceId)
{
    QList<InvoiceItem*> items;
    
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT * FROM %1 WHERE invoice_id = ? ORDER BY id").arg(INVOICE_ITEMS_TABLE));
    query.addBindValue(invoiceId);
    
    if (!query.exec()) {
        setLastError("Failed to get invoice items: " + query.lastError().text());
        return items;
    }
    
    while (query.next()) {
        if (InvoiceItem *item = invoiceItemFromQuery(query)) {
            items.append(item);
        }
    }
    
    return items;
}

// Statistics
double InvoiceDatabaseManager::getTotalRevenue()
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT SUM(total_amount) FROM %1 WHERE status = 'Paid'").arg(INVOICES_TABLE));
    
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}

int InvoiceDatabaseManager::getTotalInvoiceCount()
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1").arg(INVOICES_TABLE));
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

int InvoiceDatabaseManager::getInvoiceCountByStatus(const QString &status)
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE status = ?").arg(INVOICES_TABLE));
    query.addBindValue(status);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QList<Invoice*> InvoiceDatabaseManager::getOverdueInvoices()
{
    QMutexLocker locker(&m_mutex);
    QList<Invoice*> invoices;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        SELECT * FROM %1 
        WHERE status != 'Paid' AND due_date < date('now')
        ORDER BY due_date
    )").arg(INVOICES_TABLE));
    
    if (!query.exec()) {
        setLastError("Failed to get overdue invoices: " + query.lastError().text());
        return invoices;
    }
    
    while (query.next()) {
        if (Invoice *invoice = invoiceFromQuery(query)) {
            invoices.append(invoice);
        }
    }
    
    return invoices;
}

QList<Invoice*> InvoiceDatabaseManager::getInvoicesDueSoon(int daysThreshold)
{
    QMutexLocker locker(&m_mutex);
    QList<Invoice*> invoices;
    
    QSqlQuery query(m_database);
    query.prepare(QString(R"(
        SELECT * FROM %1 
        WHERE status != 'Paid' 
        AND due_date BETWEEN date('now') AND date('now', '+%2 days')
        ORDER BY due_date
    )").arg(INVOICES_TABLE).arg(daysThreshold));
    
    if (!query.exec()) {
        setLastError("Failed to get invoices due soon: " + query.lastError().text());
        return invoices;
    }
    
    while (query.next()) {
        if (Invoice *invoice = invoiceFromQuery(query)) {
            invoices.append(invoice);
        }
    }
    
    return invoices;
}

// Transaction management
bool InvoiceDatabaseManager::beginTransaction()
{
    return m_database.transaction();
}

bool InvoiceDatabaseManager::commitTransaction()
{
    return m_database.commit();
}

bool InvoiceDatabaseManager::rollbackTransaction()
{
    return m_database.rollback();
}

// Helper methods
Client* InvoiceDatabaseManager::clientFromQuery(const QSqlQuery &query)
{
    Client *client = new Client();
    
    client->setId(query.value("id").toString());
    client->setName(query.value("name").toString());
    client->setCompany(query.value("company").toString());
    client->setAddress(query.value("address").toString());
    client->setEmail(query.value("email").toString());
    client->setPhone(query.value("phone").toString());
    client->setTaxId(query.value("tax_id").toString());
    client->setNotes(query.value("notes").toString());
    
    return client;
}

Invoice* InvoiceDatabaseManager::invoiceFromQuery(const QSqlQuery &query)
{
    Invoice *invoice = new Invoice();
    
    invoice->setId(query.value("id").toString());
    invoice->setInvoiceNumber(query.value("invoice_number").toString());
    invoice->setClientId(query.value("client_id").toString());
    invoice->setClientName(query.value("client_name").toString());
    invoice->setClientAddress(query.value("client_address").toString());
    invoice->setClientEmail(query.value("client_email").toString());
    invoice->setClientPhone(query.value("client_phone").toString());
    invoice->setInvoiceDate(query.value("invoice_date").toDate());
    invoice->setDueDate(query.value("due_date").toDate());
    invoice->setSubtotal(query.value("subtotal").toDouble());
    invoice->setTaxRate(query.value("tax_rate").toDouble());
    invoice->setTaxAmount(query.value("tax_amount").toDouble());
    invoice->setTotalAmount(query.value("total_amount").toDouble());
    invoice->setStatus(query.value("status").toString());
    invoice->setNotes(query.value("notes").toString());
    invoice->setCurrency(query.value("currency").toString());
    
    return invoice;
}

InvoiceItem* InvoiceDatabaseManager::invoiceItemFromQuery(const QSqlQuery &query)
{
    InvoiceItem *item = new InvoiceItem();
    
    item->setId(query.value("id").toString());
    item->setInvoiceId(query.value("invoice_id").toString());
    item->setDescription(query.value("description").toString());
    item->setQuantity(query.value("quantity").toInt());
    item->setUnit(query.value("unit").toString());
    item->setUnitPrice(query.value("unit_price").toDouble());
    item->setTotalPrice(query.value("total_price").toDouble());
    
    return item;
}

void InvoiceDatabaseManager::setLastError(const QString &error)
{
    m_lastError = error;
    qWarning() << "InvoiceDatabaseManager Error:" << error;
    emit errorOccurred(error);
}

QString InvoiceDatabaseManager::generateConnectionName()
{
    return QString("InvoiceDB_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

// Additional placeholder implementations
double InvoiceDatabaseManager::getTotalRevenueByPeriod(const QDate &startDate, const QDate &endDate) { Q_UNUSED(startDate); Q_UNUSED(endDate); return 0.0; }
double InvoiceDatabaseManager::getTotalRevenueByClient(const QString &clientId) { Q_UNUSED(clientId); return 0.0; }
bool InvoiceDatabaseManager::updateInvoiceItem(const InvoiceItem *item)
{
    if (!item) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString(R"(
        UPDATE %1 SET description = ?, quantity = ?, unit = ?, 
                     unit_price = ?, total_price = ?
        WHERE id = ?
    )").arg(INVOICE_ITEMS_TABLE));
    
    query.addBindValue(item->description());
    query.addBindValue(item->quantity());
    query.addBindValue(item->unit());
    query.addBindValue(item->unitPrice());
    query.addBindValue(item->totalPrice());
    query.addBindValue(item->id());
    
    if (!query.exec()) {
        setLastError("Failed to update invoice item: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool InvoiceDatabaseManager::deleteInvoiceItem(const QString &itemId)
{
    if (itemId.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(INVOICE_ITEMS_TABLE));
    query.addBindValue(itemId);
    
    if (!query.exec()) {
        setLastError("Failed to delete invoice item: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool InvoiceDatabaseManager::deleteInvoiceItems(const QString &invoiceId)
{
    if (invoiceId.isEmpty()) return false;
    
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    
    query.prepare(QString("DELETE FROM %1 WHERE invoice_id = ?").arg(INVOICE_ITEMS_TABLE));
    query.addBindValue(invoiceId);
    
    if (!query.exec()) {
        setLastError("Failed to delete invoice items: " + query.lastError().text());
        return false;
    }
    
    return true;
}
bool InvoiceDatabaseManager::vacuum() { return false; }
bool InvoiceDatabaseManager::backup(const QString &backupPath) { Q_UNUSED(backupPath); return false; }
bool InvoiceDatabaseManager::restore(const QString &backupPath) { Q_UNUSED(backupPath); return false; }

void InvoiceDatabaseManager::refresh()
{
    emit dataChanged();
}
