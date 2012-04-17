if single_style
qtc_sub_dirs=config
endif

AUTOMAKE_OPTIONS = foreign

SUBDIRS = $(qtc_sub_dirs)

AM_CPPFLAGS = -DQT_PLUGIN
INCLUDES = -I$(top_srcdir)/kdefx $(all_includes)
noinst_HEADERS = klearlook.h common.h
kde_style_LTLIBRARIES = klearlook.la
klearlook_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
klearlook_la_LIBADD = -lkdefx 
klearlook_la_SOURCES = klearlook.cpp plugin.cpp common.c
klearlook_la_METASOURCES = AUTO

if single_style
qtc_themerc_files = klearlook.themerc
else
qtc_themerc_files = klearlook.themerc klearlook_v2.themerc klearlook_v3.themerc
endif

stylercdir = $(kde_datadir)/kstyle/themes
stylerc_DATA = $(qtc_themerc_files)

EXTRA_DIST = $(stylerc_DATA) klearlook.themerc klearlook_v2.themerc klearlook_v3.themerc

DISTCLEANFILES = $(klearlook_la_METASOURCES)
