include( $$SRCDIR/tools/QsLog/QsLog.pri )

linux {
	# gcov
	QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
	LIBS += -lgcov
}
