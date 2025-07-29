#include "projetmanager.h"
#include "database/databasemanager.h"
#include <QSqlRecord>
#include <QDebug>

ProjetManager::ProjetManager(DatabaseManager *databaseManager, QObject *parent)
    : QObject(parent)
    , m_databaseManager(databaseManager)
{
    Q_ASSERT(m_databaseManager);
}

bool ProjetManager::ajouterProjet(const Projet &projet)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        m_lastError = "Database not connected";
        emit error(m_lastError);
        return false;
    }

    QStringList validationErrors;
    if (!validateProjet(projet, validationErrors)) {
        m_lastError = "Validation failed: " + validationErrors.join("; ");
        emit error(m_lastError);
        return false;
    }

    QString insertQuery = R"(
        INSERT INTO projets (
            nom, description, categorie, statut, latitude, longitude, adresse,
            date_creation, date_modification, date_debut, date_fin_estimee,
            budget, client, architecte, surface, etage, materiau_principal, progression
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    QVariantList params;
    params << projet.getNom()
           << projet.getDescription()
           << projet.getCategorie()
           << projet.getStatut()
           << projet.getLocation().latitude
           << projet.getLocation().longitude
           << projet.getLocation().address
           << projet.getDateCreation()
           << projet.getDateModification()
           << projet.getDateDebut()
           << projet.getDateFinEstimee()
           << projet.getBudget()
           << projet.getClient()
           << projet.getArchitecte()
           << projet.getSurface()
           << projet.getEtage()
           << projet.getMateriauPrincipal()
           << projet.getProgression();

    bool success = executeNonQuery(insertQuery, params);
    if (success) {
        qDebug() << "Project added successfully:" << projet.getNom();
        emit projetAjoute(projet);
    }
    
    return success;
}

bool ProjetManager::modifierProjet(const Projet &projet)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        m_lastError = "Database not connected";
        emit error(m_lastError);
        return false;
    }

    if (projet.getId() <= 0) {
        m_lastError = "Invalid project ID for update";
        emit error(m_lastError);
        return false;
    }

    QStringList validationErrors;
    if (!validateProjet(projet, validationErrors)) {
        m_lastError = "Validation failed: " + validationErrors.join("; ");
        emit error(m_lastError);
        return false;
    }

    QString updateQuery = R"(
        UPDATE projets SET
            nom = ?, description = ?, categorie = ?, statut = ?,
            latitude = ?, longitude = ?, adresse = ?,
            date_modification = ?, date_debut = ?, date_fin_estimee = ?,
            budget = ?, client = ?, architecte = ?, surface = ?,
            etage = ?, materiau_principal = ?, progression = ?
        WHERE id = ?
    )";

    QVariantList params;
    params << projet.getNom()
           << projet.getDescription()
           << projet.getCategorie()
           << projet.getStatut()
           << projet.getLocation().latitude
           << projet.getLocation().longitude
           << projet.getLocation().address
           << QDateTime::currentDateTime()
           << projet.getDateDebut()
           << projet.getDateFinEstimee()
           << projet.getBudget()
           << projet.getClient()
           << projet.getArchitecte()
           << projet.getSurface()
           << projet.getEtage()
           << projet.getMateriauPrincipal()
           << projet.getProgression()
           << projet.getId();

    bool success = executeNonQuery(updateQuery, params);
    if (success) {
        qDebug() << "Project updated successfully:" << projet.getNom();
        emit projetModifie(projet);
    }
    
    return success;
}

bool ProjetManager::supprimerProjet(int id)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        m_lastError = "Database not connected";
        emit error(m_lastError);
        return false;
    }

    if (id <= 0) {
        m_lastError = "Invalid project ID for deletion";
        emit error(m_lastError);
        return false;
    }

    QString deleteQuery = "DELETE FROM projets WHERE id = ?";
    bool success = executeNonQuery(deleteQuery, {id});
    
    if (success) {
        qDebug() << "Project deleted successfully, ID:" << id;
        emit projetSupprime(id);
    }
    
    return success;
}

Projet ProjetManager::getProjet(int id)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return Projet();
    }

    QString selectQuery = "SELECT * FROM projets WHERE id = ?";
    QSqlQuery query = executeQuery(selectQuery, {id});
    
    if (query.next()) {
        return projetFromQuery(query);
    }
    
    return Projet();
}

QList<Projet> ProjetManager::getAllProjets()
{
    QList<Projet> projets;
    
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return projets;
    }

    QString selectQuery = "SELECT * FROM projets ORDER BY date_creation DESC";
    QSqlQuery query = executeQuery(selectQuery);
    
    while (query.next()) {
        projets.append(projetFromQuery(query));
    }
    
    qDebug() << "Loaded" << projets.size() << "projects from database";
    return projets;
}

QList<Projet> ProjetManager::rechercherProjets(const QString &terme, const QString &categorie, const QString &statut)
{
    QList<Projet> projets;
    
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return projets;
    }

    QString whereClause = "WHERE 1=1";
    QVariantList params;

    if (!terme.isEmpty()) {
        whereClause += " AND (nom LIKE ? OR description LIKE ? OR client LIKE ? OR architecte LIKE ?)";
        QString searchTerm = "%" + terme + "%";
        params << searchTerm << searchTerm << searchTerm << searchTerm;
    }

    if (!categorie.isEmpty()) {
        whereClause += " AND categorie = ?";
        params << categorie;
    }

    if (!statut.isEmpty()) {
        whereClause += " AND statut = ?";
        params << statut;
    }

    QString selectQuery = QString("SELECT * FROM projets %1 ORDER BY date_creation DESC").arg(whereClause);
    QSqlQuery query = executeQuery(selectQuery, params);
    
    while (query.next()) {
        projets.append(projetFromQuery(query));
    }
    
    return projets;
}

QList<Projet> ProjetManager::getProjetsByCategorie(const QString &categorie)
{
    return rechercherProjets("", categorie, "");
}

QList<Projet> ProjetManager::getProjetsByStatut(const QString &statut)
{
    return rechercherProjets("", "", statut);
}

QList<Projet> ProjetManager::getProjetsByDateRange(const QDate &dateDebut, const QDate &dateFin)
{
    QList<Projet> projets;
    
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return projets;
    }

    QString selectQuery = R"(
        SELECT * FROM projets 
        WHERE date_debut BETWEEN ? AND ? 
        ORDER BY date_debut ASC
    )";
    
    QSqlQuery query = executeQuery(selectQuery, {dateDebut, dateFin});
    
    while (query.next()) {
        projets.append(projetFromQuery(query));
    }
    
    return projets;
}

QList<Projet> ProjetManager::getProjetsByClient(const QString &client)
{
    QList<Projet> projets;
    
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return projets;
    }

    QString selectQuery = "SELECT * FROM projets WHERE client LIKE ? ORDER BY date_creation DESC";
    QSqlQuery query = executeQuery(selectQuery, {"%" + client + "%"});
    
    while (query.next()) {
        projets.append(projetFromQuery(query));
    }
    
    return projets;
}

QList<Projet> ProjetManager::getProjetsByArchitecte(const QString &architecte)
{
    QList<Projet> projets;
    
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return projets;
    }

    QString selectQuery = "SELECT * FROM projets WHERE architecte LIKE ? ORDER BY date_creation DESC";
    QSqlQuery query = executeQuery(selectQuery, {"%" + architecte + "%"});
    
    while (query.next()) {
        projets.append(projetFromQuery(query));
    }
    
    return projets;
}

bool ProjetManager::updateProgression(int projetId, int progression)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        m_lastError = "Database not connected";
        emit error(m_lastError);
        return false;
    }

    int clampedProgression = qMax(0, qMin(100, progression));
    QString updateQuery = "UPDATE projets SET progression = ?, date_modification = ? WHERE id = ?";
    
    return executeNonQuery(updateQuery, {clampedProgression, QDateTime::currentDateTime(), projetId});
}

bool ProjetManager::updateStatut(int projetId, const QString &nouveauStatut)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        m_lastError = "Database not connected";
        emit error(m_lastError);
        return false;
    }

    QString updateQuery = "UPDATE projets SET statut = ?, date_modification = ? WHERE id = ?";
    
    return executeNonQuery(updateQuery, {nouveauStatut, QDateTime::currentDateTime(), projetId});
}

QStringList ProjetManager::getAllClients()
{
    QStringList clients;
    
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return clients;
    }

    QString selectQuery = "SELECT DISTINCT client FROM projets WHERE client IS NOT NULL AND client != '' ORDER BY client";
    QSqlQuery query = executeQuery(selectQuery);
    
    while (query.next()) {
        clients.append(query.value(0).toString());
    }
    
    return clients;
}

QStringList ProjetManager::getAllArchitectes()
{
    QStringList architectes;
    
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return architectes;
    }

    QString selectQuery = "SELECT DISTINCT architecte FROM projets WHERE architecte IS NOT NULL AND architecte != '' ORDER BY architecte";
    QSqlQuery query = executeQuery(selectQuery);
    
    while (query.next()) {
        architectes.append(query.value(0).toString());
    }
    
    return architectes;
}

QStringList ProjetManager::getAllCategories()
{
    QStringList categories;
    
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return categories;
    }

    QString selectQuery = "SELECT DISTINCT categorie FROM projets WHERE categorie IS NOT NULL AND categorie != '' ORDER BY categorie";
    QSqlQuery query = executeQuery(selectQuery);
    
    while (query.next()) {
        categories.append(query.value(0).toString());
    }
    
    return categories;
}

int ProjetManager::getTotalProjets()
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return 0;
    }

    QString selectQuery = "SELECT COUNT(*) FROM projets";
    QSqlQuery query = executeQuery(selectQuery);
    
    if (query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

int ProjetManager::getProjetsByStatutCount(const QString &statut)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return 0;
    }

    QString selectQuery = "SELECT COUNT(*) FROM projets WHERE statut = ?";
    QSqlQuery query = executeQuery(selectQuery, {statut});
    
    if (query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

double ProjetManager::getTotalBudget()
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return 0.0;
    }

    QString selectQuery = "SELECT SUM(budget) FROM projets";
    QSqlQuery query = executeQuery(selectQuery);
    
    if (query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}

double ProjetManager::getBudgetByCategorie(const QString &categorie)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return 0.0;
    }

    QString selectQuery = "SELECT SUM(budget) FROM projets WHERE categorie = ?";
    QSqlQuery query = executeQuery(selectQuery, {categorie});
    
    if (query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}

QVariantMap ProjetManager::getStatistiques()
{
    QVariantMap stats;
    
    stats["total_projets"] = getTotalProjets();
    stats["total_budget"] = getTotalBudget();
    
    // Status statistics
    QStringList statuses = Projet::getAvailableStatuses();
    for (const QString &status : statuses) {
        stats[QString("count_%1").arg(status)] = getProjetsByStatutCount(status);
    }
    
    // Category statistics
    QStringList categories = getAllCategories();
    for (const QString &category : categories) {
        stats[QString("budget_%1").arg(category)] = getBudgetByCategorie(category);
    }
    
    return stats;
}

bool ProjetManager::validateProjet(const Projet &projet, QStringList &errors)
{
    errors.clear();
    
    if (projet.getNom().isEmpty()) {
        errors.append("Le nom du projet est obligatoire");
    }
    
    if (projet.getCategorie().isEmpty()) {
        errors.append("La catégorie du projet est obligatoire");
    }
    
    if (projet.getBudget() < 0) {
        errors.append("Le budget ne peut pas être négatif");
    }
    
    if (projet.getProgression() < 0 || projet.getProgression() > 100) {
        errors.append("La progression doit être entre 0 et 100");
    }
    
    if (!Projet::getAvailableStatuses().contains(projet.getStatut())) {
        errors.append("Statut invalide");
    }
    
    if (!Projet::getAvailableCategories().contains(projet.getCategorie()) && !projet.getCategorie().isEmpty()) {
        // Allow custom categories but warn
        qDebug() << "Custom category used:" << projet.getCategorie();
    }
    
    return errors.isEmpty();
}

bool ProjetManager::projetExists(const QString &nom, int excludeId)
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return false;
    }

    QString selectQuery = "SELECT COUNT(*) FROM projets WHERE nom = ?";
    QVariantList params = {nom};
    
    if (excludeId > 0) {
        selectQuery += " AND id != ?";
        params.append(excludeId);
    }
    
    QSqlQuery query = executeQuery(selectQuery, params);
    
    if (query.next()) {
        return query.value(0).toInt() > 0;
    }
    
    return false;
}

bool ProjetManager::createTables()
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return false;
    }

    QString createProjectsTable = R"(
        CREATE TABLE IF NOT EXISTS projets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nom TEXT NOT NULL,
            description TEXT,
            categorie TEXT NOT NULL,
            statut TEXT NOT NULL DEFAULT 'en_cours',
            latitude REAL DEFAULT 0.0,
            longitude REAL DEFAULT 0.0,
            adresse TEXT,
            date_creation DATETIME DEFAULT CURRENT_TIMESTAMP,
            date_modification DATETIME DEFAULT CURRENT_TIMESTAMP,
            date_debut DATE,
            date_fin_estimee DATE,
            budget REAL DEFAULT 0.0,
            client TEXT,
            architecte TEXT,
            surface REAL DEFAULT 0.0,
            etage INTEGER DEFAULT 0,
            materiau_principal TEXT,
            progression INTEGER DEFAULT 0 CHECK (progression >= 0 AND progression <= 100)
        )
    )";

    bool success = executeNonQuery(createProjectsTable);
    
    if (success) {
        // Create indexes for better performance
        executeNonQuery("CREATE INDEX IF NOT EXISTS idx_projets_nom ON projets(nom)");
        executeNonQuery("CREATE INDEX IF NOT EXISTS idx_projets_categorie ON projets(categorie)");
        executeNonQuery("CREATE INDEX IF NOT EXISTS idx_projets_statut ON projets(statut)");
        executeNonQuery("CREATE INDEX IF NOT EXISTS idx_projets_client ON projets(client)");
        executeNonQuery("CREATE INDEX IF NOT EXISTS idx_projets_architecte ON projets(architecte)");
        executeNonQuery("CREATE INDEX IF NOT EXISTS idx_projets_date_creation ON projets(date_creation)");
    }
    
    return success;
}

bool ProjetManager::ensureDefaultData()
{
    // This method can be used to insert sample projects if needed
    // For now, we'll just ensure the table exists
    return createTables();
}

// Private helper methods
bool ProjetManager::executeNonQuery(const QString &query, const QVariantList &params)
{
    return m_databaseManager->executeNonQuery(query, params);
}

QSqlQuery ProjetManager::executeQuery(const QString &query, const QVariantList &params)
{
    return m_databaseManager->executeQuery(query, params);
}

Projet ProjetManager::projetFromQuery(const QSqlQuery &query)
{
    Projet projet;
    
    projet.setId(query.value("id").toInt());
    projet.setNom(query.value("nom").toString());
    projet.setDescription(query.value("description").toString());
    projet.setCategorie(query.value("categorie").toString());
    projet.setStatut(query.value("statut").toString());
    
    double lat = query.value("latitude").toDouble();
    double lon = query.value("longitude").toDouble();
    QString addr = query.value("adresse").toString();
    projet.setLocation(lat, lon, addr);
    
    projet.setDateCreation(query.value("date_creation").toDateTime());
    projet.setDateModification(query.value("date_modification").toDateTime());
    projet.setDateDebut(query.value("date_debut").toDate());
    projet.setDateFinEstimee(query.value("date_fin_estimee").toDate());
    
    projet.setBudget(query.value("budget").toDouble());
    projet.setClient(query.value("client").toString());
    projet.setArchitecte(query.value("architecte").toString());
    projet.setSurface(query.value("surface").toDouble());
    projet.setEtage(query.value("etage").toInt());
    projet.setMateriauPrincipal(query.value("materiau_principal").toString());
    projet.setProgression(query.value("progression").toInt());
    
    return projet;
}

void ProjetManager::logError(const QString &operation, const QSqlError &error)
{
    QString errorMessage = QString("Database error in %1: %2").arg(operation, error.text());
    m_lastError = errorMessage;
    qCritical() << errorMessage;
    emit this->error(errorMessage);
}
