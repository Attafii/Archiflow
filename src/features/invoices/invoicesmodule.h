#ifndef INVOICESMODULE_H
#define INVOICESMODULE_H

#include "../../core/modulemanager.h"

class InvoiceWidget;
class InvoiceDatabaseManager;

/**
 * @brief The InvoicesModule class - Invoice Management module
 * 
 * This module provides comprehensive invoice management functionality
 * including invoice creation, client management, PDF generation,
 * and AI-powered assistant features.
 */
class InvoicesModule : public BaseModule
{
    Q_OBJECT

public:
    explicit InvoicesModule(QObject *parent = nullptr);
    ~InvoicesModule() override = default;

    // Module information
    QString displayName() const override { return "Invoice Management"; }
    QString description() const override { 
        return "Comprehensive invoice management system for creating, "
               "tracking, and managing client invoices"; 
    }

    // Module lifecycle
    bool initialize() override;
    void shutdown() override;

    // UI integration
    QWidget* createWidget(QWidget *parent = nullptr) override;
    QWidget* createSettingsWidget(QWidget *parent = nullptr) override;
    
    // Database integration
    void setDatabaseManager(InvoiceDatabaseManager *dbManager);
    InvoiceDatabaseManager* databaseManager() const;

private:
    InvoiceWidget *m_invoiceWidget;
    InvoiceDatabaseManager *m_dbManager;
    bool m_widgetCreated;
};

#endif // INVOICESMODULE_H
