AM_CPPFLAGS = -I$(srcdir)/.. $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libknewstuff.la

libknewstuff_la_LDFLAGS = $(all_libraries)
libknewstuff_la_LIBADD  = $(LIB_KIO)
libknewstuff_la_SOURCES = engine.cpp entry.cpp downloaddialog.cpp \
  uploaddialog.cpp providerdialog.cpp provider.cpp knewstuff.cpp \
  knewstuffgeneric.cpp knewstuffsecure.cpp security.cpp

KDE_ICON=AUTO
