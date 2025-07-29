#include "contractdialog.h"
#include "contract.h"
#include "contractdatabasemanager.h"
#include "utils/stylemanager.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QLocale>
#include <QRegularExpressionValidator>

ContractDialog::ContractDialog(QWidget *parent)
    : BaseDialog(parent)
    , m_contract(nullptr)
    , m_dbManager(nullptr)
    , m_mode(AddMode)
    , m_isModified(false)
{
    setWindowTitle("Add New Contract");
    setMinimumSize(500, 600);
    resize(600, 700);
    
    setupUi();
    setupConnections();
    applyArchiFlowStyling();
    
    // Initialize with a new contract if in Add mode
    m_contract = new Contract(this);
}

ContractDialog::ContractDialog(Mode mode, QWidget *parent)
    : BaseDialog(parent)
    , m_contract(nullptr)
    , m_dbManager(nullptr)
    , m_mode(mode)
    , m_isModified(false)
{
    setWindowTitle(mode == AddMode ? "Add New Contract" : "Edit Contract");
    setMinimumSize(500, 600);
    resize(600, 700);
    
    setupUi();
    setupConnections();
    applyArchiFlowStyling();
    
    // Initialize with a new contract if in Add mode
    if (m_mode == AddMode) {
        m_contract = new Contract(this);
    }
}

ContractDialog::~ContractDialog()
{
}

void ContractDialog::setupUi()
{
    // Create main content widget for BaseDialog
    QWidget *mainWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Title
    QLabel *titleLabel = new QLabel(m_mode == AddMode ? "Add New Contract" : "Edit Contract");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Form group
    QGroupBox *formGroup = new QGroupBox("Contract Information");
    formGroup->setObjectName("formGroup");
    m_formLayout = new QFormLayout(formGroup);
    m_formLayout->setSpacing(15);
    m_formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);

    // Client Name
    m_clientNameEdit = new QLineEdit;
    m_clientNameEdit->setPlaceholderText("Enter client or company name");
    m_clientNameEdit->setMaxLength(255);
    m_formLayout->addRow("Client Name *:", m_clientNameEdit);

    // Date range
    QHBoxLayout *dateLayout = new QHBoxLayout;
    m_startDateEdit = new QDateEdit;
    m_startDateEdit->setDate(QDate::currentDate());
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("dd/MM/yyyy");
    
    m_endDateEdit = new QDateEdit;
    m_endDateEdit->setDate(QDate::currentDate().addYears(1));
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("dd/MM/yyyy");
    
    dateLayout->addWidget(m_startDateEdit);
    dateLayout->addWidget(new QLabel("to"));
    dateLayout->addWidget(m_endDateEdit);
    m_formLayout->addRow("Contract Period *:", dateLayout);

    // Contract Value
    m_valueSpinBox = new QDoubleSpinBox;
    m_valueSpinBox->setRange(0.0, 999999999.99);
    m_valueSpinBox->setDecimals(2);
    m_valueSpinBox->setSuffix(" " + QLocale().currencySymbol());
    m_valueSpinBox->setGroupSeparatorShown(true);
    m_formLayout->addRow("Contract Value *:", m_valueSpinBox);

    // Status
    m_statusComboBox = new QComboBox;
    m_statusComboBox->addItems(Contract::availableStatuses());
    m_formLayout->addRow("Status *:", m_statusComboBox);

    // Payment Terms
    m_paymentTermsSpinBox = new QSpinBox;
    m_paymentTermsSpinBox->setRange(1, 365);
    m_paymentTermsSpinBox->setValue(30);
    m_paymentTermsSpinBox->setSuffix(" days");
    m_formLayout->addRow("Payment Terms:", m_paymentTermsSpinBox);

    // Non-Compete Clause
    m_nonCompeteCheckBox = new QCheckBox("Include non-compete clause");
    m_formLayout->addRow("Additional Terms:", m_nonCompeteCheckBox);

    // Description
    m_descriptionEdit = new QTextEdit;
    m_descriptionEdit->setMaximumHeight(100);
    m_descriptionEdit->setPlaceholderText("Optional contract description or notes");
    m_formLayout->addRow("Description:", m_descriptionEdit);

    mainLayout->addWidget(formGroup);

    // Status information
    QGroupBox *statusGroup = new QGroupBox("Contract Status");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    
    m_daysUntilExpiryLabel = new QLabel;
    m_daysUntilExpiryLabel->setObjectName("statusLabel");
    statusLayout->addWidget(m_daysUntilExpiryLabel);
    
    mainLayout->addWidget(statusGroup);

    // Validation feedback
    m_validationLabel = new QLabel;
    m_validationLabel->setObjectName("validationLabel");
    m_validationLabel->setWordWrap(true);
    m_validationLabel->hide();
    mainLayout->addWidget(m_validationLabel);

    // Set the main widget for BaseDialog
    setMainWidget(mainWidget);
    
    // Get references to the buttons from BaseDialog
    m_saveButton = m_buttonBox->button(QDialogButtonBox::Ok);
    m_cancelButton = m_buttonBox->button(QDialogButtonBox::Cancel);
    
    // Update button text
    if (m_saveButton) {
        m_saveButton->setText(m_mode == AddMode ? "Add Contract" : "Save Changes");
        m_saveButton->setObjectName("primaryButton");
    }
    if (m_cancelButton) {
        m_cancelButton->setText("Cancel");
        m_cancelButton->setObjectName("cancelButton");
    }
}

void ContractDialog::setupConnections()
{
    // Button connections - BaseDialog handles these automatically
    // We override the BaseDialog methods instead
    
    // Field change connections for validation
    connect(m_clientNameEdit, &QLineEdit::textChanged, this, &ContractDialog::onFieldChanged);
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, &ContractDialog::onStartDateChanged);
    connect(m_endDateEdit, &QDateEdit::dateChanged, this, &ContractDialog::onEndDateChanged);
    connect(m_valueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ContractDialog::onFieldChanged);
    connect(m_statusComboBox, &QComboBox::currentTextChanged, this, &ContractDialog::onFieldChanged);
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &ContractDialog::onFieldChanged);
    connect(m_paymentTermsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ContractDialog::onFieldChanged);
    connect(m_nonCompeteCheckBox, &QCheckBox::toggled, this, &ContractDialog::onFieldChanged);
}

void ContractDialog::applyArchiFlowStyling()
{
    // Get ArchiFlow colors
    QString primaryColor = StyleManager::getPrimaryColor();   // #3D485A
    QString accentColor = StyleManager::getAccentColor();     // #E3C6B0
    QString successColor = StyleManager::getSuccessColor();   // #4CAF50
    QString errorColor = StyleManager::getErrorColor();       // #FF6B6B

    setStyleSheet(QString(R"(
        ContractDialog {
            background-color: %1;
            color: %2;
        }
        
        QLabel#titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: %2;
            margin-bottom: 10px;
        }
        
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: %2;
            border: 2px solid %2;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            background-color: %1;
        }
        
        QLineEdit, QDateEdit, QDoubleSpinBox, QSpinBox, QComboBox {
            padding: 8px;
            border: 2px solid %2;
            border-radius: 6px;
            background-color: white;
            color: %1;
            font-size: 12px;
        }
        
        QLineEdit:focus, QDateEdit:focus, QDoubleSpinBox:focus, QSpinBox:focus, QComboBox:focus {
            border-color: %3;
        }
        
        QTextEdit {
            padding: 8px;
            border: 2px solid %2;
            border-radius: 6px;
            background-color: white;
            color: %1;
            font-size: 12px;
        }
        
        QTextEdit:focus {
            border-color: %3;
        }
        
        QCheckBox {
            color: %2;
            font-size: 12px;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid %2;
            border-radius: 3px;
            background-color: white;
        }
        
        QCheckBox::indicator:checked {
            background-color: %3;
            border-color: %3;
        }
        
        QPushButton {
            padding: 10px 20px;
            border: 2px solid %2;
            border-radius: 6px;
            background-color: white;
            color: %1;
            font-weight: bold;
            font-size: 12px;
            min-width: 100px;
        }
        
        QPushButton:hover {
            background-color: %2;
            color: %1;
        }
        
        QPushButton#primaryButton {
            background-color: %2;
            color: %1;
            border-color: %2;
        }
        
        QPushButton#primaryButton:hover {
            background-color: %4;
            border-color: %4;
        }
        
        QPushButton#primaryButton:disabled {
            background-color: #CCCCCC;
            border-color: #CCCCCC;
            color: #666666;
        }
        
        QPushButton#cancelButton:hover {
            background-color: %5;
            border-color: %5;
            color: white;
        }
        
        QLabel#statusLabel {
            color: %3;
            font-weight: bold;
            padding: 8px;
            border-radius: 4px;
            background-color: rgba(76, 175, 80, 0.1);
        }
        
        QLabel#validationLabel {
            color: %5;
            background-color: rgba(255, 107, 107, 0.1);
            padding: 8px;
            border-radius: 4px;
            border: 1px solid %5;
        }
    )").arg(primaryColor, accentColor, successColor, accentColor.replace("#", "rgba(").replace("B0", "B0, 0.8)"), errorColor));

    // Apply material design elevation
    StyleManager::addElevation(this, 3);
}

// IContractDialog interface implementations
void ContractDialog::setMode(DialogMode mode)
{
    m_mode = static_cast<Mode>(mode);
    
    switch (mode) {
        case CreateMode:
            setWindowTitle(tr("Add New Contract"));
            setReadOnly(false);
            break;
        case EditMode:
            setWindowTitle(tr("Edit Contract"));
            setReadOnly(false);
            break;
        case ViewMode:
            setWindowTitle(tr("View Contract"));
            setReadOnly(true);
            break;
        case DuplicateMode:
            setWindowTitle(tr("Duplicate Contract"));
            setReadOnly(false);
            break;
    }
}

IContractDialog::DialogMode ContractDialog::getMode() const
{
    return static_cast<IContractDialog::DialogMode>(m_mode);
}

void ContractDialog::setTitle(const QString &title)
{
    setWindowTitle(title);
}

void ContractDialog::setContract(Contract *contract)
{
    if (m_contract && m_contract->parent() == this) {
        delete m_contract;
    }
    
    m_contract = contract;
    
    if (contract) {
        populateFromContract();
    }
}

Contract* ContractDialog::getContract() const
{
    return m_contract;
}

void ContractDialog::setDatabaseManager(ContractDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
}

void ContractDialog::clearForm()
{
    m_clientNameEdit->clear();
    m_descriptionEdit->clear();
    m_startDateEdit->setDate(QDate::currentDate());
    m_endDateEdit->setDate(QDate::currentDate().addYears(1));
    m_valueSpinBox->setValue(0.0);
    m_paymentTermsSpinBox->setValue(30);
    m_statusComboBox->setCurrentIndex(0);
    m_nonCompeteCheckBox->setChecked(false);
    
    m_isModified = false;
    emit dataChanged();
}

bool ContractDialog::validateInput()
{
    m_validationErrors.clear();
      // Validate required fields
    if (m_clientNameEdit->text().trimmed().isEmpty()) {
        m_validationErrors.append(tr("Client name is required"));
    }
    
    if (m_valueSpinBox->value() <= 0) {
        m_validationErrors.append(tr("Contract value must be greater than 0"));
    }
    
    if (m_endDateEdit->date() <= m_startDateEdit->date()) {
        m_validationErrors.append(tr("End date must be after start date"));
    }
    
    bool isValid = m_validationErrors.isEmpty();
    emit validationChanged(isValid);
    
    if (!isValid) {
        showValidationSummary();
    } else {
        m_validationLabel->hide();
    }
    
    return isValid;
}

bool ContractDialog::hasUnsavedChanges()
{
    return m_isModified;
}

void ContractDialog::resetValidation()
{
    m_validationErrors.clear();
    m_validationLabel->hide();
    emit validationChanged(true);
}

void ContractDialog::setReadOnly(bool readOnly)
{
    m_clientNameEdit->setReadOnly(readOnly);
    m_descriptionEdit->setReadOnly(readOnly);
    m_startDateEdit->setReadOnly(readOnly);
    m_endDateEdit->setReadOnly(readOnly);
    m_valueSpinBox->setReadOnly(readOnly);
    m_paymentTermsSpinBox->setReadOnly(readOnly);
    m_statusComboBox->setEnabled(!readOnly);
    m_nonCompeteCheckBox->setEnabled(!readOnly);
    
    // Update save button state
    if (m_saveButton) {
        m_saveButton->setEnabled(!readOnly);
    }
}

bool ContractDialog::isReadOnly() const
{
    return m_clientNameEdit->isReadOnly();
}

void ContractDialog::setFieldsEnabled(bool enabled)
{
    setReadOnly(!enabled);
}

bool ContractDialog::saveContract()
{
    if (!validateInput()) {
        return false;
    }
    
    if (!m_dbManager) {
        QMessageBox::warning(this, tr("Database Error"), 
                           tr("Database manager is not available. Cannot save contract."));
        return false;
    }
    
    updateContractFromFields();
    
    bool success = false;
    QString contractId;
    
    qDebug() << "Attempting to save contract in mode:" << (m_mode == AddMode ? "Add" : "Edit");
    qDebug() << "Contract details - Client:" << m_contract->clientName() 
             << "Value:" << m_contract->value() 
             << "Status:" << m_contract->status();
    
    if (m_mode == AddMode) {
        // Adding new contract
        contractId = m_dbManager->addContract(m_contract);
        success = !contractId.isEmpty();
        
        qDebug() << "Add contract result - ID:" << contractId << "Success:" << success;
        
        if (success) {
            QMessageBox::information(this, tr("Success"), 
                                   tr("Contract added successfully with ID: %1").arg(contractId));
        } else {
            QString error = m_dbManager->getLastError();
            qDebug() << "Add contract failed with error:" << error;
            QMessageBox::critical(this, tr("Error"), 
                                tr("Failed to add contract: %1").arg(error));
        }
    } else {
        // Updating existing contract
        success = m_dbManager->updateContract(m_contract);
        contractId = m_contract->id();
        
        qDebug() << "Update contract result - ID:" << contractId << "Success:" << success;
        
        if (success) {
            QMessageBox::information(this, tr("Success"), 
                                   tr("Contract updated successfully!"));
        } else {
            QString error = m_dbManager->getLastError();
            qDebug() << "Update contract failed with error:" << error;
            QMessageBox::critical(this, tr("Error"), 
                                tr("Failed to update contract: %1").arg(error));
        }
    }
    
    if (success) {
        m_isModified = false;
        emit contractSaved(contractId);
    }
    
    return success;
}

void ContractDialog::loadContract(const QString &contractId)
{
    // TODO: Load contract from database using contractId
    // For now, this is a placeholder implementation
    Q_UNUSED(contractId)
    QMessageBox::information(this, tr("Load Contract"), 
                           tr("Load contract functionality will be implemented with database integration."));
}

void ContractDialog::focusFirstField()
{
    m_clientNameEdit->setFocus();
    m_clientNameEdit->selectAll();
}

void ContractDialog::showValidationSummary()
{
    showValidationErrors(m_validationErrors);
}

void ContractDialog::highlightInvalidFields()
{
    // Reset all field styles first
    QString normalStyle = "";
    QString errorStyle = "border: 2px solid #e74c3c; background-color: rgba(231, 76, 60, 0.1);";
    
    // Apply normal style to all fields
    m_clientNameEdit->setStyleSheet(normalStyle);
    m_valueSpinBox->setStyleSheet(normalStyle);
    m_startDateEdit->setStyleSheet(normalStyle);
    m_endDateEdit->setStyleSheet(normalStyle);
    
    // Highlight fields with errors
    if (m_clientNameEdit->text().trimmed().isEmpty()) {
        m_clientNameEdit->setStyleSheet(errorStyle);
    }
    
    if (m_valueSpinBox->value() <= 0) {
        m_valueSpinBox->setStyleSheet(errorStyle);
    }
    
    if (m_endDateEdit->date() <= m_startDateEdit->date()) {
        m_startDateEdit->setStyleSheet(errorStyle);
        m_endDateEdit->setStyleSheet(errorStyle);
    }
}



bool ContractDialog::isValid() const
{
    return m_contract && m_contract->isValid();
}

void ContractDialog::populateFromContract()
{
    if (!m_contract) return;

    m_clientNameEdit->setText(m_contract->clientName());
    m_startDateEdit->setDate(m_contract->startDate());
    m_endDateEdit->setDate(m_contract->endDate());
    m_valueSpinBox->setValue(m_contract->value());
    m_statusComboBox->setCurrentText(m_contract->status());
    m_descriptionEdit->setPlainText(m_contract->description());
    m_paymentTermsSpinBox->setValue(m_contract->paymentTerms());
    m_nonCompeteCheckBox->setChecked(m_contract->hasNonCompeteClause());

    m_isModified = false;
}

void ContractDialog::updateContractFromFields()
{
    if (!m_contract) return;

    m_contract->setClientName(m_clientNameEdit->text().trimmed());
    m_contract->setStartDate(m_startDateEdit->date());
    m_contract->setEndDate(m_endDateEdit->date());
    m_contract->setValue(m_valueSpinBox->value());
    m_contract->setStatus(m_statusComboBox->currentText());
    m_contract->setDescription(m_descriptionEdit->toPlainText().trimmed());
    m_contract->setPaymentTerms(m_paymentTermsSpinBox->value());
    m_contract->setHasNonCompeteClause(m_nonCompeteCheckBox->isChecked());
}

void ContractDialog::onAcceptClicked()
{
    updateContractFromFields();
    
    if (isValid()) {
        emit contractSaved(m_contract ? m_contract->id() : QString());
        // Don't call accept() here, BaseDialog will handle it
    } else {
        showValidationErrors(m_contract->validationErrors());
    }
}

void ContractDialog::onRejectClicked()
{
    if (m_isModified) {
        int result = QMessageBox::question(this, "Discard Changes",
                                         "You have unsaved changes. Are you sure you want to discard them?",
                                         QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            return;
        }
    }
    emit contractCancelled();
}

// Override BaseDialog methods
void ContractDialog::accept()
{
    if (saveContract()) {
        BaseDialog::accept();
    }
    // If saveContract() returns false, the dialog stays open
    // and error messages are shown by saveContract()
}

void ContractDialog::reject()
{
    if (m_isModified) {
        int result = QMessageBox::question(this, "Discard Changes",
                                         "You have unsaved changes. Are you sure you want to discard them?",
                                         QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            return;
        }
    }
    emit contractCancelled();
    BaseDialog::reject();
}

void ContractDialog::onFieldChanged()
{
    m_isModified = true;
    validateForm();
}

void ContractDialog::validateForm()
{
    updateContractFromFields();
    
    if (m_contract) {
        QStringList errors = m_contract->validationErrors();
        
        if (errors.isEmpty()) {
            m_validationLabel->hide();
            m_saveButton->setEnabled(true);
            
            // Update status information
            int daysUntilExpiry = m_contract->daysUntilExpiry();
            if (daysUntilExpiry >= 0) {
                m_daysUntilExpiryLabel->setText(QString("Contract expires in %1 days").arg(daysUntilExpiry));
                if (daysUntilExpiry <= 30) {
                    m_daysUntilExpiryLabel->setStyleSheet("color: " + StyleManager::getWarningColor() + ";");
                } else {
                    m_daysUntilExpiryLabel->setStyleSheet("color: " + StyleManager::getSuccessColor() + ";");
                }
            } else {
                m_daysUntilExpiryLabel->setText("Contract has expired");
                m_daysUntilExpiryLabel->setStyleSheet("color: " + StyleManager::getErrorColor() + ";");
            }
        } else {
            showValidationErrors(errors);
            m_saveButton->setEnabled(false);
        }
    }
}

void ContractDialog::onStartDateChanged()
{
    // Ensure end date is not before start date
    if (m_endDateEdit->date() < m_startDateEdit->date()) {
        m_endDateEdit->setDate(m_startDateEdit->date().addMonths(12));
    }
    onFieldChanged();
}

void ContractDialog::onEndDateChanged()
{
    // Ensure start date is not after end date
    if (m_startDateEdit->date() > m_endDateEdit->date()) {
        m_startDateEdit->setDate(m_endDateEdit->date().addMonths(-12));
    }
    onFieldChanged();
}

void ContractDialog::resetForm()
{
    m_clientNameEdit->clear();
    m_startDateEdit->setDate(QDate::currentDate());
    m_endDateEdit->setDate(QDate::currentDate().addYears(1));
    m_valueSpinBox->setValue(0.0);
    m_statusComboBox->setCurrentText("Draft");
    m_descriptionEdit->clear();
    m_paymentTermsSpinBox->setValue(30);
    m_nonCompeteCheckBox->setChecked(false);
    m_validationLabel->hide();
    m_isModified = false;
}

void ContractDialog::showValidationErrors(const QStringList &errors)
{
    if (errors.isEmpty()) {
        m_validationLabel->hide();
        return;
    }

    QString errorText = "<b>Please fix the following errors:</b><ul>";
    for (const QString &error : errors) {
        errorText += "<li>" + error + "</li>";
    }
    errorText += "</ul>";

    m_validationLabel->setText(errorText);
    m_validationLabel->show();
}
