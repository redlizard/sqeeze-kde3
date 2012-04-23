AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes) -I$(kde_includes)/kwin

kwindir = $(kde_datadir)/kwin/
kwin_DATA = powder.desktop

noinst_HEADERS = embeddata.h powderclient.h powderhandler.h powderbutton.h enums.h pixmaps.h

kde_module_LTLIBRARIES = kwin3_powder.la

kwin3_powder_la_LIBADD = $(LIB_KDEUI) $(kde_libraries)/libkdecorations.la
kwin3_powder_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin3_powder_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin_powder_la_METASOURCES)
kwin3_powder_la_SOURCES = powderclient.cpp powderhandler.cpp powderbutton.cpp pixmaps.cpp embeddata.cpp
