AUTOMAKE_OPTIONS = foreign

SUBDIRS	= config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes) -I$(kde_includes)/kwin

kwindir = $(kde_datadir)/kwin/
kwin_DATA = comix.desktop

noinst_HEADERS = comix.h comixClient.h comixCircle.h comixButton.h 

kde_module_LTLIBRARIES = kwin3_comix.la

kwin3_comix_la_SOURCES = comix.cpp comixClient.cpp comixCircle.cpp comixButton.cpp 
kwin3_comix_la_LIBADD = $(kde_libraries)/libkdecorations.la
kwin3_comix_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkdecore -module
kwin3_comix_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin3_comix_la_METASOURCES)
