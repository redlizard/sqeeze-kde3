# Here resides the qextmdi library.

INCLUDES = -I$(top_srcdir)/lib/compatibility/kmdi/qextmdi/res -I$(top_builddir)/lib/compatibility/kmdi/qextmdi $(all_includes)

noinst_LTLIBRARIES = libquantaqextmdi.la
libquantaqextmdi_la_LDFLAGS = $(all_libraries)
libquantaqextmdi_la_LIBADD = $(LIB_KPARTS)

libquantaqextmdi_la_SOURCES =    kmdichildarea.cpp kmdichildfrm.cpp kmdichildfrmcaption.cpp \
				kmdichildview.cpp kmdimainfrm.cpp kmditaskbar.cpp  \
				kmultitabbar.cpp \
				kmditoolviewaccessor.cpp kmdidocumentviewtabwidget.cpp kmdifocuslist.cpp \
				kmdiguiclient.cpp \
				ktabwidget.cpp ktabbar.cpp kdockwidget.cpp kdockwidget_private.cpp kmdidockcontainer.cpp

METASOURCES = AUTO

#AM_CXXFLAGS = -DNO_INCLUDE_MOCFILES
