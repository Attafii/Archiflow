#ifndef PROJET_H
#define PROJET_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QDate>
#include <QMetaType>
#include <QVariant>

/**
 * @brief The Coordinate struct - Simple coordinate structure for geographic data
 */
struct Coordinate {
    double latitude = 0.0;
    double longitude = 0.0;
    QString address;
    
    Coordinate() = default;
    Coordinate(double lat, double lon, const QString& addr = QString())
        : latitude(lat), longitude(lon), address(addr) {}
    
    bool isValid() const {
        return latitude >= -90.0 && latitude <= 90.0 && 
               longitude >= -180.0 && longitude <= 180.0;
    }
    
    QString toString() const {
        return QString("(%1, %2)").arg(latitude, 0, 'f', 6).arg(longitude, 0, 'f', 6);
    }
};

Q_DECLARE_METATYPE(Coordinate)

/**
 * @brief The Projet class - Core project entity for ArchiFlow
 * 
 * This class represents an architectural project with all its properties
 * including geographic location, timeline, and business data.
 */
class Projet : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ getId WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString nom READ getNom WRITE setNom NOTIFY nomChanged)
    Q_PROPERTY(QString description READ getDescription WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString categorie READ getCategorie WRITE setCategorie NOTIFY categorieChanged)
    Q_PROPERTY(QString statut READ getStatut WRITE setStatut NOTIFY statutChanged)
    Q_PROPERTY(double budget READ getBudget WRITE setBudget NOTIFY budgetChanged)
    Q_PROPERTY(QString client READ getClient WRITE setClient NOTIFY clientChanged)
    Q_PROPERTY(QString architecte READ getArchitecte WRITE setArchitecte NOTIFY architecteChanged)

public:
    explicit Projet(QObject *parent = nullptr);
    
    // Constructor with basic data
    Projet(const QString &nom, const QString &description, const QString &categorie, QObject *parent = nullptr);
    
    // Copy constructor
    Projet(const Projet &other);
    
    // Assignment operator
    Projet& operator=(const Projet &other);
    
    // Destructor
    ~Projet() override = default;

    // Basic properties
    int getId() const { return m_id; }
    void setId(int id);
    
    QString getNom() const { return m_nom; }
    void setNom(const QString &nom);
    
    QString getDescription() const { return m_description; }
    void setDescription(const QString &description);
    
    QString getCategorie() const { return m_categorie; }
    void setCategorie(const QString &categorie);
    
    QString getStatut() const { return m_statut; }
    void setStatut(const QString &statut);
    
    // Location properties
    Coordinate getLocation() const { return m_location; }
    void setLocation(const Coordinate &location);
    void setLocation(double latitude, double longitude, const QString &address = QString());
    
    // Timeline properties
    QDateTime getDateCreation() const { return m_dateCreation; }
    void setDateCreation(const QDateTime &dateCreation);
    
    QDateTime getDateModification() const { return m_dateModification; }
    void setDateModification(const QDateTime &dateModification);
    
    QDate getDateDebut() const { return m_dateDebut; }
    void setDateDebut(const QDate &dateDebut);
    
    QDate getDateFinEstimee() const { return m_dateFinEstimee; }
    void setDateFinEstimee(const QDate &dateFinEstimee);
    
    // Business properties
    double getBudget() const { return m_budget; }
    void setBudget(double budget);
    
    QString getClient() const { return m_client; }
    void setClient(const QString &client);
    
    QString getArchitecte() const { return m_architecte; }
    void setArchitecte(const QString &architecte);
    
    double getSurface() const { return m_surface; }
    void setSurface(double surface);
    
    int getEtage() const { return m_etage; }
    void setEtage(int etage);
    
    QString getMateriauPrincipal() const { return m_materiauPrincipal; }
    void setMateriauPrincipal(const QString &materiauPrincipal);
    
    int getProgression() const { return m_progression; }
    void setProgression(int progression);
    
    // Utility methods
    bool isValid() const;
    QString getStatusDisplayText() const;
    QString getCategoryDisplayText() const;
    QVariantMap toVariantMap() const;
    void fromVariantMap(const QVariantMap &map);
    
    // Static utility methods
    static QStringList getAvailableStatuses();
    static QStringList getAvailableCategories();
    static QString getDefaultStatus();
    
    // Comparison operators
    bool operator==(const Projet &other) const;
    bool operator!=(const Projet &other) const;

signals:
    void idChanged(int id);
    void nomChanged(const QString &nom);
    void descriptionChanged(const QString &description);
    void categorieChanged(const QString &categorie);
    void statutChanged(const QString &statut);
    void budgetChanged(double budget);
    void clientChanged(const QString &client);
    void architecteChanged(const QString &architecte);
    void locationChanged(const Coordinate &location);
    void progressionChanged(int progression);
    void projectModified();

private:
    void updateModificationDate();
    void emitProjectModified();

private:
    // Core identification
    int m_id;
    QString m_nom;
    QString m_description;
    QString m_categorie;
    QString m_statut;
    
    // Location data
    Coordinate m_location;
    
    // Timeline data
    QDateTime m_dateCreation;
    QDateTime m_dateModification;
    QDate m_dateDebut;
    QDate m_dateFinEstimee;
    
    // Business data
    double m_budget;
    QString m_client;
    QString m_architecte;
    double m_surface;
    int m_etage;
    QString m_materiauPrincipal;
    int m_progression;
};

Q_DECLARE_METATYPE(Projet)
Q_DECLARE_METATYPE(Projet*)

#endif // PROJET_H
