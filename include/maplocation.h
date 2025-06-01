#ifndef MAPLOCATION_H
#define MAPLOCATION_H

#include <QDialog>
#include "projet.h" // Inclut la définition conditionnelle de QGeoCoordinate

namespace Ui {
class MapLocation;
}

class MapLocation : public QDialog
{
    Q_OBJECT

public:
    explicit MapLocation(QWidget *parent = nullptr);
    ~MapLocation();
    
    void setCoordinate(const QGeoCoordinate &coordinate);
    QGeoCoordinate getCoordinate() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_btnSearch_clicked();

private:
    Ui::MapLocation *ui;
    QGeoCoordinate m_coordinate;
    
    void updateCoordinateDisplay();
    void setupMap();
};

#endif // MAPLOCATION_H
