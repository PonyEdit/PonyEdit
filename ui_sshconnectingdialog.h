/********************************************************************************
** Form generated from reading UI file 'sshconnectingdialog.ui'
**
** Created: Tue 14. Dec 22:05:18 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SSHCONNECTINGDIALOG_H
#define UI_SSHCONNECTINGDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_SshConnectingDialog
{
public:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QLabel *status;
    QLabel *icon;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *SshConnectingDialog)
    {
        if (SshConnectingDialog->objectName().isEmpty())
            SshConnectingDialog->setObjectName(QString::fromUtf8("SshConnectingDialog"));
        SshConnectingDialog->resize(237, 78);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SshConnectingDialog->sizePolicy().hasHeightForWidth());
        SshConnectingDialog->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(SshConnectingDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetFixedSize);
        buttonBox = new QDialogButtonBox(SshConnectingDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
        buttonBox->setCenterButtons(true);

        gridLayout->addWidget(buttonBox, 1, 0, 1, 4);

        status = new QLabel(SshConnectingDialog);
        status->setObjectName(QString::fromUtf8("status"));

        gridLayout->addWidget(status, 0, 2, 1, 1);

        icon = new QLabel(SshConnectingDialog);
        icon->setObjectName(QString::fromUtf8("icon"));
        icon->setPixmap(QPixmap(QString::fromUtf8(":/icons/loading.png")));

        gridLayout->addWidget(icon, 0, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 0, 3, 1, 1);


        retranslateUi(SshConnectingDialog);

        QMetaObject::connectSlotsByName(SshConnectingDialog);
    } // setupUi

    void retranslateUi(QDialog *SshConnectingDialog)
    {
        SshConnectingDialog->setWindowTitle(QApplication::translate("SshConnectingDialog", "Connecting ...", 0, QApplication::UnicodeUTF8));
        status->setText(QApplication::translate("SshConnectingDialog", "Please Wait ...", 0, QApplication::UnicodeUTF8));
        icon->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class SshConnectingDialog: public Ui_SshConnectingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SSHCONNECTINGDIALOG_H
