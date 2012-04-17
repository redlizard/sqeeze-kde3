INCLUDES = $(all_includes)
noinst_HEADERS = klearlookconfig.h
kde_module_LTLIBRARIES = kstyle_klearlook_config.la
kstyle_klearlook_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kstyle_klearlook_config_la_LIBADD  = $(LIB_KDEUI)
kstyle_klearlook_config_la_SOURCES = klearlookconfigbase.ui klearlookconfig.cpp
kstyle_klearlook_config_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_klearlook_config.pot
