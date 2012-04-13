lib_LTLIBRARIES = libqtmcop.la

INCLUDES = -I$(top_srcdir)/mcop -I$(top_builddir)/mcop $(all_includes) $(QT_INCLUDES)
KDE_OPTIONS = qtonly

libqtmcop_la_SOURCES = qiomanager.cc
libqtmcop_la_LIBADD = $(top_builddir)/mcop/libmcop.la $(LIB_QT)
libqtmcop_la_LDFLAGS = -no-undefined -version-info 1:0 $(KDE_RPATH) \
	$(KDE_MT_LDFLAGS) $(QT_LDFLAGS) $(USER_LDFLAGS) $(X_LDFLAGS) $(LDFLAGS_AS_NEEDED)

METASOURCES = qiomanager_p.moc

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = qiomanager.h
