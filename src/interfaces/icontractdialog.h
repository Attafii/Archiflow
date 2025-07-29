#ifndef ICONTRACTDIALOG_H
#define ICONTRACTDIALOG_H

#include <QDialog>
#include <QString>

class Contract;

/**
 * @brief Interface for contract dialog implementations
 * 
 * This interface defines the contract for all contract dialog widgets,
 * ensuring consistent dialog patterns for contract creation and editing.
 */
class IContractDialog
{
public:
    virtual ~IContractDialog() = default;

    // Dialog modes
    enum DialogMode {
        CreateMode,
        EditMode,
        ViewMode,
        DuplicateMode
    };

    // Dialog configuration
    virtual void setMode(DialogMode mode) = 0;
    virtual DialogMode getMode() const = 0;
    virtual void setTitle(const QString &title) = 0;

    // Contract data
    virtual void setContract(Contract *contract) = 0;
    virtual Contract* getContract() const = 0;
    virtual void clearForm() = 0;

    // Validation
    virtual bool validateInput() = 0;
    virtual bool hasUnsavedChanges() = 0;
    virtual void resetValidation() = 0;

    // Form state
    virtual void setReadOnly(bool readOnly) = 0;
    virtual bool isReadOnly() const = 0;
    virtual void setFieldsEnabled(bool enabled) = 0;

    // Data persistence
    virtual bool saveContract() = 0;
    virtual void loadContract(const QString &contractId) = 0;

    // UI helpers
    virtual void focusFirstField() = 0;
    virtual void showValidationSummary() = 0;
    virtual void highlightInvalidFields() = 0;
};

/**
 * @brief Signal definitions for contract dialog events
 * 
 * This class provides signal definitions that contract dialogs should emit
 * for event-driven dialog operations. Dialog implementations should declare
 * these signals in their class definitions.
 */
class IContractDialogSignals
{
public:
    virtual ~IContractDialogSignals() = default;

    // Virtual signal declarations - concrete classes should implement as Qt signals
    virtual void contractSaved(const QString &contractId) = 0;
    virtual void contractCancelled() = 0;
    virtual void validationChanged(bool isValid) = 0;
    virtual void dataChanged() = 0;
    virtual void errorOccurred(const QString &error) = 0;
    virtual void fieldChanged(const QString &fieldName) = 0;
};

#endif // ICONTRACTDIALOG_H
