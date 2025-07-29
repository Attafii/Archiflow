#ifndef CONTRACTMODULE_H
#define CONTRACTMODULE_H

#include "core/modulemanager.h"
#include "../../interfaces/imoduleinterface.h"

class ContractWidget;
class ContractDatabaseManager;
class IContractService;

/**
 * @brief The ContractModule class manages the contract feature module
 * 
 * This module provides comprehensive contract management functionality
 * including CRUD operations, search/filter, contract statistics, and AI chatbot integration.
 */
class ContractModule : public BaseModule
{
    Q_OBJECT

public:
    explicit ContractModule(QObject *parent = nullptr);
    ~ContractModule() override;

    // BaseModule interface
    QString displayName() const override;
    QString description() const override;
    QWidget *createWidget(QWidget *parent = nullptr) override;
    bool initialize() override;
    void shutdown() override;    // Contract module specific methods
    ContractDatabaseManager* getDatabaseManager() const;
    IContractService* getContractService() const;
    void setExternalDatabaseManager(class DatabaseManager *dbManager);

    // Additional module interface methods
    QStringList dependencies() const;
    QList<QAction*> getMenuActions();
    QList<QAction*> getToolbarActions();
    bool loadConfiguration();
    bool saveConfiguration();
    void setModuleManager(ModuleManager *manager);
    void setDatabaseManager(IDatabaseManager *dbManager);

signals:
    void contractsLoaded();
    void contractAdded(const QString &contractId);
    void contractUpdated(const QString &contractId);
    void contractDeleted(const QString &contractId);

private slots:
    void onDatabaseInitialized();
    void onDatabaseError(const QString &error);

private:
    void setupDatabaseConnections();

    ContractWidget *m_contractWidget;
    ContractDatabaseManager *m_databaseManager;
    class DatabaseManager *m_externalDbManager;
    bool m_isDatabaseInitialized;
};

#endif // CONTRACTMODULE_H
