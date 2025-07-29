#include "projectmodule.h"
#include "projetmanager.h"
#include "ui/projects/projetwidget.h"
#include "database/databasemanager.h"
#include "core/application.h"
#include <QDebug>

ProjectModule::ProjectModule(QObject *parent)
    : BaseModule("projects", parent)
    , m_projectWidget(nullptr)
    , m_projectManager(nullptr)
{
}

ProjectModule::~ProjectModule()
{
    shutdown();
}

QString ProjectModule::displayName() const
{
    return "Projects";
}

QString ProjectModule::description() const
{
    return "Manage architecture projects including design phases, scheduling, and client information";
}

QWidget *ProjectModule::createWidget(QWidget *parent)
{
    if (!m_projectWidget) {
        if (!m_projectManager) {
            qWarning() << "ProjectModule: ProjetManager not initialized before creating widget";
            return nullptr;
        }
        
        m_projectWidget = new ProjetWidget(m_projectManager, parent);
        
        qDebug() << "ProjectModule: Widget created successfully";
    }
    
    return m_projectWidget;
}

bool ProjectModule::initialize()
{
    qDebug() << "ProjectModule: Initializing...";
    
    // Get the database manager from the application
    Application *app = archiFlowApp();
    if (!app) {
        qWarning() << "ProjectModule: Application not available during initialization";
        emit moduleError("Application not available during initialization");
        return false;
    }
    
    DatabaseManager *dbManager = app->databaseManager();
    if (!dbManager || !dbManager->isConnected()) {
        qWarning() << "ProjectModule: Database not available during initialization";
        emit moduleError("Database not available during initialization");
        return false;
    }
    
    // Initialize the project manager
    m_projectManager = new ProjetManager(dbManager, this);
    
    setInitialized(true);
    qDebug() << "ProjectModule: Initialized successfully";
    emit moduleInitialized();
    return true;
}

void ProjectModule::shutdown()
{
    qDebug() << "ProjectModule: Shutting down...";
    
    if (m_projectWidget) {
        m_projectWidget->deleteLater();
        m_projectWidget = nullptr;
    }
    
    if (m_projectManager) {
        m_projectManager->deleteLater();
        m_projectManager = nullptr;
    }
}
