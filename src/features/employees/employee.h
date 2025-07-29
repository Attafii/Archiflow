#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief The Employee class represents an employee in the ArchiFlow system
 * 
 * This class encapsulates all employee-related data and provides
 * validation, serialization, and utility methods.
 */
class Employee : public QObject
{
    Q_OBJECT

public:
    enum EmployeeStatus {
        Active,
        Inactive,
        OnLeave,
        Terminated
    };
    Q_ENUM(EmployeeStatus)

    enum EmployeeRole {
        Architect,
        Engineer,
        ProjectManager,
        Designer,
        Contractor,
        Administrator,
        Other
    };
    Q_ENUM(EmployeeRole)

    explicit Employee(QObject *parent = nullptr);
    Employee(const QString &cin, const QString &firstName, const QString &lastName, QObject *parent = nullptr);
    Employee(const Employee &other);
    Employee& operator=(const Employee &other);
    virtual ~Employee() = default;    // Getters
    int id() const { return m_id; }
    QString cin() const { return m_cin; }
    QString firstName() const { return m_firstName; }
    QString lastName() const { return m_lastName; }
    QString email() const { return m_email; }
    QString phoneNumber() const { return m_phoneNumber; }
    QString phone() const { return m_phoneNumber; } // Alias for compatibility
    QString position() const { return m_position; }
    EmployeeRole role() const { return m_role; }
    QDateTime hireDate() const { return m_hireDate; }
    QDateTime startDate() const { return m_hireDate; } // Alias for compatibility
    EmployeeStatus status() const { return m_status; }
    bool isPresent() const { return m_isPresent; }
    double salary() const { return m_salary; }
    QString department() const { return m_department; }
    QString employmentType() const { return m_employmentType; }
    QString address() const { return m_address; }
    QString emergencyContact() const { return m_emergencyContact; }
    QString emergencyPhone() const { return m_emergencyPhone; }
    QString notes() const { return m_notes; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime updatedAt() const { return m_updatedAt; }    // Setters
    void setId(int id);
    void setCin(const QString &cin);
    void setFirstName(const QString &firstName);
    void setLastName(const QString &lastName);
    void setEmail(const QString &email);
    void setPhoneNumber(const QString &phoneNumber);
    void setPosition(const QString &position);
    void setRole(EmployeeRole role);
    void setHireDate(const QDateTime &hireDate);
    void setStatus(EmployeeStatus status);
    void setIsPresent(bool isPresent);
    void setSalary(double salary);
    void setDepartment(const QString &department);
    void setEmploymentType(const QString &employmentType);
    void setAddress(const QString &address);
    void setEmergencyContact(const QString &emergencyContact);
    void setEmergencyPhone(const QString &emergencyPhone);
    void setNotes(const QString &notes);
    void setCreatedAt(const QDateTime &createdAt);
    void setUpdatedAt(const QDateTime &updatedAt);

    // Utility methods
    QString fullName() const;
    QString displayName() const;
    QString statusString() const;
    QString roleString() const;
    int yearsOfService() const;
    bool isValid() const;
    QStringList validationErrors() const;
    
    // Serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

    // Static utilities
    static QString generateCin();
    static QString roleToString(EmployeeRole role);
    static EmployeeRole stringToRole(const QString &roleStr);
    static QString statusToString(EmployeeStatus status);
    static EmployeeStatus stringToStatus(const QString &statusStr);

signals:
    void cinChanged();
    void firstNameChanged();
    void lastNameChanged();
    void emailChanged();
    void phoneNumberChanged();
    void positionChanged();
    void roleChanged();
    void hireDateChanged();
    void statusChanged();
    void isPresentChanged();
    void salaryChanged();
    void departmentChanged();
    void addressChanged();
    void emergencyContactChanged();
    void emergencyPhoneChanged();
    void notesChanged();

private:
    int m_id;
    QString m_cin;
    QString m_firstName;
    QString m_lastName;
    QString m_email;
    QString m_phoneNumber;
    QString m_position;
    EmployeeRole m_role;
    QDateTime m_hireDate;
    EmployeeStatus m_status;
    bool m_isPresent;
    double m_salary;
    QString m_department;
    QString m_employmentType;
    QString m_address;
    QString m_emergencyContact;
    QString m_emergencyPhone;
    QString m_notes;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;

    void updateTimestamp();
};

#endif // EMPLOYEE_H
