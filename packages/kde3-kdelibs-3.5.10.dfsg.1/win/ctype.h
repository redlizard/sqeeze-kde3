/*
   This file is part of the KDE libraries
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEWIN_CTYPE_H
#define KDEWIN_CTYPE_H

#ifdef _MSC_VER
# include <msvc/ctype.h>
#elif defined(__BORLANDC__)
# include <bcc/ctype.h>
#endif

#ifdef __cplusplus
inline int KDE_isspace(int c) { return ((unsigned)(c + 1) <= 256) && isspace(c); }
#else
#define KDE_isspace(__c) (((unsigned)(__c + 1) <= 256) && isspace(__c))
#endif

#define isspace KDE_isspace

#endif /* KDEWIN_CTYPE_H */
