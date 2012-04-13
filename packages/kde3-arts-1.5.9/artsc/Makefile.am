AM_CFLAGS = -DARTSC_BACKEND='"$(libdir)/libartscbackend.la"' 
AM_CPPFLAGS = -DCOMPILING_ARTSC
lib_LTLIBRARIES = libartsc.la libartscbackend.la libartsdsp.la libartsdsp_st.la
FLOWLIBS = $(top_builddir)/flow/libartsflow.la

INCLUDES = -I$(top_srcdir)/mcop -I$(top_builddir)/mcop -I$(top_srcdir)/flow \
      -I$(top_builddir)/flow -I$(top_builddir)/soundserver \
      -I$(top_srcdir)/libltdl $(all_includes)

bin_SCRIPTS = artsc-config artsdsp

libartsdsp_la_SOURCES = artsdsp.c
libartsdsp_la_LDFLAGS = -no-undefined -module $(all_libraries)
libartsdsp_la_LIBADD = libartsc.la

libartsdsp_st_la_SOURCES = artsc.c artsdsp.c
libartsdsp_st_la_LDFLAGS = -no-undefined -module $(all_libraries)
libartsdsp_st_la_LIBADD = $(top_builddir)/libltdl/libltdlc.la

libartsc_la_SOURCES = artsc.c
libartsc_la_LDFLAGS = -no-undefined $(USE_THREADS) $(all_libraries)
libartsc_la_LIBADD = $(top_builddir)/libltdl/libltdlc.la $(LIBPTHREAD)

libartscbackend_la_SOURCES = artscbackend.cc
libartscbackend_la_LDFLAGS = -no-undefined -module $(KDE_RPATH) $(all_libraries)
libartscbackend_la_LIBADD = $(FLOWLIBS) \
    $(top_builddir)/soundserver/libsoundserver_idl.la

artscincludedir = $(includedir)/artsc
artscinclude_HEADERS = artsc.h artsc_export.h

artscbackend.lo: $(top_builddir)/soundserver/soundserver.h ../flow/artsflow.h
