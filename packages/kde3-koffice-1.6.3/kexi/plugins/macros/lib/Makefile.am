noinst_LTLIBRARIES = libkomacro.la

libkomacro_la_SOURCES = \
    exception.cpp \
    variable.cpp \
    metaparameter.cpp \
    metamethod.cpp \
    metaobject.cpp \
    action.cpp \
    macroitem.cpp \
    macro.cpp \
    context.cpp \
    xmlhandler.cpp \
    manager.cpp

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

libkomacro_la_LDFLAGS = $(all_libraries) -Wnounresolved
libkomacro_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)

libkomacro_la_METASOURCES = AUTO
SUBDIRS = .
INCLUDES = $(all_includes)
