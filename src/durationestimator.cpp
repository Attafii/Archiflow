#include "durationestimator.h"

DurationEstimator::DurationEstimator()
{
    // Initialiser les facteurs par catégorie
    m_facteurs = {
        {"Résidentiel", 60},
        {"Commercial", 120},
        {"Industriel", 180},
        {"Infrastructure", 240},
        {"Autre", 90}
    };
}

int DurationEstimator::estimerDuree(const QString &categorie, int complexite) const
{
    // Valeur par défaut
    int dureeBase = 90;
    
    // Rechercher la durée de base pour cette catégorie
    for (const FacteurCategorie &facteur : m_facteurs) {
        if (facteur.nom == categorie) {
            dureeBase = facteur.dureeBase;
            break;
        }
    }
    
    // Ajuster selon la complexité (1-5)
    // Complexité 1 = 80% de la durée de base
    // Complexité 5 = 160% de la durée de base
    float facteurComplexite = 0.8f + (complexite * 0.2f);
    
    return static_cast<int>(dureeBase * facteurComplexite);
}

QDate DurationEstimator::calculerDateFin(const QDate &dateDebut, const QString &categorie, int complexite) const
{
    if (!dateDebut.isValid()) {
        return QDate();
    }
    
    int dureeJours = estimerDuree(categorie, complexite);
    return dateDebut.addDays(dureeJours);
}
