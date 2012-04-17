INCLUDES = $(all_includes)

noinst_HEADERS = lipstikconf.h
kde_module_LTLIBRARIES = kstyle_lipstik_config.la
kstyle_lipstik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kstyle_lipstik_config_la_LIBADD  = $(LIB_KDEUI)
kstyle_lipstik_config_la_SOURCES = lipstikconf.cpp
kstyle_lipstik_config_la_METASOURCES = AUTO


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_lipstik_config.pot
