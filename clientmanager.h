#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QVector>
#include <QString>
#include "client.h"
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientManager; }
QT_END_NAMESPACE

class MainWindow;

class ClientManager : public QMainWindow
{
    Q_OBJECT

public:
    ClientManager(QWidget *parent = nullptr);
    ~ClientManager();
    void addClient(const Client &client);
    void updateClient(int row, const Client &client);
    Client getClient(int row) const;

private slots:
    void on_addButton_clicked();
    void on_editButton_clicked();
    void on_deleteButton_clicked();
    void on_searchEdit_textChanged(const QString &text);

private:
    Ui::ClientManager *ui;
    MainWindow *mainWindow;
    DatabaseManager *dbManager;
    void setupTable();
    void filterClients(const QString &text);
    void refreshTable();
    void loadClientsFromDatabase();
};

#endif // CLIENTMANAGER_H
