#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "mapboxhandler.h"
#include "envmanager.h"
#include <QMessageBox>
#include <QWebEngineView>
#include <QFormLayout>
#include <QLineEdit>
#include <QTimer>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , clientManager(nullptr)
    , currentEditingRow(-1)
    , isEditMode(false)
    , mapboxHandler(nullptr)
    , mapboxToken(EnvManager::instance().value("MAPBOX_TOKEN", "")) // Get Mapbox token from .env
    , locationUpdateTimer(nullptr)
    , groqaiApi(nullptr)
    , costEstimationEdit(nullptr)
    , estimationDetailsEdit(nullptr)
{
    ui->setupUi(this);
    this->setStyleSheet("font-family: 'Roboto';");

    // Connecter les boutons aux slots
    connect(ui->submitButton, &QPushButton::clicked, this, &MainWindow::onSubmitClicked);
    
    // Add a cancel button to the form layout
    QPushButton *cancelButton = new QPushButton("CANCEL", this);
    cancelButton->setObjectName("cancelButton");
    cancelButton->setStyleSheet("background-color: #cccccc; color: black; border: none; border-radius: 4px; padding: 10px; font-weight: bold;");
    ui->inputLayout->setWidget(9, QFormLayout::FieldRole, cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    
    // Add cost estimation fields and button
    QLabel *costEstimationLabel = new QLabel("Cost Estimation:", this);
    costEstimationEdit = new QLineEdit(this);
    costEstimationEdit->setReadOnly(true);
    costEstimationEdit->setPlaceholderText("Estimated Cost");
    ui->inputLayout->setWidget(10, QFormLayout::LabelRole, costEstimationLabel);
    ui->inputLayout->setWidget(10, QFormLayout::FieldRole, costEstimationEdit);
    
    QLabel *estimationDetailsLabel = new QLabel("Estimation Details:", this);
    estimationDetailsEdit = new QTextEdit(this);
    estimationDetailsEdit->setReadOnly(true);
    estimationDetailsEdit->setPlaceholderText("Estimation Details");
    ui->inputLayout->setWidget(11, QFormLayout::LabelRole, estimationDetailsLabel);
    ui->inputLayout->setWidget(11, QFormLayout::FieldRole, estimationDetailsEdit);
    
    QPushButton *calculateButton = new QPushButton("CALCULATE COST", this);
    calculateButton->setObjectName("calculateButton");
    calculateButton->setStyleSheet("background-color: #4CAF50; color: white; border: none; border-radius: 4px; padding: 10px; font-weight: bold;");
    ui->inputLayout->setWidget(12, QFormLayout::FieldRole, calculateButton);
    connect(calculateButton, &QPushButton::clicked, this, &MainWindow::onCalculateClicked);
    
    // Initialize Groq API client
    groqaiApi = new GroqAIAPI(this);
    // Connect Groq API signals to slots
    connect(groqaiApi, &GroqAIAPI::costEstimationReceived, this, &MainWindow::onCostEstimationReceived);
    connect(groqaiApi, &GroqAIAPI::errorOccurred, this, &MainWindow::onApiErrorOccurred);
    
    // Set Groq API key from .env file
    QString groqApiKey = EnvManager::instance().value("GROQ_API_KEY", "");
    if (groqApiKey.isEmpty()) {
        qDebug() << "Warning: GROQ_API_KEY not found in .env file";
    } else {
        groqaiApi->setApiKey(groqApiKey);
    }
    
    // Connect location and postcode fields to update map
    connect(ui->locationEdit, &QLineEdit::textChanged, this, &MainWindow::onLocationTextChanged);
    connect(ui->postcodeEdit, &QLineEdit::textChanged, this, &MainWindow::onLocationTextChanged);
    
    // Setup timer for delayed map updates
    locationUpdateTimer = new QTimer(this);
    locationUpdateTimer->setSingleShot(true);
    locationUpdateTimer->setInterval(500); // Reduced delay to 500ms for smoother updates
    connect(locationUpdateTimer, &QTimer::timeout, this, &MainWindow::performLocationUpdate);
    
    // Initialize the map
    initializeMap();
    
    // Store the parent as ClientManager if it is one
    if (parent) {
        clientManager = qobject_cast<ClientManager*>(parent);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mapboxHandler;
    delete groqaiApi;
    if (locationUpdateTimer) {
        locationUpdateTimer->stop();
    }
}

void MainWindow::clearDetails()
{
    ui->clientNameEdit->clear();
    ui->projectNameEdit->clear();
    ui->locationEdit->clear();
    ui->postcodeEdit->clear();
    ui->contactPhoneEdit->clear();
    ui->emailEdit->clear();
    ui->serviceAreaEdit->clear();
    ui->descriptionEdit->clear();
    // Clear cost estimation fields
    if (costEstimationEdit) {
        costEstimationEdit->clear();
    }
    if (estimationDetailsEdit) {
        estimationDetailsEdit->clear();
    }
    // Reset coordinates
    if (mapboxHandler) {
        mapboxHandler->setLatitude(0.0);
        mapboxHandler->setLongitude(0.0);
    }
    currentEditingRow = -1;
    isEditMode = false;
}

void MainWindow::loadClientDetails(int row)
{
    if (!clientManager) return;
    Client client = clientManager->getClient(row);
    ui->clientNameEdit->setText(client.name);
    ui->projectNameEdit->setText(client.project);
    ui->locationEdit->setText(client.location);
    ui->postcodeEdit->setText(client.postcode);
    ui->contactPhoneEdit->setText(client.phone);
    ui->emailEdit->setText(client.email);
    ui->serviceAreaEdit->setText(client.serviceArea);
    ui->descriptionEdit->setText(client.description);
    // Load cost estimation fields if available
    if (costEstimationEdit && client.costEstimation > 0) {
        costEstimationEdit->setText(QString::number(client.costEstimation, 'f', 2));
    }
    if (estimationDetailsEdit && !client.estimationDetails.isEmpty()) {
        estimationDetailsEdit->setText(client.estimationDetails);
    }
    // Always set coordinates on the map from saved values
    if (mapboxHandler) {
        mapboxHandler->setLatitude(client.latitude);
        mapboxHandler->setLongitude(client.longitude);
    }
    // Do NOT update map with location field unless coordinates are not set (first time)
    if ((client.latitude == 0.0 && client.longitude == 0.0)) {
        QString searchQuery;
        if (!client.location.isEmpty() && !client.postcode.isEmpty()) {
            searchQuery = client.location + ", " + client.postcode;
        } else if (!client.location.isEmpty()) {
            searchQuery = client.location;
        } else if (!client.postcode.isEmpty()) {
            searchQuery = client.postcode;
        }
        if (!searchQuery.isEmpty()) {
            updateMapLocation(searchQuery);
        }
    }
    currentEditingRow = row;
    isEditMode = true;
}

void MainWindow::onSubmitClicked()
{
    // Récupérer les données du formulaire
    QString clientName = ui->clientNameEdit->text();
    QString projectName = ui->projectNameEdit->text();
    QString location = ui->locationEdit->text();
    QString postcode = ui->postcodeEdit->text();
    QString contactPhone = ui->contactPhoneEdit->text();
    QString email = ui->emailEdit->text();
    QString serviceArea = ui->serviceAreaEdit->text();
    QString description = ui->descriptionEdit->toPlainText();
    // Enhanced validation for all fields
    QString errorMsg;
    if (clientName.trimmed().isEmpty()) {
        errorMsg += "- Client name is required.\n";
    }
    if (projectName.trimmed().isEmpty()) {
        errorMsg += "- Project name is required.\n";
    }
    if (location.trimmed().isEmpty()) {
        errorMsg += "- Location is required.\n";
    }
    if (postcode.trimmed().isEmpty()) {
        errorMsg += "- Postcode is required.\n";
    }
    if (contactPhone.trimmed().isEmpty()) {
        errorMsg += "- Contact phone is required.\n";
    } else {
        QRegularExpression phoneRegex("^\\+?[0-9]*$");
        QRegularExpressionMatch phoneMatch = phoneRegex.match(contactPhone);
        if (!phoneMatch.hasMatch()) {
            errorMsg += "- Invalid phone number format.\n";
        }
    }
    if (email.trimmed().isEmpty()) {
        errorMsg += "- Email is required.\n";
    } else {
        QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
        QRegularExpressionMatch match = emailRegex.match(email);
        if (!match.hasMatch()) {
            errorMsg += "- Invalid email format.\n";
        }
    }
    if (serviceArea.trimmed().isEmpty()) {
        errorMsg += "- Service area is required.\n";
    }
    if (description.trimmed().isEmpty()) {
        errorMsg += "- Description is required.\n";
    }
    // Coordinates validation (optional, but recommended)
    double latitude = mapboxHandler ? mapboxHandler->latitude() : 0.0;
    double longitude = mapboxHandler ? mapboxHandler->longitude() : 0.0;
    if (latitude == 0.0 && longitude == 0.0) {
        errorMsg += "- Please select a valid location on the map.\n";
    }
    if (!errorMsg.isEmpty()) {
        QMessageBox::warning(this, "Input Error", errorMsg);
        return;
    }
    // Create client object
    Client client;
    // If we're in edit mode, preserve the client ID
    if (isEditMode && currentEditingRow >= 0) {
        Client existingClient = clientManager->getClient(currentEditingRow);
        client.id = existingClient.id;
        // Preserve cost estimation if it exists and hasn't been updated
        if (existingClient.costEstimation > 0 && (costEstimationEdit->text().isEmpty() || costEstimationEdit->text().toDouble() == 0)) {
            client.costEstimation = existingClient.costEstimation;
            client.estimationDetails = existingClient.estimationDetails;
        }
    }
    client.name = clientName;
    client.project = projectName;
    client.location = location;
    client.postcode = postcode;
    client.phone = contactPhone;
    client.email = email;
    client.serviceArea = serviceArea;
    client.description = description;
    // Get cost estimation from fields if available
    if (costEstimationEdit && !costEstimationEdit->text().isEmpty()) {
        client.costEstimation = costEstimationEdit->text().toDouble();
    } else {
        // Set default value of 0 if no estimation is provided
        client.costEstimation = 0.0;
    }
    if (estimationDetailsEdit && !estimationDetailsEdit->toPlainText().isEmpty()) {
        client.estimationDetails = estimationDetailsEdit->toPlainText();
    } else {
        // Set empty string if no details are provided
        client.estimationDetails = "";
    }
    // Get coordinates from mapboxHandler
    if (mapboxHandler) {
        client.latitude = mapboxHandler->latitude();
        client.longitude = mapboxHandler->longitude();
    }
    // Add or update client in the manager
    if (clientManager) {
        if (isEditMode && currentEditingRow >= 0) {
            clientManager->updateClient(currentEditingRow, client);
            QMessageBox::information(this, "Client Updated", "Client information has been updated successfully.");
        } else {
            clientManager->addClient(client);
            QMessageBox::information(this, "Client Added", "Client information has been added successfully.");
        }
    }
    clearDetails();
}

void MainWindow::onCancelClicked()
{
    // Return to client manager without saving
    if (clientManager) {
        this->hide();
        clientManager->show();
    } else {
        clearDetails();
    }
}

void MainWindow::initializeMap()
{
    // Create the Mapbox handler
    mapboxHandler = new MapboxHandler(this);
    mapboxHandler->setMapboxToken(mapboxToken);
    
    // Setup the web channel and initialize the map
    mapboxHandler->setupWebChannel(ui->mapView);

    // Connect the signal to update the location field when the address changes from the map
    connect(mapboxHandler, &MapboxHandler::locationFieldShouldUpdate, this, &MainWindow::onMapLocationFieldUpdate);
}

void MainWindow::updateMapLocation(const QString &location)
{
    if (mapboxHandler && !location.isEmpty()) {
        mapboxHandler->updateLocation(location);
    }
}

void MainWindow::onLocationTextChanged(const QString &text)
{
    // Only allow location search if coordinates are not set (first time)
    if (mapboxHandler && (mapboxHandler->latitude() == 0.0 && mapboxHandler->longitude() == 0.0)) {
        locationUpdateTimer->start();
    }
}

void MainWindow::performLocationUpdate()
{
    // Only perform geocoding if coordinates are not set (first time)
    if (mapboxHandler && (mapboxHandler->latitude() == 0.0 && mapboxHandler->longitude() == 0.0)) {
        QString location = ui->locationEdit->text();
        QString postcode = ui->postcodeEdit->text();
        QString searchQuery;
        if (!location.isEmpty() && !postcode.isEmpty()) {
            searchQuery = location + ", " + postcode;
        } else if (!location.isEmpty()) {
            searchQuery = location;
        } else if (!postcode.isEmpty()) {
            searchQuery = postcode;
        }
        if (!searchQuery.isEmpty()) {
            updateMapLocation(searchQuery);
        }
    }
}

// Add this slot implementation at the end of the file
void MainWindow::onMapLocationFieldUpdate(const QString &address)
{
    ui->locationEdit->setText(address);
    // Optionally, update coordinates if needed
    // Coordinates are already updated via mapboxHandler signals
}

void MainWindow::onCalculateClicked()
{
    // Get project details for cost estimation
    QString projectName = ui->projectNameEdit->text();
    QString serviceArea = ui->serviceAreaEdit->text();
    QString description = ui->descriptionEdit->toPlainText();
    QString location = ui->locationEdit->text();
    
    // Validate that we have enough information for estimation
    if (projectName.isEmpty() || serviceArea.isEmpty() || description.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", 
                           "Please provide project name, service area, and description for cost estimation.\n\n"
                           "Note: Cost estimation is optional. You can still save the client form without an estimation.");
        return;
    }
    
    // Show a loading indicator
    costEstimationEdit->setText("Calculating...");
    estimationDetailsEdit->setText("Requesting cost estimation from Groq AI...");
    
    // Request cost estimation from Groq API
    if (groqaiApi) {
        groqaiApi->requestCostEstimation(projectName, serviceArea, description, location);
    } else {
        QMessageBox::critical(this, "API Error", "Groq API client is not initialized.");
        costEstimationEdit->clear();
        estimationDetailsEdit->clear();
    }
}

void MainWindow::onCostEstimationReceived(double cost, const QString &details)
{
    // Update the cost estimation fields
    costEstimationEdit->setText(QString::number(cost, 'f', 2));
    estimationDetailsEdit->setText(details);
    
    // Show a success message
    QMessageBox::information(this, "Groq Cost Estimation", 
                           "Cost estimation from Groq AI completed successfully.");
}

void MainWindow::onApiErrorOccurred(const QString &errorMessage)
{
    // Clear the loading indicator
    costEstimationEdit->clear();
    estimationDetailsEdit->clear();
    
    // Show an error message with a note that estimation is optional
    QMessageBox::critical(this, "Groq API Error", 
                        "Failed to get cost estimation from Groq: " + errorMessage + 
                        "\n\nNote: Cost estimation is optional. You can still save the client form without an estimation.");
}