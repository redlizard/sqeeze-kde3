Summary: Serenity theme for KDE
Name: serenity
Version: 1.7.1
Release: 1
Source: %{name}-%{version}.tar.bz2
URL: http://www.kde-look.org/content/show.php?content=35954
Vendor: maxilys (at) tele2.fr
License: GPL
Group: System/GUI/KDE
Packager: Remi Villatel <maxilys@tele2.fr>
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Serenity is a soft and quiet theme to ease your mind, 
with a lot of options to make the Serenity yours.

%prep
%setup


%build
./configure --enable-final --disable-debug --prefix=$RPM_BUILD_ROOT/opt/kde3
make

%install
make install PREFIX=$RPM_BUILD_ROOT/opt/kde3

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(0644, root, root)
/opt/kde3/lib/kde3/kwin3_serenity.la
/opt/kde3/lib/kde3/kwin3_serenity.so
/opt/kde3/lib/kde3/kwin_serenity_config.la
/opt/kde3/lib/kde3/kwin_serenity_config.so
/opt/kde3/share/apps/kwin/serenity.desktop
/opt/kde3/lib/kde3/kstyle_serenity_config.la
/opt/kde3/lib/kde3/kstyle_serenity_config.so
/opt/kde3/lib/kde3/plugins/styles/serenity.la
/opt/kde3/lib/kde3/plugins/styles/serenity.so
/opt/kde3/share/apps/kstyle/themes/serenity.themerc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Aerial.Blue.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Aurora.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Brown.Dwarf.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Caloris.Planitia.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Dark.Sun.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Deep.Water.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Frozen.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Lost.Island.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Lotus.Flower.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Mare.Serenitatis.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Matrix.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Midnight.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Milky.Way.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Mint.Candy.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Modern.KDE.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Neptune.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Ocean.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Open.Sky.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Outer.Space.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Roses.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Snow.Fields.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Spring.Leaf.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Sunlight.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Venus.kcsrc
/opt/kde3/share/apps/kdisplay/color-schemes/Serenity.Wooden.Cabin.kcsrc
