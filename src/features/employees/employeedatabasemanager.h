#ifndef EMPLOYEEDATABASEMANAGER_H
#define EMPLOYEEDATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QDateTime>
#include "employee.h"

/**
 * @brief The EmployeeDatabaseManager class handles all database operations for employees
 * 
 * This class provides a comprehensive interface for employee data persistence,
 * including CRUD operations, search functionality, and database maintenance.
 */
class EmployeeDatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit EmployeeDatabaseManager(QObject *parent = nullptr);
    virtual ~EmployeeDatabaseManager();    // Database initialization and management
    bool initialize(const QString &dbPath = QString());
    bool openDatabase(const QString &dbPath = QString());
    void closeDatabase();
    bool isConnected() const;
    QString lastError() const;    // Employee CRUD operations
    bool addEmployee(const Employee &employee);
    bool updateEmployee(const Employee &employee);
    bool deleteEmployee(const QString &cin);
    Employee getEmployee(const QString &cin) const;
    QList<Employee*> getAllEmployees() const;

    // Search and filtering
    QList<Employee*> searchEmployees(const QString &searchTerm) const;
    QList<Employee*> getEmployeesByRole(Employee::EmployeeRole role) const;
    QList<Employee*> getEmployeesByStatus(Employee::EmployeeStatus status) const;
    QList<Employee*> getEmployeesByDepartment(const QString &department) const;
    QList<Employee*> getActiveEmployees() const;
    QList<Employee*> getPresentEmployees() const;

    // Validation and checks
    bool employeeExists(const QString &cin) const;
    bool emailExists(const QString &email, const QString &excludeCin = QString()) const;
    QStringList getAllDepartments() const;
    QStringList getAllPositions() const;

    // Statistics and reporting
    int getTotalEmployeeCount() const;
    int getActiveEmployeeCount() const;
    int getPresentEmployeeCount() const;
    QMap<Employee::EmployeeRole, int> getEmployeeCountByRole() const;
    QMap<Employee::EmployeeStatus, int> getEmployeeCountByStatus() const;
    QMap<QString, int> getEmployeeCountByDepartment() const;

    // Bulk operations
    bool addEmployees(const QList<Employee> &employees);
    bool updateEmployees(const QList<Employee> &employees);
    bool deleteEmployees(const QStringList &cins);

    // Database maintenance
    bool backupDatabase(const QString &backupPath);
    bool restoreDatabase(const QString &backupPath);
    bool optimizeDatabase();
    bool verifyDatabaseIntegrity() const;

    // Data export/import
    bool exportToJson(const QString &filePath) const;
    bool importFromJson(const QString &filePath);
    bool exportToCsv(const QString &filePath) const;
    bool importFromCsv(const QString &filePath);

signals:
    void employeeAdded(const QString &cin);
    void employeeUpdated(const QString &cin);
    void employeeDeleted(const QString &cin);
    void databaseError(const QString &error);
    void operationCompleted(const QString &operation, bool success);

private slots:
    void handleDatabaseError(const QString &error);

private:
    // Database setup and management
    bool createTables();
    bool updateDatabaseSchema();
    QString generateConnectionName() const;
    void logError(const QString &operation, const QSqlError &error);

    // Query helpers
    QSqlQuery prepareQuery(const QString &sql) const;
    bool executeQuery(QSqlQuery &query) const;
    Employee employeeFromQuery(const QSqlQuery &query) const;
    void bindEmployeeToQuery(QSqlQuery &query, const Employee &employee) const;

    // Validation helpers
    bool validateEmployee(const Employee &employee) const;
    QString sanitizeInput(const QString &input) const;

    QSqlDatabase m_database;
    QString m_connectionName;
    mutable QString m_lastError;
    bool m_isConnected;

    // Table and column names
    static const QString EMPLOYEE_TABLE;
    static const QString EMPLOYEE_EVENTS_TABLE;
    
    // SQL statements
    static const QString CREATE_EMPLOYEE_TABLE_SQL;
    static const QString CREATE_EMPLOYEE_EVENTS_TABLE_SQL;
    static const QString INSERT_EMPLOYEE_SQL;
    static const QString UPDATE_EMPLOYEE_SQL;
    static const QString DELETE_EMPLOYEE_SQL;
    static const QString SELECT_EMPLOYEE_SQL;
    static const QString SELECT_ALL_EMPLOYEES_SQL;
    static const QString SEARCH_EMPLOYEES_SQL;
};

#endif // EMPLOYEEDATABASEMANAGER_H
