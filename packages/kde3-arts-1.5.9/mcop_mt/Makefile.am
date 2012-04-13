INCLUDES = -I$(top_srcdir)/mcop -I$(top_builddir)/mcop -I$(top_srcdir)/flow -I$(top_builddir)/flow $(all_includes)

lib_LTLIBRARIES = libmcop_mt.la

libmcop_mt_la_SOURCES = threads_posix.cc
libmcop_mt_la_LIBADD = $(top_builddir)/mcop/libmcop.la \
	$(top_builddir)/flow/libartsflow.la $(LIBPTHREAD) \
	$(LIBPOSIX4) $(LIBSEM)
libmcop_mt_la_LDFLAGS = -no-undefined -version-info 1:0 $(USE_THREADS) $(all_libraries)
