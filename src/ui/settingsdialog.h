#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QColorDialog>
#include <QFileDialog>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void onApplyClicked();
    void onResetClicked();
    void onBrowseClicked();
    void onColorClicked();

private:
    void setupUI();
    void setupGeneralTab();
    void setupAppearanceTab();
    void setupDatabaseTab();
    void setupMaterialsTab();
    void loadSettings();
    void saveSettings();
    void applyStyles();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // General Tab
    QWidget *m_generalTab;
    QLineEdit *m_companyNameEdit;
    QLineEdit *m_userNameEdit;
    QComboBox *m_languageCombo;
    QCheckBox *m_autoSaveCheck;
    QSpinBox *m_autoSaveIntervalSpin;
    
    // Appearance Tab
    QWidget *m_appearanceTab;
    QComboBox *m_themeCombo;
    QPushButton *m_accentColorButton;
    QColor m_accentColor;
    QCheckBox *m_animationsCheck;
    QSpinBox *m_animationSpeedSpin;
    
    // Database Tab
    QWidget *m_databaseTab;
    QLineEdit *m_databasePathEdit;
    QPushButton *m_browseDatabaseButton;
    QCheckBox *m_autoBackupCheck;
    QSpinBox *m_backupIntervalSpin;
    QLineEdit *m_backupPathEdit;
    QPushButton *m_browseBackupButton;
    
    // Materials Tab
    QWidget *m_materialsTab;
    QSpinBox *m_defaultMinStockSpin;
    QSpinBox *m_defaultMaxStockSpin;
    QSpinBox *m_defaultReorderPointSpin;
    QComboBox *m_defaultUnitCombo;
    QCheckBox *m_lowStockAlertsCheck;
    QCheckBox *m_expirationAlertsCheck;
    
    // Buttons
    QHBoxLayout *m_buttonsLayout;
    QPushButton *m_applyButton;
    QPushButton *m_resetButton;
    QPushButton *m_cancelButton;
    QPushButton *m_okButton;
};

#endif // SETTINGSDIALOG_H
