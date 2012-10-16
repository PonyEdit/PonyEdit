#-------------------------------------------------
#
# Project created by QtCreator 2011-07-17T20:54:42
#
#-------------------------------------------------

QT       += core

QT       -= gui

win32 {
	LIBS        += -L$$PWD/deps/lib-win32/ -lwsock32
	LIBS        += c:/mingw/lib/libws2_32.a
	INCLUDEPATH += $$PWD/deps/include-win32/
	INCLUDEPATH += $$PWD/deps/libssh2/include/

	RC_FILE = ponyedit.rc
}

LIBS	+= -lssh2 -lcrypto -lssl

TARGET = ssh_experimental
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ssh2/sshhost.cpp \
    ssh2/sshsession.cpp \
    ssh2/sshchannel.cpp \
    ssh2/shellchannel.cpp \
    testclient.cpp \
    ssh2/slaverequest.cpp \
    ssh2/slavechannel.cpp \
    tools/json.cpp \
    ssh2/xferchannel.cpp \
    ssh2/xferrequest.cpp

HEADERS += \
    ssh2/sshhost.h \
    ssh2/sshsession.h \
    ssh2/sshchannel.h \
    ssh2/shellchannel.h \
    testclient.h \
    ssh2/slaverequest.h \
    ssh2/slavechannel.h \
    tools/json.h \
    ssh2/xferchannel.h \
    ssh2/xferrequest.h

OTHER_FILES += \
    slave/slave.pl
