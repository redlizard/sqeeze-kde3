INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kwin_domino_config.la

kwin_domino_config_la_SOURCES = config.cpp
kwin_domino_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin_domino_config_la_LIBADD = $(LIB_KDEUI) $(LIB_QT) $(LIB_KDECORE)

METASOURCES = AUTO
noinst_HEADERS = config.h
