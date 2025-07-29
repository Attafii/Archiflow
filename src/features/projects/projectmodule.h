#ifndef PROJECTMODULE_H
#define PROJECTMODULE_H

#include "core/modulemanager.h"

class ProjetWidget;
class ProjetManager;

/**
 * @brief The ProjectModule class manages the project feature module
 * 
 * This module provides comprehensive project management functionality
 * including CRUD operations, search/filter, and project visualization.
 */
class ProjectModule : public BaseModule
{
    Q_OBJECT

public:
    explicit ProjectModule(QObject *parent = nullptr);
    ~ProjectModule() override;    // BaseModule interface
    QString displayName() const override;
    QString description() const override;
    QWidget *createWidget(QWidget *parent = nullptr) override;
    bool initialize() override;
    void shutdown() override;

private:
    ProjetWidget *m_projectWidget;
    ProjetManager *m_projectManager;
};

#endif // PROJECTMODULE_H
