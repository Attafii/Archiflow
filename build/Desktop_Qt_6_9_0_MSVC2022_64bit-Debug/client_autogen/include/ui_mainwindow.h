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
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *formLayout;
    QLabel *titleLabel;
    QFormLayout *inputLayout;
    QLineEdit *clientNameEdit;
    QLineEdit *projectNameEdit;
    QLineEdit *locationEdit;
    QLineEdit *postcodeEdit;
    QLineEdit *contactPhoneEdit;
    QLineEdit *emailEdit;
    QLineEdit *serviceAreaEdit;
    QTextEdit *descriptionEdit;
    QPushButton *submitButton;
    QWebEngineView *mapView;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName("horizontalLayout");
        formLayout = new QVBoxLayout();
        formLayout->setObjectName("formLayout");
        titleLabel = new QLabel(centralwidget);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("\n"
"          font-size: 40px;\n"
"          font-weight: bold;\n"
"          margin-bottom: 20px;\n"
"         "));

        formLayout->addWidget(titleLabel);

        inputLayout = new QFormLayout();
        inputLayout->setObjectName("inputLayout");
        clientNameEdit = new QLineEdit(centralwidget);
        clientNameEdit->setObjectName("clientNameEdit");

        inputLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, clientNameEdit);

        projectNameEdit = new QLineEdit(centralwidget);
        projectNameEdit->setObjectName("projectNameEdit");

        inputLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, projectNameEdit);

        locationEdit = new QLineEdit(centralwidget);
        locationEdit->setObjectName("locationEdit");

        inputLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, locationEdit);

        postcodeEdit = new QLineEdit(centralwidget);
        postcodeEdit->setObjectName("postcodeEdit");

        inputLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, postcodeEdit);

        contactPhoneEdit = new QLineEdit(centralwidget);
        contactPhoneEdit->setObjectName("contactPhoneEdit");

        inputLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, contactPhoneEdit);

        emailEdit = new QLineEdit(centralwidget);
        emailEdit->setObjectName("emailEdit");

        inputLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, emailEdit);

        serviceAreaEdit = new QLineEdit(centralwidget);
        serviceAreaEdit->setObjectName("serviceAreaEdit");

        inputLayout->setWidget(6, QFormLayout::ItemRole::FieldRole, serviceAreaEdit);

        descriptionEdit = new QTextEdit(centralwidget);
        descriptionEdit->setObjectName("descriptionEdit");
        descriptionEdit->setAutoFillBackground(false);

        inputLayout->setWidget(7, QFormLayout::ItemRole::FieldRole, descriptionEdit);

        submitButton = new QPushButton(centralwidget);
        submitButton->setObjectName("submitButton");

        inputLayout->setWidget(8, QFormLayout::ItemRole::FieldRole, submitButton);


        formLayout->addLayout(inputLayout);


        horizontalLayout->addLayout(formLayout);

        mapView = new QWebEngineView(centralwidget);
        mapView->setObjectName("mapView");
        mapView->setMinimumSize(QSize(400, 0));

        horizontalLayout->addWidget(mapView);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Client Form", nullptr));
        MainWindow->setStyleSheet(QCoreApplication::translate("MainWindow", "\n"
"    QMainWindow {\n"
"      background-color: white;\n"
"      font-family: 'Roboto', Arial, sans-serif;\n"
"    }\n"
"    QLineEdit, QTextEdit {\n"
"      font-family: 'Roboto', Arial, sans-serif;\n"
"      font-size: 14px;\n"
"      padding: 0 0 2px 0;\n"
"      border: none;\n"
"      border-bottom: 1px solid #bbb;\n"
"      border-radius: 0;\n"
"      background: transparent;\n"
"      outline: none;\n"
"    }\n"
"    QLineEdit:focus, QTextEdit:focus {\n"
"      border-bottom: 1px solid #0078d7;\n"
"    }\n"
"    QPushButton#submitButton {\n"
"      background-color: #0078d7;\n"
"      color: white;\n"
"      border: none;\n"
"      border-radius: 4px;\n"
"      padding: 10px 0;\n"
"      font-weight: bold;\n"
"      font-family: 'Roboto', Arial, sans-serif;\n"
"      font-size: 14px;\n"
"      letter-spacing: 1px;\n"
"    }\n"
"    QPushButton#submitButton:hover {\n"
"      background-color: #0063b1;\n"
"    }\n"
"    QWebEngineView {\n"
"      background-color:rgb(180, 180, 180);\n"
"      backgroun"
                        "d-repeat: no-repeat;\n"
"      background-position: center;\n"
"      border: 1px solid #ddd;\n"
"      border-radius: 4px;\n"
"    }\n"
"   ", nullptr));
        titleLabel->setText(QCoreApplication::translate("MainWindow", "Client Form", nullptr));
        clientNameEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Client Name", nullptr));
        projectNameEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Project Name", nullptr));
        locationEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Location", nullptr));
        postcodeEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Postcode", nullptr));
        contactPhoneEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Contact Phone", nullptr));
        emailEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "E-mail", nullptr));
        serviceAreaEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Surface Area", nullptr));
        descriptionEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Project Description", nullptr));
        submitButton->setText(QCoreApplication::translate("MainWindow", "SUBMIT", nullptr));
        mapView->setStyleSheet(QCoreApplication::translate("MainWindow", "\n"
"        QWebEngineView {\n"
"          background-color:rgb(180, 180, 180);\n"
"          background-repeat: no-repeat;\n"
"          background-position: center;\n"
"          border: 1px solid #ddd;\n"
"          border-radius: 4px;\n"
"        }\n"
"       ", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
