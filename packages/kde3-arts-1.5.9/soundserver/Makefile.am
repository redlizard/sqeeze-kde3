# $Id: Makefile.am 434809 2005-07-15 13:20:54Z mueller $

# necessary for artswrapper
artsdpath = $(bindir)/artsd

INCLUDES = -I$(top_srcdir)/mcop -I$(top_builddir)/mcop -I$(top_srcdir)/flow \
   -I$(top_builddir)/flow -DEXECUTE=\"$(artsdpath)\" $(all_includes)
MCOPINC   = -I$(srcdir) -I$(top_srcdir)/flow -I$(top_srcdir)/mcop
FLOWLIBS  = $(top_builddir)/flow/libartsflow.la

####### install idl files

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = soundserver.h soundserver.idl kmedia2.h kmedia2.idl \
	artsversion.h

####### libsoundserver_idl, libkmedia2_idl (idl files as library)

lib_LTLIBRARIES = libkmedia2_idl.la libsoundserver_idl.la \
	libartsgslplayobject.la libartswavplayobject.la libkmedia2.la

AM_LDFLAGS = $(LDFLAGS_AS_NEEDED)

libsoundserver_idl_la_LIBADD = libkmedia2_idl.la \
                          $(top_builddir)/flow/libartsflow_idl.la
libsoundserver_idl_la_LDFLAGS = -no-undefined -version-info 1:0 $(all_libraries)
libsoundserver_idl_la_SOURCES = soundserver.cc
libsoundserver_idl_la_COMPILE_FIRST = soundserver.h

libkmedia2_idl_la_SOURCES = kmedia2.cc
libkmedia2_idl_la_COMPILE_FIRST = kmedia2.h
libkmedia2_idl_la_LIBADD  = $(top_builddir)/flow/libartsflow.la
libkmedia2_idl_la_LDFLAGS = -no-undefined -version-info 1:0 $(all_libraries)

libkmedia2_la_SOURCES = fileinputstream_impl.cc stdoutwriter_impl.cc
libkmedia2_la_LIBADD  = libkmedia2_idl.la $(FLOWLIBS)
libkmedia2_la_LDFLAGS = -no-undefined -version-info 1:0 $(all_libraries)
libkmedia2_la_COMPILE_FIRST = kmedia2.h ../flow/artsflow.h

libartswavplayobject_la_SOURCES = wavplayobject_impl.cc
libartswavplayobject_la_LIBADD  = $(top_builddir)/mcop/libmcop.la \
                                  libsoundserver_idl.la $(FLOWLIBS)
libartswavplayobject_la_LDFLAGS = -no-undefined -module $(all_libraries)
libartswavplayobject_la_COMPILE_FIRST = soundserver.h ../flow/artsflow.h

libartsgslplayobject_la_SOURCES = gslplayobject_impl.cc
libartsgslplayobject_la_LIBADD  = $(top_builddir)/mcop/libmcop.la \
                                  libsoundserver_idl.la $(FLOWLIBS)
libartsgslplayobject_la_LDFLAGS = -no-undefined -module $(all_libraries)
libartsgslplayobject_la_COMPILE_FIRST = soundserver.h ../flow/artsflow.h

###### "real" programs

bin_PROGRAMS = artsd artsplay artscat artswrapper artsshell artsrec

artsd_LDADD = libsoundserver_idl.la $(FLOWLIBS) \
              $(top_builddir)/mcop_mt/libmcop_mt.la
artsd_LDFLAGS = $(USE_THREADS)
artsd_SOURCES = soundserverv2_impl.cc soundserver_impl.cc simplesoundserver_impl.cc artsd.cc cpuusage.cc samplestorage_impl.cc crashhandler.cc soundserverstartup_impl.cc
artsd_COMPILE_FIRST = soundserver.h artsversion.h

artscat_LDADD = libsoundserver_idl.la $(FLOWLIBS) $(LIBPTHREAD)
artscat_LDFLAGS = $(USE_THREADS)
artscat_SOURCES = artscat.cc
artscat_COMPILE_FIRST = soundserver.h artsversion.h 

artsrec_LDADD = libsoundserver_idl.la $(FLOWLIBS) $(LIBPTHREAD)
artsrec_LDFLAGS = $(USE_THREADS)
artsrec_SOURCES = artsrec.cc
artsrec_COMPILE_FIRST = soundserver.h artsversion.h

artsplay_LDADD = libsoundserver_idl.la $(LIBPTHREAD)
artsplay_LDFLAGS = $(USE_THREADS)
artsplay_SOURCES = artsplay.cc
artsplay_COMPILE_FIRST = soundserver.h artsversion.h

artswrapper_SOURCES = artswrapper.c
artswrapper_CFLAGS = $(KDE_USE_FPIE)
artswrapper_INCLUDES = -I$(top_builddir) -DEXECUTE=\"$(artsdpath)\"
artswrapper_LDADD = $(LIBPOSIX4)
artswrapper_LDFLAGS = $(KDE_USE_PIE)

artsshell_LDADD = libsoundserver_idl.la $(LIBPTHREAD)
artsshell_LDFLAGS = $(USE_THREADS)
artsshell_SOURCES = artsshell.cc tradercheck.cc
artsshell_COMPILE_FIRST = soundserver.h artsversion.h ../flow/artsflow.h

DISTCLEANFILES = soundserver.cc soundserver.h soundserver.mcopclass \
  soundserver.mcoptype kmedia2.h kmedia2.cc kmedia2.mcopclass kmedia2.mcoptype artsversion.h

####### Build rules
soundserver.mcoptype: soundserver.h
soundserver.mcopclass: soundserver.h
soundserver.cc soundserver.h: $(top_srcdir)/soundserver/soundserver.idl kmedia2.h $(MCOPIDL) ../flow/artsflow.h
	$(MCOPIDL) -t $(MCOPINC) $(top_srcdir)/soundserver/soundserver.idl

kmedia2.mcoptype: kmedia2.h
kmedia2.mcopclass: kmedia2.h
kmedia2.cc kmedia2.h: $(top_srcdir)/soundserver/kmedia2.idl $(MCOPIDL) ../flow/artsflow.h
	$(MCOPIDL) -t $(MCOPINC) $(top_srcdir)/soundserver/kmedia2.idl


artsversion.h: artsversion-new.h
	(cmp -s artsversion-new.h artsversion.h \
	  || cp artsversion-new.h artsversion.h) 

if HAVE_WAVSUPPORT
WAV_MCOPCLASS = WavPlayObject.mcopclass 
endif

mcopclassdir = $(libdir)/mcop/Arts
mcopclass_DATA = FileInputStream.mcopclass $(WAV_MCOPCLASS) GSLPlayObject.mcopclass

#install-exec-hook: 
#	@(chown root $(DESTDIR)$(bindir)/artswrapper && chmod 4755 $(DESTDIR)$(bindir)/artswrapper) \
#		|| echo "please make $(DESTDIR)$(bindir)/artswrapper suid root"

######## install idl typeinfo files

mcoptypedir = $(libdir)/mcop
mcoptype_DATA = soundserver.mcoptype soundserver.mcopclass \
	kmedia2.mcoptype kmedia2.mcopclass
