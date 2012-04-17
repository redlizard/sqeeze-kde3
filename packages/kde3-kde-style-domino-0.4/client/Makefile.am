
INCLUDES = $(all_includes) -I$(kde_includes)/kwin

SUBDIRS = . config

kde_module_LTLIBRARIES = kwin3_domino.la

kwin3_domino_la_SOURCES = dominoclient.cpp
kwin3_domino_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module -L/usr/X11R6/lib 
kwin3_domino_la_LIBADD  = $(LIB_KDECORE) $(kde_libraries)/libkdecorations.la -lXext -lX11 $(LIB_QT)

METASOURCES = AUTO
noinst_HEADERS = dominoclient.h

lnkdir = $(kde_datadir)/kwin
lnk_DATA = domino.desktop
