#include "invoiceitem.h"
#include <QUuid>
#include <QDebug>

InvoiceItem::InvoiceItem(QObject *parent)
    : QObject(parent)
    , m_quantity(1)
    , m_unitPrice(0.0)
    , m_totalPrice(0.0)
    , m_unit("pcs")
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

InvoiceItem::InvoiceItem(const InvoiceItem &other, QObject *parent)
    : QObject(parent)
    , m_id(other.m_id)
    , m_invoiceId(other.m_invoiceId)
    , m_description(other.m_description)
    , m_quantity(other.m_quantity)
    , m_unitPrice(other.m_unitPrice)
    , m_totalPrice(other.m_totalPrice)
    , m_unit(other.m_unit)
{
}

InvoiceItem::InvoiceItem(const QString &description, int quantity, double unitPrice, 
                        const QString &unit, QObject *parent)
    : QObject(parent)
    , m_description(description)
    , m_quantity(quantity)
    , m_unitPrice(unitPrice)
    , m_unit(unit)
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    calculateTotal();
}

void InvoiceItem::setId(const QString &id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged();
    }
}

void InvoiceItem::setInvoiceId(const QString &invoiceId)
{
    if (m_invoiceId != invoiceId) {
        m_invoiceId = invoiceId;
        emit invoiceIdChanged();
    }
}

void InvoiceItem::setDescription(const QString &description)
{
    if (m_description != description) {
        m_description = description;
        emit descriptionChanged();
    }
}

void InvoiceItem::setQuantity(int quantity)
{
    if (m_quantity != quantity) {
        m_quantity = quantity;
        calculateTotal();
        emit quantityChanged();
    }
}

void InvoiceItem::setUnitPrice(double unitPrice)
{
    if (qAbs(m_unitPrice - unitPrice) > 0.01) {
        m_unitPrice = unitPrice;
        calculateTotal();
        emit unitPriceChanged();
    }
}

void InvoiceItem::setTotalPrice(double totalPrice)
{
    if (qAbs(m_totalPrice - totalPrice) > 0.01) {
        m_totalPrice = totalPrice;
        emit totalPriceChanged();
    }
}

void InvoiceItem::setUnit(const QString &unit)
{
    if (m_unit != unit) {
        m_unit = unit;
        emit unitChanged();
    }
}

void InvoiceItem::calculateTotal()
{
    double newTotal = calculateTotalPrice();
    setTotalPrice(newTotal);
}

double InvoiceItem::calculateTotalPrice() const
{
    return m_quantity * m_unitPrice;
}

bool InvoiceItem::isValid() const
{
    return !m_description.isEmpty() && 
           m_quantity > 0 && 
           m_unitPrice >= 0.0;
}

QStringList InvoiceItem::validationErrors() const
{
    QStringList errors;
    
    if (m_description.isEmpty()) {
        errors << "Description is required";
    }
    
    if (m_quantity <= 0) {
        errors << "Quantity must be greater than zero";
    }
    
    if (m_unitPrice < 0.0) {
        errors << "Unit price cannot be negative";
    }
    
    return errors;
}

QString InvoiceItem::formatUnitPrice() const
{
    return QString::number(m_unitPrice, 'f', 2);
}

QString InvoiceItem::formatTotalPrice() const
{
    return QString::number(m_totalPrice, 'f', 2);
}

QJsonObject InvoiceItem::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["invoiceId"] = m_invoiceId;
    json["description"] = m_description;
    json["quantity"] = m_quantity;
    json["unitPrice"] = m_unitPrice;
    json["totalPrice"] = m_totalPrice;
    json["unit"] = m_unit;
    return json;
}

void InvoiceItem::fromJson(const QJsonObject &json)
{
    setId(json["id"].toString());
    setInvoiceId(json["invoiceId"].toString());
    setDescription(json["description"].toString());
    setQuantity(json["quantity"].toInt());
    setUnitPrice(json["unitPrice"].toDouble());
    setTotalPrice(json["totalPrice"].toDouble());
    setUnit(json["unit"].toString());
}

QStringList InvoiceItem::availableUnits()
{
    return {"pcs", "hrs", "days", "m", "m²", "m³", "kg", "tons", "sets", "lots"};
}
