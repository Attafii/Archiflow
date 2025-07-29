#ifndef CLIENTCONTACT_H
#define CLIENTCONTACT_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>

/**
 * @brief The ClientContact class represents a client in the ArchiFlow system
 * 
 * This class encapsulates all client-related data and provides
 * validation and conversion methods for the client management feature.
 */
class ClientContact : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString companyName READ companyName WRITE setCompanyName NOTIFY companyNameChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)
    Q_PROPERTY(QString addressStreet READ addressStreet WRITE setAddressStreet NOTIFY addressStreetChanged)
    Q_PROPERTY(QString addressCity READ addressCity WRITE setAddressCity NOTIFY addressCityChanged)
    Q_PROPERTY(QString addressState READ addressState WRITE setAddressState NOTIFY addressStateChanged)
    Q_PROPERTY(QString addressZipcode READ addressZipcode WRITE setAddressZipcode NOTIFY addressZipcodeChanged)
    Q_PROPERTY(QString addressCountry READ addressCountry WRITE setAddressCountry NOTIFY addressCountryChanged)
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)
    Q_PROPERTY(QDateTime createdAt READ createdAt WRITE setCreatedAt NOTIFY createdAtChanged)
    Q_PROPERTY(QDateTime updatedAt READ updatedAt WRITE setUpdatedAt NOTIFY updatedAtChanged)

public:
    explicit ClientContact(QObject *parent = nullptr);
    ClientContact(const ClientContact &other, QObject *parent = nullptr);
    ClientContact(const QString &id, const QString &name, const QString &email, 
           QObject *parent = nullptr);

    // Getters
    QString id() const { return m_id; }
    QString name() const { return m_name; }
    QString companyName() const { return m_companyName; }
    QString email() const { return m_email; }
    QString phoneNumber() const { return m_phoneNumber; }
    QString addressStreet() const { return m_addressStreet; }
    QString addressCity() const { return m_addressCity; }
    QString addressState() const { return m_addressState; }
    QString addressZipcode() const { return m_addressZipcode; }
    QString addressCountry() const { return m_addressCountry; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    QString notes() const { return m_notes; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime updatedAt() const { return m_updatedAt; }

    // Setters
    void setId(const QString &id);
    void setName(const QString &name);
    void setCompanyName(const QString &companyName);
    void setEmail(const QString &email);
    void setPhoneNumber(const QString &phoneNumber);
    void setAddressStreet(const QString &addressStreet);
    void setAddressCity(const QString &addressCity);
    void setAddressState(const QString &addressState);
    void setAddressZipcode(const QString &addressZipcode);
    void setAddressCountry(const QString &addressCountry);
    void setLatitude(double latitude);
    void setLongitude(double longitude);
    void setNotes(const QString &notes);
    void setCreatedAt(const QDateTime &createdAt);
    void setUpdatedAt(const QDateTime &updatedAt);

    // Utility methods
    QString fullAddress() const;
    QString displayName() const;
    bool hasValidCoordinates() const;
    bool isValid() const;
    QStringList validationErrors() const;
    
    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

    // Static utilities
    static QString generateId();

signals:
    void idChanged();
    void nameChanged();
    void companyNameChanged();
    void emailChanged();
    void phoneNumberChanged();
    void addressStreetChanged();
    void addressCityChanged();
    void addressStateChanged();
    void addressZipcodeChanged();
    void addressCountryChanged();
    void latitudeChanged();
    void longitudeChanged();
    void notesChanged();
    void createdAtChanged();
    void updatedAtChanged();

private:
    QString m_id;
    QString m_name;
    QString m_companyName;
    QString m_email;
    QString m_phoneNumber;
    QString m_addressStreet;
    QString m_addressCity;
    QString m_addressState;
    QString m_addressZipcode;
    QString m_addressCountry;
    double m_latitude;
    double m_longitude;
    QString m_notes;
    QDateTime m_createdAt;    QDateTime m_updatedAt;
};

// Type alias for client lists
using ClientContactList = QList<ClientContact*>;

#endif // CLIENTCONTACT_H
