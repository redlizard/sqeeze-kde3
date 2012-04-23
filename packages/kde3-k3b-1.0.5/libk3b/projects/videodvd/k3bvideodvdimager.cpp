/* 
 *
 * $Id: k3bvideodvdimager.cpp 633751 2007-02-15 08:22:49Z trueg $
 * Copyright (C) 2004-2007 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bvideodvdimager.h"
#include "k3bvideodvddoc.h"
#include <k3bdiritem.h>
#include <k3bfileitem.h>
#include <k3bprocess.h>
#include <k3bglobals.h>

#include <ktempfile.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>

#include <qtextstream.h>
#include <qdir.h>
#include <qfile.h>
#include <qptrlist.h>



class K3bVideoDvdImager::Private
{
public:
  K3bVideoDvdDoc* doc;

  QString tempPath;
};


K3bVideoDvdImager::K3bVideoDvdImager( K3bVideoDvdDoc* doc, K3bJobHandler* jh, QObject* parent, const char* name )
  : K3bIsoImager( doc, jh, parent, name )
{
  d = new Private;
  d->doc = doc;
}


K3bVideoDvdImager::~K3bVideoDvdImager()
{
  delete d;
}


void K3bVideoDvdImager::start()
{
  fixVideoDVDSettings();
  K3bIsoImager::start();
}


void K3bVideoDvdImager::init()
{
  fixVideoDVDSettings();
  K3bIsoImager::init();
}


void K3bVideoDvdImager::fixVideoDVDSettings()
{
  // Video DVD defaults, we cannot set these in K3bVideoDvdDoc since they
  // will be overwritten in the burn dialog unless we create some K3bVideoDVDIsoOptions
  // class with different defaults. But since the whole Video DVD project is a hack we
  // go the easy road.
  K3bIsoOptions o = d->doc->isoOptions(); 
  o.setISOLevel(1); 
  o.setISOallow31charFilenames(false); 
  o.setCreateJoliet(false); 
  o.setJolietLong(false); 
  o.setCreateRockRidge(false); 
  o.setCreateUdf(true); 
  d->doc->setIsoOptions( o ); 
}


void K3bVideoDvdImager::calculateSize()
{
  fixVideoDVDSettings();
  K3bIsoImager::calculateSize();
}


int K3bVideoDvdImager::writePathSpec()
{
  //
  // Create a temp dir and link all contents of the VIDEO_TS dir to make mkisofs 
  // able to handle the VideoDVD stuff.
  //
  // mkisofs is not able to create VideoDVDs from graft-points.
  //
  // We do this here since K3bIsoImager::start calls cleanup which deletes the temp files
  //
  QDir dir( KGlobal::dirs()->resourceDirs( "tmp" ).first() );
  d->tempPath = K3b::findUniqueFilePrefix( "k3bVideoDvd", dir.path() );
  kdDebug() << "(K3bVideoDvdImager) creating temp dir: " << d->tempPath << endl;
  if( !dir.mkdir( d->tempPath, true ) ) {
    emit infoMessage( i18n("Unable to create temporary directory '%1'.").arg(d->tempPath), ERROR );
    return -1;
  }

  dir.cd( d->tempPath );
  if( !dir.mkdir( "VIDEO_TS" ) ) {
    emit infoMessage( i18n("Unable to create temporary directory '%1'.").arg(d->tempPath + "/VIDEO_TS"), ERROR );
    return -1;
  }
  
  for( QPtrListIterator<K3bDataItem> it( d->doc->videoTsDir()->children() ); *it; ++it ) {
    if( (*it)->isDir() ) {
      emit infoMessage( i18n("Found invalid entry in the VIDEO_TS folder (%1).").arg((*it)->k3bName()), ERROR );
      return -1;
    }

    // convert to upper case names
    if( ::symlink( QFile::encodeName( (*it)->localPath() ), 
		   QFile::encodeName( d->tempPath + "/VIDEO_TS/" + (*it)->k3bName().upper() ) ) == -1 ) {
      emit infoMessage( i18n("Unable to link temporary file in folder %1.").arg( d->tempPath ), ERROR );
      return -1;
    }
  }


  return K3bIsoImager::writePathSpec();
}


int K3bVideoDvdImager::writePathSpecForDir( K3bDirItem* dirItem, QTextStream& stream )
{
  //
  // We handle the VIDEO_TS dir differently since otherwise mkisofs is not able to 
  // open the VideoDVD structures (see addMkisofsParameters)
  //
  if( dirItem == d->doc->videoTsDir() ) {
    return 0;
  }

  int num = 0;
  for( QPtrListIterator<K3bDataItem> it( dirItem->children() ); it.current(); ++it ) {
    K3bDataItem* item = it.current();
    num++;
      
    if( item->isDir() ) {
      // we cannot add the video_ts dir twice
      if( item != d->doc->videoTsDir() ) {
	stream << escapeGraftPoint( item->writtenPath() )
	       << "="
	       << escapeGraftPoint( dummyDir( static_cast<K3bDirItem*>(item) ) ) << "\n";
      }

      int x = writePathSpecForDir( dynamic_cast<K3bDirItem*>(item), stream );
      if( x >= 0 )
	num += x;
      else
	return -1;
    }
    else {
      writePathSpecForFile( static_cast<K3bFileItem*>(item), stream );
    }
  }

  return num;
}


bool K3bVideoDvdImager::addMkisofsParameters( bool printSize )
{
  // Here is another bad design: we assume that K3bIsoImager::start does not add additional 
  // parameters to the process. :(
  if( K3bIsoImager::addMkisofsParameters( printSize ) ) {
    *m_process << "-dvd-video";
    *m_process << "-f"; // follow symlinks
    *m_process << d->tempPath;
    return true;
  }
  else
    return false;
}


void K3bVideoDvdImager::cleanup()
{
  if( QFile::exists( d->tempPath ) ) {
    QDir dir( d->tempPath );
    dir.cd( "VIDEO_TS" );
    for( QPtrListIterator<K3bDataItem> it( d->doc->videoTsDir()->children() ); *it; ++it )
      dir.remove( (*it)->k3bName().upper() );
    dir.cdUp();
    dir.rmdir( "VIDEO_TS" );
    dir.cdUp();
    dir.rmdir( d->tempPath );
  }
  d->tempPath = QString::null;

  K3bIsoImager::cleanup();
}


void K3bVideoDvdImager::slotReceivedStderr( const QString& line )
{
  if( line.contains( "Unable to make a DVD-Video image" ) ) {
    emit infoMessage( i18n("The project does not contain all necessary VideoDVD files."), WARNING );
    emit infoMessage( i18n("The resulting DVD will most likely not be playable on a Hifi DVD player."), WARNING );
  }
  else 
    K3bIsoImager::slotReceivedStderr( line );
}

#include "k3bvideodvdimager.moc"
