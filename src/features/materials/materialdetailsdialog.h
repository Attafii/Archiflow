#ifndef MATERIALDETAILSDIALOG_H
#define MATERIALDETAILSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QGroupBox>
#include <QFrame>
#include "materialmodel.h"

class MaterialDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialDetailsDialog(const Material &material, QWidget *parent = nullptr);

private:
    void setupUI();
    void createInfoSection();
    void createStockSection();
    void createHistorySection();
    void applyStyles();
    
    Material m_material;
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    
    // Header
    QLabel *m_titleLabel;
    QLabel *m_idLabel;
    QLabel *m_statusLabel;
    
    // Info section
    QGroupBox *m_infoGroup;
    QGridLayout *m_infoLayout;
    
    // Stock section
    QGroupBox *m_stockGroup;
    QGridLayout *m_stockLayout;
    
    // History section
    QGroupBox *m_historyGroup;
    QGridLayout *m_historyLayout;
    
    // Actions
    QHBoxLayout *m_buttonsLayout;
    QPushButton *m_editButton;
    QPushButton *m_closeButton;

signals:
    void editRequested();
};

#endif // MATERIALDETAILSDIALOG_H
