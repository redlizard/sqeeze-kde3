kde_module_LTLIBRARIES = libkritamagickimport.la libkritamagickexport.la

libkritamagickexport_la_LDFLAGS = $(KDE_PLUGIN) $(LIBMAGICK_LDFLAGS) $(KDE_RPATH) $(LIBMAGICK_RPATH) $(all_libraries) -module -avoid-version -no-undefined
libkritamagickexport_la_LIBADD = \
	$(KOFFICE_LIBS) \
	$(LIBMAGICK_LIBS) \
	$(top_builddir)/krita/libkritacommon.la

libkritamagickimport_la_LDFLAGS = $(KDE_PLUGIN) $(LIBMAGICK_LDFLAGS) $(KDE_RPATH) $(LIBMAGICK_RPATH) $(all_libraries) -module -avoid-version -no-undefined
libkritamagickimport_la_LIBADD = \
	$(KOFFICE_LIBS) \
	$(LIBMAGICK_LIBS) \
	$(top_builddir)/krita/libkritacommon.la

INCLUDES= \
	-I$(srcdir) \
	 $(KOFFICE_INCLUDES) \
	-I$(top_srcdir)/krita \
	-I$(top_srcdir)/krita/core \
	-I$(top_srcdir)/krita/sdk \
	-I$(top_srcdir)/krita/core/tiles \
	-I$(top_srcdir)/krita/kritacolor \
	-I$(top_srcdir)/krita/ui \
        $(KOFFICE_INCLUDES) -I$(interfacedir) \
        $(KOPAINTER_INCLUDES) $(LIBMAGICK_CPPFLAGS) \
	$(all_includes) 

service_DATA = krita_magick_import.desktop krita_magick_export.desktop
servicedir = $(kde_servicesdir)

kdelnk_DATA = krita_magick.desktop
kdelnkdir = $(kde_appsdir)/Office

libkritamagickimport_la_SOURCES = magickimport.cpp kis_image_magick_converter.cc
libkritamagickexport_la_SOURCES = magickexport.cpp kis_image_magick_converter.cc

METASOURCES = AUTO


KDE_CXXFLAGS = $(USE_EXCEPTIONS)
