#include "supplierdialog.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QUrl>

SupplierDialog::SupplierDialog(Mode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_mainLayout(nullptr)
    , m_basicInfoGroup(nullptr)
    , m_basicInfoLayout(nullptr)
    , m_nameEdit(nullptr)
    , m_contactPersonEdit(nullptr)
    , m_contactGroup(nullptr)
    , m_contactLayout(nullptr)
    , m_emailEdit(nullptr)
    , m_phoneEdit(nullptr)
    , m_websiteEdit(nullptr)
    , m_addressGroup(nullptr)
    , m_addressLayout(nullptr)
    , m_addressEdit(nullptr)
    , m_cityEdit(nullptr)
    , m_countryEdit(nullptr)
    , m_additionalGroup(nullptr)
    , m_additionalLayout(nullptr)
    , m_notesEdit(nullptr)
    , m_activeCheckBox(nullptr)
    , m_buttonLayout(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_statusLabel(nullptr)
{
    setupUI();
    setupConnections();
    updateWindowTitle();
    
    // Set default values
    m_activeCheckBox->setChecked(true);
    
    resize(500, 600);
}

void SupplierDialog::setupUI()
{
    setModal(true);
    
    m_mainLayout = new QVBoxLayout(this);
    
    // Basic Information Group
    m_basicInfoGroup = new QGroupBox("Basic Information", this);
    m_basicInfoLayout = new QFormLayout(m_basicInfoGroup);
    
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Enter supplier name");
    m_basicInfoLayout->addRow("Name *:", m_nameEdit);
    
    m_contactPersonEdit = new QLineEdit(this);
    m_contactPersonEdit->setPlaceholderText("Enter contact person name");
    m_basicInfoLayout->addRow("Contact Person:", m_contactPersonEdit);
    
    m_mainLayout->addWidget(m_basicInfoGroup);
    
    // Contact Information Group
    m_contactGroup = new QGroupBox("Contact Information", this);
    m_contactLayout = new QFormLayout(m_contactGroup);
    
    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("email@example.com");
    m_contactLayout->addRow("Email:", m_emailEdit);
    
    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setPlaceholderText("+1-555-0123");
    m_contactLayout->addRow("Phone:", m_phoneEdit);
    
    m_websiteEdit = new QLineEdit(this);
    m_websiteEdit->setPlaceholderText("www.example.com");
    m_contactLayout->addRow("Website:", m_websiteEdit);
    
    m_mainLayout->addWidget(m_contactGroup);
    
    // Address Information Group
    m_addressGroup = new QGroupBox("Address Information", this);
    m_addressLayout = new QFormLayout(m_addressGroup);
    
    m_addressEdit = new QLineEdit(this);
    m_addressEdit->setPlaceholderText("Street address");
    m_addressLayout->addRow("Address:", m_addressEdit);
    
    m_cityEdit = new QLineEdit(this);
    m_cityEdit->setPlaceholderText("City");
    m_addressLayout->addRow("City:", m_cityEdit);
    
    m_countryEdit = new QLineEdit(this);
    m_countryEdit->setPlaceholderText("Country");
    m_addressLayout->addRow("Country:", m_countryEdit);
    
    m_mainLayout->addWidget(m_addressGroup);
    
    // Additional Information Group
    m_additionalGroup = new QGroupBox("Additional Information", this);
    m_additionalLayout = new QVBoxLayout(m_additionalGroup);
    
    QLabel *notesLabel = new QLabel("Notes:", this);
    m_additionalLayout->addWidget(notesLabel);
    
    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setPlaceholderText("Additional notes about the supplier");
    m_notesEdit->setMaximumHeight(100);
    m_additionalLayout->addWidget(m_notesEdit);
    
    m_activeCheckBox = new QCheckBox("Active Supplier", this);
    m_activeCheckBox->setToolTip("Uncheck to mark this supplier as inactive");
    m_additionalLayout->addWidget(m_activeCheckBox);
    
    m_mainLayout->addWidget(m_additionalGroup);
    
    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    m_statusLabel->hide();
    m_mainLayout->addWidget(m_statusLabel);
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_okButton = new QPushButton("OK", this);
    m_okButton->setDefault(true);
    m_buttonLayout->addWidget(m_okButton);
    
    m_cancelButton = new QPushButton("Cancel", this);
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Initial validation
    validateForm();
}

void SupplierDialog::setupConnections()
{
    connect(m_okButton, &QPushButton::clicked, this, &SupplierDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &SupplierDialog::onReject);
    
    // Connect field change signals for validation
    connect(m_nameEdit, &QLineEdit::textChanged, this, &SupplierDialog::onFieldChanged);
    connect(m_emailEdit, &QLineEdit::textChanged, this, &SupplierDialog::onFieldChanged);
}

void SupplierDialog::updateWindowTitle()
{
    if (m_mode == AddMode) {
        setWindowTitle("Add New Supplier");
    } else {
        setWindowTitle("Edit Supplier");
    }
}

void SupplierDialog::setSupplier(const Supplier &supplier)
{
    m_nameEdit->setText(supplier.name);
    m_contactPersonEdit->setText(supplier.contactPerson);
    m_emailEdit->setText(supplier.email);
    m_phoneEdit->setText(supplier.phone);
    m_websiteEdit->setText(supplier.website);
    m_addressEdit->setText(supplier.address);
    m_cityEdit->setText(supplier.city);
    m_countryEdit->setText(supplier.country);
    m_notesEdit->setPlainText(supplier.notes);
    m_activeCheckBox->setChecked(supplier.isActive);
    
    validateForm();
}

Supplier SupplierDialog::getSupplier() const
{
    Supplier supplier;
    supplier.name = m_nameEdit->text().trimmed();
    supplier.contactPerson = m_contactPersonEdit->text().trimmed();
    supplier.email = m_emailEdit->text().trimmed();
    supplier.phone = m_phoneEdit->text().trimmed();
    supplier.website = m_websiteEdit->text().trimmed();
    supplier.address = m_addressEdit->text().trimmed();
    supplier.city = m_cityEdit->text().trimmed();
    supplier.country = m_countryEdit->text().trimmed();
    supplier.notes = m_notesEdit->toPlainText().trimmed();
    supplier.isActive = m_activeCheckBox->isChecked();
    
    return supplier;
}

void SupplierDialog::clearForm()
{
    m_nameEdit->clear();
    m_contactPersonEdit->clear();
    m_emailEdit->clear();
    m_phoneEdit->clear();
    m_websiteEdit->clear();
    m_addressEdit->clear();
    m_cityEdit->clear();
    m_countryEdit->clear();
    m_notesEdit->clear();
    m_activeCheckBox->setChecked(true);
    m_statusLabel->hide();
    
    validateForm();
}

void SupplierDialog::onAccept()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Supplier name is required.");
        m_nameEdit->setFocus();
        return;
    }
    
    QString email = m_emailEdit->text().trimmed();
    if (!email.isEmpty() && !isValidEmail(email)) {
        QMessageBox::warning(this, "Validation Error", "Please enter a valid email address.");
        m_emailEdit->setFocus();
        return;
    }
    
    accept();
}

void SupplierDialog::onReject()
{
    reject();
}

void SupplierDialog::onFieldChanged()
{
    validateForm();
}

void SupplierDialog::validateForm()
{
    bool isValid = true;
    QString statusMessage;
    
    // Check required fields
    if (m_nameEdit->text().trimmed().isEmpty()) {
        isValid = false;
        statusMessage = "Supplier name is required";
    }
    
    // Check email format if provided
    QString email = m_emailEdit->text().trimmed();
    if (!email.isEmpty() && !isValidEmail(email)) {
        isValid = false;
        statusMessage = "Invalid email format";
    }
    
    // Update UI based on validation
    m_okButton->setEnabled(isValid);
    
    if (isValid) {
        m_statusLabel->hide();
    } else {
        m_statusLabel->setText(statusMessage);
        m_statusLabel->show();
    }
}

bool SupplierDialog::isValidEmail(const QString &email) const
{
    // Simple email validation using regex
    static QRegularExpression emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return emailRegex.match(email).hasMatch();
}
