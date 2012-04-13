
SUBDIRS = mcopclass gsl gslpp
INCLUDES = -I$(top_srcdir)/mcop -I$(top_builddir)/mcop $(all_includes)
AM_CXXFLAGS = $(MAS_CFLAGS) $(JACK_CFLAGS) -DQT_CLEAN_NAMESPACE

####### Files

lib_LTLIBRARIES = libartsflow_idl.la libartsflow.la

libartsflow_idl_la_SOURCES = artsflow.cc
libartsflow_idl_la_LDFLAGS = -no-undefined -version-info 1:0 $(all_libraries)
libartsflow_idl_la_LIBADD = $(top_builddir)/mcop/libmcop.la $(LIBPOSIX4)

libartsflow_la_LIBADD = $(top_builddir)/mcop/libmcop.la libartsflow_idl.la $(top_builddir)/flow/gslpp/libgslpp.la $(LIBAUDIOFILE) $(LIBASOUND) $(LIBAUDIOIO) $(LIBOSSAUDIO) $(LIBAUDIONAS) $(LIBCSL) $(SGILIBAUDIO) $(LIBESD) $(LIBMAS) $(JACK_LIBADD) -lm \
  $(top_builddir)/flow/gsl/libgsl.la
libartsflow_la_LDFLAGS = $(MAS_LDFLAGS) $(JACK_LDFLAGS) $(LIBAUDIOFILE_LDFLAGS) $(LIBAUDIONAS_LDFLAGS) $(LIBESD_LDFLAGS) -no-undefined -version-info 1:0
libartsflow_la_COMPILE_FIRST = artsflow.h
libartsflow_la_SOURCES =  synth_play_impl.cc \
  gslschedule.cc audiosubsys.cc \
  pipebuffer.cc convert.cc synth_wave_sin_impl.cc synth_frequency_impl.cc \
  synth_multi_add_impl.cc synth_add_impl.cc synth_mul_impl.cc \
  synth_play_wav_impl.cc stdsynthmodule.cc cache.cc asyncschedule.cc \
  bytestreamtoaudio_impl.cc stereovolumecontrol_impl.cc \
  stereoeffectstack_impl.cc fft.c stereofftscope_impl.cc virtualports.cc \
  bus.cc audiomanager_impl.cc synth_record_impl.cc resample.cc \
  audioio.cc audioiooss.cc audioioalsa.cc audioioalsa9.cc \
  audioionull.cc audioiolibaudioio.cc audioioesd.cc audioiojack.cc \
  audioiosun.cc audioioaix.cc audioionas.cc cpuinfo.cc \
  audioioossthreaded.cc audiotobytestream_impl.cc audioiosgi.cc \
  audioiocsl.cc audioiomas.cc datahandle_impl.cc

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = artsflow.h audiosubsys.h cache.h \
  cachedwav.h convert.h pipebuffer.h stdsynthmodule.h \
  synthschedule.h fft.h artsflow.idl audioio.h resample.h \
  cpuinfo.h bufferqueue.h gslschedule.h

DISTCLEANFILES = artsflow.cc artsflow.h artsflow.mcoptype artsflow.mcopclass

artsflow.cc artsflow.h: $(top_srcdir)/flow/artsflow.idl $(MCOPIDL)
	$(MCOPIDL) -t $(top_srcdir)/flow/artsflow.idl

artsflow.mcoptype: artsflow.h
artsflow.mcopclass: artsflow.h

######## install idl typeinfo files

mcoptypedir = $(libdir)/mcop
mcoptype_DATA = artsflow.mcoptype artsflow.mcopclass
