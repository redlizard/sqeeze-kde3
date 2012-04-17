Summary: lipstik theme for KDE
Name: lipstik
Version: 2.2.3
Release: 1
Source: %{name}-%{version}.tar.bz2
URL: http://www.kde-look.org/content/show.php?content=18223
Vendor: poplixos (at) gmail.com
License: GPL
Group: System/GUI/KDE
Packager: William Flynn <wdf@sifr.net>
BuildRoot: %{_tmppath}/%{name}-%{version}-root
BuildRequires: kdelibs >= 3.0
Requires: kdelibs >= 3.0

%description
Lipstik is a fast and clean style with many options to tune your desktop look.

%prep
%setup -q

%build
./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var
make

%install 
make install prefix=$RPM_BUILD_ROOT/usr

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0644, root, root)
/usr/lib/kde3/kstyle_lipstik_config.la
/usr/lib/kde3/kstyle_lipstik_config.so
/usr/lib/kde3/plugins/styles/lipstik.la
/usr/lib/kde3/plugins/styles/lipstik.so
/usr/share/apps/kstyle/themes/lipstik.themerc
/usr/share/apps/kdisplay/color-schemes/lipstikstandard.kcsrc
/usr/share/apps/kdisplay/color-schemes/lipstikwhite.kcsrc
/usr/share/apps/kdisplay/color-schemes/lipstiknoble.kcsrc

%changelog
* Mon Oct 01 2007 Patrice Tremblay
Version 2.2.3, for my PC-BSD friends... :
- Add a configurable active tab highlight color.

* Sat Feb 17 2007 Patrice Tremblay
Version 2.2.2, based on the special release for Debian, 2.2.1, which 
was never announced:
- A tweak in the build system.
- An updated spec file thanks Pramod Venugopal
- Toolbar gradients are now off by default.

* Wed Jan 31 2007 Pramod Venugopal
Updated Spec file for FC6 with the following:
- Changed dependencies to reflect FC6 packages
- Changed prefix in configure script to point to default locations
- Changed prefix in install to lowercase and to point to usr
- Changed files to reflect new locations
