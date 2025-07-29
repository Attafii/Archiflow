#ifndef CLIENTSMODULE_H
#define CLIENTSMODULE_H

#include "../../core/modulemanager.h"

class ClientWidget;
class ClientDatabaseManager;

/**
 * @brief The ClientsModule class provides the clients feature module
 * 
 * This module manages the client management feature, providing access to
 * client widgets and database management functionality.
 */
class ClientsModule : public BaseModule
{
    Q_OBJECT

public:
    explicit ClientsModule(QObject *parent = nullptr);
    ~ClientsModule() override;

    // BaseModule interface
    QString displayName() const override;
    QString description() const override;
    QWidget* createWidget(QWidget *parent = nullptr) override;
    bool initialize() override;
    void shutdown() override;    // Client-specific methods
    ClientDatabaseManager* databaseManager() const;

private:
    ClientWidget *m_widget;
    ClientDatabaseManager *m_dbManager;
};

#endif // CLIENTSMODULE_H
