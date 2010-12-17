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

QT       += core gui network xml

TARGET = remoted
TEMPLATE = app

SOURCES += \
    editor/linenumberwidget.cpp \
    editor/editor.cpp \
    editor/codeeditor.cpp \
    file/unsavedchangesdialog.cpp \
    file/sshfile.cpp \
    file/openfiletreeview.cpp \
    file/openfiletreemodel.cpp \
    file/openfilemanager.cpp \
    file/openfileitemdelegate.cpp \
    file/location.cpp \
    file/filelist.cpp \
    file/filedialog.cpp \
    file/basefile.cpp \
    main/tools.cpp \
    main/searchbar.cpp \
    main/mainwindow.cpp \
    main/main.cpp \
    options/optionsdialog.cpp \
    ssh/passworddialog.cpp \
    ssh/sshconnectingdialog.cpp \
    ssh/sshconnection.cpp \
    ssh/sshhost.cpp \
    ssh/sshremotecontroller.cpp \
    ssh/sshrequest.cpp \
    ssh/serverconfigdlg.cpp \
    syntax/syntaxdefinition.cpp \
    syntax/syntaxrule.cpp \
    syntax/syntaxhighlighter.cpp \
    syntax/syntaxdefxmlhandler.cpp

HEADERS  += \
    editor/linenumberwidget.h \
    editor/editor.h \
    editor/codeeditor.h \
    file/unsavedchangesdialog.h \
    file/sshfile.h \
    file/openfiletreeview.h \
    file/openfiletreemodel.h \
    file/openfilemanager.h \
    file/openfileitemdelegate.h \
    file/location.h \
    file/filelist.h \
    file/filedialog.h \
    file/basefile.h \
    main/tools.h \
    main/searchbar.h \
    main/mainwindow.h \
    main/globaldispatcher.h \
    options/optionsdialog.h \
    ssh/sshrequest.h \
    ssh/sshremotecontroller.h \
    ssh/sshhost.h \
    ssh/sshconnection.h \
    ssh/sshconnectingdialog.h \
    ssh/serverconfigdlg.h \
    ssh/passworddialog.h \
    syntax/syntaxrule.h \
    syntax/syntaxhighlighter.h \
    syntax/syntaxdefinition.h \
    syntax/syntaxdefxmlhandler.h

OTHER_FILES += \
    slaves/slave.py \
    slaves/slave.pl \
    syntaxdefs/perl.xml

FORMS += \
    file/filedialog.ui \
    main/searchbar.ui \
    options/optionsdialog.ui \
    ssh/sshconnectingdialog.ui \
    ssh/serverconfigdlg.ui \
    ssh/passworddialog.ui

RESOURCES += \
	resources.qrc
