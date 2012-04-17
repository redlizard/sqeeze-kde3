AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

AM_CPPFLAGS = -DQT_PLUGIN
INCLUDES = -I$(top_srcdir)/kdefx $(all_includes)
noinst_HEADERS = qtcurve.h common.h
kde_style_LTLIBRARIES = qtcurve.la
qtcurve_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
qtcurve_la_LIBADD = -lkdefx 
qtcurve_la_SOURCES = qtcurve.cpp
qtcurve_la_METASOURCES = AUTO
qtc_themerc_files = qtcurve.themerc

pixmaps.h: radio_frame.png radio_light.png radio_on.png check_on.png slider.png slider_light.png slider_v.png slider_light_v.png
	qembed radio_frame.png radio_light.png radio_on.png check_on.png slider.png slider_light.png slider_v.png slider_light_v.png > pixmaps.h

qtcurve.lo: pixmaps.h

stylercdir = $(kde_datadir)/kstyle/themes
stylerc_DATA = $(qtc_themerc_files)

EXTRA_DIST = $(stylerc_DATA) qtcurve.themerc config_file.c radio_frame.png radio_light.png radio_on.png check_on.png \
slider.png slider_light.png slider_v.png slider_light_v.png  pixmaps.h

DISTCLEANFILES = $(qtcurve_la_METASOURCES)
