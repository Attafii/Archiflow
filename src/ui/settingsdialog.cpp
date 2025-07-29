#include "settingsdialog.h"
#include "utils/stylemanager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QApplication>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_accentColor("#007BFF")
{
    setupUI();
    loadSettings();
    applyStyles();
}

void SettingsDialog::setupUI()
{
    setWindowTitle("Settings");
    setModal(true);
    resize(600, 500);
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // Create tab widget
    m_tabWidget = new QTabWidget();
    
    setupGeneralTab();
    setupAppearanceTab();
    setupDatabaseTab();
    setupMaterialsTab();
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Buttons
    m_buttonsLayout = new QHBoxLayout();
    m_buttonsLayout->setSpacing(10);
    
    m_resetButton = new QPushButton("Reset to Defaults");
    m_applyButton = new QPushButton("Apply");
    m_cancelButton = new QPushButton("Cancel");
    m_okButton = new QPushButton("OK");
    
    m_resetButton->setObjectName("warningButton");
    m_applyButton->setObjectName("primaryButton");
    m_cancelButton->setObjectName("secondaryButton");
    m_okButton->setObjectName("primaryButton");
    
    m_buttonsLayout->addWidget(m_resetButton);
    m_buttonsLayout->addStretch();
    m_buttonsLayout->addWidget(m_applyButton);
    m_buttonsLayout->addWidget(m_cancelButton);
    m_buttonsLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(m_buttonsLayout);
    
    // Connect signals
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_okButton, &QPushButton::clicked, [this]() {
        saveSettings();
        accept();
    });
}

void SettingsDialog::setupGeneralTab()
{
    m_generalTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_generalTab);
    layout->setSpacing(20);
    
    // Company Information
    QGroupBox *companyGroup = new QGroupBox("Company Information");
    QGridLayout *companyLayout = new QGridLayout(companyGroup);
    companyLayout->setSpacing(10);
    
    companyLayout->addWidget(new QLabel("Company Name:"), 0, 0);
    m_companyNameEdit = new QLineEdit();
    m_companyNameEdit->setPlaceholderText("Enter your company name");
    companyLayout->addWidget(m_companyNameEdit, 0, 1);
    
    companyLayout->addWidget(new QLabel("User Name:"), 1, 0);
    m_userNameEdit = new QLineEdit();
    m_userNameEdit->setPlaceholderText("Enter your name");
    companyLayout->addWidget(m_userNameEdit, 1, 1);
    
    layout->addWidget(companyGroup);
    
    // Application Settings
    QGroupBox *appGroup = new QGroupBox("Application Settings");
    QGridLayout *appLayout = new QGridLayout(appGroup);
    appLayout->setSpacing(10);
    
    appLayout->addWidget(new QLabel("Language:"), 0, 0);
    m_languageCombo = new QComboBox();
    m_languageCombo->addItems({"English", "French", "Spanish", "German", "Arabic"});
    appLayout->addWidget(m_languageCombo, 0, 1);
    
    m_autoSaveCheck = new QCheckBox("Auto-save changes");
    appLayout->addWidget(m_autoSaveCheck, 1, 0, 1, 2);
    
    appLayout->addWidget(new QLabel("Auto-save interval (minutes):"), 2, 0);
    m_autoSaveIntervalSpin = new QSpinBox();
    m_autoSaveIntervalSpin->setRange(1, 60);
    m_autoSaveIntervalSpin->setValue(5);
    appLayout->addWidget(m_autoSaveIntervalSpin, 2, 1);
    
    layout->addWidget(appGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_generalTab, "General");
}

void SettingsDialog::setupAppearanceTab()
{
    m_appearanceTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_appearanceTab);
    layout->setSpacing(20);
    
    // Theme Settings
    QGroupBox *themeGroup = new QGroupBox("Theme");
    QGridLayout *themeLayout = new QGridLayout(themeGroup);
    themeLayout->setSpacing(10);
    
    themeLayout->addWidget(new QLabel("Theme:"), 0, 0);
    m_themeCombo = new QComboBox();
    m_themeCombo->addItems({"Light", "Dark", "Auto"});
    themeLayout->addWidget(m_themeCombo, 0, 1);
    
    themeLayout->addWidget(new QLabel("Accent Color:"), 1, 0);
    m_accentColorButton = new QPushButton();
    m_accentColorButton->setFixedHeight(30);
    m_accentColorButton->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 4px;").arg(m_accentColor.name()));
    connect(m_accentColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorClicked);
    themeLayout->addWidget(m_accentColorButton, 1, 1);
    
    layout->addWidget(themeGroup);
    
    // Animation Settings
    QGroupBox *animationGroup = new QGroupBox("Animations");
    QGridLayout *animationLayout = new QGridLayout(animationGroup);
    animationLayout->setSpacing(10);
    
    m_animationsCheck = new QCheckBox("Enable animations");
    m_animationsCheck->setChecked(true);
    animationLayout->addWidget(m_animationsCheck, 0, 0, 1, 2);
    
    animationLayout->addWidget(new QLabel("Animation speed:"), 1, 0);
    m_animationSpeedSpin = new QSpinBox();
    m_animationSpeedSpin->setRange(50, 200);
    m_animationSpeedSpin->setValue(100);
    m_animationSpeedSpin->setSuffix("%");
    animationLayout->addWidget(m_animationSpeedSpin, 1, 1);
    
    layout->addWidget(animationGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_appearanceTab, "Appearance");
}

void SettingsDialog::setupDatabaseTab()
{
    m_databaseTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_databaseTab);
    layout->setSpacing(20);
    
    // Database Settings
    QGroupBox *dbGroup = new QGroupBox("Database");
    QGridLayout *dbLayout = new QGridLayout(dbGroup);
    dbLayout->setSpacing(10);
    
    dbLayout->addWidget(new QLabel("Database Path:"), 0, 0);
    QHBoxLayout *dbPathLayout = new QHBoxLayout();
    m_databasePathEdit = new QLineEdit();
    m_databasePathEdit->setPlaceholderText("Database file path");
    m_browseDatabaseButton = new QPushButton("Browse...");
    connect(m_browseDatabaseButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseClicked);
    dbPathLayout->addWidget(m_databasePathEdit);
    dbPathLayout->addWidget(m_browseDatabaseButton);
    dbLayout->addLayout(dbPathLayout, 0, 1);
    
    layout->addWidget(dbGroup);
    
    // Backup Settings
    QGroupBox *backupGroup = new QGroupBox("Backup");
    QGridLayout *backupLayout = new QGridLayout(backupGroup);
    backupLayout->setSpacing(10);
    
    m_autoBackupCheck = new QCheckBox("Enable automatic backups");
    backupLayout->addWidget(m_autoBackupCheck, 0, 0, 1, 2);
    
    backupLayout->addWidget(new QLabel("Backup interval (days):"), 1, 0);
    m_backupIntervalSpin = new QSpinBox();
    m_backupIntervalSpin->setRange(1, 30);
    m_backupIntervalSpin->setValue(7);
    backupLayout->addWidget(m_backupIntervalSpin, 1, 1);
    
    backupLayout->addWidget(new QLabel("Backup Path:"), 2, 0);
    QHBoxLayout *backupPathLayout = new QHBoxLayout();
    m_backupPathEdit = new QLineEdit();
    m_backupPathEdit->setPlaceholderText("Backup directory path");
    m_browseBackupButton = new QPushButton("Browse...");
    backupPathLayout->addWidget(m_backupPathEdit);
    backupPathLayout->addWidget(m_browseBackupButton);
    backupLayout->addLayout(backupPathLayout, 2, 1);
    
    layout->addWidget(backupGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_databaseTab, "Database");
}

void SettingsDialog::setupMaterialsTab()
{
    m_materialsTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_materialsTab);
    layout->setSpacing(20);
    
    // Default Values
    QGroupBox *defaultsGroup = new QGroupBox("Default Values for New Materials");
    QGridLayout *defaultsLayout = new QGridLayout(defaultsGroup);
    defaultsLayout->setSpacing(10);
    
    defaultsLayout->addWidget(new QLabel("Default Minimum Stock:"), 0, 0);
    m_defaultMinStockSpin = new QSpinBox();
    m_defaultMinStockSpin->setRange(0, 10000);
    m_defaultMinStockSpin->setValue(10);
    defaultsLayout->addWidget(m_defaultMinStockSpin, 0, 1);
    
    defaultsLayout->addWidget(new QLabel("Default Maximum Stock:"), 1, 0);
    m_defaultMaxStockSpin = new QSpinBox();
    m_defaultMaxStockSpin->setRange(1, 100000);
    m_defaultMaxStockSpin->setValue(1000);
    defaultsLayout->addWidget(m_defaultMaxStockSpin, 1, 1);
    
    defaultsLayout->addWidget(new QLabel("Default Reorder Point:"), 2, 0);
    m_defaultReorderPointSpin = new QSpinBox();
    m_defaultReorderPointSpin->setRange(0, 1000);
    m_defaultReorderPointSpin->setValue(20);
    defaultsLayout->addWidget(m_defaultReorderPointSpin, 2, 1);
    
    defaultsLayout->addWidget(new QLabel("Default Unit:"), 3, 0);
    m_defaultUnitCombo = new QComboBox();
    m_defaultUnitCombo->addItems({"pcs", "kg", "m", "l", "box", "pack"});
    defaultsLayout->addWidget(m_defaultUnitCombo, 3, 1);
    
    layout->addWidget(defaultsGroup);
    
    // Alerts
    QGroupBox *alertsGroup = new QGroupBox("Alerts");
    QVBoxLayout *alertsLayout = new QVBoxLayout(alertsGroup);
    
    m_lowStockAlertsCheck = new QCheckBox("Enable low stock alerts");
    m_lowStockAlertsCheck->setChecked(true);
    alertsLayout->addWidget(m_lowStockAlertsCheck);
    
    m_expirationAlertsCheck = new QCheckBox("Enable expiration alerts");
    m_expirationAlertsCheck->setChecked(true);
    alertsLayout->addWidget(m_expirationAlertsCheck);
    
    layout->addWidget(alertsGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_materialsTab, "Materials");
}

void SettingsDialog::loadSettings()
{
    QSettings settings;
    
    // General settings
    m_companyNameEdit->setText(settings.value("company/name", "").toString());
    m_userNameEdit->setText(settings.value("user/name", "User").toString());
    m_languageCombo->setCurrentText(settings.value("app/language", "English").toString());
    m_autoSaveCheck->setChecked(settings.value("app/autoSave", true).toBool());
    m_autoSaveIntervalSpin->setValue(settings.value("app/autoSaveInterval", 5).toInt());
    
    // Appearance settings
    m_themeCombo->setCurrentText(settings.value("appearance/theme", "Light").toString());
    m_accentColor = QColor(settings.value("appearance/accentColor", "#007BFF").toString());
    m_accentColorButton->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 4px;").arg(m_accentColor.name()));
    m_animationsCheck->setChecked(settings.value("appearance/animations", true).toBool());
    m_animationSpeedSpin->setValue(settings.value("appearance/animationSpeed", 100).toInt());
    
    // Database settings
    QString defaultDbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/archiflow.db";
    m_databasePathEdit->setText(settings.value("database/path", defaultDbPath).toString());
    m_autoBackupCheck->setChecked(settings.value("backup/enabled", true).toBool());
    m_backupIntervalSpin->setValue(settings.value("backup/interval", 7).toInt());
    QString defaultBackupPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/ArchiFlow Backups";
    m_backupPathEdit->setText(settings.value("backup/path", defaultBackupPath).toString());
    
    // Materials settings
    m_defaultMinStockSpin->setValue(settings.value("materials/defaultMinStock", 10).toInt());
    m_defaultMaxStockSpin->setValue(settings.value("materials/defaultMaxStock", 1000).toInt());
    m_defaultReorderPointSpin->setValue(settings.value("materials/defaultReorderPoint", 20).toInt());
    m_defaultUnitCombo->setCurrentText(settings.value("materials/defaultUnit", "pcs").toString());
    m_lowStockAlertsCheck->setChecked(settings.value("materials/lowStockAlerts", true).toBool());
    m_expirationAlertsCheck->setChecked(settings.value("materials/expirationAlerts", true).toBool());
}

void SettingsDialog::saveSettings()
{
    QSettings settings;
    
    // General settings
    settings.setValue("company/name", m_companyNameEdit->text());
    settings.setValue("user/name", m_userNameEdit->text());
    settings.setValue("app/language", m_languageCombo->currentText());
    settings.setValue("app/autoSave", m_autoSaveCheck->isChecked());
    settings.setValue("app/autoSaveInterval", m_autoSaveIntervalSpin->value());
    
    // Appearance settings
    settings.setValue("appearance/theme", m_themeCombo->currentText());
    settings.setValue("appearance/accentColor", m_accentColor.name());
    settings.setValue("appearance/animations", m_animationsCheck->isChecked());
    settings.setValue("appearance/animationSpeed", m_animationSpeedSpin->value());
    
    // Database settings
    settings.setValue("database/path", m_databasePathEdit->text());
    settings.setValue("backup/enabled", m_autoBackupCheck->isChecked());
    settings.setValue("backup/interval", m_backupIntervalSpin->value());
    settings.setValue("backup/path", m_backupPathEdit->text());
    
    // Materials settings
    settings.setValue("materials/defaultMinStock", m_defaultMinStockSpin->value());
    settings.setValue("materials/defaultMaxStock", m_defaultMaxStockSpin->value());
    settings.setValue("materials/defaultReorderPoint", m_defaultReorderPointSpin->value());
    settings.setValue("materials/defaultUnit", m_defaultUnitCombo->currentText());
    settings.setValue("materials/lowStockAlerts", m_lowStockAlertsCheck->isChecked());
    settings.setValue("materials/expirationAlerts", m_expirationAlertsCheck->isChecked());
}

void SettingsDialog::onApplyClicked()
{
    saveSettings();
    QMessageBox::information(this, "Settings", "Settings have been applied successfully.");
}

void SettingsDialog::onResetClicked()
{
    int ret = QMessageBox::question(this, "Reset Settings", 
                                   "Are you sure you want to reset all settings to their default values?",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        QSettings settings;
        settings.clear();
        loadSettings();
        QMessageBox::information(this, "Settings", "Settings have been reset to defaults.");
    }
}

void SettingsDialog::onBrowseClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button == m_browseDatabaseButton) {
        QString fileName = QFileDialog::getSaveFileName(this, "Database File", 
                                                       m_databasePathEdit->text(),
                                                       "SQLite Database (*.db)");
        if (!fileName.isEmpty()) {
            m_databasePathEdit->setText(fileName);
        }
    } else if (button == m_browseBackupButton) {
        QString dirName = QFileDialog::getExistingDirectory(this, "Backup Directory",
                                                           m_backupPathEdit->text());
        if (!dirName.isEmpty()) {
            m_backupPathEdit->setText(dirName);
        }
    }
}

void SettingsDialog::onColorClicked()
{
    QColor color = QColorDialog::getColor(m_accentColor, this, "Choose Accent Color");
    if (color.isValid()) {
        m_accentColor = color;
        m_accentColorButton->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 4px;").arg(color.name()));
    }
}

void SettingsDialog::applyStyles()
{
    StyleManager::applyMaterialDesign(this);
    
    setStyleSheet(styleSheet() + R"(
        QGroupBox {
            font-weight: 600;
            font-size: 14px;
            color: #495057;
            border: 1px solid #E9ECEF;
            border-radius: 8px;
            margin-top: 15px;
            padding-top: 10px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            background-color: white;
        }
        
        QTabWidget::pane {
            border: 1px solid #E9ECEF;
            border-radius: 8px;
            background-color: white;
        }
        
        QTabBar::tab {
            background-color: #F8F9FA;
            border: 1px solid #E9ECEF;
            border-bottom: none;
            padding: 8px 16px;
            margin-right: 2px;
        }
        
        QTabBar::tab:selected {
            background-color: white;
            border-top: 2px solid #007BFF;
        }
        
        QTabBar::tab:hover {
            background-color: #E9ECEF;
        }
        
        #primaryButton {
            background-color: #007BFF;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: 600;
        }
        
        #primaryButton:hover {
            background-color: #0056B3;
        }
        
        #secondaryButton {
            background-color: #6C757D;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: 600;
        }
        
        #secondaryButton:hover {
            background-color: #545B62;
        }
        
        #warningButton {
            background-color: #FFC107;
            color: #000;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: 600;
        }
        
        #warningButton:hover {
            background-color: #E0A800;
        }
    )");
}
