/*
 *
 * $Id: k3bapplication.cpp 802344 2008-04-29 07:45:46Z trueg $
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


#include "k3bapplication.h"
#include "k3b.h"
#include "k3bsplash.h"
#include "k3baudioserver.h"
#include "k3binterface.h"
#include "k3bjobinterface.h"
#include "k3bprojectmanager.h"
#include "k3bappdevicemanager.h"
#include "k3bmediacache.h"
#include "k3bpassivepopup.h"
#include "k3blsofwrapperdialog.h"
#include "k3bfirstrun.h"

#include <k3bcore.h>
#include <k3bdevicemanager.h>
#include <k3bhalconnection.h>
#include <k3bexternalbinmanager.h>
#include <k3bdefaultexternalprograms.h>
#include <k3bglobals.h>
#include <k3bversion.h>
#include <k3bdoc.h>
#include "k3bsystemproblemdialog.h"
#include <k3bthread.h>
#include <k3bpluginmanager.h>
#include <k3bthememanager.h>
#include <k3bmsf.h>
#include <k3bmovixprogram.h>
#include <k3bview.h>
#include <k3bjob.h>

#include <ktip.h>
#include <klocale.h>
#include <kconfig.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <dcopclient.h>
#include <kstandarddirs.h>
#include <kstartupinfo.h>
#include <kmessagebox.h>

#include <qguardedptr.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <qcstring.h>


K3bApplication::Core* K3bApplication::Core::s_k3bAppCore = 0;


K3bApplication::K3bApplication()
  : KUniqueApplication(),
    m_mainWindow(0),
    m_needToInit(true)
{
  // insert library i18n data
  KGlobal::locale()->insertCatalogue( "libk3bdevice" );
  KGlobal::locale()->insertCatalogue( "libk3b" );

  m_core = new Core( this );

  // TODO: move to K3bCore?
  // from this point on available through K3bAudioServer::instance()
  m_audioServer = new K3bAudioServer( this, "K3bAudioServer" );

  connect( m_core, SIGNAL(initializationInfo(const QString&)),
	   SIGNAL(initializationInfo(const QString&)) );

  connect( this, SIGNAL(shutDown()), SLOT(slotShutDown()) );
}


K3bApplication::~K3bApplication()
{
  // we must not delete m_mainWindow here, QApplication takes care of it
}


void K3bApplication::init()
{
  KConfigGroup generalOptions( config(), "General Options" );

  QGuardedPtr<K3bSplash> splash;
  if( !isRestored() ) {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if( generalOptions.readBoolEntry("Show splash", true) && args->isSet( "splash" ) ) {
      // we need the correct splash pic
      m_core->m_themeManager->readConfig( config() );

      splash = new K3bSplash( 0 );
      splash->connect( this, SIGNAL(initializationInfo(const QString&)), SLOT(addInfo(const QString&)) );

      // kill the splash after 5 seconds
      QTimer::singleShot( 5000, splash, SLOT(close()) );

      splash->show();
      qApp->processEvents();
    }
  }

  //
  // Load device, external programs, and stuff.
  //
  m_core->init();
  m_core->readSettings( config() );

  m_core->deviceManager()->printDevices();

  m_audioServer->setOutputMethod( generalOptions.readEntry( "Audio Output System", "arts" ).local8Bit() );

  emit initializationInfo( i18n("Creating GUI...") );

  m_mainWindow = new K3bMainWindow();
  m_core->m_mainWindow = m_mainWindow;
  m_core->interface()->setMainWindow( m_mainWindow );

  if( isRestored() ) {
    // we only have one single mainwindow to restore
    m_mainWindow->restore(1);
  }
  else {
    setMainWidget( m_mainWindow );

    m_mainWindow->show();

    emit initializationInfo( i18n("Ready.") );

    emit initializationDone();

    K3bFirstRun::run( m_mainWindow );

    if( K3bSystemProblemDialog::readCheckSystemConfig() ) {
      emit initializationInfo( i18n("Checking System") );
      K3bSystemProblemDialog::checkSystem( m_mainWindow );
    }

    if( processCmdLineArgs() )
      KTipDialog::showTip( m_mainWindow );
  }

  // write the current version to make sure checks such as K3bSystemProblemDialog::readCheckSystemConfig
  // use a proper value
  generalOptions.writeEntry( "config version", m_core->version() );
}


int K3bApplication::newInstance()
{
  if( m_needToInit ) {
    //    init();
    m_needToInit = false;
  }
  else
    processCmdLineArgs();

  return KUniqueApplication::newInstance();
}


bool K3bApplication::processCmdLineArgs()
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  bool showTips = true;
  bool dialogOpen = false;

  if( k3bcore->jobsRunning() > 0 ) {
    K3bPassivePopup::showPopup( i18n("K3b is currently busy and cannot start any other operations."),
				i18n("K3b is busy"),
				K3bPassivePopup::Information );
    return true;
  }

  K3bDoc* doc = 0;
  if( args->isSet( "datacd" ) ) {
    doc = m_mainWindow->slotNewDataDoc();
  }
  else if( args->isSet( "audiocd" ) ) {
    doc = m_mainWindow->slotNewAudioDoc();
  }
  else if( args->isSet( "mixedcd" ) ) {
    doc = m_mainWindow->slotNewMixedDoc();
  }
  else if( args->isSet( "videocd" ) ) {
    doc = m_mainWindow->slotNewVcdDoc();
  }
  else if( args->isSet( "emovixcd" ) ) {
    doc = m_mainWindow->slotNewMovixDoc();
  }
  else if( args->isSet( "datadvd" ) ) {
    doc = m_mainWindow->slotNewDvdDoc();
  }
  else if( args->isSet( "emovixdvd" ) ) {
    doc = m_mainWindow->slotNewMovixDvdDoc();
  }
  else if( args->isSet( "videodvd" ) ) {
    doc = m_mainWindow->slotNewVideoDvdDoc();
  }

  // if we created a doc the urls are used to populate it
  if( doc ) {
    KURL::List urls;
    for( int i = 0; i < args->count(); i++ )
      urls.append( args->url(i) );
    dynamic_cast<K3bView*>( doc->view() )->addUrls( urls );
  }
  // otherwise we open them as documents
  else {
    for( int i = 0; i < args->count(); i++ ) {
      m_mainWindow->openDocument( args->url(i) );
    }
  }

  // we only allow one dialog to be opened
  if( args->isSet( "cdimage" ) ) {
    showTips = false;
    dialogOpen = true;
    if( k3bcore->jobsRunning() == 0 ) {
      m_mainWindow->slotWriteCdImage( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "cdimage" ) ) ) );
    }
  }
  else if( args->isSet( "dvdimage" ) ) {
    showTips = false;
    dialogOpen = true;
    if( k3bcore->jobsRunning() == 0 ) {
      m_mainWindow->slotWriteDvdIsoImage( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "dvdimage" ) ) ) );
    }
  }
  else if( args->isSet( "image" ) ) {
    showTips = false;
    dialogOpen = true;
    KURL url = KURL::fromPathOrURL( QFile::decodeName( args->getOption( "image" ) ) );
    if( k3bcore->jobsRunning() == 0 ) {
      if( K3b::filesize( url ) > 1000*1024*1024 )
	m_mainWindow->slotWriteDvdIsoImage( url );
      else
	m_mainWindow->slotWriteCdImage( url );
    }
  }
  else if( args->isSet("copycd") ) {
    showTips = false;
    dialogOpen = true;
      qApp->processEvents();
    m_mainWindow->cdCopy( K3b::urlToDevice( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "copycd" ) ) ) ) );
  }
  else if( args->isSet("copydvd") ) {
    showTips = false;
    dialogOpen = true;
    m_mainWindow->dvdCopy( K3b::urlToDevice( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "copydvd" ) ) ) ) );
  }
  else if( args->isSet("erasecd") ) {
    showTips = false;
    dialogOpen = true;
    m_mainWindow->blankCdrw( K3b::urlToDevice( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "erasecd" ) ) ) ) );
  }
  else if( args->isSet("formatdvd") ) {
    showTips = false;
    dialogOpen = true;
    m_mainWindow->formatDvd( K3b::urlToDevice( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "formatdvd" ) ) ) ) );
  }

  // no dialog used here
  if( args->isSet( "cddarip" ) ) {
    m_mainWindow->cddaRip( K3b::urlToDevice( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "cddarip" ) ) ) ) );
  }
  else if( args->isSet( "videodvdrip" ) ) {
    m_mainWindow->videoDvdRip( K3b::urlToDevice( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "videodvdrip" ) ) ) ) );
  }
  else if( args->isSet( "videocdrip" ) ) {
    m_mainWindow->videoCdRip( K3b::urlToDevice( KURL::fromPathOrURL( QFile::decodeName( args->getOption( "videocdrip" ) ) ) ) );
  }

  if( !dialogOpen && args->isSet( "burn" ) ) {
    if( m_core->projectManager()->activeDoc() ) {
      showTips = false;
      dialogOpen = true;
      static_cast<K3bView*>( m_core->projectManager()->activeDoc()->view() )->slotBurn();
    }
  }

  // FIXME: seems not like the right place...
  if( args->isSet( "ao" ) )
    if( !m_audioServer->setOutputMethod( args->getOption( "ao" ) ) )
      K3bPassivePopup::showPopup( i18n("Could not find Audio Output plugin '%1'").arg( args->getOption("ao") ),
				  i18n("Initialization Problem"),
				  K3bPassivePopup::Warning );

  args->clear();

  return showTips;
}


void K3bApplication::slotShutDown()
{
  k3bappcore->mediaCache()->clearDeviceList();
  K3bThread::waitUntilFinished();
}



K3bApplication::Core::Core( QObject* parent )
  : K3bCore( parent ),
    m_appDeviceManager(0),
    m_mediaCache(0)
{
  s_k3bAppCore = this;
  m_themeManager = new K3bThemeManager( this );
  m_projectManager = new K3bProjectManager( this );
  // we need the themes on startup (loading them is fast anyway :)
  m_themeManager->loadThemes();

  m_jobInterface = new K3bJobInterface( this );
  m_interface = new K3bInterface();
  dcopClient()->setDefaultObject( m_interface->objId() );
}


K3bApplication::Core::~Core()
{
}


void K3bApplication::Core::initDeviceManager()
{
  if( !m_appDeviceManager ) {
    // our very own special device manager
    m_appDeviceManager = new K3bAppDeviceManager( this );
  }
  if( !m_mediaCache ) {
    // create the media cache but do not connect it to the device manager
    // yet to speed up application start. We connect it in init()
    // once the devicemanager has scanned for devices.
    m_mediaCache = new K3bMediaCache( this );
  }

  m_appDeviceManager->setMediaCache( m_mediaCache );
}


K3bDevice::DeviceManager* K3bApplication::Core::deviceManager() const
{
  return appDeviceManager();
}


KConfig* K3bApplication::Core::config() const
{
  return kapp->config();
}


void K3bApplication::Core::init()
{
  //
  // The eMovix program is a special case which is not part of
  // the default programs handled by K3bCore
  //
  initExternalBinManager();
  externalBinManager()->addProgram( new K3bMovixProgram() );
  externalBinManager()->addProgram( new K3bNormalizeProgram() );
  K3b::addTranscodePrograms( externalBinManager() );
  K3b::addVcdimagerPrograms( externalBinManager() );

  K3bCore::init();

  mediaCache()->buildDeviceList( deviceManager() );

  connect( deviceManager(), SIGNAL(changed(K3bDevice::DeviceManager*)),
           mediaCache(), SLOT(buildDeviceList(K3bDevice::DeviceManager*)) );
}


void K3bApplication::Core::readSettings( KConfig* cnf )
{
  K3bCore::readSettings( cnf );

  KConfig* c = cnf;
  if( !c )
    c = config();

  m_themeManager->readConfig( config() );
}


void K3bApplication::Core::saveSettings( KConfig* cnf )
{
  if( !cnf )
    cnf = config();

  K3bCore::saveSettings( cnf );
  m_themeManager->saveConfig( cnf );
}


bool K3bApplication::Core::internalBlockDevice( K3bDevice::Device* dev )
{
  if( K3bCore::internalBlockDevice( dev ) ) {
    if( mediaCache() ) {
      m_deviceBlockMap[dev] = mediaCache()->blockDevice( dev );
    }

#ifdef HAVE_HAL
    if( K3bDevice::HalConnection::instance()->lock( dev ) != K3bDevice::HalConnection::org_freedesktop_Hal_Success )
      kdDebug() << "(K3bInterferingSystemsHandler) HAL lock failed." << endl;
#endif

    //
    // Check if the device is in use
    //
    // FIXME: Use the top level widget as parent
    K3bLsofWrapperDialog::checkDevice( dev );

    return true;
  }
  else
    return false;
}


void K3bApplication::Core::internalUnblockDevice( K3bDevice::Device* dev )
{
  if( mediaCache() ) {
    mediaCache()->unblockDevice( dev, m_deviceBlockMap[dev] );
    m_deviceBlockMap.erase( dev );
  }

#ifdef HAVE_HAL
  K3bDevice::HalConnection::instance()->unlock( dev );
#endif

  K3bCore::internalUnblockDevice( dev );
}

#include "k3bapplication.moc"
