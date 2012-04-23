/* 
 *
 * $Id: k3bgrowisofshandler.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bgrowisofshandler.h"

#include <k3bjob.h>
#include <k3bcore.h>
#include <k3bglobalsettings.h>
#include <k3bdevice.h>
#include <k3bdevicehandler.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

#include <qtimer.h>

#include <errno.h>
#include <string.h>


class K3bGrowisofsHandler::Private
{
public:
  int lastBuffer;
  int lastDeviceBuffer;
};


K3bGrowisofsHandler::K3bGrowisofsHandler( QObject* parent, const char* name )
  : QObject( parent, name )
{
  d = new Private;
  reset();
}


K3bGrowisofsHandler::~K3bGrowisofsHandler()
{
  delete d;
}


void K3bGrowisofsHandler::reset( K3bDevice::Device* dev, bool dao )
{
  m_device = dev;
  m_error = ERROR_UNKNOWN;
  m_dao = dao;
  d->lastBuffer = 0;
  d->lastDeviceBuffer = 0;
}


void K3bGrowisofsHandler::handleStart()
{
//  QTimer::singleShot( 2000, this, SLOT(slotCheckBufferStatus()) );
}


void K3bGrowisofsHandler::handleLine( const QString& line )
{
  int pos = 0;

  if( line.startsWith( ":-[" ) ) {
    // Error

    if( line.contains( "ASC=30h" ) )
      m_error = ERROR_MEDIA;

    // :-[ PERFORM OPC failed with SK=3h/ASC=73h/ASCQ=03h
    else if( line.startsWith( ":-[ PERFORM OPC failed" ) )
      emit infoMessage( i18n("OPC failed. Please try writing speed 1x."), K3bJob::ERROR );

    // :-[ attempt -blank=full or re-run with -dvd-compat -dvd-compat to engage DAO ]
    else if( !m_dao && 
	     ( line.contains( "engage DAO" ) || line.contains( "media is not formatted or unsupported" ) ) )
      emit infoMessage( i18n("Please try again with writing mode DAO."), K3bJob::ERROR );

    else if( line.startsWith( ":-[ Failed to change write speed" ) ) {
      m_error = ERROR_SPEED_SET_FAILED;
    }
  }
  else if( line.startsWith( ":-(" ) ) {
    if( line.contains( "No space left on device" ) )
      m_error = ERROR_OVERSIZE;

    else if( line.contains( "blocks are free" ) && line.contains( "to be written" ) ) {
      m_error = ERROR_OVERSIZE;
      if( k3bcore->globalSettings()->overburn() )
	emit infoMessage( i18n("Trying to write more than the official disk capacity"), K3bJob::WARNING );
    }

    else if( line.startsWith( ":-( unable to anonymously mmap" ) ) {
      m_error = ERROR_MEMLOCK;
    }

    else if( line.startsWith( ":-( write failed" ) ) {
      m_error = ERROR_WRITE_FAILED;
    }

    else  
      emit infoMessage( line, K3bJob::ERROR );
  }
  else if( line.startsWith( "PERFORM OPC" ) ) {
    m_error = ERROR_OPC;
  }
  else if( line.contains( "flushing cache" ) ) {
    // here is where we already should stop queriying the buffer fill
    // since the device is only used there so far...
    m_device = 0;
    
    emit flushingCache();
    emit newSubTask( i18n("Flushing Cache")  );
    emit infoMessage( i18n("Flushing the cache may take some time."), K3bJob::INFO );
  }

  // FIXME: I think this starts with dev->blockDeviceName() so we could improve parsing with:
  //        if( line.startsWith( dev->blockDeviceName() ) ) {
  //              line = line.mid( dev->blockDeviceName().length() );
  //              if( line.startsWith( "closing.....

  else if( line.contains( "closing track" ) ) {
    emit newSubTask( i18n("Closing Track")  );
  }
  else if( line.contains( "closing disc" ) ) {
    emit newSubTask( i18n("Closing Disk")  );
  }
  else if( line.contains( "closing session" ) ) {
    emit newSubTask( i18n("Closing Session")  );
  }
  else if( line.contains( "updating RMA" ) ) {
    emit newSubTask( i18n("Updating RMA") );
    emit infoMessage( i18n("Updating RMA") + "...", K3bJob::INFO );
  }
  else if( line.contains( "closing session" ) ) {
    emit newSubTask( i18n("Closing Session") );
    emit infoMessage( i18n("Closing Session") + "...", K3bJob::INFO );
  }
  else if( line.contains( "writing lead-out" ) ) {
    emit newSubTask( i18n("Writing Lead-out") );
    emit infoMessage( i18n("Writing the lead-out may take some time."), K3bJob::INFO );
  }
  else if( line.contains( "Quick Grow" ) ) {
    emit infoMessage( i18n("Removing reference to lead-out."), K3bJob::INFO );
  }
  else if( line.contains( "copying volume descriptor" ) ) {
    emit infoMessage( i18n("Modifying ISO9660 volume descriptor"), K3bJob::INFO );
  }
  else if( line.contains( "FEATURE 21h is not on" ) ) {
    if( !m_dao ) {
      // FIXME: it's not only the writer. It may be the media: something like does not support it with this media
      //        da war was mit: wenn einmal formattiert, dann geht nur noch dao oder wenn einmal als overwrite
      //        formattiert, dann nur noch dao oder sowas
      emit infoMessage( i18n("Writing mode Incremental Streaming not available"), K3bJob::WARNING );
      emit infoMessage( i18n("Engaging DAO"), K3bJob::WARNING );
    }
  }
  else if( ( pos = line.find( "Current Write Speed" ) ) > 0 ) {
    // parse write speed
    // /dev/sr0: "Current Write Speed" is 2.4x1385KBps

    pos += 24;
    int endPos = line.find( 'x', pos+1 );
    bool ok = true;
    double speed = line.mid( pos, endPos-pos ).toDouble(&ok);
    if( ok )
      emit infoMessage( i18n("Writing speed: %1 KB/s (%2x)")
			.arg((int)(speed*1385.0))
			.arg(KGlobal::locale()->formatNumber(speed)), K3bJob::INFO );
    else
      kdDebug() << "(K3bGrowisofsHandler) parsing error: '" << line.mid( pos, endPos-pos ) << "'" << endl;
  }
  else if( (pos = line.find( "RBU" )) > 0 ) {

    // FIXME: use QRegExp

    // parse ring buffer fill for growisofs >= 6.0
    pos += 4;
    int endPos = line.find( '%', pos+1 );
    bool ok = true;
    double val = line.mid( pos, endPos-pos ).toDouble( &ok );
    if( ok ) {
      int newBuffer = (int)(val+0.5);
      if( newBuffer != d->lastBuffer ) {
	d->lastBuffer = newBuffer;
	emit buffer( newBuffer );
      }

      // device buffer for growisofs >= 7.0
      pos = line.find( "UBU", pos );
      endPos = line.find( '%', pos+5 );
      if( pos > 0 ) {
	pos += 4;
	val = line.mid( pos, endPos-pos ).toDouble( &ok );
	if( ok ) {
	  int newBuffer = (int)(val+0.5);
	  if( newBuffer != d->lastDeviceBuffer ) {
	    d->lastDeviceBuffer = newBuffer;
	    emit deviceBuffer( newBuffer );
	  }
	}
      }
    }
    else
      kdDebug() << "(K3bGrowisofsHandler) failed to parse ring buffer fill from '" << line.mid( pos, endPos-pos ) << "'" << endl;
  }

  else {
    kdDebug() << "(growisofs) " << line << endl;
  }
}


void K3bGrowisofsHandler::handleExit( int exitCode )
{
  switch( m_error ) {
  case ERROR_MEDIA:
    emit infoMessage( i18n("K3b detected a problem with the media."), K3bJob::ERROR );
    emit infoMessage( i18n("Please try another media brand, preferably one explicitly recommended by your writer's vendor."), K3bJob::ERROR );
    emit infoMessage( i18n("Report the problem if it persists anyway."), K3bJob::ERROR );
    break;

  case ERROR_OVERSIZE:
    if( k3bcore->globalSettings()->overburn() )
      emit infoMessage( i18n("Data did not fit on disk."), K3bJob::ERROR );
    else
      emit infoMessage( i18n("Data does not fit on disk."), K3bJob::ERROR );
    break;

  case ERROR_SPEED_SET_FAILED:
    emit infoMessage( i18n("Unable to set writing speed."), K3bJob::ERROR );
    emit infoMessage( i18n("Please try again with the 'ignore speed' setting."), K3bJob::ERROR );
    break;

  case ERROR_OPC:
    emit infoMessage( i18n("Optimum Power Calibration failed."), K3bJob::ERROR );
    emit infoMessage( i18n("Try adding '-use-the-force-luke=noopc' to the "
			   "growisofs user parameters in the K3b settings."), K3bJob::ERROR );
    break;

  case ERROR_MEMLOCK:
    emit infoMessage( i18n("Unable to allocate software buffer."), K3bJob::ERROR );
    emit infoMessage( i18n("This error is caused by the low memorylocked resource limit."), K3bJob::ERROR );
    emit infoMessage( i18n("It can be solved by issuing the command 'ulimit -l unlimited'..."), K3bJob::ERROR );
    emit infoMessage( i18n("...or by lowering the used software buffer size in the advanced K3b settings."), K3bJob::ERROR );
    break;

  case ERROR_WRITE_FAILED:
    emit infoMessage( i18n("Write error"), K3bJob::ERROR );
    break;

  default:

    //
    // The growisofs error codes:
    //
    // 128 + errno: fatal error upon program startup
    // errno      : fatal error during recording
    //

    if( exitCode > 128 ) {
      // for now we just emit a message with the error
      // in the future when I know more about what kinds of errors may occur
      // we will enhance this
      emit infoMessage( i18n("Fatal error at startup: %1").arg(strerror(exitCode-128)), 
			K3bJob::ERROR );
    }
    else if( exitCode == 1 ) {
      // Doku says: warning at exit
      // Example: mkisofs error
      //          unable to reload
      // So basically this is just for mkisofs failure since we do not let growisofs reload the media
      emit infoMessage( i18n("Warning at exit: (1)"), K3bJob::ERROR );
      emit infoMessage( i18n("Most likely mkisofs failed in some way."), K3bJob::ERROR );
    }
    else {
      emit infoMessage( i18n("Fatal error during recording: %1").arg(strerror(exitCode)), 
			K3bJob::ERROR );
    }
  }

  reset();
}


void K3bGrowisofsHandler::slotCheckBufferStatus()
{
  connect( K3bDevice::sendCommand( K3bDevice::DeviceHandler::BUFFER_CAPACITY, m_device ),
	   SIGNAL(finished(K3bDevice::DeviceHandler*)),
	   this,
	   SLOT(slotCheckBufferStatusDone(K3bDevice::DeviceHandler*)) );
}


void K3bGrowisofsHandler::slotCheckBufferStatusDone( K3bDevice::DeviceHandler* dh )
{
  if( dh->success() && dh->bufferCapacity() > 0 ) {
    emit deviceBuffer( 100 * (dh->bufferCapacity() - dh->availableBufferCapacity() ) / dh->bufferCapacity() );
    QTimer::singleShot( 500, this, SLOT(slotCheckBufferStatus()) );
  }
  else {
    kdDebug() << "(K3bGrowisofsHandler) stopping buffer check." << endl;
  }
}

#include "k3bgrowisofshandler.moc"
