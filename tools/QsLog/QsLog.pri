INCLUDEPATH += $$PWD
#DEFINES += QS_LOG_LINE_NUMBERS       # automatically writes the file and line for each log message
#DEFINES += QS_LOG_DISABLE            # logging code is replaced with a no-op
#DEFINES += QS_LOG_SEPARATE_THREAD    # messages are queued and written from a separate thread
#DEFINES += QS_LOG_WIN_PRINTF_CONSOLE # Use fprintf instead of OutputDebugString on Windows

contains(DEFINES, QS_LOG_WINDOW) {
    message("Will include log window destination")
    QT += gui
    greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
    SOURCES += $$PWD/QsLogDestModel.cpp
    HEADERS += $$PWD/QsLogDestModel.h
}

SOURCES += $$PWD/QsLogDest.cpp \
    $$PWD/QsLog.cpp \
    $$PWD/QsLogDestConsole.cpp \
    $$PWD/QsLogDestFile.cpp \
    $$PWD/QsLogDestFunctor.cpp \
    $$PWD/QsLogMessage.cpp \
    $$PWD/QsLogLevel.cpp

HEADERS += $$PWD/QsLogDest.h \
    $$PWD/QsLog.h \
    $$PWD/QsLogDestConsole.h \
    $$PWD/QsLogLevel.h \
    $$PWD/QsLogDestFile.h \
    $$PWD/QsLogDisableForThisFile.h \
    $$PWD/QsLogDestFunctor.h \
    $$PWD/QsLogMessage.h

OTHER_FILES += \
    $$PWD/QsLogChanges.txt \
    $$PWD/README.md \
    $$PWD/LICENSE.txt
