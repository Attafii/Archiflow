/********************************************************************************
** Form generated from reading UI file 'clientmanager.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENTMANAGER_H
#define UI_CLIENTMANAGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ClientManager
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QLabel *titleLabel;
    QHBoxLayout *searchLayout;
    QLabel *searchLabel;
    QLineEdit *searchEdit;
    QTableWidget *clientsTable;
    QHBoxLayout *buttonsLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;

    void setupUi(QMainWindow *ClientManager)
    {
        if (ClientManager->objectName().isEmpty())
            ClientManager->setObjectName("ClientManager");
        ClientManager->resize(1000, 600);
        centralwidget = new QWidget(ClientManager);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        titleLabel = new QLabel(centralwidget);
        titleLabel->setObjectName("titleLabel");

        verticalLayout->addWidget(titleLabel);

        searchLayout = new QHBoxLayout();
        searchLayout->setObjectName("searchLayout");
        searchLabel = new QLabel(centralwidget);
        searchLabel->setObjectName("searchLabel");

        searchLayout->addWidget(searchLabel);

        searchEdit = new QLineEdit(centralwidget);
        searchEdit->setObjectName("searchEdit");

        searchLayout->addWidget(searchEdit);


        verticalLayout->addLayout(searchLayout);

        clientsTable = new QTableWidget(centralwidget);
        if (clientsTable->columnCount() < 5)
            clientsTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        clientsTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        clientsTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        clientsTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        clientsTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        clientsTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        clientsTable->setObjectName("clientsTable");
        clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        clientsTable->setSelectionMode(QAbstractItemView::SingleSelection);

        verticalLayout->addWidget(clientsTable);

        buttonsLayout = new QHBoxLayout();
        buttonsLayout->setObjectName("buttonsLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        buttonsLayout->addItem(horizontalSpacer);

        addButton = new QPushButton(centralwidget);
        addButton->setObjectName("addButton");

        buttonsLayout->addWidget(addButton);

        editButton = new QPushButton(centralwidget);
        editButton->setObjectName("editButton");

        buttonsLayout->addWidget(editButton);

        deleteButton = new QPushButton(centralwidget);
        deleteButton->setObjectName("deleteButton");

        buttonsLayout->addWidget(deleteButton);


        verticalLayout->addLayout(buttonsLayout);

        ClientManager->setCentralWidget(centralwidget);

        retranslateUi(ClientManager);

        QMetaObject::connectSlotsByName(ClientManager);
    } // setupUi

    void retranslateUi(QMainWindow *ClientManager)
    {
        ClientManager->setWindowTitle(QCoreApplication::translate("ClientManager", "Client Manager", nullptr));
        ClientManager->setStyleSheet(QCoreApplication::translate("ClientManager", "\n"
"    QMainWindow {\n"
"      background-color: white;\n"
"    }\n"
"    QLineEdit, QTextEdit {\n"
"      padding: 8px;\n"
"      border: 1px solid #ddd;\n"
"      border-radius: 4px;\n"
"      background-color: white;\n"
"    }\n"
"    QPushButton#addButton, QPushButton#editButton, QPushButton#deleteButton {\n"
"      background-color: #4c4cfa;\n"
"      color: white;\n"
"      border: none;\n"
"      border-radius: 4px;\n"
"      padding: 10px;\n"
"      font-weight: bold;\n"
"    }\n"
"    QPushButton#deleteButton {\n"
"      background-color: #fa4c4c;\n"
"    }\n"
"    QTableWidget {\n"
"      border: 1px solid #ddd;\n"
"      border-radius: 4px;\n"
"      background-color: white;\n"
"    }\n"
"   ", nullptr));
        titleLabel->setText(QCoreApplication::translate("ClientManager", "Client Manager", nullptr));
        titleLabel->setStyleSheet(QCoreApplication::translate("ClientManager", "\n"
"        font-size: 24px;\n"
"        font-weight: bold;\n"
"        margin-bottom: 20px;\n"
"       ", nullptr));
        searchLabel->setText(QCoreApplication::translate("ClientManager", "Search:", nullptr));
        searchEdit->setPlaceholderText(QCoreApplication::translate("ClientManager", "Search by client name, project, location...", nullptr));
        QTableWidgetItem *___qtablewidgetitem = clientsTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("ClientManager", "Client Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = clientsTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("ClientManager", "Project Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = clientsTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("ClientManager", "Location", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = clientsTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("ClientManager", "Contact Phone", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = clientsTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("ClientManager", "Email", nullptr));
        addButton->setText(QCoreApplication::translate("ClientManager", "ADD CLIENT", nullptr));
        editButton->setText(QCoreApplication::translate("ClientManager", "EDIT CLIENT", nullptr));
        deleteButton->setText(QCoreApplication::translate("ClientManager", "DELETE CLIENT", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ClientManager: public Ui_ClientManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENTMANAGER_H
