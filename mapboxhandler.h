#ifndef MAPBOXHANDLER_H
#define MAPBOXHANDLER_H

#include <QObject>
#include <QGeoCoordinate>
#include <QWebChannel>
#include <QWebEngineView>

class MapboxHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(QString mapboxToken READ mapboxToken WRITE setMapboxToken NOTIFY mapboxTokenChanged)

public:
    explicit MapboxHandler(QObject *parent = nullptr);
    
    void setupWebChannel(QWebEngineView *webView);
    void updateLocation(const QString &location);
    
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    
    void setLatitude(double latitude);
    void setLongitude(double longitude);
    
    void setMapboxToken(const QString &token);
    QString mapboxToken() const { return m_mapboxToken; }
    
public slots:
    void updateCoordinates(double lat, double lng);
    
signals:
    void latitudeChanged(double latitude);
    void longitudeChanged(double longitude);
    void coordinatesChanged(double latitude, double longitude);
    void locationFieldShouldUpdate(const QString &address);
    void mapboxTokenChanged(const QString &token);
    
private:
    double m_latitude;
    double m_longitude;
    QString m_mapboxToken;
    QWebChannel *m_webChannel;
};

#endif // MAPBOXHANDLER_H