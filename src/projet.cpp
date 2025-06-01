#include "projet.h"
#include <QDateTime>

Projet::Projet() : 
    m_id(0), 
    m_progression(0)
{
}

Projet::Projet(int id, const QString &nom, const QString &categorie, 
               const QDate &dateDebut, const QDate &dateFin) :
    m_id(id),
    m_nom(nom),
    m_categorie(categorie),
    m_dateDebut(dateDebut),
    m_dateFinEstimee(dateFin),
    m_progression(0)
{
}

int Projet::getId() const
{
    return m_id;
}

QString Projet::getNom() const
{
    return m_nom;
}

QString Projet::getCategorie() const
{
    return m_categorie;
}

QDate Projet::getDateDebut() const
{
    return m_dateDebut;
}

QDate Projet::getDateFinEstimee() const
{
    return m_dateFinEstimee;
}

int Projet::getProgression() const
{
    return m_progression;
}

void Projet::setId(int id)
{
    m_id = id;
}

void Projet::setNom(const QString &nom)
{
    m_nom = nom;
}

void Projet::setCategorie(const QString &categorie)
{
    m_categorie = categorie;
}

void Projet::setDateDebut(const QDate &dateDebut)
{
    m_dateDebut = dateDebut;
}

void Projet::setDateFinEstimee(const QDate &dateFin)
{
    m_dateFinEstimee = dateFin;
}

void Projet::setProgression(int progression)
{
    if (progression >= 0 && progression <= 100)
        m_progression = progression;
}

int Projet::getDureeJours() const
{
    if (!m_dateDebut.isValid() || !m_dateFinEstimee.isValid())
        return 0;
    
    return m_dateDebut.daysTo(m_dateFinEstimee);
}

int Projet::getJoursRestants() const
{
    if (!m_dateFinEstimee.isValid())
        return 0;
    
    QDate aujourdhui = QDate::currentDate();
    return aujourdhui.daysTo(m_dateFinEstimee);
}

bool Projet::estEnRetard() const
{
    if (!m_dateFinEstimee.isValid())
        return false;
    
    QDate aujourdhui = QDate::currentDate();
    return m_dateFinEstimee < aujourdhui && m_progression < 100;
}
