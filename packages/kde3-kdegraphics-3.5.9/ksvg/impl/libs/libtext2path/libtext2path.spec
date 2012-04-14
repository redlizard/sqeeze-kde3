%define distversion %( perl -e 'Creating /home/nikoz/Projects/libtext2path/libtext2path.spec...=\<\>;/(\d+)\.(\d)\.?(\d)?/; print "".(||0)' /etc/*-release)
Name: libtext2path
Summary: libtext2path -- Some description
Version: 0.1
Release: %{_vendor}_%{distversion}
Copyright: GPL
Group: X11/KDE/Utilities
Source: ftp://ftp.kde.org/pub/kde/unstable/apps/utils/%{name}-%{version}.tar.gz
Packager: Nikolas Zimmermann <wildfox@kde.org>
BuildRoot: /tmp/%{name}-%{version}
Prefix: /usr/kde/cvs

%description
A long description

%prep
rm -rf $RPM_BUILD_ROOT
%setup -n %{name}-%{version}
CFLAGS="" CXXFLAGS="" ./configure 	--disable-debug --enable-final --prefix=%{prefix}

%build
# Setup for parallel builds
numprocs=1
if [ "" = "0" ]; then
  numprocs=1
fi

make -j

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd 
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > /%{name}-master.list
find . -type f -o -type l | sed 's|^\.||' >> $RPM_BUILD_DIR/%{name}-master.list

%clean
rm -rf $RPM_BUILD_DIR/%{name}-%{version}
rm -rf $RPM_BUILD_DIR/-master.list

%files -f $RPM_BUILD_DIR/%{name}-master.list
