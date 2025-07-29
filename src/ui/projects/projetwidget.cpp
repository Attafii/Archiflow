#include "projetwidget.h"

#include <QApplication>
#include <QHeaderView>
#include <QSplitter>
#include <QGroupBox>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QDebug>

ProjetWidget::ProjetWidget(ProjetManager *projetManager, QWidget *parent)
    : QWidget(parent)
    , m_projetManager(projetManager)
    , m_mainLayout(nullptr)
    , m_mainSplitter(nullptr)
    , m_searchGroup(nullptr)
    , m_searchLayout(nullptr)
    , m_searchLineEdit(nullptr)
    , m_categoryFilter(nullptr)
    , m_statusFilter(nullptr)
    , m_dateFromFilter(nullptr)
    , m_dateToFilter(nullptr)
    , m_clearFiltersButton(nullptr)
    , m_searchButton(nullptr)
    , m_tableWidget(nullptr)
    , m_tableLayout(nullptr)
    , m_projectsTable(nullptr)
    , m_projectCountLabel(nullptr)
    , m_toolbarLayout(nullptr)
    , m_newProjectButton(nullptr)
    , m_editProjectButton(nullptr)
    , m_deleteProjectButton(nullptr)
    , m_viewProjectButton(nullptr)
    , m_refreshButton(nullptr)
    , m_exportButton(nullptr)
    , m_importButton(nullptr)
    , m_detailsGroup(nullptr)
    , m_detailsScrollArea(nullptr)
    , m_detailsWidget(nullptr)
    , m_detailsLayout(nullptr)
    , m_contextMenu(nullptr)
    , m_searchTimer(nullptr)
    , m_isUpdating(false)
{
    setupUI();
    setupTable();
    setupSearchAndFilters();
    setupActions();
    setupConnections();
    applyArchiFlowStyling();
    
    // Initialize search timer
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300); // 300ms delay for search
    connect(m_searchTimer, &QTimer::timeout, this, &ProjetWidget::performSearch);
    
    // Load initial data
    refreshProjects();
}

ProjetWidget::~ProjetWidget()
{
    // Qt handles cleanup automatically
}

void ProjetWidget::setupUI()
{
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Create main splitter (horizontal)
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_mainSplitter);
    
    // Left panel (table and search)
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(10);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    // Search and filters group
    m_searchGroup = new QGroupBox("Recherche et Filtres");
    leftLayout->addWidget(m_searchGroup);
    
    // Toolbar
    QWidget *toolbarWidget = new QWidget();
    m_toolbarLayout = new QHBoxLayout(toolbarWidget);
    m_toolbarLayout->setSpacing(10);
    m_toolbarLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(toolbarWidget);
    
    // Table container
    m_tableWidget = new QWidget();
    m_tableLayout = new QVBoxLayout(m_tableWidget);
    m_tableLayout->setSpacing(5);
    m_tableLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(m_tableWidget);
    
    // Right panel (project details)
    m_detailsGroup = new QGroupBox("Détails du Projet");
    m_detailsGroup->setMinimumWidth(350);
    m_detailsGroup->setMaximumWidth(400);
    
    // Set splitter proportions
    m_mainSplitter->addWidget(leftPanel);
    m_mainSplitter->addWidget(m_detailsGroup);
    m_mainSplitter->setStretchFactor(0, 2); // Table takes 2/3
    m_mainSplitter->setStretchFactor(1, 1); // Details takes 1/3
}

void ProjetWidget::setupTable()
{
    // Create table
    m_projectsTable = new QTableWidget();
    m_projectsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_projectsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_projectsTable->setAlternatingRowColors(true);
    m_projectsTable->setSortingEnabled(true);
    m_projectsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Configure columns
    configureTableColumns();
    
    // Project count label
    m_projectCountLabel = new QLabel("0 projet(s)");
    m_projectCountLabel->setAlignment(Qt::AlignRight);
    
    // Add to layout
    m_tableLayout->addWidget(m_projectsTable);
    m_tableLayout->addWidget(m_projectCountLabel);
}

void ProjetWidget::configureTableColumns()
{
    // Define columns
    QStringList headers = {
        "ID", "Nom", "Catégorie", "Statut", "Client", "Architecte", 
        "Budget", "Surface", "Progression", "Date Début", "Date Fin", "Création"
    };
    
    m_projectsTable->setColumnCount(headers.size());
    m_projectsTable->setHorizontalHeaderLabels(headers);
    
    // Configure column widths
    QHeaderView *header = m_projectsTable->horizontalHeader();
    header->setStretchLastSection(false);
    
    // Set specific widths
    m_projectsTable->setColumnWidth(0, 50);   // ID
    m_projectsTable->setColumnWidth(1, 150);  // Nom
    m_projectsTable->setColumnWidth(2, 100);  // Catégorie
    m_projectsTable->setColumnWidth(3, 100);  // Statut
    m_projectsTable->setColumnWidth(4, 120);  // Client
    m_projectsTable->setColumnWidth(5, 120);  // Architecte
    m_projectsTable->setColumnWidth(6, 100);  // Budget
    m_projectsTable->setColumnWidth(7, 80);   // Surface
    m_projectsTable->setColumnWidth(8, 80);   // Progression
    m_projectsTable->setColumnWidth(9, 100);  // Date Début
    m_projectsTable->setColumnWidth(10, 100); // Date Fin
    m_projectsTable->setColumnWidth(11, 100); // Création
    
    // Hide ID column by default
    m_projectsTable->setColumnHidden(0, true);
    
    // Set resize modes
    header->setSectionResizeMode(1, QHeaderView::Stretch); // Nom column stretches
}

void ProjetWidget::setupSearchAndFilters()
{
    // Search layout
    m_searchLayout = new QHBoxLayout(m_searchGroup);
    
    // Search text input
    QLabel *searchLabel = new QLabel("Recherche:");
    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("Nom du projet, description, client...");
    m_searchLineEdit->setClearButtonEnabled(true);
    
    // Category filter
    QLabel *categoryLabel = new QLabel("Catégorie:");
    m_categoryFilter = new QComboBox();
    m_categoryFilter->addItem("Toutes les catégories", "");
    m_categoryFilter->addItems({
        "Résidentiel", "Commercial", "Industriel", "Institutionnel",
        "Infrastructure", "Rénovation", "Aménagement", "Autre"
    });
    
    // Status filter
    QLabel *statusLabel = new QLabel("Statut:");
    m_statusFilter = new QComboBox();
    m_statusFilter->addItem("Tous les statuts", "");
    m_statusFilter->addItems({
        "En préparation", "En cours", "En pause", "En révision",
        "Terminé", "Annulé", "Archivé"
    });
    
    // Date filters
    QLabel *dateFromLabel = new QLabel("Du:");
    m_dateFromFilter = new QDateEdit();
    m_dateFromFilter->setDate(QDate::currentDate().addYears(-1));
    m_dateFromFilter->setCalendarPopup(true);
    m_dateFromFilter->setSpecialValueText("Non défini");
    
    QLabel *dateToLabel = new QLabel("Au:");
    m_dateToFilter = new QDateEdit();
    m_dateToFilter->setDate(QDate::currentDate().addYears(1));
    m_dateToFilter->setCalendarPopup(true);
    m_dateToFilter->setSpecialValueText("Non défini");
    
    // Buttons
    m_searchButton = new QPushButton("Rechercher");
    m_clearFiltersButton = new QPushButton("Effacer");
    
    // Add to layout
    m_searchLayout->addWidget(searchLabel);
    m_searchLayout->addWidget(m_searchLineEdit, 1);
    m_searchLayout->addWidget(categoryLabel);
    m_searchLayout->addWidget(m_categoryFilter);
    m_searchLayout->addWidget(statusLabel);
    m_searchLayout->addWidget(m_statusFilter);
    m_searchLayout->addWidget(dateFromLabel);
    m_searchLayout->addWidget(m_dateFromFilter);
    m_searchLayout->addWidget(dateToLabel);
    m_searchLayout->addWidget(m_dateToFilter);
    m_searchLayout->addWidget(m_searchButton);
    m_searchLayout->addWidget(m_clearFiltersButton);
    
    // Initialize date range
    m_currentDateFrom = m_dateFromFilter->date();
    m_currentDateTo = m_dateToFilter->date();
}

void ProjetWidget::setupActions()
{
    // Toolbar buttons
    m_newProjectButton = new QPushButton("Nouveau Projet");
    m_newProjectButton->setIcon(QIcon(":/icons/add.png"));
    
    m_editProjectButton = new QPushButton("Modifier");
    m_editProjectButton->setIcon(QIcon(":/icons/edit.png"));
    m_editProjectButton->setEnabled(false);
    
    m_deleteProjectButton = new QPushButton("Supprimer");
    m_deleteProjectButton->setIcon(QIcon(":/icons/delete.png"));
    m_deleteProjectButton->setEnabled(false);
    
    m_viewProjectButton = new QPushButton("Visualiser");
    m_viewProjectButton->setIcon(QIcon(":/icons/view.png"));
    m_viewProjectButton->setEnabled(false);
    
    m_refreshButton = new QPushButton("Actualiser");
    m_refreshButton->setIcon(QIcon(":/icons/refresh.png"));
    
    m_exportButton = new QPushButton("Exporter");
    m_exportButton->setIcon(QIcon(":/icons/export.png"));
    
    m_importButton = new QPushButton("Importer");
    m_importButton->setIcon(QIcon(":/icons/import.png"));
    
    // Add to toolbar
    m_toolbarLayout->addWidget(m_newProjectButton);
    m_toolbarLayout->addWidget(m_editProjectButton);
    m_toolbarLayout->addWidget(m_deleteProjectButton);
    m_toolbarLayout->addWidget(m_viewProjectButton);
    m_toolbarLayout->addWidget(m_refreshButton);
    m_toolbarLayout->addStretch();
    m_toolbarLayout->addWidget(m_exportButton);
    m_toolbarLayout->addWidget(m_importButton);
    
    // Context menu
    m_contextMenu = new QMenu(this);
    
    m_actionNew = m_contextMenu->addAction("Nouveau Projet");
    m_actionNew->setIcon(QIcon(":/icons/add.png"));
    
    m_contextMenu->addSeparator();
    
    m_actionView = m_contextMenu->addAction("Visualiser");
    m_actionView->setIcon(QIcon(":/icons/view.png"));
    
    m_actionEdit = m_contextMenu->addAction("Modifier");
    m_actionEdit->setIcon(QIcon(":/icons/edit.png"));
    
    m_actionDuplicate = m_contextMenu->addAction("Dupliquer");
    m_actionDuplicate->setIcon(QIcon(":/icons/copy.png"));
    
    m_contextMenu->addSeparator();
    
    m_actionDelete = m_contextMenu->addAction("Supprimer");
    m_actionDelete->setIcon(QIcon(":/icons/delete.png"));
    
    m_contextMenu->addSeparator();
    
    m_actionExport = m_contextMenu->addAction("Exporter la sélection");
    m_actionExport->setIcon(QIcon(":/icons/export.png"));
}

void ProjetWidget::setupConnections()
{
    // Table connections
    connect(m_projectsTable, &QTableWidget::itemSelectionChanged,
            this, [this]() {
                QItemSelection selected, deselected;
                onProjectSelectionChanged(selected, deselected);
            });
    connect(m_projectsTable, &QTableWidget::itemDoubleClicked,
            this, [this](QTableWidgetItem *item) {
                onProjectDoubleClicked(m_projectsTable->model()->index(item->row(), item->column()));
            });
    connect(m_projectsTable, &QTableWidget::customContextMenuRequested,
            this, &ProjetWidget::onTableContextMenuRequested);
    
    // Search and filter connections
    connect(m_searchLineEdit, &QLineEdit::textChanged,
            this, &ProjetWidget::onSearchTextChanged);
    connect(m_categoryFilter, &QComboBox::currentTextChanged,
            this, &ProjetWidget::onCategoryFilterChanged);
    connect(m_statusFilter, &QComboBox::currentTextChanged,
            this, &ProjetWidget::onStatusFilterChanged);
    connect(m_dateFromFilter, &QDateEdit::dateChanged,
            this, &ProjetWidget::onDateRangeChanged);
    connect(m_dateToFilter, &QDateEdit::dateChanged,
            this, &ProjetWidget::onDateRangeChanged);
    connect(m_searchButton, &QPushButton::clicked,
            this, &ProjetWidget::performSearch);
    connect(m_clearFiltersButton, &QPushButton::clicked,
            this, &ProjetWidget::onClearFilters);
    
    // Toolbar connections
    connect(m_newProjectButton, &QPushButton::clicked,
            this, &ProjetWidget::onNewProjectAction);
    connect(m_editProjectButton, &QPushButton::clicked,
            this, &ProjetWidget::onEditProjectAction);
    connect(m_deleteProjectButton, &QPushButton::clicked,
            this, &ProjetWidget::onDeleteProjectAction);
    connect(m_viewProjectButton, &QPushButton::clicked,
            this, &ProjetWidget::onViewProjectAction);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &ProjetWidget::onRefreshRequested);
    connect(m_exportButton, &QPushButton::clicked,
            this, &ProjetWidget::onExportProjectsAction);
    connect(m_importButton, &QPushButton::clicked,
            this, &ProjetWidget::onImportProjectsAction);
    
    // Context menu connections
    connect(m_actionNew, &QAction::triggered,
            this, &ProjetWidget::onNewProjectAction);
    connect(m_actionView, &QAction::triggered,
            this, &ProjetWidget::onViewProjectAction);
    connect(m_actionEdit, &QAction::triggered,
            this, &ProjetWidget::onEditProjectAction);
    connect(m_actionDuplicate, &QAction::triggered,
            this, &ProjetWidget::onDuplicateProjectAction);
    connect(m_actionDelete, &QAction::triggered,
            this, &ProjetWidget::onDeleteProjectAction);
    connect(m_actionExport, &QAction::triggered,
            this, &ProjetWidget::onExportProjectsAction);
}

void ProjetWidget::applyArchiFlowStyling()
{
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #D4B7A1;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 5px;
            background-color: #3D485A;
            color: #E3C6B0;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
            color: #E3C6B0;
        }
        
        QTableWidget {
            background-color: #2A3340;
            alternate-background-color: #3D485A;
            color: #E3C6B0;
            border: 1px solid #D4B7A1;
            border-radius: 4px;
            gridline-color: #555;
        }
        
        QTableWidget::item {
            padding: 5px;
            border-bottom: 1px solid #555;
        }
        
        QTableWidget::item:selected {
            background-color: #E3C6B0;
            color: #3D485A;
        }
        
        QHeaderView::section {
            background-color: #2A3340;
            color: #E3C6B0;
            border: 1px solid #555;
            padding: 8px;
            font-weight: bold;
        }
        
        QPushButton {
            background-color: #2A3340;
            color: #E3C6B0;
            border: 1px solid #D4B7A1;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: 500;
            min-width: 80px;
        }
        
        QPushButton:hover {
            background-color: #E3C6B0;
            color: #3D485A;
        }
        
        QPushButton:pressed {
            background-color: #D4B7A1;
        }
        
        QPushButton:disabled {
            background-color: #555;
            color: #888;
            border-color: #888;
        }
        
        QLineEdit, QComboBox, QDateEdit {
            background-color: #2A3340;
            color: #E3C6B0;
            border: 1px solid #D4B7A1;
            padding: 6px;
            border-radius: 4px;
        }
        
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border: 2px solid #E3C6B0;
        }
        
        QComboBox::drop-down {
            border: none;
            background-color: #2A3340;
        }
        
        QComboBox::down-arrow {
            image: none;
            border: none;
        }
        
        QProgressBar {
            border: 1px solid #D4B7A1;
            border-radius: 4px;
            background-color: #2A3340;
        }
        
        QProgressBar::chunk {
            background-color: #4CAF50;
            border-radius: 3px;
        }
    )");
}

void ProjetWidget::refreshProjects()
{
    if (!m_projetManager) {
        qWarning() << "ProjetManager is null, cannot refresh projects";
        return;
    }
    
    // Show loading state
    m_projectsTable->setEnabled(false);
    emit statusMessage("Chargement des projets...");
    
    // Load projects from database
    m_allProjects = m_projetManager->getAllProjets();
    
    // Apply current filters
    applyFilters();
    
    // Update table
    populateTable();
    
    // Update status
    updateStatusBar();
    
    // Re-enable table
    m_projectsTable->setEnabled(true);
    
    emit statusMessage("Projets chargés avec succès");
    emit projectCountChanged(m_filteredProjects.size());
}

void ProjetWidget::populateTable()
{
    if (m_isUpdating) return;
    
    m_isUpdating = true;
    
    // Clear table
    m_projectsTable->setRowCount(0);
    
    // Add projects to table
    for (int i = 0; i < m_filteredProjects.size(); ++i) {
        const Projet &projet = m_filteredProjects[i];
        m_projectsTable->insertRow(i);
        addProjectToTable(projet, i);
    }
    
    m_isUpdating = false;
    
    // Update count
    updateStatusBar();
}

void ProjetWidget::addProjectToTable(const Projet &projet, int row)
{
    // ID (hidden)
    QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(projet.getId()));
    idItem->setData(Qt::UserRole, projet.getId());
    m_projectsTable->setItem(row, 0, idItem);
    
    // Nom
    QTableWidgetItem *nomItem = new QTableWidgetItem(projet.getNom());
    nomItem->setFlags(nomItem->flags() & ~Qt::ItemIsEditable);
    m_projectsTable->setItem(row, 1, nomItem);
    
    // Catégorie
    QTableWidgetItem *categorieItem = new QTableWidgetItem(projet.getCategorie());
    categorieItem->setFlags(categorieItem->flags() & ~Qt::ItemIsEditable);
    m_projectsTable->setItem(row, 2, categorieItem);
    
    // Statut avec couleur
    QTableWidgetItem *statutItem = new QTableWidgetItem(projet.getStatut());
    statutItem->setFlags(statutItem->flags() & ~Qt::ItemIsEditable);
    statutItem->setBackground(QBrush(getStatusColor(projet.getStatut())));
    m_projectsTable->setItem(row, 3, statutItem);
    
    // Client
    QTableWidgetItem *clientItem = new QTableWidgetItem(projet.getClient());
    clientItem->setFlags(clientItem->flags() & ~Qt::ItemIsEditable);
    m_projectsTable->setItem(row, 4, clientItem);
    
    // Architecte
    QTableWidgetItem *architecteItem = new QTableWidgetItem(projet.getArchitecte());
    architecteItem->setFlags(architecteItem->flags() & ~Qt::ItemIsEditable);
    m_projectsTable->setItem(row, 5, architecteItem);
    
    // Budget
    QTableWidgetItem *budgetItem = new QTableWidgetItem(formatBudget(projet.getBudget()));
    budgetItem->setFlags(budgetItem->flags() & ~Qt::ItemIsEditable);
    budgetItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_projectsTable->setItem(row, 6, budgetItem);
    
    // Surface
    QTableWidgetItem *surfaceItem = new QTableWidgetItem(formatSurface(projet.getSurface()));
    surfaceItem->setFlags(surfaceItem->flags() & ~Qt::ItemIsEditable);
    surfaceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_projectsTable->setItem(row, 7, surfaceItem);
    
    // Progression
    QTableWidgetItem *progressionItem = new QTableWidgetItem(QString("%1%").arg(projet.getProgression()));
    progressionItem->setFlags(progressionItem->flags() & ~Qt::ItemIsEditable);
    progressionItem->setTextAlignment(Qt::AlignCenter);
    m_projectsTable->setItem(row, 8, progressionItem);
    
    // Date Début
    QString dateDebut = projet.getDateDebut().isValid() ? 
        projet.getDateDebut().toString("dd/MM/yyyy") : "-";
    QTableWidgetItem *dateDebutItem = new QTableWidgetItem(dateDebut);
    dateDebutItem->setFlags(dateDebutItem->flags() & ~Qt::ItemIsEditable);
    m_projectsTable->setItem(row, 9, dateDebutItem);
    
    // Date Fin
    QString dateFin = projet.getDateFinEstimee().isValid() ? 
        projet.getDateFinEstimee().toString("dd/MM/yyyy") : "-";
    QTableWidgetItem *dateFinItem = new QTableWidgetItem(dateFin);
    dateFinItem->setFlags(dateFinItem->flags() & ~Qt::ItemIsEditable);
    m_projectsTable->setItem(row, 10, dateFinItem);
    
    // Date Création
    QString dateCreation = projet.getDateCreation().isValid() ? 
        projet.getDateCreation().toString("dd/MM/yyyy hh:mm") : "-";
    QTableWidgetItem *dateCreationItem = new QTableWidgetItem(dateCreation);
    dateCreationItem->setFlags(dateCreationItem->flags() & ~Qt::ItemIsEditable);
    m_projectsTable->setItem(row, 11, dateCreationItem);
}

void ProjetWidget::applyFilters()
{
    m_filteredProjects.clear();
    
    for (const Projet &projet : m_allProjects) {
        if (matchesSearchCriteria(projet)) {
            m_filteredProjects.append(projet);
        }
    }
}

bool ProjetWidget::matchesSearchCriteria(const Projet &projet) const
{
    // Text search
    if (!m_currentSearchText.isEmpty()) {
        QString searchLower = m_currentSearchText.toLower();
        if (!projet.getNom().toLower().contains(searchLower) &&
            !projet.getDescription().toLower().contains(searchLower) &&
            !projet.getClient().toLower().contains(searchLower) &&
            !projet.getArchitecte().toLower().contains(searchLower)) {
            return false;
        }
    }
    
    // Category filter
    if (!m_currentCategoryFilter.isEmpty()) {
        if (projet.getCategorie() != m_currentCategoryFilter) {
            return false;
        }
    }
    
    // Status filter
    if (!m_currentStatusFilter.isEmpty()) {
        if (projet.getStatut() != m_currentStatusFilter) {
            return false;
        }
    }
    
    // Date range filter
    if (m_currentDateFrom.isValid() && projet.getDateCreation().date() < m_currentDateFrom) {
        return false;
    }
    
    if (m_currentDateTo.isValid() && projet.getDateCreation().date() > m_currentDateTo) {
        return false;
    }
    
    return true;
}

// Utility methods
QColor ProjetWidget::getStatusColor(const QString &status) const
{
    if (status == "Terminé") return QColor("#4CAF50");
    if (status == "En cours") return QColor("#42A5F5");
    if (status == "En pause") return QColor("#FFA726");
    if (status == "En révision") return QColor("#AB47BC");
    if (status == "Annulé") return QColor("#FF6B6B");
    if (status == "Archivé") return QColor("#777");
    return QColor("#D4B7A1"); // Default
}

QString ProjetWidget::formatBudget(double budget) const
{
    if (budget <= 0) return "-";
    
    if (budget >= 1000000) {
        return QString("%1 M€").arg(budget / 1000000.0, 0, 'f', 1);
    } else if (budget >= 1000) {
        return QString("%1 k€").arg(budget / 1000.0, 0, 'f', 0);
    } else {
        return QString("%1 €").arg(budget, 0, 'f', 0);
    }
}

QString ProjetWidget::formatSurface(double surface) const
{
    if (surface <= 0) return "-";
    return QString("%1 m²").arg(surface, 0, 'f', 0);
}

void ProjetWidget::updateStatusBar()
{
    QString text = QString("%1 projet(s)").arg(m_filteredProjects.size());
    if (m_filteredProjects.size() != m_allProjects.size()) {
        text += QString(" (sur %1 total)").arg(m_allProjects.size());
    }
    m_projectCountLabel->setText(text);
}

// Event handlers
void ProjetWidget::onProjectSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    
    bool hasSelection = !m_projectsTable->selectedItems().isEmpty();
    
    // Update button states
    m_editProjectButton->setEnabled(hasSelection);
    m_deleteProjectButton->setEnabled(hasSelection);
    m_viewProjectButton->setEnabled(hasSelection);
    
    // Update context menu actions
    m_actionEdit->setEnabled(hasSelection);
    m_actionDelete->setEnabled(hasSelection);
    m_actionView->setEnabled(hasSelection);
    m_actionDuplicate->setEnabled(hasSelection);
    m_actionExport->setEnabled(hasSelection);
    
    if (hasSelection) {
        m_selectedProject = getSelectedProject();
        showProjectDetails(m_selectedProject);
        emit projectSelected(m_selectedProject);
    } else {
        m_selectedProject = Projet();
        clearProjectDetails();
    }
}

void ProjetWidget::onProjectDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    onViewProjectAction();
}

void ProjetWidget::onTableContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *item = m_projectsTable->itemAt(pos);
    bool hasSelection = (item != nullptr);
    
    // Update context menu state
    m_actionEdit->setEnabled(hasSelection);
    m_actionDelete->setEnabled(hasSelection);
    m_actionView->setEnabled(hasSelection);
    m_actionDuplicate->setEnabled(hasSelection);
    m_actionExport->setEnabled(hasSelection);
    
    // Show context menu
    m_contextMenu->exec(m_projectsTable->mapToGlobal(pos));
}

// Search and filter event handlers
void ProjetWidget::onSearchTextChanged(const QString &text)
{
    m_currentSearchText = text;
    m_searchTimer->start(); // Delay search for performance
}

void ProjetWidget::onCategoryFilterChanged(const QString &category)
{
    m_currentCategoryFilter = (category == "Toutes les catégories") ? "" : category;
    performSearch();
}

void ProjetWidget::onStatusFilterChanged(const QString &status)
{
    m_currentStatusFilter = (status == "Tous les statuts") ? "" : status;
    performSearch();
}

void ProjetWidget::onDateRangeChanged()
{
    m_currentDateFrom = m_dateFromFilter->date();
    m_currentDateTo = m_dateToFilter->date();
    performSearch();
}

void ProjetWidget::onClearFilters()
{
    // Block signals temporarily
    m_searchLineEdit->blockSignals(true);
    m_categoryFilter->blockSignals(true);
    m_statusFilter->blockSignals(true);
    m_dateFromFilter->blockSignals(true);
    m_dateToFilter->blockSignals(true);
    
    // Clear filters
    m_searchLineEdit->clear();
    m_categoryFilter->setCurrentIndex(0);
    m_statusFilter->setCurrentIndex(0);
    m_dateFromFilter->setDate(QDate::currentDate().addYears(-1));
    m_dateToFilter->setDate(QDate::currentDate().addYears(1));
    
    // Restore signals
    m_searchLineEdit->blockSignals(false);
    m_categoryFilter->blockSignals(false);
    m_statusFilter->blockSignals(false);
    m_dateFromFilter->blockSignals(false);
    m_dateToFilter->blockSignals(false);
    
    // Update filter values
    m_currentSearchText.clear();
    m_currentCategoryFilter.clear();
    m_currentStatusFilter.clear();
    m_currentDateFrom = m_dateFromFilter->date();
    m_currentDateTo = m_dateToFilter->date();
    
    // Perform search
    performSearch();
}

void ProjetWidget::performSearch()
{
    applyFilters();
    populateTable();
    
    emit statusMessage(QString("Recherche terminée: %1 résultat(s)")
                      .arg(m_filteredProjects.size()));
}

// Action handlers
void ProjetWidget::onNewProjectAction()
{
    if (createNewProject()) {
        refreshProjects();
        emit statusMessage("Nouveau projet créé avec succès");
    }
}

void ProjetWidget::onEditProjectAction()
{
    if (editSelectedProject()) {
        refreshProjects();
        emit statusMessage("Projet modifié avec succès");
    }
}

void ProjetWidget::onDeleteProjectAction()
{
    if (deleteSelectedProject()) {
        refreshProjects();
        emit statusMessage("Projet supprimé avec succès");
    }
}

void ProjetWidget::onViewProjectAction()
{
    viewSelectedProject();
}

void ProjetWidget::onRefreshRequested()
{
    refreshProjects();
}

void ProjetWidget::onExportProjectsAction()
{
    exportProjectsToCSV();
}

void ProjetWidget::onImportProjectsAction()
{
    importProjectsFromCSV();
}

void ProjetWidget::onDuplicateProjectAction()
{
    if (duplicateSelectedProject()) {
        refreshProjects();
        emit statusMessage("Projet dupliqué avec succès");
    }
}

// Project operations
bool ProjetWidget::createNewProject()
{
    return ProjetDialog::createNewProject(m_projetManager, this);
}

bool ProjetWidget::editSelectedProject()
{
    if (m_selectedProject.getId() <= 0) {
        QMessageBox::warning(this, "Aucune sélection", 
            "Veuillez sélectionner un projet à modifier.");
        return false;
    }
    
    return ProjetDialog::editProject(m_projetManager, m_selectedProject, this);
}

bool ProjetWidget::deleteSelectedProject()
{
    if (m_selectedProject.getId() <= 0) {
        QMessageBox::warning(this, "Aucune sélection", 
            "Veuillez sélectionner un projet à supprimer.");
        return false;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Confirmer la suppression",
        QString("Êtes-vous sûr de vouloir supprimer le projet '%1' ?\n\nCette action est irréversible.")
            .arg(m_selectedProject.getNom()),
        QMessageBox::Yes | QMessageBox::No);
        
    if (reply == QMessageBox::Yes) {
        if (m_projetManager && m_projetManager->supprimerProjet(m_selectedProject.getId())) {
            return true;
        } else {
            QMessageBox::critical(this, "Erreur", 
                "Impossible de supprimer le projet. Veuillez réessayer.");
        }
    }
    
    return false;
}

bool ProjetWidget::viewSelectedProject()
{
    if (m_selectedProject.getId() <= 0) {
        QMessageBox::warning(this, "Aucune sélection", 
            "Veuillez sélectionner un projet à visualiser.");
        return false;
    }
    
    return ProjetDialog::viewProject(m_projetManager, m_selectedProject, this);
}

bool ProjetWidget::duplicateSelectedProject()
{
    if (m_selectedProject.getId() <= 0) {
        QMessageBox::warning(this, "Aucune sélection", 
            "Veuillez sélectionner un projet à dupliquer.");
        return false;
    }
    
    // Create a copy with reset ID and updated name
    Projet copiedProject = m_selectedProject;
    copiedProject.setId(0); // Reset ID for new project
    copiedProject.setNom(copiedProject.getNom() + " (Copie)");
    copiedProject.setDateCreation(QDateTime::currentDateTime());
    copiedProject.setDateModification(QDateTime::currentDateTime());
    
    return ProjetDialog::editProject(m_projetManager, copiedProject, this);
}

Projet ProjetWidget::getSelectedProject() const
{
    QList<QTableWidgetItem*> selectedItems = m_projectsTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return Projet();
    }
    
    int row = selectedItems.first()->row();
    QTableWidgetItem *idItem = m_projectsTable->item(row, 0);
    if (!idItem) {
        return Projet();
    }
    
    int projectId = idItem->data(Qt::UserRole).toInt();
    
    // Find project in filtered list
    for (const Projet &projet : m_filteredProjects) {
        if (projet.getId() == projectId) {
            return projet;
        }
    }
    
    return Projet();
}

void ProjetWidget::showProjectDetails(const Projet &projet)
{
    // Initialize details section if not already done
    if (!m_detailsScrollArea) {
        m_detailsScrollArea = new QScrollArea(m_detailsGroup);
        m_detailsWidget = new QWidget();
        m_detailsLayout = new QFormLayout(m_detailsWidget);
        m_detailsLayout->setSpacing(10);
        m_detailsLayout->setContentsMargins(15, 15, 15, 15);
        
        // Create detail labels
        m_detailNom = new QLabel();
        m_detailDescription = new QLabel();
        m_detailCategorie = new QLabel();
        m_detailStatut = new QLabel();
        m_detailClient = new QLabel();
        m_detailArchitecte = new QLabel();
        m_detailBudget = new QLabel();
        m_detailSurface = new QLabel();
        m_detailEtage = new QLabel();
        m_detailMateriau = new QLabel();
        m_detailDateDebut = new QLabel();
        m_detailDateFin = new QLabel();
        m_detailProgression = new QLabel();
        m_detailProgressBar = new QProgressBar();
        m_detailLocation = new QLabel();
        m_detailDateCreation = new QLabel();
        m_detailDateModification = new QLabel();
        
        // Set word wrap for description
        m_detailDescription->setWordWrap(true);
        m_detailDescription->setMaximumHeight(100);
        
        // Configure progress bar
        m_detailProgressBar->setMaximum(100);
        m_detailProgressBar->setTextVisible(true);
        
        // Add to layout
        m_detailsLayout->addRow("Nom:", m_detailNom);
        m_detailsLayout->addRow("Description:", m_detailDescription);
        m_detailsLayout->addRow("Catégorie:", m_detailCategorie);
        m_detailsLayout->addRow("Statut:", m_detailStatut);
        m_detailsLayout->addRow("Client:", m_detailClient);
        m_detailsLayout->addRow("Architecte:", m_detailArchitecte);
        m_detailsLayout->addRow("Budget:", m_detailBudget);
        m_detailsLayout->addRow("Surface:", m_detailSurface);
        m_detailsLayout->addRow("Étages:", m_detailEtage);
        m_detailsLayout->addRow("Matériau:", m_detailMateriau);
        m_detailsLayout->addRow("Date début:", m_detailDateDebut);
        m_detailsLayout->addRow("Date fin:", m_detailDateFin);
        m_detailsLayout->addRow("Progression:", m_detailProgression);
        m_detailsLayout->addRow("", m_detailProgressBar);
        m_detailsLayout->addRow("Localisation:", m_detailLocation);
        m_detailsLayout->addRow("Créé le:", m_detailDateCreation);
        m_detailsLayout->addRow("Modifié le:", m_detailDateModification);
        
        m_detailsScrollArea->setWidget(m_detailsWidget);
        m_detailsScrollArea->setWidgetResizable(true);
        
        QVBoxLayout *detailsGroupLayout = new QVBoxLayout(m_detailsGroup);
        detailsGroupLayout->addWidget(m_detailsScrollArea);
    }
    
    // Update details with project data
    m_detailNom->setText(projet.getNom());
    m_detailDescription->setText(projet.getDescription().isEmpty() ? "-" : projet.getDescription());
    m_detailCategorie->setText(projet.getCategorie());
    
    // Set status with color
    m_detailStatut->setText(projet.getStatut());
    m_detailStatut->setStyleSheet(QString("color: %1; font-weight: bold;")
                                  .arg(getStatusColor(projet.getStatut()).name()));
    
    m_detailClient->setText(projet.getClient().isEmpty() ? "-" : projet.getClient());
    m_detailArchitecte->setText(projet.getArchitecte().isEmpty() ? "-" : projet.getArchitecte());
    m_detailBudget->setText(formatBudget(projet.getBudget()));
    m_detailSurface->setText(formatSurface(projet.getSurface()));
    m_detailEtage->setText(projet.getEtage() == 0 ? "-" : QString::number(projet.getEtage()));
    m_detailMateriau->setText(projet.getMateriauPrincipal().isEmpty() ? "-" : projet.getMateriauPrincipal());
    
    m_detailDateDebut->setText(projet.getDateDebut().isValid() ? 
        projet.getDateDebut().toString("dd/MM/yyyy") : "-");
    m_detailDateFin->setText(projet.getDateFinEstimee().isValid() ? 
        projet.getDateFinEstimee().toString("dd/MM/yyyy") : "-");
        
    m_detailProgression->setText(QString("%1%").arg(projet.getProgression()));
    m_detailProgressBar->setValue(projet.getProgression());
    
    Coordinate location = projet.getLocation();
    if (location.isValid()) {
        QString locationText = location.toString();
        if (!location.address.isEmpty()) {
            locationText += "\n" + location.address;
        }
        m_detailLocation->setText(locationText);
    } else {
        m_detailLocation->setText("-");
    }
    
    m_detailDateCreation->setText(projet.getDateCreation().isValid() ? 
        projet.getDateCreation().toString("dd/MM/yyyy hh:mm") : "-");
    m_detailDateModification->setText(projet.getDateModification().isValid() ? 
        projet.getDateModification().toString("dd/MM/yyyy hh:mm") : "-");
}

void ProjetWidget::clearProjectDetails()
{
    if (!m_detailsScrollArea) return;
    
    // Clear all detail labels
    m_detailNom->clear();
    m_detailDescription->clear();
    m_detailCategorie->clear();
    m_detailStatut->clear();
    m_detailClient->clear();
    m_detailArchitecte->clear();
    m_detailBudget->clear();
    m_detailSurface->clear();
    m_detailEtage->clear();
    m_detailMateriau->clear();
    m_detailDateDebut->clear();
    m_detailDateFin->clear();
    m_detailProgression->clear();
    m_detailProgressBar->setValue(0);
    m_detailLocation->clear();
    m_detailDateCreation->clear();
    m_detailDateModification->clear();
}

// Data export/import methods
void ProjetWidget::exportProjectsToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Exporter les projets", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/projets_archiflow.csv",
        "CSV Files (*.csv)");
        
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", 
            "Impossible d'ouvrir le fichier pour écriture.");
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Write header
    out << "ID,Nom,Description,Catégorie,Statut,Client,Architecte,Budget,Surface,Étages,"
        << "Matériau,Date Début,Date Fin,Progression,Latitude,Longitude,Adresse,"
        << "Date Création,Date Modification\n";
    
    // Write data
    for (const Projet &projet : m_filteredProjects) {
        Coordinate location = projet.getLocation();
        out << projet.getId() << ","
            << "\"" << projet.getNom() << "\","
            << "\"" << projet.getDescription() << "\","
            << "\"" << projet.getCategorie() << "\","
            << "\"" << projet.getStatut() << "\","
            << "\"" << projet.getClient() << "\","
            << "\"" << projet.getArchitecte() << "\","
            << projet.getBudget() << ","
            << projet.getSurface() << ","
            << projet.getEtage() << ","
            << "\"" << projet.getMateriauPrincipal() << "\","
            << "\"" << (projet.getDateDebut().isValid() ? projet.getDateDebut().toString("yyyy-MM-dd") : "") << "\","
            << "\"" << (projet.getDateFinEstimee().isValid() ? projet.getDateFinEstimee().toString("yyyy-MM-dd") : "") << "\","
            << projet.getProgression() << ","
            << location.latitude << ","
            << location.longitude << ","
            << "\"" << location.address << "\","
            << "\"" << (projet.getDateCreation().isValid() ? projet.getDateCreation().toString("yyyy-MM-dd hh:mm:ss") : "") << "\","
            << "\"" << (projet.getDateModification().isValid() ? projet.getDateModification().toString("yyyy-MM-dd hh:mm:ss") : "") << "\"\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Export terminé",
        QString("Les projets ont été exportés vers:\n%1").arg(fileName));
    
    emit statusMessage(QString("Export terminé: %1 projets exportés").arg(m_filteredProjects.size()));
}

void ProjetWidget::importProjectsFromCSV()
{
    // TODO: Implement CSV import functionality
    QMessageBox::information(this, "Import de projets",
        "La fonctionnalité d'import sera disponible dans une prochaine version.");
}

// Public interface methods
void ProjetWidget::selectProject(int projectId)
{
    for (int row = 0; row < m_projectsTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = m_projectsTable->item(row, 0);
        if (idItem && idItem->data(Qt::UserRole).toInt() == projectId) {
            m_projectsTable->selectRow(row);
            m_projectsTable->scrollToItem(idItem);
            break;
        }
    }
}

void ProjetWidget::clearSelection()
{
    m_projectsTable->clearSelection();
}

// Slot implementations for external signals
void ProjetWidget::onProjectCreated()
{
    refreshProjects();
}

void ProjetWidget::onProjectEdited()
{
    refreshProjects();
}

void ProjetWidget::onProjectDeleted()
{
    refreshProjects();
}

void ProjetWidget::onProjectViewed()
{
    // No action needed for view
}

void ProjetWidget::onSearchRequested()
{
    performSearch();
}

void ProjetWidget::onFilterChanged()
{
    performSearch();
}
