AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

METASOURCES = AUTO

INCLUDES = $(all_includes)

stylercdir = $(kde_datadir)/kstyle/themes
stylerc_DATA = serenity.themerc

EXTRA_DIST = $(stylerc_DATA)

noinst_HEADERS = serenity.h misc.h

kde_style_LTLIBRARIES = serenity.la
serenity_la_SOURCES = serenity.cpp
serenity_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
serenity_la_LIBADD = -lkdefx -lkutils $(LIB_KDEUI)
