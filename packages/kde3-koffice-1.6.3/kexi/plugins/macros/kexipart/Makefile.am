include $(top_srcdir)/kexi/Makefile.global

kde_module_LTLIBRARIES = kexihandler_macro.la

kexihandler_macro_la_SOURCES = \
    keximacropart.cpp keximacroview.cpp keximacroproperty.cpp keximacrodesignview.cpp keximacrotextview.cpp keximacroerrorbase.ui keximacroerror.cpp

kexihandler_macro_la_LDFLAGS = \
    $(KDE_PLUGIN) -module -no-undefined -Wnounresolved $(all_libraries) $(VER_INFO)

kexihandler_macro_la_LIBADD = \
    ../kexiactions/libkeximacroactions.la \
    $(top_builddir)/kexi/core/libkexicore.la \
    $(top_builddir)/kexi/widget/libkexiextendedwidgets.la \
    $(top_builddir)/lib/koproperty/libkoproperty.la

INCLUDES = \
    -I$(top_srcdir)/lib  \
    -I$(top_srcdir)/lib/kofficecore/  \
    -I$(top_srcdir)/kexi/core \
    -I$(top_srcdir)/kexi \
    -I$(top_srcdir)/kexi/widget \
    $(all_includes)

servicesdir=$(kde_servicesdir)/kexi
services_DATA=keximacrohandler.desktop

SUBDIRS = .
METASOURCES = AUTO

noinst_HEADERS = \
    keximacropart.h keximacroview.h keximacrodesignview.h keximacrotextview.h keximacroerror.h
