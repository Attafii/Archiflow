#include "employeesmodule.h"
#include "employeewidget.h"
#include "employeedatabasemanager.h"
#include <QDebug>

EmployeesModule::EmployeesModule(QObject *parent)
    : BaseModule("employees", parent)
    , m_widget(nullptr)
    , m_dbManager(nullptr)
{
    qDebug() << "EmployeesModule: Created";
}

EmployeesModule::~EmployeesModule()
{
    shutdown();
    qDebug() << "EmployeesModule: Destroyed";
}

QString EmployeesModule::displayName() const
{
    return "Employees";
}

QString EmployeesModule::description() const
{
    return "Comprehensive employee management system with CRUD operations, calendar integration, and analytics dashboard";
}

QWidget* EmployeesModule::createWidget(QWidget *parent)
{
    if (!m_widget) {
        m_widget = new EmployeeWidget(parent);
        
        // Set the database manager if we have one
        if (m_dbManager) {
            m_widget->setDatabaseManager(m_dbManager);
        }
        
        qDebug() << "EmployeesModule: Widget created";
    }
    
    return m_widget;
}

bool EmployeesModule::initialize()
{
    if (isInitialized()) {
        qDebug() << "EmployeesModule: Already initialized";
        return true;
    }
    
    qDebug() << "EmployeesModule: Initializing...";
    
    try {
        // Initialize database manager
        m_dbManager = new EmployeeDatabaseManager(this);
        if (!m_dbManager->initialize()) {
            qCritical() << "EmployeesModule: Failed to initialize database manager:" 
                       << m_dbManager->lastError();
            return false;
        }
        
        // If widget already exists, set the database manager
        if (m_widget) {
            m_widget->setDatabaseManager(m_dbManager);
        }
        
        setInitialized(true);
        emit moduleInitialized();
        
        qDebug() << "EmployeesModule: Initialization complete";
        return true;
        
    } catch (const std::exception &e) {
        qCritical() << "EmployeesModule: Exception during initialization:" << e.what();
        emit moduleError(QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

void EmployeesModule::shutdown()
{
    if (!isInitialized()) {
        return;
    }
    
    qDebug() << "EmployeesModule: Shutting down...";
    
    emit moduleShuttingDown();
    
    // Clean up widget
    if (m_widget) {
        m_widget->deleteLater();
        m_widget = nullptr;
    }
    
    // Clean up database manager
    if (m_dbManager) {
        m_dbManager->deleteLater();
        m_dbManager = nullptr;
    }
    
    setInitialized(false);
    
    qDebug() << "EmployeesModule: Shutdown complete";
}

EmployeeDatabaseManager* EmployeesModule::databaseManager() const
{
    return m_dbManager;
}
