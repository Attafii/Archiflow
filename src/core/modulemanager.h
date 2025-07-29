#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <QObject>
#include <QString>
#include <memory>
#include <map>

class QWidget;

/**
 * @brief Base class for all feature modules
 * 
 * Each feature (employees, clients, etc.) should inherit from this class
 * to provide a consistent interface for module management.
 */
class BaseModule : public QObject
{
    Q_OBJECT

public:
    explicit BaseModule(const QString &name, QObject *parent = nullptr);
    virtual ~BaseModule() = default;

    // Module information
    QString name() const { return m_name; }
    virtual QString displayName() const { return m_name; }
    virtual QString description() const { return QString(); }
    virtual QString version() const { return "1.0.0"; }

    // Module lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // UI integration
    virtual QWidget* createWidget(QWidget *parent = nullptr) = 0;
    virtual QWidget* createSettingsWidget(QWidget *parent = nullptr) { Q_UNUSED(parent); return nullptr; }

    // Module state
    bool isInitialized() const { return m_initialized; }

signals:
    void moduleInitialized();
    void moduleShuttingDown();
    void moduleError(const QString &error);

protected:
    void setInitialized(bool initialized) { m_initialized = initialized; }

private:
    QString m_name;
    bool m_initialized;
};

/**
 * @brief The ModuleManager class - Manages all feature modules
 * 
 * This class handles registration, initialization, and lifecycle management
 * of all feature modules in the application.
 */
class ModuleManager : public QObject
{
    Q_OBJECT

public:
    explicit ModuleManager(QObject *parent = nullptr);
    ~ModuleManager();

    // Module management
    bool initialize();
    void shutdown();

    // Module registration
    void registerModule(std::unique_ptr<BaseModule> module);
    BaseModule* module(const QString &name) const;    // Module access
    QStringList moduleNames() const;
    QList<BaseModule*> modules() const;
    
    // Dependencies injection
    void setDatabaseManager(class DatabaseManager *dbManager);

signals:
    void moduleRegistered(const QString &name);
    void moduleUnregistered(const QString &name);
    void allModulesInitialized();

private slots:
    void onModuleInitialized();
    void onModuleError(const QString &error);

private:
    void registerBuiltInModules();

    std::map<QString, std::unique_ptr<BaseModule>> m_modules;
    bool m_initialized;
};

#endif // MODULEMANAGER_H
