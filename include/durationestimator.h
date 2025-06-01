#ifndef DURATIONESTIMATOR_H
#define DURATIONESTIMATOR_H

#include <QString>
#include <QDate>
#include <QList>

class DurationEstimator
{
public:
    DurationEstimator();
    
    // Estimer la durée d'un projet en jours
    int estimerDuree(const QString &categorie, int complexite) const;
    
    // Calculer la date de fin estimée
    QDate calculerDateFin(const QDate &dateDebut, const QString &categorie, int complexite) const;
    
private:
    // Facteurs d'ajustement par catégorie
    struct FacteurCategorie {
        QString nom;
        int dureeBase;
    };
    
    QList<FacteurCategorie> m_facteurs;
};

#endif // DURATIONESTIMATOR_H
