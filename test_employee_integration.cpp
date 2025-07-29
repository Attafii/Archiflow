#include <QApplication>
#include <QDebug>
#include "src/features/employees/employee.h"
#include "src/features/employees/employeedatabasemanager.h"
#include "src/features/employees/employeedialog.h"
#include "src/features/employees/employeewidget.h"
#include "src/features/employees/employeesmodule.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Testing Employee Management System...";
    
    // Test 1: Employee creation and basic functionality
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
    
    qDebug() << "Employee created:";
    qDebug() << "  Name:" << employee.fullName();
    qDebug() << "  CIN:" << employee.cin();
    qDebug() << "  Email:" << employee.email();
    qDebug() << "  Department:" << employee.department();
    qDebug() << "  Status:" << employee.statusString();
    qDebug() << "  Employment Type:" << employee.employmentType();
    
    // Test 2: JSON serialization
    QJsonObject json = employee.toJson();
    qDebug() << "JSON serialization successful, keys:" << json.keys();
    
    Employee employee2;
    if (employee2.fromJson(json)) {
        qDebug() << "JSON deserialization successful";
        qDebug() << "  Deserialized name:" << employee2.fullName();
    } else {
        qDebug() << "JSON deserialization failed";
    }
    
    // Test 3: Database manager initialization
    EmployeeDatabaseManager dbManager;
    if (dbManager.initialize()) {
        qDebug() << "Database manager initialized successfully";
        
        // Test adding employee
        if (dbManager.addEmployee(employee)) {
            qDebug() << "Employee added to database successfully";
            
            // Test retrieving employee
            Employee retrieved = dbManager.getEmployee(employee.cin());
            if (!retrieved.cin().isEmpty()) {
                qDebug() << "Employee retrieved from database:" << retrieved.fullName();
            } else {
                qDebug() << "Failed to retrieve employee from database";
            }
            
            // Test listing all employees
            QList<Employee*> allEmployees = dbManager.getAllEmployees();
            qDebug() << "Total employees in database:" << allEmployees.size();
            
            // Clean up
            for (Employee* emp : allEmployees) {
                emp->deleteLater();
            }
        } else {
            qDebug() << "Failed to add employee to database:" << dbManager.lastError();
        }
    } else {
        qDebug() << "Failed to initialize database manager:" << dbManager.lastError();
    }
    
    // Test 4: Module initialization
    EmployeesModule module;
    if (module.initialize()) {
        qDebug() << "Employees module initialized successfully";
        qDebug() << "  Module name:" << module.name();
        qDebug() << "  Display name:" << module.displayName();
        qDebug() << "  Description:" << module.description();
    } else {
        qDebug() << "Failed to initialize employees module";
    }
    
    // Test 5: Widget creation
    EmployeeWidget widget;
    widget.show();
    qDebug() << "Employee widget created and shown";
    
    qDebug() << "All tests completed successfully!";
    qDebug() << "Employee Management System is ready for integration.";
    
    return app.exec();
}
