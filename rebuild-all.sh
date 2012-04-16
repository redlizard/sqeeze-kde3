#!/bin/bash

set -e

if [ "$EUID" -ne 0 ]; then
	echo "Not running as root, giving up"
	exit 1
fi

targetdir=`realpath "$1"`
test -d $targetdir

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
yakuake
"

dist=squeeze
comp=kde3
arch=amd64
mirror='http://mirrors.nl.kernel.org/debian/'


chrootrun()
{
	chroot $1 bash -c "mount -t proc proc /proc; $2; umount /proc"
}

buildrepo()
{
	pwd=`pwd`
	cd $repopath
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

dir=`mktemp -d`
echo "Building in directory $dir"

aptget="DEBIAN_FRONTEND=noninteractive apt-get -y --allow-unauthenticated"

if [ -n "$2" -a -e "$2" ]; then
	echo "Copying initial distribution from $2"
	cp -a "$2" $dir/squeeze
	cd $dir
else
	pwd=`pwd`
	cd $dir
	echo "Constructing initial distribution"
	debootstrap squeeze squeeze $mirror
	mkdir squeeze/apt-repo
	
	repopath=`realpath squeeze/apt-repo`
	
	
	chrootrun squeeze "$aptget install build-essential dpkg-dev cdbs debhelper quilt joe locales"
	echo en_US.UTF-8 UTF-8 >> squeeze/etc/locale.gen
	chrootrun squeeze locale-gen
	mkdir -p squeeze/apt-repo/pool
	buildrepo
	echo "deb $mirror squeeze main" > squeeze/etc/apt/sources.list
	echo "deb file:///apt-repo/ squeeze kde3" >> squeeze/etc/apt/sources.list
	echo "deb-src file:///apt-repo/ squeeze kde3" >> squeeze/etc/apt/sources.list
	chrootrun squeeze "$aptget update"
	
	if [ -n "$2" ]; then
		cd $pwd
		echo "Copying initial distribution to $2"
		cp -a $dir/squeeze "$2"
		cd $dir
	fi
fi

repopath=`realpath squeeze/apt-repo`
for file in `ls $targetdir`; do cp $targetdir/$file $repopath/pool; done

for package in $packages; do
	fullpackage=`ls -F $packagesdir | grep / | tr '/' ' ' | grep kde3-$package-`
	test -n "$fullpackage"
	test -d $packagesdir/$fullpackage
	
	if [ ! -f $stampdir/$fullpackage ]; then
		echo "Building source package $fullpackage..."
		cp -a squeeze $package
		mkdir $package/build-source-package
		cp -a $packagesdir/$fullpackage $package/build-source-package
		chrootrun $package "cd build-source-package/$fullpackage && dpkg-buildpackage -S -us -uc -d"
		cd $dir
		cp $package/build-source-package/kde3-${package}_* $repopath/pool
		cp $package/build-source-package/kde3-${package}_* $targetdir
		rm -rf $package/build-source-package
		buildrepo
		
		rm -rf $package/apt-repo
		cp -a squeeze/apt-repo $package
		
		echo "Installing build dependencies for package $fullpackage..."
		mkdir $package/build-binary-package
		chrootrun $package "$aptget update"
		chrootrun $package "$aptget build-dep kde3-$package"
		chrootrun $package "cd build-binary-package && apt-get source kde3-$package"
		
		echo "Building binary package $fullpackage for architecture $arch..."
		chrootrun $package "cd build-binary-package/$fullpackage && dpkg-buildpackage -us -uc"
		cp $package/build-binary-package/*.deb $repopath/pool
		cp $package/build-binary-package/*.deb $targetdir
		buildrepo
		
		echo "Package $fullpackage built successfully"
		rm -rf $package
		
		touch $stampdir/$fullpackage
	fi
done

cd .
rm -rf $dir
