#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

struct Client {
    int id = -1; // ID de la base de données, -1 signifie non enregistré
    QString name;
    QString project;
    QString location;
    QString postcode;
    QString phone;
    QString email;
    QString serviceArea;
    QString description;
    double latitude = 0.0;
    double longitude = 0.0;
    double costEstimation = 0.0;
    QString estimationDetails;
};

#endif // CLIENT_H