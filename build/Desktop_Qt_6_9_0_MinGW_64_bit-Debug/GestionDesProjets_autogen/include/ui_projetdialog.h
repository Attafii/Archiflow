/********************************************************************************
** Form generated from reading UI file 'projetdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROJETDIALOG_H
#define UI_PROJETDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ProjetDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelNom;
    QLineEdit *lineEditNom;
    QLabel *labelCategorie;
    QComboBox *comboBoxCategorie;
    QLabel *labelDateDebut;
    QDateEdit *dateEditDebut;
    QLabel *labelDateFin;
    QDateEdit *dateEditFin;
    QLabel *labelComplexite;
    QHBoxLayout *horizontalLayout;
    QSpinBox *spinBoxComplexite;
    QLabel *labelDureeEstimee;
    QLabel *labelProgression;
    QHBoxLayout *horizontalLayout_3;
    QSpinBox *spinBoxProgression;
    QProgressBar *progressBar;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ProjetDialog)
    {
        if (ProjetDialog->objectName().isEmpty())
            ProjetDialog->setObjectName("ProjetDialog");
        ProjetDialog->resize(450, 350);
        verticalLayout = new QVBoxLayout(ProjetDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelNom = new QLabel(ProjetDialog);
        labelNom->setObjectName("labelNom");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelNom);

        lineEditNom = new QLineEdit(ProjetDialog);
        lineEditNom->setObjectName("lineEditNom");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineEditNom);

        labelCategorie = new QLabel(ProjetDialog);
        labelCategorie->setObjectName("labelCategorie");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelCategorie);

        comboBoxCategorie = new QComboBox(ProjetDialog);
        comboBoxCategorie->setObjectName("comboBoxCategorie");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, comboBoxCategorie);

        labelDateDebut = new QLabel(ProjetDialog);
        labelDateDebut->setObjectName("labelDateDebut");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelDateDebut);

        dateEditDebut = new QDateEdit(ProjetDialog);
        dateEditDebut->setObjectName("dateEditDebut");
        dateEditDebut->setCalendarPopup(true);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, dateEditDebut);

        labelDateFin = new QLabel(ProjetDialog);
        labelDateFin->setObjectName("labelDateFin");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelDateFin);

        dateEditFin = new QDateEdit(ProjetDialog);
        dateEditFin->setObjectName("dateEditFin");
        dateEditFin->setCalendarPopup(true);

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, dateEditFin);

        labelComplexite = new QLabel(ProjetDialog);
        labelComplexite->setObjectName("labelComplexite");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelComplexite);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        spinBoxComplexite = new QSpinBox(ProjetDialog);
        spinBoxComplexite->setObjectName("spinBoxComplexite");
        spinBoxComplexite->setMinimum(1);
        spinBoxComplexite->setMaximum(5);
        spinBoxComplexite->setValue(3);

        horizontalLayout->addWidget(spinBoxComplexite);

        labelDureeEstimee = new QLabel(ProjetDialog);
        labelDureeEstimee->setObjectName("labelDureeEstimee");

        horizontalLayout->addWidget(labelDureeEstimee);


        formLayout->setLayout(4, QFormLayout::ItemRole::FieldRole, horizontalLayout);

        labelProgression = new QLabel(ProjetDialog);
        labelProgression->setObjectName("labelProgression");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, labelProgression);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        spinBoxProgression = new QSpinBox(ProjetDialog);
        spinBoxProgression->setObjectName("spinBoxProgression");
        spinBoxProgression->setMaximum(100);

        horizontalLayout_3->addWidget(spinBoxProgression);

        progressBar = new QProgressBar(ProjetDialog);
        progressBar->setObjectName("progressBar");
        progressBar->setValue(0);

        horizontalLayout_3->addWidget(progressBar);


        formLayout->setLayout(5, QFormLayout::ItemRole::FieldRole, horizontalLayout_3);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(ProjetDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ProjetDialog);

        QMetaObject::connectSlotsByName(ProjetDialog);
    } // setupUi

    void retranslateUi(QDialog *ProjetDialog)
    {
        ProjetDialog->setWindowTitle(QCoreApplication::translate("ProjetDialog", "Projet", nullptr));
        labelNom->setText(QCoreApplication::translate("ProjetDialog", "Nom:", nullptr));
        lineEditNom->setPlaceholderText(QCoreApplication::translate("ProjetDialog", "Entrez le nom du projet", nullptr));
        labelCategorie->setText(QCoreApplication::translate("ProjetDialog", "Cat\303\251gorie:", nullptr));
        labelDateDebut->setText(QCoreApplication::translate("ProjetDialog", "Date de d\303\251but:", nullptr));
        labelDateFin->setText(QCoreApplication::translate("ProjetDialog", "Date de fin estim\303\251e:", nullptr));
        labelComplexite->setText(QCoreApplication::translate("ProjetDialog", "Complexit\303\251:", nullptr));
        labelDureeEstimee->setText(QCoreApplication::translate("ProjetDialog", "90 jours", nullptr));
        labelProgression->setText(QCoreApplication::translate("ProjetDialog", "Progression:", nullptr));
        spinBoxProgression->setSuffix(QCoreApplication::translate("ProjetDialog", "%", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ProjetDialog: public Ui_ProjetDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROJETDIALOG_H
