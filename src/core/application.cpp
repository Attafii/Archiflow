#include "application.h"
#include "modulemanager.h"
#include "database/databasemanager.h"
#include "utils/environmentloader.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QPalette>

Application* Application::s_instance = nullptr;

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_initialized(false)
{
    s_instance = this;
    
    // Load environment variables from .env file
    EnvironmentLoader::loadFromFile();
    
    setupApplicationProperties();
    setupDarkTheme();
}

Application::~Application()
{
    shutdown();
    s_instance = nullptr;
}

Application* Application::instance()
{
    return s_instance;
}

bool Application::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing ArchiFlow Application...";

    setupDirectories();

    // Initialize settings
    m_settings = std::make_unique<QSettings>(
        QSettings::IniFormat,
        QSettings::UserScope,
        organizationName(),
        applicationName()
    );

    // Initialize database
    if (!setupDatabase()) {
        qCritical() << "Failed to initialize database";
        return false;
    }

    // Initialize modules
    if (!setupModules()) {
        qCritical() << "Failed to initialize modules";
        return false;
    }

    m_initialized = true;
    emit applicationInitialized();

    qDebug() << "ArchiFlow Application initialized successfully";
    return true;
}

void Application::shutdown()
{
    if (!m_initialized) {
        return;
    }

    qDebug() << "Shutting down ArchiFlow Application...";
    emit applicationShuttingDown();

    // Shutdown in reverse order
    m_moduleManager.reset();
    m_databaseManager.reset();
    m_settings.reset();

    m_initialized = false;
}

DatabaseManager* Application::databaseManager() const
{
    return m_databaseManager.get();
}

ModuleManager* Application::moduleManager() const
{
    return m_moduleManager.get();
}

QSettings* Application::settings() const
{
    return m_settings.get();
}

QString Application::applicationDataPath() const
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataPath.isEmpty()) {
        dataPath = QDir::homePath() + "/.archiflow";
    }
    return dataPath;
}

QString Application::databasePath() const
{
    return applicationDataPath() + "/archiflow.db";
}

void Application::setupApplicationProperties()
{
    setApplicationName("ArchiFlow");
    setApplicationDisplayName("ArchiFlow - Architecture Office Management");
    setApplicationVersion("1.0.0");
    setOrganizationName("ArchiFlow Solutions");
    setOrganizationDomain("archiflow.com");
}

void Application::setupDarkTheme()
{
    // Force ArchiFlow color scheme
    QPalette archiFlowPalette;
    
    // Window colors - Primary Dark
    archiFlowPalette.setColor(QPalette::Window, QColor(61, 72, 90)); // #3D485A
    archiFlowPalette.setColor(QPalette::WindowText, QColor(227, 198, 176)); // #E3C6B0
    
    // Base colors (for input fields) - Accent Dark
    archiFlowPalette.setColor(QPalette::Base, QColor(42, 51, 64)); // #2A3340
    archiFlowPalette.setColor(QPalette::AlternateBase, QColor(61, 72, 90)); // #3D485A
    
    // Text colors - Primary Light
    archiFlowPalette.setColor(QPalette::Text, QColor(227, 198, 176)); // #E3C6B0
    archiFlowPalette.setColor(QPalette::BrightText, QColor(227, 198, 176)); // #E3C6B0
    
    // Button colors - Accent Dark
    archiFlowPalette.setColor(QPalette::Button, QColor(42, 51, 64)); // #2A3340
    archiFlowPalette.setColor(QPalette::ButtonText, QColor(227, 198, 176)); // #E3C6B0
    
    // Highlight colors - Info Blue
    archiFlowPalette.setColor(QPalette::Highlight, QColor(66, 165, 245)); // #42A5F5
    archiFlowPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    
    // Set the palette globally
    setPalette(archiFlowPalette);
    
    qDebug() << "ArchiFlow color palette applied";
}

void Application::setupDirectories()
{
    QDir dataDir(applicationDataPath());
    if (!dataDir.exists()) {
        dataDir.mkpath(".");
        qDebug() << "Created application data directory:" << dataDir.absolutePath();
    }
}

bool Application::setupDatabase()
{
    m_databaseManager = std::make_unique<DatabaseManager>(this);
    
    connect(m_databaseManager.get(), &DatabaseManager::connected,
            this, &Application::onDatabaseConnected);
    connect(m_databaseManager.get(), &DatabaseManager::error,
            this, &Application::onDatabaseError);

    return m_databaseManager->initialize(databasePath());
}

bool Application::setupModules()
{
    m_moduleManager = std::make_unique<ModuleManager>(this);
    
    if (!m_moduleManager->initialize()) {
        return false;
    }
    
    // Connect database to modules after both are initialized
    if (m_databaseManager) {
        m_moduleManager->setDatabaseManager(m_databaseManager.get());
    }
    
    return true;
}

void Application::onDatabaseConnected()
{
    qDebug() << "Database connected successfully";
}

void Application::onDatabaseError(const QString &error)
{
    qCritical() << "Database error:" << error;
}
