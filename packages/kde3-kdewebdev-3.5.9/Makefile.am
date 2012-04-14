COMPILE_FIRST = lib
MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs


install-data-local:
	@echo ""
	@echo ""
	@echo "*************** Important *************************"
	@echo ""
	@echo " Add "$(bindir)" to your PATH and"
	@echo " add "$(prefix)" to your KDEDIRS!"
	@echo ""
	@echo " Please report bugs with our web form at"
	@echo " http://bugs.kde.org"
	@echo "    Current maintainers are"
	@echo " Eric Laffoon       <sequitur@kde.org>"
	@echo " Andras Mantia      <amantia@kde.org>"
	@echo ""
	@echo " The KDE Web Dev developers hope you enjoy Quanta+,"
	@echo " Kommander, KFileReplace, KXSL dbg, KImageMapEditor"
	@echo " and KLinkStatus!"
	@echo "****************************************************"
	@echo ""

include admin/deps.am

SUBDIRS=$(TOPSUBDIRS)
