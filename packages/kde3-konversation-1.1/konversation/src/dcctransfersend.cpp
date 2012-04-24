/*
  send a file on DCC protocol
  begin:     Mit Aug 7 2002
  copyright: (C) 2002 by Dario Abatianni
  email:     eisfuchs@tigress.com
*/
// Copyright (C) 2004-2007 Shintaro Matsuoka <shin@shoegazed.org>
// Copyright (C) 2004,2005 John Tapsell <john@geola.co.uk>

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "dcctransfersend.h"
#include "channel.h"
#include "dcccommon.h"
#include "dcctransfermanager.h"
#include "konversationapplication.h"
#include "connectionmanager.h"
#include "server.h"

#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <qfile.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kserversocket.h>
#include <ksocketaddress.h>
#include <kstreamsocket.h>
#include <kio/netaccess.h>
#include <kfileitem.h>

// TODO: remove the dependence
#include <kinputdialog.h>


using namespace KNetwork;

DccTransferSend::DccTransferSend(QObject* parent)
    : DccTransfer( DccTransfer::Send, parent )
{
    kdDebug() << "DccTransferSend::DccTransferSend()" << endl;

    m_serverSocket = 0;
    m_sendSocket = 0;

    m_connectionTimer = new QTimer( this );
    connect( m_connectionTimer, SIGNAL( timeout() ), this, SLOT( slotConnectionTimeout() ) );

    // set defualt values
    m_reverse = Preferences::dccPassiveSend();
}

DccTransferSend::~DccTransferSend()
{
    cleanUp();
}

void DccTransferSend::cleanUp()
{
    kdDebug() << "DccTransferSend::cleanUp()" << endl;
    stopConnectionTimer();
    finishTransferLogger();
    if ( !m_tmpFile.isEmpty() )
        KIO::NetAccess::removeTempFile( m_tmpFile );
    m_tmpFile = QString();
    m_file.close();
    if ( m_sendSocket )
    {
        m_sendSocket->close();
        m_sendSocket = 0;                         // the instance will be deleted automatically by its parent
    }
    if ( m_serverSocket )
    {
        m_serverSocket->close();
        m_serverSocket = 0;                       // the instance will be deleted automatically by its parent
    }
}

// just for convenience
void DccTransferSend::failed( const QString& errorMessage )
{
    setStatus( Failed, errorMessage );
    cleanUp();
    emit done( this );
}

void DccTransferSend::setFileURL( const KURL& url )
{
    if ( getStatus() == Configuring )
        m_fileURL = url;
}

void DccTransferSend::setFileName( const QString& fileName )
{
    if ( getStatus() == Configuring )
        m_fileName = fileName;
}

void DccTransferSend::setOwnIp( const QString& ownIp )
{
    if ( getStatus() == Configuring )
        m_ownIp = ownIp;
}

void DccTransferSend::setFileSize( KIO::filesize_t fileSize )
{
    if ( getStatus() == Configuring )
        m_fileSize = fileSize;
}

void DccTransferSend::setReverse( bool reverse )
{
    if ( getStatus() == Configuring )
        m_reverse = reverse;
}

bool DccTransferSend::queue()
{
    kdDebug() << "DccTransferSend::queue()" << endl;

    if ( getStatus() != Configuring )
        return false;

    if ( m_ownIp.isEmpty() )
        m_ownIp = DccCommon::getOwnIp( KonversationApplication::instance()->getConnectionManager()->getServerByConnectionId( m_connectionId ) );

    if ( !kapp->authorize( "allow_downloading" ) )
    {
        //Do not have the rights to send the file.  Shouldn't have gotten this far anyway
        //Note this is after the initialisation so the view looks correct still
        failed(i18n("The admin has restricted the right to send files"));
        return false;
    }

    if ( m_fileName.isEmpty() )
        m_fileName = sanitizeFileName( m_fileURL.fileName() );

    if ( Preferences::dccIPv4Fallback() )
    {
        KIpAddress ip( m_ownIp );
        if ( ip.isIPv6Addr() )
        {
            /* This is fucking ugly but there is no KDE way to do this yet :| -cartman */
            struct ifreq ifr;
            const char* address = Preferences::dccIPv4FallbackIface().ascii();
            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            strncpy( ifr.ifr_name, address, IF_NAMESIZE );
            ifr.ifr_addr.sa_family = AF_INET;
            if ( ioctl( sock, SIOCGIFADDR, &ifr ) >= 0 )
                m_ownIp =  inet_ntoa( ( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr );
            kdDebug() << "Falling back to IPv4 address " << m_ownIp << endl;
        }
    }

    m_fastSend = Preferences::dccFastSend();
    kdDebug() << "DccTransferSend::DccTransferSend(): Fast DCC send: " << m_fastSend << endl;

    //Check the file exists
    if ( !KIO::NetAccess::exists( m_fileURL, true, NULL ) )
    {
        failed( i18n( "The url \"%1\" does not exist" ).arg( m_fileURL.prettyURL() ) );
        return false;
    }

    //FIXME: KIO::NetAccess::download() is a synchronous function. we should use KIO::get() instead.
    //Download the file.  Does nothing if it's local (file:/)
    if ( !KIO::NetAccess::download( m_fileURL, m_tmpFile, NULL ) )
    {
        failed( i18n( "Could not retrieve \"%1\"" ).arg( m_fileURL.prettyURL() ) );
        kdDebug() << "DccTransferSend::DccTransferSend(): KIO::NetAccess::download() failed. reason: " << KIO::NetAccess::lastErrorString() << endl;
        return false;
    }

    //Some protocols, like http, maybe not return a filename, and altFileName may be empty, So prompt the user for one.
    if ( m_fileName.isEmpty() )
    {
        bool pressedOk;
        m_fileName = KInputDialog::getText( i18n( "Enter Filename" ), i18n( "<qt>The file that you are sending to <i>%1</i> does not have a filename.<br>Please enter a filename to be presented to the receiver, or cancel the dcc transfer</qt>" ).arg( getPartnerNick() ), "unknown", &pressedOk, NULL );

        if ( !pressedOk )
        {
            failed( i18n( "No filename was given" ) );
            return false;
        }
    }

    //FIXME: if "\\\"" works well on other IRC clients, replace "\"" with "\\\""
    m_fileName.replace( "\"", "_" );
    if (Preferences::dccSpaceToUnderscore())
        m_fileName.replace( " ", "_" );
    else {
        if (m_fileName.contains(" ") > 0)
            m_fileName = "\"" + m_fileName + "\"";
    }

    m_file.setName( m_tmpFile );

    if ( m_fileSize == 0 )
        m_fileSize = m_file.size();

    return DccTransfer::queue();
}

void DccTransferSend::abort()                     // public slot
{
    kdDebug() << "DccTransferSend::abort()" << endl;

    setStatus( Aborted );
    cleanUp();
    emit done( this );
}

void DccTransferSend::start()                     // public slot
{
    kdDebug() << "DccTransferSend::start()" << endl;

    if ( getStatus() != Queued )
        return;

    // common procedure

    Server* server = KonversationApplication::instance()->getConnectionManager()->getServerByConnectionId( m_connectionId );
    if ( !server )
    {
        kdDebug() << "DccTransferSend::start(): could not retrieve the instance of Server. Connection id: " << m_connectionId << endl;
        failed( i18n( "Could not send a DCC SEND request to the partner via the IRC server." ) );
        return;
    }

    if ( !m_reverse )
    {
        // Normal DCC SEND
        kdDebug() << "DccTransferSend::start(): normal DCC SEND" << endl;

        // Set up server socket
        QString failedReason;
        if ( Preferences::dccSpecificSendPorts() )
            m_serverSocket = DccCommon::createServerSocketAndListen( this, &failedReason, Preferences::dccSendPortsFirst(), Preferences::dccSendPortsLast() );
        else
            m_serverSocket = DccCommon::createServerSocketAndListen( this, &failedReason );
        if ( !m_serverSocket )
        {
            failed( failedReason );
            return;
        }

        connect( m_serverSocket, SIGNAL( readyAccept() ),   this, SLOT( acceptClient() ) );
        connect( m_serverSocket, SIGNAL( gotError( int ) ), this, SLOT( slotGotSocketError( int ) ) );
        connect( m_serverSocket, SIGNAL( closed() ),        this, SLOT( slotServerSocketClosed() ) );

        // Get own port number
        m_ownPort = QString::number( DccCommon::getServerSocketPort( m_serverSocket ) );

        kdDebug() << "DccTransferSend::start(): own Address=" << m_ownIp << ":" << m_ownPort << endl;

        startConnectionTimer( Preferences::dccSendTimeout() );

        server->dccSendRequest( m_partnerNick, m_fileName, getNumericalIpText( m_ownIp ), m_ownPort, m_fileSize );
    }
    else
    {
        // Passive DCC SEND
        kdDebug() << "DccTransferSend::start(): passive DCC SEND" << endl;

        int tokenNumber = KonversationApplication::instance()->getDccTransferManager()->generateReverseTokenNumber();
        // TODO: should we append a letter "T" to this token?
        m_reverseToken = QString::number( tokenNumber );

        kdDebug() << "DccTransferSend::start(): passive DCC key(token): " << m_reverseToken << endl;

        server->dccPassiveSendRequest( m_partnerNick, m_fileName, getNumericalIpText( m_ownIp ), m_fileSize, m_reverseToken );
    }

    setStatus( WaitingRemote, i18n( "Waiting remote user's acceptance" ) );
}

void DccTransferSend::connectToReceiver( const QString& partnerHost, const QString& partnerPort )
{
    // Reverse DCC

    m_partnerIp = partnerHost;
    m_partnerPort = partnerPort;

    m_sendSocket = new KNetwork::KStreamSocket( partnerHost, partnerPort, this );

    m_sendSocket->setBlocking( false );

    connect( m_sendSocket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( startSending() ) );
    connect( m_sendSocket, SIGNAL( gotError( int ) ), this, SLOT( slotConnectionFailed( int ) ) );

    setStatus( Connecting );

    m_sendSocket->connect();
}
                                                  // public
bool DccTransferSend::setResume( unsigned long position )
{
    kdDebug() << "DccTransferSend::setResume(): position=" << position << endl;

    if ( getStatus() > WaitingRemote )
        return false;

    if ( position >= m_fileSize )
        return false;

    m_resumed = true;
    m_transferringPosition = position;
    return true;
}

void DccTransferSend::acceptClient()                     // slot
{
    // Normal DCC

    kdDebug() << "DccTransferSend::acceptClient()" << endl;

    stopConnectionTimer();

    m_sendSocket = static_cast<KNetwork::KStreamSocket*>( m_serverSocket->accept() );
    if ( !m_sendSocket )
    {
        failed( i18n( "Could not accept the connection. (Socket Error)" ) );
        return;
    }

    // we don't need ServerSocket anymore
    m_serverSocket->close();

    startSending();
}

void DccTransferSend::startSending()
{
    connect( m_sendSocket, SIGNAL( readyWrite() ), this, SLOT( writeData() )            );
    connect( m_sendSocket, SIGNAL( readyRead() ),  this, SLOT( getAck() )               );
    connect( m_sendSocket, SIGNAL( closed() ),     this, SLOT( slotSendSocketClosed() ) );

    if ( m_sendSocket->peerAddress().asInet().ipAddress().isV4Mapped() )
        m_partnerIp = KNetwork::KIpAddress( m_sendSocket->peerAddress().asInet().ipAddress().IPv4Addr() ).toString();
    else
        m_partnerIp = m_sendSocket->peerAddress().asInet().ipAddress().toString();
    m_partnerPort = m_sendSocket->peerAddress().serviceName();

    if ( m_file.open( IO_ReadOnly ) )
    {
        // seek to file position to make resume work
        m_file.at( m_transferringPosition );
        m_transferStartPosition = m_transferringPosition;

        setStatus( Transferring );
        m_sendSocket->enableWrite( true );
        m_sendSocket->enableRead( m_fastSend );
        startTransferLogger();                      // initialize CPS counter, ETA counter, etc...
    }
    else
        failed( i18n( "Could not open the file: %1" ).arg( getQFileErrorString( m_file.status() ) ) );
}

void DccTransferSend::writeData()                 // slot
{
    //kdDebug() << "DccTransferSend::writeData()" << endl;
    if ( !m_fastSend )
    {
        m_sendSocket->enableWrite( false );
        m_sendSocket->enableRead( true );
    }
    int actual = m_file.readBlock( m_buffer, m_bufferSize );
    if ( actual > 0 )
    {
        m_sendSocket->writeBlock( m_buffer, actual );
        m_transferringPosition += actual;
        if ( (KIO::fileoffset_t)m_fileSize <= m_transferringPosition )
        {
            Q_ASSERT( (KIO::fileoffset_t)m_fileSize == m_transferringPosition );
            kdDebug() << "DccTransferSend::writeData(): Done." << endl;
            m_sendSocket->enableWrite( false );   // there is no need to call this function anymore
        }
    }
}

void DccTransferSend::getAck()                    // slot
{
    //kdDebug() << "DccTransferSend::getAck()" << endl;
    if ( !m_fastSend && m_transferringPosition < (KIO::fileoffset_t)m_fileSize )
    {
        m_sendSocket->enableWrite( true );
        m_sendSocket->enableRead( false );
    }
    unsigned long pos;
    while ( m_sendSocket->bytesAvailable() >= 4 )
    {
        m_sendSocket->readBlock( (char*)&pos, 4 );
        pos = intel( pos );
        if ( pos == m_fileSize )
        {
            kdDebug() << "DccTransferSend::getAck(): Received final ACK." << endl;
            finishTransferLogger();
            setStatus( Done );
            cleanUp();
            emit done( this );
            break;                                // for safe
        }
    }
}

void DccTransferSend::slotGotSocketError( int errorCode )
{
    kdDebug() << "DccTransferSend::slotGotSocketError(): code =  " << errorCode << " string = " << m_serverSocket->errorString() << endl;
    failed( i18n( "Socket error: %1" ).arg( m_serverSocket->errorString() ) );
}

void DccTransferSend::startConnectionTimer( int sec )
{
    kdDebug() << "DccTransferSend::startConnectionTimer()"<< endl;
    stopConnectionTimer();
    m_connectionTimer->start( sec*1000, true );
}

void DccTransferSend::stopConnectionTimer()
{
    if ( m_connectionTimer->isActive() )
    {
        kdDebug() << "DccTransferSend::stopConnectionTimer(): stop" << endl;
        m_connectionTimer->stop();
    }
}

void DccTransferSend::slotConnectionTimeout()         // slot
{
    kdDebug() << "DccTransferSend::slotConnectionTimeout()" << endl;
    failed( i18n( "Timed out" ) );
}

void DccTransferSend::slotConnectionFailed( int /* errorCode */ )
{
    failed( i18n( "Connection failure: %1" ).arg( m_sendSocket->errorString() ) );
}

void DccTransferSend::slotServerSocketClosed()
{
    kdDebug() << "DccTransferSend::slotServerSocketClosed()" << endl;
}

void DccTransferSend::slotSendSocketClosed()
{
    kdDebug() << "DccTransferSend::slotSendSocketClosed()" << endl;
    finishTransferLogger();
    if ( getStatus() == Transferring && m_transferringPosition < (KIO::fileoffset_t)m_fileSize )
        failed( i18n( "Remote user disconnected" ) );
}

                                                  // protected, static
QString DccTransferSend::getQFileErrorString( int code )
{
    QString errorString;

    switch(code)
    {
        case IO_Ok:
            errorString=i18n("The operation was successful. Should never happen in an error dialog.");
            break;
        case IO_ReadError:
            errorString=i18n("Could not read from file \"%1\".").arg( m_fileName );
            break;
        case IO_WriteError:
            errorString=i18n("Could not write to file \"%1\".").arg( m_fileName );
            break;
        case IO_FatalError:
            errorString=i18n("A fatal unrecoverable error occurred.");
            break;
        case IO_OpenError:
            errorString=i18n("Could not open file \"%1\".").arg( m_fileName );
            break;

            // Same case value? Damn!
            //        case IO_ConnectError:
            //          errorString="Could not connect to the device.";
            //        break;

        case IO_AbortError:
            errorString=i18n("The operation was unexpectedly aborted.");
            break;
        case IO_TimeOutError:
            errorString=i18n("The operation timed out.");
            break;
        case IO_UnspecifiedError:
            errorString=i18n("An unspecified error happened on close.");
            break;
        default:
            errorString=i18n("Unknown error. Code %1").arg(code);
            break;
    }

    return errorString;
}

#include "dcctransfersend.moc"
