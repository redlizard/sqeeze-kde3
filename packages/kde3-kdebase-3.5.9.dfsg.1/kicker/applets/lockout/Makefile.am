INCLUDES = $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = lockout_panelapplet.la

lockout_panelapplet_la_SOURCES = lockout.cpp
lockout_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
lockout_panelapplet_la_LIBADD = $(LIB_KSYCOCA)

noinst_HEADERS = lockout.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = lockout.desktop

EXTRA_DIST = $(lnk_DATA)

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/lockout.pot
