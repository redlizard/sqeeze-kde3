kde_services_DATA = kritapixelizefilter.desktop

INCLUDES  = \
	 -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../ui \
        -I$(srcdir)/../../../../lib/kofficecore \
        -I$(srcdir)/../../../../lib/kofficeui \
	$(all_includes)

kde_module_LTLIBRARIES = kritapixelizefilter.la

kritapixelizefilter_la_SOURCES = kis_pixelize_filter_plugin.cc \
	kis_pixelize_filter.cc

noinst_HEADERS = kis_pixelize_filter_plugin.h \
	kis_pixelize_filter.h

kritapixelizefilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritapixelizefilter_la_LIBADD =  ../../../libkritacommon.la

kritapixelizefilter_la_METASOURCES = AUTO
