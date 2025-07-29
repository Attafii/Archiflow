#ifndef INVOICE_H
#define INVOICE_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QJsonObject>
#include <QList>

class InvoiceItem;

/**
 * @brief The Invoice class represents an invoice in the ArchiFlow system
 * 
 * This class encapsulates all invoice-related data and provides
 * validation and conversion methods for the invoice management feature.
 */
class Invoice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString invoiceNumber READ invoiceNumber WRITE setInvoiceNumber NOTIFY invoiceNumberChanged)
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged)
    Q_PROPERTY(QString clientName READ clientName WRITE setClientName NOTIFY clientNameChanged)
    Q_PROPERTY(QString clientAddress READ clientAddress WRITE setClientAddress NOTIFY clientAddressChanged)
    Q_PROPERTY(QString clientEmail READ clientEmail WRITE setClientEmail NOTIFY clientEmailChanged)
    Q_PROPERTY(QString clientPhone READ clientPhone WRITE setClientPhone NOTIFY clientPhoneChanged)
    Q_PROPERTY(QDate invoiceDate READ invoiceDate WRITE setInvoiceDate NOTIFY invoiceDateChanged)
    Q_PROPERTY(QDate dueDate READ dueDate WRITE setDueDate NOTIFY dueDateChanged)
    Q_PROPERTY(double subtotal READ subtotal WRITE setSubtotal NOTIFY subtotalChanged)
    Q_PROPERTY(double taxRate READ taxRate WRITE setTaxRate NOTIFY taxRateChanged)
    Q_PROPERTY(double taxAmount READ taxAmount WRITE setTaxAmount NOTIFY taxAmountChanged)
    Q_PROPERTY(double totalAmount READ totalAmount WRITE setTotalAmount NOTIFY totalAmountChanged)
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)
    Q_PROPERTY(QString currency READ currency WRITE setCurrency NOTIFY currencyChanged)

public:
    enum Status {
        Draft,
        Sent,
        Paid,
        Overdue,
        Cancelled
    };
    Q_ENUM(Status)

    explicit Invoice(QObject *parent = nullptr);
    Invoice(const Invoice &other, QObject *parent = nullptr);
    Invoice(const QString &id, const QString &invoiceNumber, const QString &clientId,
             const QString &clientName, const QDate &invoiceDate, const QDate &dueDate,
             double totalAmount, const QString &status = "Draft", QObject *parent = nullptr);

    // Getters
    QString id() const { return m_id; }
    QString invoiceNumber() const { return m_invoiceNumber; }
    QString clientId() const { return m_clientId; }
    QString clientName() const { return m_clientName; }
    QString clientAddress() const { return m_clientAddress; }
    QString clientEmail() const { return m_clientEmail; }
    QString clientPhone() const { return m_clientPhone; }
    QDate invoiceDate() const { return m_invoiceDate; }
    QDate dueDate() const { return m_dueDate; }
    double subtotal() const { return m_subtotal; }
    double taxRate() const { return m_taxRate; }
    double taxAmount() const { return m_taxAmount; }
    double totalAmount() const { return m_totalAmount; }
    QString status() const { return m_status; }
    QString notes() const { return m_notes; }
    QString currency() const { return m_currency; }
    
    // Invoice Items
    QList<InvoiceItem*> items() const { return m_items; }
    void addItem(InvoiceItem *item);
    void removeItem(InvoiceItem *item);
    void clearItems();
    int itemCount() const { return m_items.count(); }

    // Setters
    void setId(const QString &id);
    void setInvoiceNumber(const QString &invoiceNumber);
    void setClientId(const QString &clientId);
    void setClientName(const QString &clientName);
    void setClientAddress(const QString &clientAddress);
    void setClientEmail(const QString &clientEmail);
    void setClientPhone(const QString &clientPhone);
    void setInvoiceDate(const QDate &invoiceDate);
    void setDueDate(const QDate &dueDate);
    void setSubtotal(double subtotal);
    void setTaxRate(double taxRate);
    void setTaxAmount(double taxAmount);
    void setTotalAmount(double totalAmount);
    void setStatus(const QString &status);
    void setNotes(const QString &notes);
    void setCurrency(const QString &currency);

    // Calculations
    void calculateTotals();
    double calculateSubtotal() const;
    double calculateTaxAmount() const;
    double calculateTotal() const;

    // Validation
    bool isValid() const;
    QStringList validationErrors() const;

    // Utility methods
    bool isOverdue() const;
    bool isDueSoon(int daysThreshold = 7) const;
    int daysUntilDue() const;
    int daysOverdue() const;
    QString statusDisplayText() const;
    QString formatCurrency(double amount) const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

    // Static utilities
    static QStringList availableStatuses();
    static QString statusToString(Status status);
    static Status stringToStatus(const QString &statusString);
    static QString generateInvoiceNumber();
    static QStringList availableCurrencies();

signals:
    void idChanged();
    void invoiceNumberChanged();
    void clientIdChanged();
    void clientNameChanged();
    void clientAddressChanged();
    void clientEmailChanged();
    void clientPhoneChanged();
    void invoiceDateChanged();
    void dueDateChanged();
    void subtotalChanged();
    void taxRateChanged();
    void taxAmountChanged();
    void totalAmountChanged();
    void statusChanged();
    void notesChanged();
    void currencyChanged();
    void itemsChanged();

private:
    QString m_id;
    QString m_invoiceNumber;
    QString m_clientId;
    QString m_clientName;
    QString m_clientAddress;
    QString m_clientEmail;
    QString m_clientPhone;
    QDate m_invoiceDate;
    QDate m_dueDate;
    double m_subtotal;
    double m_taxRate;
    double m_taxAmount;
    double m_totalAmount;
    QString m_status;
    QString m_notes;
    QString m_currency;
    QList<InvoiceItem*> m_items;
};

// Type alias for invoice lists
using InvoiceList = QList<Invoice*>;

#endif // INVOICE_H
