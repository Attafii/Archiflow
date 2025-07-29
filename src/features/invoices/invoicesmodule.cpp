#include "invoicesmodule.h"
#include "invoicewidget.h"
#include "invoicedatabasemanager.h"
#include <QDebug>

InvoicesModule::InvoicesModule(QObject *parent)
    : BaseModule("invoices", parent)
    , m_invoiceWidget(nullptr)
    , m_dbManager(nullptr)
    , m_widgetCreated(false)
{
}

bool InvoicesModule::initialize()
{
    if (isInitialized()) {
        return true;
    }

    qDebug() << "Initializing Invoices module...";

    // Create and initialize the database manager
    if (!m_dbManager) {
        m_dbManager = new InvoiceDatabaseManager(this);
        if (!m_dbManager->initialize()) {
            qWarning() << "Failed to initialize invoice database";
            delete m_dbManager;
            m_dbManager = nullptr;
            return false;
        }
    }

    setInitialized(true);
    emit moduleInitialized();
    
    qDebug() << "Invoices module initialized successfully";
    return true;
}

void InvoicesModule::shutdown()
{
    if (!isInitialized()) {
        return;
    }

    qDebug() << "Shutting down Invoices module...";

    // Clean up any module-specific resources
    if (m_invoiceWidget) {
        m_invoiceWidget->deleteLater();
        m_invoiceWidget = nullptr;
    }
    
    if (m_dbManager) {
        m_dbManager->deleteLater();
        m_dbManager = nullptr;
    }
    
    m_widgetCreated = false;
    setInitialized(false);
    emit moduleShuttingDown();
    
    qDebug() << "Invoices module shutdown complete";
}

QWidget* InvoicesModule::createWidget(QWidget *parent)
{
    if (!isInitialized()) {
        qWarning() << "Cannot create widget for uninitialized Invoices module";
        return nullptr;
    }

    // Create widget only once (singleton pattern for module widget)
    if (!m_widgetCreated) {
        m_invoiceWidget = new InvoiceWidget(parent);
        
        // Set the database manager if available
        if (m_dbManager) {
            m_invoiceWidget->setDatabaseManager(m_dbManager);
        }
        
        m_widgetCreated = true;
        qDebug() << "Invoices widget created and configured";
    }

    return m_invoiceWidget;
}

QWidget* InvoicesModule::createSettingsWidget(QWidget *parent)
{
    // TODO: Implement invoices-specific settings widget
    Q_UNUSED(parent);
    return nullptr;
}

void InvoicesModule::setDatabaseManager(InvoiceDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
    qDebug() << "InvoicesModule: Database manager set";
    
    // If widget already exists, update it with the database manager
    if (m_invoiceWidget && m_dbManager) {
        m_invoiceWidget->setDatabaseManager(m_dbManager);
    }
}

InvoiceDatabaseManager* InvoicesModule::databaseManager() const
{
    return m_dbManager;
}
