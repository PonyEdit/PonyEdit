#-------------------------------------------------
#
# Project created by QtCreator 2010-10-11T19:25:25
#
#-------------------------------------------------

DEFINES += "MAJOR_VERSION=0"
DEFINES += "MINOR_VERSION=91"
DEFINES += "REVISION=3"
DEFINES += "PRETTY_VERSION=\\\"0.91-beta3\\\""


INCLUDEPATH += $$PWD/deps/libssh2/include/

win32 {
	LIBS        += -L$$PWD/deps/lib-win32/ -lwsock32 -lmpr
	INCLUDEPATH += $$PWD/deps/include-win32/
	INCLUDEPATH += $$PWD/deps/libssh2/src/

	RC_FILE = ponyedit.rc
}

macx {
	DEFINES += __DARWIN_64_BIT_INO_T

	data.files = syntaxdefs slave
	data.path = Contents/Resources

	QMAKE_BUNDLE_DATA += data

	CONFIG += x86

	ICON = icons/ponyedit.icns
	TARGET = PonyEdit
}
!macx {
	TARGET = ponyedit
}

QT      += core gui network xml script webkit
LIBS	+= -lssh2 -lcrypto -lssl
TEMPLATE = app

QMAKE_CFLAGS	+= -Werror
QMAKE_CXXFLAGS	+= -Werror

SOURCES += \
    editor/linenumberwidget.cpp \
    editor/editor.cpp \
    editor/codeeditor.cpp \
    file/unsavedchangesdialog.cpp \
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
    main/json.cpp \
	main/main.cpp \
    options/optionsdialog.cpp \
    ssh/sshconnection.cpp \
    ssh/sshhost.cpp \
    ssh/serverconfigdlg.cpp \
    syntax/syntaxdefinition.cpp \
    syntax/syntaxrule.cpp \
    syntax/syntaxhighlighter.cpp \
    syntax/syntaxdefxmlhandler.cpp \
    syntax/syntaxblockdata.cpp \
    file/localfile.cpp \
    website/sitemanager.cpp \
    syntax/syntaxdefmanager.cpp \
    file/unsavedfile.cpp \
    main/stringtrie.cpp \
    website/updatemanager.cpp \
    file/favoritelocationdialog.cpp \
    file/newfolderdialog.cpp \
    main/advancedsearchdialog.cpp \
    main/gotolinedialog.cpp \
    ssh/remoteconnection.cpp \
    ssh/connectionstatuswidget.cpp \
    ssh/passwordinput.cpp \
    ssh/connectionstatuspane.cpp \
    website/updatenotificationdialog.cpp \
    ssh/serverconfigwidget.cpp \
    options/sshserveroptionswidget.cpp \
    options/fontoptionswidget.cpp \
    options/options.cpp \
    options/optionsdialogpage.cpp \
    file/filelistdelegate.cpp \
    editor/editorwarningbar.cpp \
    main/aboutdialog.cpp \
    options/editoroptionswidget.cpp \
    tools/htmlpreview.cpp \
    licence/licence.cpp \
    ssh/remotechannel.cpp \
    ssh/slavechannel.cpp \
    ssh/remoteconnectionthread.cpp \
    file/slavefile.cpp \
    ssh/slaverequest.cpp \
    ssh/remoterequest.cpp \
    ssh/rawsshconnection.cpp \
    main/ponyedit.cpp \
    main/windowmanager.cpp \
    licence/licencecheckdialog.cpp \
    main/statuswidget.cpp \
    options/startupoptionswidget.cpp \
    main/shutdownprompt.cpp \
    file/filestatuswidget.cpp \
    ssh/requeststatuswidget.cpp \
    main/regexptester.cpp \
    ssh/ftpchannel.cpp \
    file/ftpfile.cpp \
    licence/offlineactivationdialog.cpp \
    file/tabbedfilelist.cpp \
    main/editorpanel.cpp \
    main/editorstack.cpp \
    main/searchresults.cpp \
    main/searchresultmodel.cpp \
    main/searchresultdelegate.cpp

HEADERS  += \
    editor/linenumberwidget.h \
    editor/editor.h \
    editor/codeeditor.h \
    file/unsavedchangesdialog.h \
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
    main/json.h \
    options/optionsdialog.h \
    ssh/sshhost.h \
    ssh/serverconfigdlg.h \
    syntax/syntaxrule.h \
    syntax/syntaxhighlighter.h \
    syntax/syntaxdefinition.h \
    syntax/syntaxdefxmlhandler.h \
    syntax/syntaxblockdata.h \
    file/localfile.h \
    website/sitemanager.h \
    syntax/syntaxdefmanager.h \
    file/unsavedfile.h \
    main/stringtrie.h \
    website/updatemanager.h \
    main/global.h \
    file/favoritelocationdialog.h \
    file/directorytree.h \
    file/newfolderdialog.h \
    main/advancedsearchdialog.h \
    main/gotolinedialog.h \
    ssh/remoteconnection.h \
    ssh/connectionstatuswidget.h \
    main/dialogwrapper.h \
    ssh/passwordinput.h \
    ssh/connectionstatuspane.h \
    website/updatenotificationdialog.h \
    ssh/serverconfigwidget.h \
    options/sshserveroptionswidget.h \
    options/fontoptionswidget.h \
    options/options.h \
    options/optionsdialogpage.h \
    file/filelistdelegate.h \
    editor/editorwarningbar.h \
    main/aboutdialog.h \
    options/editoroptionswidget.h \
    tools/htmlpreview.h \
    licence/licence.h \
    ssh/remotechannel.h \
    ssh/slavechannel.h \
    ssh/remoteconnectionthread.h \
    ssh/rawsshconnection.h \
    ssh/sshconnection.h \
    file/slavefile.h \
    ssh/slaverequest.h \
    ssh/remoterequest.h \
    main/ponyedit.h \
    main/windowmanager.h \
    licence/licencecheckdialog.h \
    main/statuswidget.h \
    options/startupoptionswidget.h \
    main/shutdownprompt.h \
    file/filestatuswidget.h \
    ssh/requeststatuswidget.h \
    main/regexptester.h \
    ssh/ftpchannel.h \
    ssh/ftprequest.h \
    file/ftpfile.h \
    licence/offlineactivationdialog.h \
    file/tabbedfilelist.h \
    main/editorpanel.h \
    main/editorstack.h \
    main/searchresults.h \
    main/searchresultmodel.h \
    main/searchresultdelegate.h

OTHER_FILES += \
    slave/slave.pl \
    syntaxdefs/perl.xml \
    syntax/test.pl \
    syntaxdefs/zonnon.xml \
    syntaxdefs/yaml.xml \
    syntaxdefs/yacc.xml \
    syntaxdefs/yacas.xml \
    syntaxdefs/xul.xml \
    syntaxdefs/xslt.xml \
    syntaxdefs/xorg.xml \
    syntaxdefs/xmldebug.xml \
    syntaxdefs/xml.xml \
    syntaxdefs/xharbour.xml \
    syntaxdefs/winehq.xml \
    syntaxdefs/vrml.xml \
    syntaxdefs/vhdl.xml \
    syntaxdefs/verilog.xml \
    syntaxdefs/velocity.xml \
    syntaxdefs/uscript.xml \
    syntaxdefs/txt2tags.xml \
    syntaxdefs/tibasic.xml \
    syntaxdefs/texinfo.xml \
    syntaxdefs/tcl.xml \
    syntaxdefs/systemc.xml \
    syntaxdefs/stata.xml \
    syntaxdefs/sql.xml \
    syntaxdefs/sql-postgresql.xml \
    syntaxdefs/sql-mysql.xml \
    syntaxdefs/spice.xml \
    syntaxdefs/sml.xml \
    syntaxdefs/sisu.xml \
    syntaxdefs/sieve.xml \
    syntaxdefs/sgml.xml \
    syntaxdefs/sci.xml \
    syntaxdefs/scheme.xml \
    syntaxdefs/scala.xml \
    syntaxdefs/sather.xml \
    syntaxdefs/ruby.xml \
    syntaxdefs/rsiidl.xml \
    syntaxdefs/rpmspec.xml \
    syntaxdefs/roff.xml \
    syntaxdefs/rib.xml \
    syntaxdefs/rhtml.xml \
    syntaxdefs/rexx.xml \
    syntaxdefs/rapidq.xml \
    syntaxdefs/r.xml \
    syntaxdefs/qml.xml \
    syntaxdefs/python.xml \
    syntaxdefs/purebasic.xml \
    syntaxdefs/prolog.xml \
    syntaxdefs/progress.xml \
    syntaxdefs/povray.xml \
    syntaxdefs/postscript.xml \
    syntaxdefs/pike.xml \
    syntaxdefs/picsrc.xml \
    syntaxdefs/php.xml \
    syntaxdefs/pgn.xml \
    syntaxdefs/pango.xml \
    syntaxdefs/opal.xml \
    syntaxdefs/octave.xml \
    syntaxdefs/ocaml.xml \
    syntaxdefs/objectivecpp.xml \
    syntaxdefs/objectivec.xml \
    syntaxdefs/noweb.xml \
    syntaxdefs/nemerle.xml \
    syntaxdefs/nasm.xml \
    syntaxdefs/mup.xml \
    syntaxdefs/monobasic.xml \
    syntaxdefs/modula-2.xml \
    syntaxdefs/modelica.xml \
    syntaxdefs/mips.xml \
    syntaxdefs/mergetagtext.xml \
    syntaxdefs/mediawiki.xml \
    syntaxdefs/maxima.xml \
    syntaxdefs/matlab.xml \
    syntaxdefs/mason.xml \
    syntaxdefs/mandoc.xml \
    syntaxdefs/makefile.xml \
    syntaxdefs/mab.xml \
    syntaxdefs/m3u.xml \
    syntaxdefs/lua.xml \
    syntaxdefs/lsl.xml \
    syntaxdefs/lpc.xml \
    syntaxdefs/logtalk.xml \
    syntaxdefs/literate-haskell.xml \
    syntaxdefs/lilypond.xml \
    syntaxdefs/lex.xml \
    syntaxdefs/ldif.xml \
    syntaxdefs/latex.xml \
    syntaxdefs/kbasic.xml \
    syntaxdefs/jsp.xml \
    syntaxdefs/json.xml \
    syntaxdefs/javascript.xml \
    syntaxdefs/javadoc.xml \
    syntaxdefs/java.xml \
    syntaxdefs/ini.xml \
    syntaxdefs/inform.xml \
    syntaxdefs/ilerpg.xml \
    syntaxdefs/idl.xml \
    syntaxdefs/idconsole.xml \
    syntaxdefs/html.xml \
    syntaxdefs/haxe.xml \
    syntaxdefs/haskell.xml \
    syntaxdefs/grammar.xml \
    syntaxdefs/go.xml \
    syntaxdefs/gnuassembler.xml \
    syntaxdefs/glsl.xml \
    syntaxdefs/gettext.xml \
    syntaxdefs/gdl.xml \
    syntaxdefs/gdb.xml \
    syntaxdefs/gap.xml \
    syntaxdefs/fstab.xml \
    syntaxdefs/freebasic.xml \
    syntaxdefs/fortran.xml \
    syntaxdefs/fgl-per.xml \
    syntaxdefs/fgl-4gl.xml \
    syntaxdefs/ferite.xml \
    syntaxdefs/euphoria.xml \
    syntaxdefs/erlang.xml \
    syntaxdefs/email.xml \
    syntaxdefs/eiffel.xml \
    syntaxdefs/e.xml \
    syntaxdefs/dtd.xml \
    syntaxdefs/doxygenlua.xml \
    syntaxdefs/doxygen.xml \
    syntaxdefs/djangotemplate.xml \
    syntaxdefs/diff.xml \
    syntaxdefs/desktop.xml \
    syntaxdefs/debiancontrol.xml \
    syntaxdefs/debianchangelog.xml \
    syntaxdefs/d.xml \
    syntaxdefs/cue.xml \
    syntaxdefs/css.xml \
    syntaxdefs/cs.xml \
    syntaxdefs/cpp.xml \
    syntaxdefs/component-pascal.xml \
    syntaxdefs/commonlisp.xml \
    syntaxdefs/coldfusion.xml \
    syntaxdefs/cmake.xml \
    syntaxdefs/clipper.xml \
    syntaxdefs/cisco.xml \
    syntaxdefs/changelog.xml \
    syntaxdefs/cgis.xml \
    syntaxdefs/cg.xml \
    syntaxdefs/c.xml \
    syntaxdefs/boo.xml \
    syntaxdefs/bmethod.xml \
    syntaxdefs/bibtex.xml \
    syntaxdefs/bash.xml \
    syntaxdefs/awk.xml \
    syntaxdefs/asterisk.xml \
    syntaxdefs/asp.xml \
    syntaxdefs/asn1.xml \
    syntaxdefs/asm6502.xml \
    syntaxdefs/asm-m68k.xml \
    syntaxdefs/asm-dsp56k.xml \
    syntaxdefs/asm-avr.xml \
    syntaxdefs/apache.xml \
    syntaxdefs/ansys.xml \
    syntaxdefs/ansic89.xml \
    syntaxdefs/ample.xml \
    syntaxdefs/alert.xml \
    syntaxdefs/alert_indent.xml \
    syntaxdefs/ahdl.xml \
    syntaxdefs/ada.xml \
    syntaxdefs/actionscript.xml \
    syntaxdefs/abc.xml \
    syntaxdefs/abap.xml \
    syntaxdefs/zonnon.xml \
    syntaxdefs/yaml.xml \
    syntaxdefs/yacc.xml \
    syntaxdefs/yacas.xml \
    syntaxdefs/xul.xml \
    syntaxdefs/xslt.xml \
    syntaxdefs/xorg.xml \
    syntaxdefs/xmldebug.xml \
    syntaxdefs/xml.xml \
    syntaxdefs/xharbour.xml \
    syntaxdefs/winehq.xml \
    syntaxdefs/vrml.xml \
    syntaxdefs/vhdl.xml \
    syntaxdefs/verilog.xml \
    syntaxdefs/velocity.xml \
    syntaxdefs/uscript.xml \
    syntaxdefs/txt2tags.xml \
    syntaxdefs/tibasic.xml \
    syntaxdefs/texinfo.xml \
    syntaxdefs/tcl.xml \
    syntaxdefs/systemc.xml \
    syntaxdefs/stata.xml \
    syntaxdefs/sql-postgresql.xml \
    syntaxdefs/sql-mysql.xml \
    syntaxdefs/sql.xml \
    syntaxdefs/spice.xml \
    syntaxdefs/sml.xml \
    syntaxdefs/sisu.xml \
    syntaxdefs/sieve.xml \
    syntaxdefs/sgml.xml \
    syntaxdefs/sci.xml \
    syntaxdefs/scheme.xml \
    syntaxdefs/scala.xml \
    syntaxdefs/sather.xml \
    syntaxdefs/ruby.xml \
    syntaxdefs/rsiidl.xml \
    syntaxdefs/rpmspec.xml \
    syntaxdefs/roff.xml \
    syntaxdefs/rib.xml \
    syntaxdefs/rhtml.xml \
    syntaxdefs/rexx.xml \
    syntaxdefs/rapidq.xml \
    syntaxdefs/r.xml \
    syntaxdefs/qml.xml \
    syntaxdefs/python.xml \
    syntaxdefs/purebasic.xml \
    syntaxdefs/prolog.xml \
    syntaxdefs/progress.xml \
    syntaxdefs/povray.xml \
    syntaxdefs/postscript.xml \
    syntaxdefs/pike.xml \
    syntaxdefs/picsrc.xml \
    syntaxdefs/php.xml \
    syntaxdefs/pgn.xml \
    syntaxdefs/perl.xml \
    syntaxdefs/pango.xml \
    syntaxdefs/opal.xml \
    syntaxdefs/octave.xml \
    syntaxdefs/ocaml.xml \
    syntaxdefs/objectivecpp.xml \
    syntaxdefs/objectivec.xml \
    syntaxdefs/noweb.xml \
    syntaxdefs/nemerle.xml \
    syntaxdefs/nasm.xml \
    syntaxdefs/mup.xml \
    syntaxdefs/monobasic.xml \
    syntaxdefs/modula-2.xml \
    syntaxdefs/modelica.xml \
    syntaxdefs/mips.xml \
    syntaxdefs/mergetagtext.xml \
    syntaxdefs/mediawiki.xml \
    syntaxdefs/maxima.xml \
    syntaxdefs/matlab.xml \
    syntaxdefs/mason.xml \
    syntaxdefs/mandoc.xml \
    syntaxdefs/makefile.xml \
    syntaxdefs/mab.xml \
    syntaxdefs/m3u.xml \
    syntaxdefs/lua.xml \
    syntaxdefs/lsl.xml \
    syntaxdefs/lpc.xml \
    syntaxdefs/logtalk.xml \
    syntaxdefs/literate-haskell.xml \
    syntaxdefs/lilypond.xml \
    syntaxdefs/lex.xml \
    syntaxdefs/ldif.xml \
    syntaxdefs/latex.xml \
    syntaxdefs/kbasic.xml \
    syntaxdefs/jsp.xml \
    syntaxdefs/json.xml \
    syntaxdefs/javascript-php.xml \
    syntaxdefs/javascript.xml \
    syntaxdefs/javadoc.xml \
    syntaxdefs/java.xml \
    syntaxdefs/ini.xml \
    syntaxdefs/inform.xml \
    syntaxdefs/ilerpg.xml \
    syntaxdefs/idl.xml \
    syntaxdefs/idconsole.xml \
    syntaxdefs/html-php.xml \
    syntaxdefs/html.xml \
    syntaxdefs/haxe.xml \
    syntaxdefs/haskell.xml \
    syntaxdefs/grammar.xml \
    syntaxdefs/go.xml \
    syntaxdefs/gnuassembler.xml \
    syntaxdefs/glsl.xml \
    syntaxdefs/gettext.xml \
    syntaxdefs/gdl.xml \
    syntaxdefs/gdb.xml \
    syntaxdefs/gap.xml \
    syntaxdefs/fstab.xml \
    syntaxdefs/freebasic.xml \
    syntaxdefs/fortran.xml \
    syntaxdefs/fgl-per.xml \
    syntaxdefs/fgl-4gl.xml \
    syntaxdefs/ferite.xml \
    syntaxdefs/euphoria.xml \
    syntaxdefs/erlang.xml \
    syntaxdefs/email.xml \
    syntaxdefs/eiffel.xml \
    syntaxdefs/e.xml \
    syntaxdefs/dtd.xml \
    syntaxdefs/doxygenlua.xml \
    syntaxdefs/doxygen.xml \
    syntaxdefs/djangotemplate.xml \
    syntaxdefs/diff.xml \
    syntaxdefs/desktop.xml \
    syntaxdefs/debiancontrol.xml \
    syntaxdefs/debianchangelog.xml \
    syntaxdefs/d.xml \
    syntaxdefs/cue.xml \
    syntaxdefs/css-php.xml \
    syntaxdefs/css.xml \
    syntaxdefs/cs.xml \
    syntaxdefs/cpp.xml \
    syntaxdefs/component-pascal.xml \
    syntaxdefs/commonlisp.xml \
    syntaxdefs/coldfusion.xml \
    syntaxdefs/cmake.xml \
    syntaxdefs/clipper.xml \
    syntaxdefs/cisco.xml \
    syntaxdefs/changelog.xml \
    syntaxdefs/cgis.xml \
    syntaxdefs/cg.xml \
    syntaxdefs/c.xml \
    syntaxdefs/boo.xml \
    syntaxdefs/bmethod.xml \
    syntaxdefs/bibtex.xml \
    syntaxdefs/bash.xml \
    syntaxdefs/awk.xml \
    syntaxdefs/asterisk.xml \
    syntaxdefs/asp.xml \
    syntaxdefs/asn1.xml \
    syntaxdefs/asm-m68k.xml \
    syntaxdefs/asm-dsp56k.xml \
    syntaxdefs/asm-avr.xml \
    syntaxdefs/asm6502.xml \
    syntaxdefs/apache.xml \
    syntaxdefs/ansys.xml \
    syntaxdefs/ansic89.xml \
    syntaxdefs/ample.xml \
    syntaxdefs/alert_indent.xml \
    syntaxdefs/alert.xml \
    syntaxdefs/ahdl.xml \
    syntaxdefs/ada.xml \
    syntaxdefs/actionscript.xml \
    syntaxdefs/abc.xml \
    syntaxdefs/abap.xml \
    syntaxdefs/markdown.xml \
    ponyedit.rc

FORMS += \
    file/filedialog.ui \
    main/searchbar.ui \
    options/optionsdialog.ui \
    ssh/serverconfigdlg.ui \
    file/favoritelocationdialog.ui \
    file/newfolderdialog.ui \
    main/advancedsearchdialog.ui \
    main/gotolinedialog.ui \
    ssh/passwordinput.ui \
    website/updatenotificationdialog.ui \
    ssh/serverconfigwidget.ui \
    options/sshserveroptionswidget.ui \
    options/fontoptionswidget.ui \
    main/aboutdialog.ui \
    options/editoroptionswidget.ui \
    tools/htmlpreview.ui \
    licence/licencecheckdialog.ui \
    main/statuswidget.ui \
    options/startupoptionswidget.ui \
    main/shutdownprompt.ui \
    main/regexptester.ui \
    licence/offlineactivationdialog.ui

RESOURCES += \
	resources.qrc
