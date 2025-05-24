#include "mainwindow.h"
#include "database/databasemanager.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Apply dark theme stylesheet
    QFile styleFile(":/utils/style.qss");
    if (styleFile.exists()) {
        styleFile.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&styleFile);
        a.setStyleSheet(stream.readAll());
        styleFile.close();
    } else {
        // Try to load from file system if resource not found
        QFile fsStyleFile(QDir::currentPath() + "/utils/style.qss");
        if (fsStyleFile.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&fsStyleFile);
            a.setStyleSheet(stream.readAll());
            fsStyleFile.close();
        }
    }
    
    // Initialize database
    if (!DatabaseManager::instance().initialize()) {
        QMessageBox::critical(nullptr, "Database Error", "Failed to initialize database. The application will now exit.");
        return -1;
    }
    
    MainWindow w;
    w.show();
    return a.exec();
}
