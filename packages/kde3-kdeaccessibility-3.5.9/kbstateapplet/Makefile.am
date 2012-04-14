INCLUDES = $(all_includes) 

kde_module_LTLIBRARIES = kbstate_panelapplet.la

kbstate_panelapplet_la_SOURCES = kbstate.cpp

METASOURCES = AUTO
noinst_HEADERS = kbstate.h kdeexportfix.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = kbstateapplet.desktop

EXTRA_DIST = $(lnk_DATA)

kbstate_panelapplet_la_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
kbstate_panelapplet_la_LIBADD = $(LIB_KDEUI)

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kbstateapplet.pot

srcdoc:
	kdoc -a -p -H -d $$HOME/web/src/kbstateapplet kbstateapplet *.h -lqt -lkdecore -lkdeui -lkfile

SUBDIRS = images
