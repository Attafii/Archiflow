#include "contractmodule.h"
#include "contractwidget.h"
#include "contractdatabasemanager.h"
#include "database/databasemanager.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

ContractModule::ContractModule(QObject *parent)
    : BaseModule("contracts", parent)
    , m_contractWidget(nullptr)
    , m_databaseManager(nullptr)
    , m_externalDbManager(nullptr)
    , m_isDatabaseInitialized(false)
{
    qDebug() << "ContractModule created";
}

ContractModule::~ContractModule()
{
    shutdown();
}

QString ContractModule::displayName() const
{
    return "Contract Management";
}

QString ContractModule::description() const
{
    return "Comprehensive contract lifecycle management with CRUD operations, search, filtering, and AI integration";
}

QWidget* ContractModule::createWidget(QWidget *parent)
{
    if (!m_contractWidget) {
        m_contractWidget = new ContractWidget(parent);
        
        // Connect database manager if available
        if (m_databaseManager && m_isDatabaseInitialized) {
            m_contractWidget->setDatabaseManager(m_databaseManager);
        }
        
        // Connect module signals to widget signals
        connect(m_contractWidget, &ContractWidget::contractAdded,
                this, &ContractModule::contractAdded);
        connect(m_contractWidget, &ContractWidget::contractUpdated,
                this, &ContractModule::contractUpdated);
        connect(m_contractWidget, &ContractWidget::contractDeleted,
                this, &ContractModule::contractDeleted);
        
        qDebug() << "ContractWidget created and configured";
    }
    
    return m_contractWidget;
}

bool ContractModule::initialize()
{
    if (isInitialized()) {
        return true;
    }

    qDebug() << "Initializing ContractModule...";

    try {
        // Initialize database manager
        m_databaseManager = new ContractDatabaseManager(this);
        
        // Setup database connections
        setupDatabaseConnections();
        
        // Initialize database
        QString databasePath;
        if (m_externalDbManager) {
            // Use the same directory as the main database
            QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir().mkpath(appDataPath);
            databasePath = appDataPath + "/archiflow_contracts.db";
        }
        
        if (!m_databaseManager->initialize(databasePath)) {
            qWarning() << "Failed to initialize contract database:" << m_databaseManager->getLastError();
            return false;
        }
        
        m_isDatabaseInitialized = true;
        
        // If widget already exists, connect database
        if (m_contractWidget) {
            m_contractWidget->setDatabaseManager(m_databaseManager);
        }
        
        setInitialized(true);
        emit moduleInitialized();
        
        qDebug() << "ContractModule initialized successfully";
        return true;
        
    } catch (const std::exception &e) {
        qWarning() << "Exception during ContractModule initialization:" << e.what();
        return false;
    } catch (...) {
        qWarning() << "Unknown exception during ContractModule initialization";
        return false;
    }
}

void ContractModule::shutdown()
{
    if (!isInitialized()) {
        return;
    }

    qDebug() << "Shutting down ContractModule...";

    emit moduleShuttingDown();

    // Cleanup database
    if (m_databaseManager) {
        m_databaseManager->shutdown();
        m_databaseManager = nullptr;
    }

    // Widget will be deleted by parent
    m_contractWidget = nullptr;
    
    m_isDatabaseInitialized = false;
    setInitialized(false);
    
    qDebug() << "ContractModule shutdown complete";
}

ContractDatabaseManager* ContractModule::getDatabaseManager() const
{
    return m_databaseManager;
}

void ContractModule::setExternalDatabaseManager(DatabaseManager *dbManager)
{
    m_externalDbManager = dbManager;
    qDebug() << "External database manager set for ContractModule";
}

void ContractModule::onDatabaseInitialized()
{
    qDebug() << "Contract database initialized successfully";
    emit contractsLoaded();
}

void ContractModule::onDatabaseError(const QString &error)
{
    qWarning() << "Contract database error:" << error;
    emit moduleError("Database error: " + error);
}

void ContractModule::setupDatabaseConnections()
{
    if (!m_databaseManager) {
        return;
    }

    connect(m_databaseManager, &ContractDatabaseManager::contractAdded,
            this, &ContractModule::contractAdded);
    connect(m_databaseManager, &ContractDatabaseManager::contractUpdated,
            this, &ContractModule::contractUpdated);
    connect(m_databaseManager, &ContractDatabaseManager::contractDeleted,
            this, &ContractModule::contractDeleted);
    connect(m_databaseManager, &ContractDatabaseManager::databaseError,
            this, &ContractModule::onDatabaseError);
}
