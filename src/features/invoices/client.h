#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>

/**
 * @brief The Client class represents a client in the invoice system
 * 
 * This class encapsulates client information for invoice generation
 * and management within the ArchiFlow system.
 */
class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString phone READ phone WRITE setPhone NOTIFY phoneChanged)
    Q_PROPERTY(QString company READ company WRITE setCompany NOTIFY companyChanged)
    Q_PROPERTY(QString taxId READ taxId WRITE setTaxId NOTIFY taxIdChanged)
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)

public:
    explicit Client(QObject *parent = nullptr);
    Client(const Client &other, QObject *parent = nullptr);
    Client(const QString &name, const QString &email, const QString &phone,
           const QString &address = QString(), QObject *parent = nullptr);

    // Getters
    QString id() const { return m_id; }
    QString name() const { return m_name; }
    QString address() const { return m_address; }
    QString email() const { return m_email; }
    QString phone() const { return m_phone; }
    QString company() const { return m_company; }
    QString taxId() const { return m_taxId; }
    QString notes() const { return m_notes; }

    // Setters
    void setId(const QString &id);
    void setName(const QString &name);
    void setAddress(const QString &address);
    void setEmail(const QString &email);
    void setPhone(const QString &phone);
    void setCompany(const QString &company);
    void setTaxId(const QString &taxId);
    void setNotes(const QString &notes);

    // Validation
    bool isValid() const;
    QStringList validationErrors() const;

    // Utility methods
    QString displayName() const;
    QString fullAddress() const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void idChanged();
    void nameChanged();
    void addressChanged();
    void emailChanged();
    void phoneChanged();
    void companyChanged();
    void taxIdChanged();
    void notesChanged();

private:
    QString m_id;
    QString m_name;
    QString m_address;
    QString m_email;
    QString m_phone;
    QString m_company;
    QString m_taxId;
    QString m_notes;
};

// Type alias for client lists
using ClientList = QList<Client*>;

#endif // CLIENT_H
