/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionExport;
    QAction *actionExportPdf;
    QAction *actionImport;
    QAction *actionExit;
    QAction *actionContractList;
    QAction *actionExpiringContracts;
    QAction *actionValueSummary;
    QToolBar *toolBar;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *filterLayout;
    QHBoxLayout *searchLayout;
    QLabel *searchLabel;
    QLineEdit *searchLineEdit;
    QHBoxLayout *dateFilterLayout;
    QLabel *startDateFilterLabel;
    QDateEdit *startDateFilter;
    QLabel *endDateFilterLabel;
    QDateEdit *endDateFilter;
    QHBoxLayout *valueFilterLayout;
    QLabel *minValueLabel;
    QDoubleSpinBox *minValueSpinBox;
    QLabel *maxValueLabel;
    QDoubleSpinBox *maxValueSpinBox;
    QHBoxLayout *statusFilterLayout;
    QComboBox *filterComboBox;
    QHBoxLayout *mainLayout;
    QVBoxLayout *tableLayout;
    QTableWidget *contractTableWidget;
    QHBoxLayout *buttonLayout;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *exportPdfButton;
    QPushButton *statisticsButton;
    QGroupBox *detailsGroupBox;
    QFormLayout *formLayout;
    QLabel *contractIdLabel;
    QLineEdit *contractIdLineEdit;
    QLabel *clientNameLabel;
    QLineEdit *clientNameLineEdit;
    QLabel *startDateLabel;
    QDateEdit *startDateEdit;
    QLabel *endDateLabel;
    QDateEdit *endDateEdit;
    QLabel *valueLabel;
    QDoubleSpinBox *valueSpinBox;
    QLabel *statusLabel;
    QComboBox *statusComboBox;
    QLabel *descriptionLabel;
    QTextEdit *descriptionTextEdit;
    QPushButton *saveButton;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuReports;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        actionExport = new QAction(MainWindow);
        actionExport->setObjectName("actionExport");
        actionExportPdf = new QAction(MainWindow);
        actionExportPdf->setObjectName("actionExportPdf");
        actionImport = new QAction(MainWindow);
        actionImport->setObjectName("actionImport");
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName("actionExit");
        actionContractList = new QAction(MainWindow);
        actionContractList->setObjectName("actionContractList");
        actionExpiringContracts = new QAction(MainWindow);
        actionExpiringContracts->setObjectName("actionExpiringContracts");
        actionValueSummary = new QAction(MainWindow);
        actionValueSummary->setObjectName("actionValueSummary");
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName("toolBar");
        MainWindow->addToolBar(toolBar);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setStyleSheet(QString::fromUtf8("QWidget#centralwidget { background-color: #3D485A; }\n"
"QLabel { color: #E3C6B0; font-family: 'Poppins'; }\n"
"QTableWidget { background-color: white; border-radius: 5px; color: black; }\n"
"QPushButton { background-color: #E3C6B0; color: #3D485A; border-radius: 5px; padding: 5px 15px; font-weight: bold; }\n"
"QPushButton:hover { background-color: #C4A491; }\n"
"QLineEdit, QDateEdit, QDoubleSpinBox, QComboBox, QTextEdit { background-color: white; border-radius: 3px; padding: 3px; color: black; }"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        filterLayout = new QVBoxLayout();
        filterLayout->setObjectName("filterLayout");
        searchLayout = new QHBoxLayout();
        searchLayout->setObjectName("searchLayout");
        searchLabel = new QLabel(centralwidget);
        searchLabel->setObjectName("searchLabel");

        searchLayout->addWidget(searchLabel);

        searchLineEdit = new QLineEdit(centralwidget);
        searchLineEdit->setObjectName("searchLineEdit");

        searchLayout->addWidget(searchLineEdit);


        filterLayout->addLayout(searchLayout);

        dateFilterLayout = new QHBoxLayout();
        dateFilterLayout->setObjectName("dateFilterLayout");
        startDateFilterLabel = new QLabel(centralwidget);
        startDateFilterLabel->setObjectName("startDateFilterLabel");

        dateFilterLayout->addWidget(startDateFilterLabel);

        startDateFilter = new QDateEdit(centralwidget);
        startDateFilter->setObjectName("startDateFilter");
        startDateFilter->setCalendarPopup(true);

        dateFilterLayout->addWidget(startDateFilter);

        endDateFilterLabel = new QLabel(centralwidget);
        endDateFilterLabel->setObjectName("endDateFilterLabel");

        dateFilterLayout->addWidget(endDateFilterLabel);

        endDateFilter = new QDateEdit(centralwidget);
        endDateFilter->setObjectName("endDateFilter");
        endDateFilter->setCalendarPopup(true);

        dateFilterLayout->addWidget(endDateFilter);


        filterLayout->addLayout(dateFilterLayout);

        valueFilterLayout = new QHBoxLayout();
        valueFilterLayout->setObjectName("valueFilterLayout");
        minValueLabel = new QLabel(centralwidget);
        minValueLabel->setObjectName("minValueLabel");

        valueFilterLayout->addWidget(minValueLabel);

        minValueSpinBox = new QDoubleSpinBox(centralwidget);
        minValueSpinBox->setObjectName("minValueSpinBox");
        minValueSpinBox->setDecimals(2);
        minValueSpinBox->setMaximum(999999999.990000009536743);

        valueFilterLayout->addWidget(minValueSpinBox);

        maxValueLabel = new QLabel(centralwidget);
        maxValueLabel->setObjectName("maxValueLabel");

        valueFilterLayout->addWidget(maxValueLabel);

        maxValueSpinBox = new QDoubleSpinBox(centralwidget);
        maxValueSpinBox->setObjectName("maxValueSpinBox");
        maxValueSpinBox->setDecimals(2);
        maxValueSpinBox->setMaximum(999999999.990000009536743);

        valueFilterLayout->addWidget(maxValueSpinBox);


        filterLayout->addLayout(valueFilterLayout);

        statusFilterLayout = new QHBoxLayout();
        statusFilterLayout->setObjectName("statusFilterLayout");
        filterComboBox = new QComboBox(centralwidget);
        filterComboBox->addItem(QString());
        filterComboBox->addItem(QString());
        filterComboBox->addItem(QString());
        filterComboBox->addItem(QString());
        filterComboBox->setObjectName("filterComboBox");

        statusFilterLayout->addWidget(filterComboBox);


        filterLayout->addLayout(statusFilterLayout);


        verticalLayout->addLayout(filterLayout);

        mainLayout = new QHBoxLayout();
        mainLayout->setObjectName("mainLayout");
        tableLayout = new QVBoxLayout();
        tableLayout->setObjectName("tableLayout");
        contractTableWidget = new QTableWidget(centralwidget);
        if (contractTableWidget->columnCount() < 6)
            contractTableWidget->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        contractTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        contractTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        contractTableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        contractTableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        contractTableWidget->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        contractTableWidget->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        contractTableWidget->setObjectName("contractTableWidget");

        tableLayout->addWidget(contractTableWidget);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        addButton = new QPushButton(centralwidget);
        addButton->setObjectName("addButton");

        buttonLayout->addWidget(addButton);

        editButton = new QPushButton(centralwidget);
        editButton->setObjectName("editButton");

        buttonLayout->addWidget(editButton);

        deleteButton = new QPushButton(centralwidget);
        deleteButton->setObjectName("deleteButton");

        buttonLayout->addWidget(deleteButton);

        exportPdfButton = new QPushButton(centralwidget);
        exportPdfButton->setObjectName("exportPdfButton");

        buttonLayout->addWidget(exportPdfButton);

        statisticsButton = new QPushButton(centralwidget);
        statisticsButton->setObjectName("statisticsButton");

        buttonLayout->addWidget(statisticsButton);


        tableLayout->addLayout(buttonLayout);


        mainLayout->addLayout(tableLayout);

        detailsGroupBox = new QGroupBox(centralwidget);
        detailsGroupBox->setObjectName("detailsGroupBox");
        formLayout = new QFormLayout(detailsGroupBox);
        formLayout->setObjectName("formLayout");
        contractIdLabel = new QLabel(detailsGroupBox);
        contractIdLabel->setObjectName("contractIdLabel");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, contractIdLabel);

        contractIdLineEdit = new QLineEdit(detailsGroupBox);
        contractIdLineEdit->setObjectName("contractIdLineEdit");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, contractIdLineEdit);

        clientNameLabel = new QLabel(detailsGroupBox);
        clientNameLabel->setObjectName("clientNameLabel");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, clientNameLabel);

        clientNameLineEdit = new QLineEdit(detailsGroupBox);
        clientNameLineEdit->setObjectName("clientNameLineEdit");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, clientNameLineEdit);

        startDateLabel = new QLabel(detailsGroupBox);
        startDateLabel->setObjectName("startDateLabel");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, startDateLabel);

        startDateEdit = new QDateEdit(detailsGroupBox);
        startDateEdit->setObjectName("startDateEdit");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, startDateEdit);

        endDateLabel = new QLabel(detailsGroupBox);
        endDateLabel->setObjectName("endDateLabel");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, endDateLabel);

        endDateEdit = new QDateEdit(detailsGroupBox);
        endDateEdit->setObjectName("endDateEdit");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, endDateEdit);

        valueLabel = new QLabel(detailsGroupBox);
        valueLabel->setObjectName("valueLabel");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, valueLabel);

        valueSpinBox = new QDoubleSpinBox(detailsGroupBox);
        valueSpinBox->setObjectName("valueSpinBox");
        valueSpinBox->setMaximum(999999999.990000009536743);

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, valueSpinBox);

        statusLabel = new QLabel(detailsGroupBox);
        statusLabel->setObjectName("statusLabel");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, statusLabel);

        statusComboBox = new QComboBox(detailsGroupBox);
        statusComboBox->addItem(QString());
        statusComboBox->addItem(QString());
        statusComboBox->addItem(QString());
        statusComboBox->addItem(QString());
        statusComboBox->addItem(QString());
        statusComboBox->setObjectName("statusComboBox");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, statusComboBox);

        descriptionLabel = new QLabel(detailsGroupBox);
        descriptionLabel->setObjectName("descriptionLabel");

        formLayout->setWidget(6, QFormLayout::ItemRole::LabelRole, descriptionLabel);

        descriptionTextEdit = new QTextEdit(detailsGroupBox);
        descriptionTextEdit->setObjectName("descriptionTextEdit");

        formLayout->setWidget(6, QFormLayout::ItemRole::FieldRole, descriptionTextEdit);

        saveButton = new QPushButton(detailsGroupBox);
        saveButton->setObjectName("saveButton");

        formLayout->setWidget(7, QFormLayout::ItemRole::SpanningRole, saveButton);


        mainLayout->addWidget(detailsGroupBox);


        verticalLayout->addLayout(mainLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuReports = new QMenu(menubar);
        menuReports->setObjectName("menuReports");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menuFile->addAction(actionExport);
        menuFile->addAction(actionExportPdf);
        menuFile->addAction(actionImport);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuReports->addAction(actionContractList);
        menuReports->addAction(actionExpiringContracts);
        menuReports->addAction(actionValueSummary);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Contract Management System", nullptr));
        actionExport->setText(QCoreApplication::translate("MainWindow", "Export Contracts", nullptr));
        actionExportPdf->setText(QCoreApplication::translate("MainWindow", "Export PDF", nullptr));
        actionImport->setText(QCoreApplication::translate("MainWindow", "Import Contracts", nullptr));
        actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
        actionContractList->setText(QCoreApplication::translate("MainWindow", "Contract List", nullptr));
        actionExpiringContracts->setText(QCoreApplication::translate("MainWindow", "Expiring Contracts", nullptr));
        actionValueSummary->setText(QCoreApplication::translate("MainWindow", "Value Summary", nullptr));
        searchLabel->setText(QCoreApplication::translate("MainWindow", "Search:", nullptr));
        searchLineEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Enter contract number, client name or description...", nullptr));
        startDateFilterLabel->setText(QCoreApplication::translate("MainWindow", "Start Date:", nullptr));
        endDateFilterLabel->setText(QCoreApplication::translate("MainWindow", "End Date:", nullptr));
        minValueLabel->setText(QCoreApplication::translate("MainWindow", "Min Value:", nullptr));
        minValueSpinBox->setPrefix(QCoreApplication::translate("MainWindow", "$", nullptr));
        maxValueLabel->setText(QCoreApplication::translate("MainWindow", "Max Value:", nullptr));
        maxValueSpinBox->setPrefix(QCoreApplication::translate("MainWindow", "$", nullptr));
        filterComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "All Contracts", nullptr));
        filterComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "Active Contracts", nullptr));
        filterComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "Expired Contracts", nullptr));
        filterComboBox->setItemText(3, QCoreApplication::translate("MainWindow", "Pending Approval", nullptr));

        QTableWidgetItem *___qtablewidgetitem = contractTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "Contract ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = contractTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Client Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = contractTableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Start Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = contractTableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "End Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = contractTableWidget->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Value", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = contractTableWidget->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "Status", nullptr));
        addButton->setText(QCoreApplication::translate("MainWindow", "New Contract", nullptr));
        editButton->setText(QCoreApplication::translate("MainWindow", "Edit Contract", nullptr));
        deleteButton->setText(QCoreApplication::translate("MainWindow", "Delete Contract", nullptr));
        exportPdfButton->setText(QCoreApplication::translate("MainWindow", "Export PDF", nullptr));
        statisticsButton->setText(QCoreApplication::translate("MainWindow", "Statistics", nullptr));
        detailsGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Contract Details", nullptr));
        contractIdLabel->setText(QCoreApplication::translate("MainWindow", "Contract ID:", nullptr));
        clientNameLabel->setText(QCoreApplication::translate("MainWindow", "Client Name:", nullptr));
        startDateLabel->setText(QCoreApplication::translate("MainWindow", "Start Date:", nullptr));
        endDateLabel->setText(QCoreApplication::translate("MainWindow", "End Date:", nullptr));
        valueLabel->setText(QCoreApplication::translate("MainWindow", "Contract Value:", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "Status:", nullptr));
        statusComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "Draft", nullptr));
        statusComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "Pending Approval", nullptr));
        statusComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "Active", nullptr));
        statusComboBox->setItemText(3, QCoreApplication::translate("MainWindow", "Completed", nullptr));
        statusComboBox->setItemText(4, QCoreApplication::translate("MainWindow", "Terminated", nullptr));

        descriptionLabel->setText(QCoreApplication::translate("MainWindow", "Description:", nullptr));
        saveButton->setText(QCoreApplication::translate("MainWindow", "Save Changes", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuReports->setTitle(QCoreApplication::translate("MainWindow", "Reports", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
