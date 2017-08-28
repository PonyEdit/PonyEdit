TEMPLATE = subdirs

SUBDIRS = \
	$$SRCDIR \
	$$TESTSDIR

OTHER_FILES += \
	README.md \
	COMMON-TASKS.md \
	.gitignore \
	.travis.yml \
	.qmake.conf \
	uncrustify.cfg \
	ponyedit.rc
