#include "invoicedialog.h"
#include "invoice.h"
#include "invoiceitem.h"
#include "client.h"
#include "invoicedatabasemanager.h"
#include "clientdialog.h"

#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QHeaderView>
#include <QSplitter>
#include <QGridLayout>
#include <QFrame>
#include <QStyle>
#include <QInputDialog>

InvoiceDialog::InvoiceDialog(QWidget *parent)
    : QDialog(parent)
    , m_invoice(nullptr)
    , m_dbManager(nullptr)
    , m_isModified(false)
    , m_isNewInvoice(true)
{
    setupUI();
    setupConnections();
    setWindowTitle(tr("New Invoice"));
    resize(800, 600);
}

InvoiceDialog::InvoiceDialog(Invoice *invoice, QWidget *parent)
    : QDialog(parent)
    , m_invoice(invoice)
    , m_dbManager(nullptr)
    , m_isModified(false)
    , m_isNewInvoice(false)
{
    setupUI();
    setupConnections();
    populateForm();
    setWindowTitle(tr("Edit Invoice - %1").arg(invoice ? invoice->invoiceNumber() : ""));
    resize(800, 600);
}

InvoiceDialog::~InvoiceDialog()
{
    // Cleanup if needed
}

void InvoiceDialog::setInvoice(Invoice *invoice)
{
    m_invoice = invoice;
    m_isNewInvoice = (invoice == nullptr);
    populateForm();
    setWindowTitle(tr("Edit Invoice - %1").arg(invoice ? invoice->invoiceNumber() : "New Invoice"));
}

void InvoiceDialog::setDatabaseManager(InvoiceDatabaseManager *dbManager)
{
    m_dbManager = dbManager;
    if (m_dbManager) {
        m_clients = m_dbManager->getAllClients();
        populateClientCombo();
    }
}

void InvoiceDialog::setClients(const QList<Client*> &clients)
{
    m_clients = clients;
    populateClientCombo();
}

void InvoiceDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    
    setupInvoiceTab();
    setupItemsTab();
    setupClientTab();
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // Totals section
    m_totalsGroup = new QGroupBox(tr("Totals"), this);
    m_totalsLayout = new QFormLayout(m_totalsGroup);
    
    m_subtotalLabel = new QLabel("$0.00");
    m_taxAmountLabel = new QLabel("$0.00");
    m_totalAmountLabel = new QLabel("$0.00");
    
    m_subtotalLabel->setStyleSheet("font-weight: bold;");
    m_taxAmountLabel->setStyleSheet("font-weight: bold;");
    m_totalAmountLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #0066CC;");
    
    m_totalsLayout->addRow(tr("Subtotal:"), m_subtotalLabel);
    m_totalsLayout->addRow(tr("Tax Amount:"), m_taxAmountLabel);
    m_totalsLayout->addRow(tr("Total Amount:"), m_totalAmountLabel);
    
    m_mainLayout->addWidget(m_totalsGroup);
    
    // Dialog buttons
    m_buttonLayout = new QHBoxLayout();
    
    m_calculateBtn = new QPushButton(tr("Calculate"), this);
    m_previewBtn = new QPushButton(tr("Preview PDF"), this);
    m_saveBtn = new QPushButton(tr("Save"), this);
    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    
    m_saveBtn->setDefault(true);
    m_saveBtn->setStyleSheet("QPushButton { background-color: #0066CC; color: white; font-weight: bold; }");
    
    m_buttonLayout->addWidget(m_calculateBtn);
    m_buttonLayout->addWidget(m_previewBtn);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_saveBtn);
    m_buttonLayout->addWidget(m_cancelBtn);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void InvoiceDialog::setupInvoiceTab()
{
    m_invoiceTab = new QWidget();
    m_invoiceForm = new QFormLayout(m_invoiceTab);
    
    // Invoice number
    m_invoiceNumberEdit = new QLineEdit();
    m_invoiceForm->addRow(tr("Invoice Number:"), m_invoiceNumberEdit);
    
    // Client selection
    QHBoxLayout *clientLayout = new QHBoxLayout();
    m_clientCombo = new QComboBox();
    m_clientCombo->setEditable(false);
    m_selectClientBtn = new QPushButton(tr("Select"));
    m_newClientBtn = new QPushButton(tr("New"));
    
    clientLayout->addWidget(m_clientCombo, 1);
    clientLayout->addWidget(m_selectClientBtn);
    clientLayout->addWidget(m_newClientBtn);
    
    m_invoiceForm->addRow(tr("Client:"), clientLayout);
    
    // Dates
    m_invoiceDateEdit = new QDateEdit();
    m_invoiceDateEdit->setDate(QDate::currentDate());
    m_invoiceDateEdit->setCalendarPopup(true);
    m_invoiceForm->addRow(tr("Invoice Date:"), m_invoiceDateEdit);
    
    m_dueDateEdit = new QDateEdit();
    m_dueDateEdit->setDate(QDate::currentDate().addDays(30));
    m_dueDateEdit->setCalendarPopup(true);
    m_invoiceForm->addRow(tr("Due Date:"), m_dueDateEdit);
    
    // Status
    m_statusCombo = new QComboBox();
    m_statusCombo->addItems({tr("Draft"), tr("Sent"), tr("Paid"), tr("Overdue"), tr("Cancelled")});
    m_invoiceForm->addRow(tr("Status:"), m_statusCombo);
    
    // Currency
    m_currencyCombo = new QComboBox();
    m_currencyCombo->addItems({"USD", "EUR", "GBP", "CAD", "AUD"});
    m_invoiceForm->addRow(tr("Currency:"), m_currencyCombo);
    
    // Tax rate
    m_taxRateSpinBox = new QDoubleSpinBox();
    m_taxRateSpinBox->setRange(0.0, 100.0);
    m_taxRateSpinBox->setValue(0.0);
    m_taxRateSpinBox->setSuffix("%");
    m_taxRateSpinBox->setDecimals(2);
    m_invoiceForm->addRow(tr("Tax Rate:"), m_taxRateSpinBox);
    
    // Notes
    m_notesEdit = new QTextEdit();
    m_notesEdit->setMaximumHeight(100);
    m_invoiceForm->addRow(tr("Notes:"), m_notesEdit);
    
    m_tabWidget->addTab(m_invoiceTab, tr("Invoice Details"));
}

void InvoiceDialog::setupItemsTab()
{
    m_itemsTab = new QWidget();
    m_itemsLayout = new QVBoxLayout(m_itemsTab);
    
    // Toolbar
    m_itemsToolbar = new QHBoxLayout();
    m_addItemBtn = new QPushButton(tr("Add Item"));
    m_editItemBtn = new QPushButton(tr("Edit Item"));
    m_deleteItemBtn = new QPushButton(tr("Delete Item"));
    m_duplicateItemBtn = new QPushButton(tr("Duplicate Item"));
    
    m_addItemBtn->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    m_editItemBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_deleteItemBtn->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    m_duplicateItemBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    
    m_itemsToolbar->addWidget(m_addItemBtn);
    m_itemsToolbar->addWidget(m_editItemBtn);
    m_itemsToolbar->addWidget(m_deleteItemBtn);
    m_itemsToolbar->addWidget(m_duplicateItemBtn);
    m_itemsToolbar->addStretch();
    
    m_itemsLayout->addLayout(m_itemsToolbar);
    
    // Items table
    m_itemsTable = new QTableWidget();
    m_itemsTable->setColumnCount(ItemColumnCount);
    
    QStringList headers;
    headers << tr("Description") << tr("Quantity") << tr("Unit") << tr("Unit Price") << tr("Total Price");
    m_itemsTable->setHorizontalHeaderLabels(headers);
    
    m_itemsTable->horizontalHeader()->setStretchLastSection(false);
    m_itemsTable->horizontalHeader()->setSectionResizeMode(DescriptionColumn, QHeaderView::Stretch);
    m_itemsTable->horizontalHeader()->setSectionResizeMode(QuantityColumn, QHeaderView::ResizeToContents);
    m_itemsTable->horizontalHeader()->setSectionResizeMode(UnitColumn, QHeaderView::ResizeToContents);
    m_itemsTable->horizontalHeader()->setSectionResizeMode(UnitPriceColumn, QHeaderView::ResizeToContents);
    m_itemsTable->horizontalHeader()->setSectionResizeMode(TotalPriceColumn, QHeaderView::ResizeToContents);
    
    m_itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_itemsTable->setAlternatingRowColors(true);
    
    m_itemsLayout->addWidget(m_itemsTable);
    
    m_tabWidget->addTab(m_itemsTab, tr("Line Items"));
}

void InvoiceDialog::setupClientTab()
{
    m_clientTab = new QWidget();
    m_clientForm = new QFormLayout(m_clientTab);
    
    m_clientNameEdit = new QLineEdit();
    m_clientNameEdit->setReadOnly(true);
    m_clientForm->addRow(tr("Name:"), m_clientNameEdit);
    
    m_clientCompanyEdit = new QLineEdit();
    m_clientCompanyEdit->setReadOnly(true);
    m_clientForm->addRow(tr("Company:"), m_clientCompanyEdit);
    
    m_clientAddressEdit = new QTextEdit();
    m_clientAddressEdit->setReadOnly(true);
    m_clientAddressEdit->setMaximumHeight(80);
    m_clientForm->addRow(tr("Address:"), m_clientAddressEdit);
    
    m_clientEmailEdit = new QLineEdit();
    m_clientEmailEdit->setReadOnly(true);
    m_clientForm->addRow(tr("Email:"), m_clientEmailEdit);
    
    m_clientPhoneEdit = new QLineEdit();
    m_clientPhoneEdit->setReadOnly(true);
    m_clientForm->addRow(tr("Phone:"), m_clientPhoneEdit);
    
    m_clientTaxIdEdit = new QLineEdit();
    m_clientTaxIdEdit->setReadOnly(true);
    m_clientForm->addRow(tr("Tax ID:"), m_clientTaxIdEdit);
    
    m_tabWidget->addTab(m_clientTab, tr("Client Info"));
}

void InvoiceDialog::setupConnections()
{
    // Form connections
    connect(m_invoiceNumberEdit, &QLineEdit::textChanged, this, &InvoiceDialog::onInvoiceNumberChanged);
    connect(m_clientCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvoiceDialog::onClientChanged);
    connect(m_invoiceDateEdit, &QDateEdit::dateChanged, this, &InvoiceDialog::onInvoiceDateChanged);
    connect(m_dueDateEdit, &QDateEdit::dateChanged, this, &InvoiceDialog::onDueDateChanged);
    connect(m_statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvoiceDialog::onStatusChanged);
    connect(m_taxRateSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InvoiceDialog::onTaxRateChanged);
    connect(m_notesEdit, &QTextEdit::textChanged, this, &InvoiceDialog::onNotesChanged);
    connect(m_currencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvoiceDialog::onCurrencyChanged);
    
    // Client connections
    connect(m_selectClientBtn, &QPushButton::clicked, this, &InvoiceDialog::onSelectClientClicked);
    connect(m_newClientBtn, &QPushButton::clicked, this, &InvoiceDialog::onNewClientClicked);
    
    // Item connections
    connect(m_addItemBtn, &QPushButton::clicked, this, &InvoiceDialog::onAddItemClicked);
    connect(m_editItemBtn, &QPushButton::clicked, this, &InvoiceDialog::onEditItemClicked);
    connect(m_deleteItemBtn, &QPushButton::clicked, this, &InvoiceDialog::onDeleteItemClicked);
    connect(m_duplicateItemBtn, &QPushButton::clicked, this, &InvoiceDialog::onDuplicateItemClicked);
    connect(m_itemsTable, &QTableWidget::cellChanged, this, &InvoiceDialog::onItemChanged);
    connect(m_itemsTable, &QTableWidget::itemSelectionChanged, this, &InvoiceDialog::onItemSelectionChanged);
    
    // Button connections
    connect(m_calculateBtn, &QPushButton::clicked, this, &InvoiceDialog::onCalculateClicked);
    connect(m_previewBtn, &QPushButton::clicked, this, &InvoiceDialog::onPreviewClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &InvoiceDialog::onSaveClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &InvoiceDialog::onCancelClicked);
}

void InvoiceDialog::populateForm()
{
    if (!m_invoice) {
        clearForm();
        return;
    }
    
    m_invoiceNumberEdit->setText(m_invoice->invoiceNumber());
    m_invoiceDateEdit->setDate(m_invoice->invoiceDate());
    m_dueDateEdit->setDate(m_invoice->dueDate());
    m_taxRateSpinBox->setValue(m_invoice->taxRate());
    m_notesEdit->setPlainText(m_invoice->notes());
    m_currencyCombo->setCurrentText(m_invoice->currency());
    
    // Set status
    QString status = m_invoice->status();
    int statusIndex = 0;
    if (status == "Sent") statusIndex = 1;
    else if (status == "Paid") statusIndex = 2;
    else if (status == "Overdue") statusIndex = 3;
    else if (status == "Cancelled") statusIndex = 4;
    m_statusCombo->setCurrentIndex(statusIndex);
    
    // Set client
    for (int i = 0; i < m_clientCombo->count(); ++i) {
        if (m_clientCombo->itemData(i).toString() == m_invoice->clientId()) {
            m_clientCombo->setCurrentIndex(i);
            break;
        }
    }
    
    populateItemsTable();
    updateTotals();
    onClientChanged(); // Update client info
}

void InvoiceDialog::populateClientCombo()
{
    m_clientCombo->clear();
    for (const Client *client : m_clients) {
        m_clientCombo->addItem(client->name(), client->id());
    }
}

void InvoiceDialog::populateItemsTable()
{
    m_itemsTable->setRowCount(0);
    
    if (!m_invoice) return;
    
    const QList<InvoiceItem*> items = m_invoice->items();
    for (const InvoiceItem *item : items) {
        addItemToTable(item);
    }
}

void InvoiceDialog::updateTotals()
{
    if (!m_invoice) {
        m_subtotalLabel->setText("$0.00");
        m_taxAmountLabel->setText("$0.00");
        m_totalAmountLabel->setText("$0.00");
        return;
    }
    
    double subtotal = m_invoice->subtotal();
    double taxAmount = m_invoice->taxAmount();
    double total = m_invoice->totalAmount();
    
    QString currency = m_invoice->currency();
    QString currencySymbol = (currency == "EUR") ? "€" : (currency == "GBP") ? "£" : "$";
    
    m_subtotalLabel->setText(QString("%1%2").arg(currencySymbol).arg(subtotal, 0, 'f', 2));
    m_taxAmountLabel->setText(QString("%1%2").arg(currencySymbol).arg(taxAmount, 0, 'f', 2));
    m_totalAmountLabel->setText(QString("%1%2").arg(currencySymbol).arg(total, 0, 'f', 2));
}

void InvoiceDialog::addItemToTable(const InvoiceItem *item, int row)
{
    if (row == -1) {
        row = m_itemsTable->rowCount();
        m_itemsTable->insertRow(row);
    }
    
    m_itemsTable->setItem(row, DescriptionColumn, new QTableWidgetItem(item->description()));
    m_itemsTable->setItem(row, QuantityColumn, new QTableWidgetItem(QString::number(item->quantity())));
    m_itemsTable->setItem(row, UnitColumn, new QTableWidgetItem(item->unit()));
    m_itemsTable->setItem(row, UnitPriceColumn, new QTableWidgetItem(QString::number(item->unitPrice(), 'f', 2)));
    m_itemsTable->setItem(row, TotalPriceColumn, new QTableWidgetItem(QString::number(item->totalPrice(), 'f', 2)));
    
    // Make total price read-only
    m_itemsTable->item(row, TotalPriceColumn)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void InvoiceDialog::updateItemInTable(const InvoiceItem *item, int row)
{
    if (row < 0 || row >= m_itemsTable->rowCount()) return;
    
    m_itemsTable->item(row, DescriptionColumn)->setText(item->description());
    m_itemsTable->item(row, QuantityColumn)->setText(QString::number(item->quantity()));
    m_itemsTable->item(row, UnitColumn)->setText(item->unit());
    m_itemsTable->item(row, UnitPriceColumn)->setText(QString::number(item->unitPrice(), 'f', 2));
    m_itemsTable->item(row, TotalPriceColumn)->setText(QString::number(item->totalPrice(), 'f', 2));
}

void InvoiceDialog::removeItemFromTable(int row)
{
    if (row >= 0 && row < m_itemsTable->rowCount()) {
        m_itemsTable->removeRow(row);
    }
}

void InvoiceDialog::clearForm()
{
    m_invoiceNumberEdit->clear();
    m_clientCombo->setCurrentIndex(-1);
    m_invoiceDateEdit->setDate(QDate::currentDate());
    m_dueDateEdit->setDate(QDate::currentDate().addDays(30));
    m_statusCombo->setCurrentIndex(0);
    m_currencyCombo->setCurrentIndex(0);
    m_taxRateSpinBox->setValue(0.0);
    m_notesEdit->clear();
    m_itemsTable->setRowCount(0);
    updateTotals();
}

bool InvoiceDialog::validateInput()
{
    if (m_invoiceNumberEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Invoice number is required."));
        m_tabWidget->setCurrentWidget(m_invoiceTab);
        m_invoiceNumberEdit->setFocus();
        return false;
    }
    
    if (m_clientCombo->currentIndex() == -1) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Please select a client."));
        m_tabWidget->setCurrentWidget(m_invoiceTab);
        m_clientCombo->setFocus();
        return false;
    }
    
    if (m_itemsTable->rowCount() == 0) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Please add at least one line item."));
        m_tabWidget->setCurrentWidget(m_itemsTab);
        return false;
    }
    
    return true;
}

void InvoiceDialog::markAsModified()
{
    m_isModified = true;
}

void InvoiceDialog::applyFormData()
{
    if (!m_invoice) return;
    
    m_invoice->setInvoiceNumber(m_invoiceNumberEdit->text().trimmed());
    m_invoice->setClientId(m_clientCombo->currentData().toString());
    m_invoice->setInvoiceDate(m_invoiceDateEdit->date());
    m_invoice->setDueDate(m_dueDateEdit->date());
    m_invoice->setTaxRate(m_taxRateSpinBox->value());
    m_invoice->setNotes(m_notesEdit->toPlainText());
    m_invoice->setCurrency(m_currencyCombo->currentText());
    
    // Set status
    QStringList statuses = {"Draft", "Sent", "Paid", "Overdue", "Cancelled"};
    if (m_statusCombo->currentIndex() >= 0 && m_statusCombo->currentIndex() < statuses.size()) {
        m_invoice->setStatus(statuses[m_statusCombo->currentIndex()]);
    }
    
    // Update client info
    if (m_clientCombo->currentIndex() >= 0) {
        Client *client = m_clients[m_clientCombo->currentIndex()];
        m_invoice->setClientName(client->name());
        m_invoice->setClientAddress(client->address());
        m_invoice->setClientEmail(client->email());
        m_invoice->setClientPhone(client->phone());
    }
}

// Slot implementations
void InvoiceDialog::onInvoiceNumberChanged()
{
    markAsModified();
}

void InvoiceDialog::onClientChanged()
{
    markAsModified();
    
    // Update client info display
    int index = m_clientCombo->currentIndex();
    if (index >= 0 && index < m_clients.size()) {
        Client *client = m_clients[index];
        m_clientNameEdit->setText(client->name());
        m_clientCompanyEdit->setText(client->company());
        m_clientAddressEdit->setPlainText(client->address());
        m_clientEmailEdit->setText(client->email());
        m_clientPhoneEdit->setText(client->phone());
        m_clientTaxIdEdit->setText(client->taxId());
    } else {
        m_clientNameEdit->clear();
        m_clientCompanyEdit->clear();
        m_clientAddressEdit->clear();
        m_clientEmailEdit->clear();
        m_clientPhoneEdit->clear();
        m_clientTaxIdEdit->clear();
    }
}

void InvoiceDialog::onInvoiceDateChanged()
{
    markAsModified();
}

void InvoiceDialog::onDueDateChanged()
{
    markAsModified();
}

void InvoiceDialog::onStatusChanged()
{
    markAsModified();
}

void InvoiceDialog::onTaxRateChanged()
{
    markAsModified();
    updateCalculations();
}

void InvoiceDialog::onNotesChanged()
{
    markAsModified();
}

void InvoiceDialog::onCurrencyChanged()
{
    markAsModified();
    updateTotals();
}

void InvoiceDialog::onAddItemClicked()
{
    bool ok;
    QString description = QInputDialog::getText(this, tr("Add Item"), tr("Description:"), QLineEdit::Normal, "", &ok);
    if (!ok || description.trimmed().isEmpty()) return;
    
    double quantity = QInputDialog::getDouble(this, tr("Add Item"), tr("Quantity:"), 1.0, 0.01, 9999.99, 2, &ok);
    if (!ok) return;
    
    QString unit = QInputDialog::getText(this, tr("Add Item"), tr("Unit:"), QLineEdit::Normal, "ea", &ok);
    if (!ok) return;
    
    double unitPrice = QInputDialog::getDouble(this, tr("Add Item"), tr("Unit Price:"), 0.0, 0.0, 99999.99, 2, &ok);
    if (!ok) return;
    
    if (!m_invoice) {
        m_invoice = new Invoice(this);
    }
    
    InvoiceItem *item = new InvoiceItem(m_invoice);
    item->setDescription(description.trimmed());
    item->setQuantity(quantity);
    item->setUnit(unit.trimmed());
    item->setUnitPrice(unitPrice);
    
    m_invoice->addItem(item);
    addItemToTable(item);
    updateCalculations();
    markAsModified();
}

void InvoiceDialog::onEditItemClicked()
{
    int row = m_itemsTable->currentRow();
    if (row < 0) return;
    
    // Simple inline editing - more sophisticated dialog could be added
    QMessageBox::information(this, tr("Edit Item"), tr("Double-click on a cell to edit the item directly in the table."));
}

void InvoiceDialog::onDeleteItemClicked()
{
    int row = m_itemsTable->currentRow();
    if (row < 0) return;
    
    int ret = QMessageBox::question(this, tr("Delete Item"), 
                                   tr("Are you sure you want to delete this item?"),
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;
      if (m_invoice && row < m_invoice->items().size()) {
        InvoiceItem *item = m_invoice->items()[row];
        m_invoice->removeItem(item);
        removeItemFromTable(row);
        updateCalculations();
        markAsModified();
    }
}

void InvoiceDialog::onDuplicateItemClicked()
{
    int row = m_itemsTable->currentRow();
    if (row < 0 || !m_invoice || row >= m_invoice->items().size()) return;
    
    InvoiceItem *originalItem = m_invoice->items()[row];
    InvoiceItem *newItem = new InvoiceItem(m_invoice);
    newItem->setDescription(originalItem->description() + " (Copy)");
    newItem->setQuantity(originalItem->quantity());
    newItem->setUnit(originalItem->unit());
    newItem->setUnitPrice(originalItem->unitPrice());
    
    m_invoice->addItem(newItem);
    addItemToTable(newItem);
    updateCalculations();
    markAsModified();
}

void InvoiceDialog::onItemChanged(int row, int column)
{
    if (!m_invoice || row >= m_invoice->items().size()) return;
    
    InvoiceItem *item = m_invoice->items()[row];
    QTableWidgetItem *tableItem = m_itemsTable->item(row, column);
    if (!tableItem) return;
    
    switch (column) {
    case DescriptionColumn:
        item->setDescription(tableItem->text());
        break;
    case QuantityColumn:
        item->setQuantity(tableItem->text().toDouble());
        break;
    case UnitColumn:
        item->setUnit(tableItem->text());
        break;
    case UnitPriceColumn:
        item->setUnitPrice(tableItem->text().toDouble());
        break;
    default:
        return;
    }
    
    // Update total price for this item
    m_itemsTable->item(row, TotalPriceColumn)->setText(QString::number(item->totalPrice(), 'f', 2));
    updateCalculations();
    markAsModified();
}

void InvoiceDialog::onItemSelectionChanged()
{
    bool hasSelection = m_itemsTable->currentRow() >= 0;
    m_editItemBtn->setEnabled(hasSelection);
    m_deleteItemBtn->setEnabled(hasSelection);
    m_duplicateItemBtn->setEnabled(hasSelection);
}

void InvoiceDialog::onSelectClientClicked()
{
    // This could open a client selection dialog
    QMessageBox::information(this, tr("Select Client"), tr("Use the dropdown to select an existing client."));
}

void InvoiceDialog::onNewClientClicked()
{
    ClientDialog dialog(this);
    dialog.setDatabaseManager(m_dbManager);
    
    if (dialog.exec() == QDialog::Accepted) {
        Client *newClient = dialog.client();
        if (newClient && m_dbManager) {
            if (m_dbManager->addClient(newClient)) {
                m_clients.append(newClient);
                populateClientCombo();
                
                // Select the new client
                for (int i = 0; i < m_clientCombo->count(); ++i) {
                    if (m_clientCombo->itemData(i).toString() == newClient->id()) {
                        m_clientCombo->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }
    }
}

void InvoiceDialog::onClientInfoChanged()
{
    markAsModified();
}

void InvoiceDialog::onCalculateClicked()
{
    updateCalculations();
}

void InvoiceDialog::onPreviewClicked()
{
    if (!validateInput()) return;
    
    applyFormData();
    updateCalculations();
    
    // TODO: Generate and show PDF preview
    QMessageBox::information(this, tr("PDF Preview"), tr("PDF preview functionality will be implemented."));
}

void InvoiceDialog::onSaveClicked()
{
    accept();
}

void InvoiceDialog::onCancelClicked()
{
    reject();
}

void InvoiceDialog::validateForm()
{
    // Update button states
    bool isValid = validateInput();
    m_saveBtn->setEnabled(isValid);
    m_previewBtn->setEnabled(isValid);
}

void InvoiceDialog::updateCalculations()
{
    if (!m_invoice) return;
    
    // Recalculate invoice totals
    m_invoice->calculateTotals();
    updateTotals();
}

void InvoiceDialog::accept()
{
    if (!validateInput()) return;
    
    applyFormData();
    updateCalculations();
    
    QDialog::accept();
}

void InvoiceDialog::reject()
{
    if (m_isModified) {
        int ret = QMessageBox::question(this, tr("Unsaved Changes"), 
                                       tr("You have unsaved changes. Do you want to save them?"),
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            accept();
            return;
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    QDialog::reject();
}

void InvoiceDialog::closeEvent(QCloseEvent *event)
{
    if (m_isModified) {
        int ret = QMessageBox::question(this, tr("Unsaved Changes"), 
                                       tr("You have unsaved changes. Do you want to save them?"),
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            if (validateInput()) {
                accept();
                event->accept();
            } else {
                event->ignore();
            }
        } else if (ret == QMessageBox::Cancel) {
            event->ignore();
        } else {
            event->accept();
        }
    } else {
        event->accept();
    }
}
