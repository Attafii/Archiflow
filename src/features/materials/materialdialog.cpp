#include "materialdialog.h"
#include "materialmodel.h"
#include "utils/stylemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QRegularExpressionValidator>
#include <QDoubleValidator>
#include <QScrollArea>

MaterialDialog::MaterialDialog(Mode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_mainLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_scrollWidget(nullptr)
    , m_scrollLayout(nullptr)
    , m_basicGroup(nullptr)
    , m_inventoryGroup(nullptr)
    , m_stockGroup(nullptr)
    , m_supplierGroup(nullptr)
    , m_metadataGroup(nullptr)
    , m_buttonBox(nullptr)
{
    setupUI();
    setupValidation();
    populateComboBoxes();
    
    setWindowTitle(mode == AddMode ? "Add New Material - ArchiFlow" : "Edit Material - ArchiFlow");
    setModal(true);
    
    // Set a more reasonable size with scroll capability
    resize(600, 500);  // Reduced height, scroll area will handle overflow
    setMinimumSize(550, 400);
    setMaximumSize(700, 600);
    
    // Apply the dark theme styling directly
    applyDarkTheme();
    
    // Set window properties for better appearance
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
}

void MaterialDialog::setupUI()
{
    // Main layout for the dialog
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Create scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Create scroll widget that will contain all form elements
    m_scrollWidget = new QWidget();
    m_scrollLayout = new QVBoxLayout(m_scrollWidget);
    m_scrollLayout->setSpacing(12);
    m_scrollLayout->setContentsMargins(10, 10, 10, 10);
    
    // Set the scroll widget to the scroll area
    m_scrollArea->setWidget(m_scrollWidget);
    
    // Basic Information Group
    m_basicGroup = new QGroupBox("Basic Information", this);
    m_basicLayout = new QFormLayout(m_basicGroup);
    m_basicLayout->setSpacing(10);
      m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Enter material name (required)");
    m_nameEdit->setToolTip("Enter a unique name for this material");
    m_basicLayout->addRow("Name *:", m_nameEdit);
    
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setPlaceholderText("Enter material description");
    m_descriptionEdit->setToolTip("Provide a detailed description of the material");
    m_descriptionEdit->setMaximumHeight(80);
    m_basicLayout->addRow("Description:", m_descriptionEdit);
    
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setEditable(true);
    m_categoryCombo->setToolTip("Select or enter a category for this material");
    m_basicLayout->addRow("Category *:", m_categoryCombo);
    
    m_statusCombo = new QComboBox(this);
    m_statusCombo->setToolTip("Set the current status of this material");    m_basicLayout->addRow("Status:", m_statusCombo);
    
    m_scrollLayout->addWidget(m_basicGroup);
    
    // Inventory Group
    m_inventoryGroup = new QGroupBox("Inventory Information", this);
    m_inventoryLayout = new QFormLayout(m_inventoryGroup);
    m_inventoryLayout->setSpacing(10);
      m_quantitySpin = new QSpinBox(this);
    m_quantitySpin->setRange(0, 999999);
    m_quantitySpin->setSuffix(" units");
    m_quantitySpin->setToolTip("Current quantity in stock");
    m_inventoryLayout->addRow("Quantity:", m_quantitySpin);
    
    m_unitEdit = new QLineEdit(this);
    m_unitEdit->setPlaceholderText("e.g., kg, m³, pieces (required)");
    m_unitEdit->setToolTip("Unit of measurement for this material");
    m_inventoryLayout->addRow("Unit *:", m_unitEdit);
    
    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(0.0, 999999.99);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setPrefix("$ ");
    m_priceSpin->setToolTip("Price per unit of this material");
    m_inventoryLayout->addRow("Price per Unit:", m_priceSpin);
    
    m_locationEdit = new QLineEdit(this);
    m_locationEdit->setPlaceholderText("Storage location");
    m_locationEdit->setToolTip("Where this material is stored");    m_inventoryLayout->addRow("Location:", m_locationEdit);
    
    m_scrollLayout->addWidget(m_inventoryGroup);
    
    // Stock Management Group
    m_stockGroup = new QGroupBox("Stock Management", this);
    m_stockLayout = new QFormLayout(m_stockGroup);
    m_stockLayout->setSpacing(10);
    
    m_minimumStockSpin = new QSpinBox(this);
    m_minimumStockSpin->setRange(0, 999999);
    m_minimumStockSpin->setSuffix(" units");
    m_stockLayout->addRow("Minimum Stock:", m_minimumStockSpin);
    
    m_maximumStockSpin = new QSpinBox(this);
    m_maximumStockSpin->setRange(0, 999999);
    m_maximumStockSpin->setSuffix(" units");
    m_stockLayout->addRow("Maximum Stock:", m_maximumStockSpin);
    
    m_reorderPointSpin = new QSpinBox(this);
    m_reorderPointSpin->setRange(0, 999999);
    m_reorderPointSpin->setSuffix(" units");    m_stockLayout->addRow("Reorder Point:", m_reorderPointSpin);
    
    m_scrollLayout->addWidget(m_stockGroup);
    
    // Supplier Information Group
    m_supplierGroup = new QGroupBox("Supplier Information", this);
    m_supplierLayout = new QFormLayout(m_supplierGroup);
    m_supplierLayout->setSpacing(10);
    
    m_supplierIdSpin = new QSpinBox(this);
    m_supplierIdSpin->setRange(0, 999999);
    m_supplierIdSpin->setSpecialValueText("Not assigned");
    m_supplierLayout->addRow("Supplier ID:", m_supplierIdSpin);
    
    m_barcodeEdit = new QLineEdit(this);
    m_barcodeEdit->setPlaceholderText("Enter barcode");    m_supplierLayout->addRow("Barcode:", m_barcodeEdit);
    
    m_scrollLayout->addWidget(m_supplierGroup);
    
    // Metadata Group
    m_metadataGroup = new QGroupBox("Metadata", this);
    m_metadataLayout = new QFormLayout(m_metadataGroup);
    m_metadataLayout->setSpacing(10);
    
    m_createdByEdit = new QLineEdit(this);
    m_createdByEdit->setReadOnly(true);
    m_metadataLayout->addRow("Created By:", m_createdByEdit);
    
    m_createdAtEdit = new QDateTimeEdit(this);
    m_createdAtEdit->setReadOnly(true);
    m_createdAtEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    m_metadataLayout->addRow("Created At:", m_createdAtEdit);
    
    m_updatedByEdit = new QLineEdit(this);
    m_updatedByEdit->setReadOnly(true);
    m_metadataLayout->addRow("Updated By:", m_updatedByEdit);
    
    m_updatedAtEdit = new QDateTimeEdit(this);
    m_updatedAtEdit->setReadOnly(true);
    m_updatedAtEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");    m_metadataLayout->addRow("Updated At:", m_updatedAtEdit);
    
    m_scrollLayout->addWidget(m_metadataGroup);
    
    // Add scroll area to main layout
    m_mainLayout->addWidget(m_scrollArea);
    
    // Button Box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    m_saveButton = m_buttonBox->button(QDialogButtonBox::Save);
    m_cancelButton = m_buttonBox->button(QDialogButtonBox::Cancel);
    
    m_saveButton->setText(m_mode == AddMode ? "Add Material" : "Save Changes");
    m_saveButton->setDefault(true);
    
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &MaterialDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &MaterialDialog::reject);
    
    m_mainLayout->addWidget(m_buttonBox);
}

void MaterialDialog::setupValidation()
{
    // Name validation
    connect(m_nameEdit, &QLineEdit::textChanged, this, &MaterialDialog::validateInput);
    connect(m_unitEdit, &QLineEdit::textChanged, this, &MaterialDialog::validateInput);
    connect(m_categoryCombo, &QComboBox::currentTextChanged, this, &MaterialDialog::validateInput);
    
    // Status change validation
    connect(m_statusCombo, &QComboBox::currentTextChanged, this, &MaterialDialog::onStatusChanged);
    
    // Stock level validation
    connect(m_minimumStockSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int min) {
        if (m_maximumStockSpin->value() < min) {
            m_maximumStockSpin->setValue(min);
        }
        if (m_reorderPointSpin->value() < min) {
            m_reorderPointSpin->setValue(min);
        }
    });
    
    connect(m_maximumStockSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int max) {
        if (m_minimumStockSpin->value() > max) {
            m_minimumStockSpin->setValue(max);
        }
    });
}

void MaterialDialog::populateComboBoxes()
{
    // Categories
    m_categoryCombo->addItems({
        "Construction",
        "Steel",
        "Wood",
        "Concrete",
        "Electrical",
        "Plumbing",
        "Insulation",
        "Roofing",
        "Flooring",
        "Windows & Doors",
        "Hardware",
        "Tools",
        "Safety Equipment",
        "Other"
    });
    
    // Status options
    m_statusCombo->addItems({
        "active",
        "inactive",
        "discontinued",
        "out_of_stock"
    });
    m_statusCombo->setCurrentText("active");
}

void MaterialDialog::setMaterial(const Material &material)
{
    updateUIFromMaterial(material);
}

void MaterialDialog::updateUIFromMaterial(const Material &material)
{
    m_nameEdit->setText(material.name);
    m_descriptionEdit->setPlainText(material.description);
    m_categoryCombo->setCurrentText(material.category);
    m_statusCombo->setCurrentText(material.status);
    
    m_quantitySpin->setValue(material.quantity);
    m_unitEdit->setText(material.unit);
    m_priceSpin->setValue(material.price);
    m_locationEdit->setText(material.location);
    
    m_minimumStockSpin->setValue(material.minimumStock);
    m_maximumStockSpin->setValue(material.maximumStock);
    m_reorderPointSpin->setValue(material.reorderPoint);
    
    m_supplierIdSpin->setValue(material.supplierId);
    m_barcodeEdit->setText(material.barcode);
    
    // Set metadata - auto-populate for new materials
    if (m_mode == AddMode) {
        m_createdByEdit->setText("Current User");
        m_createdAtEdit->setDateTime(QDateTime::currentDateTime());
        m_updatedByEdit->setText("Current User");
        m_updatedAtEdit->setDateTime(QDateTime::currentDateTime());
    } else {
        m_createdByEdit->setText(material.createdBy);
        m_createdAtEdit->setDateTime(material.createdAt);
        m_updatedByEdit->setText(material.updatedBy);
        m_updatedAtEdit->setDateTime(material.updatedAt);
    }
}

Material MaterialDialog::getMaterial() const
{
    Material material;
    
    material.name = m_nameEdit->text().trimmed();
    material.description = m_descriptionEdit->toPlainText().trimmed();
    material.category = m_categoryCombo->currentText().trimmed();
    material.status = m_statusCombo->currentText();
    
    material.quantity = m_quantitySpin->value();
    material.unit = m_unitEdit->text().trimmed();
    material.price = m_priceSpin->value();
    material.location = m_locationEdit->text().trimmed();
    
    material.minimumStock = m_minimumStockSpin->value();
    material.maximumStock = m_maximumStockSpin->value();
    material.reorderPoint = m_reorderPointSpin->value();
    
    material.supplierId = m_supplierIdSpin->value();
    material.barcode = m_barcodeEdit->text().trimmed();
    
    // Set timestamps appropriately
    if (m_mode == AddMode) {
        material.createdBy = "Current User";
        material.createdAt = QDateTime::currentDateTime();
        material.updatedBy = "Current User";
        material.updatedAt = QDateTime::currentDateTime();
    } else {
        // Keep original creation info, update modification info
        material.createdBy = m_createdByEdit->text();
        material.createdAt = m_createdAtEdit->dateTime();
        material.updatedBy = "Current User";
        material.updatedAt = QDateTime::currentDateTime();
    }
    
    return material;
}

void MaterialDialog::accept()
{
    if (validateForm()) {
        QDialog::accept();
    }
}

bool MaterialDialog::validateForm()
{
    QStringList errors;
    
    // Required field validation
    if (m_nameEdit->text().trimmed().isEmpty()) {
        errors << "• Material name is required";
        m_nameEdit->setFocus();
    }
    
    if (m_unitEdit->text().trimmed().isEmpty()) {
        errors << "• Unit is required";
        if (errors.count() == 1) m_unitEdit->setFocus();
    }
    
    if (m_categoryCombo->currentText().trimmed().isEmpty()) {
        errors << "• Category is required";
        if (errors.count() == 1) m_categoryCombo->setFocus();
    }
    
    // Stock validation
    if (m_minimumStockSpin->value() > m_maximumStockSpin->value()) {
        errors << "• Minimum stock cannot be greater than maximum stock";
        if (errors.count() == 1) m_minimumStockSpin->setFocus();
    }
    
    if (m_reorderPointSpin->value() > m_maximumStockSpin->value()) {
        errors << "• Reorder point cannot be greater than maximum stock";
        if (errors.count() == 1) m_reorderPointSpin->setFocus();
    }
    
    if (m_reorderPointSpin->value() < m_minimumStockSpin->value()) {
        errors << "• Reorder point should be at least equal to minimum stock";
        if (errors.count() == 1) m_reorderPointSpin->setFocus();
    }
    
    // Price validation
    if (m_priceSpin->value() < 0) {
        errors << "• Price cannot be negative";
        if (errors.count() == 1) m_priceSpin->setFocus();
    }
    
    // Quantity validation
    if (m_quantitySpin->value() < 0) {
        errors << "• Quantity cannot be negative";
        if (errors.count() == 1) m_quantitySpin->setFocus();
    }
    
    if (!errors.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Validation Error");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Please fix the following errors:");
        msgBox.setDetailedText(errors.join("\n"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return false;
    }
    
    return true;
}

void MaterialDialog::onStatusChanged(const QString &status)
{
    // You could add status-specific validation or UI changes here
    Q_UNUSED(status)
}

void MaterialDialog::validateInput()
{
    bool isValid = !m_nameEdit->text().trimmed().isEmpty() &&
                   !m_unitEdit->text().trimmed().isEmpty() &&
                   !m_categoryCombo->currentText().trimmed().isEmpty() &&
                   m_minimumStockSpin->value() <= m_maximumStockSpin->value() &&
                   m_reorderPointSpin->value() <= m_maximumStockSpin->value() &&
                   m_reorderPointSpin->value() >= m_minimumStockSpin->value();
    
    m_saveButton->setEnabled(isValid);
    
    // Add visual feedback for invalid fields
    QString errorStyle = "border-color: #FF6B6B !important;";
    QString normalStyle = "";
    
    m_nameEdit->setStyleSheet(m_nameEdit->text().trimmed().isEmpty() ? errorStyle : normalStyle);
    m_unitEdit->setStyleSheet(m_unitEdit->text().trimmed().isEmpty() ? errorStyle : normalStyle);
    m_categoryCombo->setStyleSheet(m_categoryCombo->currentText().trimmed().isEmpty() ? errorStyle : normalStyle);
}

void MaterialDialog::applyDarkTheme()
{
    // Apply comprehensive dark theme styling directly to the dialog
    QString darkTheme = R"(
        /* Main Dialog */
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                                       stop:0 #3D485A, stop:1 #2A3340);
            color: #E3C6B0;
            border: 2px solid #D4B7A1;
            border-radius: 10px;
        }
        
        /* Group Boxes */
        QGroupBox {
            background-color: rgba(42, 51, 64, 0.8);
            border: 1px solid #D4B7A1;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 8px;
            font-weight: bold;
            color: #E3C6B0;
        }
        
        QGroupBox::title {
            color: #E3C6B0;
            background-color: transparent;
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            margin-left: 8px;
        }
        
        /* Line Edits */
        QLineEdit {
            background-color: #2A3340;
            border: 2px solid #3D485A;
            border-radius: 6px;
            padding: 8px 12px;
            color: #E3C6B0;
            font-size: 13px;
            min-height: 16px;
        }
        
        QLineEdit:focus {
            border-color: #D4B7A1;
            background-color: #3D485A;
        }
        
        QLineEdit:hover {
            border-color: #E3C6B0;
        }
        
        QLineEdit:read-only {
            background-color: #3D485A;
            color: #D4B7A1;
            border-color: #D4B7A1;
        }
        
        /* Text Edits */
        QTextEdit {
            background-color: #2A3340;
            border: 2px solid #3D485A;
            border-radius: 6px;
            padding: 8px;
            color: #E3C6B0;
            font-size: 13px;
        }
        
        QTextEdit:focus {
            border-color: #D4B7A1;
            background-color: #3D485A;
        }
        
        /* Combo Boxes */
        QComboBox {
            background-color: #2A3340;
            border: 2px solid #3D485A;
            border-radius: 6px;
            padding: 8px 12px;
            color: #E3C6B0;
            font-size: 13px;
            min-height: 16px;
        }
        
        QComboBox:hover {
            border-color: #E3C6B0;
        }
        
        QComboBox:focus {
            border-color: #D4B7A1;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 20px;
            background-color: transparent;
        }
        
        QComboBox QAbstractItemView {
            background-color: #2A3340;
            border: 1px solid #D4B7A1;
            border-radius: 6px;
            selection-background-color: #D4B7A1;
            selection-color: #2A3340;
            color: #E3C6B0;
        }
        
        /* Spin Boxes */
        QSpinBox, QDoubleSpinBox {
            background-color: #2A3340;
            border: 2px solid #3D485A;
            border-radius: 6px;
            padding: 8px 12px;
            color: #E3C6B0;
            font-size: 13px;
            min-height: 16px;
        }
        
        QSpinBox:hover, QDoubleSpinBox:hover {
            border-color: #E3C6B0;
        }
        
        QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: #D4B7A1;
        }
        
        QSpinBox::up-button, QDoubleSpinBox::up-button {
            background-color: #3D485A;
            border-left: 1px solid #D4B7A1;
            border-radius: 0 4px 0 0;
            width: 20px;
        }
        
        QSpinBox::down-button, QDoubleSpinBox::down-button {
            background-color: #3D485A;
            border-left: 1px solid #D4B7A1;
            border-radius: 0 0 4px 0;
            width: 20px;
        }
        
        QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,
        QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
            background-color: #D4B7A1;
        }
        
        /* Date Time Edits */
        QDateTimeEdit {
            background-color: #2A3340;
            border: 2px solid #3D485A;
            border-radius: 6px;
            padding: 8px 12px;
            color: #E3C6B0;
            font-size: 13px;
            min-height: 16px;
        }
        
        QDateTimeEdit:hover {
            border-color: #E3C6B0;
        }
        
        QDateTimeEdit:focus {
            border-color: #D4B7A1;
        }
        
        QDateTimeEdit:read-only {
            background-color: #3D485A;
            color: #D4B7A1;
            border-color: #D4B7A1;
        }
        
        /* Labels */
        QLabel {
            color: #E3C6B0;
            background-color: transparent;
            font-size: 13px;
            font-weight: 500;
        }
        
        /* Buttons */
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #D4B7A1, stop:1 #E3C6B0);
            border: 1px solid #D4B7A1;
            border-radius: 6px;
            color: #2A3340;
            font-weight: bold;
            font-size: 13px;
            padding: 10px 24px;
            min-width: 100px;
            margin: 2px;
        }
        
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #E3C6B0, stop:1 #D4B7A1);
        }
        
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #D4B7A1, stop:1 #C4A791);
        }
        
        QPushButton:default {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #4CAF50, stop:1 #45A049);
            border-color: #4CAF50;
            color: white;
        }
        
        QPushButton:default:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                       stop:0 #5CBF60, stop:1 #4CAF50);
        }
          QPushButton:disabled {
            background-color: #3D485A;
            color: #6D7A8F;
            border-color: #6D7A8F;
        }
        
        /* Scroll Area */
        QScrollArea {
            background-color: #3D485A;
            border: none;
        }
        
        QScrollBar:vertical {
            background-color: #2A3340;
            width: 12px;
            border-radius: 6px;
            margin: 0;
        }
        
        QScrollBar::handle:vertical {
            background-color: #D4B7A1;
            border-radius: 6px;
            min-height: 20px;
            margin: 2px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #E3C6B0;
        }
        
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
        
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )";
    
    setStyleSheet(darkTheme);
}
