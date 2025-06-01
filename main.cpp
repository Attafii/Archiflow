#include "clientmanager.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Register SQLite database driver
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        QMessageBox::critical(nullptr, "Database Error", 
                              "SQLite driver is not available. The application may not function correctly.");
    }
    
    // Set the application name and organization for settings
    QApplication::setApplicationName("Client Manager");
    QApplication::setOrganizationName("YourCompany");
    
    // Start the client manager
    ClientManager clientManager;
    clientManager.show();
    
    return a.exec();
}
