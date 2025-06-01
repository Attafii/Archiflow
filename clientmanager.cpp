#include "clientmanager.h"
#include "./ui_clientmanager.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>

ClientManager::ClientManager(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientManager)
    , mainWindow(nullptr)
    , dbManager(nullptr)
{
    ui->setupUi(this);
    setupTable();
    
    // Initialize database manager
    dbManager = new DatabaseManager(this);
    if (!dbManager->openDatabase()) {
        QMessageBox::critical(this, "Database Error", "Failed to open the database. The application may not function correctly.");
    } else {
        // Load clients from database
        loadClientsFromDatabase();
    }
    
    // Connect signals to slots
    connect(ui->addButton, &QPushButton::clicked, this, &ClientManager::on_addButton_clicked);
    connect(ui->editButton, &QPushButton::clicked, this, &ClientManager::on_editButton_clicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &ClientManager::on_deleteButton_clicked);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &ClientManager::on_searchEdit_textChanged);
    // Ne pas fermer la base de données ici, elle doit rester ouverte pendant toute la durée de vie de l'application
}

void ClientManager::setupTable()
{
    // Set column widths
    ui->clientsTable->setColumnWidth(0, 200); // Client Name
    ui->clientsTable->setColumnWidth(1, 200); // Project Name
    ui->clientsTable->setColumnWidth(2, 150); // Location
    ui->clientsTable->setColumnWidth(3, 150); // Contact Phone
    ui->clientsTable->setColumnWidth(4, 200); // Email
    
    // Make the table stretch to fill available space
    ui->clientsTable->horizontalHeader()->setStretchLastSection(true);
    ui->clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ClientManager::addClient(const Client &client)
{
    if (dbManager && dbManager->addClient(client)) {
        refreshTable();
    } else {
        QMessageBox::warning(this, "Database Error", "Failed to add client to the database.");
    }
}

void ClientManager::updateClient(int row, const Client &client)
{
    if (dbManager && dbManager->updateClient(client.id, client)) {
        refreshTable();
    } else {
        QMessageBox::warning(this, "Database Error", "Failed to update client in the database.");
    }
}

Client ClientManager::getClient(int row) const
{
    if (row >= 0 && row < ui->clientsTable->rowCount()) {
        int clientId = ui->clientsTable->item(row, 0)->data(Qt::UserRole).toInt();
        if (dbManager) {
            return dbManager->getClientById(clientId);
        }
    }
    return Client();
}

void ClientManager::refreshTable()
{
    if (!dbManager) return;
    
    QVector<Client> clients = dbManager->getAllClients();
    ui->clientsTable->setRowCount(clients.size());
    
    for (int i = 0; i < clients.size(); ++i) {
        QTableWidgetItem *nameItem = new QTableWidgetItem(clients[i].name);
        nameItem->setData(Qt::UserRole, clients[i].id); // Store client ID as user data
        
        ui->clientsTable->setItem(i, 0, nameItem);
        ui->clientsTable->setItem(i, 1, new QTableWidgetItem(clients[i].project));
        ui->clientsTable->setItem(i, 2, new QTableWidgetItem(clients[i].location));
        ui->clientsTable->setItem(i, 3, new QTableWidgetItem(clients[i].phone));
        ui->clientsTable->setItem(i, 4, new QTableWidgetItem(clients[i].email));
    }
}

void ClientManager::filterClients(const QString &text)
{
    if (!dbManager) return;
    
    if (text.isEmpty()) {
        refreshTable();
        return;
    }
    
    QVector<Client> filteredClients = dbManager->searchClients(text);
    ui->clientsTable->setRowCount(filteredClients.size());
    
    for (int i = 0; i < filteredClients.size(); ++i) {
        const Client &client = filteredClients[i];
        
        QTableWidgetItem *nameItem = new QTableWidgetItem(client.name);
        nameItem->setData(Qt::UserRole, client.id); // Store client ID as user data
        
        ui->clientsTable->setItem(i, 0, nameItem);
        ui->clientsTable->setItem(i, 1, new QTableWidgetItem(client.project));
        ui->clientsTable->setItem(i, 2, new QTableWidgetItem(client.location));
        ui->clientsTable->setItem(i, 3, new QTableWidgetItem(client.phone));
        ui->clientsTable->setItem(i, 4, new QTableWidgetItem(client.email));
    }
}

void ClientManager::on_addButton_clicked()
{
    if (!mainWindow) {
        mainWindow = new MainWindow(this);
    }
    
    // Clear any existing data in the form
    mainWindow->clearDetails();
    
    // Show the form for adding a new client
    mainWindow->show();
    this->hide();
}

void ClientManager::on_editButton_clicked()
{
    int row = ui->clientsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a client to edit.");
        return;
    }
    
    if (!mainWindow) {
        mainWindow = new MainWindow(this);
    }
    
    // Load the selected client's details into the form
    mainWindow->loadClientDetails(row);
    
    // Show the form for editing
    mainWindow->show();
    this->hide();
}

void ClientManager::on_deleteButton_clicked()
{
    int row = ui->clientsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a client to delete.");
        return;
    }
    
    int clientId = ui->clientsTable->item(row, 0)->data(Qt::UserRole).toInt();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Deletion",
                                                            "Are you sure you want to delete this client?",
                                                            QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (dbManager && dbManager->deleteClient(clientId)) {
            refreshTable();
        } else {
            QMessageBox::warning(this, "Database Error", "Failed to delete client from the database.");
        }
    }
}

void ClientManager::on_searchEdit_textChanged(const QString &text)
{
    filterClients(text);
}

void ClientManager::loadClientsFromDatabase()
{
    if (dbManager) {
        // S'assurer que la base de données est ouverte avant de charger les données
        if (!dbManager->openDatabase()) {
            QMessageBox::critical(this, "Erreur de base de données", "Impossible d'ouvrir la base de données pour charger les clients.");
            return;
        }
        refreshTable();
    }
}

ClientManager::~ClientManager()
{
    delete ui;
    delete mainWindow;
    // Le DatabaseManager ferme déjà la connexion dans son propre destructeur
    // Pas besoin de fermer explicitement la base de données ici
    delete dbManager;
}