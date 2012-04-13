/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2005 Lubos Lunak <l.lunak@kde.org>

relatively small extensions by Oswald Buddenhagen <ob6@inf.tu-dresden.de>

some code taken from the dcopserver (part of the KDE libraries), which is
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
Copyright (c) 1999 Preston Brown <pbrown@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pwd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qptrstack.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qguardedptr.h>
#include <qtimer.h>

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <unistd.h>
#include <kapplication.h>
#include <kstaticdeleter.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kwinmodule.h>
#include <knotifyclient.h>

#include "server.h"
#include "global.h"
#include "client.h"

#include <kdebug.h>

/*!  Restores the previous session. Ensures the window manager is
  running (if specified).
 */
void KSMServer::restoreSession( QString sessionName )
{
    if( state != Idle )
        return;
    state = LaunchingWM;

    kdDebug( 1218 ) << "KSMServer::restoreSession " << sessionName << endl;
    upAndRunning( "restore session");
    KConfig* config = KGlobal::config();

    sessionGroup = "Session: " + sessionName;

    config->setGroup( sessionGroup );
    int count =  config->readNumEntry( "count" );
    appsToStart = count;

    QValueList<QStringList> wmCommands;
    if ( !wm.isEmpty() ) {
	for ( int i = 1; i <= count; i++ ) {
	    QString n = QString::number(i);
	    if ( wm == config->readEntry( QString("program")+n ) ) {
		wmCommands << config->readListEntry( QString("restartCommand")+n );
	    }
	}
    }
    if ( wmCommands.isEmpty() )
        wmCommands << ( QStringList() << wm );

    publishProgress( appsToStart, true );
    connectDCOPSignal( launcher, launcher, "autoStart0Done()",
                       "autoStart0Done()", true);
    connectDCOPSignal( launcher, launcher, "autoStart1Done()",
                       "autoStart1Done()", true);
    connectDCOPSignal( launcher, launcher, "autoStart2Done()",
                       "autoStart2Done()", true);
    upAndRunning( "ksmserver" );

    if ( !wmCommands.isEmpty() ) {
        // when we have a window manager, we start it first and give
        // it some time before launching other processes. Results in a
        // visually more appealing startup.
        for (uint i = 0; i < wmCommands.count(); i++)
            startApplication( wmCommands[i] );
        QTimer::singleShot( 4000, this, SLOT( autoStart0() ) );
    } else {
        autoStart0();
    }
}

/*!
  Starts the default session.

  Currently, that's the window manager only (if specified).
 */
void KSMServer::startDefaultSession()
{
    if( state != Idle )
        return;

    state = LaunchingWM;
    sessionGroup = "";
    publishProgress( 0, true );
    upAndRunning( "ksmserver" );
    connectDCOPSignal( launcher, launcher, "autoStart0Done()",
                       "autoStart0Done()", true);
    connectDCOPSignal( launcher, launcher, "autoStart1Done()",
                       "autoStart1Done()", true);
    connectDCOPSignal( launcher, launcher, "autoStart2Done()",
                       "autoStart2Done()", true);
    startApplication( wm );
    QTimer::singleShot( 4000, this, SLOT( autoStart0() ) );
}


void KSMServer::clientSetProgram( KSMClient* client )
{
    if ( !wm.isEmpty() && client->program() == wm )
        autoStart0();
}

void KSMServer::autoStart0()
{
    if( state != LaunchingWM )
        return;
    if( !checkStartupSuspend())
        return;
    state = AutoStart0;
    DCOPRef( launcher ).send( "autoStart", (int) 0 );
}

void KSMServer::autoStart0Done()
{
    if( state != AutoStart0 )
        return;
    disconnectDCOPSignal( launcher, launcher, "autoStart0Done()",
                          "autoStart0Done()");
    if( !checkStartupSuspend())
        return;
    kdDebug( 1218 ) << "Autostart 0 done" << endl;
    upAndRunning( "kdesktop" );
    upAndRunning( "kicker" );
    connectDCOPSignal( "kcminit", "kcminit", "phase1Done()",
                       "kcmPhase1Done()", true);
    state = KcmInitPhase1;
    QTimer::singleShot( 10000, this, SLOT( kcmPhase1Timeout())); // protection
    DCOPRef( "kcminit", "kcminit" ).send( "runPhase1" );
}

void KSMServer::kcmPhase1Done()
{
    if( state != KcmInitPhase1 )
        return;
    kdDebug( 1218 ) << "Kcminit phase 1 done" << endl;
    disconnectDCOPSignal( "kcminit", "kcminit", "phase1Done()",
                       "kcmPhase1Done()" );
    autoStart1();
}

void KSMServer::kcmPhase1Timeout()
{
    if( state != KcmInitPhase1 )
        return;
    kdDebug( 1218 ) << "Kcminit phase 1 timeout" << endl;
    kcmPhase1Done();
}

void KSMServer::autoStart1()
{
    if( state != KcmInitPhase1 )
        return;
    state = AutoStart1;
    DCOPRef( launcher ).send( "autoStart", (int) 1 );
}

void KSMServer::autoStart1Done()
{
    if( state != AutoStart1 )
        return;
    disconnectDCOPSignal( launcher, launcher, "autoStart1Done()",
                          "autoStart1Done()");
    if( !checkStartupSuspend())
        return;
    kdDebug( 1218 ) << "Autostart 1 done" << endl;
    lastAppStarted = 0;
    lastIdStarted = QString::null;
    state = Restoring;
    if( defaultSession()) {
        autoStart2();
        return;
    }
    tryRestoreNext();
}

void KSMServer::clientRegistered( const char* previousId )
{
    if ( previousId && lastIdStarted == previousId )
        tryRestoreNext();
}

void KSMServer::tryRestoreNext()
{
    if( state != Restoring )
        return;
    restoreTimer.stop();
    KConfig* config = KGlobal::config();
    config->setGroup( sessionGroup );

    while ( lastAppStarted < appsToStart ) {
        publishProgress ( appsToStart - lastAppStarted );
        lastAppStarted++;
        QString n = QString::number(lastAppStarted);
        QStringList restartCommand = config->readListEntry( QString("restartCommand")+n );
        if ( restartCommand.isEmpty() ||
             (config->readNumEntry( QString("restartStyleHint")+n ) == SmRestartNever)) {
            continue;
        }
        if ( wm == config->readEntry( QString("program")+n ) )
            continue;
        startApplication( restartCommand,
                          config->readEntry( QString("clientMachine")+n ),
                          config->readEntry( QString("userId")+n ));
        lastIdStarted = config->readEntry( QString("clientId")+n );
        if ( !lastIdStarted.isEmpty() ) {
            restoreTimer.start( 2000, TRUE );
            return; // we get called again from the clientRegistered handler
        }
    }

    appsToStart = 0;
    lastIdStarted = QString::null;
    publishProgress( 0 );

    autoStart2();
}

void KSMServer::autoStart2()
{
    if( state != Restoring )
        return;
    if( !checkStartupSuspend())
        return;
    state = FinishingStartup;
    waitAutoStart2 = true;
    waitKcmInit2 = true;
    DCOPRef( launcher ).send( "autoStart", (int) 2 );
    DCOPRef( "kded", "kded" ).send( "loadSecondPhase" );
    DCOPRef( "kdesktop", "KDesktopIface" ).send( "runAutoStart" );
    connectDCOPSignal( "kcminit", "kcminit", "phase2Done()",
                       "kcmPhase2Done()", true);
    QTimer::singleShot( 10000, this, SLOT( kcmPhase2Timeout())); // protection
    DCOPRef( "kcminit", "kcminit" ).send( "runPhase2" );
    if( !defaultSession())
        restoreLegacySession( KGlobal::config());
    KNotifyClient::event( 0, "startkde" ); // this is the time KDE is up, more or less
}

void KSMServer::autoStart2Done()
{
    if( state != FinishingStartup )
        return;
    disconnectDCOPSignal( launcher, launcher, "autoStart2Done()",
                          "autoStart2Done()");
    kdDebug( 1218 ) << "Autostart 2 done" << endl;
    waitAutoStart2 = false;
    finishStartup();
}

void KSMServer::kcmPhase2Done()
{
    if( state != FinishingStartup )
        return;
    kdDebug( 1218 ) << "Kcminit phase 2 done" << endl;
    disconnectDCOPSignal( "kcminit", "kcminit", "phase2Done()",
                          "kcmPhase2Done()");
    waitKcmInit2 = false;
    finishStartup();
}

void KSMServer::kcmPhase2Timeout()
{
    if( !waitKcmInit2 )
        return;
    kdDebug( 1218 ) << "Kcminit phase 2 timeout" << endl;
    kcmPhase2Done();
}

void KSMServer::finishStartup()
{
    if( state != FinishingStartup )
        return;
    if( waitAutoStart2 || waitKcmInit2 )
        return;

    upAndRunning( "session ready" );
    DCOPRef( "knotify" ).send( "sessionReady" ); // knotify startup optimization

    state = Idle;

    setupXIOErrorHandler(); // From now on handle X errors as normal shutdown.
}

bool KSMServer::checkStartupSuspend()
{
    if( startupSuspendCount.isEmpty())
        return true;
    // wait for the phase to finish
    if( !startupSuspendTimeoutTimer.isActive())
        startupSuspendTimeoutTimer.start( 10000, true );
    return false;
}

void KSMServer::suspendStartup( QCString app )
{
    if( !startupSuspendCount.contains( app ))
        startupSuspendCount[ app ] = 0;
    ++startupSuspendCount[ app ];
}

void KSMServer::resumeStartup( QCString app )
{
    if( !startupSuspendCount.contains( app ))
        return;
    if( --startupSuspendCount[ app ] == 0 ) {
        startupSuspendCount.remove( app );
        if( startupSuspendCount.isEmpty() && startupSuspendTimeoutTimer.isActive()) {
            startupSuspendTimeoutTimer.stop();
            resumeStartupInternal();
        }
    }
}

void KSMServer::startupSuspendTimeout()
{
    kdDebug( 1218 ) << "Startup suspend timeout:" << state << endl;
    resumeStartupInternal();
}

void KSMServer::resumeStartupInternal()
{
    startupSuspendCount.clear();
    switch( state ) {
        case LaunchingWM:
            autoStart0();
          break;
        case AutoStart0:
            autoStart0Done();
          break;
        case AutoStart1:
            autoStart1Done();
          break;
        case Restoring:
            autoStart2();
          break;
        default:
            kdWarning( 1218 ) << "Unknown resume startup state" << endl;
          break;
    }
}

void KSMServer::publishProgress( int progress, bool max  )
{
    DCOPRef( "ksplash" ).send( max ? "setMaxProgress" : "setProgress", progress );
}


void KSMServer::upAndRunning( const QString& msg )
{
    DCOPRef( "ksplash" ).send( "upAndRunning", msg );
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom( qt_xdisplay(), "_KDE_SPLASH_PROGRESS", False );
    e.xclient.display = qt_xdisplay();
    e.xclient.window = qt_xrootwin();
    e.xclient.format = 8;
    assert( strlen( msg.latin1()) < 20 );
    strcpy( e.xclient.data.b, msg.latin1());
    XSendEvent( qt_xdisplay(), qt_xrootwin(), False, SubstructureNotifyMask, &e );
}

// these two are in the DCOP interface but I have no idea what uses them
// Remove for KDE4
void KSMServer::restoreSessionInternal()
{
    autoStart1Done();
}

void KSMServer::restoreSessionDoneInternal()
{
    autoStart2Done();
}
