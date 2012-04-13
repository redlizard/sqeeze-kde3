
EXTRA_DIST =
CLEANFILES =
MAINTAINERCLEANFILES =

gslincludedir=$(includedir)/arts/gsl
include gslmakefile.inc

INCLUDES = -I$(top_srcdir)/flow -I$(top_builddir)/flow -I$(top_srcdir)/mcop -I$(top_builddir)/mcop -I$(top_builddir) $(all_includes)
AM_CFLAGS = $(GSL_CFLAGS)

noinst_LTLIBRARIES = libgsl.la

libgsl_la_SOURCES = $(GSL_C_SRC) gslfilehash.c gslartsthreads.cc
libgsl_la_LIBADD = $(top_builddir)/mcop/libmcop.la -lm $(LIBPOSIX4) $(GSL_LIBS) $(LIB_POLL) $(GLIB_LIBADD)
libgsl_la_LDFLAGS = -no-undefined $(all_libraries)

#noinst_PROGRAMS = $(GSL_NOINST_PROGS)

GSL_progs_ldadd = libgsl.la -lm $(LIBPTHREAD)

GSL_cc_dummy = dummy.cc
$(srcdir)/dummy.cc: gslconfig.h

