#include "clientsmodule.h"
#include "clientwidget.h" // ClientWidget uses ClientContact
#include "clientdatabasemanager.h" // ClientDatabaseManager uses ClientContact
#include <QDebug>

ClientsModule::ClientsModule(QObject *parent)
    : BaseModule("clients", parent)
    , m_widget(nullptr)
    , m_dbManager(nullptr)
{
    qDebug() << "ClientsModule: Created";
}

ClientsModule::~ClientsModule()
{
    shutdown();
    qDebug() << "ClientsModule: Destroyed";
}

QString ClientsModule::displayName() const
{
    return "Client Management";
}

QString ClientsModule::description() const
{
    return "Comprehensive client management system with contact information, "
           "address management, map integration, and AI-powered insights.";
}

QWidget* ClientsModule::createWidget(QWidget *parent)
{
    if (!isInitialized()) {
        initialize();
    }
    
    if (!m_widget) {
        m_widget = new ClientWidget(parent);
        
        // Set database manager
        if (m_dbManager) {
            m_widget->setDatabaseManager(m_dbManager);
        }
        
        qDebug() << "ClientsModule: Widget created";
    }
    
    return m_widget;
}

bool ClientsModule::initialize()
{
    if (isInitialized()) {
        return true;
    }
    
    qDebug() << "ClientsModule: Initializing...";
    
    // Initialize database manager
    m_dbManager = new ClientDatabaseManager(this);
    if (!m_dbManager->initialize()) {
        qWarning() << "ClientsModule: Failed to initialize database:" << m_dbManager->lastError();
        return false;
    } else {
        qDebug() << "ClientsModule: Database initialized successfully";
    }
    
    setInitialized(true);
    qDebug() << "ClientsModule: Initialization complete";
    return true;
}

void ClientsModule::shutdown()
{
    qDebug() << "ClientsModule: Shutting down...";
    
    if (m_widget) {
        m_widget->deleteLater();
        m_widget = nullptr;
    }
      if (m_dbManager) {
        m_dbManager->close();
        m_dbManager->deleteLater();
        m_dbManager = nullptr;
    }
    
    setInitialized(false);
    qDebug() << "ClientsModule: Shutdown complete";
}

ClientDatabaseManager* ClientsModule::databaseManager() const
{
    return m_dbManager;
}
