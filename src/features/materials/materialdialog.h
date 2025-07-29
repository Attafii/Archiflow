#ifndef MATERIALDIALOG_H
#define MATERIALDIALOG_H

#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>

struct Material;

/**
 * @brief Dialog for adding and editing materials
 * 
 * This dialog provides a comprehensive form for entering material information
 * including basic details, inventory data, supplier information, and metadata.
 */
class MaterialDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        AddMode,
        EditMode
    };

    explicit MaterialDialog(Mode mode = AddMode, QWidget *parent = nullptr);
    ~MaterialDialog() = default;

    // Data access
    void setMaterial(const Material &material);
    Material getMaterial() const;

public slots:
    void accept() override;

private slots:
    void onStatusChanged(const QString &status);
    void validateInput();

private:
    void setupUI();
    void setupValidation();
    void populateComboBoxes();
    void updateUIFromMaterial(const Material &material);
    bool validateForm();
    void applyDarkTheme();    Mode m_mode;
    
    // Layout and Scroll Area
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_scrollWidget;
    QVBoxLayout *m_scrollLayout;
    
    // Basic Information Group
    QGroupBox *m_basicGroup;
    QFormLayout *m_basicLayout;
    QLineEdit *m_nameEdit;
    QTextEdit *m_descriptionEdit;
    QComboBox *m_categoryCombo;
    QComboBox *m_statusCombo;
    
    // Inventory Group
    QGroupBox *m_inventoryGroup;
    QFormLayout *m_inventoryLayout;
    QSpinBox *m_quantitySpin;
    QLineEdit *m_unitEdit;
    QDoubleSpinBox *m_priceSpin;
    QLineEdit *m_locationEdit;
    
    // Stock Management Group
    QGroupBox *m_stockGroup;
    QFormLayout *m_stockLayout;
    QSpinBox *m_minimumStockSpin;
    QSpinBox *m_maximumStockSpin;
    QSpinBox *m_reorderPointSpin;
    
    // Supplier Information Group
    QGroupBox *m_supplierGroup;
    QFormLayout *m_supplierLayout;
    QSpinBox *m_supplierIdSpin;
    QLineEdit *m_barcodeEdit;
    
    // Metadata Group
    QGroupBox *m_metadataGroup;
    QFormLayout *m_metadataLayout;
    QLineEdit *m_createdByEdit;
    QLineEdit *m_updatedByEdit;
    QDateTimeEdit *m_createdAtEdit;
    QDateTimeEdit *m_updatedAtEdit;
    
    // Buttons
    QDialogButtonBox *m_buttonBox;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif // MATERIALDIALOG_H
