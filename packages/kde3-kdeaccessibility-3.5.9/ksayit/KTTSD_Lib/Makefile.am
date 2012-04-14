INCLUDES = \
	$(KTTS_KSPEECH_INCLUDE) \
	$(all_includes)

METASOURCES = AUTO
libKTTSD_Lib_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 0:1
lib_LTLIBRARIES = libKTTSD_Lib.la
libKTTSD_Lib_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE)
noinst_HEADERS = kttsdlib.h kttsdlibsetupimpl.h kttsdlibtalker2.h kdeexportfix.h

kspeech_DIR = $(KTTS_KSPEECH_DIR)
kspeechsink_DIR = $(KTTS_KSPEECH_DIR)

libKTTSD_Lib_la_SOURCES = \
	kspeech.stub kspeechsink.skel \
	kttsdlib.cpp \
	kttsdlibsetupimpl.cpp \
	KTTSDlibSetup.ui \
	kttsdlibtalker2.cpp
AM_CXXFLAGS = -fPIC

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/libKTTSD.pot
