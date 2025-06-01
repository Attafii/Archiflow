#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QWebEngineView>
#include <QWebChannel>
#include <QGeoCoordinate>
#include <QTextEdit>
#include "groqaiapi.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ClientManager;
struct Client;
class MapboxHandler;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void clearDetails();
    void loadClientDetails(int row);

private slots:
    void onSubmitClicked();
    void onCancelClicked();
    void onCalculateClicked();
    void onCostEstimationReceived(double cost, const QString &details);
    void onApiErrorOccurred(const QString &errorMessage);

private:
    Ui::MainWindow *ui;
    ClientManager *clientManager;
    int currentEditingRow;
    bool isEditMode;
    MapboxHandler *mapboxHandler;
    QString mapboxToken;
    QTimer *locationUpdateTimer;
    QLineEdit *clientNameEdit;
    QLineEdit *projectNameEdit;
    QLineEdit *locationEdit;
    QLineEdit *postcodeEdit;
    QLineEdit *contactPhoneEdit;
    QLineEdit *emailEdit;
    QLineEdit *serviceAreaEdit;
    QTextEdit *descriptionEdit;
    GroqAIAPI *groqaiApi;
    QLineEdit *costEstimationEdit;
    QTextEdit *estimationDetailsEdit;
    
    void initializeMap();
    void updateMapLocation(const QString &location);
private slots:
    void onLocationTextChanged(const QString &text);
    void performLocationUpdate();
    void onMapLocationFieldUpdate(const QString &address);
};

#include "clientmanager.h"
#endif // MAINWINDOW_H

void onCancelClicked();
