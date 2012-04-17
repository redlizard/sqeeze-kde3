

/*
 * Copyright 2003, Sandro Giessl <ceebx@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <qcolor.h>
#include "misc.h"
#include <qimage.h>
#include <qpixmap.h>
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#undef WORDS_BIGENDIAN
#define WORDS_LITTLEENDIAN 1
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
	#undef WORDS_LITTLEENDIAN
	#define WORDS_BIGENDIAN 1
#endif




QColor alphaBlendColors(const QColor &bgColor, const QColor &fgColor, const int a)
{
	QRgb rgb = bgColor.rgb();
	QRgb rgb_b = fgColor.rgb();
	uint alpha;
	if(a > 255) alpha = 255;
	else if(a < 0) alpha = 0;
	else alpha = a;
	uint inv_alpha = 255 - alpha;
	
	return QColor((qRed(rgb_b)*inv_alpha + qRed(rgb)*alpha)>>8,
				(qGreen(rgb_b)*inv_alpha + qGreen(rgb)*alpha)>>8,
				(qBlue(rgb_b)*inv_alpha + qBlue(rgb)*alpha)>>8 );
}

QColor blendColors(const QColor &bgColor, const QColor &fgColor)
{
	
	uint fg_r = fgColor.red();
	uint fg_g = fgColor.green();
	uint fg_b = fgColor.blue();
	uint fg_a = qAlpha(fgColor.rgb());
	
	uint bg_r = bgColor.red();
	uint bg_g = bgColor.green();
	uint bg_b = bgColor.blue();
	uint bg_a = qAlpha(bgColor.rgb());
	
	uint ac = 65025 - (255 - fg_a) * (255 - bg_a);
	uint res_r = ((255 - fg_a) * bg_r * bg_a + fg_a * fg_r  * 255 + 127) / ac;
	uint res_g = ((255 - fg_a) * bg_g * bg_a + fg_a * fg_g  * 255 + 127) / ac;
	uint res_b = ((255 - fg_a) * bg_b * bg_a + fg_a * fg_b  * 255 + 127) / ac;
	uint res_a = (ac+127)/255;
	
	return QColor(qRgba(res_r,res_g, res_b, res_a ));
}

QImage tintImage(const QImage &img, const QColor &tintColor) {
	
	QImage *result = new QImage(img.width(), img.height(), 32, 0, QImage::IgnoreEndian);
	result->setAlphaBuffer( true );
	register uint *data = (unsigned int*) img.bits();
	register uint *resultData = (unsigned int*) result->bits();
	register uint total = img.width()*img.height();
	for ( uint current = 0 ; current < total ; ++current ) {
		resultData[ current ] = qRgba( tintColor.red(), tintColor.green(), tintColor.blue(), qAlpha( data[ current ] ));
	}
	return *result;
}

QImage setImageOpacity(const QImage &img, const uint &p) {
	QImage *result = new QImage(img.width(), img.height(), 32, 0, QImage::IgnoreEndian);
	result->setAlphaBuffer( true );
	register uint *data = (unsigned int*) img.bits();
	register uint *resultData = (unsigned int*) result->bits();
	register uint alpha;
	register uint total = img.width()*img.height();
	for ( uint current = 0 ; current < total ; ++current ) {
		alpha = qAlpha( data[ current ] ) * p / 100;
		resultData[ current ] = qRgba( qRed( data[ current ] ), qGreen( data[ current ] ), qBlue( data[ current ] ), alpha );
	}
	return *result;
}

bool blend( const QImage & upper, const QImage & lower, QImage & output)
// adopted from kimageeffect::blend - that is not endian safe and cannot handle complex alpha combinations...
{
	if
		(
		  upper.width()  > lower.width()  ||
		  upper.height() > lower.height() ||
		  upper.depth() != 32             ||
		  lower.depth() != 32
		)
		return false;
	
	output = lower.copy();
	
	uchar *i, *o;
	register uint a, ab, ac;
	register uint col;
	register uint w = upper.width();
	int row(upper.height() - 1);
	
	do
	{
		i = upper.scanLine(row);
		o = output.scanLine(row);
		
		col = w << 2;
		
		--col;
		
		do
		{
#ifdef WORDS_BIGENDIAN
			while (!(a = i[col-3]) && (col != 3))
#else
				while (!(a = i[col]) && (col != 3))
#endif
				{
					--col; --col; --col; --col;
				}
#ifdef WORDS_BIGENDIAN
			if ((ab = o[col-3]))
#else
				if ((ab = o[col]))
#endif
				{
					ac = 65025 - (255 - a) * (255 - ab);
#ifndef WORDS_BIGENDIAN
					o[col]= (ac+127)/255;
					--col;
#endif
					o[col] = ((255 - a) * o[col] * ab + a * i[col] * 255 + 127) / ac;
					--col;
					o[col] = ((255 - a) * o[col] * ab + a * i[col] * 255 + 127) / ac;
					--col;
					o[col] = ((255 - a) * o[col] * ab + a * i[col]  * 255 + 127) / ac;
#ifdef WORDS_BIGENDIAN
				--col;
				o[col]= (ac+127)/255;
#endif
				}
			else
			{
				o[col] = i[col]; --col;
				o[col] = i[col]; --col;
				o[col] = i[col]; --col;
				o[col] = i[col];
			}
		} while (col--);
	} while (row--);
	return true;
}
