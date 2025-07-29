#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>

class Client;
class InvoiceDatabaseManager;

/**
 * @brief The ClientDialog class provides a client editing interface
 * 
 * This dialog allows users to create and edit client information
 * for invoice management.
 */
class ClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientDialog(QWidget *parent = nullptr);
    explicit ClientDialog(Client *client, QWidget *parent = nullptr);
    ~ClientDialog();

    void setClient(Client *client);
    Client* client() const { return m_client; }
    
    void setDatabaseManager(InvoiceDatabaseManager *dbManager);
    bool isModified() const { return m_isModified; }

protected:
    void accept() override;
    void reject() override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Form handlers
    void onNameChanged();
    void onCompanyChanged();
    void onAddressChanged();
    void onEmailChanged();
    void onPhoneChanged();
    void onTaxIdChanged();
    void onNotesChanged();
    
    // Actions
    void onSaveClicked();
    void onCancelClicked();
    
    // Validation
    void validateForm();

private:
    void setupUI();
    void setupConnections();
    void populateForm();
    void clearForm();
    bool validateInput();
    void markAsModified();
    void applyFormData();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Basic info tab
    QWidget *m_basicTab;
    QFormLayout *m_basicForm;
    QLineEdit *m_nameEdit;
    QLineEdit *m_companyEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_phoneEdit;
    
    // Address tab
    QWidget *m_addressTab;
    QFormLayout *m_addressForm;
    QTextEdit *m_addressEdit;
    QLineEdit *m_taxIdEdit;
    
    // Notes tab
    QWidget *m_notesTab;
    QVBoxLayout *m_notesLayout;
    QTextEdit *m_notesEdit;
    
    // Dialog buttons
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    
    // Data
    Client *m_client;
    InvoiceDatabaseManager *m_dbManager;
    bool m_isModified;
    bool m_isNewClient;
};

#endif // CLIENTDIALOG_H
