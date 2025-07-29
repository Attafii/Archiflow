#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QSplitter>
#include <QGroupBox>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QProgressBar>

class ClientContact;
class ClientDatabaseManager;
class ClientContactDialog;
class MapboxHandler;
class GroqClient;

/**
 * @brief The ClientWidget class provides the main interface for client management
 * 
 * This widget displays clients and provides comprehensive client
 * management functionality including map integration and AI insights.
 */
class ClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWidget(QWidget *parent = nullptr);
    ~ClientWidget();

    // Public interface
    void refreshClients();
    void clearSelection();
    void setDatabaseManager(ClientDatabaseManager *dbManager);

public slots:
    void addClient();
    void editClient();
    void deleteClient();
    void viewClient();
    void showClientOnMap();
    void getAIInsight();
    
    void exportClients();
    void importClients();
    void showStatistics();

private slots:
    // UI event handlers
    void onClientSelectionChanged();
    void onSearchTextChanged();
    void onCityFilterChanged();
    void onCountryFilterChanged();
    void onTabChanged(int index);
    
    // CRUD operations
    void onAddClientClicked();
    void onEditClientClicked();
    void onDeleteClientClicked();
    void onViewClientClicked();
    void onShowOnMapClicked();
    void onGetAIInsightClicked();
    
    // Context menu handlers
    void showClientContextMenu(const QPoint &pos);
    
    // Database event handlers
    void onClientAdded(const QString &clientId);
    void onClientUpdated(const QString &clientId);
    void onClientDeleted(const QString &clientId);
    void onDatabaseError(const QString &error);
    
    // Dashboard updates
    void updateDashboard();
    void updateStatistics();
    void updateRecentActivity();

private:
    void setupUI();
    void setupDashboardTab();
    void setupClientTab();
    void setupMapTab();
    void setupConnections();    void populateClientTable();
    void populateFilters();
    void applyFilters();
    void updateActionStates();
    void showMessage(const QString &message, bool isError = false);
    QString getSelectedClientId() const;
    ClientContact* getSelectedClient() const;
    
    // Client table management
    void addClientToTable(const ClientContact *client, int row = -1);
    void updateClientInTable(const ClientContact *client, int row);
    void removeClientFromTable(int row);
    
    // Dashboard widgets
    QWidget* createStatCard(const QString &title, const QString &value, const QString &subtitle = QString());
    void updateStatCard(QWidget *card, const QString &value, const QString &subtitle = QString());
    
    // Map integration
    void initializeMap();
    void updateMapMarkers();

    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Dashboard tab
    QWidget *m_dashboardTab;
    QVBoxLayout *m_dashboardLayout;
    QWidget *m_statsContainer;
    QHBoxLayout *m_statsLayout;
    
    // Dashboard cards
    QWidget *m_totalClientsCard;
    QWidget *m_activeClientsCard;
    QWidget *m_citiesCard;
    QWidget *m_countriesCard;
    
    // Recent activity
    QListWidget *m_recentActivityList;
    
    // Client tab
    QWidget *m_clientTab;
    QVBoxLayout *m_clientLayout;
    QHBoxLayout *m_clientToolbar;
    QHBoxLayout *m_clientFilters;
    QSplitter *m_clientSplitter;
    
    QLineEdit *m_searchEdit;
    QComboBox *m_cityFilterCombo;
    QComboBox *m_countryFilterCombo;
    QPushButton *m_clearFiltersBtn;
    
    QTableWidget *m_clientTable;
    
    // Client detail panel
    QWidget *m_clientDetailPanel;
    QVBoxLayout *m_clientDetailLayout;
    QLabel *m_clientDetailTitle;
    QLabel *m_clientNameLabel;
    QLabel *m_companyNameLabel;
    QLabel *m_emailLabel;
    QLabel *m_phoneLabel;
    QLabel *m_addressLabel;
    QTextEdit *m_notesDisplay;
    
    // Map tab
    QWidget *m_mapTab;
    QVBoxLayout *m_mapLayout;
    QWidget *m_mapContainer;
    QListWidget *m_mapClientList;
    
    // Action buttons
    QPushButton *m_addClientBtn;
    QPushButton *m_editClientBtn;
    QPushButton *m_deleteClientBtn;
    QPushButton *m_viewClientBtn;
    QPushButton *m_showOnMapBtn;
    QPushButton *m_aiInsightBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_importBtn;
    QPushButton *m_statisticsBtn;
      // Context menu
    QMenu *m_clientContextMenu;
    
    // Data and services
    ClientDatabaseManager *m_dbManager;
    MapboxHandler *m_mapHandler;
    GroqClient *m_groqClient;
    QTimer *m_searchTimer;
    bool m_isLoading;
    int m_currentTab;
    QList<ClientContact*> m_clients;
    QList<ClientContact*> m_filteredClients;
    
    // Search and filtering
    QString m_currentSearchTerm;
    QString m_currentCityFilter;
    QString m_currentCountryFilter;
    
    // Table columns
    enum ClientColumnIndex {
        NameColumn = 0,
        CompanyColumn,
        EmailColumn,
        PhoneColumn,
        CityColumn,
        CountryColumn,
        ClientColumnCount
    };
};

#endif // CLIENTWIDGET_H
