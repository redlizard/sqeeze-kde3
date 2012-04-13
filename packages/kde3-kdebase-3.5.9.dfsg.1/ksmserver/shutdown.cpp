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
#include <qregexp.h>

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
#include <dmctl.h>
#include <kdebug.h>
#include <knotifyclient.h>

#include "server.h"
#include "global.h"
#include "shutdowndlg.h"
#include "client.h"

void KSMServer::logout( int confirm, int sdtype, int sdmode )
{
    shutdown( (KApplication::ShutdownConfirm)confirm,
              (KApplication::ShutdownType)sdtype,
              (KApplication::ShutdownMode)sdmode );
}

void KSMServer::shutdown( KApplication::ShutdownConfirm confirm,
    KApplication::ShutdownType sdtype, KApplication::ShutdownMode sdmode )
{
    pendingShutdown.stop();
    if( dialogActive )
        return;
    if( state >= Shutdown ) // already performing shutdown
        return;
    if( state != Idle ) // performing startup
    {
    // perform shutdown as soon as startup is finished, in order to avoid saving partial session
        if( !pendingShutdown.isActive())
        {
            pendingShutdown.start( 1000 );
            pendingShutdown_confirm = confirm;
            pendingShutdown_sdtype = sdtype;
            pendingShutdown_sdmode = sdmode;
        }
        return;
    }

    KConfig *config = KGlobal::config();
    config->reparseConfiguration(); // config may have changed in the KControl module

    config->setGroup("General" );
    bool logoutConfirmed =
        (confirm == KApplication::ShutdownConfirmYes) ? false :
       (confirm == KApplication::ShutdownConfirmNo) ? true :
                  !config->readBoolEntry( "confirmLogout", true );
    bool maysd = false;
    if (config->readBoolEntry( "offerShutdown", true ) && DM().canShutdown())
        maysd = true;
    if (!maysd) {
        if (sdtype != KApplication::ShutdownTypeNone &&
            sdtype != KApplication::ShutdownTypeDefault &&
            logoutConfirmed)
            return; /* unsupported fast shutdown */
        sdtype = KApplication::ShutdownTypeNone;
    } else if (sdtype == KApplication::ShutdownTypeDefault)
        sdtype = (KApplication::ShutdownType)
                 config->readNumEntry( "shutdownType", (int)KApplication::ShutdownTypeNone );
    if (sdmode == KApplication::ShutdownModeDefault)
        sdmode = KApplication::ShutdownModeInteractive;

    dialogActive = true;
    QString bopt;
    if ( !logoutConfirmed ) {
        KSMShutdownFeedback::start(); // make the screen gray
        logoutConfirmed =
            KSMShutdownDlg::confirmShutdown( maysd, sdtype, bopt );
        // ###### We can't make the screen remain gray while talking to the apps,
        // because this prevents interaction ("do you want to save", etc.)
        // TODO: turn the feedback widget into a list of apps to be closed,
        // with an indicator of the current status for each.
        KSMShutdownFeedback::stop(); // make the screen become normal again
    }

    if ( logoutConfirmed ) {

	shutdownType = sdtype;
	shutdownMode = sdmode;
	bootOption = bopt;

        // shall we save the session on logout?
        saveSession = ( config->readEntry( "loginMode", "restorePreviousLogout" ) == "restorePreviousLogout" );

        if ( saveSession )
            sessionGroup = QString("Session: ") + SESSION_PREVIOUS_LOGOUT;

        // Set the real desktop background to black so that exit looks
        // clean regardless of what was on "our" desktop.
        kapp->desktop()->setBackgroundColor( Qt::black );
        state = Shutdown;
        wmPhase1WaitingCount = 0;
        saveType = saveSession?SmSaveBoth:SmSaveGlobal;
	performLegacySessionSave();
        startProtection();
        for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
            c->resetState();
            // Whoever came with the idea of phase 2 got it backwards
            // unfortunately. Window manager should be the very first
            // one saving session data, not the last one, as possible
            // user interaction during session save may alter
            // window positions etc.
            // Moreover, KWin's focus stealing prevention would lead
            // to undesired effects while session saving (dialogs
            // wouldn't be activated), so it needs be assured that
            // KWin will turn it off temporarily before any other
            // user interaction takes place.
            // Therefore, make sure the WM finishes its phase 1
            // before others a chance to change anything.
            // KWin will check if the session manager is ksmserver,
            // and if yes it will save in phase 1 instead of phase 2.
            if( isWM( c )) {
                ++wmPhase1WaitingCount;
                SmsSaveYourself( c->connection(), saveType,
                             true, SmInteractStyleAny, false );
            }

        }
        if( wmPhase1WaitingCount == 0 ) { // no WM, simply start them all
            for ( KSMClient* c = clients.first(); c; c = clients.next() )
                SmsSaveYourself( c->connection(), saveType,
                             true, SmInteractStyleAny, false );
        }
        if ( clients.isEmpty() )
            completeShutdownOrCheckpoint();
    }
    dialogActive = false;
}

void KSMServer::pendingShutdownTimeout()
{
    shutdown( pendingShutdown_confirm, pendingShutdown_sdtype, pendingShutdown_sdmode );
}

void KSMServer::saveCurrentSession()
{
    if ( state != Idle || dialogActive )
        return;

    if ( currentSession().isEmpty() || currentSession() == SESSION_PREVIOUS_LOGOUT )
        sessionGroup = QString("Session: ") + SESSION_BY_USER;

    state = Checkpoint;
    wmPhase1WaitingCount = 0;
    saveType = SmSaveLocal;
    saveSession = true;
    performLegacySessionSave();
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        c->resetState();
        if( isWM( c )) {
            ++wmPhase1WaitingCount;
            SmsSaveYourself( c->connection(), saveType, false, SmInteractStyleNone, false );
        }
    }
    if( wmPhase1WaitingCount == 0 ) {
        for ( KSMClient* c = clients.first(); c; c = clients.next() )
            SmsSaveYourself( c->connection(), saveType, false, SmInteractStyleNone, false );
    }
    if ( clients.isEmpty() )
        completeShutdownOrCheckpoint();
}

void KSMServer::saveCurrentSessionAs( QString session )
{
    if ( state != Idle || dialogActive )
        return;
    sessionGroup = "Session: " + session;
    saveCurrentSession();
}

// callbacks
void KSMServer::saveYourselfDone( KSMClient* client, bool success )
{
    if ( state == Idle ) {
        // State saving when it's not shutdown or checkpoint. Probably
        // a shutdown was cancelled and the client is finished saving
        // only now. Discard the saved state in order to avoid
        // the saved data building up.
        QStringList discard = client->discardCommand();
        if( !discard.isEmpty())
            executeCommand( discard );
        return;
    }
    if ( success ) {
        client->saveYourselfDone = true;
        completeShutdownOrCheckpoint();
    } else {
        // fake success to make KDE's logout not block with broken
        // apps. A perfect ksmserver would display a warning box at
        // the very end.
        client->saveYourselfDone = true;
        completeShutdownOrCheckpoint();
    }
    startProtection();
    if( isWM( client ) && !client->wasPhase2 && wmPhase1WaitingCount > 0 ) {
        --wmPhase1WaitingCount;
        // WM finished its phase1, save the rest
        if( wmPhase1WaitingCount == 0 ) {
            for ( KSMClient* c = clients.first(); c; c = clients.next() )
                if( !isWM( c ))
                    SmsSaveYourself( c->connection(), saveType, saveType != SmSaveLocal,
                        saveType != SmSaveLocal ? SmInteractStyleAny : SmInteractStyleNone,
                        false );
        }
    }
}

void KSMServer::interactRequest( KSMClient* client, int /*dialogType*/ )
{
    if ( state == Shutdown )
        client->pendingInteraction = true;
    else
        SmsInteract( client->connection() );

    handlePendingInteractions();
}

void KSMServer::interactDone( KSMClient* client, bool cancelShutdown_ )
{
    if ( client != clientInteracting )
        return; // should not happen
    clientInteracting = 0;
    if ( cancelShutdown_ )
        cancelShutdown( client );
    else
        handlePendingInteractions();
}


void KSMServer::phase2Request( KSMClient* client )
{
    client->waitForPhase2 = true;
    client->wasPhase2 = true;
    completeShutdownOrCheckpoint();
    if( isWM( client ) && wmPhase1WaitingCount > 0 ) {
        --wmPhase1WaitingCount;
        // WM finished its phase1 and requests phase2, save the rest
        if( wmPhase1WaitingCount == 0 ) {
            for ( KSMClient* c = clients.first(); c; c = clients.next() )
                if( !isWM( c ))
                    SmsSaveYourself( c->connection(), saveType, saveType != SmSaveLocal,
                        saveType != SmSaveLocal ? SmInteractStyleAny : SmInteractStyleNone,
                        false );
        }
    }
}

void KSMServer::handlePendingInteractions()
{
    if ( clientInteracting )
        return;

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        if ( c->pendingInteraction ) {
            clientInteracting = c;
            c->pendingInteraction = false;
            break;
        }
    }
    if ( clientInteracting ) {
        endProtection();
        SmsInteract( clientInteracting->connection() );
    } else {
        startProtection();
    }
}


void KSMServer::cancelShutdown( KSMClient* c )
{
    kdDebug( 1218 ) << "Client " << c->program() << " (" << c->clientId() << ") canceled shutdown." << endl;
    KNotifyClient::event( 0, "cancellogout", i18n( "Logout canceled by '%1'" ).arg( c->program()));
    clientInteracting = 0;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        SmsShutdownCancelled( c->connection() );
        if( c->saveYourselfDone ) {
            // Discard also saved state.
            QStringList discard = c->discardCommand();
            if( !discard.isEmpty())
                executeCommand( discard );
        }
    }
    state = Idle;
}

void KSMServer::startProtection()
{
    protectionTimer.start( 10000, true );
}

void KSMServer::endProtection()
{
    protectionTimer.stop();
}

/*
   Internal protection slot, invoked when clients do not react during
  shutdown.
 */
void KSMServer::protectionTimeout()
{
    if ( ( state != Shutdown && state != Checkpoint ) || clientInteracting )
        return;

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        if ( !c->saveYourselfDone && !c->waitForPhase2 ) {
            kdDebug( 1218 ) << "protectionTimeout: client " << c->program() << "(" << c->clientId() << ")" << endl;
            c->saveYourselfDone = true;
        }
    }
    completeShutdownOrCheckpoint();
    startProtection();
}

void KSMServer::completeShutdownOrCheckpoint()
{
    if ( state != Shutdown && state != Checkpoint )
        return;

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        if ( !c->saveYourselfDone && !c->waitForPhase2 )
            return; // not done yet
    }

    // do phase 2
    bool waitForPhase2 = false;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        if ( !c->saveYourselfDone && c->waitForPhase2 ) {
            c->waitForPhase2 = false;
            SmsSaveYourselfPhase2( c->connection() );
            waitForPhase2 = true;
        }
    }
    if ( waitForPhase2 )
        return;

    if ( saveSession )
        storeSession();
    else
        discardSession();

    if ( state == Shutdown ) {
        bool waitForKNotify = true;
        if( !kapp->dcopClient()->connectDCOPSignal( "knotify", "",
            "notifySignal(QString,QString,QString,QString,QString,int,int,int,int)",
            "ksmserver", "notifySlot(QString,QString,QString,QString,QString,int,int,int,int)", false )) {
            waitForKNotify = false;
        }
        if( !kapp->dcopClient()->connectDCOPSignal( "knotify", "",
            "playingFinished(int,int)",
            "ksmserver", "logoutSoundFinished(int,int)", false )) {
            waitForKNotify = false;
        }
        // event() can return -1 if KNotifyClient short-circuits and avoids KNotify
        logoutSoundEvent = KNotifyClient::event( 0, "exitkde" ); // KDE says good bye
        if( logoutSoundEvent <= 0 )
            waitForKNotify = false;
        if( waitForKNotify ) {
            state = WaitingForKNotify;
            knotifyTimeoutTimer.start( 20000, true );
            return;
        }
        startKilling();
    } else if ( state == Checkpoint ) {
        for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
            SmsSaveComplete( c->connection());
        }
        state = Idle;
    }
}

void KSMServer::startKilling()
{
    knotifyTimeoutTimer.stop();
    // kill all clients
    state = Killing;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        if( isWM( c )) // kill the WM as the last one in order to reduce flicker
            continue;
        kdDebug( 1218 ) << "completeShutdown: client " << c->program() << "(" << c->clientId() << ")" << endl;
        SmsDie( c->connection() );
    }

    kdDebug( 1218 ) << " We killed all clients. We have now clients.count()=" <<
       clients.count() << endl;
    completeKilling();
    QTimer::singleShot( 10000, this, SLOT( timeoutQuit() ) );
}

void KSMServer::completeKilling()
{
    kdDebug( 1218 ) << "KSMServer::completeKilling clients.count()=" <<
        clients.count() << endl;
    if( state == Killing ) {
        bool wait = false;
        for( KSMClient* c = clients.first(); c; c = clients.next()) {
            if( isWM( c ))
                continue;
            wait = true; // still waiting for clients to go away
        }
        if( wait )
            return;
        killWM();
    }
}

void KSMServer::killWM()
{
    state = KillingWM;
    bool iswm = false;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        if( isWM( c )) {
            iswm = true;
            kdDebug( 1218 ) << "killWM: client " << c->program() << "(" << c->clientId() << ")" << endl;
            SmsDie( c->connection() );
        }
    }
    if( iswm ) {
        completeKillingWM();
        QTimer::singleShot( 5000, this, SLOT( timeoutWMQuit() ) );
    }
    else
        killingCompleted();
}

void KSMServer::completeKillingWM()
{
    kdDebug( 1218 ) << "KSMServer::completeKillingWM clients.count()=" <<
        clients.count() << endl;
    if( state == KillingWM ) {
        if( clients.isEmpty())
            killingCompleted();
    }
}

// shutdown is fully complete
void KSMServer::killingCompleted()
{
    kapp->quit();
}

// called when KNotify performs notification for logout (not when sound is finished though)
void KSMServer::notifySlot(QString event ,QString app,QString,QString,QString,int present,int,int,int)
{
    if( state != WaitingForKNotify )
        return;
    if( event != "exitkde" || app != "ksmserver" )
        return;
    if( present & KNotifyClient::Sound ) // logoutSoundFinished() will be called
        return;
    startKilling();
}

// This is stupid. The normal DCOP signal connected to notifySlot() above should be simply
// emitted in KNotify only after the sound is finished playing.
void KSMServer::logoutSoundFinished( int event, int )
{
    if( state != WaitingForKNotify )
        return;
    if( event != logoutSoundEvent )
        return;
    startKilling();
}

void KSMServer::knotifyTimeout()
{
    if( state != WaitingForKNotify )
        return;
    startKilling();
}

void KSMServer::timeoutQuit()
{
    for (KSMClient *c = clients.first(); c; c = clients.next()) {
        kdWarning( 1218 ) << "SmsDie timeout, client " << c->program() << "(" << c->clientId() << ")" << endl;
    }
    killWM();
}

void KSMServer::timeoutWMQuit()
{
    if( state == KillingWM ) {
        kdWarning( 1218 ) << "SmsDie WM timeout" << endl;
    }
    killingCompleted();
}
