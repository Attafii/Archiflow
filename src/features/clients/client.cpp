#include "client.h"
#include <QUuid>
#include <QRegularExpression>

ClientContact::ClientContact(QObject *parent)
    : QObject(parent)
    , m_id(generateId())
    , m_latitude(0.0)
    , m_longitude(0.0)
    , m_createdAt(QDateTime::currentDateTime())
    , m_updatedAt(QDateTime::currentDateTime())
{
}

ClientContact::ClientContact(const ClientContact &other, QObject *parent)
    : QObject(parent)
    , m_id(other.m_id)
    , m_name(other.m_name)
    , m_companyName(other.m_companyName)
    , m_email(other.m_email)
    , m_phoneNumber(other.m_phoneNumber)
    , m_addressStreet(other.m_addressStreet)
    , m_addressCity(other.m_addressCity)
    , m_addressState(other.m_addressState)
    , m_addressZipcode(other.m_addressZipcode)
    , m_addressCountry(other.m_addressCountry)
    , m_latitude(other.m_latitude)
    , m_longitude(other.m_longitude)
    , m_notes(other.m_notes)
    , m_createdAt(other.m_createdAt)
    , m_updatedAt(other.m_updatedAt)
{
}

ClientContact::ClientContact(const QString &id, const QString &name, const QString &email, QObject *parent)
    : QObject(parent)
    , m_id(id.isEmpty() ? generateId() : id)
    , m_name(name)
    , m_email(email)
    , m_latitude(0.0)
    , m_longitude(0.0)
    , m_createdAt(QDateTime::currentDateTime())
    , m_updatedAt(QDateTime::currentDateTime())
{
}

// Setters
void ClientContact::setId(const QString &id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged();
    }
}

void ClientContact::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        m_updatedAt = QDateTime::currentDateTime();
        emit nameChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setCompanyName(const QString &companyName)
{
    if (m_companyName != companyName) {
        m_companyName = companyName;
        m_updatedAt = QDateTime::currentDateTime();
        emit companyNameChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setEmail(const QString &email)
{
    if (m_email != email) {
        m_email = email;
        m_updatedAt = QDateTime::currentDateTime();
        emit emailChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setPhoneNumber(const QString &phoneNumber)
{
    if (m_phoneNumber != phoneNumber) {
        m_phoneNumber = phoneNumber;
        m_updatedAt = QDateTime::currentDateTime();
        emit phoneNumberChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setAddressStreet(const QString &addressStreet)
{
    if (m_addressStreet != addressStreet) {
        m_addressStreet = addressStreet;
        m_updatedAt = QDateTime::currentDateTime();
        emit addressStreetChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setAddressCity(const QString &addressCity)
{
    if (m_addressCity != addressCity) {
        m_addressCity = addressCity;
        m_updatedAt = QDateTime::currentDateTime();
        emit addressCityChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setAddressState(const QString &addressState)
{
    if (m_addressState != addressState) {
        m_addressState = addressState;
        m_updatedAt = QDateTime::currentDateTime();
        emit addressStateChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setAddressZipcode(const QString &addressZipcode)
{
    if (m_addressZipcode != addressZipcode) {
        m_addressZipcode = addressZipcode;
        m_updatedAt = QDateTime::currentDateTime();
        emit addressZipcodeChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setAddressCountry(const QString &addressCountry)
{
    if (m_addressCountry != addressCountry) {
        m_addressCountry = addressCountry;
        m_updatedAt = QDateTime::currentDateTime();
        emit addressCountryChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setLatitude(double latitude)
{
    if (qAbs(m_latitude - latitude) > 0.0001) {
        m_latitude = latitude;
        m_updatedAt = QDateTime::currentDateTime();
        emit latitudeChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setLongitude(double longitude)
{
    if (qAbs(m_longitude - longitude) > 0.0001) {
        m_longitude = longitude;
        m_updatedAt = QDateTime::currentDateTime();
        emit longitudeChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setNotes(const QString &notes)
{
    if (m_notes != notes) {
        m_notes = notes;
        m_updatedAt = QDateTime::currentDateTime();
        emit notesChanged();
        emit updatedAtChanged();
    }
}

void ClientContact::setCreatedAt(const QDateTime &createdAt)
{
    if (m_createdAt != createdAt) {
        m_createdAt = createdAt;
        emit createdAtChanged();
    }
}

void ClientContact::setUpdatedAt(const QDateTime &updatedAt)
{
    if (m_updatedAt != updatedAt) {
        m_updatedAt = updatedAt;
        emit updatedAtChanged();
    }
}

// Utility methods
QString ClientContact::fullAddress() const
{
    QStringList parts;
    
    if (!m_addressStreet.isEmpty()) parts << m_addressStreet;
    if (!m_addressCity.isEmpty()) parts << m_addressCity;
    if (!m_addressState.isEmpty()) parts << m_addressState;
    if (!m_addressZipcode.isEmpty()) parts << m_addressZipcode;
    if (!m_addressCountry.isEmpty()) parts << m_addressCountry;
    
    return parts.join(", ");
}

QString ClientContact::displayName() const
{
    if (!m_companyName.isEmpty()) {
        return m_companyName + (m_name.isEmpty() ? "" : " (" + m_name + ")");
    }
    return m_name;
}

bool ClientContact::hasValidCoordinates() const
{
    return (m_latitude != 0.0 || m_longitude != 0.0) &&
           (m_latitude >= -90.0 && m_latitude <= 90.0) &&
           (m_longitude >= -180.0 && m_longitude <= 180.0);
}

bool ClientContact::isValid() const
{
    return validationErrors().isEmpty();
}

QStringList ClientContact::validationErrors() const
{
    QStringList errors;
    
    if (m_name.trimmed().isEmpty()) {
        errors << "Name is required";
    }
    
    if (!m_email.isEmpty()) {
        QRegularExpression emailRegex("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
        if (!emailRegex.match(m_email).hasMatch()) {
            errors << "Invalid email format";
        }
    }
    
    if (m_latitude < -90.0 || m_latitude > 90.0) {
        errors << "Latitude must be between -90 and 90";
    }
    
    if (m_longitude < -180.0 || m_longitude > 180.0) {
        errors << "Longitude must be between -180 and 180";
    }
    
    return errors;
}

QJsonObject ClientContact::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["companyName"] = m_companyName;
    json["email"] = m_email;
    json["phoneNumber"] = m_phoneNumber;
    json["addressStreet"] = m_addressStreet;
    json["addressCity"] = m_addressCity;
    json["addressState"] = m_addressState;
    json["addressZipcode"] = m_addressZipcode;
    json["addressCountry"] = m_addressCountry;
    json["latitude"] = m_latitude;
    json["longitude"] = m_longitude;
    json["notes"] = m_notes;
    json["createdAt"] = m_createdAt.toString(Qt::ISODate);
    json["updatedAt"] = m_updatedAt.toString(Qt::ISODate);
    return json;
}

void ClientContact::fromJson(const QJsonObject &json)
{
    setId(json["id"].toString());
    setName(json["name"].toString());
    setCompanyName(json["companyName"].toString());
    setEmail(json["email"].toString());
    setPhoneNumber(json["phoneNumber"].toString());
    setAddressStreet(json["addressStreet"].toString());
    setAddressCity(json["addressCity"].toString());
    setAddressState(json["addressState"].toString());
    setAddressZipcode(json["addressZipcode"].toString());
    setAddressCountry(json["addressCountry"].toString());
    setLatitude(json["latitude"].toDouble());
    setLongitude(json["longitude"].toDouble());
    setNotes(json["notes"].toString());
    setCreatedAt(QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate));
    setUpdatedAt(QDateTime::fromString(json["updatedAt"].toString(), Qt::ISODate));
}

QString ClientContact::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
