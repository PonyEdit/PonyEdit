QT       += testlib

QT       -= gui

TARGET = tst_testssshsettings
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    tst_testssshsettings.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
