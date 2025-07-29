#ifndef PROJETMANAGER_H
#define PROJETMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QVariantList>
#include <QDebug>
#include <memory>

#include "projet.h"

class DatabaseManager;

/**
 * @brief The ProjetManager class - Data Access Layer for project management
 * 
 * This class handles all database operations for projects including CRUD operations,
 * search functionality, and business logic enforcement.
 */
class ProjetManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjetManager(DatabaseManager *databaseManager, QObject *parent = nullptr);
    ~ProjetManager() override = default;

    // Core CRUD operations
    bool ajouterProjet(const Projet &projet);
    bool modifierProjet(const Projet &projet);
    bool supprimerProjet(int id);
    Projet getProjet(int id);
    QList<Projet> getAllProjets();

    // Search and filter operations
    QList<Projet> rechercherProjets(const QString &terme, const QString &categorie = QString(), 
                                   const QString &statut = QString());
    QList<Projet> getProjetsByCategorie(const QString &categorie);
    QList<Projet> getProjetsByStatut(const QString &statut);
    QList<Projet> getProjetsByDateRange(const QDate &dateDebut, const QDate &dateFin);
    QList<Projet> getProjetsByClient(const QString &client);
    QList<Projet> getProjetsByArchitecte(const QString &architecte);

    // Business logic operations
    bool updateProgression(int projetId, int progression);
    bool updateStatut(int projetId, const QString &nouveauStatut);
    QStringList getAllClients();
    QStringList getAllArchitectes();
    QStringList getAllCategories();
    
    // Statistics and reports
    int getTotalProjets();
    int getProjetsByStatutCount(const QString &statut);
    double getTotalBudget();
    double getBudgetByCategorie(const QString &categorie);
    QVariantMap getStatistiques();

    // Validation
    bool validateProjet(const Projet &projet, QStringList &errors);
    bool projetExists(const QString &nom, int excludeId = -1);

    // Database management
    bool createTables();
    bool ensureDefaultData();

signals:
    void projetAjoute(const Projet &projet);
    void projetModifie(const Projet &projet);
    void projetSupprime(int id);
    void error(const QString &message);

private:
    bool executeNonQuery(const QString &query, const QVariantList &params = QVariantList());
    QSqlQuery executeQuery(const QString &query, const QVariantList &params = QVariantList());
    Projet projetFromQuery(const QSqlQuery &query);
    void logError(const QString &operation, const QSqlError &error);

private:
    DatabaseManager *m_databaseManager;
    QString m_lastError;
};

#endif // PROJETMANAGER_H
