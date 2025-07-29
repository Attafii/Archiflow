#ifndef CONTRACTDIALOG_H
#define CONTRACTDIALOG_H

#include "ui/basedialog.h"
#include "../../interfaces/icontractdialog.h"
#include "contractdatabasemanager.h"
#include <QLineEdit>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>

class Contract;

/**
 * @brief The ContractDialog class provides a modern UI for adding/editing contracts
 * 
 * This dialog follows ArchiFlow's design principles with the primary color scheme
 * and provides comprehensive contract data entry with validation.
 * Implements IContractDialog interface for consistent dialog behavior.
 */
class ContractDialog : public BaseDialog, public IContractDialog
{
    Q_OBJECT

public:
    // Legacy methods for backward compatibility
    enum Mode {
        AddMode = CreateMode,
        EditMode = IContractDialog::EditMode
    };

    explicit ContractDialog(QWidget *parent = nullptr);
    explicit ContractDialog(Mode mode, QWidget *parent = nullptr);
    ~ContractDialog();

    // IContractDialog interface implementation
    void setMode(DialogMode mode) override;
    DialogMode getMode() const override;
    void setTitle(const QString &title) override;
    void setContract(Contract *contract) override;
    Contract* getContract() const override;
    void clearForm() override;
    bool validateInput() override;
    bool hasUnsavedChanges() override;
    void resetValidation() override;
    void setReadOnly(bool readOnly) override;
    bool isReadOnly() const override;
    void setFieldsEnabled(bool enabled) override;
    bool saveContract() override;
    void loadContract(const QString &contractId) override;
    void focusFirstField() override;
    void showValidationSummary() override;
    void highlightInvalidFields() override;
    
    // Database operations
    void setDatabaseManager(ContractDatabaseManager *dbManager);

    // Legacy method
    void setMode(Mode mode); // Legacy method    // Dialog state
    Mode mode() const { return m_mode; }
    bool isValid() const;

signals:
    // Dialog operation signals
    void contractSaved(const QString &contractId);
    void contractCancelled();
    void validationChanged(bool isValid);
    void dataChanged();
    void errorOccurred(const QString &error);
    void fieldChanged(const QString &fieldName);

protected:
    void setupUi() override;
    void setupConnections();
    void applyArchiFlowStyling();
    
    // Override BaseDialog methods
    void accept() override;
    void reject() override;

private slots:
    void onAcceptClicked();
    void onRejectClicked();
    void onFieldChanged();
    void validateForm();
    void onStartDateChanged();
    void onEndDateChanged();

private:
    void populateFromContract();
    void updateContractFromFields();
    void resetForm();
    void showValidationErrors(const QStringList &errors);

    // UI Components
    QFormLayout *m_formLayout;
    
    // Input fields
    QLineEdit *m_clientNameEdit;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QDoubleSpinBox *m_valueSpinBox;
    QComboBox *m_statusComboBox;
    QTextEdit *m_descriptionEdit;
    QSpinBox *m_paymentTermsSpinBox;
    QCheckBox *m_nonCompeteCheckBox;
    
    // Action buttons
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
    
    // Status labels
    QLabel *m_validationLabel;
    QLabel *m_daysUntilExpiryLabel;
      // Data
    Contract *m_contract;
    ContractDatabaseManager *m_dbManager;
    Mode m_mode;
    bool m_isModified;
    QStringList m_validationErrors;
};

#endif // CONTRACTDIALOG_H
