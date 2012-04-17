INCLUDES = $(all_includes)
noinst_HEADERS = qtcurveconfig.h
kde_module_LTLIBRARIES = kstyle_qtcurve_config.la
kstyle_qtcurve_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module -lkio
kstyle_qtcurve_config_la_LIBADD  = $(LIB_KDEUI)
kstyle_qtcurve_config_la_SOURCES = qtcurveconfigbase.ui qtcurveconfig.cpp
kstyle_qtcurve_config_la_METASOURCES = AUTO

stylercdir = $(kde_datadir)/QtCurve
stylerc_DATA = Clear.qtcurve Dull_Glass.qtcurve Shiny_Glass.qtcurve Inverted.qtcurve Plain.qtcurve Glass.qtcurve Human.qtcurve Plastic.qtcurve Murrine.qtcurve Curve.qtcurve Clean.qtcurve

EXTRA_DIST = $(stylerc_DATA)

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_qtcurve_config.pot
