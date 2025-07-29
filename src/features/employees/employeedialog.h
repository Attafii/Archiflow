#ifndef EMPLOYEEDIALOG_H
#define EMPLOYEEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QTabWidget>

class Employee;
class EmployeeDatabaseManager;
class MapboxHandler;

/**
 * @brief The EmployeeDialog class provides a comprehensive employee editing interface
 * 
 * This dialog allows users to create and edit employees with full form validation,
 * role and status management, and tabbed organization.
 */
class EmployeeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeDialog(QWidget *parent = nullptr);
    explicit EmployeeDialog(Employee *employee, QWidget *parent = nullptr);
    ~EmployeeDialog();    void setEmployee(Employee *employee);
    Employee* employee() const { return m_employee; }
    
    void setDatabaseManager(EmployeeDatabaseManager *dbManager);
    void setReadOnly(bool readOnly);

    bool isModified() const { return m_isModified; }

protected:
    void accept() override;
    void reject() override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFieldChanged();
    void onGenerateCin();
    void onValidateEmail();
    void onRoleChanged();
    void onStatusChanged();
    void onOpenAIAssistant();
    void onGeocodeAddress();
    void onShowMap();
    void onGeocodeCompleted(const QString &address, double lat, double lng);
    void onGeocodeFailed(const QString &error);

private:
    void setupUI();
    void setupConnections();
    void setupBasicInfoTab();
    void setupEmploymentTab();
    void setupContactTab();
    void setupAdditionalTab();
    void populateForm();
    void clearForm();
    bool validateInput();
    void applyFormData();
    void markAsModified();

    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Basic Info Tab
    QWidget *m_basicInfoTab;
    QFormLayout *m_basicInfoLayout;
    QLineEdit *m_cinEdit;
    QPushButton *m_generateCinBtn;
    QLineEdit *m_firstNameEdit;
    QLineEdit *m_lastNameEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_phoneEdit;
    
    // Employment Tab
    QWidget *m_employmentTab;
    QFormLayout *m_employmentLayout;
    QLineEdit *m_positionEdit;
    QComboBox *m_roleCombo;
    QLineEdit *m_departmentEdit;
    QDateTimeEdit *m_hireDateEdit;
    QComboBox *m_statusCombo;
    QCheckBox *m_isPresentCheck;
    QDoubleSpinBox *m_salarySpinBox;
    
    // Contact Tab
    QWidget *m_contactTab;
    QFormLayout *m_contactLayout;
    QTextEdit *m_addressEdit;
    QPushButton *m_geocodeBtn;
    QPushButton *m_showMapBtn;
    QLineEdit *m_emergencyContactEdit;
    QLineEdit *m_emergencyPhoneEdit;
    
    // Additional Tab
    QWidget *m_additionalTab;
    QVBoxLayout *m_additionalLayout;
    QTextEdit *m_notesEdit;
    QGroupBox *m_timestampsGroup;
    QFormLayout *m_timestampsLayout;
    QLabel *m_createdAtLabel;
    QLabel *m_updatedAtLabel;
    
    // Buttons
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_aiAssistantBtn;
    
    // Data
    Employee *m_employee;
    EmployeeDatabaseManager *m_dbManager;
    bool m_isModified;
    bool m_isNewEmployee;

    // Map Integration
    MapboxHandler *m_mapHandler;
};

#endif // EMPLOYEEDIALOG_H
