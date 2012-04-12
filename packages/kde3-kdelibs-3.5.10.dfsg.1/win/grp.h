/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef GRP_H_
#define GRP_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct group {
	char *gr_name; /* group name */
	char *gr_passwd; /* group password */
	gid_t gr_gid; /* group id */ 
	char **gr_mem; /* group members */
};

KDEWIN32_EXPORT struct group *getgrnam (const char *name); 
KDEWIN32_EXPORT struct group *getgrgid (gid_t gid);
KDEWIN32_EXPORT struct group *getgrent(void);
KDEWIN32_EXPORT void setgrent(void);
KDEWIN32_EXPORT void endgrent(void);

#ifdef __cplusplus
}
#endif

#endif /* GRP_H_ */

