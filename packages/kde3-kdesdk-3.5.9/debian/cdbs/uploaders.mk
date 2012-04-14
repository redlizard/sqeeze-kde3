
MAINTAINER=Debian Qt/KDE Maintainers <debian-qt-kde@lists.debian.org>
UPLOADERS=$(shell grep -e +++ -e "^ -- " debian/changelog  | grep -v "debian-qt-kde@lists.debian.org" | head -13 | /bin/sed 's/^\s*//;s/\s*$$//;s/^+++\? Changes by //;s/^+++\? //;s/-- //;s/:$$//;s/ <.*//' | sort -u | while read line ; do grep "$$line" debian/cdbs/team-members ; done | tr "\n" ", " | sed 's/,/, /g;s/, $$//')


debian/control.tmp:
	@if [ ! -e debian/control.in ] ; then  \
		echo "this package is not yet prepared for using automatic update of uploaders"; \
		echo "Please do so."; \
		exit 1; \
	fi
	@sed  's/@@@UPLOADERS@@@/$(UPLOADERS)/;s#@@@MAINTAINER@@@#$(MAINTAINER)#' debian/control.in  > debian/control.tmp

check-uploaders: debian/control.tmp
	@if ! diff -q debian/control debian/control.tmp ; then \
		echo "WARNING:: Control file differs from manually generated one" ; \
		echo "WARNING:: Please update it manually and check it afterwards" ; \
		echo "WARNING:: Uploaders are updated by debian/rules update-uploaders" ;\
		echo "WARNING:: If this is a binNMU, NMU or security upload, just ignore" ;\
	fi
	

clean:: 
	rm -f debian/control.tmp

update-uploaders: debian/control.tmp
	@mv -f debian/control.tmp debian/control

makebuilddir:: check-uploaders
