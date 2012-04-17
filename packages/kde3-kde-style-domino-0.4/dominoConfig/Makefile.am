INCLUDES = $(all_includes) -I/usr/include/freetype2

noinst_HEADERS = dominoconf.h
kde_module_LTLIBRARIES = kstyle_domino_config.la
kstyle_domino_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module 
kstyle_domino_config_la_LIBADD  = $(LIB_KDEUI) ../domino/domino.la $(LIB_QT) $(LIB_KDECORE) -lkdefx $(LIB_KIO)
kstyle_domino_config_la_SOURCES = dominoconf.cpp
kstyle_domino_config_la_METASOURCES = AUTO
