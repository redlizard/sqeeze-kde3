check_PROGRAMS = testbuffer testifacerepo testanyref testwrapper \
   testchangenotify testflowsystem testdispatcher testnotification \
   testremote

TESTS = $(check_PROGRAMS)

INCLUDES = -I$(top_srcdir)/mcop -I$(top_srcdir)/flow \
   -I$(top_builddir)/flow -I$(top_srcdir)/soundserver \
   -I$(top_builddir)/soundserver -I$(top_srcdir)/artsc \
   -I$(top_builddir)/mcop $(all_includes)

AM_LDFLAGS  = $(all_libraries)
LDADD    = $(top_builddir)/mcop/libmcop.la
FLOWLIBS = $(top_builddir)/flow/libartsflow.la

testbuffer_SOURCES = testbuffer.cc
testifacerepo_SOURCES = testifacerepo.cc
testanyref_SOURCES = testanyref.cc
testdispatcher_SOURCES = testdispatcher.cc
testnotification_SOURCES = testnotification.cc
testwrapper_SOURCES = wrapper.cc testwrapper.cc

testchangenotify_LDADD = $(FLOWLIBS)
testchangenotify_SOURCES = value.cc value_impl.cc testchangenotify.cc

testflowsystem_LDADD = $(FLOWLIBS)
testflowsystem_SOURCES = value.cc value_impl.cc testflowsystem.cc

testremote_LDADD = $(FLOWLIBS)
testremote_SOURCES = remotetest.cc value.cc value_impl.cc testremote.cc

DISTCLEANFILES = wrapper.h wrapper.cc value.h value.cc \
				 remotetest.h remotetest.cc

wrapper.cc wrapper.h: $(top_srcdir)/tests/wrapper.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/tests/wrapper.idl

value_impl.o: value.h
value.cc value.h: $(top_srcdir)/tests/value.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/tests/value.idl

remotetest.cc remotetest.h: $(top_srcdir)/tests/remotetest.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/tests/remotetest.idl

