AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes) -I$(kde_includes)/kwin

kwindir = $(kde_datadir)/kwin/
kwin_DATA = knifty.desktop

noinst_HEADERS = knifty.h kniftyclient.h kniftybutton.h misc.h shadow.h

kde_module_LTLIBRARIES = kwin3_knifty.la
kwin3_knifty_la_SOURCES = knifty.cpp kniftyclient.cpp kniftybutton.cpp misc.cpp shadow.cpp
kwin3_knifty_la_LIBADD = $(kde_libraries)/libkdecorations.la
kwin3_knifty_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkdecore -module
kwin3_knifty_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin3_knifty_la_METASOURCES)
