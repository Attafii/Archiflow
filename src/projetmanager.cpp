#include "projetmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

ProjetManager* ProjetManager::m_instance = nullptr;

ProjetManager& ProjetManager::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new ProjetManager();
    }
    return *m_instance;
}

ProjetManager::ProjetManager() : m_nextId(1)
{
    chargerDonnees();
}

ProjetManager::~ProjetManager()
{
    sauvegarderDonnees();
}

bool ProjetManager::ajouterProjet(Projet &projet)
{
    // Assigner un nouvel ID si nécessaire
    if (projet.getId() == 0) {
        projet.setId(m_nextId++);
    }
    
    // Ajouter le projet à la map
    m_projets.insert(projet.getId(), projet);
    
    // Sauvegarder les changements
    sauvegarderDonnees();
    
    return true;
}

bool ProjetManager::modifierProjet(const Projet &projet)
{
    if (!m_projets.contains(projet.getId())) {
        return false;
    }
    
    m_projets[projet.getId()] = projet;
    sauvegarderDonnees();
    
    return true;
}

bool ProjetManager::supprimerProjet(int id)
{
    if (!m_projets.contains(id)) {
        return false;
    }
    
    m_projets.remove(id);
    sauvegarderDonnees();
    
    return true;
}

Projet ProjetManager::getProjetById(int id) const
{
    if (m_projets.contains(id)) {
        return m_projets.value(id);
    }
    
    return Projet();
}

QList<Projet> ProjetManager::getAllProjets() const
{
    return m_projets.values();
}

QList<Projet> ProjetManager::rechercherProjets(const QString &terme) const
{
    QList<Projet> resultats;
    
    for (const Projet &projet : m_projets.values()) {
        if (projet.getNom().contains(terme, Qt::CaseInsensitive) ||
            projet.getCategorie().contains(terme, Qt::CaseInsensitive)) {
            resultats.append(projet);
        }
    }
    
    return resultats;
}

QList<Projet> ProjetManager::filtrerProjets(const QString &categorie, 
                                          const QDate &dateDebut, 
                                          const QDate &dateFin) const
{
    QList<Projet> resultats;
    
    for (const Projet &projet : m_projets.values()) {
        bool match = true;
        
        if (!categorie.isEmpty() && projet.getCategorie() != categorie) {
            match = false;
        }
        
        if (dateDebut.isValid() && projet.getDateDebut() < dateDebut) {
            match = false;
        }
        
        if (dateFin.isValid() && projet.getDateFinEstimee() > dateFin) {
            match = false;
        }
        
        if (match) {
            resultats.append(projet);
        }
    }
    
    return resultats;
}

QList<Projet> ProjetManager::trierProjets(const QString &critere, bool ascendant) const
{
    QList<Projet> projets = m_projets.values();
    
    if (critere == "nom") {
        std::sort(projets.begin(), projets.end(), [ascendant](const Projet &a, const Projet &b) {
            return ascendant ? (a.getNom() < b.getNom()) : (a.getNom() > b.getNom());
        });
    } else if (critere == "categorie") {
        std::sort(projets.begin(), projets.end(), [ascendant](const Projet &a, const Projet &b) {
            return ascendant ? (a.getCategorie() < b.getCategorie()) : (a.getCategorie() > b.getCategorie());
        });
    } else if (critere == "dateDebut") {
        std::sort(projets.begin(), projets.end(), [ascendant](const Projet &a, const Projet &b) {
            return ascendant ? (a.getDateDebut() < b.getDateDebut()) : (a.getDateDebut() > b.getDateDebut());
        });
    } else if (critere == "dateFin") {
        std::sort(projets.begin(), projets.end(), [ascendant](const Projet &a, const Projet &b) {
            return ascendant ? (a.getDateFinEstimee() < b.getDateFinEstimee()) : (a.getDateFinEstimee() > b.getDateFinEstimee());
        });
    } else if (critere == "progression") {
        std::sort(projets.begin(), projets.end(), [ascendant](const Projet &a, const Projet &b) {
            return ascendant ? (a.getProgression() < b.getProgression()) : (a.getProgression() > b.getProgression());
        });
    }
    
    return projets;
}

bool ProjetManager::mettreAJourProgression(int id, int progression)
{
    if (!m_projets.contains(id)) {
        return false;
    }
    
    Projet projet = m_projets.value(id);
    projet.setProgression(progression);
    m_projets[id] = projet;
    
    sauvegarderDonnees();
    
    return true;
}

QList<Projet> ProjetManager::getProjetsEnRetard() const
{
    QList<Projet> projetsEnRetard;
    
    for (const Projet &projet : m_projets.values()) {
        if (projet.estEnRetard()) {
            projetsEnRetard.append(projet);
        }
    }
    
    return projetsEnRetard;
}

QList<Projet> ProjetManager::getProjetsProchesDeLaFin() const
{
    QList<Projet> projetsProchesFin;
    const int JOURS_SEUIL = 7; // Projets qui se terminent dans moins d'une semaine
    
    for (const Projet &projet : m_projets.values()) {
        int joursRestants = projet.getJoursRestants();
        if (joursRestants >= 0 && joursRestants <= JOURS_SEUIL && projet.getProgression() < 100) {
            projetsProchesFin.append(projet);
        }
    }
    
    return projetsProchesFin;
}

int ProjetManager::estimerDureeProjet(const QString &categorie, int complexite) const
{
    // Estimation basée sur la catégorie et la complexité
    int dureeBase = 30; // Durée par défaut en jours
    
    // Ajustement selon la catégorie
    if (categorie == "Résidentiel") {
        dureeBase = 60;
    } else if (categorie == "Commercial") {
        dureeBase = 120;
    } else if (categorie == "Industriel") {
        dureeBase = 180;
    } else if (categorie == "Infrastructure") {
        dureeBase = 240;
    }
    
    // Ajustement selon la complexité (1-5)
    float facteurComplexite = 0.8f + (complexite * 0.2f);
    
    return static_cast<int>(dureeBase * facteurComplexite);
}

void ProjetManager::sauvegarderDonnees() const
{
    QJsonArray projetsArray;
    
    for (const Projet &projet : m_projets.values()) {
        QJsonObject projetObj;
        projetObj["id"] = projet.getId();
        projetObj["nom"] = projet.getNom();
        projetObj["categorie"] = projet.getCategorie();
        projetObj["dateDebut"] = projet.getDateDebut().toString(Qt::ISODate);
        projetObj["dateFin"] = projet.getDateFinEstimee().toString(Qt::ISODate);
        projetObj["progression"] = projet.getProgression();
        
        projetsArray.append(projetObj);
    }
    
    QJsonDocument doc(projetsArray);
    QFile file(getCheminFichier());
    
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        qDebug() << "Erreur lors de la sauvegarde des données: " << file.errorString();
    }
}

void ProjetManager::chargerDonnees()
{
    QFile file(getCheminFichier());
    
    if (!file.exists()) {
        return;
    }
    
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray projetsArray = doc.array();
        
        m_projets.clear();
        m_nextId = 1;
        
        for (const QJsonValue &value : projetsArray) {
            QJsonObject projetObj = value.toObject();
            
            int id = projetObj["id"].toInt();
            QString nom = projetObj["nom"].toString();
            QString categorie = projetObj["categorie"].toString();
            QDate dateDebut = QDate::fromString(projetObj["dateDebut"].toString(), Qt::ISODate);
            QDate dateFin = QDate::fromString(projetObj["dateFin"].toString(), Qt::ISODate);
            
            Projet projet(id, nom, categorie, dateDebut, dateFin);
            projet.setProgression(projetObj["progression"].toInt());
            
            m_projets.insert(id, projet);
            
            if (id >= m_nextId) {
                m_nextId = id + 1;
            }
        }
    } else {
        qDebug() << "Erreur lors du chargement des données: " << file.errorString();
    }
}

QString ProjetManager::getCheminFichier() const
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    return dir.filePath("projets.json");
}
