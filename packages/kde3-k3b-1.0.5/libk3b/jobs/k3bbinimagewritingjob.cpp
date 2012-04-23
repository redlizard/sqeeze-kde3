/*
 *
 * $Id: k3bbinimagewritingjob.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Klaus-Dieter Krannich <kd@k3b.org>
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


#include "k3bbinimagewritingjob.h"
#include <k3bcdrecordwriter.h>
#include <k3bcdrdaowriter.h>
#include <k3bcore.h>
#include <k3bdevice.h>
#include <k3bglobals.h>
#include <k3bexternalbinmanager.h>

#include <klocale.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>



K3bBinImageWritingJob::K3bBinImageWritingJob( K3bJobHandler* hdl, QObject* parent )
  : K3bBurnJob( hdl, parent ),
    m_device(0),
    m_simulate(false),
    m_force(false),
    m_noFix(false),
    m_tocFile(0),
    m_speed(2),
    m_copies(1),
    m_writer(0)
{
}

K3bBinImageWritingJob::~K3bBinImageWritingJob()
{
}

void K3bBinImageWritingJob::start()
{
  m_canceled =  false;

  if( m_copies < 1 )
    m_copies = 1;
  m_finishedCopies = 0;

  jobStarted();
  emit newTask( i18n("Write Binary Image") );

  if( prepareWriter() )
    writerStart();
  else
    cancel();

}

void K3bBinImageWritingJob::cancel()
{
  m_canceled = true;
  m_writer->cancel();
  emit canceled();
  jobFinished( false );
}

bool K3bBinImageWritingJob::prepareWriter()
{
  if( m_writer )
    delete m_writer;

  int usedWritingApp = writingApp();
  const K3bExternalBin* cdrecordBin = k3bcore->externalBinManager()->binObject("cdrecord");
  if( usedWritingApp == K3b::CDRECORD || 
      ( usedWritingApp == K3b::DEFAULT && cdrecordBin && cdrecordBin->hasFeature("cuefile") && m_device->dao() ) ) {
    usedWritingApp = K3b::CDRECORD;

    // IMPROVEME: check if it's a cdrdao toc-file
    if( m_tocFile.right(4) == ".toc" ) {
      kdDebug() << "(K3bBinImageWritingJob) imagefile has ending toc." << endl;
      usedWritingApp = K3b::CDRDAO;
    }
    else {
      // TODO: put this into K3bCueFileParser
      // TODO: check K3bCueFileParser::imageFilenameInCue()
      // let's see if cdrecord can handle the cue file
      QFile f( m_tocFile );
      if( f.open( IO_ReadOnly ) ) {
	QTextStream fStr( &f );
	if( fStr.read().contains( "MODE1/2352" ) ) {
	  kdDebug() << "(K3bBinImageWritingJob) cuefile contains MODE1/2352 track. using cdrdao." << endl;
	  usedWritingApp = K3b::CDRDAO;
	}
	f.close();
      }
      else
	kdDebug() << "(K3bBinImageWritingJob) could not open file " << m_tocFile << endl;
    }
  }
  else
    usedWritingApp = K3b::CDRDAO;

  if( usedWritingApp == K3b::CDRECORD ) {
    // create cdrecord job
    K3bCdrecordWriter* writer = new K3bCdrecordWriter( m_device, this );

    writer->setDao( true );
    writer->setSimulate( m_simulate );
    writer->setBurnSpeed( m_speed );
    writer->setCueFile ( m_tocFile );

    if( m_noFix ) {
      writer->addArgument("-multi");
    }

    if( m_force ) {
      writer->addArgument("-force");
    }

    m_writer = writer;
  }
  else {
    // create cdrdao job
    K3bCdrdaoWriter* writer = new K3bCdrdaoWriter( m_device, this );
    writer->setCommand( K3bCdrdaoWriter::WRITE );
    writer->setSimulate( m_simulate );
    writer->setBurnSpeed( m_speed );
    writer->setForce( m_force );

    // multisession
    writer->setMulti( m_noFix );

    writer->setTocFile( m_tocFile );

    m_writer = writer;
  }

  connect( m_writer, SIGNAL(infoMessage(const QString&, int)), this, SIGNAL(infoMessage(const QString&, int)) );
  connect( m_writer, SIGNAL(percent(int)), this, SLOT(copyPercent(int)) );
  connect( m_writer, SIGNAL(subPercent(int)), this, SLOT(copySubPercent(int)) );
  connect( m_writer, SIGNAL(processedSize(int, int)), this, SIGNAL(processedSize(int, int)) );
  connect( m_writer, SIGNAL(buffer(int)), this, SIGNAL(bufferStatus(int)) );
  connect( m_writer, SIGNAL(deviceBuffer(int)), this, SIGNAL(deviceBuffer(int)) );
  connect( m_writer, SIGNAL(writeSpeed(int, int)), this, SIGNAL(writeSpeed(int, int)) );
  connect( m_writer, SIGNAL(finished(bool)), this, SLOT(writerFinished(bool)) );
  connect( m_writer, SIGNAL(newTask(const QString&)), this, SIGNAL(newTask(const QString&)) );
  connect( m_writer, SIGNAL(newSubTask(const QString&)), this, SIGNAL(newSubTask(const QString&)) );
  connect( m_writer, SIGNAL(nextTrack(int, int)), this, SLOT(slotNextTrack(int, int)) );
  connect( m_writer, SIGNAL(debuggingOutput(const QString&, const QString&)), this, SIGNAL(debuggingOutput(const QString&, const QString&)) );

  return true;
}


void K3bBinImageWritingJob::writerStart()
{

  if( waitForMedia( m_device ) < 0 ) {
    cancel();
  }
  // just to be sure we did not get canceled during the async discWaiting
  else if( !m_canceled ) {
    emit burning(true);
    m_writer->start();
  }
}

void K3bBinImageWritingJob::copyPercent(int p)
{
  emit percent( (100*m_finishedCopies + p)/m_copies );
}

void K3bBinImageWritingJob::copySubPercent(int p)
{
  emit subPercent(p);
}

void K3bBinImageWritingJob::writerFinished(bool ok)
{
  if( m_canceled )
    return;

  if (ok) {
    m_finishedCopies++;
    if ( m_finishedCopies == m_copies ) {
      emit infoMessage( i18n("%n copy successfully created", "%n copies successfully created", m_copies),K3bJob::INFO );
      jobFinished( true );
    }
    else {
      writerStart();
    }
  }
  else {
    jobFinished(false);
  }
}


void K3bBinImageWritingJob::slotNextTrack( int t, int tt )
{
  emit newSubTask( i18n("Writing track %1 of %2").arg(t).arg(tt) );
}


QString K3bBinImageWritingJob::jobDescription() const
{
  return ( i18n("Writing cue/bin Image")
	   + ( m_copies > 1 
	       ? i18n(" - %n Copy", " - %n Copies", m_copies) 
	       : QString::null ) );
}


QString K3bBinImageWritingJob::jobDetails() const
{
  return m_tocFile.section("/", -1);
}


void K3bBinImageWritingJob::setTocFile(const QString& s)
{
  m_tocFile = s;
}

#include "k3bbinimagewritingjob.moc"
