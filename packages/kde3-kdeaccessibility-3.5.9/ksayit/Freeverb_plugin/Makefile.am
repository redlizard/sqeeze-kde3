INCLUDES = -I$(top_srcdir)/ksayit/src $(all_includes)
METASOURCES = AUTO
kde_servicetypes_DATA = ksayit_libFreeverb_service.desktop
kde_services_DATA = ksayit_libFreeverb.desktop
fxpluginsdir = $(kde_libraries)/kde3
fxplugins_LTLIBRARIES = libFreeverb_plugin.la
libFreeverb_plugin_la_LDFLAGS = -avoid-version -module -no-undefined \
	$(all_libraries)
libFreeverb_plugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) -lartskde\
	-lartsmodules
noinst_HEADERS = freeverbsetupimpl.h ksayitfreeverblib.h kdeexportfix.h
libFreeverb_plugin_la_SOURCES = freeverbsetupimpl.cpp freeverb_setup.ui \
	ksayitfreeverblib.cpp
