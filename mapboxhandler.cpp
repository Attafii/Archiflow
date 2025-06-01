#include "mapboxhandler.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QUrl>

MapboxHandler::MapboxHandler(QObject *parent)
    : QObject(parent)
    , m_latitude(0.0)
    , m_longitude(0.0)
    , m_webChannel(nullptr)
{
}

void MapboxHandler::setupWebChannel(QWebEngineView *webView)
{
    if (!webView) return;
    
    m_webChannel = new QWebChannel(this);
    m_webChannel->registerObject(QStringLiteral("mapboxHandler"), this);
    webView->page()->setWebChannel(m_webChannel);
    
    // Initialize the map with Mapbox
    QString mapHtml = QString(R"(
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8">
        <title>Mapbox Map</title>
        <meta name="viewport" content="initial-scale=1,maximum-scale=1,user-scalable=no">
        <link href="https://api.mapbox.com/mapbox-gl-js/v2.14.1/mapbox-gl.css" rel="stylesheet">
        <script src="https://api.mapbox.com/mapbox-gl-js/v2.14.1/mapbox-gl.js"></script>
        <script src="qrc:///qtwebchannel/qwebchannel.js"></script>
        <style>
            body { margin: 0; padding: 0; }
            #map { position: absolute; top: 0; bottom: 0; width: 100%; }
        </style>
    </head>
    <body>
        <div id="map"></div>
        <script>
            var mapboxHandler;
            var map;
            var marker;
            
            // Initialize QWebChannel
            new QWebChannel(qt.webChannelTransport, function(channel) {
                mapboxHandler = channel.objects.mapboxHandler;
                
                // Get Mapbox token from C++
                var token = mapboxHandler.mapboxToken;
                
                // Initialize map
                mapboxgl.accessToken = token;
                map = new mapboxgl.Map({
                    container: 'map',
                    style: 'mapbox://styles/mapbox/streets-v12',
                    center: [mapboxHandler.longitude, mapboxHandler.latitude],
                    zoom: 13
                });
                
                // Add marker
                marker = new mapboxgl.Marker()
                    .setLngLat([mapboxHandler.longitude, mapboxHandler.latitude])
                    .addTo(map);
                
                // Listen for coordinate changes from C++
                mapboxHandler.coordinatesChanged.connect(function(lat, lng) {
                    map.setCenter([lng, lat]);
                    marker.setLngLat([lng, lat]);
                });
                
                // Map click event to update coordinates
                map.on('click', function(e) {
                    var lngLat = e.lngLat;
                    mapboxHandler.updateCoordinates(lngLat.lat, lngLat.lng);
                });
            });
        </script>
    </body>
    </html>
    )");
    
    webView->setHtml(mapHtml);
}

void MapboxHandler::updateLocation(const QString &location)
{
    if (location.isEmpty()) return;
    
    // Geocode the location using Mapbox Geocoding API
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    
    // Create the request URL
    QString geocodingUrl = QString("https://api.mapbox.com/geocoding/v5/mapbox.places/%1.json")
                            .arg(location);
    
    QUrl url(geocodingUrl);
    QUrlQuery query;
    query.addQueryItem("access_token", m_mapboxToken);
    query.addQueryItem("limit", "1"); // Limit to one result
    url.setQuery(query);
    
    QNetworkRequest request(url);
    
    // Send the request
    QNetworkReply *reply = manager->get(request);
    
    // Connect to the finished signal
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse the JSON response
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            
            // Get the first feature (most relevant result)
            QJsonArray features = obj["features"].toArray();
            if (!features.isEmpty()) {
                QJsonObject feature = features.first().toObject();
                QJsonArray coordinates = feature["center"].toArray();
                
                if (coordinates.size() >= 2) {
                    // Mapbox returns coordinates as [longitude, latitude]
                    double lng = coordinates.at(0).toDouble();
                    double lat = coordinates.at(1).toDouble();
                    
                    // Update the coordinates
                    setLongitude(lng);
                    setLatitude(lat);
                    emit coordinatesChanged(m_latitude, m_longitude);
                }
            }
        }
        
        reply->deleteLater();
        sender()->deleteLater();
    });
}


void MapboxHandler::setLatitude(double latitude)
{
    if (qFuzzyCompare(m_latitude, latitude))
        return;
        
    m_latitude = latitude;
    emit latitudeChanged(latitude);
}

void MapboxHandler::setLongitude(double longitude)
{
    if (qFuzzyCompare(m_longitude, longitude))
        return;
        
    m_longitude = longitude;
    emit longitudeChanged(longitude);
}

void MapboxHandler::setMapboxToken(const QString &token)
{
    if (m_mapboxToken != token) {
        m_mapboxToken = token;
        emit mapboxTokenChanged(m_mapboxToken);
    }
}

void MapboxHandler::updateCoordinates(double lat, double lng)
{
    setLatitude(lat);
    setLongitude(lng);
    emit coordinatesChanged(lat, lng);

    // Reverse geocode the coordinates to get the address
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString geocodingUrl = QString("https://api.mapbox.com/geocoding/v5/mapbox.places/%1,%2.json")
        .arg(lng, 0, 'f', 8).arg(lat, 0, 'f', 8);
    QUrl url(geocodingUrl);
    QUrlQuery query;
    query.addQueryItem("access_token", m_mapboxToken);
    query.addQueryItem("limit", "1");
    url.setQuery(query);
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            QJsonArray features = obj["features"].toArray();
            if (!features.isEmpty()) {
                QJsonObject feature = features.first().toObject();
                QString address = feature["place_name"].toString();
                // Emit a signal or call a method to update the location field in the UI
                emit locationFieldShouldUpdate(address);
            }
        }
        reply->deleteLater();
        sender()->deleteLater();
    });
}
