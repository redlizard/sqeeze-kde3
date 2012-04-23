/*
 *
 * $Id: k3bmixedjob.cpp 690212 2007-07-20 11:02:13Z trueg $
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




#include "k3bmixedjob.h"
#include "k3bmixeddoc.h"

#include <k3bdatadoc.h>
#include <k3bisoimager.h>
#include <k3bmsinfofetcher.h>
#include <k3baudioimager.h>
#include <k3baudiodoc.h>
#include <k3baudiotrack.h>
#include <k3baudionormalizejob.h>
#include <k3baudiojobtempdata.h>
#include <k3baudiomaxspeedjob.h>
#include <k3bdevicemanager.h>
#include <k3bdevice.h>
#include <k3bdevicehandler.h>
#include <k3bmsf.h>
#include <k3bglobals.h>
#include <k3bexternalbinmanager.h>
#include <k3bversion.h>
#include <k3bcore.h>
#include <k3bcdrecordwriter.h>
#include <k3bcdrdaowriter.h>
#include <k3btocfilewriter.h>
#include <k3binffilewriter.h>
#include <k3bglobalsettings.h>
#include <k3baudiofile.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kstringhandler.h>


static QString createNonExistingFilesString( const QValueList<K3bAudioFile*>& items, unsigned int max )
{
  QString s;
  unsigned int cnt = 0;
  for( QValueList<K3bAudioFile*>::const_iterator it = items.begin();
       it != items.end(); ++it ) {

    s += KStringHandler::csqueeze( (*it)->filename(), 60 );

    ++cnt;
    if( cnt >= max || it == items.end() )
      break;

    s += "<br>";
  }

  if( items.count() > max )
    s += "...";

  return s;
}



class K3bMixedJob::Private
{
public:
  Private()
    : maxSpeedJob(0) {
  }


  int copies;
  int copiesDone;

  K3bAudioMaxSpeedJob* maxSpeedJob;
  bool maxSpeed;
};


K3bMixedJob::K3bMixedJob( K3bMixedDoc* doc, K3bJobHandler* hdl, QObject* parent )
  : K3bBurnJob( hdl, parent ),
    m_doc( doc ),
    m_normalizeJob(0)
{
  d = new Private;

  m_isoImager = new K3bIsoImager( doc->dataDoc(), this, this );
  connect( m_isoImager, SIGNAL(infoMessage(const QString&, int)), this, SIGNAL(infoMessage(const QString&, int)) );
  connect( m_isoImager, SIGNAL(percent(int)), this, SLOT(slotIsoImagerPercent(int)) );
  connect( m_isoImager, SIGNAL(finished(bool)), this, SLOT(slotIsoImagerFinished(bool)) );
  connect( m_isoImager, SIGNAL(debuggingOutput(const QString&, const QString&)),
	   this, SIGNAL(debuggingOutput(const QString&, const QString&)) );

  m_audioImager = new K3bAudioImager( doc->audioDoc(), this, this );
  connect( m_audioImager, SIGNAL(infoMessage(const QString&, int)),
	   this, SIGNAL(infoMessage(const QString&, int)) );
  connect( m_audioImager, SIGNAL(percent(int)), this, SLOT(slotAudioDecoderPercent(int)) );
  connect( m_audioImager, SIGNAL(subPercent(int)), this, SLOT(slotAudioDecoderSubPercent(int)) );
  connect( m_audioImager, SIGNAL(finished(bool)), this, SLOT(slotAudioDecoderFinished(bool)) );
  connect( m_audioImager, SIGNAL(nextTrack(int, int)), this, SLOT(slotAudioDecoderNextTrack(int, int)) );

  m_msInfoFetcher = new K3bMsInfoFetcher( this, this );
  connect( m_msInfoFetcher, SIGNAL(finished(bool)), this, SLOT(slotMsInfoFetched(bool)) );
  connect( m_msInfoFetcher, SIGNAL(infoMessage(const QString&, int)), this, SIGNAL(infoMessage(const QString&, int)) );

  m_writer = 0;
  m_tocFile = 0;
  m_tempData = new K3bAudioJobTempData( m_doc->audioDoc(), this );
}


K3bMixedJob::~K3bMixedJob()
{
  delete m_tocFile;
  delete d;
}


K3bDevice::Device* K3bMixedJob::writer() const
{
  if( m_doc->onlyCreateImages() )
    return 0;
  else
    return m_doc->burner();
}


K3bDoc* K3bMixedJob::doc() const
{
  return m_doc;
}


void K3bMixedJob::start()
{
  jobStarted();

  m_canceled = false;
  m_errorOccuredAndAlreadyReported = false;
  d->copiesDone = 0;
  d->copies = m_doc->copies();
  m_currentAction = PREPARING_DATA;
  d->maxSpeed = false;

  if( m_doc->dummy() )
    d->copies = 1;

  prepareProgressInformation();

  //
  // Check if all files exist
  //
  QValueList<K3bAudioFile*> nonExistingFiles;
  K3bAudioTrack* track = m_doc->audioDoc()->firstTrack();
  while( track ) {
    K3bAudioDataSource* source = track->firstSource();
    while( source ) {
      if( K3bAudioFile* file = dynamic_cast<K3bAudioFile*>( source ) ) {
	if( !QFile::exists( file->filename() ) )
	  nonExistingFiles.append( file );
      }
      source = source->next();
    }
    track = track->next();
  }
  if( !nonExistingFiles.isEmpty() ) {
    if( questionYesNo( "<p>" + i18n("The following files could not be found. Do you want to remove them from the "
				    "project and continue without adding them to the image?") +
		       "<p>" + createNonExistingFilesString( nonExistingFiles, 10 ),
		       i18n("Warning"),
		       i18n("Remove missing files and continue"),
		       i18n("Cancel and go back") ) ) {
      for( QValueList<K3bAudioFile*>::const_iterator it = nonExistingFiles.begin();
	   it != nonExistingFiles.end(); ++it ) {
	delete *it;
      }
    }
    else {
      m_canceled = true;
      emit canceled();
      jobFinished(false);
      return;
    }
  }

  //
  // Make sure the project is not empty
  //
  if( m_doc->audioDoc()->numOfTracks() == 0 ) {
    emit infoMessage( i18n("Please add files to your project first."), ERROR );
    jobFinished(false);
    return;
  }


  // set some flags that are needed
  m_doc->audioDoc()->setOnTheFly( m_doc->onTheFly() );  // for the toc writer
  m_doc->audioDoc()->setHideFirstTrack( false );   // unsupported
  m_doc->dataDoc()->setBurner( m_doc->burner() );  // so the isoImager can read ms data

  emit newTask( i18n("Preparing data") );

  determineWritingMode();

  //
  // First we make sure the data portion is valid
  //

  // we do not have msinfo yet
  m_currentAction = INITIALIZING_IMAGER;
  m_isoImager->setMultiSessionInfo( QString::null );
  m_isoImager->init();
}


void K3bMixedJob::startFirstCopy()
{
  //
  // if not onthefly create the iso image and then the wavs
  // and write then
  // if onthefly calculate the iso size
  //
  if( m_doc->onTheFly() ) {
    if( m_doc->speed() == 0 ) {
      emit newSubTask( i18n("Determining maximum writing speed") );

      //
      // try to determine the max possible speed
      // no need to check the data track's max speed. Most current systems are able
      // to handle the maxium possible
      //
      if( !d->maxSpeedJob ) {
	// the maxspeed job gets the device from the doc:
	m_doc->audioDoc()->setBurner( m_doc->burner() );
	d->maxSpeedJob = new K3bAudioMaxSpeedJob( m_doc->audioDoc(), this, this );
	connect( d->maxSpeedJob, SIGNAL(percent(int)),
		 this, SIGNAL(subPercent(int)) );
	connect( d->maxSpeedJob, SIGNAL(finished(bool)),
		 this, SLOT(slotMaxSpeedJobFinished(bool)) );
      }
      d->maxSpeedJob->start();
    }
    else if( m_doc->mixedType() != K3bMixedDoc::DATA_SECOND_SESSION ) {
      m_currentAction = PREPARING_DATA;
      m_isoImager->calculateSize();
    }
    else {
      // we cannot calculate the size since we don't have the msinfo yet
      // so first write the audio session
      writeNextCopy();
    }
  }
  else {
    emit burning(false);

    emit infoMessage( i18n("Creating audio image files in %1").arg(m_doc->tempDir()), INFO );

    m_tempFilePrefix = K3b::findUniqueFilePrefix( ( !m_doc->audioDoc()->title().isEmpty()
						    ? m_doc->audioDoc()->title()
						    : m_doc->dataDoc()->isoOptions().volumeID() ),
						  m_doc->tempDir() );

    m_tempData->prepareTempFileNames( m_doc->tempDir() );
    QStringList filenames;
    for( K3bAudioTrack* track = m_doc->audioDoc()->firstTrack(); track; track = track->next() )
      filenames += m_tempData->bufferFileName( track );
    m_audioImager->setImageFilenames( filenames );

    if( m_doc->mixedType() != K3bMixedDoc::DATA_SECOND_SESSION ) {
      createIsoImage();
    }
    else {
      emit newTask( i18n("Creating audio image files") );
      m_currentAction = CREATING_AUDIO_IMAGE;
      m_audioImager->start();
    }
  }
}


void K3bMixedJob::slotMaxSpeedJobFinished( bool success )
{
  d->maxSpeed = success;
  if( !success )
    emit infoMessage( i18n("Unable to determine maximum speed for some reason. Ignoring."), WARNING );

  if( m_doc->mixedType() != K3bMixedDoc::DATA_SECOND_SESSION ) {
    m_currentAction = PREPARING_DATA;
    m_isoImager->calculateSize();
  }
  else {
    // we cannot calculate the size since we don't have the msinfo yet
    // so first write the audio session
    writeNextCopy();
  }
}


void K3bMixedJob::writeNextCopy()
{
  if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
    m_currentAction = WRITING_AUDIO_IMAGE;
    if( !prepareWriter() || !startWriting() ) {
      cleanupAfterError();
      jobFinished(false);
    }
    else if( m_doc->onTheFly() )
      m_audioImager->start();
  }
  else {
    // the prepareWriter method needs the action to be set
    if( m_doc->mixedType() == K3bMixedDoc::DATA_LAST_TRACK )
      m_currentAction = WRITING_AUDIO_IMAGE;
    else
      m_currentAction = WRITING_ISO_IMAGE;

    if( !prepareWriter() || !startWriting() ) {
      cleanupAfterError();
      jobFinished(false);
    }
    else if( m_doc->onTheFly() ) {
      if( m_doc->mixedType() == K3bMixedDoc::DATA_LAST_TRACK )
	m_audioImager->start();
      else
	m_isoImager->start();
    }
  }
}


void K3bMixedJob::cancel()
{
  m_canceled = true;

  if( d->maxSpeedJob )
    d->maxSpeedJob->cancel();

  if( m_writer )
    m_writer->cancel();
  m_isoImager->cancel();
  m_audioImager->cancel();
  m_msInfoFetcher->cancel();
  emit infoMessage( i18n("Writing canceled."), K3bJob::ERROR );
  removeBufferFiles();
  emit canceled();
  jobFinished(false);
}


void K3bMixedJob::slotMsInfoFetched( bool success )
{
  if( m_canceled || m_errorOccuredAndAlreadyReported )
    return;

  if( success ) {
    if( m_usedDataWritingApp == K3b::CDRECORD )
      m_isoImager->setMultiSessionInfo( m_msInfoFetcher->msInfo() );
    else  // cdrdao seems to write a 150 blocks pregap that is not used by cdrecord
      m_isoImager->setMultiSessionInfo( QString("%1,%2")
					.arg(m_msInfoFetcher->lastSessionStart())
					.arg(m_msInfoFetcher->nextSessionStart()+150) );

    if( m_doc->onTheFly() ) {
      m_currentAction = PREPARING_DATA;
      m_isoImager->calculateSize();
    }
    else {
      createIsoImage();
    }
  }
  else {
    // the MsInfoFetcher already emitted failure info
    cleanupAfterError();
    jobFinished(false);
  }
}


void K3bMixedJob::slotIsoImagerFinished( bool success )
{
  if( m_canceled || m_errorOccuredAndAlreadyReported )
    return;

  //
  // Initializing imager before the first copy
  //
  if( m_currentAction == INITIALIZING_IMAGER ) {
    if( success ) {
      m_currentAction = PREPARING_DATA;

      // check the size
      m_projectSize  = m_isoImager->size() + m_doc->audioDoc()->length();
      if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION )
	m_projectSize += 11400; // the session gap

      startFirstCopy();
    }
    else {
      cleanupAfterError();
      jobFinished( false );
    }
  }

  //
  // Recalculated iso image size
  //
  else if( m_currentAction == PREPARING_DATA ) {
    if( success ) {
      // 1. data in first track:
      //    start isoimager and writer
      //    when isoimager finishes start audiodecoder

      // 2. data in last track
      //    start audiodecoder and writer
      //    when audiodecoder finishes start isoimager

      // 3. data in second session
      //    start audiodecoder and writer
      //    start isoimager and writer

      if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
	m_currentAction = WRITING_ISO_IMAGE;
	if( !prepareWriter() || !startWriting() ) {
	  cleanupAfterError();
	  jobFinished(false);
	}
	else
	  m_isoImager->start();
      }
      else
	writeNextCopy();
    }
    else {
      cleanupAfterError();
      jobFinished( false );
    }
  }

  //
  // Image creation finished
  //
  else {
    if( !success ) {
      emit infoMessage( i18n("Error while creating ISO image."), ERROR );
      cleanupAfterError();

      jobFinished( false );
      return;
    }

    if( m_doc->onTheFly() ) {
      if( m_doc->mixedType() == K3bMixedDoc::DATA_FIRST_TRACK ) {
	m_currentAction = WRITING_AUDIO_IMAGE;
	m_audioImager->start();
      }
    }
    else {
      emit infoMessage( i18n("ISO image successfully created."), SUCCESS );

      if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
	m_currentAction = WRITING_ISO_IMAGE;

	if( !prepareWriter() || !startWriting() ) {
	  cleanupAfterError();
	  jobFinished(false);
	}
      }
      else {
	emit newTask( i18n("Creating audio image files") );
	m_currentAction = CREATING_AUDIO_IMAGE;
	m_audioImager->start();
      }
    }
  }
}


void K3bMixedJob::slotWriterFinished( bool success )
{
  if( m_canceled || m_errorOccuredAndAlreadyReported )
    return;

  if( !success ) {
    cleanupAfterError();
    jobFinished(false);
    return;
  }

  emit burning(false);

  if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION && m_currentAction == WRITING_AUDIO_IMAGE ) {
    // reload the media (as a subtask so the user does not see the "Flushing cache" or "Fixating" messages while
    // doing so
    emit newSubTask( i18n("Reloading the medium") );
    connect( K3bDevice::reload( m_doc->burner() ), SIGNAL(finished(bool)),
	     this, SLOT(slotMediaReloadedForSecondSession(bool)) );
  }
  else {
    d->copiesDone++;
    if( d->copiesDone < d->copies ) {
      K3bDevice::eject( m_doc->burner() );
      writeNextCopy();
    }
    else {
      if( !m_doc->onTheFly() && m_doc->removeImages() )
	removeBufferFiles();

      if( k3bcore->globalSettings()->ejectMedia() )
          K3bDevice::eject( m_doc->burner() );

      jobFinished(true);
    }
  }
}


void K3bMixedJob::slotMediaReloadedForSecondSession( bool success )
{
  if( !success )
    blockingInformation( i18n("Please reload the medium and press 'ok'"),
			 i18n("Unable to close the tray") );

  // start the next session
  m_currentAction = WRITING_ISO_IMAGE;
  if( d->copiesDone > 0 ) {
    // we only create the image once. This should not be a problem???
    if( !prepareWriter() || !startWriting() ) {
      cleanupAfterError();
      jobFinished(false);
    }
    else if( m_doc->onTheFly() ) {
      m_isoImager->start();
    }
  }
  else if( m_doc->dummy() ) {
    // do not try to get ms info in simulation mode since the cd is empty!
    if( m_doc->onTheFly() ) {
      m_currentAction = PREPARING_DATA;
      m_isoImager->calculateSize();
    }
    else
      createIsoImage();
  }
  else {
    m_currentAction = FETCHING_MSINFO;
    m_msInfoFetcher->setDevice( m_doc->burner() );
    m_msInfoFetcher->start();
  }
}


void K3bMixedJob::slotAudioDecoderFinished( bool success )
{
  if( m_canceled || m_errorOccuredAndAlreadyReported )
    return;

  if( !success ) {
    emit infoMessage( i18n("Error while decoding audio tracks."), ERROR );
    cleanupAfterError();
    jobFinished(false);
    return;
  }

  if( m_doc->onTheFly() ) {
    if( m_doc->mixedType() == K3bMixedDoc::DATA_LAST_TRACK ) {
      m_currentAction = WRITING_ISO_IMAGE;
      m_isoImager->start();
    }
  }
  else {
    emit infoMessage( i18n("Audio images successfully created."), SUCCESS );

    if( m_doc->audioDoc()->normalize() ) {
      normalizeFiles();
    }
    else {
      if( m_doc->mixedType() == K3bMixedDoc::DATA_FIRST_TRACK )
	m_currentAction = WRITING_ISO_IMAGE;
      else
	m_currentAction = WRITING_AUDIO_IMAGE;

      if( !prepareWriter() || !startWriting() ) {
	cleanupAfterError();
	jobFinished(false);
      }
    }
  }
}


void K3bMixedJob::slotAudioDecoderNextTrack( int t, int tt )
{
  if( m_doc->onlyCreateImages() || !m_doc->onTheFly() ) {
    K3bAudioTrack* track = m_doc->audioDoc()->getTrack(t);
    emit newSubTask( i18n("Decoding audio track %1 of %2%3")
		     .arg(t)
		     .arg(tt)
		     .arg( track->title().isEmpty() || track->artist().isEmpty()
			   ? QString::null
			   : " (" + track->artist() + " - " + track->title() + ")" ) );
  }
}


bool K3bMixedJob::prepareWriter()
{
  if( m_writer ) delete m_writer;

  if( ( m_currentAction == WRITING_ISO_IMAGE && m_usedDataWritingApp == K3b::CDRECORD ) ||
      ( m_currentAction == WRITING_AUDIO_IMAGE && m_usedAudioWritingApp == K3b::CDRECORD ) )  {

    if( !writeInfFiles() ) {
      kdDebug() << "(K3bMixedJob) could not write inf-files." << endl;
      emit infoMessage( i18n("IO Error"), ERROR );

      return false;
    }

    K3bCdrecordWriter* writer = new K3bCdrecordWriter( m_doc->burner(), this, this );

    // only write the audio tracks in DAO mode
    if( m_currentAction == WRITING_ISO_IMAGE )
      writer->setWritingMode( m_usedDataWritingMode );
    else
      writer->setWritingMode( m_usedAudioWritingMode );

    writer->setSimulate( m_doc->dummy() );
    writer->setBurnSpeed( m_doc->speed() );

    if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
      if( m_currentAction == WRITING_ISO_IMAGE ) {
	if( m_doc->onTheFly() )
	  writer->addArgument("-waiti");

	addDataTrack( writer );
      }
      else {
	writer->addArgument("-multi");
	addAudioTracks( writer );
      }
    }
    else {
      if( m_doc->mixedType() == K3bMixedDoc::DATA_FIRST_TRACK )
	addDataTrack( writer );
      addAudioTracks( writer );
      if( m_doc->mixedType() == K3bMixedDoc::DATA_LAST_TRACK )
	addDataTrack( writer );
    }

    m_writer = writer;
  }
  else {
    if( !writeTocFile() ) {
      kdDebug() << "(K3bDataJob) could not write tocfile." << endl;
      emit infoMessage( i18n("IO Error"), ERROR );

      return false;
    }

    // create the writer
    // create cdrdao job
    K3bCdrdaoWriter* writer = new K3bCdrdaoWriter( m_doc->burner(), this, this );
    writer->setSimulate( m_doc->dummy() );
    writer->setBurnSpeed( m_doc->speed() );

    // multisession only for the first session
    writer->setMulti( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION
		      && m_currentAction == WRITING_AUDIO_IMAGE );

    writer->setTocFile( m_tocFile->name() );

    m_writer = writer;
  }

  connect( m_writer, SIGNAL(infoMessage(const QString&, int)), this, SIGNAL(infoMessage(const QString&, int)) );
  connect( m_writer, SIGNAL(percent(int)), this, SLOT(slotWriterJobPercent(int)) );
  connect( m_writer, SIGNAL(processedSize(int, int)), this, SIGNAL(processedSize(int, int)) );
  connect( m_writer, SIGNAL(subPercent(int)), this, SIGNAL(subPercent(int)) );
  connect( m_writer, SIGNAL(processedSubSize(int, int)), this, SIGNAL(processedSubSize(int, int)) );
  connect( m_writer, SIGNAL(nextTrack(int, int)), this, SLOT(slotWriterNextTrack(int, int)) );
  connect( m_writer, SIGNAL(buffer(int)), this, SIGNAL(bufferStatus(int)) );
  connect( m_writer, SIGNAL(deviceBuffer(int)), this, SIGNAL(deviceBuffer(int)) );
  connect( m_writer, SIGNAL(writeSpeed(int, int)), this, SIGNAL(writeSpeed(int, int)) );
  connect( m_writer, SIGNAL(finished(bool)), this, SLOT(slotWriterFinished(bool)) );
  //  connect( m_writer, SIGNAL(newTask(const QString&)), this, SIGNAL(newTask(const QString&)) );
  connect( m_writer, SIGNAL(newSubTask(const QString&)), this, SIGNAL(newSubTask(const QString&)) );
  connect( m_writer, SIGNAL(debuggingOutput(const QString&, const QString&)),
	   this, SIGNAL(debuggingOutput(const QString&, const QString&)) );

  return true;
}


bool K3bMixedJob::writeInfFiles()
{
  K3bInfFileWriter infFileWriter;
  K3bAudioTrack* track = m_doc->audioDoc()->firstTrack();
  while( track ) {

    infFileWriter.setTrack( track->toCdTrack() );
    infFileWriter.setTrackNumber( track->trackNumber() );
    if( !m_doc->onTheFly() )
      infFileWriter.setBigEndian( false );

    if( !infFileWriter.save( m_tempData->infFileName(track) ) )
      return false;

    track = track->next();
  }
  return true;
}


bool K3bMixedJob::writeTocFile()
{
  // FIXME: create the tocfile in the same directory like all the other files.

  if( m_tocFile ) delete m_tocFile;
  m_tocFile = new KTempFile( QString::null, "toc" );
  m_tocFile->setAutoDelete(true);

  // write the toc-file
  if( QTextStream* s = m_tocFile->textStream() ) {

    K3bTocFileWriter tocFileWriter;

    //
    // TOC
    //
    tocFileWriter.setData( m_doc->toToc( m_usedDataMode == K3b::MODE2
					 ? K3bDevice::Track::XA_FORM1
					 : K3bDevice::Track::MODE1,
					 m_doc->onTheFly()
					 ? m_isoImager->size()
					 : m_doc->dataDoc()->length() ) );

    //
    // CD-Text
    //
    if( m_doc->audioDoc()->cdText() ) {
      K3bDevice::CdText text = m_doc->audioDoc()->cdTextData();
      // if data in first track we need to add a dummy cdtext
      if( m_doc->mixedType() == K3bMixedDoc::DATA_FIRST_TRACK )
	text.insert( text.begin(), K3bDevice::TrackCdText() );

      tocFileWriter.setCdText( text );
    }

    //
    // Session to write
    //
    tocFileWriter.setSession( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION &&
			      m_currentAction == WRITING_ISO_IMAGE ? 2 : 1 );

    //
    // image filenames
    //
    if( !m_doc->onTheFly() ) {
      QStringList files;
      K3bAudioTrack* track = m_doc->audioDoc()->firstTrack();
      while( track ) {
	files += m_tempData->bufferFileName( track );
	track = track->next();
      }
      if( m_doc->mixedType() == K3bMixedDoc::DATA_FIRST_TRACK )
	files.prepend( m_isoImageFilePath );
      else
	files.append( m_isoImageFilePath );

      tocFileWriter.setFilenames( files );
    }

    bool success = tocFileWriter.save( *s );

    m_tocFile->close();

    // backup for debugging
//    KIO::NetAccess::del("/tmp/trueg/tocfile_debug_backup.toc",0L);
//    KIO::NetAccess::copy( m_tocFile->name(), "/tmp/trueg/tocfile_debug_backup.toc",0L );

    return success;
  }
  else
    return false;
}


void K3bMixedJob::addAudioTracks( K3bCdrecordWriter* writer )
{
  writer->addArgument( "-useinfo" );

  // add raw cdtext data
  if( m_doc->audioDoc()->cdText() ) {
    writer->setRawCdText( m_doc->audioDoc()->cdTextData().rawPackData() );
  }

  writer->addArgument( "-audio" );

  // we always pad because although K3b makes sure all tracks' length are multiples of 2352
  // it seems that normalize sometimes corrupts these lengths
  // FIXME: see K3bAudioJob for the whole less4secs and zeroPregap handling
  writer->addArgument( "-pad" );

  // Allow tracks shorter than 4 seconds
  writer->addArgument( "-shorttrack" );

  // add all the audio tracks
  K3bAudioTrack* track = m_doc->audioDoc()->firstTrack();
  while( track ) {
    if( m_doc->onTheFly() ) {
      // this is only supported by cdrecord versions >= 2.01a13
      writer->addArgument( QFile::encodeName( m_tempData->infFileName( track ) ) );
    }
    else {
      writer->addArgument( QFile::encodeName( m_tempData->bufferFileName( track ) ) );
    }
    track = track->next();
  }
}

void K3bMixedJob::addDataTrack( K3bCdrecordWriter* writer )
{
  // add data track
  if(  m_usedDataMode == K3b::MODE2 ) {
    if( k3bcore->externalBinManager()->binObject("cdrecord") &&
	k3bcore->externalBinManager()->binObject("cdrecord")->hasFeature( "xamix" ) )
      writer->addArgument( "-xa" );
    else
      writer->addArgument( "-xa1" );
  }
  else
    writer->addArgument( "-data" );

  if( m_doc->onTheFly() )
    writer->addArgument( QString("-tsize=%1s").arg(m_isoImager->size()) )->addArgument("-");
  else
    writer->addArgument( m_isoImageFilePath );
}


void K3bMixedJob::slotWriterNextTrack( int t, int )
{
  K3bAudioTrack* track = 0;

  if( m_doc->mixedType() == K3bMixedDoc::DATA_FIRST_TRACK ) {
    if( t > 1 )
      track = m_doc->audioDoc()->getTrack(t-1);
  }
  else if( m_doc->mixedType() == K3bMixedDoc::DATA_LAST_TRACK ) {
    if( t < m_doc->audioDoc()->numOfTracks()+1 )
      track = m_doc->audioDoc()->getTrack(t);
  }
  else if( m_currentAction == WRITING_AUDIO_IMAGE )
    track = m_doc->audioDoc()->getTrack(t);
  else
    t = m_doc->numOfTracks();

  if( track )
    emit newSubTask( i18n("Writing track %1 of %2%3")
		     .arg(t)
		     .arg(m_doc->numOfTracks())
		     .arg( track->title().isEmpty() || track->artist().isEmpty()
			   ? QString::null
			   : " (" + track->artist() + " - " + track->title() + ")" ) );
  else
    emit newSubTask( i18n("Writing track %1 of %2 (%3)").arg(t).arg(m_doc->numOfTracks()).arg(i18n("ISO9660 data")) );
}


void K3bMixedJob::slotWriterJobPercent( int p )
{
  double totalTasks = d->copies;
  double tasksDone = d->copiesDone;
  if( m_doc->audioDoc()->normalize() ) {
    totalTasks+=1.0;
    tasksDone+=1.0;
  }
  if( !m_doc->onTheFly() ) {
    totalTasks+=1.0;
  }

  if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
    if( m_currentAction == WRITING_AUDIO_IMAGE ) {
      // the audio imager has finished in all cases
      // the iso imager only if this is not the first copy
      if( d->copiesDone > 0 )
	tasksDone += 1.0;
      else if( !m_doc->onTheFly() )
	tasksDone += m_audioDocPartOfProcess;

      p = (int)((double)p*m_audioDocPartOfProcess);
    }
    else {
      // all images have been created
      if( !m_doc->onTheFly() )
	tasksDone += 1.0;

      p = (int)(100.0*m_audioDocPartOfProcess + (double)p*(1.0-m_audioDocPartOfProcess));
    }
  }
  else if( !m_doc->onTheFly() )
    tasksDone += 1.0;

  emit percent( (int)((100.0*tasksDone + (double)p) / totalTasks) );
}


void K3bMixedJob::slotAudioDecoderPercent( int p )
{
  // the only thing finished here might be the isoimager which is part of this task
  if( !m_doc->onTheFly() ) {
    double totalTasks = d->copies+1;
    if( m_doc->audioDoc()->normalize() )
      totalTasks+=1.0;

    if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION )
      p = (int)((double)p*m_audioDocPartOfProcess);
    else
      p = (int)(100.0*(1.0-m_audioDocPartOfProcess) + (double)p*m_audioDocPartOfProcess);

    emit percent( (int)((double)p / totalTasks) );
  }
}


void K3bMixedJob::slotAudioDecoderSubPercent( int p )
{
  if( !m_doc->onTheFly() ) {
    emit subPercent( p );
  }
}


void K3bMixedJob::slotIsoImagerPercent( int p )
{
  if( !m_doc->onTheFly() ) {
    emit subPercent( p );
    if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {

      double totalTasks = d->copies+1.0;
      double tasksDone = d->copiesDone;
      if( m_doc->audioDoc()->normalize() ) {
	totalTasks+=1.0;
	// the normalizer finished
	tasksDone+=1.0;
      }

      // the writing of the audio part finished
      tasksDone += m_audioDocPartOfProcess;

      // the audio decoder finished (which is part of this task in terms of progress)
      p = (int)(100.0*m_audioDocPartOfProcess + (double)p*(1.0-m_audioDocPartOfProcess));

      emit percent( (int)((100.0*tasksDone + (double)p) / totalTasks) );
    }
    else {
      double totalTasks = d->copies+1.0;
      if( m_doc->audioDoc()->normalize() )
	totalTasks+=1.0;

      emit percent( (int)((double)(p*(1.0-m_audioDocPartOfProcess)) / totalTasks) );
    }
  }
}


bool K3bMixedJob::startWriting()
{
  if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
    if( m_currentAction == WRITING_ISO_IMAGE) {
      if( m_doc->dummy() )
	emit newTask( i18n("Simulating second session") );
      else if( d->copies > 1 )
	emit newTask( i18n("Writing second session of copy %1").arg(d->copiesDone+1) );
      else
	emit newTask( i18n("Writing second session") );
    }
    else {
      if( m_doc->dummy() )
	emit newTask( i18n("Simulating first session") );
      else if( d->copies > 1 )
	emit newTask( i18n("Writing first session of copy %1").arg(d->copiesDone+1) );
      else
	emit newTask( i18n("Writing first session") );
    }
  }
  else if( m_doc->dummy() )
    emit newTask( i18n("Simulating") );
  else
    emit newTask( i18n("Writing Copy %1").arg(d->copiesDone+1) );


  // if we append the second session the cd is already in the drive
  if( !(m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION
	&& m_currentAction == WRITING_ISO_IMAGE) ) {

    emit newSubTask( i18n("Waiting for media") );
    if( waitForMedia( m_doc->burner() ) < 0 ) {
      cancel();
      return false;
    }

    // just to be sure we did not get canceled during the async discWaiting
    if( m_canceled )
      return false;

    // check if the project will fit on the CD
    if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
      // the media is in and has been checked so this should be fast (hopefully)
      K3b::Msf mediaSize = m_doc->burner()->diskInfo().capacity();
      if( mediaSize < m_projectSize ) {
	if( k3bcore->globalSettings()->overburn() ) {
	  emit infoMessage( i18n("Trying to write more than the official disk capacity"), K3bJob::WARNING );
	}
	else {
	  emit infoMessage( i18n("Data does not fit on disk."), ERROR );
	  return false;
	}
      }
    }
  }

  // in case we determined the max possible writing speed we have to reset the speed on the writer job
  // here since an inserted media is necessary
  // the Max speed job will compare the max speed value with the supported values of the writer
  if( d->maxSpeed )
    m_writer->setBurnSpeed( d->maxSpeedJob->maxSpeed() );

  emit burning(true);
  m_writer->start();

  if( m_doc->onTheFly() ) {
    // now the writer is running and we can get it's stdin
    // we only use this method when writing on-the-fly since
    // we cannot easily change the audioDecode fd while it's working
    // which we would need to do since we write into several
    // image files.
    m_audioImager->writeToFd( m_writer->fd() );
    m_isoImager->writeToFd( m_writer->fd() );
  }

  return true;
}


void K3bMixedJob::createIsoImage()
{
  m_currentAction = CREATING_ISO_IMAGE;

  // prepare iso image file
  m_isoImageFilePath = m_tempFilePrefix + "_datatrack.iso";

  if( !m_doc->onTheFly() )
    emit newTask( i18n("Creating ISO image file") );
  emit newSubTask( i18n("Creating ISO image in %1").arg(m_isoImageFilePath) );
  emit infoMessage( i18n("Creating ISO image in %1").arg(m_isoImageFilePath), INFO );

  m_isoImager->writeToImageFile( m_isoImageFilePath );
  m_isoImager->start();
}


void K3bMixedJob::cleanupAfterError()
{
  m_errorOccuredAndAlreadyReported = true;
  //  m_audioImager->cancel();
  m_isoImager->cancel();
  if( m_writer )
    m_writer->cancel();

  if( m_tocFile ) delete m_tocFile;
  m_tocFile = 0;

  // remove the temp files
  removeBufferFiles();
}


void K3bMixedJob::removeBufferFiles()
{
    if ( !m_doc->onTheFly() ) {
        emit infoMessage( i18n("Removing buffer files."), INFO );
    }

    if( QFile::exists( m_isoImageFilePath ) )
        if( !QFile::remove( m_isoImageFilePath ) )
            emit infoMessage( i18n("Could not delete file %1.").arg(m_isoImageFilePath), ERROR );

    // removes buffer images and temp toc or inf files
    m_tempData->cleanup();
}


void K3bMixedJob::determineWritingMode()
{
  // we don't need this when only creating image and it is possible
  // that the burn device is null
  if( m_doc->onlyCreateImages() )
    return;

  // at first we determine the data mode
  // --------------------------------------------------------------
  if( m_doc->dataDoc()->dataMode() == K3b::DATA_MODE_AUTO ) {
    if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION )
      m_usedDataMode = K3b::MODE2;
    else
      m_usedDataMode = K3b::MODE1;
  }
  else
    m_usedDataMode = m_doc->dataDoc()->dataMode();


  // we try to use cdrecord if possible
  bool cdrecordOnTheFly = false;
  bool cdrecordCdText = false;
  bool cdrecordUsable = false;

  if( k3bcore->externalBinManager()->binObject("cdrecord") ) {
    cdrecordOnTheFly =
      k3bcore->externalBinManager()->binObject("cdrecord")->hasFeature( "audio-stdin" );
    cdrecordCdText =
      k3bcore->externalBinManager()->binObject("cdrecord")->hasFeature( "cdtext" );
    cdrecordUsable =
      !( !cdrecordOnTheFly && m_doc->onTheFly() ) &&
      !( m_doc->audioDoc()->cdText() && !cdrecordCdText );
  }

  // Writing Application
  // --------------------------------------------------------------
  // cdrecord seems to have problems writing xa 1 disks in dao mode? At least on my system!
  if( writingApp() == K3b::DEFAULT ) {
    if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
      if( m_doc->writingMode() == K3b::DAO ||
	  ( m_doc->writingMode() == K3b::WRITING_MODE_AUTO && !cdrecordUsable ) ) {
	m_usedAudioWritingApp = K3b::CDRDAO;
	m_usedDataWritingApp = K3b::CDRDAO;
      }
      else {
	m_usedAudioWritingApp = K3b::CDRECORD;
	m_usedDataWritingApp = K3b::CDRECORD;
      }
    }
    else {
      if( cdrecordUsable ) {
	m_usedAudioWritingApp = K3b::CDRECORD;
	m_usedDataWritingApp = K3b::CDRECORD;
      }
      else {
	m_usedAudioWritingApp = K3b::CDRDAO;
	m_usedDataWritingApp = K3b::CDRDAO;
      }
    }
  }
  else {
    m_usedAudioWritingApp = writingApp();
    m_usedDataWritingApp = writingApp();
  }

  // TODO: use K3bExceptions::brokenDaoAudio

  // Writing Mode (TAO/DAO/RAW)
  // --------------------------------------------------------------
  if( m_doc->writingMode() == K3b::WRITING_MODE_AUTO ) {

    if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
      if( m_usedDataWritingApp == K3b::CDRECORD )
	m_usedDataWritingMode = K3b::TAO;
      else
	m_usedDataWritingMode = K3b::DAO;

      // default to Session at once for the audio part
      m_usedAudioWritingMode = K3b::DAO;
    }
    else if( writer()->dao() ) {
      m_usedDataWritingMode = K3b::DAO;
      m_usedAudioWritingMode = K3b::DAO;
    }
    else {
      m_usedDataWritingMode = K3b::TAO;
      m_usedAudioWritingMode = K3b::TAO;
    }
  }
  else {
    m_usedAudioWritingMode = m_doc->writingMode();
    m_usedDataWritingMode = m_doc->writingMode();
  }


  if( m_usedDataWritingApp == K3b::CDRECORD ) {
    if( !cdrecordOnTheFly && m_doc->onTheFly() ) {
      m_doc->setOnTheFly( false );
      emit infoMessage( i18n("On-the-fly writing with cdrecord < 2.01a13 not supported."), ERROR );
    }

    if( m_doc->audioDoc()->cdText() ) {
      if( !cdrecordCdText ) {
	m_doc->audioDoc()->writeCdText( false );
	emit infoMessage( i18n("Cdrecord %1 does not support CD-Text writing.").arg(k3bcore->externalBinManager()->binObject("cdrecord")->version), ERROR );
      }
      else if( m_usedAudioWritingMode == K3b::TAO ) {
	emit infoMessage( i18n("It is not possible to write CD-Text in TAO mode. Try DAO or RAW."), WARNING );
      }
    }
  }
}


void K3bMixedJob::normalizeFiles()
{
  if( !m_normalizeJob ) {
    m_normalizeJob = new K3bAudioNormalizeJob( this, this );

    connect( m_normalizeJob, SIGNAL(infoMessage(const QString&, int)),
	     this, SIGNAL(infoMessage(const QString&, int)) );
    connect( m_normalizeJob, SIGNAL(percent(int)), this, SLOT(slotNormalizeProgress(int)) );
    connect( m_normalizeJob, SIGNAL(subPercent(int)), this, SLOT(slotNormalizeSubProgress(int)) );
    connect( m_normalizeJob, SIGNAL(finished(bool)), this, SLOT(slotNormalizeJobFinished(bool)) );
    connect( m_normalizeJob, SIGNAL(newTask(const QString&)), this, SIGNAL(newSubTask(const QString&)) );
    connect( m_normalizeJob, SIGNAL(debuggingOutput(const QString&, const QString&)),
	     this, SIGNAL(debuggingOutput(const QString&, const QString&)) );
  }

  // add all the files
  QValueVector<QString> files;
  K3bAudioTrack* track = m_doc->audioDoc()->firstTrack();
  while( track ) {
    files.append( m_tempData->bufferFileName(track) );
    track = track->next();
  }

  m_normalizeJob->setFilesToNormalize( files );

  emit newTask( i18n("Normalizing volume levels") );
  m_normalizeJob->start();
}

void K3bMixedJob::slotNormalizeJobFinished( bool success )
{
  if( m_canceled || m_errorOccuredAndAlreadyReported )
    return;

  if( success ) {
    if( m_doc->mixedType() == K3bMixedDoc::DATA_FIRST_TRACK )
      m_currentAction = WRITING_ISO_IMAGE;
    else
      m_currentAction = WRITING_AUDIO_IMAGE;

    if( !prepareWriter() || !startWriting() ) {
      cleanupAfterError();
      jobFinished(false);
    }
  }
  else {
    cleanupAfterError();
    jobFinished(false);
  }
}

void K3bMixedJob::slotNormalizeProgress( int p )
{
  double totalTasks = d->copies+2.0;
  double tasksDone = 0;

  if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION ) {
    // the audio imager finished (m_audioDocPartOfProcess*1 task)
    // plus the normalize progress
    tasksDone = m_audioDocPartOfProcess;
  }
  else {
    // the iso and audio imagers already finished (one task)
    // plus the normalize progress
    tasksDone = 1.0;
  }

  emit percent( (int)((100.0*tasksDone + (double)p) / totalTasks) );
}


void K3bMixedJob::slotNormalizeSubProgress( int p )
{
  emit subPercent( p );
}


void K3bMixedJob::prepareProgressInformation()
{
  // calculate percentage of audio and data
  // this is also used in on-the-fly mode
  double ds = (double)m_doc->dataDoc()->length().totalFrames();
  double as = (double)m_doc->audioDoc()->length().totalFrames();
  m_audioDocPartOfProcess = as/(ds+as);
}


QString K3bMixedJob::jobDescription() const
{
  if( m_doc->mixedType() == K3bMixedDoc::DATA_SECOND_SESSION )
    return i18n("Writing Enhanced Audio CD")
      + ( m_doc->audioDoc()->title().isEmpty()
	  ? QString::null
	  : QString( " (%1)" ).arg(m_doc->audioDoc()->title()) );
  else
    return i18n("Writing Mixed Mode CD")
      + ( m_doc->audioDoc()->title().isEmpty()
	  ? QString::null
	  : QString( " (%1)" ).arg(m_doc->audioDoc()->title()) );
}


QString K3bMixedJob::jobDetails() const
{
  return ( i18n("%1 tracks (%2 minutes audio data, %3 ISO9660 data)")
	   .arg(m_doc->numOfTracks())
	   .arg(m_doc->audioDoc()->length().toString())
	   .arg(KIO::convertSize(m_doc->dataDoc()->size()))
	   + ( m_doc->copies() > 1 && !m_doc->dummy()
	       ? i18n(" - %n copy", " - %n copies", m_doc->copies())
	       : QString::null ) );
}

#include "k3bmixedjob.moc"
