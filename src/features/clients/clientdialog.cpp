#include "clientdialog.h"
#include "client.h"
#include "clientdatabasemanager.h"
#include "clientaiassistant.h"
#include "../../utils/mapboxhandler.h"
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTimer>

ClientContactDialog::ClientContactDialog(QWidget *parent)
    : QDialog(parent)
    , m_client(nullptr)
    , m_dbManager(nullptr)
    , m_mapHandler(nullptr)
    , m_aiAssistant(nullptr)
    , m_isModified(false)
    , m_isNewClient(true)
{
    setupUI();
    setupConnections();
    setWindowTitle(tr("New Client"));
    resize(500, 600);
}

ClientContactDialog::ClientContactDialog(ClientContact *client, QWidget *parent)
    : QDialog(parent)
    , m_client(client)
    , m_dbManager(nullptr)
    , m_mapHandler(nullptr)
    , m_aiAssistant(nullptr)
    , m_isModified(false)
    , m_isNewClient(false)
{
    setupUI();
    setupConnections();
    populateForm();
    setWindowTitle(tr("Edit Client - %1").arg(client ? client->name() : ""));
    resize(500, 600);
}

ClientContactDialog::~ClientContactDialog()
{
    // Cleanup if needed
}

void ClientContactDialog::setClient(ClientContact *client)
{
    m_client = client;
    m_isNewClient = (client == nullptr);
    populateForm();
    setWindowTitle(tr("Edit Client - %1").arg(client ? client->name() : "New Client"));
}

void ClientContactDialog::setDatabaseManager(ClientDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
}

void ClientContactDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Basic Information Group
    m_basicGroup = new QGroupBox(tr("Basic Information"), this);
    m_basicLayout = new QFormLayout(m_basicGroup);
    
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("Enter client name");
    m_basicLayout->addRow(tr("Name *:"), m_nameEdit);
    
    m_companyEdit = new QLineEdit();
    m_companyEdit->setPlaceholderText("Enter company name");
    m_basicLayout->addRow(tr("Company:"), m_companyEdit);
    
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("Enter email address");
    m_basicLayout->addRow(tr("Email:"), m_emailEdit);
    
    m_phoneEdit = new QLineEdit();
    m_phoneEdit->setPlaceholderText("Enter phone number");
    m_basicLayout->addRow(tr("Phone:"), m_phoneEdit);
    
    m_mainLayout->addWidget(m_basicGroup);
    
    // Address Group
    m_addressGroup = new QGroupBox(tr("Address"), this);
    m_addressLayout = new QFormLayout(m_addressGroup);
    
    m_streetEdit = new QLineEdit();
    m_streetEdit->setPlaceholderText("Enter street address");
    m_addressLayout->addRow(tr("Street:"), m_streetEdit);
    
    m_cityEdit = new QLineEdit();
    m_cityEdit->setPlaceholderText("Enter city");
    m_addressLayout->addRow(tr("City:"), m_cityEdit);
    
    m_stateEdit = new QLineEdit();
    m_stateEdit->setPlaceholderText("Enter state/province");
    m_addressLayout->addRow(tr("State/Province:"), m_stateEdit);
    
    m_zipcodeEdit = new QLineEdit();
    m_zipcodeEdit->setPlaceholderText("Enter postal/zip code");
    m_addressLayout->addRow(tr("Postal/Zip Code:"), m_zipcodeEdit);
    
    m_countryEdit = new QLineEdit();
    m_countryEdit->setPlaceholderText("Enter country");
    m_addressLayout->addRow(tr("Country:"), m_countryEdit);
    
    m_geocodeBtn = new QPushButton(tr("Get Coordinates"));
    m_geocodeBtn->setToolTip("Automatically get latitude/longitude from address");
    QHBoxLayout *geocodeLayout = new QHBoxLayout();
    geocodeLayout->addStretch();
    geocodeLayout->addWidget(m_geocodeBtn);
    m_addressLayout->addRow("", geocodeLayout);
    
    m_mainLayout->addWidget(m_addressGroup);
    
    // Coordinates Group
    m_coordinatesGroup = new QGroupBox(tr("Location Coordinates"), this);
    m_coordinatesLayout = new QFormLayout(m_coordinatesGroup);
    
    m_latitudeSpinBox = new QDoubleSpinBox();
    m_latitudeSpinBox->setRange(-90.0, 90.0);
    m_latitudeSpinBox->setDecimals(6);
    m_latitudeSpinBox->setSingleStep(0.000001);
    m_latitudeSpinBox->setToolTip("Latitude (-90 to 90)");
    m_coordinatesLayout->addRow(tr("Latitude:"), m_latitudeSpinBox);
    
    m_longitudeSpinBox = new QDoubleSpinBox();
    m_longitudeSpinBox->setRange(-180.0, 180.0);
    m_longitudeSpinBox->setDecimals(6);
    m_longitudeSpinBox->setSingleStep(0.000001);
    m_longitudeSpinBox->setToolTip("Longitude (-180 to 180)");
    m_coordinatesLayout->addRow(tr("Longitude:"), m_longitudeSpinBox);
    
    m_clearCoordsBtn = new QPushButton(tr("Clear Coordinates"));
    QHBoxLayout *clearCoordsLayout = new QHBoxLayout();
    clearCoordsLayout->addStretch();
    clearCoordsLayout->addWidget(m_clearCoordsBtn);
    m_coordinatesLayout->addRow("", clearCoordsLayout);
    
    m_mainLayout->addWidget(m_coordinatesGroup);
    
    // Notes Group
    m_notesGroup = new QGroupBox(tr("Notes"), this);
    m_notesLayout = new QVBoxLayout(m_notesGroup);
    
    m_notesEdit = new QTextEdit();
    m_notesEdit->setPlaceholderText("Enter any additional notes about this client...");
    m_notesEdit->setMaximumHeight(100);
    m_notesLayout->addWidget(m_notesEdit);
    
    m_mainLayout->addWidget(m_notesGroup);
    
    // Timestamps Group (only shown for existing clients)
    m_timestampsGroup = new QGroupBox(tr("Timestamps"), this);
    m_timestampsLayout = new QFormLayout(m_timestampsGroup);
    
    m_createdAtLabel = new QLabel();
    m_timestampsLayout->addRow(tr("Created:"), m_createdAtLabel);
    
    m_updatedAtLabel = new QLabel();
    m_timestampsLayout->addRow(tr("Last Updated:"), m_updatedAtLabel);
    
    m_timestampsGroup->setVisible(false); // Initially hidden
    m_mainLayout->addWidget(m_timestampsGroup);
      // Buttons
    m_buttonLayout = new QHBoxLayout();
    
    m_aiAssistantBtn = new QPushButton(tr("🤖 AI Assistant"));
    m_aiAssistantBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #9b59b6; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { background-color: #8e44ad; }"
    );
    
    m_saveBtn = new QPushButton(tr("Save"));
    m_saveBtn->setDefault(true);
    
    m_cancelBtn = new QPushButton(tr("Cancel"));
    
    m_buttonLayout->addWidget(m_aiAssistantBtn);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_saveBtn);
    m_buttonLayout->addWidget(m_cancelBtn);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Set focus to name field
    m_nameEdit->setFocus();
    
    // Apply styling
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QLineEdit, QTextEdit, QDoubleSpinBox {
            border: 1px solid #cccccc;
            border-radius: 3px;
            padding: 5px;
        }
        QLineEdit:focus, QTextEdit:focus, QDoubleSpinBox:focus {
            border: 2px solid #4a90e2;
        }
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton:pressed {
            background-color: #2968a3;
        }
    )");
}

void ClientContactDialog::setupConnections()
{
    // Field change monitoring
    connect(m_nameEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_companyEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_emailEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_phoneEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_streetEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_cityEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_stateEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_zipcodeEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_countryEdit, &QLineEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    connect(m_latitudeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ClientContactDialog::onFieldChanged);
    connect(m_longitudeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ClientContactDialog::onFieldChanged);
    connect(m_notesEdit, &QTextEdit::textChanged, this, &ClientContactDialog::onFieldChanged);
    
    // Special actions
    connect(m_emailEdit, &QLineEdit::editingFinished, this, &ClientContactDialog::onValidateEmail);
    connect(m_geocodeBtn, &QPushButton::clicked, this, &ClientContactDialog::onGeocodeAddress);
    connect(m_clearCoordsBtn, &QPushButton::clicked, this, &ClientContactDialog::onClearCoordinates);
      // Buttons
    connect(m_saveBtn, &QPushButton::clicked, this, &ClientContactDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ClientContactDialog::reject);
    connect(m_aiAssistantBtn, &QPushButton::clicked, this, &ClientContactDialog::onOpenAIAssistant);
}

void ClientContactDialog::populateForm()
{
    if (!m_client) {
        clearForm();
        return;
    }
    
    // Basic information
    m_nameEdit->setText(m_client->name());
    m_companyEdit->setText(m_client->companyName());
    m_emailEdit->setText(m_client->email());
    m_phoneEdit->setText(m_client->phoneNumber());
    
    // Address
    m_streetEdit->setText(m_client->addressStreet());
    m_cityEdit->setText(m_client->addressCity());
    m_stateEdit->setText(m_client->addressState());
    m_zipcodeEdit->setText(m_client->addressZipcode());
    m_countryEdit->setText(m_client->addressCountry());
    
    // Coordinates
    m_latitudeSpinBox->setValue(m_client->latitude());
    m_longitudeSpinBox->setValue(m_client->longitude());
    
    // Notes
    m_notesEdit->setPlainText(m_client->notes());
    
    // Timestamps (for existing clients)
    if (!m_isNewClient) {
        m_createdAtLabel->setText(m_client->createdAt().toString("yyyy-MM-dd hh:mm:ss"));
        m_updatedAtLabel->setText(m_client->updatedAt().toString("yyyy-MM-dd hh:mm:ss"));
        m_timestampsGroup->setVisible(true);
    }
    
    m_isModified = false;
}

void ClientContactDialog::clearForm()
{
    m_nameEdit->clear();
    m_companyEdit->clear();
    m_emailEdit->clear();
    m_phoneEdit->clear();
    m_streetEdit->clear();
    m_cityEdit->clear();
    m_stateEdit->clear();
    m_zipcodeEdit->clear();
    m_countryEdit->clear();
    m_latitudeSpinBox->setValue(0.0);
    m_longitudeSpinBox->setValue(0.0);
    m_notesEdit->clear();
    
    m_timestampsGroup->setVisible(false);
    m_isModified = false;
}

bool ClientContactDialog::validateInput()
{
    // Name is required
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Client name is required."));
        m_nameEdit->setFocus();
        return false;
    }
    
    // Email validation (if provided)
    QString email = m_emailEdit->text().trimmed();
    if (!email.isEmpty()) {
        QRegularExpression emailRegex("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
        if (!emailRegex.match(email).hasMatch()) {
            QMessageBox::warning(this, tr("Validation Error"), tr("Please enter a valid email address."));
            m_emailEdit->setFocus();
            return false;
        }
        
        // Check for duplicate email
        if (m_dbManager && m_dbManager->emailExists(email, m_client ? m_client->id() : QString())) {
            QMessageBox::warning(this, tr("Validation Error"), tr("A client with this email address already exists."));
            m_emailEdit->setFocus();
            return false;
        }
    }
    
    // Coordinate validation
    double lat = m_latitudeSpinBox->value();
    double lng = m_longitudeSpinBox->value();
    
    if (lat < -90.0 || lat > 90.0) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Latitude must be between -90 and 90 degrees."));
        m_latitudeSpinBox->setFocus();
        return false;
    }
    
    if (lng < -180.0 || lng > 180.0) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Longitude must be between -180 and 180 degrees."));
        m_longitudeSpinBox->setFocus();
        return false;
    }
    
    return true;
}

void ClientContactDialog::applyFormData()
{
    if (!m_client) {
        m_client = new ClientContact(this); // Ensure ClientContact can be parented or handle ownership
    }
    
    // Basic information
    m_client->setName(m_nameEdit->text().trimmed());
    m_client->setCompanyName(m_companyEdit->text().trimmed());
    m_client->setEmail(m_emailEdit->text().trimmed());
    m_client->setPhoneNumber(m_phoneEdit->text().trimmed());
    
    // Address
    m_client->setAddressStreet(m_streetEdit->text().trimmed());
    m_client->setAddressCity(m_cityEdit->text().trimmed());
    m_client->setAddressState(m_stateEdit->text().trimmed());
    m_client->setAddressZipcode(m_zipcodeEdit->text().trimmed());
    m_client->setAddressCountry(m_countryEdit->text().trimmed());
    
    // Coordinates
    m_client->setLatitude(m_latitudeSpinBox->value());
    m_client->setLongitude(m_longitudeSpinBox->value());
    
    // Notes
    m_client->setNotes(m_notesEdit->toPlainText().trimmed());
    
    // Update timestamp for existing clients
    if (!m_isNewClient) {
        m_client->setUpdatedAt(QDateTime::currentDateTime());
    }
}

void ClientContactDialog::markAsModified()
{
    m_isModified = true;
}

void ClientContactDialog::accept()
{
    if (!validateInput()) return;
    
    applyFormData();
    
    QDialog::accept();
}

void ClientContactDialog::reject()
{
    if (m_isModified) {
        int ret = QMessageBox::question(this, tr("Unsaved Changes"), 
                                       tr("You have unsaved changes. Do you want to save them?"),
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            accept();
            return;
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    QDialog::reject();
}

void ClientContactDialog::closeEvent(QCloseEvent *event)
{
    if (m_isModified) {
        int ret = QMessageBox::question(this, tr("Unsaved Changes"), 
                                       tr("You have unsaved changes. Do you want to save them?"),
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            if (validateInput()) {
                accept();
                event->accept();
            } else {
                event->ignore();
            }
        } else if (ret == QMessageBox::Cancel) {
            event->ignore();
        } else {
            event->accept();
        }
    } else {
        event->accept();
    }
}

// Slot implementations
void ClientContactDialog::onFieldChanged()
{
    markAsModified();
}

void ClientContactDialog::onGeocodeAddress()
{
    QString address = QString("%1, %2, %3, %4, %5")
                     .arg(m_streetEdit->text().trimmed())
                     .arg(m_cityEdit->text().trimmed())
                     .arg(m_stateEdit->text().trimmed())
                     .arg(m_zipcodeEdit->text().trimmed())
                     .arg(m_countryEdit->text().trimmed());
    
    address = address.remove(QRegularExpression("^,\\s*|,\\s*$|,\\s*,"));
    
    if (address.trimmed().isEmpty()) {
        QMessageBox::information(this, tr("Geocoding"), tr("Please enter an address first."));
        return;
    }
    
    // Use MapboxHandler for actual geocoding
    if (!m_mapHandler) {
        m_mapHandler = new MapboxHandler(this);
    }
    
    // Connect to geocoding results
    connect(m_mapHandler, &MapboxHandler::geocodingCompleted, this, 
            [this](const QString &addr, double lat, double lng) {
                Q_UNUSED(addr)
                m_latitudeSpinBox->setValue(lat);
                m_longitudeSpinBox->setValue(lng);
                markAsModified();
                QMessageBox::information(this, tr("Geocoding Successful"), 
                                       tr("Address geocoded successfully!\nLatitude: %1\nLongitude: %2")
                                       .arg(lat, 0, 'f', 6).arg(lng, 0, 'f', 6));
            });
    
    connect(m_mapHandler, &MapboxHandler::geocodingFailed, this,
            [this](const QString &error) {
                QMessageBox::warning(this, tr("Geocoding Failed"), 
                                    tr("Failed to geocode address:\n%1").arg(error));
            });
    
    // Disable button during geocoding
    m_geocodeBtn->setEnabled(false);
    m_geocodeBtn->setText(tr("Geocoding..."));
    
    // Start geocoding
    m_mapHandler->geocodeAddress(address);
    
    // Re-enable button after a delay
    QTimer::singleShot(5000, [this]() {
        m_geocodeBtn->setEnabled(true);
        m_geocodeBtn->setText(tr("Get Coordinates"));
    });
}

void ClientContactDialog::onValidateEmail()
{
    QString email = m_emailEdit->text().trimmed();
    if (email.isEmpty()) return;
    
    QRegularExpression emailRegex("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        m_emailEdit->setStyleSheet("border: 2px solid red;");
        m_emailEdit->setToolTip("Invalid email format");
    } else if (m_dbManager && m_dbManager->emailExists(email, m_client ? m_client->id() : QString())) {
        m_emailEdit->setStyleSheet("border: 2px solid orange;");
        m_emailEdit->setToolTip("Email already exists for another client");
    } else {
        m_emailEdit->setStyleSheet("");
        m_emailEdit->setToolTip("");
    }
}

void ClientContactDialog::onClearCoordinates()
{
    m_latitudeSpinBox->setValue(0.0);
    m_longitudeSpinBox->setValue(0.0);
    markAsModified();
}

void ClientContactDialog::onOpenAIAssistant()
{
    if (!m_aiAssistant) {
        m_aiAssistant = new ClientAIAssistant(m_client, this);
    } else {
        m_aiAssistant->setClient(m_client);
    }
    
    m_aiAssistant->show();
    m_aiAssistant->raise();
    m_aiAssistant->activateWindow();
}
