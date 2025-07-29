#ifndef PROJETWIDGET_H
#define PROJETWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QProgressBar>
#include <QFrame>
#include <QScrollArea>
#include <QGridLayout>
#include <QFormLayout>
#include <QDateEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>

#include "../../features/projects/projet.h"
#include "../../features/projects/projetmanager.h"
#include "projetdialog.h"

/**
 * @brief The ProjetWidget class - Main project management interface
 * 
 * This widget provides the main interface for viewing, searching, and managing
 * architectural projects. It includes a project list, search filters, and
 * detailed project information display.
 */
class ProjetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjetWidget(ProjetManager *projetManager, QWidget *parent = nullptr);
    ~ProjetWidget();

    // Public interface
    void refreshProjects();
    void selectProject(int projectId);
    void clearSelection();

public slots:
    void onProjectCreated();
    void onProjectEdited();
    void onProjectDeleted();
    void onProjectViewed();
    void onSearchRequested();
    void onFilterChanged();
    void onRefreshRequested();

signals:
    void projectSelected(const Projet &projet);
    void projectCountChanged(int count);
    void statusMessage(const QString &message);

private slots:
    // Table interactions
    void onProjectSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onProjectDoubleClicked(const QModelIndex &index);
    void onTableContextMenuRequested(const QPoint &pos);
    
    // Search and filter
    void onSearchTextChanged(const QString &text);
    void onCategoryFilterChanged(const QString &category);
    void onStatusFilterChanged(const QString &status);
    void onDateRangeChanged();
    void onClearFilters();
    
    // Action handlers
    void onNewProjectAction();
    void onEditProjectAction();
    void onDeleteProjectAction();
    void onViewProjectAction();
    void onDuplicateProjectAction();
    void onExportProjectsAction();
    void onImportProjectsAction();
    
    // Timer-based search (for performance)
    void performSearch();

private:
    // UI setup
    void setupUI();
    void setupTable();
    void setupSearchAndFilters();
    void setupActions();
    void setupConnections();
    void applyArchiFlowStyling();
    
    // Table management
    void populateTable();
    void updateTableData();
    void configureTableColumns();
    void addProjectToTable(const Projet &projet, int row);
    QColor getStatusColor(const QString &status) const;
    QString formatBudget(double budget) const;
    QString formatSurface(double surface) const;
    
    // Project operations
    bool createNewProject();
    bool editSelectedProject();
    bool deleteSelectedProject();
    bool viewSelectedProject();
    bool duplicateSelectedProject();
    
    // Search and filtering
    void applyFilters();
    QList<Projet> getFilteredProjects();
    bool matchesSearchCriteria(const Projet &projet) const;
    
    // Data export/import
    void exportProjectsToCSV();
    void importProjectsFromCSV();
    
    // Utility methods
    Projet getSelectedProject() const;
    QList<Projet> getSelectedProjects() const;
    void showProjectDetails(const Projet &projet);
    void updateStatusBar();
    void clearProjectDetails();
    
private:
    ProjetManager *m_projetManager;
    QList<Projet> m_allProjects;
    QList<Projet> m_filteredProjects;
    Projet m_selectedProject;
    
    // Main layout
    QVBoxLayout *m_mainLayout;
    QSplitter *m_mainSplitter;
    
    // Search and filter section
    QGroupBox *m_searchGroup;
    QHBoxLayout *m_searchLayout;
    QLineEdit *m_searchLineEdit;
    QComboBox *m_categoryFilter;
    QComboBox *m_statusFilter;
    QDateEdit *m_dateFromFilter;
    QDateEdit *m_dateToFilter;
    QPushButton *m_clearFiltersButton;
    QPushButton *m_searchButton;
    
    // Table section
    QWidget *m_tableWidget;
    QVBoxLayout *m_tableLayout;
    QTableWidget *m_projectsTable;
    QLabel *m_projectCountLabel;
    
    // Toolbar
    QHBoxLayout *m_toolbarLayout;
    QPushButton *m_newProjectButton;
    QPushButton *m_editProjectButton;
    QPushButton *m_deleteProjectButton;
    QPushButton *m_viewProjectButton;
    QPushButton *m_refreshButton;
    QPushButton *m_exportButton;
    QPushButton *m_importButton;
    
    // Project details section
    QGroupBox *m_detailsGroup;
    QScrollArea *m_detailsScrollArea;
    QWidget *m_detailsWidget;
    QFormLayout *m_detailsLayout;
    
    // Detail labels
    QLabel *m_detailNom;
    QLabel *m_detailDescription;
    QLabel *m_detailCategorie;
    QLabel *m_detailStatut;
    QLabel *m_detailClient;
    QLabel *m_detailArchitecte;
    QLabel *m_detailBudget;
    QLabel *m_detailSurface;
    QLabel *m_detailEtage;
    QLabel *m_detailMateriau;
    QLabel *m_detailDateDebut;
    QLabel *m_detailDateFin;
    QLabel *m_detailProgression;
    QProgressBar *m_detailProgressBar;
    QLabel *m_detailLocation;
    QLabel *m_detailDateCreation;
    QLabel *m_detailDateModification;
    
    // Context menu
    QMenu *m_contextMenu;
    QAction *m_actionNew;
    QAction *m_actionEdit;
    QAction *m_actionDelete;
    QAction *m_actionView;
    QAction *m_actionDuplicate;
    QAction *m_actionExport;
    
    // Search timer for performance
    QTimer *m_searchTimer;
    
    // State
    bool m_isUpdating;
    QString m_currentSearchText;
    QString m_currentCategoryFilter;
    QString m_currentStatusFilter;
    QDate m_currentDateFrom;
    QDate m_currentDateTo;
};

#endif // PROJETWIDGET_H
