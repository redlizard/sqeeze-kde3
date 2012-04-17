INCLUDES = $(all_includes)

noinst_HEADERS = comixconf.h
kde_module_LTLIBRARIES = kstyle_comix_config.la
kstyle_comix_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kstyle_comix_config_la_LIBADD  = $(LIB_KDEUI)
kstyle_comix_config_la_SOURCES = comixconf.cpp
kstyle_comix_config_la_METASOURCES = AUTO


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_comix_config.pot
