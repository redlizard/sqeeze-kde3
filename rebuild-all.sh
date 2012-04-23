#!/bin/bash

set -e

if [ "$EUID" -ne 0 ]; then
	echo "Not running as root, giving up"
	exit 1
fi

mkdir -p output
targetdir=`realpath output`

mkdir -p stamp
stampdir=`realpath stamp`

packagesdir=`realpath packages`
packages="
akode
arts
dbus-qt3
imlib
libxxf86misc
wv2

kdelibs
kdebase
kdepim
kdeartwork
kdeedu
kdegames
kdegraphics
kdemultimedia
kdenetwork
kdesdk
kdeaccessibility
kdeaddons
kdeadmin
kdetoys
kdeutils
kdevelop
kdewebdev
koffice
meta-kde
kde-i18n

kde-icons-crystal
kde-icons-crystalproject
kde-icons-gorilla
kde-icons-kneu
kde-icons-korilla
kde-icons-noia
kde-icons-nuovext
kde-icons-nuvola
kde-style-comix
kde-style-domino
kde-style-klearlook
kde-style-lipstik
kde-style-polyester
kde-style-qtcurve
kde-style-serenity
kwin-style-crystal
kwin-style-dekorator
kwin-style-knifty
kwin-style-powder
kdmtheme
kde-kdm-themes
kde-wallpapers-lineartreworked
kwin-decor-suse2

amarok
yakuake
"

dist=squeeze
comp=kde3
arch=amd64
mirror='http://ftp.surfnet.nl/os/Linux/distr/debian/'


chrootrun()
{
	chroot $1 bash -c "mount -t proc proc /proc; $2; umount /proc"
}

buildrepo()
{
	pwd=`pwd`
	cd $1
	mkdir -p dists/$dist/$comp/source
	mkdir -p dists/$dist/$comp/binary-$arch
	
	cd pool
	dpkg-scansources . /dev/null pool/ > ../dists/$dist/$comp/source/Sources
	dpkg-scanpackages -a $arch . /dev/null pool/ > ../dists/$dist/$comp/binary-$arch/Packages
	cd ..
	
	gzip -c dists/$dist/$comp/source/Sources > dists/$dist/$comp/source/Sources.gz
	bzip2 -c dists/$dist/$comp/source/Sources > dists/$dist/$comp/source/Sources.bz2
	gzip -c dists/$dist/$comp/binary-$arch/Packages > dists/$dist/$comp/binary-$arch/Packages.gz
	bzip2 -c dists/$dist/$comp/binary-$arch/Packages > dists/$dist/$comp/binary-$arch/Packages.bz2
	
	cd dists/$dist
	echo "Origin: rebuild-all" > Release
	echo "Label: rebuid-all" >> Release
	echo "Components: $comp" >> Release
	echo "Architectures: $arch" >> Release
	echo "Suite: $dist" >> Release
	
	files="$comp/source/Sources $comp/source/Sources.gz $comp/source/Sources.bz2 $comp/binary-$arch/Packages $comp/binary-$arch/Packages.gz $comp/binary-$arch/Packages.bz2"
	
	echo "MD5Sum:" >> Release
	for file in $files; do
		md5=`md5sum $file | awk '{ print $1 }'`
		size=`wc -c $file | awk '{ print $1 }'`
		printf " %s %17i %s\n" $md5 $size $file >> Release
	done
	echo "" >> Release
	
	echo "SHA1:" >> Release
	for file in $files; do
		sha1=`sha1sum $file | awk '{ print $1 }'`
		size=`wc -c $file | awk '{ print $1 }'`
		printf " %s %17i %s\n" $sha1 $size $file >> Release
	done
	echo "" >> Release
	
	cd $pwd
}


aptget="DEBIAN_FRONTEND=noninteractive apt-get -y --allow-unauthenticated"

if [ ! -d squeeze-base ]; then
	echo "Constructing base distribution"
	debootstrap squeeze squeeze-base $mirror
	mkdir squeeze-base/apt-repo
	
	echo "#!/bin/dash" > squeeze-base/usr/sbin/invoke-rc.d
	chmod 755 squeeze-base/usr/sbin/invoke-rc.d
	
	chrootrun squeeze-base "$aptget install build-essential dpkg-dev cdbs debhelper quilt dpatch joe locales"
	echo en_US.UTF-8 UTF-8 >> squeeze-base/etc/locale.gen
	chrootrun squeeze-base locale-gen
	mkdir -p squeeze-base/apt-repo/pool
	buildrepo squeeze-base/apt-repo
	echo "deb $mirror squeeze main" > squeeze-base/etc/apt/sources.list
	echo "deb file:///apt-repo/ squeeze kde3" >> squeeze-base/etc/apt/sources.list
	echo "deb-src file:///apt-repo/ squeeze kde3" >> squeeze-base/etc/apt/sources.list
	chrootrun squeeze-base "$aptget update"
	chrootrun squeeze-base "$aptget clean"
fi

dir=`mktemp -d build-XXXXXXXXXX`
mount -t tmpfs tmpfs $dir
chmod 755 $dir
echo "Building in directory $dir"
cd $dir

for package in $packages; do
	fullpackage=`ls -F $packagesdir | grep / | tr '/' ' ' | grep kde3-$package-`
	test -n "$fullpackage"
	test -d $packagesdir/$fullpackage
	
	if [ ! -f $stampdir/$fullpackage ]; then
		echo "Building source package $fullpackage..."
		cp -a ../squeeze-base $package
		
		mkdir $package/build-source-package
		cp -a $packagesdir/$fullpackage $package/build-source-package
		chrootrun $package "cd build-source-package/$fullpackage && dpkg-buildpackage -S -us -uc -d"
		cp $package/build-source-package/kde3-${package}_* $targetdir
		rm -rf $package/build-source-package
		
		echo "Preparing build dependencies for package $fullpackage..."
		rm -rf $package/apt-repo
		mkdir -p $package/apt-repo/pool
		#for file in `ls $targetdir`; do ln $targetdir/$file $package/apt-repo/pool; done
		for file in `ls $targetdir`; do cp $targetdir/$file $package/apt-repo/pool; done
		buildrepo $package/apt-repo
		
		echo "Installing build dependencies for package $fullpackage..."
		mkdir $package/build-binary-package
		chrootrun $package "$aptget update"
		chrootrun $package "$aptget build-dep kde3-$package"
		chrootrun $package "cd build-binary-package && apt-get source kde3-$package"
		chrootrun $package "$aptget clean"
		
		echo "Building binary package $fullpackage for architecture $arch..."
		chrootrun $package "cd build-binary-package/$fullpackage && dpkg-buildpackage -us -uc"
		cp $package/build-binary-package/*.deb $targetdir
		
		echo "Package $fullpackage built successfully"
		rm -rf $package
		
		touch $stampdir/$fullpackage
	fi
done

cd ..
umount $dir
rmdir $dir
