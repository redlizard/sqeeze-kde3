if include_x11

INCLUDES = -I$(top_srcdir)/mcop -I$(top_builddir)/mcop $(all_includes)

lib_LTLIBRARIES = libx11globalcomm.la

libx11globalcomm_la_SOURCES = x11globalcomm.cc x11globalcomm_impl.cc
libx11globalcomm_la_LIBADD = $(top_builddir)/mcop/libmcop.la $(LIB_X11)
libx11globalcomm_la_LDFLAGS = -no-undefined -module -version-info 1:0 $(X_LDFLAGS) $(all_libraries)

DISTCLEANFILES = x11globalcomm.cc x11globalcomm.h \
                 x11globalcomm.mcoptype x11globalcomm.mcopclass

x11globalcomm.cc x11globalcomm.h: $(srcdir)/x11globalcomm.idl $(MCOPIDL)
	$(MCOPIDL) -t $(INCLUDES) $(srcdir)/x11globalcomm.idl

x11globalcomm_impl.lo: x11globalcomm.h
x11globalcomm.mcoptype: x11globalcomm.h
x11globalcomm.mcopclass: x11globalcomm.h

mcopclassdir = $(libdir)/mcop/Arts
mcopclass_DATA = X11GlobalComm.mcopclass

######## install idl typeinfo files

mcoptypedir = $(libdir)/mcop
mcoptype_DATA = x11globalcomm.mcoptype x11globalcomm.mcopclass

endif
