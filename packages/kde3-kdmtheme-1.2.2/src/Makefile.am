INCLUDES    = $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kcm_kdmtheme.la

kcm_kdmtheme_la_SOURCES = kdmtheme.cpp
kcm_kdmtheme_la_LIBADD = -lkio -lknewstuff $(LIB_KDEUI)
kcm_kdmtheme_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries)


xdg_apps_DATA = kdmtheme.desktop


messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkdmtheme.pot
