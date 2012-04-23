INCLUDES                     = $(all_includes)

kde_module_LTLIBRARIES       = kwin_powder_config.la

kwin_powder_config_la_SOURCES = powderconfig.cpp configdialog.ui
kwin_powder_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin_powder_config_la_LIBADD  = $(LIB_KDEUI)

METASOURCES                  = AUTO
# noinst_HEADERS                 = powderconfig.h configdialog.h
noinst_HEADERS               = powderconfig.h
DISTCLEANFILES               = $(METASOURCES)

lnkdir                       = $(kde_datadir)/kwin

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kwin_powder_config.pot

# AUTOMAKE_OPTIONS = foreign
#
# KDE_CXXFLAGS = -DQT_PLUGIN
#
# INCLUDES = $(all_includes)
#
# noinst_HEADERS = powderconfig.h configdialog.h
#
# kde_module_LTLIBRARIES = kwin_powder_config.la
# kwin_powder_config_la_SOURCES = powderconfig.cpp configdialog.ui
# kwin_powder_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
# kwin_powder_config_la_LIBADD = $(LIB_KDEUI)
# kwin_powder_config_la_METASOURCES = AUTO
#
# DISTCLEANFILES = $(kwin_powder_la_METASOURCES)
#
# messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kwin_powder_config.pot
