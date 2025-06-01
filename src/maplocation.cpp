#include "maplocation.h"
#include "ui_maplocation.h"
#include <QMessageBox>
#include <QUrl>

MapLocation::MapLocation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapLocation)
{
    ui->setupUi(this);
    setupMap();
    
    // Coordonnées par défaut (Paris, France)
    m_coordinate = QGeoCoordinate(48.8566, 2.3522);
    updateCoordinateDisplay();
}

MapLocation::~MapLocation()
{
    delete ui;
}

void MapLocation::setCoordinate(const QGeoCoordinate &coordinate)
{
    if (coordinate.isValid()) {
        m_coordinate = coordinate;
        updateCoordinateDisplay();
    }
}

QGeoCoordinate MapLocation::getCoordinate() const
{
    return m_coordinate;
}

void MapLocation::on_buttonBox_accepted()
{
    // Vérifier si les coordonnées sont valides
    double latitude = ui->doubleSpinBoxLatitude->value();
    double longitude = ui->doubleSpinBoxLongitude->value();
    
    QGeoCoordinate coordinate(latitude, longitude);
    if (!coordinate.isValid()) {
        QMessageBox::warning(this, tr("Coordonnées invalides"), 
                            tr("Veuillez entrer des coordonnées valides."));
        return;
    }
    
    m_coordinate = coordinate;
    accept();
}

void MapLocation::on_buttonBox_rejected()
{
    reject();
}

void MapLocation::on_btnSearch_clicked()
{
    QString address = ui->lineEditAddress->text().trimmed();
    if (address.isEmpty()) {
        QMessageBox::warning(this, tr("Adresse vide"), 
                            tr("Veuillez entrer une adresse à rechercher."));
        return;
    }
    
    // Dans une application réelle, nous utiliserions un service de géocodage
    // Pour cette démo, nous allons simplement afficher un message
    QMessageBox::information(this, tr("Recherche d'adresse"), 
                           tr("La recherche d'adresse nécessite une API de géocodage.\n"
                              "Dans une application complète, cette fonctionnalité serait implémentée."));
    
    // Simuler une recherche réussie pour la démo
    // Dans une application réelle, ces valeurs viendraient du service de géocodage
    if (address.contains("paris", Qt::CaseInsensitive)) {
        m_coordinate = QGeoCoordinate(48.8566, 2.3522);
    } else if (address.contains("lyon", Qt::CaseInsensitive)) {
        m_coordinate = QGeoCoordinate(45.7640, 4.8357);
    } else if (address.contains("marseille", Qt::CaseInsensitive)) {
        m_coordinate = QGeoCoordinate(43.2965, 5.3698);
    } else {
        // Coordonnées aléatoires en France pour la démo
        double lat = 46.0 + (qrand() % 400) / 100.0;
        double lon = 2.0 + (qrand() % 400) / 100.0;
        m_coordinate = QGeoCoordinate(lat, lon);
    }
    
    updateCoordinateDisplay();
}

void MapLocation::updateCoordinateDisplay()
{
    ui->doubleSpinBoxLatitude->setValue(m_coordinate.latitude());
    ui->doubleSpinBoxLongitude->setValue(m_coordinate.longitude());
    
    // Dans une application réelle, nous mettrions à jour la carte ici
    // Pour cette démo, nous affichons simplement les coordonnées
    ui->labelMapPlaceholder->setText(
        tr("Carte: Lat %1, Long %2").arg(m_coordinate.latitude()).arg(m_coordinate.longitude())
    );
}

void MapLocation::setupMap()
{
    // Dans une application réelle, nous initialiserions ici un widget de carte
    // Pour cette démo, nous utilisons simplement un QLabel comme placeholder
    ui->labelMapPlaceholder->setStyleSheet(
        "QLabel { background-color: #e0e0e0; border: 1px solid #a0a0a0; }"
    );
    ui->labelMapPlaceholder->setAlignment(Qt::AlignCenter);
    
#if defined(HAS_QT_POSITIONING)
    ui->labelMapPlaceholder->setText(tr("Carte (fonctionnalité complète)"));
#else
    ui->labelMapPlaceholder->setText(tr("Carte (fonctionnalité limitée - Qt Positioning non disponible)"));
#endif
}
