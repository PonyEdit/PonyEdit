#-------------------------------------------------
#
# Project created by QtCreator 2010-10-11T19:25:25
#
#-------------------------------------------------

win32 {
	INCLUDEPATH += $$PWD/../libssh2-1.2.7/include/

	LIBS     += -L$$PWD/../libssh2-1.2.7/lib/ -llibgpg-error -lwsock32
}

!win32 {
	LIBS	+= -lgpg-error
}

macx {
	DEFINES += __DARWIN_64_BIT_INO_T
}

LIBS	+= -lssh2

QT       += core gui network

TARGET = remoted
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    sshconnection.cpp \
    sshremotecontroller.cpp \
    serverconfigdlg.cpp \
    filedialog.cpp \
    location.cpp \
    sshhost.cpp \
    sshrequest.cpp \
    tools.cpp \
    passworddialog.cpp \
    sshconnectingdialog.cpp \
    editor.cpp \
    sshfile.cpp \
    filelist.cpp \
    basefile.cpp \
    codeeditor.cpp \
    linenumberwidget.cpp \
    optionsdialog.cpp \
    syntaxhighlighter.cpp \
    searchbar.cpp \
    filelistitemdelegate.cpp \
    unsavedchangesdialog.cpp \
    openfilemanager.cpp \
    openfiletreemodel.cpp

HEADERS  += mainwindow.h \
    sshconnection.h \
    sshremotecontroller.h \
    serverconfigdlg.h \
    filedialog.h \
    location.h \
    sshhost.h \
    sshrequest.h \
    tools.h \
    globaldispatcher.h \
    passworddialog.h \
    sshconnectingdialog.h \
    editor.h \
    sshfile.h \
    filelist.h \
    basefile.h \
	codeeditor.h \
	linenumberwidget.h \
    optionsdialog.h \
    syntaxhighlighter.h \
    searchbar.h \
    filelistitemdelegate.h \
    unsavedchangesdialog.h \
    openfilemanager.h \
    openfiletreemodel.h

OTHER_FILES += \
    slave.py \
    slave.pl

FORMS += \
    serverconfigdlg.ui \
    filedialog.ui \
    passworddialog.ui \
    sshconnectingdialog.ui \
    optionsdialog.ui \
    searchbar.ui \
    unsavedchangesdialog.ui

RESOURCES += \
    resources.qrc
