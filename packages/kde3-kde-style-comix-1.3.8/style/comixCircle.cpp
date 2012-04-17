/* Comix widget style v. 1.3 for KDE 3 circle addition
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <qimage.h>

#include "comixCircle.h"

ComixCircle::ComixCircle(int radius, int width) 
{

	blackMap = NULL;
	whiteMap = NULL;
	fillMap = NULL;
	
	blackMap = circlePixels(radius, 0, blackMap);
	whiteMap = circlePixels(radius - width, width, whiteMap);
	fillMap = fillPixels(blackMap, whiteMap, fillMap);
	
	cradius = radius;
	
/*
		the pixels are ordered as follows (12x12@2 whiteMap),
		blackMap Pixel are ordered accordingly.
		
		 0                     11
		+-+-+-+-+-+-+-+-+-+-+-+-+
	   0| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | | |5|3|2|1|0|
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | |7|4| | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | |9|8|6| | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
		| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
	  11| | | | | | | | | | | | |
		+-+-+-+-+-+-+-+-+-+-+-+-+
*/
	
}


ComixCircle::~ComixCircle()
{

	while( blackMap != NULL ) {
        blackMap = RemPixel(blackMap);
    }
	while( whiteMap != NULL ) {
        whiteMap = RemPixel(whiteMap);
    }
	while( fillMap != NULL ) {
        fillMap = RemPixel(fillMap);
    }
	
}

//
// circlePixmap(const QColor &color)
//
QPixmap* ComixCircle::circlePixmap(const QColor &color) const {

	QRgb rgb = color.rgb();
	unsigned int rgba;
	PixelMap *thePixel;
	int reversealpha = 0;
	int border = cradius + cradius - 1;
	int w = 0, h = 0, max = 0;
	

	QImage cImage = QImage( (cradius+cradius), (cradius+cradius), 32 );
	cImage.setAlphaBuffer(true);
	
	// deep-clear the image
	Q_UINT32 *rdata = reinterpret_cast<Q_UINT32*>( cImage.bits() );
	w = h = cradius + cradius;
	max = w * h;
	for (int i = 0; i < max; i++) {
		*(rdata++) = 0;
	}
	
		thePixel = fillMap;
		while( thePixel ) {
			// solid draw
			rgba = qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),255);
			// top left
			cImage.setPixel( thePixel->x, thePixel->y, rgba );
			cImage.setPixel( thePixel->y, thePixel->x, rgba );
			// top right
			cImage.setPixel( border - thePixel->x, thePixel->y, rgba );
			cImage.setPixel( border - thePixel->y, thePixel->x, rgba );
			// bottom left
			cImage.setPixel( thePixel->x, border - thePixel->y, rgba );
			cImage.setPixel( thePixel->y, border - thePixel->x, rgba );
			// bottom right
			cImage.setPixel( border - thePixel->x, border - thePixel->y, rgba );
			cImage.setPixel( border - thePixel->y, border - thePixel->x, rgba );

			thePixel = thePixel->next;
		}

		thePixel = blackMap;
		while( thePixel ) {

			rgba = qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),thePixel->alpha);

			// top left
			cImage.setPixel( thePixel->x, thePixel->y, rgba );
			cImage.setPixel( thePixel->y, thePixel->x, rgba );
			// top right
			cImage.setPixel( border - thePixel->x, thePixel->y, rgba );
			cImage.setPixel( border - thePixel->y, thePixel->x, rgba );
			// bottom left
			cImage.setPixel( thePixel->x, border - thePixel->y, rgba );
			cImage.setPixel( thePixel->y, border - thePixel->x, rgba );
			// bottom right
			cImage.setPixel( border - thePixel->x, border - thePixel->y, rgba );
			cImage.setPixel( border - thePixel->y, border - thePixel->x, rgba );

			thePixel = thePixel->next;
		}

		thePixel = whiteMap;
		while( thePixel ) {

			reversealpha = 255 - thePixel->alpha;
			rgba = qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),reversealpha);

			// top left
			cImage.setPixel( thePixel->x, thePixel->y, rgba );
			cImage.setPixel( thePixel->y, thePixel->x, rgba );
			// top right
			cImage.setPixel( border - thePixel->x, thePixel->y, rgba );
			cImage.setPixel( border - thePixel->y, thePixel->x, rgba );
			// bottom left
			cImage.setPixel( thePixel->x, border - thePixel->y, rgba );
			cImage.setPixel( thePixel->y, border - thePixel->x, rgba );
			// bottom right
			cImage.setPixel( border - thePixel->x, border - thePixel->y, rgba );
			cImage.setPixel( border - thePixel->y, border - thePixel->x, rgba );

			thePixel = thePixel->next;
		}
	
	return new QPixmap( cImage );
	
}


//
// circlePixels(int radius)
//
PixelMap* ComixCircle::circlePixels(double radius, double inset, PixelMap *pixelMap) {

	double x, y;
	double pr;
	double inside, outside;
	double xexp2;
	double r;
	double alpha;

	pr = radius - 0.5;
	outside = radius + 0.71;
	inside = radius - 0.71;

	x = radius;
	y = radius;	
	
	//
	// my brute force pixel distance anti-aliasing algorithm
	//
	while( x > 0 ) {
		xexp2 = (x-0.5) * (x-0.5);
		while( y > 0 ) {
			if( y < x ) break;
			r = sqrt( xexp2 + ((y-0.5)*(y-0.5)) );
			if( r <= (radius - 1) ) break;
			if( r < outside && r > inside ) {
				if( r > pr ) {
					alpha = abs((int)(255 * (1.0 - (r - pr))));
				}
				else {
					alpha = abs((int)(255 * (1.0 - (r - inside))));
				}
				//pixelMap = AddPixel(pixelMap,(int)(x-1),(int)y,(int)alpha);
				// better make this be the top left corner:
				if( inset > 0 ) {
					pixelMap = AddPixel(pixelMap,(int)(radius-x+inset),(int)(radius-y+inset),(int)alpha);
				}
				else {
					pixelMap = AddPixel(pixelMap,(int)(radius-x),(int)(radius-y),(int)alpha);
				}
			}
			y--;
		}
		y = radius;
		x--;
	}
	
	return pixelMap;
}

//
// fillPixels(void)
//
PixelMap* ComixCircle::fillPixels(
	PixelMap* blackMap, 
	PixelMap* whiteMap, 
	PixelMap* pixelMap) {
	
	PixelMap* bMap;
	PixelMap* wMap;
	int x=0,y=0,d=0;

/*
	Both Maps represent the top-left corner
	(0° to -45°) with 0/0 in top left
	Pixel in Maps come ordered:
	- bottom to top
	- right to left
	
	so we will loop through white pixels from
	bottom to top and right to left until we 
	find a blck pixel for the same x and y-1.

*/

	bMap = blackMap;
	wMap = whiteMap;
	
	while( wMap != NULL ) {
		
		x = wMap->x;
		while( wMap && wMap->x == x ) {
			y = wMap->y;
			// skip this row
			wMap = wMap->next;
		}
		// we have the topmost white pixel in a column in y.
		// wMap already is in the next column
		while( bMap && bMap->x != x ) {
			bMap = bMap->next;
		}
		// bMap is in the same row bottommost pixel
		if( wMap && bMap ) {
			d = y - 1;
			while( d > bMap->y ) {
				pixelMap = AddPixel(pixelMap,x,d,1);
				d--;
			}
		}
		else {
			break;
		}
	}
	
	return pixelMap;
}

//
// AddPixel(PixelMap *pixelMap, int x, int y, int alpha)
//
PixelMap* ComixCircle::AddPixel(PixelMap *pixelMap, int x, int y, int alpha) {

	PixelMap *newitem;

    if (pixelMap == NULL) {
		pixelMap = new PixelMap;
        pixelMap->next = NULL;
		pixelMap->x = x;
		pixelMap->y = y;
		pixelMap->alpha = alpha;
        return pixelMap;
	}
	else {
		newitem = new PixelMap;
		newitem->x = x;
		newitem->y = y;
		newitem->alpha = alpha;
        newitem->next = pixelMap;
        return newitem;
	}

}

//
// RemPixel(PixelMap *pixelMap)
//
PixelMap* ComixCircle::RemPixel(PixelMap *pixelMap) {

    PixelMap *tempp;
	
    tempp = pixelMap->next;
    delete pixelMap;
    return tempp;
	
}

