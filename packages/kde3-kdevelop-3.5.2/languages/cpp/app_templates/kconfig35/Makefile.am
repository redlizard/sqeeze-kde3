dataFiles = src-Makefile.am app.cpp app.h main.cpp appui.rc app.kdevelop \
	subdirs app.desktop kxt35.png appview.h appview.cpp appview_base.ui \
	prefs-base.ui prefs.cpp prefs.h app.kcfg settings.kcfgc README \
	src-configure.in.in
templateName= kconfig35

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP_COMMAND) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz
