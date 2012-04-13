/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003 <ravi@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include <kwin.h>
#include <dcopclient.h>

#include <qdir.h>
#include <qpixmap.h>
#include <qtimer.h>

#include "objkstheme.h"
#include "wndmain.h"
#include "wndmain.moc"

#include "themeengine.h"
#include "themelegacy.h"

// KSplash::KSplash(): This is a hidden object. Its sole purpose
// is to manage the other objects, which are presented on the screen.
KSplash::KSplash(const char *name)
    : DCOPObject( name ),  QWidget( 0, name, WStyle_Customize|WStyle_NoBorder|WX11BypassWM ),
      mState( 0 ), mMaxProgress( 0 ), mStep( 0 )
{
  hide(); // We never show this object.
  mThemeLibName = QString::null;
  mSessMgrCalled = false;
  mTimeToGo = false;

  KConfig * config = kapp->config();
  slotReadProperties(config);

  prepareSplashScreen();
  prepareIconList();

  mCurrentAction = mActionList.first();

  config->setGroup( "General" );
  if ( config->readBoolEntry( "CloseOnClick", TRUE ) )
    mThemeEngine->installEventFilter( this );

  connect( mThemeEngine, SIGNAL(destroyed()), this, SLOT(close()) );
  connect( this, SIGNAL(stepsChanged(int)), SLOT(slotUpdateSteps(int)) );
  connect( this, SIGNAL(progressChanged(int)), SLOT(slotUpdateProgress(int)) );

  if( mKsTheme->testing() )
  {
    slotUpdateSteps(7);
    QTimer::singleShot( 1000, this, SLOT(slotExec()));
  }
  else
    QTimer::singleShot( 100, this, SLOT(initDcop()));

  // Make sure we don't stay up forever.
  if (!mKsTheme->managedMode())
  {
    close_timer = new QTimer( this );
    connect( close_timer, SIGNAL( timeout() ), this, SLOT( close() ) );
    close_timer->start( 60000, TRUE );
  }
}

KSplash::~KSplash()
{
  delete mThemeEngine;
  delete mKsTheme;
  delete close_timer;
  if (!mThemeLibName.isEmpty())
    KLibLoader::self()->unloadLibrary( mThemeLibName.latin1() );
}

void KSplash::slotReadProperties( KConfig *config )
{
  KCmdLineArgs *arg = KCmdLineArgs::parsedArgs();
  mTheme = arg->getOption("theme");
  if (mTheme.isEmpty())
  {
    config->setGroup( "KSplash" );
    mTheme = config->readEntry( "Theme", "Default" );
  }
  loadTheme( mTheme ); // Guaranteed to return a valid theme.
}

void KSplash::prepareIconList()
{
  // Managed mode icons are specified via DCOP.
  if( mKsTheme->managedMode() )
    return;

  slotInsertAction( mKsTheme->icon( 1 ), mKsTheme->text( 1 ) );

  mCurrentAction = mActionList.first();
  slotSetText( mCurrentAction->ItemText );
  slotSetPixmap( mCurrentAction->ItemPixmap );
  emit progressChanged( mStep );

  for (int indx = 2; indx <= 8; indx++)
    slotInsertAction( mKsTheme->icon( indx ), mKsTheme->text( indx ) );
}

void KSplash::prepareSplashScreen()
{
  mThemeEngine->show();
}

void KSplash::slotInsertAction( const QString& pix, const QString& msg )
{
  Action *a = new Action;
  a->ItemText = msg;
  a->ItemPixmap = pix;
  mActionList.append( a );
}

void KSplash::slotExec()
{
  QTimer::singleShot( 200, this, SLOT(nextIcon()));
}

void KSplash::nextIcon()
{
  if( !mCurrentAction || mTimeToGo )
  {
    QTimer::singleShot( 1000, this, SLOT(close()));
    return;
  }

  mCurrentAction = mActionList.next();

  if( mCurrentAction )
  {
    slotSetText( mCurrentAction->ItemText );
    slotSetPixmap( mCurrentAction->ItemPixmap );
    emit progressChanged( ++mStep );
  }

  if( mKsTheme->testing() )
    QTimer::singleShot( 1000, this, SLOT(nextIcon()));
}

void KSplash::initDcop()
{
  disconnect( kapp->dcopClient(), SIGNAL( attachFailed(const QString&) ), kapp, SLOT( dcopFailure(const QString&) ) );

  if ( kapp->dcopClient()->isAttached() )
    return;

  if ( kapp->dcopClient()->attach() )
  {
    if(!mKsTheme->managedMode())
      upAndRunning( "dcop" );
    kapp->dcopClient()->registerAs( "ksplash", false );
    kapp->dcopClient()->setDefaultObject( objId() );
  }
  else
  {
    QTimer::singleShot( 100, this, SLOT(initDcop()) );
  }
}

void KSplash::updateState( unsigned int state )
{
// The whole state updating in ksplashml is simply weird,
// nextIcon() and also the themes naively assume all states
// will come, and will come in the expected order, which
// is not guaranteed, and can happen easily with faster machines.
// And upAndRunning() even is written to handle it gracefully.
  while( state > mState )
  {
    ++mState;
    nextIcon();
  }
}

// For KDE startup only.
void KSplash::upAndRunning( QString s )
{
// This code is written to match ksmserver. Touch it without knowing
// what you are doing and prepare to bite the dust.
  bool update = true;
  static bool firstTime = true;

  if (firstTime)
  {
    emit stepsChanged(7);
    firstTime = false;
  }
  if ( close_timer->isActive() )
    close_timer->start( 60000, TRUE );

  if( s == "dcop" )
  {
    if( mState > 1 ) return;
    updateState( 1 );
    mStep = 1;
  }
  else if( s == "kded" )
  {
    if( mState > 2 ) return;
    updateState( 2 );
    mStep = 2;
  }
  else if( s == "kcminit" )
    ; // No icon
  else if( s == "ksmserver" )
  {
    if( mState > 3 ) return;
    updateState( 3 );
    mStep = 3;
  }
  else if( s == "wm started" )
  {
    if( mState > 4 ) return;
    updateState( 4 );
    mStep = 4;
  }
  else if( s == "kdesktop" )
  {
    if( mState > 5 ) return;
    updateState( 5 );
    mStep = 5;
  }
  else if( s == "kicker" || s == "session ready" )
  {
    updateState( 7 );
    mStep = 9;
    //if(!mSessMgrCalled) emit nextIcon();
    mTimeToGo = true;
    close_timer->stop();
    QTimer::singleShot( 1000, this, SLOT(close()));
  }
  else
  {
    kdDebug() << "KSplash::upAndRunning(): bad s: " << s << endl;
    update = false;
  }
}

// For KDE startup only.
void KSplash::setMaxProgress(int max)
{
  if( max < 1 )
      max = 1;
  if( mThemeEngine && mState >= 6 ) // show the progressbar only after kicker is ready
    mThemeEngine->slotUpdateSteps( max );
  mMaxProgress = max;
}

// For KDE startup only.
void KSplash::setProgress(int step)
{
  if( mThemeEngine )
    mThemeEngine->slotUpdateProgress( mMaxProgress - step );
}


/*
 * When a program starts, it sends a generic signal to KSplash indicating
 * (a) which icon is to be displayed to the user //OR// a generic token-name
 * indicating that KSplash can load a pre-configured icon, (b) the textual
 * name of the process being started, and (c) a description of what the
 * process is handling. Some examples:
 *
 * programStarted( QString("desktop"), QString("kdesktop"), QString("Preparing your desktop..."));
 */
void KSplash::programStarted( QString icon, QString name, QString desc )
{
  if (mTimeToGo)
    return;
  // No isEmpty() here: empty strings are handled by the plugins and can be passed to update the counter.
  if (name.isNull() && icon.isNull() && desc.isNull())
    return;

  slotInsertAction( icon, desc );
  mCurrentAction = mActionList.next();
  slotSetText( desc );
  slotSetPixmap( icon );
  emit progressChanged( ++mStep );
}

void KSplash::setStartupItemCount( int count )
{
  emit stepsChanged( count );
  emit progressChanged( mStep );
}

void KSplash::startupComplete()
{
  mTimeToGo = true;
  QTimer::singleShot( 1000, this, SLOT(close()));
}

void KSplash::close()
{
  QWidget::close();
}

void KSplash::hide()
{
  QWidget::hide();
}

void KSplash::show()
{
  QWidget::show();
}

// Guaranteed to return a valid theme.
void KSplash::loadTheme( const QString& theme )
{
  mKsTheme = new ObjKsTheme( theme );
  // kdDebug() << "KSplash::loadTheme: " << theme << " : "<< mKsTheme->themeEngine() << endl;
  mThemeEngine = _loadThemeEngine( mKsTheme->themeEngine(), theme );
  if (!mThemeEngine)
  {
    mThemeEngine = new ThemeDefault( this, "", theme );
    kdDebug() << "Standard theme loaded." << endl;
  }
  // The theme engine we get may not be the theme engine we requested.
  delete mKsTheme;
  mKsTheme = mThemeEngine->ksTheme();
}

ThemeEngine *KSplash::_loadThemeEngine( const QString& pluginName, const QString& theme )
{
  // Since we may be called before the DCOP server is active, we cannot use the KTrader framework for obtaining plugins. In its
  // place, we use the following naive heuristic to locate plugins. If we are not in managed mode, and we are not in testing mode
  // either, we assume that we have been called by startkde. In this case, we simply try to load the library whose name should
  // conform to the following specification:
  //       QString("ksplash") + pluginName.lower()
  // The object should be called as follows:
  //       QString("Theme") + pluginName
  KLibFactory *factory = 0L;
  QString libName;
  QString objName;
  // Replace this test by a "nodcop" command line option.
  if ( /*!mKsTheme->managedMode() ||*/ !KCmdLineArgs::parsedArgs()->isSet( "dcop" ) )
  {
    libName = QString("ksplash%1").arg(pluginName.lower());
    objName = QString("Theme%1").arg(pluginName);
    // kdDebug() << "*KSplash::_loadThemeEngine: Loading " << objName << " from " << libName << endl;
    // libname.latin1() instead of QFile::encodeName() because these are not user-modifiable files.
    if ( (factory = KLibLoader::self()->factory ( libName.latin1() )) )
      mThemeLibName = libName;
  }
  else
  {
    // Fancier way of locating plugins.
    KService::List list= KTrader::self()->query("KSplash/Plugin", QString("[X-KSplash-PluginName] == '%1'").arg(pluginName));
    KService::Ptr ptr;
    if (!list.isEmpty())
    {
      ptr = list.first();
      // libname.latin1() instead of QFile::encodeName() because these are not user-modifiable files.
      if( (factory = KLibLoader::self()->factory( ptr->library().latin1() )) )
      {
        mThemeLibName = ptr->library();
        objName = ptr->property("X-KSplash-ObjectName").toString();
      }
    }
  }
  if (factory)
  {
    QStringList themeTitle;
    themeTitle << theme;
    return static_cast<ThemeEngine *>(factory->create(this, "theme", objName.latin1(), themeTitle));
  }
  else
    return 0L;
}

void KSplash::slotSetText( const QString& s )
{
  if( mThemeEngine )
    mThemeEngine->slotSetText( s );
}

void KSplash::slotSetPixmap( const QString& px )
{
  if( mThemeEngine )
    mThemeEngine->slotSetPixmap( px );
}

void KSplash::slotUpdateSteps( int )
{
// ??
}

void KSplash::slotUpdateProgress( int )
{
// ??
}

QPtrList<Action> KSplash::actionList()
{
  return mActionList;
}

bool KSplash::eventFilter( QObject *o, QEvent *e )
{
  if ( ( e->type() == QEvent::MouseButtonRelease ) && ( o == mThemeEngine ) )
  {
    QTimer::singleShot( 0, this, SLOT(close()));
    return TRUE;
  }
  else
    return FALSE;
}
