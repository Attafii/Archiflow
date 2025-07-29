#ifndef INVOICEDATABASEMANAGER_H
#define INVOICEDATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QMutex>

class Invoice;
class InvoiceItem;
class Client;

/**
 * @brief The InvoiceDatabaseManager class handles all database operations for invoices
 * 
 * This class provides a complete database abstraction layer for invoice management
 * including CRUD operations, search, and reporting functionality.
 */
class InvoiceDatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit InvoiceDatabaseManager(QObject *parent = nullptr);
    ~InvoiceDatabaseManager();

    // Database connection
    bool initialize(const QString &databasePath = QString());
    bool isConnected() const;
    void close();
    QString lastError() const;

    // Client operations
    bool addClient(const Client *client);
    bool updateClient(const Client *client);
    bool deleteClient(const QString &clientId);
    Client* getClient(const QString &clientId);
    QList<Client*> getAllClients();
    QList<Client*> searchClients(const QString &searchTerm);
    bool clientExists(const QString &clientId);

    // Invoice operations
    bool addInvoice(const Invoice *invoice);
    bool updateInvoice(const Invoice *invoice);
    bool deleteInvoice(const QString &invoiceId);
    Invoice* getInvoice(const QString &invoiceId);
    QList<Invoice*> getAllInvoices();
    QList<Invoice*> getInvoicesByClient(const QString &clientId);
    QList<Invoice*> getInvoicesByStatus(const QString &status);
    QList<Invoice*> getInvoicesByDateRange(const QDate &startDate, const QDate &endDate);
    QList<Invoice*> searchInvoices(const QString &searchTerm);
    bool invoiceExists(const QString &invoiceId);
    bool invoiceNumberExists(const QString &invoiceNumber);

    // Invoice item operations
    bool addInvoiceItem(const InvoiceItem *item);
    bool updateInvoiceItem(const InvoiceItem *item);
    bool deleteInvoiceItem(const QString &itemId);
    QList<InvoiceItem*> getInvoiceItems(const QString &invoiceId);
    bool deleteInvoiceItems(const QString &invoiceId);

    // Statistics and reporting
    double getTotalRevenue();
    double getTotalRevenueByPeriod(const QDate &startDate, const QDate &endDate);
    double getTotalRevenueByClient(const QString &clientId);
    int getTotalInvoiceCount();
    int getInvoiceCountByStatus(const QString &status);
    QList<Invoice*> getOverdueInvoices();
    QList<Invoice*> getInvoicesDueSoon(int daysThreshold = 7);
    
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
    void invoiceAdded(const QString &invoiceId);
    void invoiceUpdated(const QString &invoiceId);
    void invoiceDeleted(const QString &invoiceId);
    void errorOccurred(const QString &error);

private:
    bool createTables();
    bool createClientsTable();
    bool createInvoicesTable();
    bool createInvoiceItemsTable();
    bool createIndexes();
    
    Client* clientFromQuery(const QSqlQuery &query);
    Invoice* invoiceFromQuery(const QSqlQuery &query);
    InvoiceItem* invoiceItemFromQuery(const QSqlQuery &query);
    
    void setLastError(const QString &error);
    QString generateConnectionName();

    QSqlDatabase m_database;
    QString m_connectionName;
    QString m_lastError;
    mutable QMutex m_mutex;
    
    static const QString DATABASE_VERSION;
    static const QString CLIENTS_TABLE;
    static const QString INVOICES_TABLE;
    static const QString INVOICE_ITEMS_TABLE;
};

#endif // INVOICEDATABASEMANAGER_H
