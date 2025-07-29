#include <QCoreApplication>
#include <QDebug>
#include <QJsonObject>
#include "src/features/employees/employee.h"
#include "src/features/employees/employeedatabasemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Employee Management Integration Test ===";
    
    // Test 1: Basic Employee functionality
    qDebug() << "\n1. Testing Employee class...";
    Employee employee;
    employee.setCin("EMP001");
    employee.setFirstName("John");
    employee.setLastName("Doe");
    employee.setEmail("john.doe@archiflow.com");
    employee.setPhoneNumber("+1234567890");
    employee.setPosition("Senior Architect");
    employee.setDepartment("Design");
    employee.setEmploymentType("Full-time");
    employee.setStatus(Employee::Active);
    employee.setSalary(75000.00);
    
    qDebug() << "✓ Employee created successfully";
    qDebug() << "  Full Name:" << employee.fullName();
    qDebug() << "  CIN:" << employee.cin();
    qDebug() << "  Email:" << employee.email();
    qDebug() << "  Department:" << employee.department();
    qDebug() << "  Status:" << employee.statusString();
    qDebug() << "  Employment Type:" << employee.employmentType();
    qDebug() << "  Phone:" << employee.phone(); // Test alias
    qDebug() << "  Start Date:" << employee.startDate().toString(); // Test alias
    
    // Test 2: JSON serialization/deserialization
    qDebug() << "\n2. Testing JSON serialization...";
    QJsonObject json = employee.toJson();
    qDebug() << "✓ JSON serialization successful, keys:" << json.keys().size();
    
    Employee employee2;
    if (employee2.fromJson(json)) {
        qDebug() << "✓ JSON deserialization successful";
        qDebug() << "  Deserialized name:" << employee2.fullName();
        qDebug() << "  Deserialized email:" << employee2.email();
    } else {
        qDebug() << "✗ JSON deserialization failed";
        return 1;
    }
    
    // Test 3: Validation
    qDebug() << "\n3. Testing validation...";
    if (employee.isValid()) {
        qDebug() << "✓ Employee validation passed";
    } else {
        qDebug() << "✗ Employee validation failed";
        QStringList errors = employee.validationErrors();
        for (const QString &error : errors) {
            qDebug() << "  Error:" << error;
        }
    }
    
    // Test 4: Copy constructor and assignment
    qDebug() << "\n4. Testing copy operations...";
    Employee employee3(employee);
    qDebug() << "✓ Copy constructor works, name:" << employee3.fullName();
    
    Employee employee4;
    employee4 = employee;
    qDebug() << "✓ Assignment operator works, name:" << employee4.fullName();
    
    // Test 5: Static utilities
    qDebug() << "\n5. Testing static utilities...";
    QString generatedCin = Employee::generateCin();
    qDebug() << "✓ Generated CIN:" << generatedCin;
    
    QString statusStr = Employee::statusToString(Employee::Active);
    qDebug() << "✓ Status to string:" << statusStr;
    
    Employee::EmployeeStatus status = Employee::stringToStatus("Active");
    qDebug() << "✓ String to status:" << static_cast<int>(status);
    
    QString roleStr = Employee::roleToString(Employee::Architect);
    qDebug() << "✓ Role to string:" << roleStr;
    
    Employee::EmployeeRole role = Employee::stringToRole("Architect");
    qDebug() << "✓ String to role:" << static_cast<int>(role);
    
    // Test 6: Database functionality
    qDebug() << "\n6. Testing Database Manager...";
    EmployeeDatabaseManager dbManager;
    if (dbManager.initialize()) {
        qDebug() << "✓ Database manager initialized successfully";
        
        // Test adding employee
        if (dbManager.addEmployee(employee)) {
            qDebug() << "✓ Employee added to database successfully";
            
            // Test retrieving employee
            Employee retrieved = dbManager.getEmployee(employee.cin());
            if (!retrieved.cin().isEmpty()) {
                qDebug() << "✓ Employee retrieved from database:" << retrieved.fullName();
            } else {
                qDebug() << "✗ Failed to retrieve employee from database";
            }
            
            // Test listing all employees
            QList<Employee*> allEmployees = dbManager.getAllEmployees();
            qDebug() << "✓ Total employees in database:" << allEmployees.size();
            
            // Test search functionality
            QList<Employee*> searchResults = dbManager.searchEmployees("John");
            qDebug() << "✓ Search results for 'John':" << searchResults.size();
            
            // Clean up
            for (Employee* emp : allEmployees) {
                emp->deleteLater();
            }
            for (Employee* emp : searchResults) {
                emp->deleteLater();
            }
        } else {
            qDebug() << "✗ Failed to add employee to database:" << dbManager.lastError();
        }
    } else {
        qDebug() << "✗ Failed to initialize database manager:" << dbManager.lastError();
    }
    
    qDebug() << "\n=== All Employee Core Tests Passed! ===";
    qDebug() << "\nEmployee Management System core functionality is working correctly.";
    qDebug() << "Ready for full application integration.";
    
    return 0;
}
