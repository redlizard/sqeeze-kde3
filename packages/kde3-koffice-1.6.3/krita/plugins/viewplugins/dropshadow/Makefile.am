kritarcdir = $(kde_datadir)/kritaplugins
kritarc_DATA = dropshadow.rc
EXTRA_DIST = $(kritarc_DATA)

INCLUDES  =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../colorspaces/rgb_u8 \
	-I$(srcdir)/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)


kde_module_LTLIBRARIES = kritadropshadow.la

kritadropshadow_la_SOURCES = wdg_dropshadow.ui \
	kis_dropshadow.cc dlg_dropshadow.cc  \
	kis_dropshadow_plugin.cc

noinst_HEADERS = wdg_dropshadow.h kis_dropshadow_plugin.h \
		kis_dropshadow.h dlg_dropshadow.h

kritadropshadow_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritadropshadow_la_LIBADD =  ../../../libkritacommon.la ../../../colorspaces/rgb_u8/libkritargb.la

kde_services_DATA = kritadropshadow.desktop

kritadropshadow_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
