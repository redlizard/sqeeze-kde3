# this makefile defines:
#
# GSL_C_SRC		C source files which need to be added to *_SOURCES
# GSL_NOINST_PROGS	GSL test programs, needs to be added to noinst_PROGRAMS
#
# this makefile supports:
#
# GSL_progs_ldadd	add link options to this varibale, which are required
#			to link GSL_NOINST_PROGS targets
# gslincludedir		directory to install public headers into if not empty
#
# this makefile introduces:
#
# gsltestoutput:	make target to generate a sample set of test data

# make sure gslincludedir= is defined
# GSL targets
GSL_H_SRC     =	gslcommon.h gsldatacache.h gsldatahandle.h gsldefs.h \
		gslloader.h gslmath.h gslfilter.h gsldatautils.h gsldatahandle-vorbis.h \
		gslconvert.h gslfft.h gslieee754.h gslsignal.h gslmagic.h \
		gslengine.h gslwaveosc.h gslwavechunk.h gsldatahandle-mad.h \
		gslosctable.h gsloscillator.h
GSL_C_SRC     =	gsldatacache.c gsldatahandle.c gslwavechunk.c gsldatahandle-vorbis.c \
		gslmath.c gslfilter.c gslcommon.c gsldatautils.c gslmagic.c \
		gslloader-wav.c gslloader-gslwave.c gslloader-mad.c gslloader-oggvorbis.c \
		gslconvert.c gslfft.c gslsignal.c gslloader.c gslwaveosc.c \
		gslengine.c gsloputil.c gslopmaster.c gslopschedule.c gsldatahandle-mad.c \
		gslosctable.c gsloscillator.c
GSL_EXTRA_SRC =	gslconfig.h gsloputil.h gslopmaster.h gslopnode.h \
		gslopschedule.h gslincluder.c gslwaveosc-aux.c gsloscillator-aux.c
GSL_EXTRA_DAT = gsl.gnuplot gsl-mplan.txt gslarrows gslwave.header gslglib.c gslglib.h gsl-fftgen.pl
EXTRA_DIST   +=	$(GSL_H_SRC) $(GSL_EXTRA_SRC) $(GSL_EXTRA_DAT)

GSL_NOINST_PROGS = gslwchunk gsltests gslffttest
gslwchunk_SOURCES = gslwchunk.c
gslwchunk_LDADD = $(GSL_progs_ldadd)
gslwchunk_LDFLAGS = $(USE_THREADS)
gsltests_SOURCES = gsltests.c
gsltests_LDADD = $(GSL_progs_ldadd)
gsltests_LDFLAGS = $(USE_THREADS)
gslffttest_SOURCES = gslffttest.c
gslffttest_LDADD = $(GSL_progs_ldadd)
gslffttest_LDFLAGS = $(USE_THREADS)

$(srcdir)/gslfft.c: $(srcdir)/gsl-fftgen.pl $(srcdir)/gsl-fftconf.sh
	cd $(srcdir) && ./gsl-fftconf.sh 'perl ./gsl-fftgen.pl' \"gslfft.h\" >gslfft.c
MAINTAINERCLEANFILES += gslfft.c

$(srcdir)/gslwchunk.c: gslconfig.h
$(srcdir)/gsltests.c: gslconfig.h
$(srcdir)/gslffttest.c: gslconfig.h
# $(OBJECTS): gslconfig.h

gsl_public_HEADERS = $(GSL_H_SRC) gslconfig.h
gsl_publicdir = $(gslincludedir)

.PHONY:	gsltestoutput 

gsltestoutput:
	@./gsltests blp     7 0.3		0.1211
	@./gsltests bhp    12 1.8332		0.1033
	@./gsltests bbp    14 0.5 0.6		0.1033
	@./gsltests bbs    12 1.5 1.6		0.2
	@./gsltests t1l     7 0.3		0.0125
	@./gsltests t1h     8 1.8332		0.1033
	@./gsltests t1p     6 0.5 0.6		0.1033
	@./gsltests t1s     8 1.4 1.5		0.25
	@./gsltests t2l    13 0.1	1.1	0.176
	@./gsltests t2h    10 1.14	1.2	0.0763
	@./gsltests t2p    14 0.7 0.8	1.3	0.0763
	@./gsltests t2p    10 0.9 1.2	1.4	0.15
	@./gsltests t2s    10 0.9 1.2	1.1	0.15
	@echo -n "plot [0:pi] [-96:1]"
	@echo -n " dB(BL7(Z(x))),"
	@echo -n " dB(BH12(Z(x))),"
	@echo -n " dB(BP14(Z(x))),"
	@echo -n " dB(BS12(Z(x))),"
	@echo -n " dB(T1L7(Z(x))),"
	@echo -n " dB(T1H8(Z(x))),"
	@echo -n " dB(T1P6(Z(x))),"
	@echo -n " dB(T1S8(Z(x))),"
	@echo -n " dB(T2L13(Z(x))),"
	@echo -n " dB(T2H10(Z(x))),"
	@echo -n " dB(T2P14(Z(x))),"
	@echo -n " dB(T2P10(Z(x))),"
	@echo -n " dB(T2S10(Z(x))),"
	@echo " -3"
