####### General stuff

INCLUDES = $(KOFFICE_INCLUDES) $(all_includes)

####### Files
kde_module_LTLIBRARIES = libkwordkword1dot3import.la


libkwordkword1dot3import_la_SOURCES = \
kword13document.cpp kword13parser.cpp kword13frameset.cpp kword13layout.cpp \
kword13paragraph.cpp kword13utils.cpp kword13import.cpp \
kword13oasisgenerator.cpp kword13format.cpp kword13formatone.cpp \
kword13formatother.cpp kword13picture.cpp kword13postparsing.cpp

libkwordkword1dot3import_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkwordkword1dot3import_la_LIBADD  = $(KOFFICE_LIBS)

METASOURCES = AUTO

#service_DATA = kword_kword1dot3_import.desktop
#servicedir = $(kde_servicesdir)
install-data-local: uninstall.desktop
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_servicesdir)/kword_kword1dot3_import.desktop

