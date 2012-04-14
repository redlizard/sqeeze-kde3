#!/bin/sh

ADMIN="$1"

test -d "$ADMIN" || {
    echo >&2 "Error, admin dir '$ADMIN' does not exist."
    exit 1
}

rm -rf a b
mkdir  a b

cp -a "$ADMIN" a/admin

############################################################
PATCH=03_libtool_update.diff
rm -rf b/admin; cp -a a/admin b

cp -f /usr/share/libtool/libtool.m4 b/admin/libtool.m4.in
cp -f /usr/share/libtool/ltmain.sh  b/admin/ltmain.sh

diff -Nrua a b >$PATCH
############################################################


############################################################
PATCH=02_autotools_update.diff
AUTOMAKE_DIR=/usr/share/automake-1.10
rm -rf b/admin; cp -a a/admin b

( cd b/admin
  for file in *; do
      case "$file" in
	  config.guess|config.sub)
	      continue
	      ;;
      esac
      if test -r "$AUTOMAKE_DIR/$file"; then
	  cp -f "$AUTOMAKE_DIR/$file" "$file"
      fi
  done
)

diff -Nrua a b >$PATCH
############################################################

rm -rf a b
