/* Comix widget style for KDE 3 circle addition
   Copyright (C) 2004 Jens Luetkens <j.luetkens@limitland.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#ifndef __COMIXCIRCLE_H
#define __COMIXCIRCLE_H

#include <qcolor.h>
#include <qpixmap.h>

struct PixelMap {
	int x;
	int y;
	int alpha;
	PixelMap *next;
};

class ComixCircle
{

public:

    ComixCircle(int radius, int width);
    virtual ~ComixCircle();

	QPixmap*	circlePixmap(const QColor &color) const;

	PixelMap*	blackMap;
	PixelMap*	whiteMap;
	PixelMap*	fillMap;

private:

	PixelMap*	circlePixels(double radius, double inset, PixelMap* pixelMap);
	PixelMap*	fillPixels(
					PixelMap* outerPixels, 
					PixelMap* innerPixels, 
					PixelMap* pixelMap);

	PixelMap*	AddPixel(PixelMap *pixelMap, int x, int y, int alpha);
	PixelMap*	RemPixel(PixelMap *pixelMap);
	
	int			cradius;
	
};

#endif // __COMIXCIRCLE_H
