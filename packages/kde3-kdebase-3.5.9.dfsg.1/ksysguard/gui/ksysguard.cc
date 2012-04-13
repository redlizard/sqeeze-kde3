/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger
    <cs@kde.org>. Please do not commit any changes without consulting
    me first. Thanks!

    KSysGuard has been written with some source code and ideas from
    ktop (<1.0). Early versions of ktop have been written by Bernd
    Johannes Wuebben <wuebben@math.cornell.edu> and Nicolas Leclercq
    <nicknet@planete.net>.

*/

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksgrd/SensorAgent.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <dnssd/remoteservice.h>


#include "../version.h"
#include "SensorBrowser.h"
#include "Workspace.h"

#include "ksysguard.h"

static const char Description[] = I18N_NOOP( "KDE system guard" );
TopLevel* topLevel;

/**
  This is the constructor for the main widget. It sets up the menu and the
  TaskMan widget.
 */
TopLevel::TopLevel( const char *name )
  : KMainWindow( 0, name ), DCOPObject( "KSysGuardIface" )
{
  setPlainCaption( i18n( "KDE System Guard" ) );
  mDontSaveSession = false;
  mTimerId = -1;

  mSplitter = new QSplitter( this );
  mSplitter->setOrientation( Horizontal );
  mSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  setCentralWidget( mSplitter );

  mSensorBrowser = new SensorBrowser( mSplitter, KSGRD::SensorMgr );
  
  mServiceBrowser = new DNSSD::ServiceBrowser("_ksysguard._tcp", 0, true);
  connect(mServiceBrowser,SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),this,
  	SLOT(serviceAdded(DNSSD::RemoteService::Ptr)));

  mWorkSpace = new Workspace( mSplitter );
  connect( mWorkSpace, SIGNAL( announceRecentURL( const KURL& ) ),
           SLOT( registerRecentURL( const KURL& ) ) );
  connect( mWorkSpace, SIGNAL( setCaption( const QString&, bool ) ),
           SLOT( setCaption( const QString&, bool ) ) );
  connect( KSGRD::Style, SIGNAL( applyStyleToWorksheet() ), mWorkSpace,
           SLOT( applyStyle() ) );

  /* Create the status bar. It displays some information about the
   * number of processes and the memory consumption of the local
   * host. */
  statusBar()->insertFixedItem( i18n( "88888 Processes" ), 0 );
  statusBar()->insertFixedItem( i18n( "Memory: 88888888888 kB used, "
                                     "88888888888 kB free" ), 1 );
  statusBar()->insertFixedItem( i18n( "Swap: 888888888 kB used, "
                                     "888888888 kB free" ), 2 );
  statusBar()->hide();

  // create actions for menue entries
  new KAction( i18n( "&New Worksheet..." ), "tab_new", 0, mWorkSpace,
		   SLOT( newWorkSheet() ), actionCollection(), "new_worksheet" );
  
  new KAction( i18n( "Import Worksheet..." ), "fileopen", 0, mWorkSpace,
		   SLOT( loadWorkSheet() ), actionCollection(), "import_worksheet" );
  
  mActionOpenRecent = new KRecentFilesAction( i18n( "&Import Recent Worksheet" ),"fileopen", 0,
		   mWorkSpace, SLOT( loadWorkSheet( const KURL& ) ), actionCollection(), "recent_import_worksheet" );	
  
  new KAction( i18n( "&Remove Worksheet" ), "tab_remove", 0, mWorkSpace,
		   SLOT( deleteWorkSheet() ), actionCollection(), "remove_worksheet" );

  new KAction( i18n( "&Export Worksheet..." ), "filesaveas", 0, mWorkSpace,
		   SLOT( saveWorkSheetAs() ), actionCollection(), "export_worksheet" );
   
  KStdAction::quit( this, SLOT( close() ), actionCollection() );

  new KAction( i18n( "C&onnect Host..." ), "connect_established", 0, this,
               SLOT( connectHost() ), actionCollection(), "connect_host" );
  new KAction( i18n( "D&isconnect Host" ), "connect_no", 0, this,
               SLOT( disconnectHost() ), actionCollection(), "disconnect_host" );

//  KStdAction::cut( mWorkSpace, SLOT( cut() ), actionCollection() );
//  KStdAction::copy( mWorkSpace, SLOT( copy() ), actionCollection() );
//  KStdAction::paste( mWorkSpace, SLOT( paste() ), actionCollection() );
  new KAction( i18n( "&Worksheet Properties" ), "configure", 0, mWorkSpace,
               SLOT( configure() ), actionCollection(), "configure_sheet" );

  new KAction( i18n( "Load Standard Sheets" ), "revert",
               0, this, SLOT( resetWorkSheets() ),
               actionCollection(), "revert_all_worksheets"  );

  new KAction( i18n( "Configure &Style..." ), "colorize", 0, this,
               SLOT( editStyle() ), actionCollection(), "configure_style" );

  // TODO remove resize and fix so sizeHints() determines default size.
  if (!initialGeometrySet())
    resize( 640, 480 );
  setupGUI(ToolBar | Keys | StatusBar | Create);
  setAutoSaveSettings();
}


/*
 * DCOP Interface functions
 */
void TopLevel::resetWorkSheets()
{
  if ( KMessageBox::warningContinueCancel( this,
       i18n( "Do you really want to restore the default worksheets?" ),
       i18n( "Reset All Worksheets" ),
       i18n("Reset"),
       "AskResetWorkSheets") == KMessageBox::Cancel )
    return;

  mWorkSpace->removeAllWorkSheets();

  KStandardDirs* kstd = KGlobal::dirs();
  kstd->addResourceType( "data", "share/apps/ksysguard" );

  QString workDir = kstd->saveLocation( "data", "ksysguard" );

  QString file = kstd->findResource( "data", "SystemLoad.sgrd" );
  QString newFile = workDir + "/" + i18n( "System Load" ) + ".sgrd";
  if ( !file.isEmpty() )
    mWorkSpace->restoreWorkSheet( file, newFile );

  file = kstd->findResource( "data", "ProcessTable.sgrd" );
  newFile = workDir + "/" + i18n( "Process Table" ) + ".sgrd";
  if ( !file.isEmpty() )
    mWorkSpace->restoreWorkSheet( file, newFile );
}

void TopLevel::showProcesses()
{
  mWorkSpace->showProcesses();
}

void TopLevel::showOnCurrentDesktop()
{
  KWin::setOnDesktop( winId(), KWin::currentDesktop() );
  kapp->updateUserTimestamp();
  KWin::forceActiveWindow( winId() );
}

void TopLevel::loadWorkSheet( const QString &fileName )
{
  mWorkSpace->loadWorkSheet( KURL( fileName ) );
}

void TopLevel::removeWorkSheet( const QString &fileName )
{
  mWorkSpace->deleteWorkSheet( fileName );
}

QStringList TopLevel::listSensors( const QString &hostName )
{
  return mSensorBrowser->listSensors( hostName );
}

QStringList TopLevel::listHosts()
{
  return mSensorBrowser->listHosts();
}

QString TopLevel::readIntegerSensor( const QString &sensorLocator )
{
  QString host = sensorLocator.left( sensorLocator.find( ':' ) );
  QString sensor = sensorLocator.right( sensorLocator.length() -
                                        sensorLocator.find( ':' ) - 1 );

  DCOPClientTransaction *dcopTransaction = kapp->dcopClient()->beginTransaction();
  mDCopFIFO.prepend( dcopTransaction );

  KSGRD::SensorMgr->engage( host, "", "ksysguardd" );
  KSGRD::SensorMgr->sendRequest( host, sensor, (KSGRD::SensorClient*)this, 133 );

  return QString::null;
}

QStringList TopLevel::readListSensor( const QString& sensorLocator )
{
  QStringList retval;

  QString host = sensorLocator.left( sensorLocator.find( ':' ) );
  QString sensor = sensorLocator.right( sensorLocator.length() -
                                        sensorLocator.find( ':' ) - 1 );

  DCOPClientTransaction *dcopTransaction = kapp->dcopClient()->beginTransaction();
  mDCopFIFO.prepend( dcopTransaction );

  KSGRD::SensorMgr->engage( host, "", "ksysguardd" );
  KSGRD::SensorMgr->sendRequest( host, sensor, (KSGRD::SensorClient*)this, 134 );

  return retval;
}
/*
 * End of DCOP Interface section
 */
 
void TopLevel::serviceAdded(DNSSD::RemoteService::Ptr srv)
{
  KSGRD::SensorMgr->engage( srv->hostName(), "", "", srv->port() );
}

void TopLevel::registerRecentURL( const KURL &url )
{
  mActionOpenRecent->addURL( url );
}

void TopLevel::beATaskManager()
{
  mWorkSpace->showProcesses();

  // Avoid displaying splitter widget
  mSensorBrowser->hide();

  // No toolbar and status bar in taskmanager mode.
  toolBar( "mainToolBar" )->hide();

  mDontSaveSession = true;

  stateChanged( "showProcessState" );
}

void TopLevel::showRequestedSheets()
{
  toolBar( "mainToolBar" )->hide();

  QValueList<int> sizes;
  sizes.append( 0 );
  sizes.append( 100 );
  mSplitter->setSizes( sizes );
}

void TopLevel::initStatusBar()
{
  KSGRD::SensorMgr->engage( "localhost", "", "ksysguardd" );
  /* Request info about the swap space size and the units it is
   * measured in.  The requested info will be received by
   * answerReceived(). */
  KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/used?",
                                 (KSGRD::SensorClient*)this, 5 );
  updateStatusBar();
  mServiceBrowser->startBrowse();
  
  KToggleAction *sb = dynamic_cast<KToggleAction*>(action("options_show_statusbar"));
  if (sb)
     connect(sb, SIGNAL(toggled(bool)), this, SLOT(updateStatusBar()));
}

void TopLevel::updateStatusBar()
{
  if ( mTimerId == -1 )
    mTimerId = startTimer( 2000 );

  // call timerEvent to fill the status bar with real values
  timerEvent( 0 );
}

void TopLevel::connectHost()
{
  KSGRD::SensorMgr->engageHost( "" );
}

void TopLevel::disconnectHost()
{
  mSensorBrowser->disconnect();
}

void TopLevel::editToolbars()
{
  saveMainWindowSettings( kapp->config() );
  KEditToolbar dlg( actionCollection() );
  connect( &dlg, SIGNAL( newToolbarConfig() ), this,
           SLOT( slotNewToolbarConfig() ) );

  dlg.exec();
}

void TopLevel::slotNewToolbarConfig()
{
  createGUI();
  applyMainWindowSettings( kapp->config() );
}

void TopLevel::editStyle()
{
  KSGRD::Style->configure();
}

void TopLevel::customEvent( QCustomEvent *e )
{
  if ( e->type() == QEvent::User ) {
    /* Due to the asynchronous communication between ksysguard and its
     * back-ends, we sometimes need to show message boxes that were
     * triggered by objects that have died already. */
    KMessageBox::error( this, *((QString*)e->data()) );
    delete (QString*)e->data();
  }
}

void TopLevel::timerEvent( QTimerEvent* )
{
  if ( statusBar()->isVisibleTo( this ) ) {
    /* Request some info about the memory status. The requested
     * information will be received by answerReceived(). */
    KSGRD::SensorMgr->sendRequest( "localhost", "pscount",
                                   (KSGRD::SensorClient*)this, 0 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/physical/free",
                                   (KSGRD::SensorClient*)this, 1 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/physical/used",
                                   (KSGRD::SensorClient*)this, 2 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/free",
                                   (KSGRD::SensorClient*)this, 3 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/used",
                                   (KSGRD::SensorClient*)this, 4 );
  }
}

bool TopLevel::queryClose()
{
  if ( !mDontSaveSession ) {
    if ( !mWorkSpace->saveOnQuit() )
      return false;

    saveProperties( kapp->config() );
    kapp->config()->sync();
  }

  return true;
}

void TopLevel::readProperties( KConfig *cfg )
{
  /* we can ignore 'isMaximized' because we can't set the window
     maximized, so we save the coordinates instead */
  if ( cfg->readBoolEntry( "isMinimized" ) == true )
    showMinimized();

  QValueList<int> sizes = cfg->readIntListEntry( "SplitterSizeList" );
  if ( sizes.isEmpty() ) {
    // start with a 30/70 ratio
    sizes.append( 30 );
    sizes.append( 70 );
  }
  mSplitter->setSizes( sizes );

  KSGRD::SensorMgr->readProperties( cfg );
  KSGRD::Style->readProperties( cfg );

  mWorkSpace->readProperties( cfg );

  mActionOpenRecent->loadEntries( cfg );

  applyMainWindowSettings( cfg );
}

void TopLevel::saveProperties( KConfig *cfg )
{
  mActionOpenRecent->saveEntries( cfg );

  cfg->writeEntry( "isMinimized", isMinimized() );
  cfg->writeEntry( "SplitterSizeList", mSplitter->sizes() );

  KSGRD::Style->saveProperties( cfg );
  KSGRD::SensorMgr->saveProperties( cfg );

  saveMainWindowSettings( cfg );
  mWorkSpace->saveProperties( cfg );
}

void TopLevel::answerReceived( int id, const QString &answer )
{
  QString s;
  static QString unit;
  static long mUsed = 0;
  static long mFree = 0;
  static long sUsed = 0;
  static long sFree = 0;

  switch ( id ) {
    case 0:
      // yes, I know there is never 1 process, but that's the way
      // singular vs. plural works :/
      //
      // To use pluralForms, though, you need to convert to
      // an integer, not use the QString straight.
      s = i18n( "1 Process", "%n Processes", answer.toInt() );
      statusBar()->changeItem( s, 0 );
      break;

    case 1:
      mFree = answer.toLong();
      break;

    case 2:
      mUsed = answer.toLong();
      s = i18n( "Memory: %1 %2 used, %3 %4 free" )
              .arg( KGlobal::locale()->formatNumber( mUsed, 0 ) ).arg( unit )
              .arg( KGlobal::locale()->formatNumber( mFree, 0 ) ).arg( unit );
      statusBar()->changeItem( s, 1 );
      break;

    case 3:
      sFree = answer.toLong();
      setSwapInfo( sUsed, sFree, unit );
      break;

    case 4:
      sUsed = answer.toLong();
      setSwapInfo( sUsed, sFree, unit );
      break;

    case 5: {
      KSGRD::SensorIntegerInfo info( answer );
      unit = KSGRD::SensorMgr->translateUnit( info.unit() );
    }

    case 133: {
      QCString replyType = "QString";
      QByteArray replyData;
      QDataStream reply( replyData, IO_WriteOnly );
      reply << answer;

      DCOPClientTransaction *dcopTransaction = mDCopFIFO.last();
      kapp->dcopClient()->endTransaction( dcopTransaction, replyType, replyData );
      mDCopFIFO.removeLast();
      break;
    }

    case 134: {
      QStringList resultList;
      QCString replyType = "QStringList";
      QByteArray replyData;
      QDataStream reply( replyData, IO_WriteOnly );

      KSGRD::SensorTokenizer lines( answer, '\n' );

      for ( unsigned int i = 0; i < lines.count(); i++ )
        resultList.append( lines[ i ] );

      reply << resultList;

      DCOPClientTransaction *dcopTransaction = mDCopFIFO.last();
      kapp->dcopClient()->endTransaction( dcopTransaction, replyType, replyData );
      mDCopFIFO.removeLast();
      break;
    }
  }
}

void TopLevel::setSwapInfo( long used, long free, const QString &unit )
{
  QString msg;
  if ( used == 0 && free == 0 ) // no swap available
    msg = i18n( "No swap space available" );
  else {
    msg = i18n( "Swap: %1 %2 used, %3 %4 free" )
              .arg( KGlobal::locale()->formatNumber( used, 0 ) ).arg( unit )
              .arg( KGlobal::locale()->formatNumber( free, 0 ) ).arg( unit );
  }

  statusBar()->changeItem( msg, 2 );
}

static const KCmdLineOptions options[] = {
  { "showprocesses", I18N_NOOP( "Show only process list of local host" ), 0 },
  { "+[worksheet]", I18N_NOOP( "Optional worksheet files to load" ), 0 },
  KCmdLineLastOption
};

/*
 * Once upon a time...
 */
int main( int argc, char** argv )
{
  // initpipe is used to keep the parent process around till the child
  // has registered with dcop.
  int initpipe[ 2 ];
  pipe( initpipe );

  /* This forking will put ksysguard in it's on session not having a
   * controlling terminal attached to it. This prevents ssh from
   * using this terminal for password requests. Unfortunately you
   * now need a ssh with ssh-askpass support to popup an X dialog to
   * enter the password. Currently only the original ssh provides this
   * but not open-ssh. */

  pid_t pid;
  if ( ( pid = fork() ) < 0 )
    return -1;
  else
    if ( pid != 0 ) {
      close( initpipe[ 1 ] );

      // wait till init is complete
      char c;
      while( read( initpipe[ 0 ], &c, 1 ) < 0 );

      // then exit
      close( initpipe[ 0 ] );
      exit( 0 );
    }

  close( initpipe[ 0 ] );
  setsid();

  KAboutData aboutData( "ksysguard", I18N_NOOP( "KDE System Guard" ),
                        KSYSGUARD_VERSION, Description, KAboutData::License_GPL,
                        I18N_NOOP( "(c) 1996-2002 The KSysGuard Developers" ) );
  aboutData.addAuthor( "Chris Schlaeger", "Current Maintainer", "cs@kde.org" );
  aboutData.addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  aboutData.addAuthor( "Nicolas Leclercq", 0, "nicknet@planete.net" );
  aboutData.addAuthor( "Alex Sanda", 0, "alex@darkstart.ping.at" );
  aboutData.addAuthor( "Bernd Johannes Wuebben", 0, "wuebben@math.cornell.edu" );
  aboutData.addAuthor( "Ralf Mueller", 0, "rlaf@bj-ig.de" );
  aboutData.addAuthor( "Hamish Rodda", 0, "rodda@kde.org" );
  aboutData.addAuthor( "Torsten Kasch", I18N_NOOP( "Solaris Support\n"
                       "Parts derived (by permission) from the sunos5\n"
                       "module of William LeFebvre's \"top\" utility." ),
                       "tk@Genetik.Uni-Bielefeld.DE" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication::disableAutoDcopRegistration();
  // initialize KDE application
  KApplication *app = new KApplication;

  KSGRD::SensorMgr = new KSGRD::SensorManager();
  KSGRD::Style = new KSGRD::StyleEngine();

  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

  int result = 0;

  if ( args->isSet( "showprocesses" ) ) {
    /* To avoid having multiple instances of ksysguard in
     * taskmanager mode we check if another taskmanager is running
     * already. If so, we terminate this one immediately. */
    if ( app->dcopClient()->registerAs( "ksysguard_taskmanager", false ) ==
                                                    "ksysguard_taskmanager" ) {
      // We have registered with DCOP, our parent can exit now.
      char c = 0;
      write( initpipe[ 1 ], &c, 1 );
      close( initpipe[ 1 ] );

      topLevel = new TopLevel( "KSysGuard" );
      topLevel->beATaskManager();
      topLevel->initStatusBar();
      topLevel->show();
      KSGRD::SensorMgr->setBroadcaster( topLevel );

      // run the application
      result = app->exec();
    } else {
      QByteArray data;
      app->dcopClient()->send( "ksysguard_taskmanager", "KSysGuardIface",
                               "showOnCurrentDesktop()", data );
    }
  } else {
    app->dcopClient()->registerAs( "ksysguard" );
    app->dcopClient()->setDefaultObject( "KSysGuardIface" );

    // We have registered with DCOP, our parent can exit now.
    char c = 0;
    write( initpipe[ 1 ], &c, 1 );
    close( initpipe[ 1 ] );

    topLevel = new TopLevel( "KSysGuard" );

    // create top-level widget
    if ( args->count() > 0 ) {
      /* The user has specified a list of worksheets to load. In this
       * case we do not restore any previous settings but load all the
       * requested worksheets. */
      topLevel->showRequestedSheets();
      for ( int i = 0; i < args->count(); ++i )
        topLevel->loadWorkSheet( args->arg( i ) );
    } else {
      if ( app->isRestored() )
        topLevel->restore( 1 );
      else
        topLevel->readProperties( app->config() );
    }

    topLevel->initStatusBar();
    topLevel->show();
    KSGRD::SensorMgr->setBroadcaster( topLevel );

    // run the application
    result = app->exec();
  }

  delete KSGRD::Style;
  delete KSGRD::SensorMgr;
  delete app;

  return result;
}

#include "ksysguard.moc"
