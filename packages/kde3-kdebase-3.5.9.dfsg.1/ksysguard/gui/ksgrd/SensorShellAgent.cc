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

*/

#include <stdlib.h>

#include <kdebug.h>
#include <kpassdlg.h> 
#include <kprocess.h>

#include "SensorClient.h"
#include "SensorManager.h"

#include "SensorShellAgent.h"

using namespace KSGRD;

SensorShellAgent::SensorShellAgent( SensorManager *sm )
  : SensorAgent( sm ), mDaemon( 0 )
{
}

SensorShellAgent::~SensorShellAgent()
{
  if ( mDaemon ) {
    mDaemon->writeStdin( "quit\n", strlen( "quit\n" ) );
    delete mDaemon;
    mDaemon = 0;
  }
}
	
bool SensorShellAgent::start( const QString &host, const QString &shell,
                              const QString &command, int )
{
  mRetryCount = 3;
  mDaemon = new KProcess;
  mDaemon->setUseShell(true);
  setHostName( host );
  mShell = shell;
  mCommand = command;

  connect( mDaemon, SIGNAL( processExited( KProcess* ) ),
           SLOT( daemonExited( KProcess* ) ) );
  connect( mDaemon, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
           SLOT( msgRcvd( KProcess*, char*, int ) ) );
  connect( mDaemon, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
           SLOT( errMsgRcvd( KProcess*, char*, int ) ) );
  connect( mDaemon, SIGNAL( wroteStdin( KProcess* ) ),
           SLOT( msgSent( KProcess* ) ) );

  QString cmd;
  if ( !command.isEmpty() )
    cmd =  command;
  else
    cmd = mShell + " " + hostName() + " ksysguardd";
  *mDaemon << cmd;

  if ( !mDaemon->start( KProcess::NotifyOnExit, KProcess::All ) ) {
    sensorManager()->hostLost( this );
    kdDebug (1215) << "Command '" << cmd << "' failed"  << endl;
    return false;
  }

  return true;
}

void SensorShellAgent::hostInfo( QString &shell, QString &command,
                                 int &port) const
{
  shell = mShell;
  command = mCommand;
  port = -1;
}

void SensorShellAgent::msgSent( KProcess* )
{
  setTransmitting( false );

	// Try to send next request if available.
  executeCommand();
}

void SensorShellAgent::msgRcvd( KProcess*, char *buffer, int buflen )
{
  if ( !buffer || buflen == 0 )
    return;
  mRetryCount = 3; //we recieved an answer, so reset our retry count back to 3
  QString aux = QString::fromLocal8Bit( buffer, buflen );

  processAnswer( aux );
}

void SensorShellAgent::errMsgRcvd( KProcess*, char *buffer, int buflen )
{
  if ( !buffer || buflen == 0 )
    return;

  QString buf = QString::fromLocal8Bit( buffer, buflen );

  kdDebug(1215) << "SensorShellAgent: Warning, received text over stderr!"
                << endl << buf << endl;
}

void SensorShellAgent::daemonExited( KProcess *process )
{
  kdDebug() << "daemonExited" << endl;
  if ( mRetryCount-- <= 0 || !mDaemon->start( KProcess::NotifyOnExit, KProcess::All ) ) {
    kdDebug() << "daemon could not be restart" << endl;
    setDaemonOnLine( false );
    sensorManager()->hostLost( this );
    sensorManager()->requestDisengage( this );
  }
}

bool SensorShellAgent::writeMsg( const char *msg, int len )
{
  return mDaemon->writeStdin( msg, len );
}

bool SensorShellAgent::txReady()
{
  return !transmitting();
}

#include "SensorShellAgent.moc"
