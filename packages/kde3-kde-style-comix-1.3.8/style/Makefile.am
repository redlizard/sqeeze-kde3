AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes)

stylercdir = $(kde_datadir)/kstyle/themes
stylerc_DATA = Comix.themerc

EXTRA_DIST = $(stylerc_DATA)

noinst_HEADERS = comix.h misc.h comixCircle.h

kde_style_LTLIBRARIES = comix.la
comix_la_SOURCES = comix.cpp misc.cpp comixCircle.cpp
comix_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
comix_la_LIBADD = -lkdefx
comix_la_METASOURCES = AUTO

DISTCLEANFILES = $(comix_la_METASOURCES)
