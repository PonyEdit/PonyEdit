/********************************************************************************
** Form generated from reading UI file 'optionsdialog.ui'
**
** Created: Tue 14. Dec 22:05:18 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPTIONSDIALOG_H
#define UI_OPTIONSDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OptionsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QListWidget *optionList;
    QLineEdit *filterInput;
    QLabel *optionLabel;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QVBoxLayout *verticalLayout_2;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *ediLabel;
    QLineEdit *ediLocation;
    QPushButton *ediBrowseButton;
    QPushButton *ediDownloadButton;
    QTableWidget *ediLanguagesTable;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *OptionsDialog)
    {
        if (OptionsDialog->objectName().isEmpty())
            OptionsDialog->setObjectName(QString::fromUtf8("OptionsDialog"));
        OptionsDialog->resize(672, 541);
        verticalLayout = new QVBoxLayout(OptionsDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        optionList = new QListWidget(OptionsDialog);
        optionList->setObjectName(QString::fromUtf8("optionList"));
        optionList->setMinimumSize(QSize(200, 200));
        optionList->setMaximumSize(QSize(200, 16777215));
        optionList->setSelectionBehavior(QAbstractItemView::SelectRows);

        gridLayout->addWidget(optionList, 1, 0, 1, 1);

        filterInput = new QLineEdit(OptionsDialog);
        filterInput->setObjectName(QString::fromUtf8("filterInput"));
        filterInput->setMinimumSize(QSize(200, 20));
        filterInput->setMaximumSize(QSize(200, 20));

        gridLayout->addWidget(filterInput, 0, 0, 1, 1);

        optionLabel = new QLabel(OptionsDialog);
        optionLabel->setObjectName(QString::fromUtf8("optionLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(optionLabel->sizePolicy().hasHeightForWidth());
        optionLabel->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(11);
        font.setBold(true);
        font.setWeight(75);
        optionLabel->setFont(font);

        gridLayout->addWidget(optionLabel, 0, 1, 1, 1);

        stackedWidget = new QStackedWidget(OptionsDialog);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        verticalLayout_2 = new QVBoxLayout(page);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        tabWidget = new QTabWidget(page);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_3 = new QVBoxLayout(tab);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox = new QGroupBox(tab);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        ediLabel = new QLabel(groupBox);
        ediLabel->setObjectName(QString::fromUtf8("ediLabel"));

        gridLayout_2->addWidget(ediLabel, 0, 0, 1, 1);

        ediLocation = new QLineEdit(groupBox);
        ediLocation->setObjectName(QString::fromUtf8("ediLocation"));

        gridLayout_2->addWidget(ediLocation, 0, 1, 1, 1);

        ediBrowseButton = new QPushButton(groupBox);
        ediBrowseButton->setObjectName(QString::fromUtf8("ediBrowseButton"));

        gridLayout_2->addWidget(ediBrowseButton, 0, 2, 1, 1);

        ediDownloadButton = new QPushButton(groupBox);
        ediDownloadButton->setObjectName(QString::fromUtf8("ediDownloadButton"));

        gridLayout_2->addWidget(ediDownloadButton, 1, 2, 1, 1);

        ediLanguagesTable = new QTableWidget(groupBox);
        if (ediLanguagesTable->columnCount() < 3)
            ediLanguagesTable->setColumnCount(3);
        ediLanguagesTable->setObjectName(QString::fromUtf8("ediLanguagesTable"));
        ediLanguagesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ediLanguagesTable->setColumnCount(3);

        gridLayout_2->addWidget(ediLanguagesTable, 2, 0, 1, 3);


        verticalLayout_3->addWidget(groupBox);

        tabWidget->addTab(tab, QString());

        verticalLayout_2->addWidget(tabWidget);

        stackedWidget->addWidget(page);

        gridLayout->addWidget(stackedWidget, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        buttonBox = new QDialogButtonBox(OptionsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(OptionsDialog);

        stackedWidget->setCurrentIndex(0);
        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(OptionsDialog);
    } // setupUi

    void retranslateUi(QDialog *OptionsDialog)
    {
        OptionsDialog->setWindowTitle(QApplication::translate("OptionsDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        optionLabel->setText(QApplication::translate("OptionsDialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("OptionsDialog", "Definition Files", 0, QApplication::UnicodeUTF8));
        ediLabel->setText(QApplication::translate("OptionsDialog", "Location:", 0, QApplication::UnicodeUTF8));
        ediBrowseButton->setText(QApplication::translate("OptionsDialog", "Browse...", 0, QApplication::UnicodeUTF8));
        ediDownloadButton->setText(QApplication::translate("OptionsDialog", "Download Definitions", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("OptionsDialog", "Syntax Highlighter", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class OptionsDialog: public Ui_OptionsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPTIONSDIALOG_H
