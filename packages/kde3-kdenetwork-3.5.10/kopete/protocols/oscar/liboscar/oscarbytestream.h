
/***************************************************************************
                   gwbytestream.h  -  Byte Stream using KNetwork sockets
 adapted from jabberbytestream.h
                             -------------------
    begin                : Wed Jul 7 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

			   Kopete (C) 2004 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNETWORKBYTESTREAM_H
#define KNETWORKBYTESTREAM_H

#include <kbufferedsocket.h>

#include "bytestream.h"


/**
 * Low level socket class, using KDE's KNetwork socket classes
 * @author Till Gerken
 */
 
class KNetworkByteStream : public ByteStream
{

Q_OBJECT

public:
	KNetworkByteStream ( QObject *parent = 0, const char *name = 0 );

	~KNetworkByteStream ();

	bool connect ( QString host, QString service );
	virtual bool isOpen () const;
	virtual void close ();

	KNetwork::KBufferedSocket *socket () const;

signals:
	void connected ();

protected:
	virtual int tryWrite ();

private slots:
	void slotConnected ();
	void slotConnectionClosed ();
	void slotReadyRead ();
	void slotBytesWritten ( int );
	void slotError ( int );

private:
	KNetwork::KBufferedSocket *mSocket;
	bool mClosing;

};

#endif

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;

