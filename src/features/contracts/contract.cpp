#include "contract.h"
#include <QUuid>
#include <QJsonArray>
#include <QDebug>

Contract::Contract(QObject *parent)
    : QObject(parent)
    , m_value(0.0)
    , m_paymentTerms(30)
    , m_hasNonCompeteClause(false)
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_status = "Draft";
}

Contract::Contract(const Contract &other, QObject *parent)
    : QObject(parent)
    , m_id(other.m_id)
    , m_clientName(other.m_clientName)
    , m_startDate(other.m_startDate)
    , m_endDate(other.m_endDate)
    , m_value(other.m_value)
    , m_status(other.m_status)
    , m_description(other.m_description)
    , m_paymentTerms(other.m_paymentTerms)
    , m_hasNonCompeteClause(other.m_hasNonCompeteClause)
{
    // Generate new ID for copy to ensure uniqueness
    if (parent == nullptr) {
        m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
}

Contract::Contract(const QString &id, const QString &clientName, const QDate &startDate,
                   const QDate &endDate, double value, const QString &status,
                   const QString &description, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_clientName(clientName)
    , m_startDate(startDate)
    , m_endDate(endDate)
    , m_value(value)
    , m_status(status)
    , m_description(description)
    , m_paymentTerms(30)
    , m_hasNonCompeteClause(false)
{
}

void Contract::setId(const QString &id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged();
    }
}

void Contract::setClientName(const QString &clientName)
{
    if (m_clientName != clientName) {
        m_clientName = clientName;
        emit clientNameChanged();
    }
}

void Contract::setStartDate(const QDate &startDate)
{
    if (m_startDate != startDate) {
        m_startDate = startDate;
        emit startDateChanged();
    }
}

void Contract::setEndDate(const QDate &endDate)
{
    if (m_endDate != endDate) {
        m_endDate = endDate;
        emit endDateChanged();
    }
}

void Contract::setValue(double value)
{
    if (qFuzzyCompare(m_value, value) == false) {
        m_value = value;
        emit valueChanged();
    }
}

void Contract::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void Contract::setDescription(const QString &description)
{
    if (m_description != description) {
        m_description = description;
        emit descriptionChanged();
    }
}

void Contract::setPaymentTerms(int paymentTerms)
{
    if (m_paymentTerms != paymentTerms) {
        m_paymentTerms = paymentTerms;
        emit paymentTermsChanged();
    }
}

void Contract::setHasNonCompeteClause(bool hasNonCompeteClause)
{
    if (m_hasNonCompeteClause != hasNonCompeteClause) {
        m_hasNonCompeteClause = hasNonCompeteClause;
        emit hasNonCompeteClauseChanged();
    }
}

bool Contract::isValid() const
{
    return !m_clientName.isEmpty() && 
           m_startDate.isValid() && 
           m_endDate.isValid() && 
           m_startDate <= m_endDate &&
           m_value >= 0.0 &&
           !m_status.isEmpty();
}

QStringList Contract::validationErrors() const
{
    QStringList errors;
    
    if (m_clientName.isEmpty()) {
        errors << "Client name is required";
    }
    
    if (!m_startDate.isValid()) {
        errors << "Start date is required";
    }
    
    if (!m_endDate.isValid()) {
        errors << "End date is required";
    }
    
    if (m_startDate.isValid() && m_endDate.isValid() && m_startDate > m_endDate) {
        errors << "Start date must be before end date";
    }
    
    if (m_value < 0.0) {
        errors << "Contract value cannot be negative";
    }
    
    if (m_status.isEmpty()) {
        errors << "Status is required";
    }
    
    return errors;
}

bool Contract::isExpired() const
{
    return m_endDate.isValid() && m_endDate < QDate::currentDate();
}

bool Contract::isExpiringSoon(int daysThreshold) const
{
    if (!m_endDate.isValid()) return false;
    
    QDate currentDate = QDate::currentDate();
    return m_endDate >= currentDate && 
           currentDate.daysTo(m_endDate) <= daysThreshold;
}

int Contract::daysUntilExpiry() const
{
    if (!m_endDate.isValid()) return -1;
    
    return QDate::currentDate().daysTo(m_endDate);
}

QString Contract::statusDisplayText() const
{
    // Add visual indicators for status
    if (isExpired()) {
        return "Expired ⚠️";
    } else if (isExpiringSoon()) {
        return m_status + " ⏰";
    }
    return m_status;
}

QJsonObject Contract::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["clientName"] = m_clientName;
    json["startDate"] = m_startDate.toString(Qt::ISODate);
    json["endDate"] = m_endDate.toString(Qt::ISODate);
    json["value"] = m_value;
    json["status"] = m_status;
    json["description"] = m_description;
    json["paymentTerms"] = m_paymentTerms;
    json["hasNonCompeteClause"] = m_hasNonCompeteClause;
    return json;
}

void Contract::fromJson(const QJsonObject &json)
{
    setId(json["id"].toString());
    setClientName(json["clientName"].toString());
    setStartDate(QDate::fromString(json["startDate"].toString(), Qt::ISODate));
    setEndDate(QDate::fromString(json["endDate"].toString(), Qt::ISODate));
    setValue(json["value"].toDouble());
    setStatus(json["status"].toString());
    setDescription(json["description"].toString());
    setPaymentTerms(json["paymentTerms"].toInt(30));
    setHasNonCompeteClause(json["hasNonCompeteClause"].toBool());
}

QStringList Contract::availableStatuses()
{
    return {"Draft", "Active", "Completed", "Expired", "Cancelled"};
}

QString Contract::statusToString(Status status)
{
    switch (status) {
    case Draft: return "Draft";
    case Active: return "Active";
    case Completed: return "Completed";
    case Expired: return "Expired";
    case Cancelled: return "Cancelled";
    }
    return "Draft";
}

Contract::Status Contract::stringToStatus(const QString &statusString)
{
    if (statusString == "Active") return Active;
    if (statusString == "Completed") return Completed;
    if (statusString == "Expired") return Expired;
    if (statusString == "Cancelled") return Cancelled;
    return Draft;
}
