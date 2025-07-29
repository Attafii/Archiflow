#ifndef INVOICEWIDGET_H
#define INVOICEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QSplitter>
#include <QProgressBar>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QListWidget>
#include <QChartView>
#include <QBarSeries>
#include <QPieSeries>

class Invoice;
class InvoiceItem;
class Client;
class InvoiceDatabaseManager;
class InvoiceDialog;
class ClientDialog;
class InvoicePDFGenerator;
class GroqClient;
class InvoiceAIAssistantDialog;

/**
 * @brief The InvoiceWidget class provides the main interface for invoice management
 * 
 * This widget displays invoices, clients, and provides comprehensive invoice
 * management functionality including AI assistance for invoice processing.
 */
class InvoiceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InvoiceWidget(QWidget *parent = nullptr);
    ~InvoiceWidget();

    // Public interface
    void refreshInvoices();
    void refreshClients();
    void clearSelection();
    void setDatabaseManager(InvoiceDatabaseManager *dbManager);

public slots:
    void addInvoice();
    void editInvoice();
    void deleteInvoice();
    void duplicateInvoice();
    void viewInvoice();
    void generatePDF();
    void sendInvoice();
    void markAsPaid();
    void markAsOverdue();
    
    void addClient();
    void editClient();
    void deleteClient();
    
    void showInvoiceStatistics();
    void exportInvoices();
    void importInvoices();
    void openAIAssistant();

private slots:
    // UI event handlers
    void onInvoiceSelectionChanged();
    void onClientSelectionChanged();
    void onSearchTextChanged();
    void onStatusFilterChanged();
    void onDateRangeChanged();
    void onTabChanged(int index);
    
    // CRUD operations
    void onAddInvoiceClicked();
    void onEditInvoiceClicked();
    void onDeleteInvoiceClicked();
    void onDuplicateInvoiceClicked();
    void onViewInvoiceClicked();
    void onGeneratePDFClicked();
    void onSendInvoiceClicked();
    void onMarkAsPaidClicked();
    
    void onAddClientClicked();
    void onEditClientClicked();
    void onDeleteClientClicked();
    
    // Context menu handlers
    void showInvoiceContextMenu(const QPoint &pos);
    void showClientContextMenu(const QPoint &pos);
    
    // Database event handlers
    void onInvoiceAdded(const QString &invoiceId);
    void onInvoiceUpdated(const QString &invoiceId);
    void onInvoiceDeleted(const QString &invoiceId);
    void onClientAdded(const QString &clientId);
    void onClientUpdated(const QString &clientId);
    void onClientDeleted(const QString &clientId);
    void onDatabaseError(const QString &error);
    
    // Dashboard updates
    void updateDashboard();
    void updateStatistics();
    void updateCharts();
    void updateRecentActivity();

private:
    void setupUI();
    void setupDashboardTab();
    void setupInvoiceTab();
    void setupClientTab();
    void setupConnections();
    void populateInvoiceTable();
    void populateClientTable();
    void showMessage(const QString &message, bool isError = false);
    QString generateInvoiceNumber() const;
    
    // Missing method declarations
    void setupInvoiceDetailPanel();
    void updateActionStates();
    void applyFilters();
    
    // Invoice table management
    void addInvoiceToTable(const Invoice *invoice, int row = -1);
    void updateInvoiceInTable(const Invoice *invoice, int row);
    void removeInvoiceFromTable(int row);
    QString getSelectedInvoiceId() const;
    Invoice* getSelectedInvoice() const;
    
    // Client table management
    void addClientToTable(const Client *client, int row = -1);
    void updateClientInTable(const Client *client, int row);
    void removeClientFromTable(int row);
    QString getSelectedClientId() const;
    Client* getSelectedClient() const;
    
    // Dashboard widgets
    QWidget* createStatCard(const QString &title, const QString &value, const QString &subtitle = QString());
    void updateStatCard(QWidget *card, const QString &value, const QString &subtitle = QString());
    
    // AI Assistant integration
    void initializeAIAssistant();
    void showAISetupDialog();

    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Invoice tab
    QWidget *m_invoiceTab;
    QVBoxLayout *m_invoiceLayout;
    QHBoxLayout *m_invoiceToolbar;
    QHBoxLayout *m_invoiceFilters;
    QSplitter *m_invoiceSplitter;
    
    QLineEdit *m_invoiceSearchEdit;
    QComboBox *m_statusFilterCombo;
    QDateEdit *m_dateFromEdit;
    QDateEdit *m_dateToEdit;
    QPushButton *m_clearFiltersBtn;
    
    QTableWidget *m_invoiceTable;
    
    // Invoice detail panel
    QWidget *m_invoiceDetailPanel;
    QVBoxLayout *m_invoiceDetailLayout;
    QLabel *m_invoiceDetailTitle;
    QLabel *m_invoiceNumberLabel;
    QLabel *m_clientNameLabel;
    QLabel *m_invoiceDateLabel;
    QLabel *m_dueDateLabel;
    QLabel *m_totalAmountLabel;
    QLabel *m_statusLabel;
    QTextEdit *m_notesDisplay;
    QTableWidget *m_itemsTable;
    
    // Client tab
    QWidget *m_clientTab;
    QVBoxLayout *m_clientLayout;
    QHBoxLayout *m_clientToolbar;
    QLineEdit *m_clientSearchEdit;
    QTableWidget *m_clientTable;
    
    // Dashboard tab
    QWidget *m_dashboardTab;
    QVBoxLayout *m_dashboardLayout;
    QWidget *m_statsContainer;
    QHBoxLayout *m_statsLayout;
    
    // Dashboard cards
    QWidget *m_totalRevenueCard;
    QWidget *m_totalInvoicesCard;
    QWidget *m_paidInvoicesCard;
    QWidget *m_overdueInvoicesCard;
    QWidget *m_draftInvoicesCard;
    QWidget *m_pendingAmountCard;
    
    // Charts
    QChartView *m_revenueChart;
    QChartView *m_statusChart;
    QBarSeries *m_revenueBarSeries;
    QPieSeries *m_statusPieSeries;
    
    // Recent activity
    QListWidget *m_recentActivityList;
    
    // Action buttons
    QPushButton *m_addInvoiceBtn;
    QPushButton *m_editInvoiceBtn;
    QPushButton *m_deleteInvoiceBtn;
    QPushButton *m_duplicateInvoiceBtn;
    QPushButton *m_viewInvoiceBtn;
    QPushButton *m_generatePDFBtn;
    QPushButton *m_sendInvoiceBtn;
    QPushButton *m_markPaidBtn;
    QPushButton *m_aiAssistantBtn;
    
    QPushButton *m_addClientBtn;
    QPushButton *m_editClientBtn;
    QPushButton *m_deleteClientBtn;
    
    QPushButton *m_statisticsBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_importBtn;
    
    // Context menus
    QMenu *m_invoiceContextMenu;
    QMenu *m_clientContextMenu;
      // Data and services
    InvoiceDatabaseManager *m_dbManager;
    InvoicePDFGenerator *m_pdfGenerator;
    GroqClient *m_groqClient;
    InvoiceAIAssistantDialog *m_aiDialog;
    QTimer *m_searchTimer;
    bool m_isLoading;
    int m_currentTab;
    QList<Invoice*> m_invoices;
    QList<Client*> m_clients;
    QList<Invoice*> m_filteredInvoices;
    
    // Search and filtering
    QString m_currentSearchTerm;
    QString m_currentStatusFilter;
    QDate m_dateRangeStart;
    QDate m_dateRangeEnd;
    
    // Table columns
    enum InvoiceColumnIndex {
        InvoiceNumberColumn = 0,
        ClientNameColumn,
        InvoiceDateColumn,
        DueDateColumn,
        TotalAmountColumn,
        StatusColumn,
        InvoiceColumnCount
    };
    
    enum ClientColumnIndex {
        ClientNameCol = 0,
        ClientEmailCol,
        ClientPhoneCol,
        ClientCompanyCol,
        ClientColumnCount
    };
};

#endif // INVOICEWIDGET_H
