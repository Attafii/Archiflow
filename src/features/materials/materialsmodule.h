#ifndef MATERIALSMODULE_H
#define MATERIALSMODULE_H

#include "core/modulemanager.h"

class MaterialWidget;

/**
 * @brief The MaterialsModule class - Materials Management module
 * 
 * This module provides comprehensive materials management functionality
 * including material catalog, inventory tracking, supplier management,
 * and cost analysis.
 */
class MaterialsModule : public BaseModule
{
    Q_OBJECT

public:
    explicit MaterialsModule(QObject *parent = nullptr);
    ~MaterialsModule() override = default;

    // Module information
    QString displayName() const override { return "Materials Management"; }
    QString description() const override { 
        return "Comprehensive materials management system for tracking "
               "inventory, suppliers, and costs"; 
    }

    // Module lifecycle
    bool initialize() override;
    void shutdown() override;

    // UI integration
    QWidget* createWidget(QWidget *parent = nullptr) override;
    QWidget* createSettingsWidget(QWidget *parent = nullptr) override;

private:
    MaterialWidget *m_materialWidget;
    bool m_widgetCreated;
};

#endif // MATERIALSMODULE_H
