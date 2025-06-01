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
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "searchfilter.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNouveau;
    QAction *actionModifier;
    QAction *actionSupprimer;
    QAction *actionQuitter;
    QAction *actionAPropos;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_3;
    SearchFilter *widgetSearchFilter;
    QTableView *tableViewProjets;
    QGroupBox *groupBoxDetails;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelNom;
    QLabel *labelNomValeur;
    QLabel *labelCategorie;
    QLabel *labelCategorieValeur;
    QLabel *labelDateDebut;
    QLabel *labelDateDebutValeur;
    QLabel *labelDateFin;
    QLabel *labelDateFinValeur;
    QLabel *labelJoursRestants;
    QLabel *labelJoursRestantsValeur;
    QHBoxLayout *horizontalLayout_2;
    QLabel *labelAvancement;
    QProgressBar *progressBarAvancement;
    QMenuBar *menubar;
    QMenu *menuFichier;
    QMenu *menuAide;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        actionNouveau = new QAction(MainWindow);
        actionNouveau->setObjectName("actionNouveau");
        actionModifier = new QAction(MainWindow);
        actionModifier->setObjectName("actionModifier");
        actionSupprimer = new QAction(MainWindow);
        actionSupprimer->setObjectName("actionSupprimer");
        actionQuitter = new QAction(MainWindow);
        actionQuitter->setObjectName("actionQuitter");
        actionAPropos = new QAction(MainWindow);
        actionAPropos->setObjectName("actionAPropos");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_3 = new QVBoxLayout(centralwidget);
        verticalLayout_3->setObjectName("verticalLayout_3");
        widgetSearchFilter = new SearchFilter(centralwidget);
        widgetSearchFilter->setObjectName("widgetSearchFilter");

        verticalLayout_3->addWidget(widgetSearchFilter);

        tableViewProjets = new QTableView(centralwidget);
        tableViewProjets->setObjectName("tableViewProjets");

        verticalLayout_3->addWidget(tableViewProjets);

        groupBoxDetails = new QGroupBox(centralwidget);
        groupBoxDetails->setObjectName("groupBoxDetails");
        verticalLayout_2 = new QVBoxLayout(groupBoxDetails);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelNom = new QLabel(groupBoxDetails);
        labelNom->setObjectName("labelNom");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelNom);

        labelNomValeur = new QLabel(groupBoxDetails);
        labelNomValeur->setObjectName("labelNomValeur");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, labelNomValeur);

        labelCategorie = new QLabel(groupBoxDetails);
        labelCategorie->setObjectName("labelCategorie");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelCategorie);

        labelCategorieValeur = new QLabel(groupBoxDetails);
        labelCategorieValeur->setObjectName("labelCategorieValeur");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, labelCategorieValeur);

        labelDateDebut = new QLabel(groupBoxDetails);
        labelDateDebut->setObjectName("labelDateDebut");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelDateDebut);

        labelDateDebutValeur = new QLabel(groupBoxDetails);
        labelDateDebutValeur->setObjectName("labelDateDebutValeur");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, labelDateDebutValeur);

        labelDateFin = new QLabel(groupBoxDetails);
        labelDateFin->setObjectName("labelDateFin");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelDateFin);

        labelDateFinValeur = new QLabel(groupBoxDetails);
        labelDateFinValeur->setObjectName("labelDateFinValeur");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, labelDateFinValeur);

        labelJoursRestants = new QLabel(groupBoxDetails);
        labelJoursRestants->setObjectName("labelJoursRestants");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelJoursRestants);

        labelJoursRestantsValeur = new QLabel(groupBoxDetails);
        labelJoursRestantsValeur->setObjectName("labelJoursRestantsValeur");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, labelJoursRestantsValeur);


        verticalLayout->addLayout(formLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        labelAvancement = new QLabel(groupBoxDetails);
        labelAvancement->setObjectName("labelAvancement");

        horizontalLayout_2->addWidget(labelAvancement);

        progressBarAvancement = new QProgressBar(groupBoxDetails);
        progressBarAvancement->setObjectName("progressBarAvancement");
        progressBarAvancement->setValue(0);

        horizontalLayout_2->addWidget(progressBarAvancement);


        verticalLayout->addLayout(horizontalLayout_2);


        horizontalLayout->addLayout(verticalLayout);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout_3->addWidget(groupBoxDetails);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 22));
        menuFichier = new QMenu(menubar);
        menuFichier->setObjectName("menuFichier");
        menuAide = new QMenu(menubar);
        menuAide->setObjectName("menuAide");
        MainWindow->setMenuBar(menubar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName("toolBar");
        MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);

        menubar->addAction(menuFichier->menuAction());
        menubar->addAction(menuAide->menuAction());
        menuFichier->addAction(actionNouveau);
        menuFichier->addAction(actionModifier);
        menuFichier->addAction(actionSupprimer);
        menuFichier->addSeparator();
        menuFichier->addAction(actionQuitter);
        menuAide->addAction(actionAPropos);
        toolBar->addAction(actionNouveau);
        toolBar->addAction(actionModifier);
        toolBar->addAction(actionSupprimer);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Gestion des Projets", nullptr));
        actionNouveau->setText(QCoreApplication::translate("MainWindow", "Nouveau projet", nullptr));
#if QT_CONFIG(shortcut)
        actionNouveau->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionModifier->setText(QCoreApplication::translate("MainWindow", "Modifier projet", nullptr));
#if QT_CONFIG(shortcut)
        actionModifier->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+E", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSupprimer->setText(QCoreApplication::translate("MainWindow", "Supprimer projet", nullptr));
#if QT_CONFIG(shortcut)
        actionSupprimer->setShortcut(QCoreApplication::translate("MainWindow", "Del", nullptr));
#endif // QT_CONFIG(shortcut)
        actionQuitter->setText(QCoreApplication::translate("MainWindow", "Quitter", nullptr));
#if QT_CONFIG(shortcut)
        actionQuitter->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAPropos->setText(QCoreApplication::translate("MainWindow", "\303\200 propos", nullptr));
        groupBoxDetails->setTitle(QCoreApplication::translate("MainWindow", "D\303\251tails du projet", nullptr));
        labelNom->setText(QCoreApplication::translate("MainWindow", "Nom:", nullptr));
        labelNomValeur->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        labelCategorie->setText(QCoreApplication::translate("MainWindow", "Cat\303\251gorie:", nullptr));
        labelCategorieValeur->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        labelDateDebut->setText(QCoreApplication::translate("MainWindow", "Date de d\303\251but:", nullptr));
        labelDateDebutValeur->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        labelDateFin->setText(QCoreApplication::translate("MainWindow", "Date de fin estim\303\251e:", nullptr));
        labelDateFinValeur->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        labelJoursRestants->setText(QCoreApplication::translate("MainWindow", "Jours restants:", nullptr));
        labelJoursRestantsValeur->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        labelAvancement->setText(QCoreApplication::translate("MainWindow", "Avancement:", nullptr));
        menuFichier->setTitle(QCoreApplication::translate("MainWindow", "Fichier", nullptr));
        menuAide->setTitle(QCoreApplication::translate("MainWindow", "Aide", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
