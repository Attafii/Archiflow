#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include "src/core/modulemanager.h"
#include "src/features/employees/employeesmodule.h"
#include "src/features/employees/employeewidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== Employee Module Integration Test ===";
    
    // Test 1: Module Manager and Registration
    qDebug() << "\n1. Testing Module Manager...";
    ModuleManager manager;
    
    if (manager.initialize()) {
        qDebug() << "✓ Module Manager initialized successfully";
        
        // List all registered modules
        QStringList moduleNames = manager.moduleNames();
        qDebug() << "✓ Registered modules:" << moduleNames.size();
        for (const QString &name : moduleNames) {
            qDebug() << "  -" << name;
        }
        
        // Test if Employees module is registered
        BaseModule *employeesModule = manager.module("Employees");
        if (employeesModule) {
            qDebug() << "✓ Employees module found:" << employeesModule->name();
            
            // Test widget creation
            QWidget *widget = employeesModule->createWidget();
            if (widget) {
                qDebug() << "✓ Employee widget created successfully";
                qDebug() << "  Widget class:" << widget->metaObject()->className();
                
                // Show the widget in a test window
                QMainWindow window;
                window.setWindowTitle("Employee Module Integration Test");
                window.setCentralWidget(widget);
                window.resize(800, 600);
                
                // Create a simple UI to test
                QWidget *testWidget = new QWidget;
                QVBoxLayout *layout = new QVBoxLayout(testWidget);
                
                QLabel *label = new QLabel("Employee Module Integration Test");
                label->setStyleSheet("font-size: 14px; font-weight: bold; margin: 10px;");
                layout->addWidget(label);
                
                layout->addWidget(widget);
                  QPushButton *testBtn = new QPushButton("Test Module Functions");
                QObject::connect(testBtn, &QPushButton::clicked, [widget]() {
                    if (EmployeeWidget *empWidget = qobject_cast<EmployeeWidget*>(widget)) {
                        QMessageBox::information(nullptr, "Success", 
                            "Employee Module is working!\nWidget class: " + 
                            QString(empWidget->metaObject()->className()));
                    }
                });
                layout->addWidget(testBtn);
                
                window.setCentralWidget(testWidget);
                window.show();
                
                qDebug() << "✓ Test window created and shown";
                qDebug() << "\n=== Integration test window opened ===";
                qDebug() << "Close the window to complete the test.";
                
                return app.exec();
                
            } else {
                qDebug() << "✗ Failed to create employee widget";
                return 1;
            }
        } else {
            qDebug() << "✗ Employees module not found";
            return 1;
        }
    } else {
        qDebug() << "✗ Failed to initialize Module Manager";
        return 1;
    }
    
    return 0;
}
