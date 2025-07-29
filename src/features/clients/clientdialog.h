#ifndef CLIENTCONTACTDIALOG_H
#define CLIENTCONTACTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>

class ClientContact;
class ClientDatabaseManager;
class MapboxHandler;
class ClientAIAssistant;

/**
 * @brief The ClientContactDialog class provides a comprehensive client editing interface
 * 
 * This dialog allows users to create and edit clients with full form validation,
 * address geocoding, and coordinate input.
 */
class ClientContactDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientContactDialog(QWidget *parent = nullptr);
    explicit ClientContactDialog(ClientContact *client, QWidget *parent = nullptr);
    ~ClientContactDialog();

    void setClient(ClientContact *client);
    ClientContact* client() const { return m_client; }
    
    void setDatabaseManager(ClientDatabaseManager *dbManager);

    bool isModified() const { return m_isModified; }

protected:
    void accept() override;
    void reject() override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFieldChanged();
    void onGeocodeAddress();
    void onValidateEmail();
    void onClearCoordinates();
    void onOpenAIAssistant();

private:
    void setupUI();
    void setupConnections();
    void populateForm();
    void clearForm();
    bool validateInput();
    void applyFormData();
    void markAsModified();

    // UI Components
    QVBoxLayout *m_mainLayout;
    
    // Basic information group
    QGroupBox *m_basicGroup;
    QFormLayout *m_basicLayout;
    QLineEdit *m_nameEdit;
    QLineEdit *m_companyEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_phoneEdit;
    
    // Address group
    QGroupBox *m_addressGroup;
    QFormLayout *m_addressLayout;
    QLineEdit *m_streetEdit;
    QLineEdit *m_cityEdit;
    QLineEdit *m_stateEdit;
    QLineEdit *m_zipcodeEdit;
    QLineEdit *m_countryEdit;
    QPushButton *m_geocodeBtn;
    
    // Coordinates group
    QGroupBox *m_coordinatesGroup;
    QFormLayout *m_coordinatesLayout;
    QDoubleSpinBox *m_latitudeSpinBox;
    QDoubleSpinBox *m_longitudeSpinBox;
    QPushButton *m_clearCoordsBtn;
    
    // Notes group
    QGroupBox *m_notesGroup;
    QVBoxLayout *m_notesLayout;
    QTextEdit *m_notesEdit;
    
    // Timestamps (read-only)
    QGroupBox *m_timestampsGroup;
    QFormLayout *m_timestampsLayout;
    QLabel *m_createdAtLabel;
    QLabel *m_updatedAtLabel;      // Buttons
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_aiAssistantBtn;
    
    // Data
    ClientContact *m_client;
    ClientDatabaseManager *m_dbManager;
    MapboxHandler *m_mapHandler;
    ClientAIAssistant *m_aiAssistant;
    bool m_isModified;
    bool m_isNewClient;
};

#endif // CLIENTDIALOG_H
