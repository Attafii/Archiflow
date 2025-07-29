#ifndef IMODULEINTERFACE_H
#define IMODULEINTERFACE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QAction>
#include <QList>

class ModuleManager;
class IDatabaseManager;

/**
 * @brief Interface for application modules
 * 
 * This interface defines the contract for all feature modules in the ArchiFlow application,
 * ensuring consistent integration and lifecycle management.
 */
class IModuleInterface
{
public:
    virtual ~IModuleInterface() = default;

    // Module identification
    virtual QString moduleName() const = 0;
    virtual QString displayName() const = 0;
    virtual QString description() const = 0;
    virtual QString version() const = 0;

    // Module dependencies
    virtual QStringList dependencies() const = 0;

    // Module lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;

    // UI integration
    virtual QWidget* createWidget(QWidget *parent = nullptr) = 0;
    virtual QWidget* createSettingsWidget(QWidget *parent = nullptr) = 0;

    // Menu and toolbar integration
    virtual QList<QAction*> getMenuActions() = 0;
    virtual QList<QAction*> getToolbarActions() = 0;

    // Configuration
    virtual bool loadConfiguration() = 0;
    virtual bool saveConfiguration() = 0;

    // External dependencies
    virtual void setModuleManager(ModuleManager *manager) = 0;
    virtual void setDatabaseManager(IDatabaseManager *dbManager) = 0;
};

/**
 * @brief QObject-based interface for modules with signals
 * 
 * This interface extends IModuleInterface with Qt's signal-slot mechanism
 * for event-driven module operations.
 */
class IModuleSignals : public QObject, public IModuleInterface
{
    Q_OBJECT

public:
    explicit IModuleSignals(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IModuleSignals() = default;

signals:
    void moduleInitialized();
    void moduleShuttingDown();
    void moduleError(const QString &error);
    void configurationChanged();
    void dataChanged();
};

#endif // IMODULEINTERFACE_H
