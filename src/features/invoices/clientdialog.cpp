#include "clientdialog.h"
#include "client.h"
#include "invoicedatabasemanager.h"

#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QUuid>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

ClientDialog::ClientDialog(QWidget *parent)
    : QDialog(parent)
    , m_client(nullptr)
    , m_dbManager(nullptr)
    , m_isModified(false)
    , m_isNewClient(true)
{
    setupUI();
    setupConnections();
    setWindowTitle(tr("New Client"));
    resize(500, 400);
}

ClientDialog::ClientDialog(Client *client, QWidget *parent)
    : QDialog(parent)
    , m_client(client)
    , m_dbManager(nullptr)
    , m_isModified(false)
    , m_isNewClient(false)
{
    setupUI();
    setupConnections();
    populateForm();
    setWindowTitle(tr("Edit Client - %1").arg(client ? client->name() : ""));
    resize(500, 400);
}

ClientDialog::~ClientDialog()
{
    // Cleanup if needed
}

void ClientDialog::setClient(Client *client)
{
    m_client = client;
    m_isNewClient = (client == nullptr);
    populateForm();
    setWindowTitle(tr("Edit Client - %1").arg(client ? client->name() : "New Client"));
}

void ClientDialog::setDatabaseManager(InvoiceDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
}

void ClientDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    
    // Basic info tab
    m_basicTab = new QWidget();
    m_basicForm = new QFormLayout(m_basicTab);
    
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText(tr("Enter client name"));
    m_basicForm->addRow(tr("Name:*"), m_nameEdit);
    
    m_companyEdit = new QLineEdit();
    m_companyEdit->setPlaceholderText(tr("Enter company name"));
    m_basicForm->addRow(tr("Company:"), m_companyEdit);
    
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText(tr("Enter email address"));
    // Add email validation
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    m_emailEdit->setValidator(new QRegularExpressionValidator(emailRegex, this));
    m_basicForm->addRow(tr("Email:"), m_emailEdit);
    
    m_phoneEdit = new QLineEdit();
    m_phoneEdit->setPlaceholderText(tr("Enter phone number"));
    m_basicForm->addRow(tr("Phone:"), m_phoneEdit);
    
    m_tabWidget->addTab(m_basicTab, tr("Basic Info"));
    
    // Address tab
    m_addressTab = new QWidget();
    m_addressForm = new QFormLayout(m_addressTab);
    
    m_addressEdit = new QTextEdit();
    m_addressEdit->setPlaceholderText(tr("Enter full address"));
    m_addressEdit->setMaximumHeight(120);
    m_addressForm->addRow(tr("Address:"), m_addressEdit);
    
    m_taxIdEdit = new QLineEdit();
    m_taxIdEdit->setPlaceholderText(tr("Enter tax ID number"));
    m_addressForm->addRow(tr("Tax ID:"), m_taxIdEdit);
    
    m_tabWidget->addTab(m_addressTab, tr("Address & Tax"));
    
    // Notes tab
    m_notesTab = new QWidget();
    m_notesLayout = new QVBoxLayout(m_notesTab);
    
    QLabel *notesLabel = new QLabel(tr("Notes:"));
    m_notesEdit = new QTextEdit();
    m_notesEdit->setPlaceholderText(tr("Enter any additional notes about this client"));
    
    m_notesLayout->addWidget(notesLabel);
    m_notesLayout->addWidget(m_notesEdit);
    
    m_tabWidget->addTab(m_notesTab, tr("Notes"));
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Dialog buttons
    m_buttonLayout = new QHBoxLayout();
    
    m_saveBtn = new QPushButton(tr("Save"), this);
    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    
    m_saveBtn->setDefault(true);
    m_saveBtn->setStyleSheet("QPushButton { background-color: #0066CC; color: white; font-weight: bold; }");
    
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_saveBtn);
    m_buttonLayout->addWidget(m_cancelBtn);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Add required field indicator
    QLabel *requiredLabel = new QLabel(tr("* Required fields"));
    requiredLabel->setStyleSheet("color: #666; font-style: italic;");
    m_mainLayout->addWidget(requiredLabel);
}

void ClientDialog::setupConnections()
{
    // Form connections
    connect(m_nameEdit, &QLineEdit::textChanged, this, &ClientDialog::onNameChanged);
    connect(m_companyEdit, &QLineEdit::textChanged, this, &ClientDialog::onCompanyChanged);
    connect(m_addressEdit, &QTextEdit::textChanged, this, &ClientDialog::onAddressChanged);
    connect(m_emailEdit, &QLineEdit::textChanged, this, &ClientDialog::onEmailChanged);
    connect(m_phoneEdit, &QLineEdit::textChanged, this, &ClientDialog::onPhoneChanged);
    connect(m_taxIdEdit, &QLineEdit::textChanged, this, &ClientDialog::onTaxIdChanged);
    connect(m_notesEdit, &QTextEdit::textChanged, this, &ClientDialog::onNotesChanged);
    
    // Button connections
    connect(m_saveBtn, &QPushButton::clicked, this, &ClientDialog::onSaveClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ClientDialog::onCancelClicked);
}

void ClientDialog::populateForm()
{
    if (!m_client) {
        clearForm();
        return;
    }
    
    m_nameEdit->setText(m_client->name());
    m_companyEdit->setText(m_client->company());
    m_emailEdit->setText(m_client->email());
    m_phoneEdit->setText(m_client->phone());
    m_addressEdit->setPlainText(m_client->address());
    m_taxIdEdit->setText(m_client->taxId());
    m_notesEdit->setPlainText(m_client->notes());
}

void ClientDialog::clearForm()
{
    m_nameEdit->clear();
    m_companyEdit->clear();
    m_emailEdit->clear();
    m_phoneEdit->clear();
    m_addressEdit->clear();
    m_taxIdEdit->clear();
    m_notesEdit->clear();
}

bool ClientDialog::validateInput()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Client name is required."));
        m_tabWidget->setCurrentWidget(m_basicTab);
        m_nameEdit->setFocus();
        return false;
    }
    
    QString email = m_emailEdit->text().trimmed();
    if (!email.isEmpty()) {
        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(email).hasMatch()) {
            QMessageBox::warning(this, tr("Validation Error"), tr("Please enter a valid email address."));
            m_tabWidget->setCurrentWidget(m_basicTab);
            m_emailEdit->setFocus();
            return false;
        }
    }
    
    return true;
}

void ClientDialog::markAsModified()
{
    m_isModified = true;
    validateForm();
}

void ClientDialog::applyFormData()
{
    if (!m_client) {
        m_client = new Client(this);
        m_client->setId(QUuid::createUuid().toString(QUuid::WithoutBraces));
        m_isNewClient = true;
    }
    
    m_client->setName(m_nameEdit->text().trimmed());
    m_client->setCompany(m_companyEdit->text().trimmed());
    m_client->setEmail(m_emailEdit->text().trimmed());
    m_client->setPhone(m_phoneEdit->text().trimmed());
    m_client->setAddress(m_addressEdit->toPlainText().trimmed());
    m_client->setTaxId(m_taxIdEdit->text().trimmed());
    m_client->setNotes(m_notesEdit->toPlainText().trimmed());
}

// Slot implementations
void ClientDialog::onNameChanged()
{
    markAsModified();
}

void ClientDialog::onCompanyChanged()
{
    markAsModified();
}

void ClientDialog::onAddressChanged()
{
    markAsModified();
}

void ClientDialog::onEmailChanged()
{
    markAsModified();
}

void ClientDialog::onPhoneChanged()
{
    markAsModified();
}

void ClientDialog::onTaxIdChanged()
{
    markAsModified();
}

void ClientDialog::onNotesChanged()
{
    markAsModified();
}

void ClientDialog::onSaveClicked()
{
    accept();
}

void ClientDialog::onCancelClicked()
{
    reject();
}

void ClientDialog::validateForm()
{
    // Update button states based on validation
    bool isValid = !m_nameEdit->text().trimmed().isEmpty();
    
    // Check email format if provided
    QString email = m_emailEdit->text().trimmed();
    if (!email.isEmpty()) {
        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        isValid = isValid && emailRegex.match(email).hasMatch();
    }
    
    m_saveBtn->setEnabled(isValid);
    
    // Update window title with modification indicator
    QString title = m_isNewClient ? tr("New Client") : tr("Edit Client - %1").arg(m_client ? m_client->name() : "");
    if (m_isModified) {
        title += " *";
    }
    setWindowTitle(title);
}

void ClientDialog::accept()
{
    if (!validateInput()) return;
    
    applyFormData();
    
    QDialog::accept();
}

void ClientDialog::reject()
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

void ClientDialog::closeEvent(QCloseEvent *event)
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
