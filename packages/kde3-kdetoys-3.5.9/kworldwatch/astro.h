/*
**
** Copyright (C) 1998-2001 by Matthias H�lzer-Kl�pfel <hoelzer@kde.org>
**	Maintainence has ceased - send questions to kde-devel@kde.org.
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/

#ifndef ASTRO_H
#define ASTRO_H

#include <time.h>

extern "C" {
double jtime(struct tm *t);
double kepler(double m, double ecc);
void sunpos(double jd, int apparent, double *ra, double *dec, double *rv, double *slong);
void projillum(short *wtab, int xdots, int ydots, double dec);
}

#endif // ASTRO_H

