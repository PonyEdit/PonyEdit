TEMPLATE = subdirs

SUBDIRS = \
	$$SRCDIR \
	$$TESTSDIR

OTHER_FILES += \
	README.md \
	COMMON-TASKS.md \
	.gitignore \
	.travis.yml \
	uncrustify.cfg \
	ponyedit.rc
