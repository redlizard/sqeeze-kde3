#    This file is part of the KDE libraries
#    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
#              (C) 1997,1998 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

include admin/deps.am

$(top_srcdir)/acinclude.m4: $(top_srcdir)/libltdl/ltdl.m4

AUTOMAKE_OPTIONS = foreign 1.6.1
COMPILE_FIRST = libltdl mcop mcopidl flow mcop_mt soundserver artsc examples tests doc 
EXTRA_DIST = admin

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin

#DIST_SUBDIRS = mcop mcop_mt mcopidl flow soundserver artsc examples tests doc
#  gmcop x11 qtmcop kde knotify message
SUBDIRS=$(TOPSUBDIRS)
