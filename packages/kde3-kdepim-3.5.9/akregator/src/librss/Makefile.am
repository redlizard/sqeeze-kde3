INCLUDES = \
    -I$(top_srcdir)/src \
    $(all_includes)

noinst_LTLIBRARIES = \
    librsslocal.la

noinst_HEADERS = article.h document.h global.h image.h textinput.h \
  loader.h librss.h enclosure.h

librsslocal_la_SOURCES = article.cpp document.cpp image.cpp textinput.cpp \
  tools_p.cpp loader.cpp enclosure.cpp category.cpp feeddetector.cpp

librsslocal_la_METASOURCES = AUTO

check_PROGRAMS = testlibrss
testlibrss_SOURCES = testlibrss.cpp
testlibrss_LDFLAGS = $(all_libraries)
testlibrss_LDADD =  librsslocal.la $(LIB_KIO)

DOXYGEN_REFERENCES = kdeui
include $(top_srcdir)/admin/Doxyfile.am
