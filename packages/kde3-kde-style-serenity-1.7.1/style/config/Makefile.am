INCLUDES = $(all_includes)

METASOURCES = AUTO

noinst_HEADERS = serenityconf.h
kde_module_LTLIBRARIES = kstyle_serenity_config.la
kstyle_serenity_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kstyle_serenity_config_la_LIBADD  = $(LIB_KDEUI)
kstyle_serenity_config_la_SOURCES = serenityconf.cpp configdialog.ui

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_serenity_config.pot
