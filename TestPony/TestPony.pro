#-------------------------------------------------
#
# Project created by QtCreator 2011-02-26T20:29:25
#
#-------------------------------------------------

DEPENDPATH += $$PWD/../
DEFINES += TESTPONY
PONYTEST = 1

include(../PonyEdit.pro)

QT       += xml testlib network gui script webkit

TARGET = TestPony
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH += $$PWD/../deps/libssh2/include/
INCLUDEPATH += $$PWD/../
win32 {
	LIBS        += -L$$PWD/../deps/lib-win32/ -lwsock32 -lmpr
	INCLUDEPATH += $$PWD/../deps/include-win32/
	INCLUDEPATH += $$PWD/../deps/libssh2/src/
	RC_FILE = ../ponyedit.rc
}

SOURCES += \
    itest_localfiles.cpp \
    testmain.cpp

HEADERS += \
    itest_localfiles.h \
    globals.h
