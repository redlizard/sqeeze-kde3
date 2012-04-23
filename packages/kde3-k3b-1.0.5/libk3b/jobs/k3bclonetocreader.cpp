/* 
 *
 * $Id: k3bclonetocreader.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <config.h>


#include "k3bclonetocreader.h"

#include <k3bdeviceglobals.h>
#include <k3bglobals.h>

#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>


class K3bCloneTocReader::Private
{
public:
  Private()
    : size(0) {
  }

  K3b::Msf size;
  QString tocFile;
};



K3bCloneTocReader::K3bCloneTocReader( const QString& filename )
  : K3bImageFileReader()
{
  d = new Private;
  openFile( filename );
}


K3bCloneTocReader::~K3bCloneTocReader()
{
  delete d;
}


const K3b::Msf& K3bCloneTocReader::imageSize() const
{
  return d->size;
}


void K3bCloneTocReader::readFile()
{
  // first of all we check if we find the image file which contains the data for this toc
  // cdrecord always uses this strange file naming:
  //   somedata
  //   somedata.toc

  // filename should always be the toc file
  if( filename().right( 4 ) == ".toc" )
    d->tocFile = filename();
  else
    d->tocFile = filename() + ".toc";

  // now get rid of the ".toc" extension
  QString imageFileName = d->tocFile.left( d->tocFile.length()-4 );
  if( !QFile::exists( imageFileName ) ) {
    kdDebug() << "(K3bCloneTocReader) could not find image file " << imageFileName << endl;
    return;
  }

  setImageFilename( imageFileName );

  d->size = 0;

  QFile f( d->tocFile );
  if( f.open( IO_ReadOnly ) ) {
    //
    // Inspired by clone.c from the cdrecord sources
    //
    char buffer[2048];
    int read = f.readBlock( buffer, 2048 );
    f.close();

    if( read == 2048 ) {
      kdDebug() << "(K3bCloneTocReader) TOC too large." << endl;
      return;
    }

    // the toc starts with a tocheader
    struct tocheader {
      unsigned char len[2];
      unsigned char first; // first session
      unsigned char last; // last session
    };

    struct tocheader* th = (struct tocheader*)buffer;
    int dataLen = K3bDevice::from2Byte( th->len ) + 2;  // the len field does not include it's own length

    if( th->first != 1 ) {
      kdDebug() << "(K3bCloneTocReader) first session != 1" << endl;
      return;
    }

    // the following bytes are multiple instances of
    struct ftrackdesc {
      unsigned char sess_number;
#ifdef WORDS_BIGENDIAN // __BYTE_ORDER == __BIG_ENDIAN
      unsigned char adr      : 4;
      unsigned char control  : 4;
#else
      unsigned char control  : 4;
      unsigned char adr      : 4;
#endif
      unsigned char track;
      unsigned char point;
      unsigned char amin;
      unsigned char asec;
      unsigned char aframe;
      unsigned char res7;
      unsigned char pmin;
      unsigned char psec;
      unsigned char pframe;
    };

    for( int i = 4; i < dataLen; i += 11) {
      struct ftrackdesc* ft = (struct ftrackdesc*)&buffer[i];
      
      if( ft->sess_number != 1 ) {
	kdDebug() << "(K3bCloneTocReader} session number != 1" << endl;
	return;
      }

      // now we check some of the values
      if( ft->point >= 0x1 && ft->point <= 0x63 ) {
	if( ft->adr == 1 ) {
	  // check track starttime
	  if( ft->psec > 60 || ft->pframe > 75 ) {
	    kdDebug() << "(K3bCloneTocReader) invalid track start: " 
		      << (int)ft->pmin << "." 
		      << (int)ft->psec << "."
		      << (int)ft->pframe << endl;
	    return;
	  }
	}
      }
      else {
	switch( ft->point ) {
	case 0xa0:
	  if( ft->adr != 1 ) {
	    kdDebug() << "(K3bCloneTocReader) adr != 1" << endl;
	    return;
	  }

	  // disk type in psec
	  if( ft->psec != 0x00 && ft->psec != 0x10 && ft->psec != 0x20 ) {
	    kdDebug() << "(K3bCloneTocReader) invalid disktype: " << ft->psec << endl;
	    return;
	  }

	  if( ft->pmin != 1 ) {
	    kdDebug() << "(K3bCloneTocReader) first track number != 1 " << endl;
	    return;
	  }

	  if( ft->pframe != 0x0 ) {
	    kdDebug() << "(K3bCloneTocReader) found data when there should be 0x0" << endl;
	    return;
	  }
	  break;

	case  0xa1:
	  if( ft->adr != 1 ) {
	    kdDebug() << "(K3bCloneTocReader) adr != 1" << endl;
	    return;
	  }

	  if( !(ft->pmin >= 1) ) {
	    kdDebug() << "(K3bCloneTocReader) last track number needs to be >= 1." << endl;
	    return;
	  }
	  if( ft->psec != 0x0 || ft->pframe != 0x0 ) {
	    kdDebug() << "(K3bCloneTocReader) found data when there should be 0x0" << endl;
	    return;
	  }
	  break;

	case 0xa2:
	  if( ft->adr != 1 ) {
	    kdDebug() << "(K3bCloneTocReader) adr != 1" << endl;
	    return;
	  }

	  // start of the leadout = size of the image
	  // substract 2 seconds since in cdrecord other than in K3b lba 0 = msf 2:00
	  // (the cdrecord way is actually more accurate but we use k3b::Msf for many
	  // things and it is simpler this way.)
	  d->size = K3b::Msf( ft->pmin, ft->psec, ft->pframe ) - K3b::Msf( 0, 2, 0 );

	  // leadout... no check so far...
	  break;

	default:
	  if( ft->adr != 5 ) {
	    kdDebug() << "(K3bCloneTocReader) adr != 5" << endl;
	    return;
	  }
	  break;
	}
      }
    }

    if( d->size.rawBytes() != K3b::filesize( imageFileName ) ) {
      kdDebug() << "(K3bCloneTocReader) image file size invalid." << endl;
      return;
    }
    
    // ok, could be a cdrecord toc file
    setValid(true);
  }
  else {
    kdDebug() << "(K3bCloneTocReader) could not open file " << d->tocFile << endl;
  }
}
