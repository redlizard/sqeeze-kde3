noinst_LTLIBRARIES = libkxsldbgcommon.la
libkxsldbgcommon_la_SOURCES = xsldbgwalkspeedimpl.cpp xsldbgtemplatesimpl.cpp xsldbgtemplatelistitem.cpp xsldbgsourcesimpl.cpp xsldbgoutputview.cpp xsldbglocalvariablesimpl.cpp xsldbglocallistitem.cpp xsldbglistitem.cpp xsldbginspector.cpp xsldbgglobalvariablesimpl.cpp xsldbggloballistitem.cpp xsldbgentitiesimpl.cpp xsldbgdoc.cpp xsldbgdialogbase.cpp xsldbgdebugger.cpp xsldbgconfigimpl.cpp xsldbgcallstackimpl.cpp xsldbgbreakpointsimpl.cpp xsldbgbreakpointlistitem.cpp qxsldbgdoc.cpp kxsldbg_part.cpp kxsldbg_partif.skel xsldbgwalkspeed.ui xsldbgtemplates.ui xsldbgsources.ui xsldbgmsgdialog.ui xsldbglocalvariables.ui xsldbgglobalvariables.ui xsldbgentities.ui xsldbgconfig.ui xsldbgcallstack.ui xsldbgbreakpoints.ui
libkxsldbgcommon_la_METASOURCES = AUTO
libkxsldbgcommon_la_LDFLAGS = $(all_libraries)
libkxsldbgcommon_la_LIBADD  =  libxsldbg/libxsldbg.la libqtnotfier/libqtnotfier.la 

noinst_HEADERS = kxsldbg_part.h qxsldbgdoc.h xsldbgbreakpointlistitem.h xsldbgbreakpointsimpl.h xsldbgcallstackimpl.h xsldbgconfigimpl.h xsldbgdebugger.h xsldbgdialogbase.h xsldbgdoc.h xsldbgdocmap.h xsldbgentitiesimpl.h xsldbggloballistitem.h xsldbgglobalvariablesimpl.h xsldbginspector.h xsldbglistitem.h xsldbglocallistitem.h xsldbglocalvariablesimpl.h xsldbgmsgdialogimpl.h xsldbgoutputview.h xsldbgsourcesimpl.h xsldbgtemplatelistitem.h xsldbgtemplatesimpl.h xsldbgwalkspeedimpl.h

kde_module_LTLIBRARIES = libkxsldbgpart.la

libkxsldbgpart_la_SOURCES = dummy.cpp

SUBDIRS = libqtnotfier libxsldbg

INCLUDES = $(XSLDBG_CFLAGS) -I.. $(all_includes)


libkxsldbgpart_la_LDFLAGS = -module $(KDE_PLUGIN) -lkatepartinterfaces $(all_libraries)  $(XSLDBG_EXTRA_LIBS)
libkxsldbgpart_la_LIBADD  =  libkxsldbgcommon.la $(LIB_KPARTS)


# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kxsldbg_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/kxsldbgpart
partrc_DATA = kxsldbg_part.rc

KDE_ICON = AUTO

