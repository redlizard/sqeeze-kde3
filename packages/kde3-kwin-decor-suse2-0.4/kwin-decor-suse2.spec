#
# spec file for package kwin-decor-suse (Version 0.2)
#
# Copyright (c) 2005 Gerd Fleischer
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.

%define DIST %(if [ -r /etc/SuSE-release ]; then echo "SuSE"; else echo "Unknown"; fi)

Name:         kwin-decor-suse2
URL:          http://www.gerdfleischer.de/kwin-decor-suse2/
License:      GPL
Group:        System/GUI/KDE
Summary:      The KDE window decoration from SUSE 9.3 - 10.1
Version:      0.4
Release:      1
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Source:       %name-%version.tar.bz2

%description
The KDE window decoration from SUSE 9.3 - 10.1 extracted from the kdebase3-SUSE package.

Authors:
--------
    Adrian Schroeter <adrian@suse.de>
    Sandro Giessl <ceebx@users.sourceforge.net>
    Rik Hemsley (rikkus) <rik@kde.org>
    Gerd Fleischer <gerdfleischer@web.de>

%prep

%setup -q

%if %{DIST} == SuSE
cp extra/kroete.png src/titlebar_decor.png
. /etc/opt/kde3/common_options
update_admin --no-unsermake
%endif

%build
%if %{DIST} == SuSE
. /etc/opt/kde3/common_options
./configure $configkde
%else
./configure --enable-final
%endif

make

%install
make DESTDIR=$RPM_BUILD_ROOT install-strip
install -d -m 755 $RPM_BUILD_ROOT%{_docdir}/kwin-decor-suse2
install -m 644 ChangeLog README COPYING INSTALL AUTHORS NEWS $RPM_BUILD_ROOT%{_docdir}/kwin-decor-suse2/

%files
%defattr(-,root,root)
%if %{DIST} == SuSE
/opt/kde3/%{_lib}
/opt/kde3/share/
%else
/usr/%{_lib}
/usr/share/
%endif
%{_docdir}/kwin-decor-suse2/

%changelog -n kwin-decor-suse2
* Sun Nov 20 2005 - gerdfleischer@web.de
- use _lib
* Sun May 01 2005 - gerdfleischer@web.de
- initial spec
