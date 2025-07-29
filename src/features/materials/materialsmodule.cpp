#include "materialsmodule.h"
#include "materialwidget.h"
#include <QDebug>

MaterialsModule::MaterialsModule(QObject *parent)
    : BaseModule("materials", parent)
    , m_materialWidget(nullptr)
    , m_widgetCreated(false)
{
}

bool MaterialsModule::initialize()
{
    if (isInitialized()) {
        return true;
    }

    qDebug() << "Initializing Materials module...";

    // Initialize module-specific resources or services here
    // For example: database connections, configurations, etc.

    setInitialized(true);
    emit moduleInitialized();
    
    qDebug() << "Materials module initialized successfully";
    return true;
}

void MaterialsModule::shutdown()
{
    if (!isInitialized()) {
        return;
    }

    qDebug() << "Shutting down Materials module...";

    // Clean up any module-specific resources
    if (m_materialWidget) {
        m_materialWidget->deleteLater();
        m_materialWidget = nullptr;
    }
    
    m_widgetCreated = false;
    setInitialized(false);
    emit moduleShuttingDown();
    
    qDebug() << "Materials module shutdown complete";
}

QWidget* MaterialsModule::createWidget(QWidget *parent)
{
    if (!isInitialized()) {
        qWarning() << "Cannot create widget for uninitialized Materials module";
        return nullptr;
    }

    // Create widget only once (singleton pattern for module widget)
    if (!m_widgetCreated) {
        m_materialWidget = new MaterialWidget(parent);
        m_widgetCreated = true;
        qDebug() << "Materials widget created";
    }

    return m_materialWidget;
}

QWidget* MaterialsModule::createSettingsWidget(QWidget *parent)
{
    // TODO: Implement materials-specific settings widget
    Q_UNUSED(parent);
    return nullptr;
}
