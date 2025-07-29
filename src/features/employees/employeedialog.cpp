#include "employeedialog.h"
#include "employee.h"
#include "employeedatabasemanager.h"
#include "../../utils/mapboxhandler.h"
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDateTime>

EmployeeDialog::EmployeeDialog(QWidget *parent)
    : QDialog(parent)
    , m_employee(nullptr)
    , m_dbManager(nullptr)
    , m_isModified(false)
    , m_isNewEmployee(true)
    , m_mapHandler(nullptr)
{
    setupUI();
    setupConnections();
    setWindowTitle(tr("New Employee"));
    resize(600, 700);
}

EmployeeDialog::EmployeeDialog(Employee *employee, QWidget *parent)
    : QDialog(parent)
    , m_employee(employee)
    , m_dbManager(nullptr)
    , m_isModified(false)
    , m_isNewEmployee(false)
    , m_mapHandler(nullptr)
{
    setupUI();
    setupConnections();
    populateForm();
    setWindowTitle(tr("Edit Employee - %1").arg(employee ? employee->fullName() : ""));
    resize(600, 700);
}

EmployeeDialog::~EmployeeDialog()
{
    // Cleanup if needed
}

void EmployeeDialog::setEmployee(Employee *employee)
{
    m_employee = employee;
    m_isNewEmployee = (employee == nullptr);
    populateForm();
    setWindowTitle(tr("Edit Employee - %1").arg(employee ? employee->fullName() : "New Employee"));
}

void EmployeeDialog::setDatabaseManager(EmployeeDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
}

void EmployeeDialog::setReadOnly(bool readOnly)
{
    // Disable all input widgets for read-only mode
    m_cinEdit->setReadOnly(readOnly);
    m_generateCinBtn->setEnabled(!readOnly);
    m_firstNameEdit->setReadOnly(readOnly);
    m_lastNameEdit->setReadOnly(readOnly);
    m_emailEdit->setReadOnly(readOnly);
    m_phoneEdit->setReadOnly(readOnly);
    
    // Employment tab
    m_positionEdit->setReadOnly(readOnly);
    m_roleCombo->setEnabled(!readOnly);
    m_departmentEdit->setReadOnly(readOnly);
    m_hireDateEdit->setReadOnly(readOnly);
    m_statusCombo->setEnabled(!readOnly);
    m_isPresentCheck->setEnabled(!readOnly);
    m_salarySpinBox->setReadOnly(readOnly);
    
    // Contact tab
    m_addressEdit->setReadOnly(readOnly);
    m_geocodeBtn->setEnabled(!readOnly);
    m_showMapBtn->setEnabled(!readOnly && m_showMapBtn->isEnabled()); // Keep geocode state
    m_emergencyContactEdit->setReadOnly(readOnly);
    m_emergencyPhoneEdit->setReadOnly(readOnly);
    
    // Additional tab
    m_notesEdit->setReadOnly(readOnly);
    
    // Buttons
    m_saveBtn->setVisible(!readOnly);
    m_aiAssistantBtn->setEnabled(!readOnly);
    
    if (readOnly) {
        m_cancelBtn->setText(tr("Close"));
        setWindowTitle(windowTitle().replace("Edit", "View"));
    }
}

void EmployeeDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget();
    
    setupBasicInfoTab();
    setupEmploymentTab();
    setupContactTab();
    setupAdditionalTab();
    
    m_mainLayout->addWidget(m_tabWidget);
    
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
    
    // Apply styling
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QLineEdit, QTextEdit, QComboBox, QDoubleSpinBox, QDateTimeEdit {
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 8px;
            font-size: 13px;
        }
        QLineEdit:focus, QTextEdit:focus, QComboBox:focus, 
        QDoubleSpinBox:focus, QDateTimeEdit:focus {
            border-color: #4a90e2;
            outline: none;
        }
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton:pressed {
            background-color: #2a5d8f;
        }
        QTabWidget::pane {
            border: 1px solid #cccccc;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: #f0f0f0;
            border: 1px solid #cccccc;
            padding: 8px 16px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #4a90e2;
            color: white;
        }
    )");
}

void EmployeeDialog::setupBasicInfoTab()
{
    m_basicInfoTab = new QWidget();
    m_basicInfoLayout = new QFormLayout(m_basicInfoTab);
    
    // CIN with generate button
    QHBoxLayout *cinLayout = new QHBoxLayout();
    m_cinEdit = new QLineEdit();
    m_cinEdit->setPlaceholderText("Enter CIN or generate");
    m_generateCinBtn = new QPushButton("Generate");
    m_generateCinBtn->setMaximumWidth(80);
    cinLayout->addWidget(m_cinEdit);
    cinLayout->addWidget(m_generateCinBtn);
    m_basicInfoLayout->addRow(tr("CIN *:"), cinLayout);
    
    m_firstNameEdit = new QLineEdit();
    m_firstNameEdit->setPlaceholderText("Enter first name");
    m_basicInfoLayout->addRow(tr("First Name *:"), m_firstNameEdit);
    
    m_lastNameEdit = new QLineEdit();
    m_lastNameEdit->setPlaceholderText("Enter last name");
    m_basicInfoLayout->addRow(tr("Last Name *:"), m_lastNameEdit);
    
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("Enter email address");
    m_basicInfoLayout->addRow(tr("Email:"), m_emailEdit);
    
    m_phoneEdit = new QLineEdit();
    m_phoneEdit->setPlaceholderText("Enter phone number");
    m_basicInfoLayout->addRow(tr("Phone:"), m_phoneEdit);
    
    m_tabWidget->addTab(m_basicInfoTab, "👤 Basic Info");
}

void EmployeeDialog::setupEmploymentTab()
{
    m_employmentTab = new QWidget();
    m_employmentLayout = new QFormLayout(m_employmentTab);
    
    m_positionEdit = new QLineEdit();
    m_positionEdit->setPlaceholderText("Enter position/job title");
    m_employmentLayout->addRow(tr("Position:"), m_positionEdit);
    
    m_roleCombo = new QComboBox();
    m_roleCombo->addItem("Architect", static_cast<int>(Employee::Architect));
    m_roleCombo->addItem("Engineer", static_cast<int>(Employee::Engineer));
    m_roleCombo->addItem("Project Manager", static_cast<int>(Employee::ProjectManager));
    m_roleCombo->addItem("Designer", static_cast<int>(Employee::Designer));
    m_roleCombo->addItem("Contractor", static_cast<int>(Employee::Contractor));
    m_roleCombo->addItem("Administrator", static_cast<int>(Employee::Administrator));
    m_roleCombo->addItem("Other", static_cast<int>(Employee::Other));
    m_employmentLayout->addRow(tr("Role:"), m_roleCombo);
    
    m_departmentEdit = new QLineEdit();
    m_departmentEdit->setPlaceholderText("Enter department");
    m_employmentLayout->addRow(tr("Department:"), m_departmentEdit);
    
    m_hireDateEdit = new QDateTimeEdit();
    m_hireDateEdit->setDateTime(QDateTime::currentDateTime());
    m_hireDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_hireDateEdit->setCalendarPopup(true);
    m_employmentLayout->addRow(tr("Hire Date:"), m_hireDateEdit);
    
    m_statusCombo = new QComboBox();
    m_statusCombo->addItem("Active", static_cast<int>(Employee::Active));
    m_statusCombo->addItem("Inactive", static_cast<int>(Employee::Inactive));
    m_statusCombo->addItem("On Leave", static_cast<int>(Employee::OnLeave));
    m_statusCombo->addItem("Terminated", static_cast<int>(Employee::Terminated));
    m_employmentLayout->addRow(tr("Status:"), m_statusCombo);
    
    m_isPresentCheck = new QCheckBox("Currently present");
    m_employmentLayout->addRow(tr("Presence:"), m_isPresentCheck);
      m_salarySpinBox = new QDoubleSpinBox();
    m_salarySpinBox->setRange(0, 999999.99);
    m_salarySpinBox->setDecimals(2);
    m_salarySpinBox->setSuffix(" €");
    m_employmentLayout->addRow(tr("Salary:"), m_salarySpinBox);
    
    m_tabWidget->addTab(m_employmentTab, "💼 Employment");
}

void EmployeeDialog::setupContactTab()
{
    m_contactTab = new QWidget();
    m_contactLayout = new QFormLayout(m_contactTab);
    
    // Address section with map integration
    QVBoxLayout *addressLayout = new QVBoxLayout();
    m_addressEdit = new QTextEdit();
    m_addressEdit->setMaximumHeight(100);
    m_addressEdit->setPlaceholderText("Enter full address");
    addressLayout->addWidget(m_addressEdit);
    
    // Map buttons
    QHBoxLayout *mapButtonLayout = new QHBoxLayout();
    m_geocodeBtn = new QPushButton("🌍 Geocode Address");
    m_geocodeBtn->setToolTip("Get coordinates for this address");
    m_showMapBtn = new QPushButton("🗺️ Show on Map");
    m_showMapBtn->setToolTip("Show location on map");
    m_showMapBtn->setEnabled(false); // Enable after geocoding
    
    mapButtonLayout->addWidget(m_geocodeBtn);
    mapButtonLayout->addWidget(m_showMapBtn);
    mapButtonLayout->addStretch();
    addressLayout->addLayout(mapButtonLayout);
    
    m_contactLayout->addRow(tr("Address:"), addressLayout);
    
    m_emergencyContactEdit = new QLineEdit();
    m_emergencyContactEdit->setPlaceholderText("Emergency contact name");
    m_contactLayout->addRow(tr("Emergency Contact:"), m_emergencyContactEdit);
    
    m_emergencyPhoneEdit = new QLineEdit();
    m_emergencyPhoneEdit->setPlaceholderText("Emergency contact phone");
    m_contactLayout->addRow(tr("Emergency Phone:"), m_emergencyPhoneEdit);
    
    m_tabWidget->addTab(m_contactTab, "📞 Contact");
}

void EmployeeDialog::setupAdditionalTab()
{
    m_additionalTab = new QWidget();
    m_additionalLayout = new QVBoxLayout(m_additionalTab);
    
    // Notes section
    QLabel *notesLabel = new QLabel("Notes:");
    notesLabel->setStyleSheet("font-weight: bold; margin-bottom: 5px;");
    m_additionalLayout->addWidget(notesLabel);
    
    m_notesEdit = new QTextEdit();
    m_notesEdit->setPlaceholderText("Additional notes about the employee...");
    m_additionalLayout->addWidget(m_notesEdit);
    
    // Timestamps (read-only)
    m_timestampsGroup = new QGroupBox(tr("Record Information"));
    m_timestampsLayout = new QFormLayout(m_timestampsGroup);
    
    m_createdAtLabel = new QLabel();
    m_createdAtLabel->setStyleSheet("color: #666; font-family: monospace;");
    m_timestampsLayout->addRow(tr("Created:"), m_createdAtLabel);
    
    m_updatedAtLabel = new QLabel();
    m_updatedAtLabel->setStyleSheet("color: #666; font-family: monospace;");
    m_timestampsLayout->addRow(tr("Updated:"), m_updatedAtLabel);
    
    m_additionalLayout->addWidget(m_timestampsGroup);
    m_additionalLayout->addStretch();
    
    m_tabWidget->addTab(m_additionalTab, "📝 Additional");
}

void EmployeeDialog::setupConnections()
{
    // Basic info fields
    connect(m_cinEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_firstNameEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_lastNameEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_emailEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_phoneEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    
    // Employment fields
    connect(m_positionEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_roleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &EmployeeDialog::onRoleChanged);
    connect(m_departmentEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_hireDateEdit, &QDateTimeEdit::dateTimeChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &EmployeeDialog::onStatusChanged);
    connect(m_isPresentCheck, &QCheckBox::toggled, this, &EmployeeDialog::onFieldChanged);
    connect(m_salarySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &EmployeeDialog::onFieldChanged);
    
    // Contact fields
    connect(m_addressEdit, &QTextEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_emergencyContactEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_emergencyPhoneEdit, &QLineEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    connect(m_notesEdit, &QTextEdit::textChanged, this, &EmployeeDialog::onFieldChanged);
    
    // Special actions
    connect(m_generateCinBtn, &QPushButton::clicked, this, &EmployeeDialog::onGenerateCin);
    connect(m_emailEdit, &QLineEdit::editingFinished, this, &EmployeeDialog::onValidateEmail);
    
    // Map functionality
    connect(m_geocodeBtn, &QPushButton::clicked, this, &EmployeeDialog::onGeocodeAddress);
    connect(m_showMapBtn, &QPushButton::clicked, this, &EmployeeDialog::onShowMap);
    
    // Buttons
    connect(m_saveBtn, &QPushButton::clicked, this, &EmployeeDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &EmployeeDialog::reject);
    connect(m_aiAssistantBtn, &QPushButton::clicked, this, &EmployeeDialog::onOpenAIAssistant);
}

void EmployeeDialog::populateForm()
{
    if (!m_employee) {
        clearForm();
        return;
    }
    
    // Basic info
    m_cinEdit->setText(m_employee->cin());
    m_firstNameEdit->setText(m_employee->firstName());
    m_lastNameEdit->setText(m_employee->lastName());
    m_emailEdit->setText(m_employee->email());
    m_phoneEdit->setText(m_employee->phoneNumber());
    
    // Employment
    m_positionEdit->setText(m_employee->position());
    m_roleCombo->setCurrentIndex(m_roleCombo->findData(static_cast<int>(m_employee->role())));
    m_departmentEdit->setText(m_employee->department());
    if (m_employee->hireDate().isValid()) {
        m_hireDateEdit->setDateTime(m_employee->hireDate());
    }
    m_statusCombo->setCurrentIndex(m_statusCombo->findData(static_cast<int>(m_employee->status())));
    m_isPresentCheck->setChecked(m_employee->isPresent());
    m_salarySpinBox->setValue(m_employee->salary());
    
    // Contact
    m_addressEdit->setPlainText(m_employee->address());
    m_emergencyContactEdit->setText(m_employee->emergencyContact());
    m_emergencyPhoneEdit->setText(m_employee->emergencyPhone());
    
    // Additional
    m_notesEdit->setPlainText(m_employee->notes());
    
    // Timestamps
    if (m_employee->createdAt().isValid()) {
        m_createdAtLabel->setText(m_employee->createdAt().toString("yyyy-MM-dd hh:mm:ss"));
    }
    if (m_employee->updatedAt().isValid()) {
        m_updatedAtLabel->setText(m_employee->updatedAt().toString("yyyy-MM-dd hh:mm:ss"));
    }
    
    m_isModified = false;
}

void EmployeeDialog::clearForm()
{
    // Basic info
    m_cinEdit->clear();
    m_firstNameEdit->clear();
    m_lastNameEdit->clear();
    m_emailEdit->clear();
    m_phoneEdit->clear();
    
    // Employment
    m_positionEdit->clear();
    m_roleCombo->setCurrentIndex(m_roleCombo->findData(static_cast<int>(Employee::Other)));
    m_departmentEdit->clear();
    m_hireDateEdit->setDateTime(QDateTime::currentDateTime());
    m_statusCombo->setCurrentIndex(m_statusCombo->findData(static_cast<int>(Employee::Active)));
    m_isPresentCheck->setChecked(false);
    m_salarySpinBox->setValue(0.0);
    
    // Contact
    m_addressEdit->clear();
    m_emergencyContactEdit->clear();
    m_emergencyPhoneEdit->clear();
    
    // Additional
    m_notesEdit->clear();
    m_createdAtLabel->clear();
    m_updatedAtLabel->clear();
    
    m_isModified = false;
}

bool EmployeeDialog::validateInput()
{
    QStringList errors;
    
    if (m_cinEdit->text().trimmed().isEmpty()) {
        errors << tr("CIN is required");
    }
    
    if (m_firstNameEdit->text().trimmed().isEmpty()) {
        errors << tr("First name is required");
    }
    
    if (m_lastNameEdit->text().trimmed().isEmpty()) {
        errors << tr("Last name is required");
    }
    
    QString email = m_emailEdit->text().trimmed();
    if (!email.isEmpty()) {
        QRegularExpression emailRegex("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
        if (!emailRegex.match(email).hasMatch()) {
            errors << tr("Invalid email format");
        }
    }
    
    if (m_salarySpinBox->value() < 0) {
        errors << tr("Salary cannot be negative");
    }
    
    // Check for duplicate CIN if this is a new employee or CIN changed
    if (m_dbManager) {
        QString cin = m_cinEdit->text().trimmed();
        if (m_isNewEmployee || (m_employee && m_employee->cin() != cin)) {
            if (m_dbManager->employeeExists(cin)) {
                errors << tr("An employee with this CIN already exists");
            }
        }
        
        // Check for duplicate email
        if (!email.isEmpty()) {
            QString excludeCin = m_employee ? m_employee->cin() : QString();
            if (m_dbManager->emailExists(email, excludeCin)) {
                errors << tr("An employee with this email already exists");
            }
        }
    }
    
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), 
                           tr("Please correct the following errors:\n\n%1").arg(errors.join("\n")));
        return false;
    }
    
    return true;
}

void EmployeeDialog::applyFormData()
{
    if (!m_employee) {
        m_employee = new Employee(this);
        m_isNewEmployee = true;
    }
    
    // Basic info
    m_employee->setCin(m_cinEdit->text().trimmed());
    m_employee->setFirstName(m_firstNameEdit->text().trimmed());
    m_employee->setLastName(m_lastNameEdit->text().trimmed());
    m_employee->setEmail(m_emailEdit->text().trimmed());
    m_employee->setPhoneNumber(m_phoneEdit->text().trimmed());
    
    // Employment
    m_employee->setPosition(m_positionEdit->text().trimmed());
    m_employee->setRole(static_cast<Employee::EmployeeRole>(m_roleCombo->currentData().toInt()));
    m_employee->setDepartment(m_departmentEdit->text().trimmed());
    m_employee->setHireDate(m_hireDateEdit->dateTime());
    m_employee->setStatus(static_cast<Employee::EmployeeStatus>(m_statusCombo->currentData().toInt()));
    m_employee->setIsPresent(m_isPresentCheck->isChecked());
    m_employee->setSalary(m_salarySpinBox->value());
    
    // Contact
    m_employee->setAddress(m_addressEdit->toPlainText().trimmed());
    m_employee->setEmergencyContact(m_emergencyContactEdit->text().trimmed());
    m_employee->setEmergencyPhone(m_emergencyPhoneEdit->text().trimmed());
    
    // Additional
    m_employee->setNotes(m_notesEdit->toPlainText().trimmed());
    
    // Set timestamps if new employee
    if (m_isNewEmployee) {
        QDateTime now = QDateTime::currentDateTime();
        m_employee->setCreatedAt(now);
        m_employee->setUpdatedAt(now);
    }
}

void EmployeeDialog::markAsModified()
{
    m_isModified = true;
}

void EmployeeDialog::accept()
{
    if (!validateInput()) {
        return;
    }
    
    applyFormData();
    QDialog::accept();
}

void EmployeeDialog::reject()
{
    if (m_isModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Unsaved Changes"),
            tr("You have unsaved changes. Are you sure you want to close?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    
    QDialog::reject();
}

void EmployeeDialog::closeEvent(QCloseEvent *event)
{
    if (m_isModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Unsaved Changes"),
            tr("You have unsaved changes. Are you sure you want to close?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (reply != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    
    event->accept();
}

void EmployeeDialog::onFieldChanged()
{
    markAsModified();
}

void EmployeeDialog::onGenerateCin()
{
    QString newCin = Employee::generateCin();
    m_cinEdit->setText(newCin);
    markAsModified();
}

void EmployeeDialog::onValidateEmail()
{
    QString email = m_emailEdit->text().trimmed();
    if (email.isEmpty()) return;
    
    QRegularExpression emailRegex("^[A-Za-z0-9+_.-]+@[A-ZaZ0-9.-]+\\.[A-Za-z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, tr("Invalid Email"), 
                           tr("Please enter a valid email address."));
        m_emailEdit->setFocus();
        m_emailEdit->selectAll();
    }
}

void EmployeeDialog::onRoleChanged()
{
    markAsModified();
}

void EmployeeDialog::onStatusChanged()
{
    markAsModified();
}

void EmployeeDialog::onOpenAIAssistant()
{
    QMessageBox::information(this, tr("AI Assistant"), 
                           tr("AI Assistant functionality will be implemented in a future update."));
}

void EmployeeDialog::onGeocodeAddress()
{
    QString address = m_addressEdit->toPlainText().trimmed();
    if (address.isEmpty()) {
        QMessageBox::warning(this, tr("No Address"), 
                           tr("Please enter an address to geocode."));
        return;
    }
    
    // Initialize MapboxHandler if not already done
    if (!m_mapHandler) {
        m_mapHandler = new MapboxHandler(this);
        connect(m_mapHandler, &MapboxHandler::geocodingCompleted, this, 
                &EmployeeDialog::onGeocodeCompleted);
        connect(m_mapHandler, &MapboxHandler::geocodingFailed, this,
                &EmployeeDialog::onGeocodeFailed);
    }
    
    m_geocodeBtn->setEnabled(false);
    m_geocodeBtn->setText("Geocoding...");
    m_mapHandler->geocodeAddress(address);
}

void EmployeeDialog::onShowMap()
{
    if (m_mapHandler) {
        QWidget *mapWidget = m_mapHandler->createMapWidget();
        if (mapWidget) {
            mapWidget->setWindowTitle(tr("Employee Location - %1").arg(m_employee ? m_employee->fullName() : "New Employee"));
            mapWidget->show();
        }
    }
}

void EmployeeDialog::onGeocodeCompleted(const QString &address, double lat, double lng)
{
    Q_UNUSED(address)
    
    m_geocodeBtn->setEnabled(true);
    m_geocodeBtn->setText("🌍 Geocode Address");
    m_showMapBtn->setEnabled(true);
    
    if (m_mapHandler) {
        m_mapHandler->showLocation(lat, lng, m_employee ? m_employee->fullName() : "Employee Location");
    }
    
    QMessageBox::information(this, tr("Geocoding Success"), 
                           tr("Address geocoded successfully!\nCoordinates: %1, %2").arg(lat).arg(lng));
}

void EmployeeDialog::onGeocodeFailed(const QString &error)
{
    m_geocodeBtn->setEnabled(true);
    m_geocodeBtn->setText("🌍 Geocode Address");
    
    QMessageBox::warning(this, tr("Geocoding Failed"), 
                       tr("Failed to geocode address: %1").arg(error));
}
