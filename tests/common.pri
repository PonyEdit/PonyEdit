include( ../common.pri )

QT       += testlib
QT       -= gui

TARGET = tst_tests sshsettings

CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$SRCDIR
