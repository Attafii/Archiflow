#ifndef INVOICEDIALOG_H
#define INVOICEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QGroupBox>
#include <QTabWidget>
#include <QSplitter>

class Invoice;
class InvoiceItem;
class Client;
class InvoiceDatabaseManager;

/**
 * @brief The InvoiceDialog class provides a comprehensive invoice editing interface
 * 
 * This dialog allows users to create and edit invoices with full line item support,
 * client selection, and automatic calculations.
 */
class InvoiceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InvoiceDialog(QWidget *parent = nullptr);
    explicit InvoiceDialog(Invoice *invoice, QWidget *parent = nullptr);
    ~InvoiceDialog();

    void setInvoice(Invoice *invoice);
    Invoice* invoice() const { return m_invoice; }
    
    void setDatabaseManager(InvoiceDatabaseManager *dbManager);
    void setClients(const QList<Client*> &clients);

    bool isModified() const { return m_isModified; }

protected:
    void accept() override;
    void reject() override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Form handlers
    void onInvoiceNumberChanged();
    void onClientChanged();
    void onInvoiceDateChanged();
    void onDueDateChanged();
    void onStatusChanged();
    void onTaxRateChanged();
    void onNotesChanged();
    void onCurrencyChanged();
    
    // Item management
    void onAddItemClicked();
    void onEditItemClicked();
    void onDeleteItemClicked();
    void onDuplicateItemClicked();
    void onItemChanged(int row, int column);
    void onItemSelectionChanged();
    
    // Client management
    void onSelectClientClicked();
    void onNewClientClicked();
    void onClientInfoChanged();
    
    // Actions
    void onCalculateClicked();
    void onPreviewClicked();
    void onSaveClicked();
    void onCancelClicked();
    
    // Validation
    void validateForm();
    void updateCalculations();

private:
    void setupUI();
    void setupInvoiceTab();
    void setupItemsTab();
    void setupClientTab();
    void setupConnections();
    void populateForm();
    void populateClientCombo();
    void populateItemsTable();
    void updateTotals();
    void addItemToTable(const InvoiceItem *item, int row = -1);
    void updateItemInTable(const InvoiceItem *item, int row);
    void removeItemFromTable(int row);
    void clearForm();
    bool validateInput();
    void markAsModified();
    void applyFormData();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    
    // Invoice tab
    QWidget *m_invoiceTab;
    QFormLayout *m_invoiceForm;
    QLineEdit *m_invoiceNumberEdit;
    QComboBox *m_clientCombo;
    QPushButton *m_selectClientBtn;
    QPushButton *m_newClientBtn;
    QDateEdit *m_invoiceDateEdit;
    QDateEdit *m_dueDateEdit;
    QComboBox *m_statusCombo;
    QComboBox *m_currencyCombo;
    QDoubleSpinBox *m_taxRateSpinBox;
    QTextEdit *m_notesEdit;
    
    // Items tab
    QWidget *m_itemsTab;
    QVBoxLayout *m_itemsLayout;
    QHBoxLayout *m_itemsToolbar;
    QTableWidget *m_itemsTable;
    QPushButton *m_addItemBtn;
    QPushButton *m_editItemBtn;
    QPushButton *m_deleteItemBtn;
    QPushButton *m_duplicateItemBtn;
    
    // Client info tab
    QWidget *m_clientTab;
    QFormLayout *m_clientForm;
    QLineEdit *m_clientNameEdit;
    QLineEdit *m_clientCompanyEdit;
    QTextEdit *m_clientAddressEdit;
    QLineEdit *m_clientEmailEdit;
    QLineEdit *m_clientPhoneEdit;
    QLineEdit *m_clientTaxIdEdit;
    
    // Totals section
    QGroupBox *m_totalsGroup;
    QFormLayout *m_totalsLayout;
    QLabel *m_subtotalLabel;
    QLabel *m_taxAmountLabel;
    QLabel *m_totalAmountLabel;
    
    // Dialog buttons
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_calculateBtn;
    QPushButton *m_previewBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    
    // Data
    Invoice *m_invoice;
    InvoiceDatabaseManager *m_dbManager;
    QList<Client*> m_clients;
    bool m_isModified;
    bool m_isNewInvoice;
    
    // Item table columns
    enum ItemColumnIndex {
        DescriptionColumn = 0,
        QuantityColumn,
        UnitColumn,
        UnitPriceColumn,
        TotalPriceColumn,
        ItemColumnCount
    };
};

#endif // INVOICEDIALOG_H
