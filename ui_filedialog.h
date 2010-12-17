/********************************************************************************
** Form generated from reading UI file 'filedialog.ui'
**
** Created: Tue 14. Dec 22:05:18 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILEDIALOG_H
#define UI_FILEDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>
#include <QtGui/QTableView>
#include <QtGui/QToolButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FileDialog
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLineEdit *currentPath;
    QToolButton *upLevelButton;
    QToolButton *newFolderButton;
    QToolButton *addFavoriteButton;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QLineEdit *fileName;
    QComboBox *comboBox;
    QDialogButtonBox *buttonBox;
    QSplitter *splitter;
    QTreeWidget *directoryTree;
    QStackedWidget *fileListStack;
    QWidget *fileListLayer;
    QVBoxLayout *verticalLayout_2;
    QTableView *fileList;
    QWidget *loaderLayer;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QLabel *loaderIcon;
    QLabel *loaderLabel;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *FileDialog)
    {
        if (FileDialog->objectName().isEmpty())
            FileDialog->setObjectName(QString::fromUtf8("FileDialog"));
        FileDialog->resize(733, 378);
        FileDialog->setSizeGripEnabled(true);
        FileDialog->setModal(true);
        gridLayout = new QGridLayout(FileDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        currentPath = new QLineEdit(FileDialog);
        currentPath->setObjectName(QString::fromUtf8("currentPath"));

        horizontalLayout->addWidget(currentPath);

        upLevelButton = new QToolButton(FileDialog);
        upLevelButton->setObjectName(QString::fromUtf8("upLevelButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("icons/up.png"), QSize(), QIcon::Normal, QIcon::Off);
        upLevelButton->setIcon(icon);

        horizontalLayout->addWidget(upLevelButton);

        newFolderButton = new QToolButton(FileDialog);
        newFolderButton->setObjectName(QString::fromUtf8("newFolderButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("icons/newfolder.png"), QSize(), QIcon::Normal, QIcon::Off);
        newFolderButton->setIcon(icon1);

        horizontalLayout->addWidget(newFolderButton);

        addFavoriteButton = new QToolButton(FileDialog);
        addFavoriteButton->setObjectName(QString::fromUtf8("addFavoriteButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("icons/favorite.png"), QSize(), QIcon::Normal, QIcon::Off);
        addFavoriteButton->setIcon(icon2);

        horizontalLayout->addWidget(addFavoriteButton);


        verticalLayout->addLayout(horizontalLayout);


        gridLayout->addLayout(verticalLayout, 1, 0, 1, 1);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label = new QLabel(FileDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        fileName = new QLineEdit(FileDialog);
        fileName->setObjectName(QString::fromUtf8("fileName"));

        gridLayout_2->addWidget(fileName, 0, 1, 1, 1);

        comboBox = new QComboBox(FileDialog);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        gridLayout_2->addWidget(comboBox, 0, 2, 1, 2);

        buttonBox = new QDialogButtonBox(FileDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(buttonBox->sizePolicy().hasHeightForWidth());
        buttonBox->setSizePolicy(sizePolicy);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(buttonBox, 1, 2, 1, 1);


        gridLayout->addLayout(gridLayout_2, 3, 0, 1, 1);

        splitter = new QSplitter(FileDialog);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy1);
        splitter->setOrientation(Qt::Horizontal);
        directoryTree = new QTreeWidget(splitter);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        directoryTree->setHeaderItem(__qtreewidgetitem);
        directoryTree->setObjectName(QString::fromUtf8("directoryTree"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(directoryTree->sizePolicy().hasHeightForWidth());
        directoryTree->setSizePolicy(sizePolicy2);
        directoryTree->setMinimumSize(QSize(200, 0));
        directoryTree->setBaseSize(QSize(1, 0));
        splitter->addWidget(directoryTree);
        directoryTree->header()->setVisible(false);
        fileListStack = new QStackedWidget(splitter);
        fileListStack->setObjectName(QString::fromUtf8("fileListStack"));
        fileListStack->setStyleSheet(QString::fromUtf8(""));
        fileListStack->setFrameShape(QFrame::NoFrame);
        fileListStack->setFrameShadow(QFrame::Plain);
        fileListLayer = new QWidget();
        fileListLayer->setObjectName(QString::fromUtf8("fileListLayer"));
        fileListLayer->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_2 = new QVBoxLayout(fileListLayer);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        fileList = new QTableView(fileListLayer);
        fileList->setObjectName(QString::fromUtf8("fileList"));
        fileList->setMinimumSize(QSize(500, 0));
        fileList->setBaseSize(QSize(2, 0));

        verticalLayout_2->addWidget(fileList);

        fileListStack->addWidget(fileListLayer);
        loaderLayer = new QWidget();
        loaderLayer->setObjectName(QString::fromUtf8("loaderLayer"));
        loaderLayer->setStyleSheet(QString::fromUtf8("#loaderLayer {\n"
"background-color: rgb(195, 195, 195);\n"
"border: 1px solid grey;\n"
"}"));
        horizontalLayout_2 = new QHBoxLayout(loaderLayer);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        loaderIcon = new QLabel(loaderLayer);
        loaderIcon->setObjectName(QString::fromUtf8("loaderIcon"));
        sizePolicy.setHeightForWidth(loaderIcon->sizePolicy().hasHeightForWidth());
        loaderIcon->setSizePolicy(sizePolicy);
        loaderIcon->setBaseSize(QSize(16, 16));

        horizontalLayout_2->addWidget(loaderIcon);

        loaderLabel = new QLabel(loaderLayer);
        loaderLabel->setObjectName(QString::fromUtf8("loaderLabel"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(loaderLabel->sizePolicy().hasHeightForWidth());
        loaderLabel->setSizePolicy(sizePolicy3);

        horizontalLayout_2->addWidget(loaderLabel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        fileListStack->addWidget(loaderLayer);
        splitter->addWidget(fileListStack);

        gridLayout->addWidget(splitter, 2, 0, 1, 1);


        retranslateUi(FileDialog);

        fileListStack->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(FileDialog);
    } // setupUi

    void retranslateUi(QDialog *FileDialog)
    {
        FileDialog->setWindowTitle(QApplication::translate("FileDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        upLevelButton->setText(QApplication::translate("FileDialog", "...", 0, QApplication::UnicodeUTF8));
        newFolderButton->setText(QApplication::translate("FileDialog", "...", 0, QApplication::UnicodeUTF8));
        addFavoriteButton->setText(QApplication::translate("FileDialog", "...", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("FileDialog", "File name : ", 0, QApplication::UnicodeUTF8));
        loaderIcon->setText(QApplication::translate("FileDialog", "[icon]", 0, QApplication::UnicodeUTF8));
        loaderLabel->setText(QApplication::translate("FileDialog", "Loading...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FileDialog: public Ui_FileDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEDIALOG_H
