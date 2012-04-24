# set the include path for X, qt and KDE
INCLUDES = $(all_includes) $(LIBXSLT_CFLAGS) $(LIBXML_CFLAGS) $(LIBBIBUTILS_CFLAGS)

# these are the headers for your project
noinst_HEADERS = 						documentlistviewitem.h 						fileexportertoolchain.h \
	comment.h commentwidget.h documentlistview.h documentlistviewitem.h \
	documentsourceview.h documentwidget.h element.h encoder.h encoderlatex.h encoderxml.h \
	entry.h entryfield.h entrywidget.h entrywidgetauthor.h entrywidgetexternal.h \
	entrywidgetkeyword.h entrywidgetmisc.h entrywidgetother.h entrywidgetpublication.h \
	entrywidgetsource.h entrywidgettab.h entrywidgettitle.h entrywidgetuserdefined.h \
	entrywidgetwarningsitem.h fieldlineedit.h fieldlistview.h file.h fileexporter.h \
	fileexporterbibtex.h fileexporterbibutils.h fileexporterexternal.h fileexporterpdf.h \
	fileexporterps.h fileexporterris.h		fileimporterbibtex.h fileexporterrtf.h \
	fileexporterxml.h fileexporterxslt.h fileimporter.h fileimporterbibutils.h \
	fileimporterexternal.h fileimporterris.h idsuggestions.h idsuggestionswidget.h kbibtex_part.h \
	kbibtexshell.h macro.h macrowidget.h mergeentries.h preamble.h preamblewidget.h \
	searchbar.h settings.h settingsdlg.h settingsediting.h settingsfileio.h \
	settingsidsuggestions.h settingskeyword.h settingssearchurl.h settingsuserdefinedinput.h \
	sidebar.h value.h valuewidget.h webquery.h webqueryamatex.h webqueryarxiv.h \
	webquerybibsonomy.h webquerycitebase.h webquerydblp.h webquerygooglescholar.h \
	webquerypubmed.h webqueryspireshep.h webqueryzmath.h xsltransform.h
# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) *.rc >>rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kbibtex.pot

KDE_ICON = AUTO

# this Makefile creates both a KPart application and a KPart
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kbibtex

# the application source, library search path, and link libraries
kbibtex_SOURCES = main.cpp kbibtexshell.cpp
kbibtex_LDFLAGS = $(all_libraries)
kbibtex_LDADD   = $(LIB_KPARTS)

# this is where the desktop file will go
shelldesktopdir   = $(xdg_appsdir)
shelldesktop_DATA = kbibtex.desktop

#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libkbibtexpart.la

# the Part's source, library search path, and link libraries
libkbibtexpart_la_SOURCES = comment.cpp commentwidget.cpp documentlistview.cpp \
	documentlistviewitem.cpp documentsourceview.cpp documentwidget.cpp element.cpp encoder.cpp \
	encoderlatex.cpp encoderxml.cpp entry.cpp entryfield.cpp entrywidget.cpp \
	entrywidgetauthor.cpp entrywidgetexternal.cpp entrywidgetkeyword.cpp entrywidgetmisc.cpp \
	entrywidgetother.cpp entrywidgetpublication.cpp entrywidgetsource.cpp entrywidgettab.cpp \
	entrywidgettitle.cpp entrywidgetuserdefined.cpp entrywidgetwarningsitem.cpp \
	fieldlineedit.cpp fieldlistview.cpp file.cpp fileexporter.cpp \
	fileexporterbibtex.cpp	fileexporterris.cpp fileexporterbibutils.cpp fileexporterexternal.cpp fileexporterpdf.cpp \
	fileexporterps.cpp fileexporterrtf.cpp fileexportertoolchain.cpp fileexporterxml.cpp \
	fileexporterxslt.cpp fileimporter.cpp fileimporterbibtex.cpp fileimporterbibutils.cpp \
	fileimporterexternal.cpp fileimporterris.cpp idsuggestions.cpp idsuggestionswidget.cpp \
	kbibtex_part.cpp macro.cpp macrowidget.cpp mergeentries.cpp preamble.cpp \
	preamblewidget.cpp searchbar.cpp settings.cpp settingsdlg.cpp settingsediting.cpp \
	settingsfileio.cpp settingsidsuggestions.cpp settingskeyword.cpp settingssearchurl.cpp \
	settingsuserdefinedinput.cpp sidebar.cpp value.cpp valuewidget.cpp webquery.cpp webqueryamatex.cpp \
	webqueryarxiv.cpp webquerybibsonomy.cpp webquerycitebase.cpp webquerydblp.cpp \
	webquerygooglescholar.cpp webquerypubmed.cpp webqueryspireshep.cpp webqueryzmath.cpp \
	xsltransform.cpp
libkbibtexpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkbibtexpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KUTILS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(LIBBIBUTILS_LIBS) -lkatepartinterfaces

# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kbibtex_part.desktop

# this is where the XML-GUI resource fileis go
partrcdir = ${kde_datadir}/kbibtexpart
partrc_DATA = kbibtex_part.rc
shellrcdir = ${kde_datadir}/kbibtex
shellrc_DATA = kbibtex_shell.rc

