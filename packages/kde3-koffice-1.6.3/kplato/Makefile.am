SUBDIRS = templates pics reports toolbar tests

INCLUDES = $(KOFFICE_INCLUDES) $(KOTEXT_INCLUDES) \
           -I$(top_srcdir)/kdgantt -I$(top_srcdir)/kugar/lib \
           $(all_includes)

libkplatopart_la_SOURCES = \
    kptsummarytaskgeneralpanelbase.ui \
    kptsummarytaskdialog.cc \
    kptsummarytaskgeneralpanel.cc \
    kptschedule.cc \
    kptappointment.cc \
    kptcalendarlistpanel.ui \
    kptresourceappointmentsview.cc \
    kpttaskappointmentsview.cc \
    kptdoublelistviewbase.cc \
    kptaccountsviewconfigdialog.cc \
    kptaccountsviewconfigurepanelbase.ui \
    kptaccountsview.cc \
    kpttaskcostpanelbase.ui \
    kpttaskcostpanel.cc \
    kptmilestoneprogresspanel.cc \
    kptmilestoneprogresspanelbase.ui \
    kptmilestoneprogressdialog.cc \
    kptaccount.cc kptaccountsdialog.cc kptaccountspanel.cc kptaccountspanelbase.ui \
    kptnode.cc kptproject.cc \
    kptrelation.cc kptresource.cc kpttask.cc \
    kptduration.cc kptfactory.cc kptpart.cc kptview.cc \
    kptdurationwidget.ui relationpanel.ui\
    kptprojectdialog.cc kpttaskdialog.cc  \
    kptmainprojectdialog.cc kptmainprojectpanel.cc kptmainprojectpanelbase.ui \
    kptganttview.cc kptresourcespanel.cc \
    kptpertview.cc kptpertcanvas.cc kptrelationdialog.cc \
    kptcanvasitem.cc kptprojectdialogbase.ui \
    resourcespanelbase.ui \
    kptresourceview.cc \
    kpttaskresourcespanelbase.ui kptrequestresourcespanel.cc \
    resourcedialogbase.ui kptresourcedialog.cc \
    kpttaskgeneralpanelbase.ui \
    kpttasknotespanelbase.ui \
    kptdatetime.cc \
    kptcalendar.cc kptcalendaredit.cc kptcalendarlistdialog.cc \
    kptcalendarpanel.cc kptdatetable.cc \
    kptcalendarlistdialogbase.cc kptcalendareditbase.cc \
    kptcommand.cc KPtViewIface.cc KPtViewIface.skel  \
    standardworktimedialogbase.ui kptstandardworktimedialog.cc \
    kptintervaleditbase.ui kptintervaledit.cc \
    kpttaskgeneralpanel.cc kpttaskdefaultpanel.cc \
    kptconfigtaskpanelbase.ui \
    kptconfigdialog.cc \
    kpttaskprogresspanel.cc kpttaskprogresspanelbase.ui \
    kpttaskprogressdialog.cc \
    kptconfig.cc kptcontext.cc \
    kptresourcesdialog.cc \
    kptwbsdefinitiondialog.cc kptwbsdefinitionpanel.cc kptwbsdefinitionpanelbase.ui \
    kptwbsdefinition.cc
##    kptconfigbehaviorpanel.cc kptconfigbehaviorpanelbase.ui \
##    kptreportview.cc 

# (this is not used)
noinst_HEADERS = \
    kptxmlloaderobject.h \
    kptsummarytaskdialog.h \
    kptsummarytaskgeneralpanel.h \
    kptschedule.h \
    kptappointment.h \
    kptresourceappointmentsview.h \
    kpttaskappointmentsview.h \
    kptdoublelistviewbase.h \
    kptaccountsviewconfigdialog.h \
    kptaccountsview.h \
    kpttaskcostpanel.h \
    kpttaskprogresspanel.h \
    kpttaskprogressdialog.h \
    kptaccount.h kptaccountsdialog.h kptaccountspanel.h \
    kptaboutdata.h kptduration.h kptfactory.h \
    kptnode.h kptpart.h kptproject.h kptrelation.h \
    kptresource.h kpttask.h kptview.h \
    kptprojectdialog.h kpttaskdialog.h  \
    kptmainprojectdialog.h \
    kptganttview.h kptcanvasitem.h \
    kptpertview.h kptpertcanvas.h kptrelationdialog.h \
    kptresourcespanel.h kptresourceview.h \
    kptrequestresourcespanel.h kptresourcedialog.h\
    kptdatetime.h kptmap.h \
    kptcalendar.h kptcalendaredit.h kptcalendarlistdialog.h \
    kptcalendarpanel.h kptdatetable.h \
    kptcalendarlistdialogbase.h kptcalendareditbase.h \
    kptcommand.h \
    kptstandardworktimedialog.h kptintervaledit.h intervalitem.h \
    kpttaskgeneralpanel.h kpttaskdefaultpanel.h \
    kptconfigdialog.h \
    kpttaskprogresspanel.h \
    kptconfig.h kptcontext.h \
    kptresourcesdialog.h \
    kptwbsdefinitiondialog.h kptwbsdefinitionpanel.h \
    kptwbsdefinition.h \
    kpteffortcostmap.h
##    kptconfigbehaviorpanel.h \
##    kptreportview.h

## The part
lib_LTLIBRARIES = 
kde_module_LTLIBRARIES = libkplatopart.la
libkplatopart_la_LDFLAGS = $(KDE_LDFLAGS) $(KDE_PLUGIN)
libkplatopart_la_LIBADD = $(LIB_KABC) $(LIB_KOFFICEUI) \
                          $(top_builddir)/kdgantt/libkdgantt.la
##                          $(top_builddir)/kugar/lib/libkugarlib.la

libkplatopart_la_METASOURCES = AUTO

## The kdeinit loadable module and executable
kdeinit_LTLIBRARIES = kplato.la
bin_PROGRAMS =
kplato_la_SOURCES = main.cc
kplato_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kplato_la_LIBADD = $(LIB_KOFFICECORE)

xdg_apps_DATA = kplato.desktop

rcdir = $(kde_datadir)/kplato
rc_DATA = kplato.rc kplato_readonly.rc

## NOTE: extractattr needs <Label & Text on same line!
messages: rc.cpp
	$(EXTRACTATTR) --attr=Label,Text reports/*.ktf >>rc.cpp
	$(XGETTEXT) rc.cpp *.cc kptaboutdata.h -o $(podir)/kplato.pot

kde_services_DATA = kplatopart.desktop

include $(top_srcdir)/admin/Doxyfile.am
