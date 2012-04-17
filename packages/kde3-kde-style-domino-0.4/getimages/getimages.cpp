/*
	Copyright (C) 2006 Michael Lentner <michaell@gmx.net>
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Library General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/
#include <qdir.h>
#include "../client/clientData.h"
namespace dembed {
#include "../domino/data.h"
}


int main ( int /*argc*/, char **/*argv*/ )
{
	
	QDir d;
	d.mkdir("style_pixmaps");
	d.mkdir("client_pixmaps");
	
	QImage * img;
	
	for ( int i = 0; embed_image_vec[i].data; i++ ) {
		qDebug("ss");
		img = new QImage((uchar*)embed_image_vec[i].data,
				embed_image_vec[i].width,
				embed_image_vec[i].height,
				embed_image_vec[i].depth,
				(QRgb*)embed_image_vec[i].colorTable,
				embed_image_vec[i].numColors,
				QImage::BigEndian );
		
		if ( embed_image_vec[i].alpha )
			img->setAlphaBuffer( TRUE );
		
		img->save(QString("style_pixmaps/") + embed_image_vec[i].name + QString(".png"), "PNG");
		delete img;
	}
	
	for ( int i = 0; dembed::embed_image_vec[i].data; i++ ) {
		img = new QImage((uchar*)embed_image_vec[i].data,
				dembed::embed_image_vec[i].width,
				dembed::embed_image_vec[i].height,
				dembed::embed_image_vec[i].depth,
				(QRgb*)dembed::embed_image_vec[i].colorTable,
				dembed::embed_image_vec[i].numColors,
				QImage::BigEndian );
		
		if ( dembed::embed_image_vec[i].alpha )
			img->setAlphaBuffer( TRUE );
		
		img->save(QString("client_pixmaps/") + dembed::embed_image_vec[i].name + QString(".png"), "PNG");
		delete img;
	}
	return 0;
}
