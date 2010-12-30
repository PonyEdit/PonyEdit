/********************************************************************************
** Form generated from reading UI file 'favoritelocationdialog.ui'
**
** Created: Thu 30. Dec 17:02:35 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FAVORITELOCATIONDIALOG_H
#define UI_FAVORITELOCATIONDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_FavoriteLocationDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *name;
    QLabel *label_2;
    QLineEdit *path;
    QSpacerItem *horizontalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *FavoriteLocationDialog)
    {
        if (FavoriteLocationDialog->objectName().isEmpty())
            FavoriteLocationDialog->setObjectName(QString::fromUtf8("FavoriteLocationDialog"));
        FavoriteLocationDialog->resize(344, 93);
        gridLayout = new QGridLayout(FavoriteLocationDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(FavoriteLocationDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        name = new QLineEdit(FavoriteLocationDialog);
        name->setObjectName(QString::fromUtf8("name"));

        gridLayout->addWidget(name, 0, 1, 1, 1);

        label_2 = new QLabel(FavoriteLocationDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        path = new QLineEdit(FavoriteLocationDialog);
        path->setObjectName(QString::fromUtf8("path"));
        path->setEnabled(false);

        gridLayout->addWidget(path, 1, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 2, 0, 1, 1);

        buttonBox = new QDialogButtonBox(FavoriteLocationDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 2, 1, 1, 1);


        retranslateUi(FavoriteLocationDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), FavoriteLocationDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), FavoriteLocationDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(FavoriteLocationDialog);
    } // setupUi

    void retranslateUi(QDialog *FavoriteLocationDialog)
    {
        FavoriteLocationDialog->setWindowTitle(QApplication::translate("FavoriteLocationDialog", "Favorite Location", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("FavoriteLocationDialog", "Name : ", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("FavoriteLocationDialog", "Path : ", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FavoriteLocationDialog: public Ui_FavoriteLocationDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FAVORITELOCATIONDIALOG_H
