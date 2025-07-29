#ifndef CONTRACTWIDGET_H
#define CONTRACTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QProgressBar>
#include <QTimer>
#include "../../interfaces/icontractwidget.h"

class Contract;
class ContractDatabaseManager;
class ContractDialog;
class IContractService;
class ContractExportManager;
class IContractChatbot;
class IContractImporter;
class ContractChatbotDialog;
class ContractImportDialog;
class GroqClient;
class ContractAIAssistantDialog;
class GroqContractChatbot;

/**
 * @brief The ContractWidget class provides the main interface for contract management
 * 
 * This widget displays a list of contracts with search, filter, and CRUD operations.
 * It follows ArchiFlow's design principles and integrates with the contract database.
 * Implements IContractWidget interface for consistent widget behavior.
 */
class ContractWidget : public QWidget, public IContractWidget
{
    Q_OBJECT

public:
    explicit ContractWidget(QWidget *parent = nullptr);
    ~ContractWidget();

    // IContractWidget interface implementation
    void setContractService(IContractService *service) override;
    void refreshContracts() override;
    void clearSelection() override;
    void addContract() override;
    void editContract(const QString &contractId) override;
    void deleteContract(const QString &contractId) override;
    void duplicateContract(const QString &contractId) override;
    void setSearchFilter(const QString &filter) override;
    void setStatusFilter(const QString &status) override;
    void setDateRangeFilter(const QDate &startDate, const QDate &endDate) override;
    void clearFilters() override;
    QList<QString> getSelectedContractIds() override;
    Contract* getCurrentContract() override;
    void selectContract(const QString &contractId) override;
    void exportSelectedContracts() override;
    void exportAllContracts() override;
    void setViewMode(int mode) override;
    void setSortOrder(int column, bool ascending = true) override;
    void setColumnsVisible(const QStringList &columns) override;    void showStatistics() override;
    void showContractDetails(const QString &contractId) override;
    
    // Legacy method for backward compatibility
    void setDatabaseManager(ContractDatabaseManager *dbManager);
    
    // Service setters for new features
    void setChatbot(IContractChatbot *chatbot);
    void setImporter(IContractImporter *importer);
    void setExportManager(ContractExportManager *exportManager);

signals:
    // Contract operation signals
    void contractAdded(const QString &contractId);
    void contractUpdated(const QString &contractId);
    void contractDeleted(const QString &contractId);
    void contractSelected(const QString &contractId);
    void contractDoubleClicked(const QString &contractId);
    void selectionChanged();
    void filterChanged();
    void exportRequested();
    void statisticsRequested();
    void chatbotRequested();
    void importRequested();
    void errorOccurred(const QString &error);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // Contract operations
    void onAddContractClicked();
    void onEditContractClicked();
    void onDeleteContractClicked();
    void onDuplicateContractClicked();
    void onViewContractDetailsClicked();

    // Search and filter
    void onSearchTextChanged();
    void onFilterChanged();
    void onDateRangeChanged();
    void onClearFiltersClicked();

    // Table operations
    void onContractSelectionChanged();
    void onContractDoubleClicked();
    void onTableContextMenu(const QPoint &pos);

    // Database signals
    void onContractAdded(const QString &contractId);
    void onContractUpdated(const QString &contractId);
    void onContractDeleted(const QString &contractId);
    void onDatabaseError(const QString &error);    // UI updates
    void updateStatusBar();
    void updateActionStates();

    // Export operations
    void onExportClicked();
    void onExportToCSV();
    void onExportToPDF();
    void onExportToExcel();
    void onExportToJSON();
    void onShowExportDialog();    // AI and Import operations
    void onShowChatbotClicked();
    void openAIAssistant();
    void onImportContractsClicked();
    void onShowAdvancedSearchClicked();
    void onBulkOperationsClicked();

private:
    void setupUi();
    void setupActions(); // New method matching MaterialWidget
    void setupToolbar();
    void setupTable();
    void setupSearchAndFilter();
    void setupStatusBar();
    void setupConnections();
    void applyArchiFlowStyling();

    // Data operations
    void loadContracts();
    void populateTable(const QList<Contract*> &contracts);
    void addContractToTable(Contract *contract, int row = -1);
    void updateContractInTable(Contract *contract);
    void removeContractFromTable(const QString &contractId);

    // UI helpers
    QString formatCurrency(double value) const;
    QString formatDate(const QDate &date) const;
    QString getStatusIcon(const QString &status) const;
    QColor getStatusColor(const QString &status) const;
    Contract* getSelectedContract() const;
    QList<Contract*> getSelectedContracts() const;
    void showMessage(const QString &message, bool isError = false);

    // Filter helpers
    void applyFilters();
    bool matchesFilter(Contract *contract) const;    // UI Components
    QVBoxLayout *m_mainLayout;
    QToolBar *m_toolbar;
    
    // Actions widget - matching MaterialWidget structure
    QWidget *m_actionsWidget;
    QHBoxLayout *m_actionsLayout;
    
    // Search and filter section
    QWidget *m_filterWidget;
    QLineEdit *m_searchEdit;
    QComboBox *m_statusFilterCombo;
    QDateEdit *m_startDateFilter;
    QDateEdit *m_endDateFilter;
    QPushButton *m_clearFiltersButton;
    
    // Contract table
    QTableWidget *m_contractsTable;
    
    // Status bar
    QWidget *m_statusWidget;
    QLabel *m_statusLabel;
    QLabel *m_countLabel;
    QProgressBar *m_progressBar;
    
    // Actions
    QPushButton *m_addButton;
    QPushButton *m_editButton;    QPushButton *m_deleteButton;
    QPushButton *m_duplicateButton;
    QPushButton *m_viewDetailsButton;
    QPushButton *m_statisticsButton;
    QPushButton *m_exportButton;
    QPushButton *m_chatbotButton;
    QPushButton *m_importButton;
    QPushButton *m_advancedSearchButton;
    QPushButton *m_bulkOperationsButton;    // Data and state
    ContractDatabaseManager *m_dbManager;
    IContractService *m_contractService;
    ContractExportManager *m_exportManager;
    IContractChatbot *m_chatbot;
    IContractImporter *m_importer;
    
    // AI Assistant integration (initialized before m_searchTimer)
    GroqClient *m_groqClient;
    ContractAIAssistantDialog *m_aiDialog;
    GroqContractChatbot *m_groqChatbot;
    
    QList<Contract*> m_contracts;
    QList<Contract*> m_filteredContracts;
    QTimer *m_searchTimer;
    bool m_isLoading;
    int m_currentViewMode; // 0: List, 1: Grid, 2: Cards
    QStringList m_visibleColumns;
    
    // Table columns
    enum ColumnIndex {
        ClientNameColumn = 0,
        StartDateColumn,
        EndDateColumn,
        ValueColumn,
        StatusColumn,
        PaymentTermsColumn,
        DescriptionColumn,
        ColumnCount
    };

    // Enhanced CRUD operations
    void addMultipleContracts();
    void editMultipleContracts();
    void deleteMultipleContracts();
    void bulkUpdateStatus();
    void bulkExport();
    
    // Database synchronization
    void synchronizeDatabase();
    void optimizeDatabase();
    void backupDatabase();
    void restoreDatabase();    // Performance and caching
    void refreshDatabaseCache();
    void showDatabaseStatus();
    
    // AI Assistant methods
    void initializeAIAssistant();
    void showAISetupDialog();
};

#endif // CONTRACTWIDGET_H
