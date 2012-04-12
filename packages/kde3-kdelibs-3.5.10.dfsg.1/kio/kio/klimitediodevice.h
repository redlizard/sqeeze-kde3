/* This file is part of the KDE libraries
   Copyright (C) 2001, 2002 David Faure <david@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef klimitediodevice_h
#define klimitediodevice_h

#include <kdebug.h>
#include <qiodevice.h>
/**
 * A readonly device that reads from an underlying device
 * from a given point to another (e.g. to give access to a single
 * file inside an archive).
 * @author David Faure <david@mandrakesoft.com>
 * @since 3.1
 */
class KIO_EXPORT KLimitedIODevice : public QIODevice
{
public:
    /**
     * Creates a new KLimitedIODevice.
     * @param dev the underlying device, opened or not
     * This device itself auto-opens (in readonly mode), no need to open it.
     * @param start where to start reading (position in bytes)
     * @param length the length of the data to read (in bytes)
     */
    KLimitedIODevice( QIODevice *dev, int start, int length )
        : m_dev( dev ), m_start( start ), m_length( length )
    {
        //kdDebug(7005) << "KLimitedIODevice::KLimitedIODevice start=" << start << " length=" << length << endl;
        setType( IO_Direct ); // we support sequential too, but then atEnd() tries getch/ungetch !
        open( IO_ReadOnly );
    }
    virtual ~KLimitedIODevice() {}

    virtual bool open( int m ) {
        //kdDebug(7005) << "KLimitedIODevice::open m=" << m << endl;
        if ( m & IO_ReadOnly ) {
            /*bool ok = false;
            if ( m_dev->isOpen() )
                ok = ( m_dev->mode() == IO_ReadOnly );
            else
                ok = m_dev->open( m );
            if ( ok )*/
                m_dev->at( m_start ); // No concurrent access !
        }
        else
            kdWarning(7005) << "KLimitedIODevice::open only supports IO_ReadOnly!" << endl;
        setState( IO_Open );
        setMode( m );
        return true;
    }
    virtual void close() {}
    virtual void flush() {}

    virtual Offset size() const { return m_length; }

    virtual Q_LONG readBlock ( char * data, Q_ULONG maxlen )
    {
        maxlen = QMIN( maxlen, m_length - at() ); // Apply upper limit
        return m_dev->readBlock( data, maxlen );
    }
    virtual Q_LONG writeBlock ( const char *, Q_ULONG ) { return -1; } // unsupported
    virtual int putch( int ) { return -1; } // unsupported

    virtual int getch() {
        char c[2];
        if ( readBlock(c, 1) == -1)
            return -1;
        else
            return c[0];
    }
    virtual int ungetch( int c ) { return m_dev->ungetch(c); } // ## apply lower limit ?
    virtual Offset at() const { return m_dev->at() - m_start; }
    virtual bool at( Offset pos ) {
        Q_ASSERT( pos <= m_length );
        pos = QMIN( pos, m_length ); // Apply upper limit
        return m_dev->at( m_start + pos );
    }
    virtual bool atEnd() const { return m_dev->atEnd() || m_dev->at() >= m_start + m_length; }
private:
    QIODevice* m_dev;
    Q_ULONG m_start;
    Q_ULONG m_length;
};

#endif
