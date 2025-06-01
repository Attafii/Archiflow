/********************************************************************************
** Form generated from reading UI file 'searchfilter.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEARCHFILTER_H
#define UI_SEARCHFILTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SearchFilter
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEditSearch;
    QComboBox *comboBoxCategorie;
    QPushButton *btnSearch;
    QPushButton *btnReset;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *checkBoxDateRange;
    QDateEdit *dateEditDebut;
    QLabel *labelTo;
    QDateEdit *dateEditFin;
    QSpacerItem *horizontalSpacer;
    QLabel *labelSort;
    QComboBox *comboBoxSort;
    QPushButton *btnSortDirection;

    void setupUi(QWidget *SearchFilter)
    {
        if (SearchFilter->objectName().isEmpty())
            SearchFilter->setObjectName("SearchFilter");
        SearchFilter->resize(600, 120);
        verticalLayout = new QVBoxLayout(SearchFilter);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        lineEditSearch = new QLineEdit(SearchFilter);
        lineEditSearch->setObjectName("lineEditSearch");

        horizontalLayout->addWidget(lineEditSearch);

        comboBoxCategorie = new QComboBox(SearchFilter);
        comboBoxCategorie->setObjectName("comboBoxCategorie");

        horizontalLayout->addWidget(comboBoxCategorie);

        btnSearch = new QPushButton(SearchFilter);
        btnSearch->setObjectName("btnSearch");

        horizontalLayout->addWidget(btnSearch);

        btnReset = new QPushButton(SearchFilter);
        btnReset->setObjectName("btnReset");

        horizontalLayout->addWidget(btnReset);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        checkBoxDateRange = new QCheckBox(SearchFilter);
        checkBoxDateRange->setObjectName("checkBoxDateRange");

        horizontalLayout_2->addWidget(checkBoxDateRange);

        dateEditDebut = new QDateEdit(SearchFilter);
        dateEditDebut->setObjectName("dateEditDebut");
        dateEditDebut->setEnabled(false);
        dateEditDebut->setCalendarPopup(true);

        horizontalLayout_2->addWidget(dateEditDebut);

        labelTo = new QLabel(SearchFilter);
        labelTo->setObjectName("labelTo");

        horizontalLayout_2->addWidget(labelTo);

        dateEditFin = new QDateEdit(SearchFilter);
        dateEditFin->setObjectName("dateEditFin");
        dateEditFin->setEnabled(false);
        dateEditFin->setCalendarPopup(true);

        horizontalLayout_2->addWidget(dateEditFin);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        labelSort = new QLabel(SearchFilter);
        labelSort->setObjectName("labelSort");

        horizontalLayout_2->addWidget(labelSort);

        comboBoxSort = new QComboBox(SearchFilter);
        comboBoxSort->setObjectName("comboBoxSort");

        horizontalLayout_2->addWidget(comboBoxSort);

        btnSortDirection = new QPushButton(SearchFilter);
        btnSortDirection->setObjectName("btnSortDirection");
        btnSortDirection->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_2->addWidget(btnSortDirection);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(SearchFilter);

        QMetaObject::connectSlotsByName(SearchFilter);
    } // setupUi

    void retranslateUi(QWidget *SearchFilter)
    {
        SearchFilter->setWindowTitle(QCoreApplication::translate("SearchFilter", "Form", nullptr));
        lineEditSearch->setPlaceholderText(QCoreApplication::translate("SearchFilter", "Rechercher un projet...", nullptr));
        btnSearch->setText(QCoreApplication::translate("SearchFilter", "Rechercher", nullptr));
        btnReset->setText(QCoreApplication::translate("SearchFilter", "R\303\251initialiser", nullptr));
        checkBoxDateRange->setText(QCoreApplication::translate("SearchFilter", "Filtrer par dates", nullptr));
        labelTo->setText(QCoreApplication::translate("SearchFilter", "\303\240", nullptr));
        labelSort->setText(QCoreApplication::translate("SearchFilter", "Trier par:", nullptr));
        btnSortDirection->setText(QCoreApplication::translate("SearchFilter", "\342\206\221", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SearchFilter: public Ui_SearchFilter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEARCHFILTER_H
