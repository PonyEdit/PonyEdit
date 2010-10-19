#-------------------------------------------------
#
# Project created by QtCreator 2010-10-11T19:25:25
#
#-------------------------------------------------

INCLUDEPATH += $$PWD/../libssh2-1.2.7/include/

LIBS     += -L$$PWD/../libssh2-1.2.7/lib/ -lgcrypt -llibgpg-error -lssh2 -lwsock32

QT       += core gui

TARGET = remoted
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    sshconnection.cpp \
    sshremotecontroller.cpp \
    serverconfigdlg.cpp \
    remotefile.cpp \
    remotecontrolmessage.cpp \
    filedialog.cpp \
    location.cpp

HEADERS  += mainwindow.h \
    sshconnection.h \
    sshremotecontroller.h \
    serverconfigdlg.h \
    remotefile.h \
    remotecontrolmessage.h \
    filedialog.h \
    location.h

OTHER_FILES += \
    slave.py

FORMS += \
    serverconfigdlg.ui \
    filedialog.ui
