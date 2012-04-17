#!/bin/sh

make -f admin/Makefile.common package-messages
cd po
make merge
cd ..
