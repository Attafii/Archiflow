#include "invoice.h"
#include "invoiceitem.h"
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>

Invoice::Invoice(QObject *parent)
    : QObject(parent)
    , m_subtotal(0.0)
    , m_taxRate(0.0)
    , m_taxAmount(0.0)
    , m_totalAmount(0.0)
    , m_status("Draft")
    , m_currency("USD")
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_invoiceNumber = generateInvoiceNumber();
    m_invoiceDate = QDate::currentDate();
    m_dueDate = QDate::currentDate().addDays(30);
}

Invoice::Invoice(const Invoice &other, QObject *parent)
    : QObject(parent)
    , m_id(other.m_id)
    , m_invoiceNumber(other.m_invoiceNumber)
    , m_clientId(other.m_clientId)
    , m_clientName(other.m_clientName)
    , m_clientAddress(other.m_clientAddress)
    , m_clientEmail(other.m_clientEmail)
    , m_clientPhone(other.m_clientPhone)
    , m_invoiceDate(other.m_invoiceDate)
    , m_dueDate(other.m_dueDate)
    , m_subtotal(other.m_subtotal)
    , m_taxRate(other.m_taxRate)
    , m_taxAmount(other.m_taxAmount)
    , m_totalAmount(other.m_totalAmount)
    , m_status(other.m_status)
    , m_notes(other.m_notes)
    , m_currency(other.m_currency)
{
    // Deep copy items
    for (InvoiceItem *item : other.m_items) {
        InvoiceItem *newItem = new InvoiceItem(*item, this);
        m_items.append(newItem);
    }
}

Invoice::Invoice(const QString &id, const QString &invoiceNumber, const QString &clientId,
                 const QString &clientName, const QDate &invoiceDate, const QDate &dueDate,
                 double totalAmount, const QString &status, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_invoiceNumber(invoiceNumber)
    , m_clientId(clientId)
    , m_clientName(clientName)
    , m_invoiceDate(invoiceDate)
    , m_dueDate(dueDate)
    , m_totalAmount(totalAmount)
    , m_status(status)
    , m_subtotal(0.0)
    , m_taxRate(0.0)
    , m_taxAmount(0.0)
    , m_currency("USD")
{
}

void Invoice::addItem(InvoiceItem *item)
{
    if (item && !m_items.contains(item)) {
        item->setInvoiceId(m_id);
        m_items.append(item);
        connect(item, &InvoiceItem::quantityChanged, this, &Invoice::calculateTotals);
        connect(item, &InvoiceItem::unitPriceChanged, this, &Invoice::calculateTotals);
        calculateTotals();
        emit itemsChanged();
    }
}

void Invoice::removeItem(InvoiceItem *item)
{
    if (m_items.removeOne(item)) {
        item->deleteLater();
        calculateTotals();
        emit itemsChanged();
    }
}

void Invoice::clearItems()
{
    qDeleteAll(m_items);
    m_items.clear();
    calculateTotals();
    emit itemsChanged();
}

void Invoice::setId(const QString &id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged();
    }
}

void Invoice::setInvoiceNumber(const QString &invoiceNumber)
{
    if (m_invoiceNumber != invoiceNumber) {
        m_invoiceNumber = invoiceNumber;
        emit invoiceNumberChanged();
    }
}

void Invoice::setClientId(const QString &clientId)
{
    if (m_clientId != clientId) {
        m_clientId = clientId;
        emit clientIdChanged();
    }
}

void Invoice::setClientName(const QString &clientName)
{
    if (m_clientName != clientName) {
        m_clientName = clientName;
        emit clientNameChanged();
    }
}

void Invoice::setClientAddress(const QString &clientAddress)
{
    if (m_clientAddress != clientAddress) {
        m_clientAddress = clientAddress;
        emit clientAddressChanged();
    }
}

void Invoice::setClientEmail(const QString &clientEmail)
{
    if (m_clientEmail != clientEmail) {
        m_clientEmail = clientEmail;
        emit clientEmailChanged();
    }
}

void Invoice::setClientPhone(const QString &clientPhone)
{
    if (m_clientPhone != clientPhone) {
        m_clientPhone = clientPhone;
        emit clientPhoneChanged();
    }
}

void Invoice::setInvoiceDate(const QDate &invoiceDate)
{
    if (m_invoiceDate != invoiceDate) {
        m_invoiceDate = invoiceDate;
        emit invoiceDateChanged();
    }
}

void Invoice::setDueDate(const QDate &dueDate)
{
    if (m_dueDate != dueDate) {
        m_dueDate = dueDate;
        emit dueDateChanged();
    }
}

void Invoice::setSubtotal(double subtotal)
{
    if (qAbs(m_subtotal - subtotal) > 0.01) {
        m_subtotal = subtotal;
        emit subtotalChanged();
    }
}

void Invoice::setTaxRate(double taxRate)
{
    if (qAbs(m_taxRate - taxRate) > 0.01) {
        m_taxRate = taxRate;
        calculateTotals();
        emit taxRateChanged();
    }
}

void Invoice::setTaxAmount(double taxAmount)
{
    if (qAbs(m_taxAmount - taxAmount) > 0.01) {
        m_taxAmount = taxAmount;
        emit taxAmountChanged();
    }
}

void Invoice::setTotalAmount(double totalAmount)
{
    if (qAbs(m_totalAmount - totalAmount) > 0.01) {
        m_totalAmount = totalAmount;
        emit totalAmountChanged();
    }
}

void Invoice::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void Invoice::setNotes(const QString &notes)
{
    if (m_notes != notes) {
        m_notes = notes;
        emit notesChanged();
    }
}

void Invoice::setCurrency(const QString &currency)
{
    if (m_currency != currency) {
        m_currency = currency;
        emit currencyChanged();
    }
}

void Invoice::calculateTotals()
{
    double newSubtotal = calculateSubtotal();
    double newTaxAmount = calculateTaxAmount();
    double newTotal = calculateTotal();
    
    setSubtotal(newSubtotal);
    setTaxAmount(newTaxAmount);
    setTotalAmount(newTotal);
}

double Invoice::calculateSubtotal() const
{
    double subtotal = 0.0;
    for (const InvoiceItem *item : m_items) {
        subtotal += item->totalPrice();
    }
    return subtotal;
}

double Invoice::calculateTaxAmount() const
{
    return calculateSubtotal() * (m_taxRate / 100.0);
}

double Invoice::calculateTotal() const
{
    return calculateSubtotal() + calculateTaxAmount();
}

bool Invoice::isValid() const
{
    return !m_invoiceNumber.isEmpty() && 
           !m_clientName.isEmpty() && 
           m_invoiceDate.isValid() && 
           m_dueDate.isValid() &&
           m_dueDate >= m_invoiceDate;
}

QStringList Invoice::validationErrors() const
{
    QStringList errors;
    
    if (m_invoiceNumber.isEmpty()) {
        errors << "Invoice number is required";
    }
    
    if (m_clientName.isEmpty()) {
        errors << "Client name is required";
    }
    
    if (!m_invoiceDate.isValid()) {
        errors << "Invoice date is invalid";
    }
    
    if (!m_dueDate.isValid()) {
        errors << "Due date is invalid";
    }
    
    if (m_dueDate < m_invoiceDate) {
        errors << "Due date cannot be before invoice date";
    }
    
    if (m_items.isEmpty()) {
        errors << "Invoice must have at least one item";
    }
    
    return errors;
}

bool Invoice::isOverdue() const
{
    return m_status != "Paid" && m_dueDate < QDate::currentDate();
}

bool Invoice::isDueSoon(int daysThreshold) const
{
    return m_status != "Paid" && 
           m_dueDate >= QDate::currentDate() && 
           m_dueDate <= QDate::currentDate().addDays(daysThreshold);
}

int Invoice::daysUntilDue() const
{
    return QDate::currentDate().daysTo(m_dueDate);
}

int Invoice::daysOverdue() const
{
    if (!isOverdue()) return 0;
    return m_dueDate.daysTo(QDate::currentDate());
}

QString Invoice::statusDisplayText() const
{
    if (m_status == "Draft") return "Draft";
    if (m_status == "Sent") return "Sent";
    if (m_status == "Paid") return "Paid";
    if (m_status == "Overdue") return "Overdue";
    if (m_status == "Cancelled") return "Cancelled";
    return m_status;
}

QString Invoice::formatCurrency(double amount) const
{
    return QString("%1 %2").arg(amount, 0, 'f', 2).arg(m_currency);
}

QJsonObject Invoice::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["invoiceNumber"] = m_invoiceNumber;
    json["clientId"] = m_clientId;
    json["clientName"] = m_clientName;
    json["clientAddress"] = m_clientAddress;
    json["clientEmail"] = m_clientEmail;
    json["clientPhone"] = m_clientPhone;
    json["invoiceDate"] = m_invoiceDate.toString(Qt::ISODate);
    json["dueDate"] = m_dueDate.toString(Qt::ISODate);
    json["subtotal"] = m_subtotal;
    json["taxRate"] = m_taxRate;
    json["taxAmount"] = m_taxAmount;
    json["totalAmount"] = m_totalAmount;
    json["status"] = m_status;
    json["notes"] = m_notes;
    json["currency"] = m_currency;
    
    QJsonArray itemsArray;
    for (const InvoiceItem *item : m_items) {
        itemsArray.append(item->toJson());
    }
    json["items"] = itemsArray;
    
    return json;
}

void Invoice::fromJson(const QJsonObject &json)
{
    setId(json["id"].toString());
    setInvoiceNumber(json["invoiceNumber"].toString());
    setClientId(json["clientId"].toString());
    setClientName(json["clientName"].toString());
    setClientAddress(json["clientAddress"].toString());
    setClientEmail(json["clientEmail"].toString());
    setClientPhone(json["clientPhone"].toString());
    setInvoiceDate(QDate::fromString(json["invoiceDate"].toString(), Qt::ISODate));
    setDueDate(QDate::fromString(json["dueDate"].toString(), Qt::ISODate));
    setSubtotal(json["subtotal"].toDouble());
    setTaxRate(json["taxRate"].toDouble());
    setTaxAmount(json["taxAmount"].toDouble());
    setTotalAmount(json["totalAmount"].toDouble());
    setStatus(json["status"].toString());
    setNotes(json["notes"].toString());
    setCurrency(json["currency"].toString());
    
    // Load items
    clearItems();
    QJsonArray itemsArray = json["items"].toArray();
    for (const QJsonValue &value : itemsArray) {
        InvoiceItem *item = new InvoiceItem(this);
        item->fromJson(value.toObject());
        addItem(item);
    }
}

QStringList Invoice::availableStatuses()
{
    return {"Draft", "Sent", "Paid", "Overdue", "Cancelled"};
}

QString Invoice::statusToString(Status status)
{
    switch (status) {
    case Draft: return "Draft";
    case Sent: return "Sent";
    case Paid: return "Paid";
    case Overdue: return "Overdue";
    case Cancelled: return "Cancelled";
    }
    return "Draft";
}

Invoice::Status Invoice::stringToStatus(const QString &statusString)
{
    if (statusString == "Draft") return Draft;
    if (statusString == "Sent") return Sent;
    if (statusString == "Paid") return Paid;
    if (statusString == "Overdue") return Overdue;
    if (statusString == "Cancelled") return Cancelled;
    return Draft;
}

QString Invoice::generateInvoiceNumber()
{
    static int counter = 1;
    QString dateStr = QDate::currentDate().toString("yyyyMM");
    return QString("INV-%1-%2").arg(dateStr).arg(counter++, 4, 10, QChar('0'));
}

QStringList Invoice::availableCurrencies()
{
    return {"USD", "EUR", "GBP", "CAD", "AUD", "JPY", "CHF", "CNY"};
}
