#ifndef INVOICEITEM_H
#define INVOICEITEM_H

#include <QObject>
#include <QString>
#include <QJsonObject>

/**
 * @brief The InvoiceItem class represents a line item in an invoice
 * 
 * This class encapsulates individual invoice line items with description,
 * quantity, unit price, and automatic total calculation.
 */
class InvoiceItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString invoiceId READ invoiceId WRITE setInvoiceId NOTIFY invoiceIdChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(int quantity READ quantity WRITE setQuantity NOTIFY quantityChanged)
    Q_PROPERTY(double unitPrice READ unitPrice WRITE setUnitPrice NOTIFY unitPriceChanged)
    Q_PROPERTY(double totalPrice READ totalPrice WRITE setTotalPrice NOTIFY totalPriceChanged)
    Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)

public:
    explicit InvoiceItem(QObject *parent = nullptr);
    InvoiceItem(const InvoiceItem &other, QObject *parent = nullptr);
    InvoiceItem(const QString &description, int quantity, double unitPrice, 
                const QString &unit = "pcs", QObject *parent = nullptr);

    // Getters
    QString id() const { return m_id; }
    QString invoiceId() const { return m_invoiceId; }
    QString description() const { return m_description; }
    int quantity() const { return m_quantity; }
    double unitPrice() const { return m_unitPrice; }
    double totalPrice() const { return m_totalPrice; }
    QString unit() const { return m_unit; }

    // Setters
    void setId(const QString &id);
    void setInvoiceId(const QString &invoiceId);
    void setDescription(const QString &description);
    void setQuantity(int quantity);
    void setUnitPrice(double unitPrice);
    void setTotalPrice(double totalPrice);
    void setUnit(const QString &unit);

    // Calculations
    void calculateTotal();
    double calculateTotalPrice() const;

    // Validation
    bool isValid() const;
    QStringList validationErrors() const;

    // Utility methods
    QString formatUnitPrice() const;
    QString formatTotalPrice() const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

    // Static utilities
    static QStringList availableUnits();

signals:
    void idChanged();
    void invoiceIdChanged();
    void descriptionChanged();
    void quantityChanged();
    void unitPriceChanged();
    void totalPriceChanged();
    void unitChanged();

private:
    QString m_id;
    QString m_invoiceId;
    QString m_description;
    int m_quantity;
    double m_unitPrice;
    double m_totalPrice;
    QString m_unit;
};

#endif // INVOICEITEM_H
