KDE_CXXFLAGS = $(NOOPT_CXXFLAGS) 
INCLUDES = -I$(top_srcdir)/mcop -I$(top_builddir)/mcop $(all_includes)
####### Files

bin_PROGRAMS = mcopidl

mcopidl_SOURCES = mcopidl.cc yacc.cc scanner.cc namespace.cc
mcopidl_LDFLAGS = $(all_libraries)
mcopidl_LDADD = ../mcop/libmcop.la  $(LIBPTHREAD)
noinst_HEADERS = yacc.cc.h

mcopidl.o: $(top_srcdir)/mcop/common.h

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cc scanner.ll  ;\
	bison -d -t -o yacc.cc yacc.yy

