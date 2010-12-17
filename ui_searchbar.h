/********************************************************************************
** Form generated from reading UI file 'searchbar.ui'
**
** Created: Tue 14. Dec 22:05:18 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEARCHBAR_H
#define UI_SEARCHBAR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SearchBar
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *find;
    QToolButton *prevButton;
    QToolButton *nextButton;
    QToolButton *closeButton;

    void setupUi(QWidget *SearchBar)
    {
        if (SearchBar->objectName().isEmpty())
            SearchBar->setObjectName(QString::fromUtf8("SearchBar"));
        SearchBar->resize(822, 23);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SearchBar->sizePolicy().hasHeightForWidth());
        SearchBar->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(SearchBar);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(-1, 0, -1, 0);
        label = new QLabel(SearchBar);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        find = new QLineEdit(SearchBar);
        find->setObjectName(QString::fromUtf8("find"));

        gridLayout->addWidget(find, 0, 1, 1, 1);

        prevButton = new QToolButton(SearchBar);
        prevButton->setObjectName(QString::fromUtf8("prevButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/previous.png"), QSize(), QIcon::Normal, QIcon::Off);
        prevButton->setIcon(icon);

        gridLayout->addWidget(prevButton, 0, 2, 1, 1);

        nextButton = new QToolButton(SearchBar);
        nextButton->setObjectName(QString::fromUtf8("nextButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        nextButton->setIcon(icon1);

        gridLayout->addWidget(nextButton, 0, 3, 1, 1);

        closeButton = new QToolButton(SearchBar);
        closeButton->setObjectName(QString::fromUtf8("closeButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/cross.png"), QSize(), QIcon::Normal, QIcon::Off);
        closeButton->setIcon(icon2);

        gridLayout->addWidget(closeButton, 0, 4, 1, 1);


        retranslateUi(SearchBar);

        QMetaObject::connectSlotsByName(SearchBar);
    } // setupUi

    void retranslateUi(QWidget *SearchBar)
    {
        SearchBar->setWindowTitle(QApplication::translate("SearchBar", "Search", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("SearchBar", "Find :", 0, QApplication::UnicodeUTF8));
        prevButton->setText(QString());
        nextButton->setText(QString());
        closeButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class SearchBar: public Ui_SearchBar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEARCHBAR_H
