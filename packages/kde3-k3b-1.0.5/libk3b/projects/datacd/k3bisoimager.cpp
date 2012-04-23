/*
 *
 * $Id: k3bisoimager.cpp 655085 2007-04-17 17:48:36Z trueg $
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
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
#include <k3bglobals.h>

#include "k3bisoimager.h"
#include "k3bdiritem.h"
#include "k3bbootitem.h"
#include "k3bdatadoc.h"
#include "k3bdatapreparationjob.h"
#include <k3bexternalbinmanager.h>
#include <k3bdevice.h>
#include <k3bprocess.h>
#include <k3bcore.h>
#include <k3bversion.h>
#include <k3bglobals.h>
#include <k3bchecksumpipe.h>
#include <k3bfilesplitter.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kstringhandler.h>

#include <qfile.h>
#include <qregexp.h>
#include <qdir.h>
#include <qapplication.h>
#include <qvaluestack.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>


int K3bIsoImager::s_imagerSessionCounter = 0;


class K3bIsoImager::Private
{
public:
  Private()
    : pipe(0) {
  }

  ~Private() {
    delete pipe;
  }

  QString imagePath;
  K3bFileSplitter imageFile;
  const K3bExternalBin* mkisofsBin;

  enum LinkHandling {
    KEEP_ALL,
    FOLLOW,
    DISCARD_ALL,
    DISCARD_BROKEN
  };

  int usedLinkHandling;

  bool knownError;

  K3bActivePipe* pipe;
  K3bDataPreparationJob* dataPreparationJob;
};


K3bIsoImager::K3bIsoImager( K3bDataDoc* doc, K3bJobHandler* hdl, QObject* parent, const char* name )
  : K3bJob( hdl, parent, name ),
    m_pathSpecFile(0),
    m_rrHideFile(0),
    m_jolietHideFile(0),
    m_sortWeightFile(0),
    m_process( 0 ),
    m_processExited(false),
    m_doc( doc ),
    m_noDeepDirectoryRelocation( false ),
    m_importSession( false ),
    m_device(0),
    m_mkisofsPrintSizeResult( 0 ),
    m_fdToWriteTo(-1)
{
  d = new Private();
  d->dataPreparationJob = new K3bDataPreparationJob( doc, this, this );
  connectSubJob( d->dataPreparationJob,
		 SLOT(slotDataPreparationDone(bool)),
		 DEFAULT_SIGNAL_CONNECTION );
}


K3bIsoImager::~K3bIsoImager()
{
  cleanup();
  delete d;
}


bool K3bIsoImager::active() const
{
  return K3bJob::active();
}


void K3bIsoImager::writeToFd( int fd )
{
  m_fdToWriteTo = fd;
}


void K3bIsoImager::writeToImageFile( const QString& path )
{
  d->imagePath = path;
  m_fdToWriteTo = -1;
}


void K3bIsoImager::slotReceivedStderr( const QString& line )
{
  parseMkisofsOutput( line );
  emit debuggingOutput( "mkisofs", line );
}


void K3bIsoImager::handleMkisofsProgress( int p )
{
  emit percent( p );
}


void K3bIsoImager::handleMkisofsInfoMessage( const QString& line, int type )
{
  emit infoMessage( line, type );
  if( type == ERROR )
    d->knownError = true;
}


void K3bIsoImager::slotProcessExited( KProcess* p )
{
  kdDebug() << k_funcinfo << endl;

  m_processExited = true;

  d->pipe->close();

  emit debuggingOutput( "K3bIsoImager",
			QString("Pipe throughput: %1 bytes read, %2 bytes written.")
			.arg(d->pipe->bytesRead()).arg(d->pipe->bytesWritten()) );

  if( d->imageFile.isOpen() ) {
    d->imageFile.close();

    if( m_canceled || p->exitStatus() != 0 ) {
      d->imageFile.remove();
      emit infoMessage( i18n("Removed incomplete image file %1.").arg(d->imageFile.name()), WARNING );
    }
  }

  if( m_canceled ) {
    emit canceled();
    jobFinished(false);
  }
  else {
    if( p->normalExit() ) {
      if( p->exitStatus() == 0 ) {
	jobFinished( !mkisofsReadError() );
      }
      else {
	switch( p->exitStatus() ) {
	case 104:
	  // connection reset by peer
	  // This only happens if cdrecord does not finish successfully
	  // so we may leave the error handling to it meaning we handle this
	  // as a known error
	  break;

	case 2:
	  // mkisofs seems to have a bug that prevents to use filenames
	  // that contain one or more backslashes
	  // mkisofs 1.14 has the bug, 1.15a40 not
	  // TODO: find out the version that fixed the bug
	  if( m_containsFilesWithMultibleBackslashes &&
	      !k3bcore->externalBinManager()->binObject( "mkisofs" )->hasFeature( "backslashed_filenames" ) ) {
	    emit infoMessage( i18n("Due to a bug in mkisofs <= 1.15a40, K3b is unable to handle "
				   "filenames that contain more than one backslash:"), ERROR );

	    break;
	  }
	  // otherwise just fall through

	default:
	  if( !d->knownError && !mkisofsReadError() ) {
	    emit infoMessage( i18n("%1 returned an unknown error (code %2).").arg("mkisofs").arg(p->exitStatus()),
			      K3bJob::ERROR );
	    emit infoMessage( i18n("Please send me an email with the last output."), K3bJob::ERROR );
	  }
	}

	jobFinished( false );
      }
    }
    else {
      emit infoMessage( i18n("%1 did not exit cleanly.").arg("mkisofs"), ERROR );
      jobFinished( false );
    }
  }

  cleanup();
}


void K3bIsoImager::cleanup()
{
  // remove all temp files
  delete m_pathSpecFile;
  delete m_rrHideFile;
  delete m_jolietHideFile;
  delete m_sortWeightFile;

  // remove boot-images-temp files
  for( QStringList::iterator it = m_tempFiles.begin();
       it != m_tempFiles.end(); ++it )
    QFile::remove( *it );
  m_tempFiles.clear();

  m_pathSpecFile = m_jolietHideFile = m_rrHideFile = m_sortWeightFile = 0;

  delete m_process;
  m_process = 0;

  clearDummyDirs();
}


void K3bIsoImager::init()
{
  jobStarted();

  cleanup();

  d->dataPreparationJob->start();
}


void K3bIsoImager::slotDataPreparationDone( bool success )
{
  if( success ) {
    //
    // We always calculate the image size. It does not take long and at least the mixed job needs it
    // anyway
    //
    startSizeCalculation();
  }
  else {
    if( d->dataPreparationJob->hasBeenCanceled() ) {
      m_canceled = true;
      emit canceled();
    }
    jobFinished( false );
  }
}


void K3bIsoImager::calculateSize()
{
  jobStarted();
  startSizeCalculation();
}


void K3bIsoImager::startSizeCalculation()
{
  d->mkisofsBin = initMkisofs();
  if( !d->mkisofsBin ) {
    jobFinished( false );
    return;
  }

  initVariables();

  delete m_process;
  m_process = new K3bProcess();
  m_process->setRunPrivileged(true);
  m_process->setSplitStdout(true);

  emit debuggingOutput( "Used versions", "mkisofs: " + d->mkisofsBin->version );

  *m_process << d->mkisofsBin;

  if( !prepareMkisofsFiles() ||
      !addMkisofsParameters(true) ) {
    cleanup();
    jobFinished( false );
    return;
  }

  // add empty dummy dir since one path-spec is needed
  // ??? Seems it is not needed after all. At least mkisofs 1.14 and above don't need it. ???
  //  *m_process << dummyDir();

  kdDebug() << "***** mkisofs calculate size parameters:\n";
  const QValueList<QCString>& args = m_process->args();
  QString s;
  for( QValueList<QCString>::const_iterator it = args.begin(); it != args.end(); ++it ) {
    s += *it + " ";
  }
  kdDebug() << s << endl << flush;
  emit debuggingOutput("mkisofs calculate size command:", s);

  // since output changed during mkisofs version changes we grab both
  // stdout and stderr

  // mkisofs version >= 1.15 (don't know about 1.14!)
  // the extends on stdout (as lonely number)
  // and error and warning messages on stderr

  // mkisofs >= 1.13
  // everything is written to stderr
  // last line is: "Total extents scheduled to be written = XXXXX"

  // TODO: use K3bProcess::OutputCollector instead iof our own two slots.

  connect( m_process, SIGNAL(receivedStderr(KProcess*, char*, int)),
	   this, SLOT(slotCollectMkisofsPrintSizeStderr(KProcess*, char*, int)) );
  connect( m_process, SIGNAL(stdoutLine(const QString&)),
	   this, SLOT(slotCollectMkisofsPrintSizeStdout(const QString&)) );
  connect( m_process, SIGNAL(processExited(KProcess*)),
	   this, SLOT(slotMkisofsPrintSizeFinished()) );

  // we also want error messages
  connect( m_process, SIGNAL(stderrLine( const QString& )),
	   this, SLOT(slotReceivedStderr( const QString& )) );

  m_collectedMkisofsPrintSizeStdout = QString::null;
  m_collectedMkisofsPrintSizeStderr = QString::null;
  m_mkisofsPrintSizeResult = 0;

  if( !m_process->start( KProcess::NotifyOnExit, KProcess::AllOutput ) ) {
    emit infoMessage( i18n("Could not start %1.").arg("mkisofs"), K3bJob::ERROR );
    cleanup();

    jobFinished( false );
    return;
  }
}


void K3bIsoImager::slotCollectMkisofsPrintSizeStderr(KProcess*, char* data , int len)
{
  emit debuggingOutput( "mkisofs", QString::fromLocal8Bit( data, len ) );
  m_collectedMkisofsPrintSizeStderr.append( QString::fromLocal8Bit( data, len ) );
}


void K3bIsoImager::slotCollectMkisofsPrintSizeStdout( const QString& line )
{
  // newer versions of mkisofs outut additional lines of junk before the size :(
  // so we only use the last line
  emit debuggingOutput( "mkisofs", line );
  m_collectedMkisofsPrintSizeStdout = line;
}


void K3bIsoImager::slotMkisofsPrintSizeFinished()
{
  if( m_canceled ) {
    emit canceled();
    jobFinished( false );
    return;
  }

  bool success = true;

  // if m_collectedMkisofsPrintSizeStdout is not empty we have a recent version of
  // mkisofs and parsing is very easy (s.o.)
  if( !m_collectedMkisofsPrintSizeStdout.isEmpty() ) {
    kdDebug() << "(K3bIsoImager) iso size: " << m_collectedMkisofsPrintSizeStdout << endl;
    m_mkisofsPrintSizeResult = m_collectedMkisofsPrintSizeStdout.toInt( &success );
  }
  else {
    // parse the stderr output
    // I hope parsing the last line is enough!
    int pos = m_collectedMkisofsPrintSizeStderr.findRev( "extents scheduled to be written" );

    if( pos == -1 )
      success = false;
    else
      m_mkisofsPrintSizeResult = m_collectedMkisofsPrintSizeStderr.mid( pos+33 ).toInt( &success );
  }

  emit debuggingOutput( "K3bIsoImager",
			QString("mkisofs print size result: %1 (%2 bytes)")
			.arg(m_mkisofsPrintSizeResult)
			.arg(Q_UINT64(m_mkisofsPrintSizeResult)*2048ULL) );

  cleanup();


  if( success ) {
    jobFinished( true );
  }
  else {
    m_mkisofsPrintSizeResult = 0;
    kdDebug() << "(K3bIsoImager) Parsing mkisofs -print-size failed: " << m_collectedMkisofsPrintSizeStdout << endl;
    emit infoMessage( i18n("Could not determine size of resulting image file."), ERROR );
    jobFinished( false );
  }
}


void K3bIsoImager::initVariables()
{
  m_containsFilesWithMultibleBackslashes = false;
  m_processExited = false;
  m_canceled = false;
  d->knownError = false;

  // determine symlink handling
  // follow links superseeds discard all links which superseeds discard broken links
  // without rockridge we follow the links or discard all
  if( m_doc->isoOptions().followSymbolicLinks() )
    d->usedLinkHandling = Private::FOLLOW;
  else if( m_doc->isoOptions().discardSymlinks() )
    d->usedLinkHandling = Private::DISCARD_ALL;
  else if( m_doc->isoOptions().createRockRidge() ) {
    if( m_doc->isoOptions().discardBrokenSymlinks() )
      d->usedLinkHandling = Private::DISCARD_BROKEN;
    else
      d->usedLinkHandling = Private::KEEP_ALL;
  }
  else {
    d->usedLinkHandling = Private::FOLLOW;
  }

  m_sessionNumber = s_imagerSessionCounter++;
}


void K3bIsoImager::start()
{
  jobStarted();

  cleanup();

  d->mkisofsBin = initMkisofs();
  if( !d->mkisofsBin ) {
    jobFinished( false );
    return;
  }

  initVariables();

  m_process = new K3bProcess();
  m_process->setRunPrivileged(true);

  *m_process << d->mkisofsBin;

  // prepare the filenames as written to the image
  m_doc->prepareFilenames();

  if( !prepareMkisofsFiles() ||
      !addMkisofsParameters() ) {
    cleanup();
    jobFinished( false );
    return;
  }

  connect( m_process, SIGNAL(processExited(KProcess*)),
	   this, SLOT(slotProcessExited(KProcess*)) );

  connect( m_process, SIGNAL(stderrLine( const QString& )),
	   this, SLOT(slotReceivedStderr( const QString& )) );

  //
  // Check the image file
  if( m_fdToWriteTo == -1 ) {
    d->imageFile.setName( d->imagePath );
    if( !d->imageFile.open( IO_WriteOnly ) ) {
      emit infoMessage( i18n("Could not open %1 for writing").arg(d->imagePath), ERROR );
      cleanup();
      jobFinished(false);
      return;
    }
  }

  //
  // Open the active pipe which does the streaming
  delete d->pipe;
  if( m_doc->verifyData() )
    d->pipe = new K3bChecksumPipe();
  else
    d->pipe = new K3bActivePipe();

  if( m_fdToWriteTo == -1 )
    d->pipe->writeToIODevice( &d->imageFile );
  else
    d->pipe->writeToFd( m_fdToWriteTo );
  d->pipe->open();
  m_process->writeToFd( d->pipe->in() );


  kdDebug() << "***** mkisofs parameters:\n";
  const QValueList<QCString>& args = m_process->args();
  QString s;
  for( QValueList<QCString>::const_iterator it = args.begin(); it != args.end(); ++it ) {
    s += *it + " ";
  }
  kdDebug() << s << endl << flush;
  emit debuggingOutput("mkisofs command:", s);

  if( !m_process->start( KProcess::NotifyOnExit, KProcess::AllOutput) ) {
    // something went wrong when starting the program
    // it "should" be the executable
    kdDebug() << "(K3bIsoImager) could not start mkisofs" << endl;
    emit infoMessage( i18n("Could not start %1.").arg("mkisofs"), K3bJob::ERROR );
    jobFinished( false );
    cleanup();
  }
}


void K3bIsoImager::cancel()
{
  m_canceled = true;

  if( m_process && !m_processExited ) {
    m_process->kill();
  }
  else if( active() ) {
    emit canceled();
    jobFinished(false);
  }
}


void K3bIsoImager::setMultiSessionInfo( const QString& info, K3bDevice::Device* dev )
{
  m_multiSessionInfo = info;
  m_device = dev;
}


// iso9660 + RR use some latin1 variant. So we need to cut the desc fields
// counting 8bit chars. The GUI should take care of restricting the length
// and the charset
static void truncateTheHardWay( QString& s, int max )
{
  QCString cs = s.utf8();
  cs.truncate(max);
  s = QString::fromUtf8( cs );
}


bool K3bIsoImager::addMkisofsParameters( bool printSize )
{
  // add multisession info
  if( !m_multiSessionInfo.isEmpty() ) {
    *m_process << "-cdrecord-params" << m_multiSessionInfo;
    if( m_device )
      *m_process << "-prev-session" << m_device->blockDeviceName();
  }

  // add the arguments
  *m_process << "-gui";
  *m_process << "-graft-points";

  if( printSize )
    *m_process << "-print-size" << "-quiet";

  if( !m_doc->isoOptions().volumeID().isEmpty() ) {
    QString s = m_doc->isoOptions().volumeID();
    truncateTheHardWay(s, 32);  // ensure max length
    *m_process << "-volid" << s;
  }
  else {
    emit infoMessage( i18n("No volume id specified. Using default."), WARNING );
    *m_process << "-volid" << "CDROM";
  }

  QString s = m_doc->isoOptions().volumeSetId();
  truncateTheHardWay(s, 128);  // ensure max length
  *m_process << "-volset" << s;

  s = m_doc->isoOptions().applicationID();
  truncateTheHardWay(s, 128);  // ensure max length
  *m_process << "-appid" << s;

  s = m_doc->isoOptions().publisher();
  truncateTheHardWay(s, 128);  // ensure max length
  *m_process << "-publisher" << s;

  s = m_doc->isoOptions().preparer();
  truncateTheHardWay(s, 128);  // ensure max length
  *m_process << "-preparer" << s;

  s = m_doc->isoOptions().systemId();
  truncateTheHardWay(s, 32);  // ensure max length
  *m_process << "-sysid" << s;

  s = m_doc->isoOptions().abstractFile();
  truncateTheHardWay(s, 37);  // ensure max length
  if ( !s.isEmpty() )
      *m_process << "-abstract" << s;

  s = m_doc->isoOptions().copyrightFile();
  truncateTheHardWay(s, 37);  // ensure max length
  if ( !s.isEmpty() )
      *m_process << "-copyright" << s;

  s = m_doc->isoOptions().bibliographFile();
  truncateTheHardWay(s, 37);  // ensure max length
  if ( !s.isEmpty() )
      *m_process << "-biblio" << s;

  int volsetSize = m_doc->isoOptions().volumeSetSize();
  int volsetSeqNo = m_doc->isoOptions().volumeSetNumber();
  if( volsetSeqNo > volsetSize ) {
    kdDebug() << "(K3bIsoImager) invalid volume set sequence number: " << volsetSeqNo
	      << " with volume set size: " << volsetSize << endl;
    volsetSeqNo = volsetSize;
  }
  *m_process << "-volset-size" << QString::number(volsetSize);
  *m_process << "-volset-seqno" << QString::number(volsetSeqNo);

  if( m_sortWeightFile ) {
    *m_process << "-sort" << m_sortWeightFile->name();
  }

  if( m_doc->isoOptions().createRockRidge() ) {
    if( m_doc->isoOptions().preserveFilePermissions() )
      *m_process << "-rock";
    else
      *m_process << "-rational-rock";
    if( m_rrHideFile )
      *m_process << "-hide-list" << m_rrHideFile->name();
  }

  if( m_doc->isoOptions().createJoliet() ) {
    *m_process << "-joliet";
    if( m_doc->isoOptions().jolietLong() )
      *m_process << "-joliet-long";
    if( m_jolietHideFile )
      *m_process << "-hide-joliet-list" << m_jolietHideFile->name();
  }

  if( m_doc->isoOptions().doNotCacheInodes() )
    *m_process << "-no-cache-inodes";

  //
  // Check if we have files > 2 GB and enable udf in that case.
  //
  bool filesGreaterThan2Gb = false;
  K3bDataItem* item = m_doc->root();
  while( (item = item->nextSibling()) ) {
    if( item->isFile() && item->size() > 2LL*1024LL*1024LL*1024LL ) {
      filesGreaterThan2Gb = true;
      break;
    }
  }

  if( filesGreaterThan2Gb ) {
    emit infoMessage( i18n("Found files bigger than 2 GB. These files will only be fully accessible if mounted with UDF."),
		      WARNING );

    // in genisoimage 1.1.3 "they" silently introduced this aweful parameter
    if ( d->mkisofsBin->hasFeature( "genisoimage" ) && d->mkisofsBin->version >= K3bVersion( 1, 1, 3 ) ) {
        *m_process << "-allow-limited-size";
    }
  }

  bool udf = m_doc->isoOptions().createUdf();
  if( !udf && filesGreaterThan2Gb ) {
    emit infoMessage( i18n("Enabling UDF extension."), INFO );
    udf = true;
  }
  if( udf )
    *m_process << "-udf";

  if( m_doc->isoOptions().ISOuntranslatedFilenames()  ) {
    *m_process << "-untranslated-filenames";
  }
  else {
    if( m_doc->isoOptions().ISOallowPeriodAtBegin()  )
      *m_process << "-allow-leading-dots";
    if( m_doc->isoOptions().ISOallow31charFilenames()  )
      *m_process << "-full-iso9660-filenames";
    if( m_doc->isoOptions().ISOomitVersionNumbers() && !m_doc->isoOptions().ISOmaxFilenameLength() )
      *m_process << "-omit-version-number";
    if( m_doc->isoOptions().ISOrelaxedFilenames()  )
      *m_process << "-relaxed-filenames";
    if( m_doc->isoOptions().ISOallowLowercase()  )
      *m_process << "-allow-lowercase";
    if( m_doc->isoOptions().ISOnoIsoTranslate()  )
      *m_process << "-no-iso-translate";
    if( m_doc->isoOptions().ISOallowMultiDot()  )
      *m_process << "-allow-multidot";
    if( m_doc->isoOptions().ISOomitTrailingPeriod() )
      *m_process << "-omit-period";
  }

  if( m_doc->isoOptions().ISOmaxFilenameLength()  )
    *m_process << "-max-iso9660-filenames";

  if( m_noDeepDirectoryRelocation  )
    *m_process << "-disable-deep-relocation";

  // We do our own following
//   if( m_doc->isoOptions().followSymbolicLinks() || !m_doc->isoOptions().createRockRidge() )
//     *m_process << "-follow-links";

  if( m_doc->isoOptions().createTRANS_TBL()  )
    *m_process << "-translation-table";
  if( m_doc->isoOptions().hideTRANS_TBL()  )
    *m_process << "-hide-joliet-trans-tbl";

  *m_process << "-iso-level" << QString::number(m_doc->isoOptions().ISOLevel());

  if( m_doc->isoOptions().forceInputCharset() )
    *m_process << "-input-charset" << m_doc->isoOptions().inputCharset();

  *m_process << "-path-list" << QFile::encodeName(m_pathSpecFile->name());


  // boot stuff
  if( !m_doc->bootImages().isEmpty() ) {
    bool first = true;
    for( QPtrListIterator<K3bBootItem> it( m_doc->bootImages() );
	 *it; ++it ) {
      if( !first )
	*m_process << "-eltorito-alt-boot";

      K3bBootItem* bootItem = *it;

      *m_process << "-eltorito-boot";
      *m_process << bootItem->writtenPath();

      if( bootItem->imageType() == K3bBootItem::HARDDISK ) {
	*m_process << "-hard-disk-boot";
      }
      else if( bootItem->imageType() == K3bBootItem::NONE ) {
	*m_process << "-no-emul-boot";
	if( bootItem->loadSegment() > 0 )
	  *m_process << "-boot-load-seg" << QString::number(bootItem->loadSegment());
	if( bootItem->loadSize() > 0 )
	  *m_process << "-boot-load-size" << QString::number(bootItem->loadSize());
      }

      if( bootItem->imageType() != K3bBootItem::NONE && bootItem->noBoot() )
	*m_process << "-no-boot";
      if( bootItem->bootInfoTable() )
	*m_process << "-boot-info-table";

      first = false;
    }

    *m_process << "-eltorito-catalog" << m_doc->bootCataloge()->writtenPath();
  }


  // additional parameters from config
  const QStringList& params = k3bcore->externalBinManager()->binObject( "mkisofs" )->userParameters();
  for( QStringList::const_iterator it = params.begin(); it != params.end(); ++it )
    *m_process << *it;

  return true;
}


int K3bIsoImager::writePathSpec()
{
  delete m_pathSpecFile;
  m_pathSpecFile = new KTempFile();
  m_pathSpecFile->setAutoDelete(true);

  if( QTextStream* t = m_pathSpecFile->textStream() ) {
    // recursive path spec writing
    int num = writePathSpecForDir( m_doc->root(), *t );

    m_pathSpecFile->close();

    return num;
  }
  else
    return -1;
}


int K3bIsoImager::writePathSpecForDir( K3bDirItem* dirItem, QTextStream& stream )
{
  if( !m_noDeepDirectoryRelocation && dirItem->depth() > 7 ) {
    kdDebug() << "(K3bIsoImager) found directory depth > 7. Enabling no deep directory relocation." << endl;
    m_noDeepDirectoryRelocation = true;
  }

  // now create the graft points
  int num = 0;
  for( QPtrListIterator<K3bDataItem> it( dirItem->children() ); it.current(); ++it ) {
    K3bDataItem* item = it.current();
    bool writeItem = item->writeToCd();

    if( item->isSymLink() ) {
      if( d->usedLinkHandling == Private::DISCARD_ALL ||
	  ( d->usedLinkHandling == Private::DISCARD_BROKEN &&
	    !item->isValid() ) )
	writeItem = false;

      else if( d->usedLinkHandling == Private::FOLLOW ) {
	QFileInfo f( K3b::resolveLink( item->localPath() ) );
	if( !f.exists() ) {
	  emit infoMessage( i18n("Could not follow link %1 to non-existing file %2. Skipping...")
			    .arg(item->k3bName())
			    .arg(f.filePath()), WARNING );
	  writeItem = false;
	}
	else if( f.isDir() ) {
	  emit infoMessage( i18n("Ignoring link %1 to folder %2. K3b is unable to follow links to folders.")
			    .arg(item->k3bName())
			    .arg(f.filePath()), WARNING );
	  writeItem = false;
	}
      }
    }
    else if( item->isFile() ) {
      QFileInfo f( item->localPath() );
      if( !f.exists() ) {
	emit infoMessage( i18n("Could not find file %1. Skipping...").arg(item->localPath()), WARNING );
	writeItem = false;
      }
      else if( !f.isReadable() ) {
	emit infoMessage( i18n("Could not read file %1. Skipping...").arg(item->localPath()), WARNING );
	writeItem = false;
      }
    }

    if( writeItem ) {
      num++;

      // some versions of mkisofs seem to have a bug that prevents to use filenames
      // that contain one or more backslashes
      if( item->writtenPath().contains("\\") )
	m_containsFilesWithMultibleBackslashes = true;


      if( item->isDir() ) {
	stream << escapeGraftPoint( item->writtenPath() )
	       << "="
	       << escapeGraftPoint( dummyDir( static_cast<K3bDirItem*>(item) ) ) << "\n";

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
  }

  return num;
}


void K3bIsoImager::writePathSpecForFile( K3bFileItem* item, QTextStream& stream )
{
  stream << escapeGraftPoint( item->writtenPath() )
	 << "=";

  if( m_doc->bootImages().containsRef( dynamic_cast<K3bBootItem*>(item) ) ) { // boot-image-backup-hack

    // create temp file
    KTempFile temp;
    QString tempPath = temp.name();
    temp.unlink();

    if( !KIO::NetAccess::copy( KURL(item->localPath()), KURL::fromPathOrURL(tempPath) ) ) {
      emit infoMessage( i18n("Failed to backup boot image file %1").arg(item->localPath()), ERROR );
      return;
    }

    static_cast<K3bBootItem*>(item)->setTempPath( tempPath );

    m_tempFiles.append(tempPath);
    stream << escapeGraftPoint( tempPath ) << "\n";
  }
  else if( item->isSymLink() && d->usedLinkHandling == Private::FOLLOW )
    stream << escapeGraftPoint( K3b::resolveLink( item->localPath() ) ) << "\n";
  else
    stream << escapeGraftPoint( item->localPath() ) << "\n";
}


bool K3bIsoImager::writeRRHideFile()
{
  delete m_rrHideFile;
  m_rrHideFile = new KTempFile();
  m_rrHideFile->setAutoDelete(true);

  if( QTextStream* t = m_rrHideFile->textStream() ) {

    K3bDataItem* item = m_doc->root();
    while( item ) {
      if( item->hideOnRockRidge() ) {
	if( !item->isDir() )  // hiding directories does not work (all dirs point to the dummy-dir)
	  *t << escapeGraftPoint( item->localPath() ) << endl;
      }
      item = item->nextSibling();
    }

    m_rrHideFile->close();
    return true;
  }
  else
    return false;
}


bool K3bIsoImager::writeJolietHideFile()
{
  delete m_jolietHideFile;
  m_jolietHideFile = new KTempFile();
  m_jolietHideFile->setAutoDelete(true);

  if( QTextStream* t = m_jolietHideFile->textStream() ) {

    K3bDataItem* item = m_doc->root();
    while( item ) {
      if( item->hideOnRockRidge() ) {
	if( !item->isDir() )  // hiding directories does not work (all dirs point to the dummy-dir but we could introduce a second hidden dummy dir)
	  *t << escapeGraftPoint( item->localPath() ) << endl;
      }
      item = item->nextSibling();
    }

    m_jolietHideFile->close();
    return true;
  }
  else
    return false;
}


bool K3bIsoImager::writeSortWeightFile()
{
  delete m_sortWeightFile;
  m_sortWeightFile = new KTempFile();
  m_sortWeightFile->setAutoDelete(true);

  if( QTextStream* t = m_sortWeightFile->textStream() ) {
    //
    // We need to write the local path in combination with the sort weight
    // mkisofs will take care of multiple entries for one local file and always
    // use the highest weight
    //
    K3bDataItem* item = m_doc->root();
    while( (item = item->nextSibling()) ) {  // we skip the root here
      if( item->sortWeight() != 0 ) {
	if( m_doc->bootImages().containsRef( dynamic_cast<K3bBootItem*>(item) ) ) { // boot-image-backup-hack
	  *t << escapeGraftPoint( static_cast<K3bBootItem*>(item)->tempPath() ) << " " << item->sortWeight() << endl;
	}
	else if( item->isDir() ) {
	  //
	  // Since we use dummy dirs for all directories in the filesystem and mkisofs uses the local path
	  // for sorting we need to create a different dummy dir for every sort weight value.
	  //
	  *t << escapeGraftPoint( dummyDir( static_cast<K3bDirItem*>(item) ) ) << " " << item->sortWeight() << endl;
	}
	else
	  *t << escapeGraftPoint( item->localPath() ) << " " << item->sortWeight() << endl;
      }
    }

    m_sortWeightFile->close();
    return true;
  }
  else
    return false;
}


QString K3bIsoImager::escapeGraftPoint( const QString& str )
{
  QString enc = str;

  //
  // mkisofs manpage (-graft-points) is incorrect (as of mkisofs 2.01.01)
  //
  // Actually an equal sign needs to be escaped with one backslash only
  // Single backslashes inside a filename can be used without change
  // while single backslashes at the end of a filename need to be escaped
  // with two backslashes.
  //
  // There is one more problem though: the name in the iso tree can never
  // in any number of backslashes. mkisofs simply cannot handle it. So we
  // need to remove these slashes somewhere or ignore those files (we do
  // that in K3bDataDoc::addUrls)
  //

  //
  // we do not use QString::replace to have full control
  // this might be slow since QString::insert is slow but we don't care
  // since this is only called to prepare the iso creation which is not
  // time critical. :)
  //

  unsigned int pos = 0;
  while( pos < enc.length() ) {
    // escape every equal sign with one backslash
    if( enc[pos] == '=' ) {
      enc.insert( pos, "\\" );
      pos += 2;
    }
    else if( enc[pos] == '\\' ) {
      // escape every occurrence of two backslashes with two backslashes
      if( pos+1 < enc.length() && enc[pos+1] == '\\' ) {
	enc.insert( pos, "\\\\" );
	pos += 4;
      }
      // escape the last single backslash in the filename (see above)
      else if( pos == enc.length()-1 ) {
	enc.insert( pos, "\\" );
	pos += 2;
      }
      else
	++pos;
    }
    else
      ++pos;
  }

//   enc.replace( "\\\\", "\\\\\\\\" );
//   enc.replace( "=", "\\=" );

  return enc;
}


bool K3bIsoImager::prepareMkisofsFiles()
{
  // write path spec file
  // ----------------------------------------------------
  int num = writePathSpec();
  if( num < 0 ) {
    emit infoMessage( i18n("Could not write temporary file"), K3bJob::ERROR );
    return false;
  }
  else if( num == 0 ) {
    emit infoMessage( i18n("No files to be written."), K3bJob::ERROR );
    return false;
  }

  if( m_doc->isoOptions().createRockRidge() ) {
    if( !writeRRHideFile() ) {
      emit infoMessage( i18n("Could not write temporary file"), K3bJob::ERROR );
      return false;
    }
  }

  if( m_doc->isoOptions().createJoliet() ) {
    if( !writeJolietHideFile() ) {
      emit infoMessage( i18n("Could not write temporary file"), K3bJob::ERROR );
      return false ;
    }
  }

  if( !writeSortWeightFile() ) {
    emit infoMessage( i18n("Could not write temporary file"), K3bJob::ERROR );
    return false;
  }

  return true;
}


QString K3bIsoImager::dummyDir( K3bDirItem* dir )
{
  //
  // since we use virtual folders in order to have folders with different weight factors and different
  // permissions we create different dummy dirs to be passed to mkisofs
  //

  QDir _appDir( locateLocal( "appdata", "temp/" ) );

  //
  // create a unique isoimager session id
  // This might become important in case we will allow multiple instances of the isoimager
  // to run at the same time.
  //
  QString jobId = qApp->sessionId() + "_" + QString::number( m_sessionNumber );

  if( !_appDir.cd( jobId ) ) {
    _appDir.mkdir( jobId );
    _appDir.cd( jobId );
  }

  QString name( "dummydir_" );
  name += QString::number( dir->sortWeight() );

  bool perm = false;
  k3b_struct_stat statBuf;
  if( !dir->localPath().isEmpty() ) {
    // permissions
    if( k3b_stat( QFile::encodeName(dir->localPath()), &statBuf ) == 0 ) {
      name += "_";
      name += QString::number( statBuf.st_uid );
      name += "_";
      name += QString::number( statBuf.st_gid );
      name += "_";
      name += QString::number( statBuf.st_mode );
      name += "_";
      name += QString::number( statBuf.st_mtime );

      perm = true;
    }
  }


  if( !_appDir.cd( name ) ) {

    kdDebug() << "(K3bIsoImager) creating dummy dir: " << _appDir.absPath() << "/" << name << endl;

    _appDir.mkdir( name );
    _appDir.cd( name );

    if( perm ) {
      ::chmod( QFile::encodeName( _appDir.absPath() ), statBuf.st_mode );
      ::chown( QFile::encodeName( _appDir.absPath() ), statBuf.st_uid, statBuf.st_gid );
      struct utimbuf tb;
      tb.actime = tb.modtime = statBuf.st_mtime;
      ::utime( QFile::encodeName( _appDir.absPath() ), &tb );
    }
  }

  return _appDir.absPath() + "/";
}


void K3bIsoImager::clearDummyDirs()
{
  QString jobId = qApp->sessionId() + "_" + QString::number( m_sessionNumber );
  QDir appDir( locateLocal( "appdata", "temp/" ) );
  if( appDir.cd( jobId ) ) {
    QStringList dummyDirEntries = appDir.entryList( "dummydir*", QDir::Dirs );
    for( QStringList::iterator it = dummyDirEntries.begin(); it != dummyDirEntries.end(); ++it )
      appDir.rmdir( *it );
    appDir.cdUp();
    appDir.rmdir( jobId );
  }
}


QCString K3bIsoImager::checksum() const
{
  if( K3bChecksumPipe* p = dynamic_cast<K3bChecksumPipe*>( d->pipe ) )
    return p->checksum();
  else
    return QCString();
}


bool K3bIsoImager::hasBeenCanceled() const
{
  return m_canceled;
}

#include "k3bisoimager.moc"
