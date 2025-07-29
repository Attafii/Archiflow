#include "materialdetailsdialog.h"
#include "utils/stylemanager.h"
#include <QApplication>

MaterialDetailsDialog::MaterialDetailsDialog(const Material &material, QWidget *parent)
    : QDialog(parent)
    , m_material(material)
    , m_mainLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_contentLayout(nullptr)
{
    setupUI();
    applyStyles();
}

void MaterialDetailsDialog::setupUI()
{
    setWindowTitle("Material Details");
    setModal(true);
    resize(600, 700);
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(30, 30, 30, 20);
    m_contentLayout->setSpacing(25);
    
    // Header
    QWidget *headerWidget = new QWidget();
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setSpacing(8);
    
    m_titleLabel = new QLabel(m_material.name);
    m_titleLabel->setObjectName("detailsTitle");
    
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_idLabel = new QLabel(QString("ID: %1").arg(m_material.id));
    m_idLabel->setObjectName("detailsId");
    
    m_statusLabel = new QLabel(m_material.status.toUpper());
    m_statusLabel->setObjectName("detailsStatus");
    if (m_material.status == "active") {
        m_statusLabel->setProperty("status", "active");
    } else if (m_material.status == "inactive") {
        m_statusLabel->setProperty("status", "inactive");
    } else {
        m_statusLabel->setProperty("status", "discontinued");
    }
    
    statusLayout->addWidget(m_idLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_statusLabel);
    
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addLayout(statusLayout);
    
    m_contentLayout->addWidget(headerWidget);
    
    // Add separator
    QFrame *separator1 = new QFrame();
    separator1->setFrameShape(QFrame::HLine);
    separator1->setObjectName("separator");
    m_contentLayout->addWidget(separator1);
    
    createInfoSection();
    createStockSection();
    createHistorySection();
    
    // Buttons
    m_buttonsLayout = new QHBoxLayout();
    m_buttonsLayout->setSpacing(15);
    
    m_editButton = new QPushButton("Edit Material");
    m_editButton->setObjectName("primaryButton");
    
    m_closeButton = new QPushButton("Close");
    m_closeButton->setObjectName("secondaryButton");
    
    m_buttonsLayout->addStretch();
    m_buttonsLayout->addWidget(m_editButton);
    m_buttonsLayout->addWidget(m_closeButton);
    
    m_contentLayout->addStretch();
    m_contentLayout->addLayout(m_buttonsLayout);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);
    
    // Connect signals
    connect(m_editButton, &QPushButton::clicked, this, &MaterialDetailsDialog::editRequested);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
}

void MaterialDetailsDialog::createInfoSection()
{
    m_infoGroup = new QGroupBox("Material Information");
    m_infoGroup->setObjectName("detailsGroup");
    m_infoLayout = new QGridLayout(m_infoGroup);
    m_infoLayout->setSpacing(15);
    m_infoLayout->setColumnStretch(1, 1);
    
    int row = 0;
    
    // Helper function to add info row
    auto addInfoRow = [this, &row](const QString &label, const QString &value) {
        QLabel *labelWidget = new QLabel(label + ":");
        labelWidget->setObjectName("detailsLabel");
        
        QLabel *valueWidget = new QLabel(value.isEmpty() ? "N/A" : value);
        valueWidget->setObjectName("detailsValue");
        valueWidget->setWordWrap(true);
        
        m_infoLayout->addWidget(labelWidget, row, 0, Qt::AlignTop);
        m_infoLayout->addWidget(valueWidget, row, 1);
        row++;
    };
    
    addInfoRow("Name", m_material.name);
    addInfoRow("Description", m_material.description);
    addInfoRow("Category", m_material.category);
    addInfoRow("Unit", m_material.unit);
    addInfoRow("Barcode", m_material.barcode);
    addInfoRow("Location", m_material.location);
    addInfoRow("Supplier ID", QString::number(m_material.supplierId));
    
    m_contentLayout->addWidget(m_infoGroup);
}

void MaterialDetailsDialog::createStockSection()
{
    m_stockGroup = new QGroupBox("Stock Information");
    m_stockGroup->setObjectName("detailsGroup");
    m_stockLayout = new QGridLayout(m_stockGroup);
    m_stockLayout->setSpacing(15);
    m_stockLayout->setColumnStretch(1, 1);
    
    int row = 0;
    
    // Helper function to add stock row
    auto addStockRow = [this, &row](const QString &label, const QString &value, 
                                   const QString &styleClass = QString()) {
        QLabel *labelWidget = new QLabel(label + ":");
        labelWidget->setObjectName("detailsLabel");
        
        QLabel *valueWidget = new QLabel(value);
        valueWidget->setObjectName("detailsValue");
        if (!styleClass.isEmpty()) {
            valueWidget->setProperty("class", styleClass);
        }
        
        m_stockLayout->addWidget(labelWidget, row, 0);
        m_stockLayout->addWidget(valueWidget, row, 1);
        row++;
    };
    
    // Check stock level
    QString stockStatus = "Normal";
    QString stockClass = "";
    if (m_material.quantity <= m_material.reorderPoint) {
        stockStatus = "Low Stock";
        stockClass = "warning";
    } else if (m_material.quantity >= m_material.maximumStock) {
        stockStatus = "Overstock";
        stockClass = "warning";
    }
    
    addStockRow("Current Quantity", QString::number(m_material.quantity), stockClass);
    addStockRow("Unit Price", QString("$%1").arg(m_material.price, 0, 'f', 2));
    addStockRow("Total Value", QString("$%1").arg(m_material.quantity * m_material.price, 0, 'f', 2));
    addStockRow("Minimum Stock", QString::number(m_material.minimumStock));
    addStockRow("Maximum Stock", QString::number(m_material.maximumStock));
    addStockRow("Reorder Point", QString::number(m_material.reorderPoint));
    addStockRow("Stock Status", stockStatus, stockClass);
    
    m_contentLayout->addWidget(m_stockGroup);
}

void MaterialDetailsDialog::createHistorySection()
{
    m_historyGroup = new QGroupBox("History");
    m_historyGroup->setObjectName("detailsGroup");
    m_historyLayout = new QGridLayout(m_historyGroup);
    m_historyLayout->setSpacing(15);
    m_historyLayout->setColumnStretch(1, 1);
    
    int row = 0;
    
    // Helper function to add history row
    auto addHistoryRow = [this, &row](const QString &label, const QString &value) {
        QLabel *labelWidget = new QLabel(label + ":");
        labelWidget->setObjectName("detailsLabel");
        
        QLabel *valueWidget = new QLabel(value);
        valueWidget->setObjectName("detailsValue");
        
        m_historyLayout->addWidget(labelWidget, row, 0);
        m_historyLayout->addWidget(valueWidget, row, 1);
        row++;
    };
    
    addHistoryRow("Created At", m_material.createdAt.toString("yyyy-MM-dd hh:mm:ss"));
    addHistoryRow("Created By", m_material.createdBy);
    addHistoryRow("Updated At", m_material.updatedAt.toString("yyyy-MM-dd hh:mm:ss"));
    addHistoryRow("Updated By", m_material.updatedBy);
    
    m_contentLayout->addWidget(m_historyGroup);
}

void MaterialDetailsDialog::applyStyles()
{
    StyleManager::applyMaterialDesign(this);
    
    setStyleSheet(styleSheet() + R"(
        #detailsTitle {
            font-size: 24px;
            font-weight: 600;
            color: #2C3E50;
            margin-bottom: 5px;
        }
        
        #detailsId {
            font-size: 14px;
            color: #6C757D;
            font-weight: 500;
        }
        
        #detailsStatus {
            font-size: 12px;
            font-weight: 600;
            padding: 4px 12px;
            border-radius: 12px;
        }
        
        #detailsStatus[status="active"] {
            background-color: #D4EDDA;
            color: #155724;
        }
        
        #detailsStatus[status="inactive"] {
            background-color: #F8D7DA;
            color: #721C24;
        }
        
        #detailsStatus[status="discontinued"] {
            background-color: #FCF8E3;
            color: #856404;
        }
        
        #detailsGroup {
            font-size: 16px;
            font-weight: 600;
            color: #495057;
            border: 1px solid #E9ECEF;
            border-radius: 8px;
            padding-top: 15px;
        }
        
        #detailsGroup::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            background-color: white;
        }
        
        #detailsLabel {
            font-weight: 600;
            color: #6C757D;
            font-size: 14px;
        }
        
        #detailsValue {
            color: #495057;
            font-size: 14px;
            background-color: #F8F9FA;
            padding: 8px 12px;
            border-radius: 4px;
            border: 1px solid #E9ECEF;
        }
        
        #detailsValue[class="warning"] {
            background-color: #FFF3CD;
            color: #856404;
            border-color: #FFEAA7;
        }
        
        #separator {
            background-color: #E9ECEF;
            max-height: 1px;
        }
        
        #primaryButton {
            background-color: #007BFF;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 6px;
            font-weight: 600;
            font-size: 14px;
        }
        
        #primaryButton:hover {
            background-color: #0056B3;
        }
        
        #secondaryButton {
            background-color: #6C757D;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 6px;
            font-weight: 600;
            font-size: 14px;
        }
        
        #secondaryButton:hover {
            background-color: #545B62;
        }
    )");
}
