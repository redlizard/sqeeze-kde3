AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes)

stylercdir = $(kde_datadir)/kstyle/themes
stylerc_DATA = lipstik.themerc

colorshemedir = $(kde_datadir)/kdisplay/color-schemes
colorsheme_DATA = lipstikstandard.kcsrc lipstikwhite.kcsrc lipstiknoble.kcsrc

EXTRA_DIST = $(stylerc_DATA)

noinst_HEADERS = lipstik.h misc.h

kde_style_LTLIBRARIES = lipstik.la
lipstik_la_SOURCES = lipstik.cpp misc.cpp
lipstik_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
lipstik_la_LIBADD = -lkdefx
lipstik_la_METASOURCES = AUTO

DISTCLEANFILES = $(lipstik_la_METASOURCES)
