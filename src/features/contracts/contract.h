#ifndef CONTRACT_H
#define CONTRACT_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QJsonObject>
#include <QList>

/**
 * @brief The Contract class represents a contract in the ArchiFlow system
 * 
 * This class encapsulates all contract-related data and provides
 * validation and conversion methods for the contract management feature.
 */
class Contract : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString clientName READ clientName WRITE setClientName NOTIFY clientNameChanged)
    Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDate endDate READ endDate WRITE setEndDate NOTIFY endDateChanged)
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(int paymentTerms READ paymentTerms WRITE setPaymentTerms NOTIFY paymentTermsChanged)
    Q_PROPERTY(bool hasNonCompeteClause READ hasNonCompeteClause WRITE setHasNonCompeteClause NOTIFY hasNonCompeteClauseChanged)

public:
    enum Status {
        Draft,
        Active,
        Completed,
        Expired,
        Cancelled
    };
    Q_ENUM(Status)

    explicit Contract(QObject *parent = nullptr);
    Contract(const Contract &other, QObject *parent = nullptr);
    Contract(const QString &id, const QString &clientName, const QDate &startDate,
             const QDate &endDate, double value, const QString &status,
             const QString &description = QString(), QObject *parent = nullptr);

    // Getters
    QString id() const { return m_id; }
    QString clientName() const { return m_clientName; }
    QDate startDate() const { return m_startDate; }
    QDate endDate() const { return m_endDate; }
    double value() const { return m_value; }    QString status() const { return m_status; }
    QString description() const { return m_description; }
    int paymentTerms() const { return m_paymentTerms; }
    QString paymentTermsString() const { return QString::number(m_paymentTerms) + " days"; }
    bool hasNonCompeteClause() const { return m_hasNonCompeteClause; }

    // Setters
    void setId(const QString &id);
    void setClientName(const QString &clientName);
    void setStartDate(const QDate &startDate);
    void setEndDate(const QDate &endDate);
    void setValue(double value);
    void setStatus(const QString &status);
    void setDescription(const QString &description);
    void setPaymentTerms(int paymentTerms);
    void setHasNonCompeteClause(bool hasNonCompeteClause);

    // Validation
    bool isValid() const;
    QStringList validationErrors() const;

    // Utility methods
    bool isExpired() const;
    bool isExpiringSoon(int daysThreshold = 30) const;
    int daysUntilExpiry() const;
    QString statusDisplayText() const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

    // Static utilities
    static QStringList availableStatuses();
    static QString statusToString(Status status);
    static Status stringToStatus(const QString &statusString);

signals:
    void idChanged();
    void clientNameChanged();
    void startDateChanged();
    void endDateChanged();
    void valueChanged();
    void statusChanged();
    void descriptionChanged();
    void paymentTermsChanged();
    void hasNonCompeteClauseChanged();

private:
    QString m_id;
    QString m_clientName;
    QDate m_startDate;
    QDate m_endDate;
    double m_value;
    QString m_status;
    QString m_description;
    int m_paymentTerms;
    bool m_hasNonCompeteClause;
};

// Type alias for contract lists
using ContractList = QList<Contract*>;

#endif // CONTRACT_H
