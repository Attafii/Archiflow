#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Définir les informations sur l'application
    QApplication::setApplicationName("Gestion des Projets");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("Architecture Office");
    
    // Créer et afficher la fenêtre principale
    MainWindow w;
    w.show();
    
    return a.exec();
}
