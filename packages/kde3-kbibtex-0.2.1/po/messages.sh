#!/bin/sh

PROJECT=kbibtex

cd .. ; make -f admin/Makefile.common package-messages ; cd po

catalogs=`find . -name '*.po'`
for cat in $catalogs; do
  echo $cat
  msgmerge -o $cat.new $cat ${PROJECT}.pot && mv $cat.new $cat
done

echo "Done"
