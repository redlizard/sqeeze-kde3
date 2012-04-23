Name: kdiff3
Version: 0.9.91
Release: 1.fc

URL: http://www.kde-apps.org/content/show.php?content=9807
License: GPL
Summary: Tool for Comparison and Merge of Files and Directories
Group: Development/Tools

Source: http://heanet.dl.sourceforge.net/sourceforge/kdiff3/kdiff3-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

Requires: kdelibs > 3.3
BuildRequires: gcc-c++ >= 3.2
BuildRequires: xorg-x11-devel
BuildRequires: qt-devel >= 3.3
BuildRequires: kdelibs-devel >= 3.3

%description
Shows the differences line by line and character by character (!).
Provides an automatic merge-facility and
an integrated editor for comfortable solving of merge-conflicts.
Supports KIO on KDE (allows accessing ftp, sftp, fish, smb etc.).
Unicode & UTF-8 support

%prep
%setup

%build
[ -n "$QTDIR" ] || . %{_sysconfdir}/profile.d/qt.sh

export KDEDIR=%{_prefix}

%configure --prefix=/usr
%{__make} %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
source  /etc/profile.d/qt.sh
%makeinstall

%clean
%{__rm} -rf %{buildroot}

%files
%doc AUTHORS ChangeLog COPYING NEWS README TODO
%{_bindir}/kdiff3
%{_datadir}/applnk/*
%{_datadir}/apps/kdiff3/*
%{_datadir}/apps/kdiff3part/*
%{_datadir}/doc/HTML/*
%{_datadir}/icons/*
%{_datadir}/locale/*
%{_datadir}/man/man1/kdiff3*
%{_datadir}/services/kdiff3*
%{_libdir}/kde3/libkdiff3*

%changelog
* Mon May 15 2006 Vadim Likhota <vadim-lvv@yandex.ru> - 0.9.90-1.fc
- write spec for fc/rhel/centos/asp for kdiff3

