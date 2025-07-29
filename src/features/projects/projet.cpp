#include "projet.h"
#include <QDebug>

Projet::Projet(QObject *parent)
    : QObject(parent)
    , m_id(-1)
    , m_nom("")
    , m_description("")
    , m_categorie("")
    , m_statut(getDefaultStatus())
    , m_location()
    , m_dateCreation(QDateTime::currentDateTime())
    , m_dateModification(QDateTime::currentDateTime())
    , m_dateDebut(QDate::currentDate())
    , m_dateFinEstimee(QDate::currentDate().addDays(30))
    , m_budget(0.0)
    , m_client("")
    , m_architecte("")
    , m_surface(0.0)
    , m_etage(0)
    , m_materiauPrincipal("")
    , m_progression(0)
{
}

Projet::Projet(const QString &nom, const QString &description, const QString &categorie, QObject *parent)
    : QObject(parent)
    , m_id(-1)
    , m_nom(nom)
    , m_description(description)
    , m_categorie(categorie)
    , m_statut(getDefaultStatus())
    , m_location()
    , m_dateCreation(QDateTime::currentDateTime())
    , m_dateModification(QDateTime::currentDateTime())
    , m_dateDebut(QDate::currentDate())
    , m_dateFinEstimee(QDate::currentDate().addDays(30))
    , m_budget(0.0)
    , m_client("")
    , m_architecte("")
    , m_surface(0.0)
    , m_etage(0)
    , m_materiauPrincipal("")
    , m_progression(0)
{
}

Projet::Projet(const Projet &other)
    : QObject(other.parent())
    , m_id(other.m_id)
    , m_nom(other.m_nom)
    , m_description(other.m_description)
    , m_categorie(other.m_categorie)
    , m_statut(other.m_statut)
    , m_location(other.m_location)
    , m_dateCreation(other.m_dateCreation)
    , m_dateModification(other.m_dateModification)
    , m_dateDebut(other.m_dateDebut)
    , m_dateFinEstimee(other.m_dateFinEstimee)
    , m_budget(other.m_budget)
    , m_client(other.m_client)
    , m_architecte(other.m_architecte)
    , m_surface(other.m_surface)
    , m_etage(other.m_etage)
    , m_materiauPrincipal(other.m_materiauPrincipal)
    , m_progression(other.m_progression)
{
}

Projet& Projet::operator=(const Projet &other)
{
    if (this != &other) {
        m_id = other.m_id;
        m_nom = other.m_nom;
        m_description = other.m_description;
        m_categorie = other.m_categorie;
        m_statut = other.m_statut;
        m_location = other.m_location;
        m_dateCreation = other.m_dateCreation;
        m_dateModification = other.m_dateModification;
        m_dateDebut = other.m_dateDebut;
        m_dateFinEstimee = other.m_dateFinEstimee;
        m_budget = other.m_budget;
        m_client = other.m_client;
        m_architecte = other.m_architecte;
        m_surface = other.m_surface;
        m_etage = other.m_etage;
        m_materiauPrincipal = other.m_materiauPrincipal;
        m_progression = other.m_progression;
        
        emitProjectModified();
    }
    return *this;
}

// Setters with change notifications
void Projet::setId(int id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged(id);
    }
}

void Projet::setNom(const QString &nom)
{
    if (m_nom != nom) {
        m_nom = nom;
        updateModificationDate();
        emit nomChanged(nom);
        emitProjectModified();
    }
}

void Projet::setDescription(const QString &description)
{
    if (m_description != description) {
        m_description = description;
        updateModificationDate();
        emit descriptionChanged(description);
        emitProjectModified();
    }
}

void Projet::setCategorie(const QString &categorie)
{
    if (m_categorie != categorie) {
        m_categorie = categorie;
        updateModificationDate();
        emit categorieChanged(categorie);
        emitProjectModified();
    }
}

void Projet::setStatut(const QString &statut)
{
    if (m_statut != statut) {
        m_statut = statut;
        updateModificationDate();
        emit statutChanged(statut);
        emitProjectModified();
    }
}

void Projet::setLocation(const Coordinate &location)
{
    if (!(m_location.latitude == location.latitude && 
          m_location.longitude == location.longitude &&
          m_location.address == location.address)) {
        m_location = location;
        updateModificationDate();
        emit locationChanged(location);
        emitProjectModified();
    }
}

void Projet::setLocation(double latitude, double longitude, const QString &address)
{
    setLocation(Coordinate(latitude, longitude, address));
}

void Projet::setDateCreation(const QDateTime &dateCreation)
{
    if (m_dateCreation != dateCreation) {
        m_dateCreation = dateCreation;
    }
}

void Projet::setDateModification(const QDateTime &dateModification)
{
    if (m_dateModification != dateModification) {
        m_dateModification = dateModification;
    }
}

void Projet::setDateDebut(const QDate &dateDebut)
{
    if (m_dateDebut != dateDebut) {
        m_dateDebut = dateDebut;
        updateModificationDate();
        emitProjectModified();
    }
}

void Projet::setDateFinEstimee(const QDate &dateFinEstimee)
{
    if (m_dateFinEstimee != dateFinEstimee) {
        m_dateFinEstimee = dateFinEstimee;
        updateModificationDate();
        emitProjectModified();
    }
}

void Projet::setBudget(double budget)
{
    if (m_budget != budget) {
        m_budget = budget;
        updateModificationDate();
        emit budgetChanged(budget);
        emitProjectModified();
    }
}

void Projet::setClient(const QString &client)
{
    if (m_client != client) {
        m_client = client;
        updateModificationDate();
        emit clientChanged(client);
        emitProjectModified();
    }
}

void Projet::setArchitecte(const QString &architecte)
{
    if (m_architecte != architecte) {
        m_architecte = architecte;
        updateModificationDate();
        emit architecteChanged(architecte);
        emitProjectModified();
    }
}

void Projet::setSurface(double surface)
{
    if (m_surface != surface) {
        m_surface = surface;
        updateModificationDate();
        emitProjectModified();
    }
}

void Projet::setEtage(int etage)
{
    if (m_etage != etage) {
        m_etage = etage;
        updateModificationDate();
        emitProjectModified();
    }
}

void Projet::setMateriauPrincipal(const QString &materiauPrincipal)
{
    if (m_materiauPrincipal != materiauPrincipal) {
        m_materiauPrincipal = materiauPrincipal;
        updateModificationDate();
        emitProjectModified();
    }
}

void Projet::setProgression(int progression)
{
    // Clamp progression between 0 and 100
    int clampedProgression = qMax(0, qMin(100, progression));
    if (m_progression != clampedProgression) {
        m_progression = clampedProgression;
        updateModificationDate();
        emit progressionChanged(clampedProgression);
        emitProjectModified();
    }
}

// Utility methods
bool Projet::isValid() const
{
    return !m_nom.isEmpty() && !m_categorie.isEmpty() && !m_statut.isEmpty();
}

QString Projet::getStatusDisplayText() const
{
    if (m_statut == "en_cours") return "En cours";
    if (m_statut == "termine") return "Terminé";
    if (m_statut == "en_attente") return "En attente";
    if (m_statut == "annule") return "Annulé";
    if (m_statut == "suspendu") return "Suspendu";
    return m_statut;
}

QString Projet::getCategoryDisplayText() const
{
    if (m_categorie == "residentiel") return "Résidentiel";
    if (m_categorie == "commercial") return "Commercial";
    if (m_categorie == "industriel") return "Industriel";
    if (m_categorie == "institutionnel") return "Institutionnel";
    if (m_categorie == "renovation") return "Rénovation";
    return m_categorie;
}

QVariantMap Projet::toVariantMap() const
{
    QVariantMap map;
    map["id"] = m_id;
    map["nom"] = m_nom;
    map["description"] = m_description;
    map["categorie"] = m_categorie;
    map["statut"] = m_statut;
    map["latitude"] = m_location.latitude;
    map["longitude"] = m_location.longitude;
    map["adresse"] = m_location.address;
    map["date_creation"] = m_dateCreation;
    map["date_modification"] = m_dateModification;
    map["date_debut"] = m_dateDebut;
    map["date_fin_estimee"] = m_dateFinEstimee;
    map["budget"] = m_budget;
    map["client"] = m_client;
    map["architecte"] = m_architecte;
    map["surface"] = m_surface;
    map["etage"] = m_etage;
    map["materiau_principal"] = m_materiauPrincipal;
    map["progression"] = m_progression;
    return map;
}

void Projet::fromVariantMap(const QVariantMap &map)
{
    m_id = map.value("id", -1).toInt();
    m_nom = map.value("nom").toString();
    m_description = map.value("description").toString();
    m_categorie = map.value("categorie").toString();
    m_statut = map.value("statut", getDefaultStatus()).toString();
    
    m_location.latitude = map.value("latitude", 0.0).toDouble();
    m_location.longitude = map.value("longitude", 0.0).toDouble();
    m_location.address = map.value("adresse").toString();
    
    m_dateCreation = map.value("date_creation", QDateTime::currentDateTime()).toDateTime();
    m_dateModification = map.value("date_modification", QDateTime::currentDateTime()).toDateTime();
    m_dateDebut = map.value("date_debut", QDate::currentDate()).toDate();
    m_dateFinEstimee = map.value("date_fin_estimee", QDate::currentDate().addDays(30)).toDate();
    
    m_budget = map.value("budget", 0.0).toDouble();
    m_client = map.value("client").toString();
    m_architecte = map.value("architecte").toString();
    m_surface = map.value("surface", 0.0).toDouble();
    m_etage = map.value("etage", 0).toInt();
    m_materiauPrincipal = map.value("materiau_principal").toString();
    m_progression = map.value("progression", 0).toInt();
}

// Static utility methods
QStringList Projet::getAvailableStatuses()
{
    return QStringList() << "en_cours" << "termine" << "en_attente" << "annule" << "suspendu";
}

QStringList Projet::getAvailableCategories()
{
    return QStringList() << "residentiel" << "commercial" << "industriel" << "institutionnel" << "renovation";
}

QString Projet::getDefaultStatus()
{
    return "en_cours";
}

// Comparison operators
bool Projet::operator==(const Projet &other) const
{
    return m_id == other.m_id;
}

bool Projet::operator!=(const Projet &other) const
{
    return !(*this == other);
}

// Private helper methods
void Projet::updateModificationDate()
{
    m_dateModification = QDateTime::currentDateTime();
}

void Projet::emitProjectModified()
{
    emit projectModified();
}
