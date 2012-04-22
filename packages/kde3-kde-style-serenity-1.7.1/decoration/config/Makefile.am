INCLUDES                     = $(all_includes)

kde_module_LTLIBRARIES       = kwin_serenity_config.la

kwin_serenity_config_la_SOURCES = serenityconfig.cpp configdialog.ui
kwin_serenity_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin_serenity_config_la_LIBADD  = $(LIB_KDEUI)

METASOURCES                  = AUTO
# noinst_HEADERS                 = serenityconfig.h configdialog.h
noinst_HEADERS               = serenityconfig.h
DISTCLEANFILES               = $(METASOURCES)

lnkdir                       = $(kde_datadir)/kwin

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kwin_serenity_config.pot

# AUTOMAKE_OPTIONS = foreign
#
# KDE_CXXFLAGS = -DQT_PLUGIN
#
# INCLUDES = $(all_includes)
#
# noinst_HEADERS = serenityconfig.h configdialog.h
#
# kde_module_LTLIBRARIES = kwin_serenity_config.la
# kwin_serenity_config_la_SOURCES = serenityconfig.cpp configdialog.ui
# kwin_serenity_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
# kwin_serenity_config_la_LIBADD = $(LIB_KDEUI)
# kwin_serenity_config_la_METASOURCES = AUTO
#
# DISTCLEANFILES = $(kwin_serenity_la_METASOURCES)
#
# messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kwin_serenity_config.pot
