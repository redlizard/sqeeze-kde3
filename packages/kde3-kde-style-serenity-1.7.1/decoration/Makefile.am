AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes) -I$(kde_includes)/kwin

kwindir = $(kde_datadir)/kwin/
kwin_DATA = serenity.desktop

noinst_HEADERS = embeddata.h serenityclient.h serenityhandler.h serenitybutton.h enums.h pixmaps.h

kde_module_LTLIBRARIES = kwin3_serenity.la

kwin3_serenity_la_LIBADD = $(LIB_KDEUI) $(kde_libraries)/libkdecorations.la
kwin3_serenity_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin3_serenity_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin_serenity_la_METASOURCES)
kwin3_serenity_la_SOURCES = serenityclient.cpp serenityhandler.cpp serenitybutton.cpp pixmaps.cpp embeddata.cpp
