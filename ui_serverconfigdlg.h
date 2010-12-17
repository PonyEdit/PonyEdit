/********************************************************************************
** Form generated from reading UI file 'serverconfigdlg.ui'
**
** Created: Tue 14. Dec 22:05:18 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERCONFIGDLG_H
#define UI_SERVERCONFIGDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ServerConfigDlg
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLineEdit *hostName;
    QLabel *label;
    QLineEdit *hostPort;
    QLabel *label_2;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QLabel *label_3;
    QLineEdit *userName;
    QLabel *label_4;
    QLineEdit *password;
    QCheckBox *savePassword;
    QLabel *label_8;
    QPushButton *keyFileBrowse;
    QLineEdit *keyFile;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_4;
    QLabel *label_7;
    QComboBox *scriptType;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_3;
    QCheckBox *saveServer;
    QLineEdit *serverName;
    QLabel *label_5;
    QLabel *label_6;
    QLineEdit *defaultDirectory;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ServerConfigDlg)
    {
        if (ServerConfigDlg->objectName().isEmpty())
            ServerConfigDlg->setObjectName(QString::fromUtf8("ServerConfigDlg"));
        ServerConfigDlg->resize(448, 400);
        ServerConfigDlg->setSizeGripEnabled(false);
        ServerConfigDlg->setModal(true);
        verticalLayout = new QVBoxLayout(ServerConfigDlg);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(ServerConfigDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        hostName = new QLineEdit(groupBox);
        hostName->setObjectName(QString::fromUtf8("hostName"));

        gridLayout->addWidget(hostName, 2, 3, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 3, 1, 1);

        hostPort = new QLineEdit(groupBox);
        hostPort->setObjectName(QString::fromUtf8("hostPort"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(hostPort->sizePolicy().hasHeightForWidth());
        hostPort->setSizePolicy(sizePolicy);
        hostPort->setMaxLength(5);

        gridLayout->addWidget(hostPort, 2, 4, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 4, 1, 1);


        verticalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(ServerConfigDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout_2->addWidget(label_3, 0, 0, 1, 1);

        userName = new QLineEdit(groupBox_2);
        userName->setObjectName(QString::fromUtf8("userName"));

        gridLayout_2->addWidget(userName, 0, 1, 1, 1);

        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_2->addWidget(label_4, 1, 0, 1, 1);

        password = new QLineEdit(groupBox_2);
        password->setObjectName(QString::fromUtf8("password"));
        password->setEchoMode(QLineEdit::Password);

        gridLayout_2->addWidget(password, 1, 1, 1, 1);

        savePassword = new QCheckBox(groupBox_2);
        savePassword->setObjectName(QString::fromUtf8("savePassword"));

        gridLayout_2->addWidget(savePassword, 1, 2, 1, 2);

        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout_2->addWidget(label_8, 2, 0, 1, 1);

        keyFileBrowse = new QPushButton(groupBox_2);
        keyFileBrowse->setObjectName(QString::fromUtf8("keyFileBrowse"));

        gridLayout_2->addWidget(keyFileBrowse, 2, 2, 1, 1);

        keyFile = new QLineEdit(groupBox_2);
        keyFile->setObjectName(QString::fromUtf8("keyFile"));

        gridLayout_2->addWidget(keyFile, 2, 1, 1, 1);


        verticalLayout->addWidget(groupBox_2);

        groupBox_4 = new QGroupBox(ServerConfigDlg);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        gridLayout_4 = new QGridLayout(groupBox_4);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        label_7 = new QLabel(groupBox_4);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy1);

        gridLayout_4->addWidget(label_7, 0, 0, 1, 1);

        scriptType = new QComboBox(groupBox_4);
        scriptType->setObjectName(QString::fromUtf8("scriptType"));

        gridLayout_4->addWidget(scriptType, 0, 1, 1, 1);


        verticalLayout->addWidget(groupBox_4);

        groupBox_3 = new QGroupBox(ServerConfigDlg);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        gridLayout_3 = new QGridLayout(groupBox_3);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        saveServer = new QCheckBox(groupBox_3);
        saveServer->setObjectName(QString::fromUtf8("saveServer"));

        gridLayout_3->addWidget(saveServer, 0, 0, 1, 3);

        serverName = new QLineEdit(groupBox_3);
        serverName->setObjectName(QString::fromUtf8("serverName"));

        gridLayout_3->addWidget(serverName, 1, 2, 1, 1);

        label_5 = new QLabel(groupBox_3);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_3->addWidget(label_5, 1, 0, 1, 1);

        label_6 = new QLabel(groupBox_3);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout_3->addWidget(label_6, 2, 0, 1, 1);

        defaultDirectory = new QLineEdit(groupBox_3);
        defaultDirectory->setObjectName(QString::fromUtf8("defaultDirectory"));

        gridLayout_3->addWidget(defaultDirectory, 2, 2, 1, 1);


        verticalLayout->addWidget(groupBox_3);

        buttonBox = new QDialogButtonBox(ServerConfigDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

        QWidget::setTabOrder(buttonBox, hostName);
        QWidget::setTabOrder(hostName, hostPort);
        QWidget::setTabOrder(hostPort, userName);
        QWidget::setTabOrder(userName, password);
        QWidget::setTabOrder(password, savePassword);
        QWidget::setTabOrder(savePassword, scriptType);
        QWidget::setTabOrder(scriptType, saveServer);
        QWidget::setTabOrder(saveServer, serverName);
        QWidget::setTabOrder(serverName, defaultDirectory);

        retranslateUi(ServerConfigDlg);

        QMetaObject::connectSlotsByName(ServerConfigDlg);
    } // setupUi

    void retranslateUi(QDialog *ServerConfigDlg)
    {
        ServerConfigDlg->setWindowTitle(QApplication::translate("ServerConfigDlg", "Dialog", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("ServerConfigDlg", "Host", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ServerConfigDlg", "Host Name / IP Address", 0, QApplication::UnicodeUTF8));
        hostPort->setInputMask(QString());
        label_2->setText(QApplication::translate("ServerConfigDlg", "Port", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("ServerConfigDlg", "Authentication", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ServerConfigDlg", "Login : ", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ServerConfigDlg", "Password : ", 0, QApplication::UnicodeUTF8));
        savePassword->setText(QApplication::translate("ServerConfigDlg", "Remember Password (insecure)", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("ServerConfigDlg", "Key File :", 0, QApplication::UnicodeUTF8));
        keyFileBrowse->setText(QApplication::translate("ServerConfigDlg", "Browse ...", 0, QApplication::UnicodeUTF8));
        groupBox_4->setTitle(QApplication::translate("ServerConfigDlg", "Advanced", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("ServerConfigDlg", "Server Script Language:", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("ServerConfigDlg", "Save Server", 0, QApplication::UnicodeUTF8));
        saveServer->setText(QApplication::translate("ServerConfigDlg", "Save Server", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ServerConfigDlg", "Server Name : ", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("ServerConfigDlg", "Default Directory : ", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ServerConfigDlg: public Ui_ServerConfigDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERCONFIGDLG_H
