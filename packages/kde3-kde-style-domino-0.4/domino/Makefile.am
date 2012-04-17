AUTOMAKE_OPTIONS = foreign

KDE_CXXFLAGS = -DQT_PLUGIN -I/usr/include/freetype2

INCLUDES = $(all_includes)

stylercdir = $(kde_datadir)/kstyle/themes
stylerc_DATA = domino.themerc
schemedir = $(kde_datadir)/kdisplay/color-schemes
scheme_DATA = Domino.kcsrc

noinst_HEADERS = domino.h misc.h data.h

kde_style_LTLIBRARIES = domino.la
domino_la_SOURCES = domino.cpp misc.cpp
domino_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module $(KDE_RPATH)
domino_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS) $(LIB_QT) -lkdefx -lXrender -lXft $(LIB_KDECORE)
domino_la_METASOURCES = AUTO

DISTCLEANFILES = $(domino_la_METASOURCES)
