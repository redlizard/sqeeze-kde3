#!/bin/sh

make dist-bzip2
cp *.tar.bz2 /usr/src/packages/SOURCES
rpmbuild -bb polyester.spec
mv /usr/src/packages/RPMS/i586/*.rpm .
rm /usr/src/packages/SOURCES/*.tar.bz2
