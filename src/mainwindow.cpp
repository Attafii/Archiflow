#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "projetdialog.h"
#include "projetmanager.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDate>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    setupTheme();
    setupTableView();
    setupConnections();
    
    // Charger les projets
    loadProjets();
    
    // Mettre à jour la barre de statut
    updateStatusBar();
    
    // Définir le titre de la fenêtre
    setWindowTitle(tr("Gestion des Projets - Architecture"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNouveau_triggered()
{
    ProjetDialog dialog(this);
    dialog.setEditMode(false);
    
    if (dialog.exec() == QDialog::Accepted) {
        Projet projet = dialog.getProjet();
        ProjetManager::getInstance().ajouterProjet(projet);
        updateProjectList();
        updateStatusBar();
    }
}

void MainWindow::on_actionModifier_triggered()
{
    int projetId = getSelectedProjetId();
    if (projetId <= 0) {
        QMessageBox::warning(this, tr("Aucune sélection"), 
                            tr("Veuillez sélectionner un projet à modifier."));
        return;
    }
    
    Projet projet = ProjetManager::getInstance().getProjetById(projetId);
    if (projet.getId() <= 0) {
        QMessageBox::warning(this, tr("Projet introuvable"), 
                            tr("Le projet sélectionné n'a pas pu être trouvé."));
        return;
    }
    
    ProjetDialog dialog(this);
    dialog.setEditMode(true);
    dialog.setProjet(projet);
    
    if (dialog.exec() == QDialog::Accepted) {
        Projet projetModifie = dialog.getProjet();
        ProjetManager::getInstance().modifierProjet(projetModifie);
        updateProjectList();
        refreshProjetDetails(projetId);
    }
}

void MainWindow::on_actionSupprimer_triggered()
{
    int projetId = getSelectedProjetId();
    if (projetId <= 0) {
        QMessageBox::warning(this, tr("Aucune sélection"), 
                            tr("Veuillez sélectionner un projet à supprimer."));
        return;
    }
    
    Projet projet = ProjetManager::getInstance().getProjetById(projetId);
    if (projet.getId() <= 0) {
        QMessageBox::warning(this, tr("Projet introuvable"), 
                            tr("Le projet sélectionné n'a pas pu être trouvé."));
        return;
    }
    
    QMessageBox::StandardButton reponse = QMessageBox::question(
        this, 
        tr("Confirmer la suppression"),
        tr("Êtes-vous sûr de vouloir supprimer le projet '%1' ?").arg(projet.getNom()),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reponse == QMessageBox::Yes) {
        ProjetManager::getInstance().supprimerProjet(projetId);
        updateProjectList();
        updateStatusBar();
        ui->groupBoxDetails->setVisible(false);
    }
}

void MainWindow::on_actionQuitter_triggered()
{
    close();
}

void MainWindow::on_actionAPropos_triggered()
{
    QMessageBox::about(this, tr("À propos de Gestion des Projets"),
                      tr("Gestion des Projets v1.0\n\n"
                         "Une application simple pour la gestion des projets d'architecture.\n\n"
                         "Fonctionnalités:\n"
                         "- Gestion complète des projets (CRUD)\n"
                         "- Recherche et tri\n"
                         "- Suivi du projet\n"
                         "- Estimation automatique de la durée"));
}

void MainWindow::on_tableViewProjets_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    
    // Récupérer l'ID du projet sélectionné
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableViewProjets->model());
    if (!model) {
        return;
    }
    
    int row = index.row();
    QModelIndex idIndex = model->index(row, 0);
    int projetId = model->data(idIndex).toInt();
    
    refreshProjetDetails(projetId);
}

void MainWindow::onSearchRequested()
{
    QString searchTerm = ui->widgetSearchFilter->getSearchTerm();
    QString categorie = ui->widgetSearchFilter->getCategorie();
    QDate dateDebut = ui->widgetSearchFilter->getDateDebut();
    QDate dateFin = ui->widgetSearchFilter->getDateFin();
    
    if (!searchTerm.isEmpty()) {
        m_projets = ProjetManager::getInstance().rechercherProjets(searchTerm);
    } else {
        m_projets = ProjetManager::getInstance().filtrerProjets(categorie, dateDebut, dateFin);
    }
    
    // Appliquer le tri actuel
    QString critere = ui->widgetSearchFilter->getSortCriteria();
    bool ascendant = ui->widgetSearchFilter->isSortAscending();
    m_projets = ProjetManager::getInstance().trierProjets(critere, ascendant);
    
    // Mettre à jour l'affichage
    updateProjectList();
    updateStatusBar();
}

void MainWindow::onResetRequested()
{
    loadProjets();
    updateStatusBar();
}

void MainWindow::onSortRequested(const QString &criteria, bool ascending)
{
    m_projets = ProjetManager::getInstance().trierProjets(criteria, ascending);
    updateProjectList();
}

void MainWindow::updateProjectList()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableViewProjets->model());
    if (!model) {
        return;
    }
    
    model->removeRows(0, model->rowCount());
    
    for (const Projet &projet : m_projets) {
        QList<QStandardItem*> row;
        
        row.append(new QStandardItem(QString::number(projet.getId())));
        row.append(new QStandardItem(projet.getNom()));
        row.append(new QStandardItem(projet.getCategorie()));
        row.append(new QStandardItem(projet.getDateDebut().toString("dd/MM/yyyy")));
        row.append(new QStandardItem(projet.getDateFinEstimee().toString("dd/MM/yyyy")));
        
        // Progression
        QStandardItem *progressItem = new QStandardItem(QString("%1 %").arg(projet.getProgression()));
        
        // Coloration selon la progression
        if (projet.estEnRetard()) {
            progressItem->setForeground(QBrush(Qt::red));
        } else if (projet.getProgression() >= 100) {
            progressItem->setForeground(QBrush(Qt::darkGreen));
        }
        
        row.append(progressItem);
        
        model->appendRow(row);
    }
}

void MainWindow::updateStatusBar()
{
    ui->statusBar->showMessage(tr("%1 projets trouvés").arg(m_projets.size()));
}

void MainWindow::setupConnections()
{
    // Connexions pour le widget de recherche et filtre
    connect(ui->widgetSearchFilter, &SearchFilter::searchRequested, 
            this, &MainWindow::onSearchRequested);
    
    connect(ui->widgetSearchFilter, &SearchFilter::resetRequested, 
            this, &MainWindow::onResetRequested);
    
    connect(ui->widgetSearchFilter, &SearchFilter::sortRequested, 
            this, &MainWindow::onSortRequested);
}

void MainWindow::setupTableView()
{
    QStandardItemModel *model = new QStandardItemModel(this);
    
    // Définir les en-têtes
    QStringList headers;
    headers << tr("ID") << tr("Nom") << tr("Catégorie") 
            << tr("Date début") << tr("Date fin") << tr("Progression");
    
    model->setHorizontalHeaderLabels(headers);
    
    ui->tableViewProjets->setModel(model);
    ui->tableViewProjets->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewProjets->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewProjets->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewProjets->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewProjets->verticalHeader()->setVisible(false);
    
    // Masquer la colonne ID
    ui->tableViewProjets->setColumnHidden(0, true);
    
    // Ajuster les largeurs des colonnes
    ui->tableViewProjets->setColumnWidth(1, 200); // Nom
    ui->tableViewProjets->setColumnWidth(2, 120); // Catégorie
    ui->tableViewProjets->setColumnWidth(3, 100); // Date début
    ui->tableViewProjets->setColumnWidth(4, 100); // Date fin
}

void MainWindow::setupTheme()
{
    // Appliquer le thème bleu moderne
    setStyleSheet(
        "QMainWindow { background-color: #f0f0f0; }"
        "QMenuBar { background-color: #2c3e50; color: white; }"
        "QMenuBar::item:selected { background-color: #34495e; }"
        "QMenu { background-color: #2c3e50; color: white; border: 1px solid #34495e; }"
        "QMenu::item:selected { background-color: #34495e; }"
        "QToolBar { background-color: #2c3e50; border: none; }"
        "QToolButton { background-color: transparent; color: white; padding: 5px; }"
        "QToolButton:hover { background-color: #34495e; }"
        "QStatusBar { background-color: #2c3e50; color: white; }"
        "QTableView { alternate-background-color: #e8f0fe; selection-background-color: #3498db; }"
        "QTableView::item:selected { color: white; }"
        "QHeaderView::section { background-color: #2c3e50; color: white; padding: 5px; }"
        "QPushButton { background-color: #3498db; color: white; border: none; padding: 5px 10px; border-radius: 3px; }"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #1c6ea4; }"
        "QLineEdit, QDateEdit, QComboBox, QSpinBox { border: 1px solid #bdc3c7; padding: 5px; border-radius: 3px; }"
        "QGroupBox { border: 1px solid #bdc3c7; border-radius: 5px; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; }"
    );
    
    // Masquer le groupe de détails par défaut
    ui->groupBoxDetails->setVisible(false);
}

void MainWindow::loadProjets()
{
    m_projets = ProjetManager::getInstance().getAllProjets();
    updateProjectList();
}

int MainWindow::getSelectedProjetId() const
{
    QModelIndexList selection = ui->tableViewProjets->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        return 0;
    }
    
    QModelIndex index = selection.first();
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableViewProjets->model());
    if (!model) {
        return 0;
    }
    
    QModelIndex idIndex = model->index(index.row(), 0);
    return model->data(idIndex).toInt();
}

void MainWindow::refreshProjetDetails(int projetId)
{
    Projet projet = ProjetManager::getInstance().getProjetById(projetId);
    if (projet.getId() <= 0) {
        ui->groupBoxDetails->setVisible(false);
        return;
    }
    
    // Mettre à jour les détails du projet
    ui->labelNomValeur->setText(projet.getNom());
    ui->labelCategorieValeur->setText(projet.getCategorie());
    ui->labelDateDebutValeur->setText(projet.getDateDebut().toString("dd/MM/yyyy"));
    ui->labelDateFinValeur->setText(projet.getDateFinEstimee().toString("dd/MM/yyyy"));
    ui->progressBarAvancement->setValue(projet.getProgression());
    
    // Calculer les jours restants
    int joursRestants = projet.getJoursRestants();
    QString joursRestantsText;
    
    if (joursRestants < 0) {
        joursRestantsText = tr("En retard de %1 jours").arg(abs(joursRestants));
        ui->labelJoursRestantsValeur->setStyleSheet("color: red;");
    } else if (projet.getProgression() >= 100) {
        joursRestantsText = tr("Projet terminé");
        ui->labelJoursRestantsValeur->setStyleSheet("color: green;");
    } else {
        joursRestantsText = tr("%1 jours").arg(joursRestants);
        ui->labelJoursRestantsValeur->setStyleSheet("");
    }
    
    ui->labelJoursRestantsValeur->setText(joursRestantsText);
    
    // Afficher le groupe de détails
    ui->groupBoxDetails->setVisible(true);
}
