#include "projetdialog.h"
#include "ui_projetdialog.h"

#include <QRegularExpression>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

ProjetDialog::ProjetDialog(ProjetManager *projetManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjetDialog)
    , m_projetManager(projetManager)
    , m_isEditMode(false)
    , m_isViewMode(false)
    , m_isModified(false)
    , m_isNewProject(true)
    , m_nameValidator(nullptr)
    , m_coordinateValidator(nullptr)
    , m_budgetValidator(nullptr)
    , m_progressionValidator(nullptr)
    , m_clientCompleter(nullptr)
    , m_architecteCompleter(nullptr)
    , m_adresseCompleter(nullptr)
    , m_clientModel(nullptr)
    , m_architecteModel(nullptr)
{
    ui->setupUi(this);
    setupUI();
    setupValidators();
    setupConnections();
    setupCompleters();
    populateComboBoxes();
    applyArchiFlowStyling();
    
    // Initialize with default values for new project
    m_currentProjet = Projet();
    resetForm();
    updateButtonStates();
}

ProjetDialog::ProjetDialog(ProjetManager *projetManager, const Projet &projet, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjetDialog)
    , m_projetManager(projetManager)
    , m_currentProjet(projet)
    , m_originalProjet(projet)
    , m_isEditMode(true)
    , m_isViewMode(false)
    , m_isModified(false)
    , m_isNewProject(false)
    , m_nameValidator(nullptr)
    , m_coordinateValidator(nullptr)
    , m_budgetValidator(nullptr)
    , m_progressionValidator(nullptr)
    , m_clientCompleter(nullptr)
    , m_architecteCompleter(nullptr)
    , m_adresseCompleter(nullptr)
    , m_clientModel(nullptr)
    , m_architecteModel(nullptr)
{
    ui->setupUi(this);
    setupUI();
    setupValidators();
    setupConnections();
    setupCompleters();
    populateComboBoxes();
    applyArchiFlowStyling();
    
    // Load existing project data
    loadProjectData();
    updateButtonStates();
}

ProjetDialog::~ProjetDialog()
{
    delete ui;
}

// Static convenience methods
bool ProjetDialog::createNewProject(ProjetManager *projetManager, QWidget *parent)
{
    ProjetDialog dialog(projetManager, parent);
    dialog.setWindowTitle("Nouveau Projet - ArchiFlow");
    return dialog.exec() == QDialog::Accepted;
}

bool ProjetDialog::editProject(ProjetManager *projetManager, const Projet &projet, QWidget *parent)
{
    ProjetDialog dialog(projetManager, projet, parent);
    dialog.setWindowTitle(QString("Modifier Projet: %1 - ArchiFlow").arg(projet.getNom()));
    return dialog.exec() == QDialog::Accepted;
}

bool ProjetDialog::viewProject(ProjetManager *projetManager, const Projet &projet, QWidget *parent)
{
    ProjetDialog dialog(projetManager, projet, parent);
    dialog.setWindowTitle(QString("Visualiser Projet: %1 - ArchiFlow").arg(projet.getNom()));
    dialog.m_isViewMode = true;
    dialog.setFormReadOnly(true);
    dialog.updateButtonStates();
    return dialog.exec() == QDialog::Accepted;
}

void ProjetDialog::setupUI()
{
    // Set dialog properties
    setModal(true);
    setMinimumSize(800, 600);
    setAttribute(Qt::WA_DeleteOnClose, false);
    
    // Set initial tab
    ui->tabWidget->setCurrentIndex(0);
    
    // Configure spinboxes and sliders
    ui->doubleSpinBoxBudget->setMaximum(999999999.99);
    ui->doubleSpinBoxBudget->setDecimals(2);
    ui->doubleSpinBoxBudget->setSuffix(" €");
    
    ui->doubleSpinBoxSurface->setMaximum(999999.99);
    ui->doubleSpinBoxSurface->setDecimals(2);
    ui->doubleSpinBoxSurface->setSuffix(" m²");
    
    ui->spinBoxEtage->setMinimum(-10);
    ui->spinBoxEtage->setMaximum(200);
    
    ui->doubleSpinBoxLatitude->setRange(-90.0, 90.0);
    ui->doubleSpinBoxLatitude->setDecimals(6);
      ui->doubleSpinBoxLongitude->setRange(-180.0, 180.0);
    ui->doubleSpinBoxLongitude->setDecimals(6);
    
    ui->sliderProgression->setRange(0, 100);
    ui->spinBoxProgression->setRange(0, 100);
    ui->spinBoxProgression->setSuffix(" %");
    
    // Set date constraints
    QDate currentDate = QDate::currentDate();
    ui->dateEditDebut->setDate(currentDate);
    ui->dateEditFinEstimee->setDate(currentDate.addMonths(6));
    ui->dateEditDebut->setMinimumDate(currentDate.addYears(-10));
    ui->dateEditFinEstimee->setMinimumDate(currentDate);
}

void ProjetDialog::setupValidators()
{
    // Name validator - no special characters except spaces, hyphens, and apostrophes
    m_nameValidator = new QRegularExpressionValidator(
        QRegularExpression("[a-zA-ZÀ-ÿ0-9\\s\\-']{1,100}"), this);
    ui->lineEditNom->setValidator(m_nameValidator);
    
    // Coordinate validators
    m_coordinateValidator = new QDoubleValidator(-180.0, 180.0, 6, this);
    ui->doubleSpinBoxLatitude->findChild<QLineEdit*>()->setValidator(
        new QDoubleValidator(-90.0, 90.0, 6, this));
    ui->doubleSpinBoxLongitude->findChild<QLineEdit*>()->setValidator(m_coordinateValidator);
    
    // Budget validator
    m_budgetValidator = new QDoubleValidator(0.0, 999999999.99, 2, this);
    m_budgetValidator->setNotation(QDoubleValidator::StandardNotation);
    
    // Progression validator
    m_progressionValidator = new QIntValidator(0, 100, this);
}

void ProjetDialog::setupConnections()
{
    // Form field connections for change detection
    connect(ui->lineEditNom, &QLineEdit::textChanged, this, &ProjetDialog::onFormDataChanged);
    connect(ui->textEditDescription, &QTextEdit::textChanged, this, &ProjetDialog::onFormDataChanged);
    connect(ui->comboBoxCategorie, &QComboBox::currentTextChanged, this, &ProjetDialog::onFormDataChanged);
    connect(ui->comboBoxStatut, &QComboBox::currentTextChanged, this, &ProjetDialog::onFormDataChanged);
    connect(ui->lineEditClient, &QLineEdit::textChanged, this, &ProjetDialog::onFormDataChanged);
    connect(ui->lineEditArchitecte, &QLineEdit::textChanged, this, &ProjetDialog::onFormDataChanged);
    connect(ui->doubleSpinBoxBudget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ProjetDialog::onFormDataChanged);
    connect(ui->doubleSpinBoxSurface, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ProjetDialog::onFormDataChanged);
    connect(ui->spinBoxEtage, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &ProjetDialog::onFormDataChanged);
    connect(ui->comboBoxMateriauPrincipal, &QComboBox::currentTextChanged, this, &ProjetDialog::onFormDataChanged);
    connect(ui->dateEditDebut, &QDateEdit::dateChanged, this, &ProjetDialog::onFormDataChanged);
    connect(ui->dateEditFinEstimee, &QDateEdit::dateChanged, this, &ProjetDialog::onFormDataChanged);
    
    // Location connections
    connect(ui->doubleSpinBoxLatitude, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ProjetDialog::onFormDataChanged);
    connect(ui->doubleSpinBoxLongitude, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ProjetDialog::onFormDataChanged);
    connect(ui->lineEditAdresse, &QLineEdit::textChanged, this, &ProjetDialog::onFormDataChanged);    
    // Progression slider synchronization
    connect(ui->sliderProgression, &QSlider::valueChanged, 
            ui->spinBoxProgression, &QSpinBox::setValue);
    connect(ui->spinBoxProgression, QOverload<int>::of(&QSpinBox::valueChanged), 
            ui->sliderProgression, &QSlider::setValue);
    connect(ui->sliderProgression, &QSlider::valueChanged, 
            this, &ProjetDialog::onFormDataChanged);
    
    // Validation triggers
    connect(ui->lineEditNom, &QLineEdit::textChanged, this, &ProjetDialog::validateForm);
    connect(ui->comboBoxCategorie, &QComboBox::currentTextChanged, this, &ProjetDialog::validateForm);
    connect(ui->dateEditDebut, &QDateEdit::dateChanged, this, &ProjetDialog::validateForm);
    connect(ui->dateEditFinEstimee, &QDateEdit::dateChanged, this, &ProjetDialog::validateForm);
    
    // Special handlers
    connect(ui->dateEditDebut, &QDateEdit::dateChanged, this, &ProjetDialog::updateDurationEstimate);
    connect(ui->comboBoxCategorie, &QComboBox::currentTextChanged, this, &ProjetDialog::updateDurationEstimate);
}

void ProjetDialog::setupCompleters()
{
    if (!m_projetManager) return;
    
    // Setup client completer
    QStringList clients = m_projetManager->getAllClients();
    m_clientModel = new QStandardItemModel(this);
    for (const QString &client : clients) {
        m_clientModel->appendRow(new QStandardItem(client));
    }
    m_clientCompleter = new QCompleter(m_clientModel, this);
    m_clientCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_clientCompleter->setFilterMode(Qt::MatchContains);
    ui->lineEditClient->setCompleter(m_clientCompleter);
    
    // Setup architect completer
    QStringList architectes = m_projetManager->getAllArchitectes();
    m_architecteModel = new QStandardItemModel(this);
    for (const QString &architecte : architectes) {
        m_architecteModel->appendRow(new QStandardItem(architecte));
    }
    m_architecteCompleter = new QCompleter(m_architecteModel, this);
    m_architecteCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_architecteCompleter->setFilterMode(Qt::MatchContains);
    ui->lineEditArchitecte->setCompleter(m_architecteCompleter);
}

void ProjetDialog::populateComboBoxes()
{
    // Populate categories
    ui->comboBoxCategorie->clear();
    ui->comboBoxCategorie->addItems({
        "Résidentiel",
        "Commercial",
        "Industriel",
        "Institutionnel",
        "Infrastructure",
        "Rénovation",
        "Aménagement",
        "Autre"
    });
    
    // Populate status
    ui->comboBoxStatut->clear();
    ui->comboBoxStatut->addItems({
        "En préparation",
        "En cours",
        "En pause",
        "En révision",
        "Terminé",
        "Annulé",
        "Archivé"
    });
    ui->comboBoxStatut->setCurrentText("En préparation");
    
    // Populate materials
    ui->comboBoxMateriauPrincipal->clear();
    ui->comboBoxMateriauPrincipal->addItems({
        "Béton",
        "Acier",
        "Bois",
        "Brique",
        "Pierre",
        "Verre",
        "Aluminium",
        "Mixte",
        "Autre"
    });
}

void ProjetDialog::loadProjectData()
{
    if (m_currentProjet.getId() <= 0) return;
    
    projectToForm(m_currentProjet);
    m_isModified = false;
    updateButtonStates();
}

void ProjetDialog::projectToForm(const Projet &projet)
{
    // Block signals to prevent triggering change detection
    blockSignals(true);
    
    // General information
    ui->lineEditNom->setText(projet.getNom());
    ui->textEditDescription->setPlainText(projet.getDescription());
    
    // Set category
    int categoryIndex = ui->comboBoxCategorie->findText(projet.getCategorie());
    if (categoryIndex >= 0) {
        ui->comboBoxCategorie->setCurrentIndex(categoryIndex);
    }
    
    // Set status
    int statusIndex = ui->comboBoxStatut->findText(projet.getStatut());
    if (statusIndex >= 0) {
        ui->comboBoxStatut->setCurrentIndex(statusIndex);
    }
    
    // Business data
    ui->lineEditClient->setText(projet.getClient());
    ui->lineEditArchitecte->setText(projet.getArchitecte());
    ui->doubleSpinBoxBudget->setValue(projet.getBudget());
    ui->doubleSpinBoxSurface->setValue(projet.getSurface());
    ui->spinBoxEtage->setValue(projet.getEtage());
    
    // Set material
    int materialIndex = ui->comboBoxMateriauPrincipal->findText(projet.getMateriauPrincipal());
    if (materialIndex >= 0) {
        ui->comboBoxMateriauPrincipal->setCurrentIndex(materialIndex);
    }
    
    // Dates
    if (projet.getDateDebut().isValid()) {
        ui->dateEditDebut->setDate(projet.getDateDebut());
    }
    if (projet.getDateFinEstimee().isValid()) {
        ui->dateEditFinEstimee->setDate(projet.getDateFinEstimee());
    }
      // Progression
    ui->sliderProgression->setValue(projet.getProgression());
    ui->spinBoxProgression->setValue(projet.getProgression());
    
    // Location
    Coordinate location = projet.getLocation();
    if (location.isValid()) {
        ui->doubleSpinBoxLatitude->setValue(location.latitude);
        ui->doubleSpinBoxLongitude->setValue(location.longitude);
    }
    ui->lineEditAdresse->setText(location.address);
    
    blockSignals(false);
}

Projet ProjetDialog::formToProject()
{
    Projet projet = m_currentProjet; // Keep existing ID and timestamps
    
    // General information
    projet.setNom(ui->lineEditNom->text().trimmed());
    projet.setDescription(ui->textEditDescription->toPlainText().trimmed());
    projet.setCategorie(ui->comboBoxCategorie->currentText());
    projet.setStatut(ui->comboBoxStatut->currentText());
    
    // Business data
    projet.setClient(ui->lineEditClient->text().trimmed());
    projet.setArchitecte(ui->lineEditArchitecte->text().trimmed());
    projet.setBudget(ui->doubleSpinBoxBudget->value());
    projet.setSurface(ui->doubleSpinBoxSurface->value());
    projet.setEtage(ui->spinBoxEtage->value());
    projet.setMateriauPrincipal(ui->comboBoxMateriauPrincipal->currentText());
    
    // Dates
    projet.setDateDebut(ui->dateEditDebut->date());
    projet.setDateFinEstimee(ui->dateEditFinEstimee->date());
    
    // Progression
    projet.setProgression(ui->spinBoxProgression->value());
    
    // Location
    Coordinate location;
    location.latitude = ui->doubleSpinBoxLatitude->value();
    location.longitude = ui->doubleSpinBoxLongitude->value();
    location.address = ui->lineEditAdresse->text().trimmed();
    projet.setLocation(location);
    
    return projet;
}

void ProjetDialog::accept()
{
    if (!validateRequiredFields()) {
        QString errors = getValidationErrors();
        showValidationMessage("Veuillez corriger les erreurs suivantes :\n\n" + errors);
        return;
    }
    
    // Save the project
    m_currentProjet = formToProject();
    
    if (m_isNewProject) {
        if (m_projetManager && m_projetManager->ajouterProjet(m_currentProjet)) {
            emit projetSaved(m_currentProjet);
            QDialog::accept();
        } else {
            QMessageBox::critical(this, "Erreur", 
                "Impossible de sauvegarder le projet. Veuillez vérifier les données.");
        }
    } else {
        if (m_projetManager && m_projetManager->modifierProjet(m_currentProjet)) {
            emit projetModified(m_currentProjet);
            QDialog::accept();
        } else {
            QMessageBox::critical(this, "Erreur", 
                "Impossible de modifier le projet. Veuillez vérifier les données.");
        }
    }
}

void ProjetDialog::reject()
{
    if (m_isModified && !m_isViewMode) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "Modifications non sauvegardées",
            "Des modifications ont été apportées. Voulez-vous vraiment fermer sans sauvegarder ?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    
    QDialog::reject();
}

// Validation methods
bool ProjetDialog::validateRequiredFields()
{
    clearFieldErrors();
    
    bool isValid = true;
    
    // Validate project name
    if (ui->lineEditNom->text().trimmed().isEmpty()) {
        setFieldError(ui->lineEditNom, true);
        isValid = false;
    }
    
    // Validate category selection
    if (ui->comboBoxCategorie->currentText().isEmpty()) {
        setFieldError(ui->comboBoxCategorie, true);
        isValid = false;
    }
    
    // Validate coordinates if provided
    if (!validateCoordinates()) {
        isValid = false;
    }
    
    // Validate dates
    if (!validateDates()) {
        isValid = false;
    }
    
    // Validate budget
    if (!validateBudget()) {
        isValid = false;
    }
    
    return isValid;
}

bool ProjetDialog::validateCoordinates()
{
    double lat = ui->doubleSpinBoxLatitude->value();
    double lon = ui->doubleSpinBoxLongitude->value();
    
    // If either coordinate is non-zero, both should be valid
    if (lat != 0.0 || lon != 0.0) {
        Coordinate coord(lat, lon);
        if (!coord.isValid()) {
            setFieldError(ui->doubleSpinBoxLatitude, true);
            setFieldError(ui->doubleSpinBoxLongitude, true);
            return false;
        }
    }
    
    return true;
}

bool ProjetDialog::validateDates()
{
    QDate debut = ui->dateEditDebut->date();
    QDate fin = ui->dateEditFinEstimee->date();
    
    if (fin < debut) {
        setFieldError(ui->dateEditFinEstimee, true);
        return false;
    }
    
    return true;
}

bool ProjetDialog::validateBudget()
{
    // Budget can be zero but not negative (already handled by spinbox minimum)
    return true;
}

bool ProjetDialog::validateProgression()
{
    int progression = ui->spinBoxProgression->value();
    return progression >= 0 && progression <= 100;
}

QString ProjetDialog::getValidationErrors()
{
    QStringList errors;
    
    if (ui->lineEditNom->text().trimmed().isEmpty()) {
        errors << "- Le nom du projet est obligatoire";
    }
    
    if (ui->comboBoxCategorie->currentText().isEmpty()) {
        errors << "- La catégorie du projet est obligatoire";
    }
    
    if (!validateCoordinates()) {
        errors << "- Les coordonnées géographiques sont invalides";
    }
    
    if (!validateDates()) {
        errors << "- La date de fin estimée doit être postérieure à la date de début";
    }
    
    return errors.join("\n");
}

// Event handlers
void ProjetDialog::onFormDataChanged()
{
    m_isModified = true;
    updateButtonStates();
}

void ProjetDialog::validateForm()
{
    clearFieldErrors();
    validateRequiredFields();
}

void ProjetDialog::updateProgressionDisplay()
{
    int value = ui->sliderProgression->value();
    ui->spinBoxProgression->setValue(value);
}

void ProjetDialog::updateDurationEstimate()
{
    // This could integrate with a duration estimation service
    // For now, just ensure end date is after start date
    QDate debut = ui->dateEditDebut->date();
    QDate fin = ui->dateEditFinEstimee->date();
    
    if (fin <= debut) {
        // Auto-adjust end date based on category
        QString categorie = ui->comboBoxCategorie->currentText();
        int estimatedDays = 180; // Default 6 months
        
        if (categorie == "Résidentiel") estimatedDays = 365;
        else if (categorie == "Commercial") estimatedDays = 730;
        else if (categorie == "Industriel") estimatedDays = 1095;
        else if (categorie == "Infrastructure") estimatedDays = 1460;
        
        ui->dateEditFinEstimee->setDate(debut.addDays(estimatedDays));
    }
}

void ProjetDialog::showValidationMessage(const QString &message)
{
    QMessageBox::warning(this, "Validation", message);
}

void ProjetDialog::setFieldError(QWidget *widget, bool hasError)
{
    if (hasError) {
        widget->setStyleSheet("border: 2px solid #FF6B6B; background-color: rgba(255, 107, 107, 0.1);");
    } else {
        widget->setStyleSheet("");
    }
}

void ProjetDialog::clearFieldErrors()
{
    // Clear all field error styles
    ui->lineEditNom->setStyleSheet("");
    ui->comboBoxCategorie->setStyleSheet("");
    ui->doubleSpinBoxLatitude->setStyleSheet("");
    ui->doubleSpinBoxLongitude->setStyleSheet("");
    ui->dateEditFinEstimee->setStyleSheet("");
}

void ProjetDialog::resetForm()
{
    // Reset to default values
    ui->lineEditNom->clear();
    ui->textEditDescription->clear();
    ui->comboBoxCategorie->setCurrentIndex(0);
    ui->comboBoxStatut->setCurrentText("En préparation");
    ui->lineEditClient->clear();
    ui->lineEditArchitecte->clear();
    ui->doubleSpinBoxBudget->setValue(0.0);
    ui->doubleSpinBoxSurface->setValue(0.0);
    ui->spinBoxEtage->setValue(0);
    ui->comboBoxMateriauPrincipal->setCurrentIndex(0);    ui->dateEditDebut->setDate(QDate::currentDate());
    ui->dateEditFinEstimee->setDate(QDate::currentDate().addMonths(6));
    ui->sliderProgression->setValue(0);
    ui->spinBoxProgression->setValue(0);
    ui->doubleSpinBoxLatitude->setValue(0.0);
    ui->doubleSpinBoxLongitude->setValue(0.0);
    ui->lineEditAdresse->clear();
    
    m_isModified = false;
    clearFieldErrors();
    updateButtonStates();
}

void ProjetDialog::setFormReadOnly(bool readOnly)
{
    // Set all form fields as read-only
    ui->lineEditNom->setReadOnly(readOnly);
    ui->textEditDescription->setReadOnly(readOnly);
    ui->comboBoxCategorie->setEnabled(!readOnly);
    ui->comboBoxStatut->setEnabled(!readOnly);
    ui->lineEditClient->setReadOnly(readOnly);
    ui->lineEditArchitecte->setReadOnly(readOnly);
    ui->doubleSpinBoxBudget->setReadOnly(readOnly);
    ui->doubleSpinBoxSurface->setReadOnly(readOnly);
    ui->spinBoxEtage->setReadOnly(readOnly);
    ui->comboBoxMateriauPrincipal->setEnabled(!readOnly);    ui->dateEditDebut->setReadOnly(readOnly);
    ui->dateEditFinEstimee->setReadOnly(readOnly);
    ui->sliderProgression->setEnabled(!readOnly);
    ui->spinBoxProgression->setReadOnly(readOnly);
    ui->doubleSpinBoxLatitude->setReadOnly(readOnly);
    ui->doubleSpinBoxLongitude->setReadOnly(readOnly);
    ui->lineEditAdresse->setReadOnly(readOnly);
    ui->pushButtonRechercherLocalisation->setEnabled(!readOnly);
}

void ProjetDialog::updateButtonStates()
{
    // Get individual buttons from the button box
    QPushButton *saveButton = ui->buttonBox->button(QDialogButtonBox::Save);
    QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
    
    if (m_isViewMode) {
        if (saveButton) saveButton->setVisible(false);
        if (cancelButton) cancelButton->setText("Fermer");
    } else {
        if (saveButton) {
            saveButton->setVisible(true);
            saveButton->setEnabled(m_isModified || m_isNewProject);
        }
        if (cancelButton) cancelButton->setText("Annuler");
    }
}

void ProjetDialog::applyArchiFlowStyling()
{
    // Apply ArchiFlow color scheme
    setStyleSheet(R"(
        QDialog {
            background-color: #3D485A;
            color: #E3C6B0;
        }
        
        QTabWidget::pane {
            border: 1px solid #D4B7A1;
            background-color: #3D485A;
        }
        
        QTabWidget::tab-bar {
            alignment: left;
        }
        
        QTabBar::tab {
            background-color: #2A3340;
            color: #D4B7A1;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        
        QTabBar::tab:selected {
            background-color: #3D485A;
            color: #E3C6B0;
            border-bottom: 2px solid #E3C6B0;
        }
        
        QPushButton {
            background-color: #2A3340;
            color: #E3C6B0;
            border: 1px solid #D4B7A1;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: 500;
        }
        
        QPushButton:hover {
            background-color: #E3C6B0;
            color: #3D485A;
        }
        
        QPushButton:pressed {
            background-color: #D4B7A1;
        }
        
        QPushButton:disabled {
            background-color: #2A3340;
            color: #666;
            border-color: #666;
        }
        
        #pushButtonSauvegarder {
            background-color: #4CAF50;
            border-color: #4CAF50;
        }
        
        #pushButtonSauvegarder:hover {
            background-color: #45a049;
        }
        
        #pushButtonSupprimer {
            background-color: #FF6B6B;
            border-color: #FF6B6B;
        }
        
        #pushButtonSupprimer:hover {
            background-color: #ff5252;
        }
    )");
}

// Button event handlers
void ProjetDialog::on_pushButtonSauvegarder_clicked()
{
    accept();
}

void ProjetDialog::on_pushButtonAnnuler_clicked()
{
    reject();
}

void ProjetDialog::on_pushButtonSupprimer_clicked()
{
    if (m_isNewProject) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Confirmer la suppression",
        QString("Êtes-vous sûr de vouloir supprimer le projet '%1' ?\n\nCette action est irréversible.")
            .arg(m_currentProjet.getNom()),
        QMessageBox::Yes | QMessageBox::No);
        
    if (reply == QMessageBox::Yes) {
        if (m_projetManager && m_projetManager->supprimerProjet(m_currentProjet.getId())) {
            emit projetDeleted(m_currentProjet.getId());
            QDialog::accept();
        } else {
            QMessageBox::critical(this, "Erreur", 
                "Impossible de supprimer le projet. Veuillez réessayer.");
        }
    }
}

void ProjetDialog::on_pushButtonReinitialiser_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Réinitialiser le formulaire",
        "Voulez-vous vraiment réinitialiser tous les champs ?\n\nToutes les modifications seront perdues.",
        QMessageBox::Yes | QMessageBox::No);
        
    if (reply == QMessageBox::Yes) {
        if (m_isNewProject) {
            resetForm();
        } else {
            projectToForm(m_originalProjet);
            m_isModified = false;
            updateButtonStates();
        }
    }
}

void ProjetDialog::on_pushButtonRechercherLocalisation_clicked()
{
    // TODO: Implement map dialog for location selection
    // For now, show a simple message
    QMessageBox::information(this, "Localisation", 
        "La fonctionnalité de sélection sur carte sera implémentée dans la prochaine version.");
}

// Auto-connection slots implementation
void ProjetDialog::on_lineEditNom_textChanged(const QString &text)
{
    Q_UNUSED(text)
    onFormDataChanged();
}

void ProjetDialog::on_textEditDescription_textChanged()
{
    onFormDataChanged();
}

void ProjetDialog::on_comboBoxCategorie_currentTextChanged(const QString &text)
{
    Q_UNUSED(text)
    onFormDataChanged();
}

void ProjetDialog::on_comboBoxStatut_currentTextChanged(const QString &text)
{
    Q_UNUSED(text)
    onFormDataChanged();
}

void ProjetDialog::on_lineEditClient_textChanged(const QString &text)
{
    Q_UNUSED(text)
    onFormDataChanged();
}

void ProjetDialog::on_lineEditArchitecte_textChanged(const QString &text)
{
    Q_UNUSED(text)
    onFormDataChanged();
}

void ProjetDialog::on_doubleSpinBoxBudget_valueChanged(double value)
{
    Q_UNUSED(value)
    onFormDataChanged();
}

void ProjetDialog::on_doubleSpinBoxSurface_valueChanged(double value)
{
    Q_UNUSED(value)
    onFormDataChanged();
}

void ProjetDialog::on_spinBoxEtage_valueChanged(int value)
{
    Q_UNUSED(value)
    onFormDataChanged();
}

void ProjetDialog::on_comboBoxMateriauPrincipal_currentTextChanged(const QString &text)
{
    Q_UNUSED(text)
    onFormDataChanged();
}

void ProjetDialog::on_dateEditDebut_dateChanged(const QDate &date)
{
    Q_UNUSED(date)
    onFormDataChanged();
    updateDurationEstimate();
}

void ProjetDialog::on_dateEditFinEstimee_dateChanged(const QDate &date)
{
    Q_UNUSED(date)
    onFormDataChanged();
    updateDurationEstimate();
}

void ProjetDialog::on_sliderProgression_valueChanged(int value)
{
    Q_UNUSED(value)
    updateProgressionDisplay();
    onFormDataChanged();
}

void ProjetDialog::on_spinBoxProgression_valueChanged(int value)
{
    Q_UNUSED(value)
    onFormDataChanged();
}

void ProjetDialog::on_doubleSpinBoxLatitude_valueChanged(double value)
{
    Q_UNUSED(value)
    onFormDataChanged();
}

void ProjetDialog::on_doubleSpinBoxLongitude_valueChanged(double value)
{
    Q_UNUSED(value)
    onFormDataChanged();
}

void ProjetDialog::on_lineEditAdresse_textChanged(const QString &text)
{
    Q_UNUSED(text)
    onFormDataChanged();
}
