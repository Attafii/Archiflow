#ifndef EMPLOYEESMODULE_H
#define EMPLOYEESMODULE_H

#include "../../core/modulemanager.h"

class EmployeeWidget;
class EmployeeDatabaseManager;

/**
 * @brief The EmployeesModule class provides the employees feature module
 * 
 * This module manages the employee management feature, providing access to
 * employee widgets and database management functionality.
 */
class EmployeesModule : public BaseModule
{
    Q_OBJECT

public:
    explicit EmployeesModule(QObject *parent = nullptr);
    ~EmployeesModule() override;

    // BaseModule interface
    QString displayName() const override;
    QString description() const override;
    QWidget* createWidget(QWidget *parent = nullptr) override;
    bool initialize() override;
    void shutdown() override;

    // Employee-specific methods
    EmployeeDatabaseManager* databaseManager() const;

private:
    EmployeeWidget *m_widget;
    EmployeeDatabaseManager *m_dbManager;
};

#endif // EMPLOYEESMODULE_H
