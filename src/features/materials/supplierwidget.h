#ifndef SUPPLIERWIDGET_H
#define SUPPLIERWIDGET_H

#include "suppliermodel.h"
#include "supplierdialog.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QGroupBox>
#include <QSplitter>
#include <QTextEdit>

/**
 * @brief Widget for managing suppliers within the materials feature
 */
class SupplierWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SupplierWidget(QWidget *parent = nullptr);
    
    // Public interface for materials module
    QList<Supplier> getActiveSuppliers() const;
    Supplier getSupplierById(int id) const;
    QString getSupplierName(int id) const;

signals:
    void supplierSelected(int supplierId);
    void suppliersUpdated();

private slots:
    void onAddSupplier();
    void onEditSupplier();
    void onDeleteSupplier();
    void onSearchTextChanged(const QString &text);
    void onSelectionChanged();
    void onSupplierDoubleClicked(const QModelIndex &index);
    void refreshView();

private:    void setupUI();
    void setupConnections();
    void updateButtons();
    void updateStatusLabel();
    void showSupplierDetails(const Supplier &supplier);
    int getSelectedRow() const;

    // Main layout
    QVBoxLayout *m_mainLayout;
    QSplitter *m_splitter;
    
    // Toolbar
    QHBoxLayout *m_toolbarLayout;
    QLineEdit *m_searchEdit;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_refreshButton;
    
    // Supplier table
    QTableView *m_supplierTable;
    SupplierModel *m_supplierModel;
    QSortFilterProxyModel *m_proxyModel;
    
    // Details panel
    QGroupBox *m_detailsGroup;
    QVBoxLayout *m_detailsLayout;
    QTextEdit *m_detailsText;
    
    // Status
    QLabel *m_statusLabel;
};

#endif // SUPPLIERWIDGET_H
