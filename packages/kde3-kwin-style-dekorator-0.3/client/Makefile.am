AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes) -I$(kde_includes)/kwin

kwindir = $(kde_datadir)/kwin/
kwin_DATA = deKorator.desktop

noinst_HEADERS = deKoratorclient.h shadow.h

kde_module_LTLIBRARIES = kwin3_deKorator.la
kwin3_deKorator_la_SOURCES = deKoratorclient.cc shadow.cpp
kwin3_deKorator_la_LIBADD = $(kde_libraries)/libkdecorations.la
kwin3_deKorator_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN) -lkdecore
kwin3_deKorator_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin3_deKorator_la_METASOURCES)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kwin_deKorator.po
