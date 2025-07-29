#ifndef SUPPLIERDIALOG_H
#define SUPPLIERDIALOG_H

#include "suppliermodel.h"
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>

/**
 * @brief Dialog for adding/editing supplier information
 */
class SupplierDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        AddMode,
        EditMode
    };

    explicit SupplierDialog(Mode mode, QWidget *parent = nullptr);
    
    void setSupplier(const Supplier &supplier);
    Supplier getSupplier() const;
    
    void clearForm();

private slots:
    void onAccept();
    void onReject();
    void onFieldChanged();

private:
    void setupUI();
    void setupConnections();
    void updateWindowTitle();
    void validateForm();
    bool isValidEmail(const QString &email) const;

    Mode m_mode;
    
    // Main layout
    QVBoxLayout *m_mainLayout;
    
    // Basic Information Group
    QGroupBox *m_basicInfoGroup;
    QFormLayout *m_basicInfoLayout;
    QLineEdit *m_nameEdit;
    QLineEdit *m_contactPersonEdit;
    
    // Contact Information Group
    QGroupBox *m_contactGroup;
    QFormLayout *m_contactLayout;
    QLineEdit *m_emailEdit;
    QLineEdit *m_phoneEdit;
    QLineEdit *m_websiteEdit;
    
    // Address Information Group
    QGroupBox *m_addressGroup;
    QFormLayout *m_addressLayout;
    QLineEdit *m_addressEdit;
    QLineEdit *m_cityEdit;
    QLineEdit *m_countryEdit;
    
    // Additional Information Group
    QGroupBox *m_additionalGroup;
    QVBoxLayout *m_additionalLayout;
    QTextEdit *m_notesEdit;
    QCheckBox *m_activeCheckBox;
    
    // Button layout
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    
    // Status label
    QLabel *m_statusLabel;
};

#endif // SUPPLIERDIALOG_H
