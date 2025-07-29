#include "modulemanager.h"
#include "features/materials/materialsmodule.h"
#include "features/materials/materialwidget.h"
#include "features/projects/projectmodule.h"
#include "ui/projects/projetwidget.h"
#include "features/contracts/contractmodule.h"
#include "features/invoices/invoicesmodule.h"
#include "features/invoices/invoicewidget.h"
#include "features/clients/clientsmodule.h"
#include "features/clients/clientwidget.h"
#include "features/employees/employeesmodule.h"
#include "features/employees/employeewidget.h"
#include "../database/databasemanager.h"
// #include "features/suppliers/suppliermodule.h"  // DISABLED
#include <QDebug>
#include <QWidget>

// BaseModule implementation
BaseModule::BaseModule(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_initialized(false)
{
}

// ModuleManager implementation
ModuleManager::ModuleManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
}

ModuleManager::~ModuleManager()
{
    shutdown();
}

bool ModuleManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing ModuleManager...";

    // Register built-in modules
    registerBuiltInModules();    // Initialize all registered modules
    bool allInitialized = true;
    for (auto& [name, module] : m_modules) {
        connect(module.get(), &BaseModule::moduleInitialized,
                this, &ModuleManager::onModuleInitialized);
        connect(module.get(), &BaseModule::moduleError,
                this, &ModuleManager::onModuleError);

        if (!module->initialize()) {
            qWarning() << "Failed to initialize module:" << module->name();
            allInitialized = false;
        } else {
            qDebug() << "Module initialized:" << module->name();
        }
    }

    m_initialized = allInitialized;
    
    if (m_initialized) {
        emit allModulesInitialized();
        qDebug() << "All modules initialized successfully";
    }

    return m_initialized;
}

void ModuleManager::shutdown()
{
    if (!m_initialized) {
        return;
    }

    qDebug() << "Shutting down ModuleManager...";    // Shutdown all modules in reverse order
    QList<BaseModule*> moduleList = modules();
    for (auto it = moduleList.rbegin(); it != moduleList.rend(); ++it) {
        (*it)->shutdown();
        qDebug() << "Module shutdown:" << (*it)->name();
    }

    m_modules.clear();
    m_initialized = false;
}

void ModuleManager::registerModule(std::unique_ptr<BaseModule> module)
{
    if (!module) {
        qWarning() << "Attempted to register null module";
        return;
    }    QString name = module->name();
    if (m_modules.find(name) != m_modules.end()) {
        qWarning() << "Module already registered:" << name;
        return;
    }

    qDebug() << "Registering module:" << name;
    m_modules[name] = std::move(module);
    emit moduleRegistered(name);
}

BaseModule* ModuleManager::module(const QString &name) const
{
    auto it = m_modules.find(name);
    return (it != m_modules.end()) ? it->second.get() : nullptr;
}

QStringList ModuleManager::moduleNames() const
{
    QStringList result;
    for (const auto& [name, module] : m_modules) {
        result.append(name);
    }
    return result;
}

QList<BaseModule*> ModuleManager::modules() const
{
    QList<BaseModule*> result;
    for (const auto& [name, module] : m_modules) {
        result.append(module.get());
    }
    return result;
}

void ModuleManager::onModuleInitialized()
{
    BaseModule *module = qobject_cast<BaseModule*>(sender());
    if (module) {
        qDebug() << "Module initialized signal received from:" << module->name();
    }
}

void ModuleManager::onModuleError(const QString &error)
{
    BaseModule *module = qobject_cast<BaseModule*>(sender());
    QString moduleName = module ? module->name() : "Unknown";
    qWarning() << "Module error from" << moduleName << ":" << error;
}

void ModuleManager::registerBuiltInModules()
{
    qDebug() << "Registering built-in modules...";    // Register Materials Management module
    registerModule(std::make_unique<MaterialsModule>());
    
    // Register Project Management module
    registerModule(std::make_unique<ProjectModule>());
    
    // Register Contract Management module
    registerModule(std::make_unique<ContractModule>());
    
    // Register Invoice Management module
    registerModule(std::make_unique<InvoicesModule>());
      // Register Client Management module
    registerModule(std::make_unique<ClientsModule>());
    
    // Register Employee Management module
    registerModule(std::make_unique<EmployeesModule>());
    
    // Register Supplier Management module - DISABLED
    // registerModule(std::make_unique<SupplierModule>());
    
    // TODO: Register other feature modules as they are implemented
    // Examples:
    // registerModule(std::make_unique<EmployeesModule>());
    
    qDebug() << "Built-in modules registration complete";
}

void ModuleManager::setDatabaseManager(DatabaseManager *dbManager)
{
    qDebug() << "Setting database manager for all modules";
    
    for (auto& [name, module] : m_modules) {        // Check if it's a MaterialsModule and connect the database
        if (MaterialsModule *materialsModule = qobject_cast<MaterialsModule*>(module.get())) {
            // Get the MaterialWidget and set database manager
            if (QWidget *widget = materialsModule->createWidget()) {
                if (MaterialWidget *materialWidget = qobject_cast<MaterialWidget*>(widget)) {
                    materialWidget->setDatabaseManager(dbManager);
                    qDebug() << "Database manager connected to MaterialsModule";
                }
            }
        }
          // Check if it's a ProjectModule and connect the database
        if (ProjectModule *projectModule = qobject_cast<ProjectModule*>(module.get())) {
            Q_UNUSED(projectModule)
            // The ProjectModule handles database internally through ProjetManager
            // which gets initialized during module initialization
            qDebug() << "Database manager connected to ProjectModule";
        }
          // Check if it's a ContractModule and connect the database
        if (ContractModule *contractModule = qobject_cast<ContractModule*>(module.get())) {
            contractModule->setExternalDatabaseManager(dbManager);
            qDebug() << "Database manager connected to ContractModule";
        }        // Check if it's an InvoicesModule and connect the database
        if (InvoicesModule *invoicesModule = qobject_cast<InvoicesModule*>(module.get())) {
            // Get the InvoiceWidget and set database manager
            if (QWidget *widget = invoicesModule->createWidget()) {
                if (InvoiceWidget *invoiceWidget = qobject_cast<InvoiceWidget*>(widget)) {
                    Q_UNUSED(invoiceWidget)
                    // The InvoiceWidget will handle database connection internally
                    // through its InvoiceDatabaseManager
                    qDebug() << "Database manager connected to InvoicesModule";
                }
            }
        }
        
        // Check if it's an EmployeesModule and connect the database
        if (EmployeesModule *employeesModule = qobject_cast<EmployeesModule*>(module.get())) {
            // Get the EmployeeWidget and set database manager
            if (QWidget *widget = employeesModule->createWidget()) {
                if (EmployeeWidget *employeeWidget = qobject_cast<EmployeeWidget*>(widget)) {
                    Q_UNUSED(employeeWidget)
                    // The EmployeeWidget will handle database connection internally
                    // through its EmployeeDatabaseManager
                    qDebug() << "Database manager connected to EmployeesModule";
                }
            }
        }
        
        // TODO: Add database connections for other modules as they are implemented
    }
}
