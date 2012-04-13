INCLUDES = -I$(top_srcdir)/mcop -I$(top_srcdir)/flow \
   -I$(top_builddir)/flow -I$(top_srcdir)/soundserver \
   -I$(top_builddir)/soundserver -I$(top_srcdir)/artsc \
   -I$(top_builddir)/mcop -I$(top_builddir)/artsc \
   $(all_includes)

AM_LDFLAGS = $(all_libraries) 
LDADD = $(top_builddir)/mcop/libmcop.la
FLOWLIBS = $(top_builddir)/flow/libartsflow.la

####### Files

check_PROGRAMS = helloserver helloclient referenceinfo \
  irdemo x11commtest streamsound hellomain flow artsctest \
  artscrec artsccat artscmt \
  dcasttest hellodynamic hellodynamic2 testasubsys testaggregation \
  playtofile testthreads testnothreads catfile testdhandle
helloserver_SOURCES = hello.cc helloserver.cc hello_impl.cc
helloclient_SOURCES = hello.cc helloclient.cc
hellodynamic_SOURCES = hello.cc hellodynamic.cc hello_impl.cc
hellodynamic2_SOURCES = hello.cc hellodynamic2.cc
referenceinfo_SOURCES = referenceinfo.cc

testdhandle_SOURCES = testdhandle.cc
testdhandle.o: ../soundserver/soundserver.h ../flow/artsflow.h
testdhandle_LDADD = $(FLOWLIBS) \
    $(top_builddir)/soundserver/libsoundserver_idl.la \
    $(top_builddir)/flow/gslpp/libgslpp.la

irdemo_SOURCES = irdemo.cc
x11commtest_SOURCES = x11commtest.cc
hellomain_SOURCES = hellomain.cc hello.cc hello_impl.cc

testasubsys_SOURCES = testasubsys.cc
testasubsys_LDADD = $(FLOWLIBS)

## testthreads/testnothreads:
## if we link testthreads.cc against libmcop_mt, it will be able to use
## real threading, if we don't, we won't
testthreads_SOURCES = testthreads.cc
testthreads_LDFLAGS = $(LDFLAGS)
testthreads_LDADD   = $(top_builddir)/mcop_mt/libmcop_mt.la
testnothreads_SOURCES = testthreads.cc

testaggregation_SOURCES = aggregation.cc testaggregation.cc

flow_LDADD = $(FLOWLIBS)
flow_SOURCES = flow.cc

streamsound_LDADD = $(FLOWLIBS) \
    $(top_builddir)/soundserver/libsoundserver_idl.la
streamsound_SOURCES = streamsound.cc

dcasttest_LDADD = $(FLOWLIBS) \
    $(top_builddir)/soundserver/libsoundserver_idl.la
dcasttest_SOURCES = dcasttest.cc

artsctest_LDADD = $(top_builddir)/artsc/libartsc.la
artsctest_SOURCES = artsctest.c

artsccat_LDADD = $(top_builddir)/artsc/libartsc.la
artsccat_SOURCES = artsccat.c

artscrec_LDADD = $(top_builddir)/artsc/libartsc.la
artscrec_SOURCES = artscrec.c

artscmt_LDFLAGS = $(LDFLAGS) $(USE_THREADS)
artscmt_LDADD = $(top_builddir)/artsc/libartsc.la -lm $(LIBPTHREAD) $(LIBPOSIX4)
artscmt_SOURCES = artscmt.c

playtofile_SOURCES = playtofile.cc playtofile_impl.cc playtofile_main.cc

playtofile_LDADD	= $(FLOWLIBS) \
    $(top_builddir)/soundserver/libsoundserver_idl.la \
    $(top_builddir)/soundserver/libkmedia2_idl.la

catfile_SOURCES = catfile.cc 

catfile_LDADD	= $(top_builddir)/soundserver/libkmedia2.la

DISTCLEANFILES = hello.cc hello.h aggregation.cc aggregation.h \
				 playtofile.cc playtofile.h

####### Implicit rules

####### Build rules

helloclient.o: hello.h
hello.cc hello.h: $(top_srcdir)/examples/hello.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/examples/hello.idl

aggregation.cc aggregation.h: $(top_srcdir)/examples/aggregation.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/examples/aggregation.idl

playtofile.cc playtofile.h: $(srcdir)/playtofile.idl $(MCOPIDL)
	$(MCOPIDL) -I$(top_srcdir)/flow $(srcdir)/playtofile.idl

