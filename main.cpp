#include "src/core/application.h"
#include "src/ui/mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    Application app(argc, argv);    // Load and apply stylesheet
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        app.setStyleSheet(styleSheet);
        qDebug() << "Stylesheet loaded successfully from resources";
        qDebug() << "Stylesheet size:" << styleSheet.length() << "characters";
    } else {
        qWarning() << "Failed to load stylesheet from resources";
        
        // Try loading from file system as fallback
        QFile fallbackFile("resources/styles/main.qss");
        if (fallbackFile.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&fallbackFile);
            QString styleSheet = stream.readAll();
            app.setStyleSheet(styleSheet);
            qDebug() << "Stylesheet loaded from file system";
            qDebug() << "Stylesheet size:" << styleSheet.length() << "characters";
        } else {
            qWarning() << "Failed to load stylesheet from file system";
        }
    }
    
    // Create main window first
    MainWindow window;
    
    // Initialize the application after main window is created
    // so it can receive the initialization signal
    if (!app.initialize()) {
        qCritical() << "Failed to initialize ArchiFlow application";
        return -1;
    }
    
    window.show();
    
    return app.exec();
}
