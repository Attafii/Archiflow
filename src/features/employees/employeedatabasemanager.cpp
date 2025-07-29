#include "employeedatabasemanager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QUuid>

// Static constants
const QString EmployeeDatabaseManager::EMPLOYEE_TABLE = "employees";
const QString EmployeeDatabaseManager::EMPLOYEE_EVENTS_TABLE = "employee_events";

const QString EmployeeDatabaseManager::CREATE_EMPLOYEE_TABLE_SQL = 
    "CREATE TABLE IF NOT EXISTS employees ("
    "cin TEXT PRIMARY KEY, "
    "first_name TEXT NOT NULL, "
    "last_name TEXT NOT NULL, "
    "email TEXT UNIQUE, "
    "phone_number TEXT, "
    "position TEXT, "
    "role INTEGER DEFAULT 6, "
    "hire_date TEXT, "
    "status INTEGER DEFAULT 0, "
    "is_present BOOLEAN DEFAULT 0, "
    "salary REAL DEFAULT 0.0, "
    "department TEXT, "
    "address TEXT, "
    "emergency_contact TEXT, "
    "emergency_phone TEXT, "
    "notes TEXT, "
    "created_at TEXT NOT NULL, "
    "updated_at TEXT NOT NULL"
    ")";

const QString EmployeeDatabaseManager::CREATE_EMPLOYEE_EVENTS_TABLE_SQL = 
    "CREATE TABLE IF NOT EXISTS employee_events ("
    "id TEXT PRIMARY KEY, "
    "employee_cin TEXT NOT NULL, "
    "event_type TEXT NOT NULL, "
    "start_time TEXT NOT NULL, "
    "end_time TEXT, "
    "description TEXT, "
    "created_at TEXT NOT NULL, "
    "FOREIGN KEY (employee_cin) REFERENCES employees (cin) ON DELETE CASCADE"
    ")";

const QString EmployeeDatabaseManager::INSERT_EMPLOYEE_SQL = 
    "INSERT INTO employees (cin, first_name, last_name, email, phone_number, position, "
    "role, hire_date, status, is_present, salary, department, address, "
    "emergency_contact, emergency_phone, notes, created_at, updated_at) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

const QString EmployeeDatabaseManager::UPDATE_EMPLOYEE_SQL = 
    "UPDATE employees SET first_name = ?, last_name = ?, email = ?, phone_number = ?, "
    "position = ?, role = ?, hire_date = ?, status = ?, is_present = ?, salary = ?, "
    "department = ?, address = ?, emergency_contact = ?, emergency_phone = ?, "
    "notes = ?, updated_at = ? WHERE cin = ?";

const QString EmployeeDatabaseManager::DELETE_EMPLOYEE_SQL = 
    "DELETE FROM employees WHERE cin = ?";

const QString EmployeeDatabaseManager::SELECT_EMPLOYEE_SQL = 
    "SELECT * FROM employees WHERE cin = ?";

const QString EmployeeDatabaseManager::SELECT_ALL_EMPLOYEES_SQL = 
    "SELECT * FROM employees ORDER BY last_name, first_name";

const QString EmployeeDatabaseManager::SEARCH_EMPLOYEES_SQL = 
    "SELECT * FROM employees WHERE "
    "cin LIKE ? OR first_name LIKE ? OR last_name LIKE ? OR "
    "email LIKE ? OR phone_number LIKE ? OR position LIKE ? OR "
    "department LIKE ? OR notes LIKE ? "
    "ORDER BY last_name, first_name";

EmployeeDatabaseManager::EmployeeDatabaseManager(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
{
    m_connectionName = generateConnectionName();
}

EmployeeDatabaseManager::~EmployeeDatabaseManager()
{
    closeDatabase();
}

bool EmployeeDatabaseManager::initialize(const QString &dbPath)
{
    qDebug() << "EmployeeDatabaseManager: Initializing...";
    
    if (!openDatabase(dbPath)) {
        return false;
    }
    
    if (!createTables()) {
        closeDatabase();
        return false;
    }
    
    qDebug() << "EmployeeDatabaseManager: Initialization successful";
    return true;
}

bool EmployeeDatabaseManager::openDatabase(const QString &dbPath)
{
    closeDatabase();

    QString databasePath = dbPath;
    if (databasePath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        databasePath = dataDir + "/employees.db";
    }

    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_database.setDatabaseName(databasePath);

    if (!m_database.open()) {
        m_lastError = tr("Failed to open database: %1").arg(m_database.lastError().text());
        qWarning() << "EmployeeDatabaseManager:" << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    // Enable foreign key constraints
    QSqlQuery query(m_database);
    query.exec("PRAGMA foreign_keys = ON");

    if (!createTables()) {
        closeDatabase();
        return false;
    }

    m_isConnected = true;
    qDebug() << "EmployeeDatabaseManager: Database opened successfully:" << databasePath;
    return true;
}

void EmployeeDatabaseManager::closeDatabase()
{
    if (m_isConnected) {
        m_database.close();
        QSqlDatabase::removeDatabase(m_connectionName);
        m_isConnected = false;
        qDebug() << "EmployeeDatabaseManager: Database closed";
    }
}

bool EmployeeDatabaseManager::isConnected() const
{
    return m_isConnected && m_database.isOpen();
}

QString EmployeeDatabaseManager::lastError() const
{
    return m_lastError;
}

bool EmployeeDatabaseManager::addEmployee(const Employee &employee)
{
    if (!isConnected()) {
        m_lastError = tr("Database not connected");
        return false;
    }

    if (!validateEmployee(employee)) {
        return false;
    }

    QSqlQuery query = prepareQuery(INSERT_EMPLOYEE_SQL);
    bindEmployeeToQuery(query, employee);

    if (!executeQuery(query)) {
        logError("addEmployee", query.lastError());
        return false;
    }

    emit employeeAdded(employee.cin());
    emit operationCompleted("addEmployee", true);
    return true;
}

bool EmployeeDatabaseManager::updateEmployee(const Employee &employee)
{
    if (!isConnected()) {
        m_lastError = tr("Database not connected");
        return false;
    }

    if (!validateEmployee(employee)) {
        return false;
    }

    QSqlQuery query = prepareQuery(UPDATE_EMPLOYEE_SQL);
    
    // Bind all fields except CIN (which is the WHERE condition)
    query.addBindValue(employee.firstName());
    query.addBindValue(employee.lastName());
    query.addBindValue(employee.email());
    query.addBindValue(employee.phoneNumber());
    query.addBindValue(employee.position());
    query.addBindValue(static_cast<int>(employee.role()));
    query.addBindValue(employee.hireDate().toString(Qt::ISODate));
    query.addBindValue(static_cast<int>(employee.status()));
    query.addBindValue(employee.isPresent());
    query.addBindValue(employee.salary());
    query.addBindValue(employee.department());
    query.addBindValue(employee.address());
    query.addBindValue(employee.emergencyContact());
    query.addBindValue(employee.emergencyPhone());
    query.addBindValue(employee.notes());
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.addBindValue(employee.cin()); // WHERE condition

    if (!executeQuery(query)) {
        logError("updateEmployee", query.lastError());
        return false;
    }

    emit employeeUpdated(employee.cin());
    emit operationCompleted("updateEmployee", true);
    return true;
}

bool EmployeeDatabaseManager::deleteEmployee(const QString &cin)
{
    if (!isConnected()) {
        m_lastError = tr("Database not connected");
        return false;
    }

    QSqlQuery query = prepareQuery(DELETE_EMPLOYEE_SQL);
    query.addBindValue(cin);

    if (!executeQuery(query)) {
        logError("deleteEmployee", query.lastError());
        return false;
    }

    emit employeeDeleted(cin);
    emit operationCompleted("deleteEmployee", true);
    return true;
}

Employee EmployeeDatabaseManager::getEmployee(const QString &cin) const
{
    Employee employee;
    
    if (!isConnected()) {
        return employee;
    }

    QSqlQuery query = prepareQuery(SELECT_EMPLOYEE_SQL);
    query.addBindValue(cin);

    if (executeQuery(query) && query.next()) {
        employee = employeeFromQuery(query);
    }

    return employee;
}

QList<Employee*> EmployeeDatabaseManager::getAllEmployees() const
{
    QList<Employee*> employees;
    
    if (!isConnected()) {
        return employees;
    }

    QSqlQuery query = prepareQuery(SELECT_ALL_EMPLOYEES_SQL);

    if (executeQuery(query)) {
        while (query.next()) {
            Employee* employee = new Employee();
            *employee = employeeFromQuery(query);
            employees.append(employee);
        }
    }

    return employees;
}

QList<Employee*> EmployeeDatabaseManager::searchEmployees(const QString &searchTerm) const
{
    QList<Employee*> employees;
    
    if (!isConnected() || searchTerm.trimmed().isEmpty()) {
        return employees;
    }

    QString term = "%" + searchTerm.trimmed() + "%";
    QSqlQuery query = prepareQuery(SEARCH_EMPLOYEES_SQL);
      // Bind search term to all LIKE placeholders
    for (int i = 0; i < 8; ++i) {
        query.addBindValue(term);
    }

    if (executeQuery(query)) {
        while (query.next()) {
            Employee* employee = new Employee();
            *employee = employeeFromQuery(query);
            employees.append(employee);
        }
    }

    return employees;
}

QList<Employee*> EmployeeDatabaseManager::getEmployeesByRole(Employee::EmployeeRole role) const
{
    QList<Employee*> employees;
    
    if (!isConnected()) {
        return employees;
    }

    QSqlQuery query = prepareQuery("SELECT * FROM employees WHERE role = ? ORDER BY last_name, first_name");
    query.addBindValue(static_cast<int>(role));

    if (executeQuery(query)) {
        while (query.next()) {
            Employee* employee = new Employee();
            *employee = employeeFromQuery(query);
            employees.append(employee);
        }
    }

    return employees;
}

QList<Employee*> EmployeeDatabaseManager::getEmployeesByStatus(Employee::EmployeeStatus status) const
{
    QList<Employee*> employees;
    
    if (!isConnected()) {
        return employees;
    }

    QSqlQuery query = prepareQuery("SELECT * FROM employees WHERE status = ? ORDER BY last_name, first_name");
    query.addBindValue(static_cast<int>(status));

    if (executeQuery(query)) {
        while (query.next()) {
            Employee* employee = new Employee();
            *employee = employeeFromQuery(query);
            employees.append(employee);
        }
    }

    return employees;
}

QList<Employee*> EmployeeDatabaseManager::getEmployeesByDepartment(const QString &department) const
{
    QList<Employee*> employees;
    
    if (!isConnected()) {
        return employees;
    }

    QSqlQuery query = prepareQuery("SELECT * FROM employees WHERE department = ? ORDER BY last_name, first_name");
    query.addBindValue(department);

    if (executeQuery(query)) {
        while (query.next()) {
            Employee* employee = new Employee();
            *employee = employeeFromQuery(query);
            employees.append(employee);
        }
    }

    return employees;
}

QList<Employee*> EmployeeDatabaseManager::getActiveEmployees() const
{
    return getEmployeesByStatus(Employee::Active);
}

QList<Employee*> EmployeeDatabaseManager::getPresentEmployees() const
{
    QList<Employee*> employees;
    
    if (!isConnected()) {
        return employees;
    }

    QSqlQuery query = prepareQuery("SELECT * FROM employees WHERE is_present = 1 ORDER BY last_name, first_name");

    if (executeQuery(query)) {
        while (query.next()) {
            Employee* employee = new Employee();
            *employee = employeeFromQuery(query);
            employees.append(employee);
        }
    }

    return employees;
}

bool EmployeeDatabaseManager::employeeExists(const QString &cin) const
{
    if (!isConnected()) {
        return false;
    }

    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM employees WHERE cin = ?");
    query.addBindValue(cin);

    if (executeQuery(query) && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool EmployeeDatabaseManager::emailExists(const QString &email, const QString &excludeCin) const
{
    if (!isConnected() || email.isEmpty()) {
        return false;
    }

    QString sql = "SELECT COUNT(*) FROM employees WHERE email = ?";
    QStringList params;
    params << email;

    if (!excludeCin.isEmpty()) {
        sql += " AND cin != ?";
        params << excludeCin;
    }

    QSqlQuery query = prepareQuery(sql);
    for (const QString &param : params) {
        query.addBindValue(param);
    }

    if (executeQuery(query) && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QStringList EmployeeDatabaseManager::getAllDepartments() const
{
    QStringList departments;
    
    if (!isConnected()) {
        return departments;
    }

    QSqlQuery query = prepareQuery("SELECT DISTINCT department FROM employees WHERE department IS NOT NULL AND department != '' ORDER BY department");

    if (executeQuery(query)) {
        while (query.next()) {
            departments.append(query.value(0).toString());
        }
    }

    return departments;
}

QStringList EmployeeDatabaseManager::getAllPositions() const
{
    QStringList positions;
    
    if (!isConnected()) {
        return positions;
    }

    QSqlQuery query = prepareQuery("SELECT DISTINCT position FROM employees WHERE position IS NOT NULL AND position != '' ORDER BY position");

    if (executeQuery(query)) {
        while (query.next()) {
            positions.append(query.value(0).toString());
        }
    }

    return positions;
}

int EmployeeDatabaseManager::getTotalEmployeeCount() const
{
    if (!isConnected()) {
        return 0;
    }

    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM employees");

    if (executeQuery(query) && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int EmployeeDatabaseManager::getActiveEmployeeCount() const
{
    if (!isConnected()) {
        return 0;
    }

    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM employees WHERE status = ?");
    query.addBindValue(static_cast<int>(Employee::Active));

    if (executeQuery(query) && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int EmployeeDatabaseManager::getPresentEmployeeCount() const
{
    if (!isConnected()) {
        return 0;
    }

    QSqlQuery query = prepareQuery("SELECT COUNT(*) FROM employees WHERE is_present = 1");

    if (executeQuery(query) && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// Helper methods
bool EmployeeDatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // Create employees table
    if (!query.exec(CREATE_EMPLOYEE_TABLE_SQL)) {
        logError("createTables (employees)", query.lastError());
        return false;
    }

    // Create employee events table
    if (!query.exec(CREATE_EMPLOYEE_EVENTS_TABLE_SQL)) {
        logError("createTables (employee_events)", query.lastError());
        return false;
    }

    return true;
}

QString EmployeeDatabaseManager::generateConnectionName() const
{
    return QString("EmployeeDB_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

void EmployeeDatabaseManager::logError(const QString &operation, const QSqlError &error)
{
    m_lastError = tr("%1 failed: %2").arg(operation, error.text());
    qWarning() << "EmployeeDatabaseManager:" << m_lastError;
    emit databaseError(m_lastError);
}

QSqlQuery EmployeeDatabaseManager::prepareQuery(const QString &sql) const
{
    QSqlQuery query(m_database);
    query.prepare(sql);
    return query;
}

bool EmployeeDatabaseManager::executeQuery(QSqlQuery &query) const
{
    if (!query.exec()) {
        const_cast<EmployeeDatabaseManager*>(this)->logError("executeQuery", query.lastError());
        return false;
    }
    return true;
}

Employee EmployeeDatabaseManager::employeeFromQuery(const QSqlQuery &query) const
{
    Employee employee;
    
    employee.setCin(query.value("cin").toString());
    employee.setFirstName(query.value("first_name").toString());
    employee.setLastName(query.value("last_name").toString());
    employee.setEmail(query.value("email").toString());
    employee.setPhoneNumber(query.value("phone_number").toString());
    employee.setPosition(query.value("position").toString());
    employee.setRole(static_cast<Employee::EmployeeRole>(query.value("role").toInt()));
    employee.setHireDate(QDateTime::fromString(query.value("hire_date").toString(), Qt::ISODate));
    employee.setStatus(static_cast<Employee::EmployeeStatus>(query.value("status").toInt()));
    employee.setIsPresent(query.value("is_present").toBool());
    employee.setSalary(query.value("salary").toDouble());
    employee.setDepartment(query.value("department").toString());
    employee.setAddress(query.value("address").toString());
    employee.setEmergencyContact(query.value("emergency_contact").toString());
    employee.setEmergencyPhone(query.value("emergency_phone").toString());
    employee.setNotes(query.value("notes").toString());
    employee.setCreatedAt(QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate));
    employee.setUpdatedAt(QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate));
    
    return employee;
}

void EmployeeDatabaseManager::bindEmployeeToQuery(QSqlQuery &query, const Employee &employee) const
{
    query.addBindValue(employee.cin());
    query.addBindValue(employee.firstName());
    query.addBindValue(employee.lastName());
    query.addBindValue(employee.email());
    query.addBindValue(employee.phoneNumber());
    query.addBindValue(employee.position());
    query.addBindValue(static_cast<int>(employee.role()));
    query.addBindValue(employee.hireDate().toString(Qt::ISODate));
    query.addBindValue(static_cast<int>(employee.status()));
    query.addBindValue(employee.isPresent());
    query.addBindValue(employee.salary());
    query.addBindValue(employee.department());
    query.addBindValue(employee.address());
    query.addBindValue(employee.emergencyContact());
    query.addBindValue(employee.emergencyPhone());
    query.addBindValue(employee.notes());
    query.addBindValue(employee.createdAt().toString(Qt::ISODate));
    query.addBindValue(employee.updatedAt().toString(Qt::ISODate));
}

bool EmployeeDatabaseManager::validateEmployee(const Employee &employee) const
{
    QStringList errors = employee.validationErrors();
    if (!errors.isEmpty()) {
        m_lastError = tr("Validation failed: %1").arg(errors.join("; "));
        return false;
    }
    return true;
}

QString EmployeeDatabaseManager::sanitizeInput(const QString &input) const
{
    return input.trimmed();
}

void EmployeeDatabaseManager::handleDatabaseError(const QString &error)
{
    qWarning() << "EmployeeDatabaseManager error:" << error;
}
