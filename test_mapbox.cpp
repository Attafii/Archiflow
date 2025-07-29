#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include "src/utils/mapboxhandler.h"
#include "src/utils/environmentloader.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Testing Mapbox integration...";
    
    // Test environment loading
    bool envLoaded = EnvironmentLoader::loadFromFile(".env");
    qDebug() << "Environment loaded:" << envLoaded;
    
    QString apiKey = EnvironmentLoader::getEnv("MAPBOX_API_KEY");
    qDebug() << "API Key length:" << apiKey.length();
    qDebug() << "API Key first 10 chars:" << apiKey.left(10);
    
    // Test MapboxHandler
    MapboxHandler handler;
    QWidget* mapWidget = handler.createMapWidget();
    
    if (mapWidget) {
        qDebug() << "Map widget created successfully";
        mapWidget->show();
        mapWidget->resize(600, 400);
        
        // Test geocoding with a simple address
        QObject::connect(&handler, &MapboxHandler::geocodingCompleted, 
                        [](const QString &address, double lat, double lng) {
            qDebug() << "Geocoding success:" << address << "at" << lat << lng;
            QMessageBox::information(nullptr, "Success", 
                QString("Geocoded: %1\nLat: %2, Lng: %3").arg(address).arg(lat).arg(lng));
        });
        
        QObject::connect(&handler, &MapboxHandler::geocodingFailed, 
                        [](const QString &error) {
            qDebug() << "Geocoding failed:" << error;
            QMessageBox::warning(nullptr, "Error", "Geocoding failed: " + error);
        });
        
        // Test with a simple address
        handler.geocodeAddress("Paris, France");
        
        return app.exec();
    } else {
        qDebug() << "Failed to create map widget";
        QMessageBox::critical(nullptr, "Error", "Failed to create map widget");
        return 1;
    }
}
