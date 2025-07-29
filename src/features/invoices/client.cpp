#include "client.h"
#include <QUuid>
#include <QDebug>

Client::Client(QObject *parent)
    : QObject(parent)
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

Client::Client(const Client &other, QObject *parent)
    : QObject(parent)
    , m_id(other.m_id)
    , m_name(other.m_name)
    , m_address(other.m_address)
    , m_email(other.m_email)
    , m_phone(other.m_phone)
    , m_company(other.m_company)
    , m_taxId(other.m_taxId)
    , m_notes(other.m_notes)
{
}

Client::Client(const QString &name, const QString &email, const QString &phone,
               const QString &address, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_address(address)
    , m_email(email)
    , m_phone(phone)
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Client::setId(const QString &id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged();
    }
}

void Client::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

void Client::setAddress(const QString &address)
{
    if (m_address != address) {
        m_address = address;
        emit addressChanged();
    }
}

void Client::setEmail(const QString &email)
{
    if (m_email != email) {
        m_email = email;
        emit emailChanged();
    }
}

void Client::setPhone(const QString &phone)
{
    if (m_phone != phone) {
        m_phone = phone;
        emit phoneChanged();
    }
}

void Client::setCompany(const QString &company)
{
    if (m_company != company) {
        m_company = company;
        emit companyChanged();
    }
}

void Client::setTaxId(const QString &taxId)
{
    if (m_taxId != taxId) {
        m_taxId = taxId;
        emit taxIdChanged();
    }
}

void Client::setNotes(const QString &notes)
{
    if (m_notes != notes) {
        m_notes = notes;
        emit notesChanged();
    }
}

bool Client::isValid() const
{
    return !m_name.isEmpty() && 
           (!m_email.isEmpty() || !m_phone.isEmpty());
}

QStringList Client::validationErrors() const
{
    QStringList errors;
    
    if (m_name.isEmpty()) {
        errors << "Client name is required";
    }
    
    if (m_email.isEmpty() && m_phone.isEmpty()) {
        errors << "Either email or phone number is required";
    }
    
    if (!m_email.isEmpty() && !m_email.contains("@")) {
        errors << "Invalid email format";
    }
    
    return errors;
}

QString Client::displayName() const
{
    if (!m_company.isEmpty()) {
        return QString("%1 (%2)").arg(m_name).arg(m_company);
    }
    return m_name;
}

QString Client::fullAddress() const
{
    return m_address;
}

QJsonObject Client::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["address"] = m_address;
    json["email"] = m_email;
    json["phone"] = m_phone;
    json["company"] = m_company;
    json["taxId"] = m_taxId;
    json["notes"] = m_notes;
    return json;
}

void Client::fromJson(const QJsonObject &json)
{
    setId(json["id"].toString());
    setName(json["name"].toString());
    setAddress(json["address"].toString());
    setEmail(json["email"].toString());
    setPhone(json["phone"].toString());
    setCompany(json["company"].toString());
    setTaxId(json["taxId"].toString());
    setNotes(json["notes"].toString());
}
