
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kpresenter/templates/legal
	for file in OneColumnLandscape.desktop TitleLandscape.desktop \
	   TwoColumnLandscape.desktop emptyLandscape.desktop OneColumnPortrait.desktop \
	   TitlePortrait.desktop TwoColumnPortrait.desktop emptyPortrait.desktop; do \
	  $(INSTALL_DATA) $(srcdir)/../common_desktop/$$file $(DESTDIR)$(kde_datadir)/kpresenter/templates/legal/$$file ;\
	done
	$(INSTALL_DATA) $(srcdir)/.directory $(DESTDIR)$(kde_datadir)/kpresenter/templates/legal

templatesrc_DATA = \
 OneColumnLandscape.kpt \
 OneColumnPortrait.kpt \
 TitleLandscape.kpt \
 TitlePortrait.kpt \
 TwoColumnLandscape.kpt \
 TwoColumnPortrait.kpt \
 emptyLandscape.kpt \
 emptyPortrait.kpt
templatesrcdir = $(kde_datadir)/kpresenter/templates/legal/.source
