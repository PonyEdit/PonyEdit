#-------------------------------------------------
#
# Project created by QtCreator 2010-10-11T19:25:25
# 
#-------------------------------------------------

cache()

DEFINES += "MAJOR_VERSION=0"
DEFINES += "MINOR_VERSION=91"
DEFINES += "REVISION=11"
DEFINES += "PRETTY_VERSION=\\\"0.91-beta11\\\""

DEFINES += QT_DEPRECATED_WARNINGS

win32 {
	DEFINES += NOMINMAX

	INCLUDEPATH += $$PWD/deps/include/ $$PWD/deps/include/libssh2/

	LIBS        += -L$$PWD/deps/lib-win32/ -lwsock32 -lmpr
	INCLUDEPATH += $$PWD/deps/libssh2/src/

	# Just how dumb is MSVC anyway?
	INCLUDEPATH += $$PWD/

	RC_FILE     = ponyedit.rc
	LIBS	+= -llibssh2 -lssleay32 -llibeay32
}

linux {
	TARGET = PonyEdit
	LIBS += -lz
	LIBS += -lssh2 -lcrypto -lssl

	QMAKE_CFLAGS += -Werror -Wunused-parameter -Wno-terminate
	QMAKE_CXXFLAGS += -Werror -Wunused-parameter -Wno-terminate
}

macx {
	DEFINES += __DARWIN_64_BIT_INO_T
	CONFIG  += x86_64
	CONFIG  -= i386

	data.files = syntaxdefs server
	data.path = Contents/Resources
	QMAKE_BUNDLE_DATA += data
	ICON = icons/ponyedit.icns
	TARGET = PonyEdit
	LIBS += -lz

	INCLUDEPATH += /usr/local/include /usr/local/opt/openssl@1.1/include

	# Bundle dynamic libs in .app
	dylibs.path = Contents/MacOS
	dylibs.files = /usr/local/opt/libssh2/lib/libssh2.1.dylib
	dylibs.files += /usr/local/opt/openssl@1.1/lib/libssl.1.1.dylib
	dylibs.files += /usr/local/opt/openssl@1.1/lib/libcrypto.1.1.dylib
	QMAKE_BUNDLE_DATA += dylibs

	LIBS += /usr/local/opt/libssh2/lib/libssh2.1.dylib
	LIBS += /usr/local/opt/openssl@1.1/lib/libssl.1.1.dylib
	LIBS += /usr/local/opt/openssl@1.1/lib/libcrypto.1.1.dylib

	# Post-build steps; configure executable to look in .app for dylibs.
	QMAKE_POST_LINK += /usr/bin/install_name_tool -change /usr/local/opt/libssh2/lib/libssh2.1.dylib @executable_path/libssh2.1.dylib $$OUT_PWD/PonyEdit.app/Contents/MacOS/PonyEdit;
	QMAKE_POST_LINK += /usr/bin/install_name_tool -change /usr/local/opt/openssl@1.1/lib/libssl.1.1.dylib @executable_path/libssl.1.1.dylib $$OUT_PWD/PonyEdit.app/Contents/MacOS/PonyEdit;
	QMAKE_POST_LINK += /usr/bin/install_name_tool -change /usr/local/opt/openssl@1.1/lib/libcrypto.1.1.dylib @executable_path/libcrypto.1.1.dylib $$OUT_PWD/PonyEdit.app/Contents/MacOS/PonyEdit;

	QMAKE_CFLAGS += -Werror -Wunused-parameter
	QMAKE_CXXFLAGS += -Werror -Wunused-parameter
}

QT		+= core widgets gui network xml webengine webenginewidgets printsupport
TEMPLATE = app

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
	main/main.cpp \
	options/optionsdialog.cpp \
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
	website/updatenotificationdialog.cpp \
	options/sshserveroptionswidget.cpp \
	options/fontoptionswidget.cpp \
	options/options.cpp \
	options/optionsdialogpage.cpp \
	file/filelistdelegate.cpp \
	editor/editorwarningbar.cpp \
	main/aboutdialog.cpp \
	options/editoroptionswidget.cpp \
	tools/htmlpreview.cpp \
	main/ponyedit.cpp \
	main/windowmanager.cpp \
	main/statuswidget.cpp \
	options/startupoptionswidget.cpp \
	main/shutdownprompt.cpp \
	file/filestatuswidget.cpp \
	main/regexptester.cpp \
	file/ftpfile.cpp \
	file/tabbedfilelist.cpp \
	main/editorpanel.cpp \
	main/editorstack.cpp \
	main/searchresults.cpp \
	main/searchresultmodel.cpp \
	main/searchresultdelegate.cpp \
	ssh2/xferrequest.cpp \
	ssh2/xferchannel.cpp \
	ssh2/sshsession.cpp \
	ssh2/sshhost.cpp \
	ssh2/sshchannel.cpp \
	ssh2/shellchannel.cpp \
	ssh2/serverconfigwidget.cpp \
	ssh2/serverconfigdlg.cpp \
	ssh2/dialogrethreader.cpp \
	ssh2/passworddlg.cpp \
	ssh2/threadcrossingdialog.cpp \
	main/customtreewidget.cpp \
	main/customtreeentry.cpp \
	main/customtreemodel.cpp \
	main/customtreedelegate.cpp \
	ssh2/sshhosttreeentry.cpp \
	ssh2/hostkeydlg.cpp \
	tools/callback.cpp \
	ssh2/sftprequest.cpp \
	ssh2/sftpchannel.cpp \
	options/advancedoptionswidget.cpp \
	ssh2/hostlog.cpp \
	file/serverfile.cpp \
	ssh2/serverchannel.cpp \
	ssh2/serverrequest.cpp \
	ssh2/sshsettings.cpp

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
	options/optionsdialog.h \
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
	main/dialogwrapper.h \
	website/updatenotificationdialog.h \
	options/sshserveroptionswidget.h \
	options/fontoptionswidget.h \
	options/options.h \
	options/optionsdialogpage.h \
	file/filelistdelegate.h \
	editor/editorwarningbar.h \
	main/aboutdialog.h \
	options/editoroptionswidget.h \
	tools/htmlpreview.h \
	main/ponyedit.h \
	main/windowmanager.h \
	main/statuswidget.h \
	options/startupoptionswidget.h \
	main/shutdownprompt.h \
	file/filestatuswidget.h \
	main/regexptester.h \
	file/ftpfile.h \
	file/tabbedfilelist.h \
	main/editorpanel.h \
	main/editorstack.h \
	main/searchresults.h \
	main/searchresultmodel.h \
	main/searchresultdelegate.h \
	ssh2/xferrequest.h \
	ssh2/xferchannel.h \
	ssh2/sshsession.h \
	ssh2/sshhost.h \
	ssh2/sshchannel.h \
	ssh2/shellchannel.h \
	ssh2/serverconfigwidget.h \
	ssh2/serverconfigdlg.h \
	ssh2/dialogrethreader.h \
	ssh2/passworddlg.h \
	ssh2/threadcrossingdialog.h \
	main/customtreewidget.h \
	main/customtreeentry.h \
	main/customtreemodel.h \
	main/customtreedelegate.h \
	ssh2/sshhosttreeentry.h \
	ssh2/hostkeydlg.h \
	tools/callback.h \
	ssh2/sftprequest.h \
	ssh2/sftpchannel.h \
	options/advancedoptionswidget.h \
	ssh2/hostlog.h \
	file/serverfile.h \
	ssh2/serverchannel.h \
	ssh2/serverrequest.h \
	ssh2/sshsettings.h

FORMS += \
	file/filedialog.ui \
	main/searchbar.ui \
	options/optionsdialog.ui \
	file/favoritelocationdialog.ui \
	file/newfolderdialog.ui \
	main/advancedsearchdialog.ui \
	main/gotolinedialog.ui \
	website/updatenotificationdialog.ui \
	options/sshserveroptionswidget.ui \
	options/fontoptionswidget.ui \
	main/aboutdialog.ui \
	options/editoroptionswidget.ui \
	tools/htmlpreview.ui \
	main/statuswidget.ui \
	options/startupoptionswidget.ui \
	main/shutdownprompt.ui \
	main/regexptester.ui \
	ssh2/serverconfigwidget.ui \
	ssh2/serverconfigdlg.ui \
	ssh2/passworddlg.ui \
	ssh2/hostkeydlg.ui \
	options/advancedoptionswidget.ui \
	ssh2/hostlog.ui

OTHER_FILES += \
	syntaxdefs/4dos.xml \
	syntaxdefs/abap.xml \
	syntaxdefs/abc.xml \
	syntaxdefs/actionscript.xml \
	syntaxdefs/ada.xml \
	syntaxdefs/agda.xml \
	syntaxdefs/ahdl.xml \
	syntaxdefs/ahk.xml \
	syntaxdefs/alert.xml \
	syntaxdefs/alert_indent.xml \
	syntaxdefs/ample.xml \
	syntaxdefs/ansforth94.xml \
	syntaxdefs/ansic89.xml \
	syntaxdefs/ansys.xml \
	syntaxdefs/apache.xml \
	syntaxdefs/asm-avr.xml \
	syntaxdefs/asm-dsp56k.xml \
	syntaxdefs/asm-m68k.xml \
	syntaxdefs/asm6502.xml \
	syntaxdefs/asn1.xml \
	syntaxdefs/asp.xml \
	syntaxdefs/asterisk.xml \
	syntaxdefs/awk.xml \
	syntaxdefs/bash.xml \
	syntaxdefs/bibtex.xml \
	syntaxdefs/bitbake.xml \
	syntaxdefs/bmethod.xml \
	syntaxdefs/boo.xml \
	syntaxdefs/c.xml \
	syntaxdefs/carto-css.xml \
	syntaxdefs/ccss.xml \
	syntaxdefs/cg.xml \
	syntaxdefs/cgis.xml \
	syntaxdefs/changelog.xml \
	syntaxdefs/chicken.xml \
	syntaxdefs/cisco.xml \
	syntaxdefs/clipper.xml \
	syntaxdefs/clojure.xml \
	syntaxdefs/cmake.xml \
	syntaxdefs/coffee.xml \
	syntaxdefs/coldfusion.xml \
	syntaxdefs/commonlisp.xml \
	syntaxdefs/component-pascal.xml \
	syntaxdefs/context.xml \
	syntaxdefs/cpp.xml \
	syntaxdefs/crk.xml \
	syntaxdefs/cs.xml \
	syntaxdefs/css.xml \
	syntaxdefs/cubescript.xml \
	syntaxdefs/cue.xml \
	syntaxdefs/curry.xml \
	syntaxdefs/d.xml \
	syntaxdefs/debianchangelog.xml \
	syntaxdefs/debiancontrol.xml \
	syntaxdefs/desktop.xml \
	syntaxdefs/diff.xml \
	syntaxdefs/djangotemplate.xml \
	syntaxdefs/dockerfile.xml \
	syntaxdefs/dosbat.xml \
	syntaxdefs/dot.xml \
	syntaxdefs/doxyfile.xml \
	syntaxdefs/doxygen.xml \
	syntaxdefs/doxygenlua.xml \
	syntaxdefs/dtd.xml \
	syntaxdefs/e.xml \
	syntaxdefs/eiffel.xml \
	syntaxdefs/elixir.xml \
	syntaxdefs/email.xml \
	syntaxdefs/erlang.xml \
	syntaxdefs/euphoria.xml \
	syntaxdefs/fasm.xml \
	syntaxdefs/fastq.xml \
	syntaxdefs/ferite.xml \
	syntaxdefs/fgl-4gl.xml \
	syntaxdefs/fgl-per.xml \
	syntaxdefs/fortran.xml \
	syntaxdefs/freebasic.xml \
	syntaxdefs/fsharp.xml \
	syntaxdefs/fstab.xml \
	syntaxdefs/ftl.xml \
	syntaxdefs/gap.xml \
	syntaxdefs/gcc.xml \
	syntaxdefs/gcode.xml \
	syntaxdefs/gdb.xml \
	syntaxdefs/gdl.xml \
	syntaxdefs/gettext.xml \
	syntaxdefs/git-ignore.xml \
	syntaxdefs/git-rebase.xml \
	syntaxdefs/gitolite.xml \
	syntaxdefs/glosstex.xml \
	syntaxdefs/glsl.xml \
	syntaxdefs/gnuassembler.xml \
	syntaxdefs/gnuplot.xml \
	syntaxdefs/go.xml \
	syntaxdefs/grammar.xml \
	syntaxdefs/groovy.xml \
	syntaxdefs/haml.xml \
	syntaxdefs/hamlet.xml \
	syntaxdefs/haskell.xml \
	syntaxdefs/haxe.xml \
	syntaxdefs/html.xml \
	syntaxdefs/hunspell-aff.xml \
	syntaxdefs/hunspell-dat.xml \
	syntaxdefs/hunspell-dic.xml \
	syntaxdefs/hunspell-idx.xml \
	syntaxdefs/idconsole.xml \
	syntaxdefs/idl.xml \
	syntaxdefs/ilerpg.xml \
	syntaxdefs/inform.xml \
	syntaxdefs/ini.xml \
	syntaxdefs/isocpp.xml \
	syntaxdefs/j.xml \
	syntaxdefs/jam.xml \
	syntaxdefs/java.xml \
	syntaxdefs/javadoc.xml \
	syntaxdefs/javascript.xml \
	syntaxdefs/jira.xml \
	syntaxdefs/json.xml \
	syntaxdefs/jsp.xml \
	syntaxdefs/julia.xml \
	syntaxdefs/k.xml \
	syntaxdefs/kbasic.xml \
	syntaxdefs/kconfig.xml \
	syntaxdefs/kdesrc-buildrc.xml \
	syntaxdefs/kotlin.xml \
	syntaxdefs/latex.xml \
	syntaxdefs/ld.xml \
	syntaxdefs/ldif.xml \
	syntaxdefs/less.xml \
	syntaxdefs/lex.xml \
	syntaxdefs/lilypond.xml \
	syntaxdefs/literate-curry.xml \
	syntaxdefs/literate-haskell.xml \
	syntaxdefs/logtalk.xml \
	syntaxdefs/lpc.xml \
	syntaxdefs/lsl.xml \
	syntaxdefs/lua.xml \
	syntaxdefs/m3u.xml \
	syntaxdefs/m4.xml \
	syntaxdefs/mab.xml \
	syntaxdefs/magma.xml \
	syntaxdefs/mako.xml \
	syntaxdefs/mandoc.xml \
	syntaxdefs/markdown.xml \
	syntaxdefs/mason.xml \
	syntaxdefs/mathematica.xml \
	syntaxdefs/matlab.xml \
	syntaxdefs/maxima.xml \
	syntaxdefs/mediawiki.xml \
	syntaxdefs/mel.xml \
	syntaxdefs/mergetagtext.xml \
	syntaxdefs/meson.xml \
	syntaxdefs/metafont.xml \
	syntaxdefs/mips.xml \
	syntaxdefs/modelica.xml \
	syntaxdefs/modelines.xml \
	syntaxdefs/modula-2.xml \
	syntaxdefs/monobasic.xml \
	syntaxdefs/mup.xml \
	syntaxdefs/nagios.xml \
	syntaxdefs/nasm.xml \
	syntaxdefs/nemerle.xml \
	syntaxdefs/nesc.xml \
	syntaxdefs/noweb.xml \
	syntaxdefs/nsis.xml \
	syntaxdefs/objectivec.xml \
	syntaxdefs/objectivecpp.xml \
	syntaxdefs/ocaml.xml \
	syntaxdefs/ocamllex.xml \
	syntaxdefs/ocamlyacc.xml \
	syntaxdefs/octave.xml \
	syntaxdefs/oors.xml \
	syntaxdefs/opal.xml \
	syntaxdefs/opencl.xml \
	syntaxdefs/pango.xml \
	syntaxdefs/pascal.xml \
	syntaxdefs/perl.xml \
	syntaxdefs/pgn.xml \
	syntaxdefs/php.xml \
	syntaxdefs/picsrc.xml \
	syntaxdefs/pig.xml \
	syntaxdefs/pike.xml \
	syntaxdefs/pli.xml \
	syntaxdefs/ply.xml \
	syntaxdefs/postscript.xml \
	syntaxdefs/povray.xml \
	syntaxdefs/powershell.xml \
	syntaxdefs/ppd.xml \
	syntaxdefs/praat.xml \
	syntaxdefs/progress.xml \
	syntaxdefs/prolog.xml \
	syntaxdefs/protobuf.xml \
	syntaxdefs/pug.xml \
	syntaxdefs/puppet.xml \
	syntaxdefs/purebasic.xml \
	syntaxdefs/python.xml \
	syntaxdefs/q.xml \
	syntaxdefs/qmake.xml \
	syntaxdefs/qml.xml \
	syntaxdefs/r.xml \
	syntaxdefs/rapidq.xml \
	syntaxdefs/relaxng.xml \
	syntaxdefs/relaxngcompact.xml \
	syntaxdefs/replicode.xml \
	syntaxdefs/rest.xml \
	syntaxdefs/rexx.xml \
	syntaxdefs/rhtml.xml \
	syntaxdefs/rib.xml \
	syntaxdefs/rmarkdown.xml \
	syntaxdefs/roff.xml \
	syntaxdefs/rpmspec.xml \
	syntaxdefs/rsiidl.xml \
	syntaxdefs/rtf.xml \
	syntaxdefs/ruby.xml \
	syntaxdefs/rust.xml \
	syntaxdefs/sather.xml \
	syntaxdefs/scala.xml \
	syntaxdefs/scheme.xml \
	syntaxdefs/sci.xml \
	syntaxdefs/scss.xml \
	syntaxdefs/sed.xml \
	syntaxdefs/sgml.xml \
	syntaxdefs/sieve.xml \
	syntaxdefs/sisu.xml \
	syntaxdefs/sml.xml \
	syntaxdefs/spice.xml \
	syntaxdefs/sql-mysql.xml \
	syntaxdefs/sql-oracle.xml \
	syntaxdefs/sql-postgresql.xml \
	syntaxdefs/sql.xml \
	syntaxdefs/stata.xml \
	syntaxdefs/stl.xml \
	syntaxdefs/systemc.xml \
	syntaxdefs/systemverilog.xml \
	syntaxdefs/tads3.xml \
	syntaxdefs/taskjuggler.xml \
	syntaxdefs/tcl.xml \
	syntaxdefs/tcsh.xml \
	syntaxdefs/template-toolkit.xml \
	syntaxdefs/texinfo.xml \
	syntaxdefs/textile.xml \
	syntaxdefs/tibasic.xml \
	syntaxdefs/toml.xml \
	syntaxdefs/txt2tags.xml \
	syntaxdefs/uscript.xml \
	syntaxdefs/vala.xml \
	syntaxdefs/valgrind-suppression.xml \
	syntaxdefs/varnish.xml \
	syntaxdefs/varnish4.xml \
	syntaxdefs/varnishcc.xml \
	syntaxdefs/varnishcc4.xml \
	syntaxdefs/varnishtest.xml \
	syntaxdefs/varnishtest4.xml \
	syntaxdefs/vcard.xml \
	syntaxdefs/velocity.xml \
	syntaxdefs/vera.xml \
	syntaxdefs/verilog.xml \
	syntaxdefs/vhdl.xml \
	syntaxdefs/vrml.xml \
	syntaxdefs/wavefront-obj.xml \
	syntaxdefs/winehq.xml \
	syntaxdefs/wml.xml \
	syntaxdefs/xharbour.xml \
	syntaxdefs/xml.xml \
	syntaxdefs/xmldebug.xml \
	syntaxdefs/xonotic-console.xml \
	syntaxdefs/xorg.xml \
	syntaxdefs/xslt.xml \
	syntaxdefs/xul.xml \
	syntaxdefs/yacas.xml \
	syntaxdefs/yacc.xml \
	syntaxdefs/yaml.xml \
	syntaxdefs/yang.xml \
	syntaxdefs/zonnon.xml \
	syntaxdefs/zsh.xml

RESOURCES += \
	resources.qrc

include($$PWD/tools/QsLog/QsLog.pri)

DISTFILES += \
	server/server.pl
