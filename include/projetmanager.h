#ifndef PROJETMANAGER_H
#define PROJETMANAGER_H

#include "projet.h"
#include <QList>
#include <QMap>
#include <QString>
#include <QDate>

class ProjetManager
{
public:
    static ProjetManager& getInstance();
    
    // CRUD Operations
    bool ajouterProjet(Projet &projet);
    bool modifierProjet(const Projet &projet);
    bool supprimerProjet(int id);
    Projet getProjetById(int id) const;
    QList<Projet> getAllProjets() const;
    
    // Search and Sort
    QList<Projet> rechercherProjets(const QString &terme) const;
    QList<Projet> filtrerProjets(const QString &categorie, 
                                const QDate &dateDebut = QDate(), 
                                const QDate &dateFin = QDate()) const;
    QList<Projet> trierProjets(const QString &critere, bool ascendant = true) const;
    
    // Project Tracking
    bool mettreAJourProgression(int id, int progression);
    QList<Projet> getProjetsEnRetard() const;
    QList<Projet> getProjetsProchesDeLaFin() const;
    
    // Estimation
    int estimerDureeProjet(const QString &categorie, int complexite) const;
    
private:
    ProjetManager();
    ~ProjetManager();
    
    // Singleton instance
    static ProjetManager* m_instance;
    
    // Data storage
    QMap<int, Projet> m_projets;
    int m_nextId;
    
    // Helper methods
    void sauvegarderDonnees() const;
    void chargerDonnees();
    QString getCheminFichier() const;
};

#endif // PROJETMANAGER_H
