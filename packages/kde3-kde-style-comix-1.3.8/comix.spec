Name:           comix
Version:        1.3.8
Release:        1%{?dist}
Summary:        Comix style and window decoration for KDE

Group:          System/GUI/KDE
License:        GPL
URL:            http://kde-look.org/content/show.php?content=16028
Source0:        comix-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

Requires: kdebase3 >= 3.2 kdelibs3 >= 3.2 qt3 >= 3.3
BuildRequires:  xorg-x11-devel qt3-devel kdelibs3-devel kdebase3-devel

Packager:		Jens Luetkens <j.luetkens@hamburg.de>
Distribution:	openSuSE-10.0

prefix: /opt/kde3

%description
This Package contains the Comix style and window decoration for KDE 3.2 and later.

%prep
%setup -q

%build
./configure --prefix=%{prefix} --enable-final
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot} 


%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING README TODO ChangeLog
%{prefix}/lib/kde3/kstyle_comix_config.la
%{prefix}/lib/kde3/kstyle_comix_config.so
%{prefix}/lib/kde3/kwin3_comix.la
%{prefix}/lib/kde3/kwin3_comix.so
%{prefix}/lib/kde3/kwin_comix_config.la
%{prefix}/lib/kde3/kwin_comix_config.so
%{prefix}/lib/kde3/plugins/styles/comix.la
%{prefix}/lib/kde3/plugins/styles/comix.so
%{prefix}/share/apps/kdisplay/color-schemes/Comix.kcsrc
%{prefix}/share/apps/kstyle/themes/Comix.themerc
%{prefix}/share/apps/kwin/comix.desktop



%changelog
* Sun May 01 2005 Jens Luetkens <j.luetkens@hamburg.de> - 1.3
- Version 1.3 Release

* Thu Jan 20 2005 Jens Luetkens <j.luetkens@hamburg.de> - 1.2
- Version 1.2 Release

* Tue Sep 23 2004 Jens Luetkens <j.luetkens@hamburg.de> - 1.1
- Version 1.1 Release

* Tue Sep 14 2004 Jens Luetkens <j.luetkens@hamburg.de> - 1.0
- Version 1.0 Release

* Mon Sep 06 2004 Jens Luetkens <j.luetkens@hamburg.de> - 0.1
- Initial Release

