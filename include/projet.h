#ifndef PROJET_H
#define PROJET_H

#include <QString>
#include <QDate>

class Projet
{
public:
    Projet();
    Projet(int id, const QString &nom, const QString &categorie, 
           const QDate &dateDebut, const QDate &dateFin);
    
    // Getters
    int getId() const;
    QString getNom() const;
    QString getCategorie() const;
    QDate getDateDebut() const;
    QDate getDateFinEstimee() const;
    int getProgression() const;
    
    // Setters
    void setId(int id);
    void setNom(const QString &nom);
    void setCategorie(const QString &categorie);
    void setDateDebut(const QDate &dateDebut);
    void setDateFinEstimee(const QDate &dateFin);
    void setProgression(int progression);
    
    // Méthodes utilitaires
    int getDureeJours() const;
    int getJoursRestants() const;
    bool estEnRetard() const;
    
private:
    int m_id;
    QString m_nom;
    QString m_categorie;
    QDate m_dateDebut;
    QDate m_dateFinEstimee;
    int m_progression; // Pourcentage de progression (0-100)
};

#endif // PROJET_H
