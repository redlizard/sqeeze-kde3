## kdebase/Makefile.am
## (C) 1997 Stephan Kulow

AUTOMAKE_OPTIONS = foreign 1.4
SUBDIRS = $(TOPSUBDIRS)

DISTCLEANFILES = inst-apps

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 SUBDIRS

#
# Not sure about this really... admin/am_edit (if not modified) seems to add all files to the distribution. But when modified the
# following don't get added. So as a quick solution, I've just added the files below...
#
EXTRA_DIST = admin/acinclude.m4.in admin/conf.change.pl admin/configure.in.bot.end admin/debianrules admin/Doxyfile.global \
             admin/Makefile.common \
             admin/am_edit admin/config.guess admin/configure.in.min admin/depcomp admin/install-sh admin/missing \
             admin/ChangeLog admin/config.pl admin/cvs-clean.pl admin/detect-autoconf.sh admin/libtool.m4.in admin/mkinstalldirs \
             admin/compile admin/config.sub admin/cvs.sh admin/Doxyfile.am admin/ltmain.sh admin/ylwrap \
             configure.in.in \
             kde/config/klearlookconfigbase.ui
