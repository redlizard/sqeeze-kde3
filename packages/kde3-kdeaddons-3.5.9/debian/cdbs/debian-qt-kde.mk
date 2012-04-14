ifndef _cdbs_bootstrap
_cdbs_scripts_path ?= /usr/lib/cdbs
_cdbs_rules_path ?= /usr/share/cdbs/1/rules
_cdbs_class_path ?= /usr/share/cdbs/1/class
endif

ifndef _cdbs_class_debian-qt-kde
_cdbs_class_debian-qt-kde := 1

# Note: This _must_ be included before autotools.mk, or it won't work.
common-configure-arch common-configure-indep:: debian/stamp-cvs-make
debian/stamp-cvs-make:
	if test -n "$(DEB_KDE_CVS_MAKE)" -a ! -f configure; then \
		$(MAKE) -C $(DEB_SRCDIR) -f admin/Makefile.common dist; \
	fi
	touch debian/stamp-cvs-make

include /usr/share/cdbs/1/class/kde.mk$(_cdbs_makefile_suffix)
include debian/cdbs/uploaders.mk

DEB_PATCHDIRS := debian/patches/common debian/patches

DEB_KDE_ENABLE_FINAL := yes
DEB_INSTALL_DOCS_ALL :=

DEB_DH_MAKESHLIBS_ARGS_ALL := -V
DEB_SHLIBDEPS_INCLUDE = $(foreach p,$(PACKAGES_WITH_LIBS),debian/$(p)/usr/lib)

ifeq (,$(findstring noopt,$(DEB_BUILD_OPTIONS)))
    cdbs_treat_me_gently_arches := arm m68k alpha ppc64 armel armeb
    ifeq (,$(filter $(DEB_HOST_ARCH_CPU),$(cdbs_treat_me_gently_arches)))
        cdbs_kde_enable_final = $(if $(DEB_KDE_ENABLE_FINAL),--enable-final,)
    else
        cdbs_kde_enable_final =
    endif
endif

common-build-arch:: debian/stamp-man-pages
debian/stamp-man-pages:
	if ! test -d debian/man/out; then mkdir -p debian/man/out; fi
	for f in $$(find debian/man -name '*.sgml'); do \
		docbook-to-man $$f > debian/man/out/`basename $$f .sgml`.1; \
	done
	for f in $$(find debian/man -name '*.man'); do \
		soelim -I debian/man $$f \
		> debian/man/out/`basename $$f .man`.`head -n1 $$f | awk '{print $$NF}'`; \
	done
	touch debian/stamp-man-pages

common-binary-indep::
	( set -e; \
	tmpf=`mktemp debian/versions.XXXXXX`; \
	perl debian/cdbs/versions.pl >$$tmpf; \
	for p in $(DEB_INDEP_PACKAGES); do \
	    cat $$tmpf >>debian/$$p.substvars; \
	done; \
	rm -f $$tmpf )

common-binary-arch::
	( set -e; \
	tmpf=`mktemp debian/versions.XXXXXX`; \
	perl debian/cdbs/versions.pl >$$tmpf; \
	for p in $(DEB_ARCH_PACKAGES); do \
	    cat $$tmpf >>debian/$$p.substvars; \
	done; \
	rm -f $$tmpf )

clean::
	rm -rf debian/man/out
	-rmdir debian/man
	rm -f debian/stamp-man-pages
	rm -rf debian/shlibs-check

$(patsubst %,binary-install/%,$(DEB_PACKAGES)) :: binary-install/%:
	if test -x /usr/bin/dh_desktop; then dh_desktop -p$(cdbs_curpkg) $(DEB_DH_DESKTOP_ARGS); fi
	if test -e debian/$(cdbs_curpkg).lintian; then \
		install -p -D -m644 debian/$(cdbs_curpkg).lintian \
			debian/$(cdbs_curpkg)/usr/share/lintian/overrides/$(cdbs_curpkg); \
	fi
	if test -e debian/$(cdbs_curpkg).presubj; then \
		install -p -D -m644 debian/$(cdbs_curpkg).presubj \
			debian/$(cdbs_curpkg)/usr/share/bug/$(cdbs_curpkg)/presubj; \
	fi

binary-install/$(DEB_SOURCE_PACKAGE)-doc-html::
	set -e; \
	for doc in `cd $(DEB_DESTDIR)/usr/share/doc/kde/HTML/en; find . -name index.docbook`; do \
		pkg=$${doc%/index.docbook}; pkg=$${pkg#./}; \
		echo Building $$pkg HTML docs...; \
		mkdir -p $(CURDIR)/debian/$(DEB_SOURCE_PACKAGE)-doc-html/usr/share/doc/kde/HTML/en/$$pkg; \
		cd $(CURDIR)/debian/$(DEB_SOURCE_PACKAGE)-doc-html/usr/share/doc/kde/HTML/en/$$pkg; \
		meinproc $(DEB_DESTDIR)/usr/share/doc/kde/HTML/en/$$pkg/index.docbook; \
	done
	for pkg in $(DOC_HTML_PRUNE) ; do \
	  rm -rf debian/$(DEB_SOURCE_PACKAGE)-doc-html/usr/share/doc/kde/HTML/en/$$pkg; \
	done

clean::
	if test -n "$(DEB_KDE_CVS_MAKE)" && test -d $(DEB_SRCDIR); then \
		cd $(DEB_SRCDIR); \
		find . -name Makefile.in -print | \
			xargs --no-run-if-empty rm -f; \
		rm -f Makefile.am acinclude.m4 aclocal.m4 config.h.in \
			configure configure.files configure.in stamp-h.in \
			subdirs; \
	fi
	rm -f debian/stamp-cvs-make

endif
