noinst_LTLIBRARIES = libxsldbg.la

libxsldbg_la_METASOURCES = AUTO

libxsldbg_la_SOURCES = allmoc.cpp xsldbgthread.cpp xsldbg.cpp xsldbgmsg.cpp variable_cmds.cpp utils.cpp trace_cmds.cpp template_cmds.cpp search_cmds.cpp search.cpp param_cmds.cpp os_cmds.cpp options_unix.cpp options.cpp option_cmds.cpp nodeview_cmds.cpp help_unix.cpp files_unix.cpp files.cpp file_cmds.cpp debugXSL.cpp debug.cpp callstack.cpp breakpoint_cmds.cpp breakpoint.cpp arraylist.cpp

libxsldbg_la_LIBADD = $(LIBXSLT_LIBS)
libxsldbg_la_LDFLAGS = $(all_libraries)

AM_CPPFLAGS = -DDOCS_PATH=\"$(DOCS_PATH)\" -I.. $(LIBXSLT_CFLAGS) $(all_includes)

KDE_OPTIONS= nofinal
