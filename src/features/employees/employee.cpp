#include "employee.h"
#include <QUuid>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDebug>

Employee::Employee(QObject *parent)
    : QObject(parent)
    , m_id(0)
    , m_role(Other)
    , m_status(Active)
    , m_isPresent(false)
    , m_salary(0.0)
    , m_employmentType("Full-time")
    , m_createdAt(QDateTime::currentDateTime())
    , m_updatedAt(QDateTime::currentDateTime())
{
}

Employee::Employee(const QString &cin, const QString &firstName, const QString &lastName, QObject *parent)
    : QObject(parent)
    , m_id(0)
    , m_cin(cin)
    , m_firstName(firstName)
    , m_lastName(lastName)
    , m_role(Other)
    , m_status(Active)
    , m_isPresent(false)
    , m_salary(0.0)
    , m_employmentType("Full-time")
    , m_createdAt(QDateTime::currentDateTime())
    , m_updatedAt(QDateTime::currentDateTime())
{
}

Employee::Employee(const Employee &other)
    : QObject(other.parent())
    , m_id(other.m_id)
    , m_cin(other.m_cin)
    , m_firstName(other.m_firstName)
    , m_lastName(other.m_lastName)
    , m_email(other.m_email)
    , m_phoneNumber(other.m_phoneNumber)
    , m_position(other.m_position)
    , m_role(other.m_role)
    , m_hireDate(other.m_hireDate)
    , m_status(other.m_status)
    , m_isPresent(other.m_isPresent)
    , m_salary(other.m_salary)
    , m_department(other.m_department)
    , m_employmentType(other.m_employmentType)
    , m_address(other.m_address)
    , m_emergencyContact(other.m_emergencyContact)
    , m_emergencyPhone(other.m_emergencyPhone)
    , m_notes(other.m_notes)
    , m_createdAt(other.m_createdAt)
    , m_updatedAt(other.m_updatedAt)
{
}

Employee& Employee::operator=(const Employee &other)
{
    if (this != &other) {
        m_cin = other.m_cin;
        m_firstName = other.m_firstName;
        m_lastName = other.m_lastName;
        m_email = other.m_email;
        m_phoneNumber = other.m_phoneNumber;
        m_position = other.m_position;
        m_role = other.m_role;
        m_hireDate = other.m_hireDate;
        m_status = other.m_status;
        m_isPresent = other.m_isPresent;
        m_salary = other.m_salary;
        m_department = other.m_department;
        m_address = other.m_address;
        m_emergencyContact = other.m_emergencyContact;
        m_emergencyPhone = other.m_emergencyPhone;
        m_notes = other.m_notes;
        m_createdAt = other.m_createdAt;
        m_updatedAt = other.m_updatedAt;
    }
    return *this;
}

// Setters with signal emission
void Employee::setId(int id)
{
    if (m_id != id) {
        m_id = id;
        updateTimestamp();
    }
}

void Employee::setCin(const QString &cin)
{
    if (m_cin != cin) {
        m_cin = cin;
        updateTimestamp();
        emit cinChanged();
    }
}

void Employee::setFirstName(const QString &firstName)
{
    if (m_firstName != firstName) {
        m_firstName = firstName;
        updateTimestamp();
        emit firstNameChanged();
    }
}

void Employee::setLastName(const QString &lastName)
{
    if (m_lastName != lastName) {
        m_lastName = lastName;
        updateTimestamp();
        emit lastNameChanged();
    }
}

void Employee::setEmail(const QString &email)
{
    if (m_email != email) {
        m_email = email;
        updateTimestamp();
        emit emailChanged();
    }
}

void Employee::setPhoneNumber(const QString &phoneNumber)
{
    if (m_phoneNumber != phoneNumber) {
        m_phoneNumber = phoneNumber;
        updateTimestamp();
        emit phoneNumberChanged();
    }
}

void Employee::setPosition(const QString &position)
{
    if (m_position != position) {
        m_position = position;
        updateTimestamp();
        emit positionChanged();
    }
}

void Employee::setRole(EmployeeRole role)
{
    if (m_role != role) {
        m_role = role;
        updateTimestamp();
        emit roleChanged();
    }
}

void Employee::setHireDate(const QDateTime &hireDate)
{
    if (m_hireDate != hireDate) {
        m_hireDate = hireDate;
        updateTimestamp();
        emit hireDateChanged();
    }
}

void Employee::setStatus(EmployeeStatus status)
{
    if (m_status != status) {
        m_status = status;
        updateTimestamp();
        emit statusChanged();
    }
}

void Employee::setIsPresent(bool isPresent)
{
    if (m_isPresent != isPresent) {
        m_isPresent = isPresent;
        updateTimestamp();
        emit isPresentChanged();
    }
}

void Employee::setSalary(double salary)
{
    if (qAbs(m_salary - salary) > 0.01) {
        m_salary = salary;
        updateTimestamp();
        emit salaryChanged();
    }
}

void Employee::setDepartment(const QString &department)
{
    if (m_department != department) {
        m_department = department;
        updateTimestamp();
        emit departmentChanged();
    }
}

void Employee::setEmploymentType(const QString &employmentType)
{
    if (m_employmentType != employmentType) {
        m_employmentType = employmentType;
        updateTimestamp();
    }
}

void Employee::setAddress(const QString &address)
{
    if (m_address != address) {
        m_address = address;
        updateTimestamp();
        emit addressChanged();
    }
}

void Employee::setEmergencyContact(const QString &emergencyContact)
{
    if (m_emergencyContact != emergencyContact) {
        m_emergencyContact = emergencyContact;
        updateTimestamp();
        emit emergencyContactChanged();
    }
}

void Employee::setEmergencyPhone(const QString &emergencyPhone)
{
    if (m_emergencyPhone != emergencyPhone) {
        m_emergencyPhone = emergencyPhone;
        updateTimestamp();
        emit emergencyPhoneChanged();
    }
}

void Employee::setNotes(const QString &notes)
{
    if (m_notes != notes) {
        m_notes = notes;
        updateTimestamp();
        emit notesChanged();
    }
}

void Employee::setCreatedAt(const QDateTime &createdAt)
{
    m_createdAt = createdAt;
}

void Employee::setUpdatedAt(const QDateTime &updatedAt)
{
    m_updatedAt = updatedAt;
}

// Utility methods
QString Employee::fullName() const
{
    return QString("%1 %2").arg(m_firstName, m_lastName).trimmed();
}

QString Employee::displayName() const
{
    QString name = fullName();
    if (!name.isEmpty()) {
        return name;
    }
    return m_cin.isEmpty() ? tr("Unnamed Employee") : m_cin;
}

QString Employee::statusString() const
{
    return statusToString(m_status);
}

QString Employee::roleString() const
{
    return roleToString(m_role);
}

int Employee::yearsOfService() const
{
    if (!m_hireDate.isValid()) {
        return 0;
    }
    return m_hireDate.daysTo(QDateTime::currentDateTime()) / 365;
}

bool Employee::isValid() const
{
    return validationErrors().isEmpty();
}

QStringList Employee::validationErrors() const
{
    QStringList errors;
    
    if (m_cin.trimmed().isEmpty()) {
        errors << tr("CIN is required");
    }
    
    if (m_firstName.trimmed().isEmpty()) {
        errors << tr("First name is required");
    }
    
    if (m_lastName.trimmed().isEmpty()) {
        errors << tr("Last name is required");
    }
    
    if (!m_email.isEmpty()) {
        QRegularExpression emailRegex("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
        if (!emailRegex.match(m_email).hasMatch()) {
            errors << tr("Invalid email format");
        }
    }
    
    if (m_salary < 0) {
        errors << tr("Salary cannot be negative");
    }
    
    return errors;
}

QJsonObject Employee::toJson() const
{
    QJsonObject json;
    json["cin"] = m_cin;
    json["firstName"] = m_firstName;
    json["lastName"] = m_lastName;
    json["email"] = m_email;
    json["phoneNumber"] = m_phoneNumber;
    json["position"] = m_position;
    json["role"] = static_cast<int>(m_role);
    json["roleString"] = roleToString(m_role);
    json["hireDate"] = m_hireDate.toString(Qt::ISODate);
    json["status"] = static_cast<int>(m_status);
    json["statusString"] = statusToString(m_status);
    json["isPresent"] = m_isPresent;
    json["salary"] = m_salary;
    json["department"] = m_department;
    json["address"] = m_address;
    json["emergencyContact"] = m_emergencyContact;
    json["emergencyPhone"] = m_emergencyPhone;
    json["notes"] = m_notes;
    json["createdAt"] = m_createdAt.toString(Qt::ISODate);
    json["updatedAt"] = m_updatedAt.toString(Qt::ISODate);
    return json;
}

bool Employee::fromJson(const QJsonObject &json)
{
    try {
        m_cin = json["cin"].toString();
        m_firstName = json["firstName"].toString();
        m_lastName = json["lastName"].toString();
        m_email = json["email"].toString();
        m_phoneNumber = json["phoneNumber"].toString();
        m_position = json["position"].toString();
        
        if (json.contains("role")) {
            m_role = static_cast<EmployeeRole>(json["role"].toInt());
        } else if (json.contains("roleString")) {
            m_role = stringToRole(json["roleString"].toString());
        }
        
        m_hireDate = QDateTime::fromString(json["hireDate"].toString(), Qt::ISODate);
          if (json.contains("status")) {
            m_status = static_cast<EmployeeStatus>(json["status"].toInt());
        } else if (json.contains("statusString")) {
            m_status = stringToStatus(json["statusString"].toString());
        }
        
        m_isPresent = json["isPresent"].toBool();
        m_salary = json["salary"].toDouble();
        m_department = json["department"].toString();
        m_employmentType = json["employmentType"].toString();
        m_address = json["address"].toString();
        m_emergencyContact = json["emergencyContact"].toString();
        m_emergencyPhone = json["emergencyPhone"].toString();
        m_notes = json["notes"].toString();
        m_createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
        m_updatedAt = QDateTime::fromString(json["updatedAt"].toString(), Qt::ISODate);
        
        return true;
    } catch (...) {
        return false;
    }
}

// Static utilities
QString Employee::generateCin()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper();
}

QString Employee::roleToString(EmployeeRole role)
{
    switch (role) {
        case Architect: return tr("Architect");
        case Engineer: return tr("Engineer");
        case ProjectManager: return tr("Project Manager");
        case Designer: return tr("Designer");
        case Contractor: return tr("Contractor");
        case Administrator: return tr("Administrator");
        case Other: return tr("Other");
        default: return tr("Unknown");
    }
}

Employee::EmployeeRole Employee::stringToRole(const QString &roleStr)
{
    QString role = roleStr.toLower();
    if (role.contains("architect")) return Architect;
    if (role.contains("engineer")) return Engineer;
    if (role.contains("project") || role.contains("manager")) return ProjectManager;
    if (role.contains("designer")) return Designer;
    if (role.contains("contractor")) return Contractor;
    if (role.contains("administrator") || role.contains("admin")) return Administrator;
    return Other;
}

QString Employee::statusToString(EmployeeStatus status)
{
    switch (status) {
        case Active: return tr("Active");
        case Inactive: return tr("Inactive");
        case OnLeave: return tr("On Leave");
        case Terminated: return tr("Terminated");
        default: return tr("Unknown");
    }
}

Employee::EmployeeStatus Employee::stringToStatus(const QString &statusStr)
{
    QString status = statusStr.toLower();
    if (status.contains("active")) return Active;
    if (status.contains("inactive")) return Inactive;
    if (status.contains("leave")) return OnLeave;
    if (status.contains("terminated")) return Terminated;
    return Active;
}

void Employee::updateTimestamp()
{
    m_updatedAt = QDateTime::currentDateTime();
}
