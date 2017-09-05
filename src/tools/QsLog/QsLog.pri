INCLUDEPATH += $$PWD
#DEFINES += QS_LOG_LINE_NUMBERS       # automatically writes the file and line for each log message
#DEFINES += QS_LOG_DISABLE            # logging code is replaced with a no-op
#DEFINES += QS_LOG_SEPARATE_THREAD    # messages are queued and written from a separate thread
#DEFINES += QS_LOG_WIN_PRINTF_CONSOLE # Use fprintf instead of OutputDebugString on Windows

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
    $$PWD/QsLogMessage.h \
    $$PWD/QsLogSharedLibrary.h

OTHER_FILES += \
    $$PWD/README.md \
    $$PWD/LICENSE.txt
