AUTOMAKE_OPTIONS = foreign

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes)

noinst_HEADERS = deKoratorconfig.h themes.h

kde_module_LTLIBRARIES = kwin_deKorator_config.la
kwin_deKorator_config_la_SOURCES = deKoratorconfig.cc configdialog.ui \
	themes.cpp
kwin_deKorator_config_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
kwin_deKorator_config_la_LIBADD = $(LIB_KIO) $(LIB_KDEUI)
kwin_deKorator_config_la_METASOURCES = AUTO

DISTCLEANFILES = $(kwin_deKorator_config_la_METASOURCES)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kwin_deKorator_config.pot
