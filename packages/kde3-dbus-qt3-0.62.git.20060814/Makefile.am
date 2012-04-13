if HAVE_QT3
   QT3_SUBDIR=qt3
endif

## really we should require gcj/mcs/python also but since they are 
## annoying to install, we don't for now
dist-local:
	if test -z "$(QT_SUBDIR)" ; then		\
		echo "You have to build with Qt to make dist" ;	\
	fi 

SUBDIRS=$(QT3_SUBDIR)
DIST_SUBDIRS=qt3

EXTRA_DIST = HACKING
