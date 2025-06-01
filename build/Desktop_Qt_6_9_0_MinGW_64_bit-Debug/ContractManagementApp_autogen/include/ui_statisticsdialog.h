/********************************************************************************
** Form generated from reading UI file 'statisticsdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STATISTICSDIALOG_H
#define UI_STATISTICSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StatisticsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *summaryGroupBox;
    QGridLayout *gridLayout;
    QLabel *titleLabel;
    QLabel *label;
    QLabel *totalValueLabel;
    QLabel *avgValueLabel;
    QLabel *avgValueDisplay;
    QLabel *avgDurationLabel;
    QLabel *avgDurationDisplay;
    QLabel *statusTitle;
    QLabel *label_2;
    QLabel *activeCountLabel;
    QLabel *label_3;
    QLabel *pendingCountLabel;
    QLabel *label_4;
    QLabel *completedCountLabel;
    QLabel *label_5;
    QLabel *expiredCountLabel;
    QWidget *chartsWidget;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *StatisticsDialog)
    {
        if (StatisticsDialog->objectName().isEmpty())
            StatisticsDialog->setObjectName("StatisticsDialog");
        StatisticsDialog->resize(800, 600);
        verticalLayout = new QVBoxLayout(StatisticsDialog);
        verticalLayout->setObjectName("verticalLayout");
        summaryGroupBox = new QGroupBox(StatisticsDialog);
        summaryGroupBox->setObjectName("summaryGroupBox");
        gridLayout = new QGridLayout(summaryGroupBox);
        gridLayout->setObjectName("gridLayout");
        titleLabel = new QLabel(summaryGroupBox);
        titleLabel->setObjectName("titleLabel");
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        titleLabel->setFont(font);
        titleLabel->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(titleLabel, 0, 0, 1, 2);

        label = new QLabel(summaryGroupBox);
        label->setObjectName("label");
        QFont font1;
        font1.setBold(true);
        label->setFont(font1);

        gridLayout->addWidget(label, 1, 0, 1, 1);

        totalValueLabel = new QLabel(summaryGroupBox);
        totalValueLabel->setObjectName("totalValueLabel");

        gridLayout->addWidget(totalValueLabel, 1, 1, 1, 1);

        avgValueLabel = new QLabel(summaryGroupBox);
        avgValueLabel->setObjectName("avgValueLabel");
        avgValueLabel->setFont(font1);

        gridLayout->addWidget(avgValueLabel, 2, 0, 1, 1);

        avgValueDisplay = new QLabel(summaryGroupBox);
        avgValueDisplay->setObjectName("avgValueDisplay");

        gridLayout->addWidget(avgValueDisplay, 2, 1, 1, 1);

        avgDurationLabel = new QLabel(summaryGroupBox);
        avgDurationLabel->setObjectName("avgDurationLabel");
        avgDurationLabel->setFont(font1);

        gridLayout->addWidget(avgDurationLabel, 3, 0, 1, 1);

        avgDurationDisplay = new QLabel(summaryGroupBox);
        avgDurationDisplay->setObjectName("avgDurationDisplay");

        gridLayout->addWidget(avgDurationDisplay, 3, 1, 1, 1);

        statusTitle = new QLabel(summaryGroupBox);
        statusTitle->setObjectName("statusTitle");
        QFont font2;
        font2.setPointSize(10);
        font2.setBold(true);
        statusTitle->setFont(font2);
        statusTitle->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(statusTitle, 4, 0, 1, 2);

        label_2 = new QLabel(summaryGroupBox);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 5, 0, 1, 1);

        activeCountLabel = new QLabel(summaryGroupBox);
        activeCountLabel->setObjectName("activeCountLabel");

        gridLayout->addWidget(activeCountLabel, 5, 1, 1, 1);

        label_3 = new QLabel(summaryGroupBox);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 6, 0, 1, 1);

        pendingCountLabel = new QLabel(summaryGroupBox);
        pendingCountLabel->setObjectName("pendingCountLabel");

        gridLayout->addWidget(pendingCountLabel, 6, 1, 1, 1);

        label_4 = new QLabel(summaryGroupBox);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 7, 0, 1, 1);

        completedCountLabel = new QLabel(summaryGroupBox);
        completedCountLabel->setObjectName("completedCountLabel");

        gridLayout->addWidget(completedCountLabel, 7, 1, 1, 1);

        label_5 = new QLabel(summaryGroupBox);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 8, 0, 1, 1);

        expiredCountLabel = new QLabel(summaryGroupBox);
        expiredCountLabel->setObjectName("expiredCountLabel");

        gridLayout->addWidget(expiredCountLabel, 8, 1, 1, 1);


        verticalLayout->addWidget(summaryGroupBox);

        chartsWidget = new QWidget(StatisticsDialog);
        chartsWidget->setObjectName("chartsWidget");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(chartsWidget->sizePolicy().hasHeightForWidth());
        chartsWidget->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(chartsWidget);

        buttonBox = new QDialogButtonBox(StatisticsDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(StatisticsDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, StatisticsDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, StatisticsDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(StatisticsDialog);
    } // setupUi

    void retranslateUi(QDialog *StatisticsDialog)
    {
        StatisticsDialog->setWindowTitle(QCoreApplication::translate("StatisticsDialog", "Contract Statistics", nullptr));
        summaryGroupBox->setTitle(QCoreApplication::translate("StatisticsDialog", "Summary", nullptr));
        titleLabel->setText(QCoreApplication::translate("StatisticsDialog", "Contract Statistics Overview", nullptr));
        label->setText(QCoreApplication::translate("StatisticsDialog", "Total Contract Value:", nullptr));
        totalValueLabel->setText(QCoreApplication::translate("StatisticsDialog", "$0.00", nullptr));
        avgValueLabel->setText(QCoreApplication::translate("StatisticsDialog", "Average Contract Value:", nullptr));
        avgValueDisplay->setText(QCoreApplication::translate("StatisticsDialog", "$0.00", nullptr));
        avgDurationLabel->setText(QCoreApplication::translate("StatisticsDialog", "Average Contract Duration:", nullptr));
        avgDurationDisplay->setText(QCoreApplication::translate("StatisticsDialog", "0 days", nullptr));
        statusTitle->setText(QCoreApplication::translate("StatisticsDialog", "Contract Status Distribution", nullptr));
        label_2->setText(QCoreApplication::translate("StatisticsDialog", "Active Contracts:", nullptr));
        activeCountLabel->setText(QCoreApplication::translate("StatisticsDialog", "0", nullptr));
        label_3->setText(QCoreApplication::translate("StatisticsDialog", "Pending Contracts:", nullptr));
        pendingCountLabel->setText(QCoreApplication::translate("StatisticsDialog", "0", nullptr));
        label_4->setText(QCoreApplication::translate("StatisticsDialog", "Completed Contracts:", nullptr));
        completedCountLabel->setText(QCoreApplication::translate("StatisticsDialog", "0", nullptr));
        label_5->setText(QCoreApplication::translate("StatisticsDialog", "Expired Contracts:", nullptr));
        expiredCountLabel->setText(QCoreApplication::translate("StatisticsDialog", "0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StatisticsDialog: public Ui_StatisticsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STATISTICSDIALOG_H
