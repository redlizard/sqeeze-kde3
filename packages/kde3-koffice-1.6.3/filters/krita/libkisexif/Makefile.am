INCLUDES = \
        -I$(srcdir) \
         $(KOFFICE_INCLUDES) \
        -I$(top_srcdir)/krita \
        -I$(top_srcdir)/krita/core \
        -I$(top_srcdir)/krita/sdk \
        -I$(top_srcdir)/krita/core/tiles \
        -I$(top_srcdir)/krita/kritacolor \
        -I$(top_srcdir)/krita/ui \
        $(KOFFICE_INCLUDES) -I$(interfacedir) \
        $(KOPAINTER_INCLUDES) \
        $(all_includes) 
METASOURCES = AUTO
libkisexif_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libkisexif.la


libkisexif_la_SOURCES = kis_exif_io.cpp
