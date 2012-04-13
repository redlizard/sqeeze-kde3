lib_LTLIBRARIES = libgmcop.la

INCLUDES = -I$(top_srcdir)/mcop -I$(top_builddir)/mcop $(GLIB_CFLAGS) $(all_includes)

libgmcop_la_SOURCES = giomanager.cc
libgmcop_la_LIBADD = $(top_builddir)/mcop/libmcop.la $(GLIB_LIBADD)
libgmcop_la_LDFLAGS = -no-undefined -version-info 1:0 $(GLIB_LDFLAGS) $(all_libraries)

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = giomanager.h
