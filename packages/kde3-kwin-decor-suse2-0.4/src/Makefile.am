INCLUDES = -I$(top_srcdir) $(all_includes)

SUBDIRS = config

kde_module_LTLIBRARIES = kwin3_SUSE2.la

kwin3_SUSE2_la_SOURCES = SUSE2.cpp SUSE2client.cpp SUSE2button.cpp misc.cpp shadow.cpp

kwin3_SUSE2_la_LIBADD = $(LIB_KDEUI) -lkdecorations
kwin3_SUSE2_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) config/*.ui >> rc.cpp
	$(XGETTEXT) *.cpp config/*.cpp -o $(podir)/kwin_SUSE2.pot

linkdir = $(kde_datadir)/kwin/
link_DATA = SUSE2.desktop

kwin_pics_data_DATA = titlebar_decor.png
kwin_pics_datadir = $(kde_datadir)/kwin/pics

kdisplay_colorscheme_DATA = SUSE2.kcsrc
kdisplay_colorschemedir = $(kde_datadir)/kdisplay/color-schemes

EXTRA_DIST = $(kwin_pics_data_DATA) $(link_DATA) $(kdisplay_colorscheme_DATA)

