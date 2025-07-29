#ifndef MAPBOXHANDLER_H
#define MAPBOXHANDLER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief The MapboxHandler class - Mapbox integration with real API
 * 
 * This class handles Mapbox API interactions including geocoding,
 * map display using interactive widgets, and marker management.
 */
class MapboxHandler : public QObject
{
    Q_OBJECT

public:
    explicit MapboxHandler(QObject *parent = nullptr);
    virtual ~MapboxHandler() = default;
    
    void setApiKey(const QString &apiKey);
    void displayLocation(double latitude, double longitude);
    void geocodeAddress(const QString &address);
    QWidget* createMapWidget();
    
    // Additional methods needed by client widget
    void showLocation(double latitude, double longitude, const QString &title = QString());
    void addMarker(double latitude, double longitude, const QString &title = QString());

signals:
    void locationFound(double latitude, double longitude);
    void geocodingCompleted(const QString &address, double lat, double lng);
    void geocodingFailed(const QString &error);

private slots:
    void handleGeocodeResponse();
    void onOpenInBrowser();
    void onCopyCoordinates();

private:
    QString m_apiKey;
    QWidget *m_mapWidget;
    QLabel *m_mapPlaceholder;
    QTextEdit *m_locationInfo;
    QPushButton *m_openMapBtn;
    QPushButton *m_copyBtn;
    QNetworkAccessManager *m_networkManager;
    
    double m_currentLat;
    double m_currentLng;
    QString m_currentTitle;
    
    void loadEnvironmentVariables();
    void updateMapDisplay();
    QString formatLocationInfo(double lat, double lng, const QString &title = QString());
    QString generateMapUrl(double lat, double lng, int zoom = 15);
};

#endif // MAPBOXHANDLER_H
