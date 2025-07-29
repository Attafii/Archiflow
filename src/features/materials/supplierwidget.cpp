#include "supplierwidget.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>

SupplierWidget::SupplierWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_toolbarLayout(nullptr)
    , m_searchEdit(nullptr)
    , m_addButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_refreshButton(nullptr)
    , m_supplierTable(nullptr)
    , m_supplierModel(nullptr)
    , m_proxyModel(nullptr)
    , m_detailsGroup(nullptr)
    , m_detailsLayout(nullptr)
    , m_detailsText(nullptr)
    , m_statusLabel(nullptr)
{
    setupUI();
    setupConnections();
    updateButtons();
}

void SupplierWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Toolbar
    m_toolbarLayout = new QHBoxLayout();
    
    // Search
    QLabel *searchLabel = new QLabel("Search:", this);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search suppliers...");
    m_searchEdit->setMaximumWidth(200);
    
    m_toolbarLayout->addWidget(searchLabel);
    m_toolbarLayout->addWidget(m_searchEdit);
    m_toolbarLayout->addStretch();
    
    // Action buttons
    m_addButton = new QPushButton("Add Supplier", this);
    m_addButton->setIcon(QIcon("://icons/add.png"));
    
    m_editButton = new QPushButton("Edit", this);
    m_editButton->setIcon(QIcon("://icons/edit.png"));
    
    m_deleteButton = new QPushButton("Delete", this);
    m_deleteButton->setIcon(QIcon("://icons/delete.png"));
    
    m_refreshButton = new QPushButton("Refresh", this);
    m_refreshButton->setIcon(QIcon("://icons/refresh.png"));
    
    m_toolbarLayout->addWidget(m_addButton);
    m_toolbarLayout->addWidget(m_editButton);
    m_toolbarLayout->addWidget(m_deleteButton);
    m_toolbarLayout->addWidget(m_refreshButton);
    
    m_mainLayout->addLayout(m_toolbarLayout);
    
    // Main content area with splitter
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    // Supplier table
    m_supplierModel = new SupplierModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_supplierModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1); // Search all columns
    
    m_supplierTable = new QTableView(this);
    m_supplierTable->setModel(m_proxyModel);
    m_supplierTable->setSortingEnabled(true);
    m_supplierTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_supplierTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_supplierTable->setAlternatingRowColors(true);
    m_supplierTable->horizontalHeader()->setStretchLastSection(true);
    m_supplierTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_supplierTable->verticalHeader()->hide();
    
    m_splitter->addWidget(m_supplierTable);
    
    // Details panel
    m_detailsGroup = new QGroupBox("Supplier Details", this);
    m_detailsLayout = new QVBoxLayout(m_detailsGroup);
    
    m_detailsText = new QTextEdit(this);
    m_detailsText->setReadOnly(true);
    m_detailsText->setMaximumHeight(200);
    m_detailsLayout->addWidget(m_detailsText);
    
    m_splitter->addWidget(m_detailsGroup);
    m_splitter->setSizes({700, 300});
    
    m_mainLayout->addWidget(m_splitter);
    
    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: blue; font-style: italic;");
    m_mainLayout->addWidget(m_statusLabel);
    
    // Initially show welcome message
    m_detailsText->setHtml("<p><i>Select a supplier to view details</i></p>");
    updateStatusLabel();
}

void SupplierWidget::setupConnections()
{
    // Button connections
    connect(m_addButton, &QPushButton::clicked, this, &SupplierWidget::onAddSupplier);
    connect(m_editButton, &QPushButton::clicked, this, &SupplierWidget::onEditSupplier);
    connect(m_deleteButton, &QPushButton::clicked, this, &SupplierWidget::onDeleteSupplier);
    connect(m_refreshButton, &QPushButton::clicked, this, &SupplierWidget::refreshView);
    
    // Search connection
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SupplierWidget::onSearchTextChanged);
    
    // Table connections
    connect(m_supplierTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &SupplierWidget::onSelectionChanged);
    connect(m_supplierTable, &QTableView::doubleClicked,
            this, &SupplierWidget::onSupplierDoubleClicked);
    
    // Model connections
    connect(m_supplierModel, &SupplierModel::supplierAdded,
            this, &SupplierWidget::suppliersUpdated);
    connect(m_supplierModel, &SupplierModel::supplierUpdated,
            this, &SupplierWidget::suppliersUpdated);
    connect(m_supplierModel, &SupplierModel::supplierRemoved,
            this, &SupplierWidget::suppliersUpdated);
}

void SupplierWidget::onAddSupplier()
{
    SupplierDialog dialog(SupplierDialog::AddMode, this);
    if (dialog.exec() == QDialog::Accepted) {
        Supplier newSupplier = dialog.getSupplier();
        if (m_supplierModel->addSupplier(newSupplier)) {
            updateStatusLabel();
            // Select the newly added supplier
            int newRow = m_supplierModel->rowCount() - 1;
            QModelIndex proxyIndex = m_proxyModel->mapFromSource(m_supplierModel->index(newRow, 0));
            m_supplierTable->selectRow(proxyIndex.row());
        }
    }
}

void SupplierWidget::onEditSupplier()
{
    int selectedRow = getSelectedRow();
    if (selectedRow < 0) {
        QMessageBox::information(this, "No Selection", "Please select a supplier to edit.");
        return;
    }
    
    Supplier supplier = m_supplierModel->supplier(selectedRow);
    SupplierDialog dialog(SupplierDialog::EditMode, this);
    dialog.setSupplier(supplier);
    
    if (dialog.exec() == QDialog::Accepted) {
        Supplier updatedSupplier = dialog.getSupplier();
        if (m_supplierModel->updateSupplier(selectedRow, updatedSupplier)) {
            updateStatusLabel();
            showSupplierDetails(updatedSupplier);
        }
    }
}

void SupplierWidget::onDeleteSupplier()
{
    int selectedRow = getSelectedRow();
    if (selectedRow < 0) {
        QMessageBox::information(this, "No Selection", "Please select a supplier to delete.");
        return;
    }
    
    Supplier supplier = m_supplierModel->supplier(selectedRow);
    int result = QMessageBox::question(this, "Confirm Delete",
                                      QString("Are you sure you want to delete supplier '%1'?")
                                      .arg(supplier.name),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        if (m_supplierModel->removeSupplier(selectedRow)) {
            updateStatusLabel();
            m_detailsText->setHtml("<p><i>Select a supplier to view details</i></p>");
        }
    }
}

void SupplierWidget::onSearchTextChanged(const QString &text)
{
    m_proxyModel->setFilterFixedString(text);
    updateStatusLabel();
}

void SupplierWidget::onSelectionChanged()
{
    updateButtons();
    
    int selectedRow = getSelectedRow();
    if (selectedRow >= 0) {
        Supplier supplier = m_supplierModel->supplier(selectedRow);
        showSupplierDetails(supplier);
        emit supplierSelected(supplier.id);
    } else {
        m_detailsText->setHtml("<p><i>Select a supplier to view details</i></p>");
    }
}

void SupplierWidget::onSupplierDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    onEditSupplier();
}

void SupplierWidget::refreshView()
{
    m_supplierModel->loadSampleData();
    updateStatusLabel();
    m_detailsText->setHtml("<p><i>Select a supplier to view details</i></p>");
}

void SupplierWidget::updateButtons()
{
    bool hasSelection = getSelectedRow() >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void SupplierWidget::showSupplierDetails(const Supplier &supplier)
{
    QString html = QString(
        "<h3>%1</h3>"
        "<p><b>Status:</b> %2</p>"
        "<hr>"
        "<p><b>Contact Person:</b> %3</p>"
        "<p><b>Email:</b> %4</p>"
        "<p><b>Phone:</b> %5</p>"
        "<p><b>Website:</b> %6</p>"
        "<hr>"
        "<p><b>Address:</b><br>%7<br>%8, %9</p>"
        "<hr>"
        "<p><b>Notes:</b><br>%10</p>"
    ).arg(supplier.name)
     .arg(supplier.isActive ? "Active" : "Inactive")
     .arg(supplier.contactPerson.isEmpty() ? "Not specified" : supplier.contactPerson)
     .arg(supplier.email.isEmpty() ? "Not specified" : supplier.email)
     .arg(supplier.phone.isEmpty() ? "Not specified" : supplier.phone)
     .arg(supplier.website.isEmpty() ? "Not specified" : supplier.website)
     .arg(supplier.address.isEmpty() ? "Not specified" : supplier.address)
     .arg(supplier.city.isEmpty() ? "Not specified" : supplier.city)
     .arg(supplier.country.isEmpty() ? "Not specified" : supplier.country)
     .arg(supplier.notes.isEmpty() ? "No additional notes" : supplier.notes);
    
    m_detailsText->setHtml(html);
}

int SupplierWidget::getSelectedRow() const
{
    QModelIndexList selected = m_supplierTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return -1;
    }
    
    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    return sourceIndex.row();
}

void SupplierWidget::updateStatusLabel()
{
    int totalSuppliers = m_supplierModel->rowCount();
    int visibleSuppliers = m_proxyModel->rowCount();
    
    QString statusText;
    if (totalSuppliers == 0) {
        statusText = "No suppliers available";
    } else if (visibleSuppliers == totalSuppliers) {
        statusText = QString("Showing %1 supplier(s)").arg(totalSuppliers);
    } else {
        statusText = QString("Showing %1 of %2 supplier(s)").arg(visibleSuppliers).arg(totalSuppliers);
    }
    
    m_statusLabel->setText(statusText);
}

// Public interface methods for materials module
QList<Supplier> SupplierWidget::getActiveSuppliers() const
{
    QList<Supplier> activeSuppliers;
    QList<Supplier> allSuppliers = m_supplierModel->suppliers();
    
    for (const Supplier &supplier : allSuppliers) {
        if (supplier.isActive) {
            activeSuppliers.append(supplier);
        }
    }
    
    return activeSuppliers;
}

Supplier SupplierWidget::getSupplierById(int id) const
{
    int row = m_supplierModel->findSupplierById(id);
    if (row >= 0) {
        return m_supplierModel->supplier(row);
    }
    return Supplier();
}

QString SupplierWidget::getSupplierName(int id) const
{
    Supplier supplier = getSupplierById(id);
    return supplier.name;
}
