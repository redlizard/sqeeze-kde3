/* 
 *
 * $Id: k3baudionormalizejob.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3baudionormalizejob.h"
#include <k3bexternalbinmanager.h>
#include <k3bprocess.h>
#include <k3bcore.h>

#include <kdebug.h>
#include <klocale.h>


K3bAudioNormalizeJob::K3bAudioNormalizeJob( K3bJobHandler* hdl, QObject* parent, const char* name )
  : K3bJob( hdl, parent, name ),
    m_process(0)
{
}


K3bAudioNormalizeJob::~K3bAudioNormalizeJob()
{
  if( m_process )
    delete m_process;
}


void K3bAudioNormalizeJob::start()
{
  m_canceled = false;
  m_currentAction = COMPUTING_LEVELS;
  m_currentTrack = 1;
    
  jobStarted();

  if( m_process )
    delete m_process;

  m_process = new K3bProcess();
  connect( m_process, SIGNAL(stderrLine(const QString&)), this, SLOT(slotStdLine(const QString&)) );
  connect( m_process, SIGNAL(processExited(KProcess*)), this, SLOT(slotProcessExited(KProcess*)) );

  const K3bExternalBin* bin = k3bcore->externalBinManager()->binObject( "normalize" );

  if( !bin ) {
    emit infoMessage( i18n("Could not find normalize executable."), ERROR );
    jobFinished(false);
    return;
  }

  if( !bin->copyright.isEmpty() )
    emit infoMessage( i18n("Using %1 %2 - Copyright (C) %3").arg(bin->name()).arg(bin->version).arg(bin->copyright), INFO );

  // create the commandline
  *m_process << bin;

  // additional user parameters from config
  const QStringList& params = bin->userParameters();
  for( QStringList::const_iterator it = params.begin(); it != params.end(); ++it )
    *m_process << *it;

  // end the options
  *m_process << "--";

  // add the files
  for( uint i = 0; i < m_files.count(); ++i )
    *m_process << m_files[i];

  // now start the process
  if( !m_process->start( KProcess::NotifyOnExit, KProcess::AllOutput ) ) {
    // something went wrong when starting the program
    // it "should" be the executable
    kdDebug() << "(K3bAudioNormalizeJob) could not start normalize" << endl;
    emit infoMessage( i18n("Could not start normalize."), K3bJob::ERROR );
    jobFinished(false);
  }
}


void K3bAudioNormalizeJob::cancel()
{
  m_canceled = true;

  if( m_process )
    if( m_process->isRunning() ) {
      m_process->kill();
    }
}


void K3bAudioNormalizeJob::slotStdLine( const QString& line )
{
  // percent, subPercent, newTask (compute level and adjust)

  //  emit newSubTask( i18n("Normalizing track %1 of %2 (%3)").arg(t).arg(tt).arg(m_files.at(t-1)) );

  emit debuggingOutput( "normalize", line );

  // wenn "% done" drin:
  //    wenn ein --% drin ist, so beginnt ein neuer track
  //    sonst prozent parsen "batch xxx" ist der fortschritt der action
  //                         also ev. den batch fortschritt * 1/2

  if( line.startsWith( "Applying adjustment" ) ) {
    if( m_currentAction == COMPUTING_LEVELS ) {
      // starting the adjustment with track 1
      m_currentTrack = 1;
      m_currentAction = ADJUSTING_LEVELS;
    }
  }
  
  else if( line.contains( "already normalized" ) ) {
    // no normalization necessary for the current track
    emit infoMessage( i18n("Track %1 is already normalized.").arg(m_currentTrack), INFO );
    m_currentTrack++;
  }

  else if( line.contains( "--% done") ) {
    if( m_currentAction == ADJUSTING_LEVELS ) {
      emit newTask( i18n("Adjusting volume level for track %1 of %2").arg(m_currentTrack).arg(m_files.count()) );
      kdDebug() << "(K3bAudioNormalizeJob) adjusting level for track " 
		<< m_currentTrack
		<< " "
		<< m_files.at(m_currentTrack-1)
		<< endl;
    }
    else {
      emit newTask( i18n("Computing level for track %1 of %2").arg(m_currentTrack).arg(m_files.count()) );
      kdDebug() << "(K3bAudioNormalizeJob) computing level for track " 
		<< m_currentTrack
		<< " "
		<< m_files.at(m_currentTrack-1)
		<< endl;
    }

    m_currentTrack++;
  }
  
  else if( int pos = line.find( "% done" ) > 0 ) {
    // parse progress: "XXX% done" and "batch XXX% done"
    pos -= 3;
    bool ok;
    // TODO: do not use fixed values
    // track progress starts at position 19 in version 0.7.6
    int p = line.mid( 19, 3 ).toInt(&ok);
    if( ok )
      emit subPercent( p );
    else
      kdDebug() << "(K3bAudioNormalizeJob) subPercent parsing error at pos " 
		<< 19 << " in line '" << line.mid( 19, 3 ) << "'" << endl;

    // batch progress starts at position 50 in version 0.7.6
    p = line.mid( 50, 3 ).toInt(&ok);
    if( ok && m_currentAction == COMPUTING_LEVELS )
      emit percent( (int)((double)p/2.0) );
    else if( ok && m_currentAction == ADJUSTING_LEVELS )
      emit percent( 50 + (int)((double)p/2.0) );
    else
      kdDebug() << "(K3bAudioNormalizeJob) percent parsing error at pos " 
		<< 50 << " in line '" << line.mid( 50, 3 ) << "'" << endl;

  }
}


void K3bAudioNormalizeJob::slotProcessExited( KProcess* p )
{
  if( p->normalExit() ) {
    switch( p->exitStatus() ) {
    case 0:
      emit infoMessage( i18n("Successfully normalized all tracks."), SUCCESS );
      jobFinished(true);
      break;
    default:
      if( !m_canceled ) {
	emit infoMessage( i18n("%1 returned an unknown error (code %2).").arg("normalize").arg(p->exitStatus()), 
			  K3bJob::ERROR );
	emit infoMessage( i18n("Please send me an email with the last output."), K3bJob::ERROR );
	emit infoMessage( i18n("Error while normalizing tracks."), ERROR );
      }
      else
	emit canceled();
      jobFinished(false);
      break;
    }
  }
  else {
    emit infoMessage( i18n("%1 did not exit cleanly.").arg("Normalize"), K3bJob::ERROR );
    jobFinished( false );
  }
}

#include "k3baudionormalizejob.moc"
