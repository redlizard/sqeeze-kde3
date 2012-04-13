//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//


#include <config.h>

#include <stdlib.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <klocale.h>
#include <qfile.h>
#include <dcopclient.h>
#include <assert.h>

#include "lockeng.h"
#include "lockeng.moc"
#include "kdesktopsettings.h"

#include "xautolock_c.h"
extern xautolock_corner_t xautolock_corners[ 4 ];


//===========================================================================
//
// Screen saver engine. Doesn't handle the actual screensaver window,
// starting screensaver hacks, or password entry. That's done by
// a newly started process.
//
SaverEngine::SaverEngine()
    : DCOPObject("KScreensaverIface"),
      QWidget(),
      mBlankOnly(false)
{
    // Save X screensaver parameters
    XGetScreenSaver(qt_xdisplay(), &mXTimeout, &mXInterval,
                    &mXBlanking, &mXExposures);

    mState = Waiting;
    mXAutoLock = 0;
    mEnabled = false;

    connect(&mLockProcess, SIGNAL(processExited(KProcess *)),
                        SLOT(lockProcessExited()));

    configure();
}

//---------------------------------------------------------------------------
//
// Destructor - usual cleanups.
//
SaverEngine::~SaverEngine()
{
    mLockProcess.detach(); // don't kill it if we crash
    delete mXAutoLock;

    // Restore X screensaver parameters
    XSetScreenSaver(qt_xdisplay(), mXTimeout, mXInterval, mXBlanking,
                    mXExposures);
}

//---------------------------------------------------------------------------

// This should be called only using DCOP.
void SaverEngine::lock()
{
    bool ok = true;
    if (mState == Waiting)
    {
        ok = startLockProcess( ForceLock );
// It takes a while for kdesktop_lock to start and lock the screen.
// Therefore delay the DCOP call until it tells kdesktop that the locking is in effect.
// This is done only for --forcelock .
        if( ok && mState != Saving )
        {
            DCOPClientTransaction* trans = kapp->dcopClient()->beginTransaction();
            mLockTransactions.append( trans );
        }
    }
    else
    {
        mLockProcess.kill( SIGHUP );
    }
}

void SaverEngine::processLockTransactions()
{
    for( QValueVector< DCOPClientTransaction* >::ConstIterator it = mLockTransactions.begin();
         it != mLockTransactions.end();
         ++it )
    {
        QCString replyType = "void";
        QByteArray arr;
        kapp->dcopClient()->endTransaction( *it, replyType, arr );
    }
    mLockTransactions.clear();
}

void SaverEngine::saverLockReady()
{
    if( mState != Preparing )
    {
	kdDebug( 1204 ) << "Got unexpected saverReady()" << endl;
    }
    kdDebug( 1204 ) << "Saver Lock Ready" << endl;
    processLockTransactions();
}

//---------------------------------------------------------------------------
void SaverEngine::save()
{
    if (mState == Waiting)
    {
        startLockProcess( DefaultLock );
    }
}

//---------------------------------------------------------------------------
void SaverEngine::quit()
{
    if (mState == Saving || mState == Preparing)
    {
        stopLockProcess();
    }
}

//---------------------------------------------------------------------------
bool SaverEngine::isEnabled()
{
  return mEnabled;
}

//---------------------------------------------------------------------------
bool SaverEngine::enable( bool e )
{
    if ( e == mEnabled )
	return true;

    // If we aren't in a suitable state, we will not reconfigure.
    if (mState != Waiting)
        return false;

    mEnabled = e;

    if (mEnabled)
    {
	if ( !mXAutoLock ) {
	    mXAutoLock = new XAutoLock();
	    connect(mXAutoLock, SIGNAL(timeout()), SLOT(idleTimeout()));
	}
        mXAutoLock->setTimeout(mTimeout);
        mXAutoLock->setDPMS(true);
	//mXAutoLock->changeCornerLockStatus( mLockCornerTopLeft, mLockCornerTopRight, mLockCornerBottomLeft, mLockCornerBottomRight);

        // We'll handle blanking
        XSetScreenSaver(qt_xdisplay(), mTimeout + 10, mXInterval, PreferBlanking, mXExposures);
        kdDebug() << "XSetScreenSaver " << mTimeout + 10 << endl;

        mXAutoLock->start();

        kdDebug(1204) << "Saver Engine started, timeout: " << mTimeout << endl;
    }
    else
    {
	if (mXAutoLock)
	{
	    delete mXAutoLock;
	    mXAutoLock = 0;
	}

        XForceScreenSaver(qt_xdisplay(), ScreenSaverReset );
        XSetScreenSaver(qt_xdisplay(), 0, mXInterval,  PreferBlanking, DontAllowExposures);
        kdDebug(1204) << "Saver Engine disabled" << endl;
    }

    return true;
}

//---------------------------------------------------------------------------
bool SaverEngine::isBlanked()
{
  return (mState != Waiting);
}

//---------------------------------------------------------------------------
//
// Read and apply configuration.
//
void SaverEngine::configure()
{
    // If we aren't in a suitable state, we will not reconfigure.
    if (mState != Waiting)
        return;

    // create a new config obj to ensure we read the latest options
    KDesktopSettings::self()->readConfig();

    bool e  = KDesktopSettings::screenSaverEnabled();
    mTimeout = KDesktopSettings::timeout();

    mEnabled = !e;   // force the enable()

    int action;
    action = KDesktopSettings::actionTopLeft();
    xautolock_corners[0] = applyManualSettings(action);
    action = KDesktopSettings::actionTopRight();
    xautolock_corners[1] = applyManualSettings(action);
    action = KDesktopSettings::actionBottomLeft();
    xautolock_corners[2] = applyManualSettings(action);
    action = KDesktopSettings::actionBottomRight();
    xautolock_corners[3] = applyManualSettings(action);

    enable( e );
}

//---------------------------------------------------------------------------
//
//  Set a variable to indicate only using the blanker and not the saver.
//
void SaverEngine::setBlankOnly( bool blankOnly )
{
	mBlankOnly = blankOnly;
	// FIXME: if running, stop  and restart?  What about security
	// implications of this?
}

//---------------------------------------------------------------------------
//
// Start the screen saver.
//
bool SaverEngine::startLockProcess( LockType lock_type )
{
    if (mState != Waiting)
        return true;

    kdDebug(1204) << "SaverEngine: starting saver" << endl;
    emitDCOPSignal("KDE_start_screensaver()", QByteArray());

    if (mLockProcess.isRunning())
    {
        stopLockProcess();
    }
    mLockProcess.clearArguments();
    QString path = KStandardDirs::findExe( "kdesktop_lock" );
    if( path.isEmpty())
    {
	kdDebug( 1204 ) << "Can't find kdesktop_lock!" << endl;
	return false;
    }
    mLockProcess << path;
    switch( lock_type )
    {
	case ForceLock:
    	    mLockProcess << QString( "--forcelock" );
	  break;
	case DontLock:
	    mLockProcess << QString( "--dontlock" );
	  break;
	default:
	  break;
    }
    if (mBlankOnly)
	    mLockProcess << QString( "--blank" );

    if (mLockProcess.start() == false )
    {
	kdDebug( 1204 ) << "Failed to start kdesktop_lock!" << endl;
	return false;
    }
    XSetScreenSaver(qt_xdisplay(), 0, mXInterval,  PreferBlanking, mXExposures);

    mState = Preparing;
    if (mXAutoLock)
    {
        mXAutoLock->stop();
    }
    return true;
}

//---------------------------------------------------------------------------
//
// Stop the screen saver.
//
void SaverEngine::stopLockProcess()
{
    if (mState == Waiting)
    {
        kdWarning(1204) << "SaverEngine::stopSaver() saver not active" << endl;
        return;
    }
    kdDebug(1204) << "SaverEngine: stopping lock" << endl;
    emitDCOPSignal("KDE_stop_screensaver()", QByteArray());

    mLockProcess.kill();

    if (mEnabled)
    {
        if (mXAutoLock)
        {
            mXAutoLock->start();
        }
        XForceScreenSaver(qt_xdisplay(), ScreenSaverReset );
        XSetScreenSaver(qt_xdisplay(), mTimeout + 10, mXInterval, PreferBlanking, mXExposures);
    }
    processLockTransactions();
    mState = Waiting;
}

void SaverEngine::lockProcessExited()
{
    kdDebug(1204) << "SaverEngine: lock exited" << endl;
    if( mState == Waiting )
	return;
    emitDCOPSignal("KDE_stop_screensaver()", QByteArray());
    if (mEnabled)
    {
        if (mXAutoLock)
        {
            mXAutoLock->start();
        }
        XForceScreenSaver(qt_xdisplay(), ScreenSaverReset );
        XSetScreenSaver(qt_xdisplay(), mTimeout + 10, mXInterval, PreferBlanking, mXExposures);
    }
    processLockTransactions();
    mState = Waiting;
}

//---------------------------------------------------------------------------
//
// XAutoLock has detected the required idle time.
//
void SaverEngine::idleTimeout()
{
    // disable X screensaver
    XForceScreenSaver(qt_xdisplay(), ScreenSaverReset );
    XSetScreenSaver(qt_xdisplay(), 0, mXInterval, PreferBlanking, DontAllowExposures);
    startLockProcess( DefaultLock );
}

xautolock_corner_t SaverEngine::applyManualSettings(int action)
{
	if (action == 0)
	{
		kdDebug() << "no lock" << endl;
		return ca_nothing;
	}
	else
	if (action == 1)
	{
		kdDebug() << "lock screen" << endl;
		return ca_forceLock;
	}
	else
	if (action == 2)
	{
		kdDebug() << "prevent lock" << endl;
		return ca_dontLock;
	}
	else
	{
		kdDebug() << "no lock nothing" << endl;
		return ca_nothing;
	}
}
