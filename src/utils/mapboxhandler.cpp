#include "mapboxhandler.h"
#include "environmentloader.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QTextEdit>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QDir>
#include <QCoreApplication>

MapboxHandler::MapboxHandler(QObject *parent)
    : QObject(parent)
    , m_mapWidget(nullptr)
    , m_mapPlaceholder(nullptr)
    , m_locationInfo(nullptr)
    , m_openMapBtn(nullptr)
    , m_copyBtn(nullptr)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentLat(0.0)
    , m_currentLng(0.0)
{
    loadEnvironmentVariables();
}

void MapboxHandler::loadEnvironmentVariables()
{
    // Load API key from environment
    bool loadResult = EnvironmentLoader::loadFromFile(".env");
    qDebug() << "MapboxHandler: Environment load result:" << loadResult;
    
    m_apiKey = EnvironmentLoader::getEnv("MAPBOX_API_KEY");
    
    if (m_apiKey.isEmpty()) {
        qWarning() << "MapboxHandler: No API key found in environment. Map functionality will be limited.";
        qDebug() << "MapboxHandler: Checking working directory:" << QDir::currentPath();
        qDebug() << "MapboxHandler: Checking app directory:" << QCoreApplication::applicationDirPath();
    } else {
        qDebug() << "MapboxHandler: Initialized with API key (length:" << m_apiKey.length() << ")";
    }
}

void MapboxHandler::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
    qDebug() << "MapboxHandler: API key updated";
}

void MapboxHandler::displayLocation(double latitude, double longitude)
{
    qDebug() << "MapboxHandler: Display location" << latitude << longitude;
    
    if (m_mapPlaceholder) {
        QString locationText = formatLocationInfo(latitude, longitude);
        m_mapPlaceholder->setText(locationText);
    }
    
    emit locationFound(latitude, longitude);
}

void MapboxHandler::showLocation(double latitude, double longitude, const QString &title)
{
    qDebug() << "MapboxHandler: Show location" << latitude << longitude << "with title:" << title;
    
    m_currentLat = latitude;
    m_currentLng = longitude;
    m_currentTitle = title.isEmpty() ? "Location" : title;
    
    updateMapDisplay();
    emit locationFound(latitude, longitude);
}

void MapboxHandler::addMarker(double latitude, double longitude, const QString &title)
{
    qDebug() << "MapboxHandler: Add marker at" << latitude << longitude << "with title:" << title;
    
    m_currentLat = latitude;
    m_currentLng = longitude;
    m_currentTitle = title.isEmpty() ? "Marker" : title;
    
    updateMapDisplay();
    emit locationFound(latitude, longitude);
}

void MapboxHandler::updateMapDisplay()
{
    if (!m_locationInfo) return;
    
    QString locationText = formatLocationInfo(m_currentLat, m_currentLng, m_currentTitle);
    m_locationInfo->setPlainText(locationText);
    
    // Enable buttons if we have valid coordinates
    bool hasValidCoords = (m_currentLat != 0.0 || m_currentLng != 0.0);
    if (m_openMapBtn) m_openMapBtn->setEnabled(hasValidCoords);
    if (m_copyBtn) m_copyBtn->setEnabled(hasValidCoords);
}

void MapboxHandler::onOpenInBrowser()
{
    if (m_currentLat == 0.0 && m_currentLng == 0.0) return;
    
    QString mapUrl = generateMapUrl(m_currentLat, m_currentLng);
    QDesktopServices::openUrl(QUrl(mapUrl));
}

void MapboxHandler::onCopyCoordinates()
{
    if (m_currentLat == 0.0 && m_currentLng == 0.0) return;
    
    QString coords = QString("%1, %2").arg(m_currentLat, 0, 'f', 6).arg(m_currentLng, 0, 'f', 6);
    QApplication::clipboard()->setText(coords);
    
    // Show a temporary message
    if (m_copyBtn) {
        QString originalText = m_copyBtn->text();
        m_copyBtn->setText("✅ Copied!");
        QTimer::singleShot(2000, [this, originalText]() {
            if (m_copyBtn) m_copyBtn->setText(originalText);
        });
    }
}

QString MapboxHandler::formatLocationInfo(double lat, double lng, const QString &title)
{
    return QString(
        "� %1\n"
        "\n"
        "Coordinates:\n"
        "  Latitude:  %2\n"
        "  Longitude: %3\n"
        "\n"
        "Decimal Degrees: %4, %5\n"
        "\n"
        "Google Maps format: %6,%7\n"
        "What3Words: Available via web interface"
    ).arg(title)
     .arg(lat, 0, 'f', 6)
     .arg(lng, 0, 'f', 6)
     .arg(lat, 0, 'f', 6)
     .arg(lng, 0, 'f', 6)
     .arg(lat, 0, 'f', 6)
     .arg(lng, 0, 'f', 6);
}

QString MapboxHandler::generateMapUrl(double lat, double lng, int zoom)
{
    if (!m_apiKey.isEmpty()) {
        // Generate Mapbox URL
        return QString("https://api.mapbox.com/styles/v1/mapbox/streets-v12/static/pin-s+ff0000(%1,%2)/%1,%2,%3/800x600@2x?access_token=%4")
               .arg(lng, 0, 'f', 6)
               .arg(lat, 0, 'f', 6)
               .arg(zoom)
               .arg(m_apiKey);
    } else {
        // Fallback to OpenStreetMap
        return QString("https://www.openstreetmap.org/?mlat=%1&mlon=%2&zoom=%3")
               .arg(lat, 0, 'f', 6)
               .arg(lng, 0, 'f', 6)               .arg(zoom);
    }
}

void MapboxHandler::geocodeAddress(const QString &address)
{
    qDebug() << "MapboxHandler: Geocoding address:" << address;
    qDebug() << "MapboxHandler: API key available:" << !m_apiKey.isEmpty() << "(length:" << m_apiKey.length() << ")";
    
    if (m_apiKey.isEmpty()) {
        qWarning() << "MapboxHandler: Cannot geocode - no API key";
        emit geocodingFailed("No API key configured");
        return;
    }
    
    qDebug() << "MapboxHandler: Starting geocoding for:" << address;
      // Mapbox Geocoding API endpoint
    QString encodedAddress = QUrl::toPercentEncoding(address);
    QString url = QString("https://api.mapbox.com/geocoding/v5/mapbox.places/%1.json?access_token=%2&limit=1")
                  .arg(encodedAddress)
                  .arg(m_apiKey);
    
    QUrl requestUrl(url);
    QNetworkRequest request;
    request.setUrl(requestUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ArchiFlow/1.0");
      QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply, address]() {
        reply->deleteLater();
        
        qDebug() << "MapboxHandler: Geocoding response received for:" << address;
        qDebug() << "MapboxHandler: HTTP status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "MapboxHandler: Geocoding failed:" << reply->errorString();
            qDebug() << "MapboxHandler: Error code:" << reply->error();
            emit geocodingFailed(reply->errorString());
            return;        }
        
        QByteArray responseData = reply->readAll();
        qDebug() << "MapboxHandler: Response data:" << responseData.left(200) << "..."; // First 200 chars
        
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();
        QJsonArray features = obj["features"].toArray();
        
        qDebug() << "MapboxHandler: Found features:" << features.size();
        
        if (features.isEmpty()) {
            qDebug() << "MapboxHandler: No results found for address:" << address;
            emit geocodingFailed("No results found for address");
            return;
        }
        
        QJsonObject feature = features[0].toObject();
        QJsonArray coordinates = feature["geometry"].toObject()["coordinates"].toArray();
        
        if (coordinates.size() >= 2) {
            double lng = coordinates[0].toDouble();
            double lat = coordinates[1].toDouble();
            qDebug() << "MapboxHandler: Geocoded" << address << "to" << lat << lng;
            emit geocodingCompleted(address, lat, lng);
        } else {
            emit geocodingFailed("Invalid coordinates in response");
        }
    });
}

QWidget* MapboxHandler::createMapWidget()
{
    if (!m_mapWidget) {
        m_mapWidget = new QWidget();
        m_mapWidget->setMinimumSize(500, 400);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(m_mapWidget);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(10);
        
        // Header
        QLabel *headerLabel = new QLabel("🗺️ Interactive Map View");
        headerLabel->setStyleSheet(
            "QLabel { "
            "font-size: 16px; "
            "font-weight: bold; "
            "color: #2c3e50; "
            "padding: 10px; "
            "background-color: #ecf0f1; "
            "border-radius: 8px; "
            "}"
        );
        headerLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(headerLabel);
        
        // Map status group
        QGroupBox *statusGroup = new QGroupBox("Map Status");
        QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
        
        QLabel *statusLabel = new QLabel();
        if (m_apiKey.isEmpty()) {
            statusLabel->setText("⚠️ Configure MAPBOX_API_KEY in .env file for full functionality");
            statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
        } else {
            statusLabel->setText("✅ Mapbox API Key configured - Geocoding available");
            statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
        }
        statusLayout->addWidget(statusLabel);
        mainLayout->addWidget(statusGroup);
        
        // Location info group
        QGroupBox *locationGroup = new QGroupBox("Location Information");
        QVBoxLayout *locationLayout = new QVBoxLayout(locationGroup);
        
        m_locationInfo = new QTextEdit();
        m_locationInfo->setMaximumHeight(150);
        m_locationInfo->setReadOnly(true);
        m_locationInfo->setPlainText("No location selected.\nUse the geocoding feature to find a location.");
        m_locationInfo->setStyleSheet(
            "QTextEdit { "
            "background-color: #f8f9fa; "
            "border: 1px solid #dee2e6; "
            "border-radius: 5px; "
            "padding: 10px; "
            "font-family: monospace; "
            "}"
        );
        locationLayout->addWidget(m_locationInfo);
        
        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        
        m_openMapBtn = new QPushButton("🌐 Open in Browser");
        m_openMapBtn->setEnabled(false);
        m_openMapBtn->setStyleSheet(
            "QPushButton { "
            "background-color: #3498db; "
            "color: white; "
            "border: none; "
            "padding: 10px 20px; "
            "border-radius: 5px; "
            "font-weight: bold; "
            "} "
            "QPushButton:hover { background-color: #2980b9; } "
            "QPushButton:disabled { background-color: #bdc3c7; }"
        );
        connect(m_openMapBtn, &QPushButton::clicked, this, &MapboxHandler::onOpenInBrowser);
        
        m_copyBtn = new QPushButton("📋 Copy Coordinates");
        m_copyBtn->setEnabled(false);
        m_copyBtn->setStyleSheet(
            "QPushButton { "
            "background-color: #2ecc71; "
            "color: white; "
            "border: none; "
            "padding: 10px 20px; "
            "border-radius: 5px; "
            "font-weight: bold; "
            "} "
            "QPushButton:hover { background-color: #27ae60; } "
            "QPushButton:disabled { background-color: #bdc3c7; }"
        );
        connect(m_copyBtn, &QPushButton::clicked, this, &MapboxHandler::onCopyCoordinates);
        
        buttonLayout->addWidget(m_openMapBtn);
        buttonLayout->addWidget(m_copyBtn);
        buttonLayout->addStretch();
        
        locationLayout->addLayout(buttonLayout);
        mainLayout->addWidget(locationGroup);
        
        // Instructions
        QLabel *instructionsLabel = new QLabel(
            "💡 <b>Instructions:</b><br>"
            "• Use the geocoding button in the address section to find locations<br>"
            "• Coordinates will be displayed here once geocoded<br>"
            "• Click 'Open in Browser' to view the location on Mapbox<br>"
            "• Use 'Copy Coordinates' to copy lat/lng to clipboard"
        );
        instructionsLabel->setWordWrap(true);
        instructionsLabel->setStyleSheet(
            "QLabel { "
            "background-color: #e8f4f8; "
            "border: 1px solid #bee5eb; "
            "border-radius: 5px; "
            "padding: 15px; "
            "color: #0c5460; "
            "}"
        );
        mainLayout->addWidget(instructionsLabel);
        
        mainLayout->addStretch();
        m_mapWidget->setLayout(mainLayout);
    }
    
    return m_mapWidget;
}

void MapboxHandler::handleGeocodeResponse()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "MapboxHandler: Geocoding error:" << reply->errorString();
        emit geocodingFailed(reply->errorString());
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    QJsonArray features = obj["features"].toArray();
    if (features.isEmpty()) {
        emit geocodingFailed("No results found for address");
        return;
    }
    
    QJsonObject feature = features[0].toObject();
    QJsonArray coordinates = feature["geometry"].toObject()["coordinates"].toArray();
    
    if (coordinates.size() >= 2) {
        double lng = coordinates[0].toDouble();
        double lat = coordinates[1].toDouble();
        QString placeName = feature["place_name"].toString();
        qDebug() << "MapboxHandler: Geocoded to" << lat << lng << placeName;
        emit geocodingCompleted(placeName, lat, lng);
    } else {
        emit geocodingFailed("Invalid coordinates in response");
    }
}
