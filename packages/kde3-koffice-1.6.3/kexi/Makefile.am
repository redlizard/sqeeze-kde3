include $(top_srcdir)/kexi/Makefile.global

lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kexi.la
bin_PROGRAMS =
kexi_la_SOURCES = main.cpp
kexi_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO)
kexi_la_LIBADD = ./core/libkexicore.la ./main/libkeximain.la

SUBDIRS= 3rdparty kexiutils kexidb core widget formeditor \
data pics main migration plugins . tests examples tools

#(will be reenabled after main/startup move)

xdg_apps_DATA = kexi.desktop

install-data-local:
#	$(mkinstalldirs) $(kde_servicetypesdir)/
#	$(INSTALL_DATA) $(srcdir)/kexiinterface.desktop $(kde_servicetypesdir)/kexiinterface.desktop
#	$(mkinstalldirs) $(kde_servicedir)
#	$(INSTALL_DATA) $(srcdir)/interfaces/mysqlinterface.desktop $(kde_servicesdir)/mysqlinterface.desktop

# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES = $(LIB_KEXI_KMDI_INCLUDES) $(all_includes)

METASOURCES = AUTO

# Note: koffice/kexi/formeditor should be kept out of kexi.pot
# as it might get moved out of Kexi.
#
# If a second directory must be "pruned", the syntax becomes:
# find . \( -name formeditor -o -name test \) -prune , -name \*.ui
# (The comma is needed or the pruned directory name would be printed to stdout)
#
# Temporary omitted files: kexifinddialog*
messages: rc.cpp
	rm -f tips.cpp
#	(cd data && $(PREPARETIPS) > ../tips.cpp)
	EXCLUDE="-path ./formeditor -o -path ./doc \
	-o -path ./tests -o -path ./plugins/macros/tests \
	-o -path ./scriptingplugins -o -path ./plugins/scripting -o -path ./3rdparty"; \
	LIST="data/*.rc `find . \( \( $$EXCLUDE \) -prune -o -name \*.ui \) -type f | grep -v -e '/\.'`"; \
	if test -n "$$LIST"; then \
	 $(EXTRACTRC) $$LIST >> rc.cpp; \
	fi; \
	LIST=`find . \( \( $$EXCLUDE \) -prune -o -name \*.h -o -name \*.cpp \) -type f | grep -v -e '/\.' -e kexidswelcome.cpp`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kexi.pot; \
	fi
	rm -f tips.cpp


DOXYGEN_EXCLUDE = 3rdparty kexidb/parser/sqlparser.h kexidb/drivers/odbc

include $(top_srcdir)/admin/Doxyfile.am


