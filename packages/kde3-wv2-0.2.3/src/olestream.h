/* This file is part of the wvWare 2 project
   Copyright (C) 2001-2003 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef OLESTREAM_H
#define OLESTREAM_H

#include "olestorage.h"
#include "global.h"  // U8,... typedefs
#include <stack>

#include <glib/giochannel.h> // GSeekType

namespace wvWare {

class OLEStream
{
public:
    /**
     * Create an OLE stream
     */
    OLEStream( OLEStorage* storage );
    virtual ~OLEStream();

    /**
     * Is this still a valid stream?
     */
    virtual bool isValid() const = 0;

    /**
     * works like plain fseek
     */
    virtual bool seek( int offset, GSeekType whence = G_SEEK_SET ) = 0;
    /**
     * works like plain ftell
     */
    virtual int tell() const = 0;
    /**
     * The size of the stream
     */
    virtual size_t size() const = 0;

    /**
     * Push the current offset on the stack
     */
    void push();
    /**
     * Pop the topmost position (false if the stack was empty)
     */
    bool pop();

private:
    /**
     * we don't want to allow copying and assigning streams
     */
    OLEStream( const OLEStream& rhs );
    /**
     * we don't want to allow copying and assigning streams
     */
    OLEStream& operator=( const OLEStream& rhs );

    std::stack<int> m_positions;
    /**
     *  for bookkeeping :}
     */
    OLEStorage *m_storage;
};


class OLEStreamReader : public OLEStream
{
public:
    OLEStreamReader( GsfInput* stream, OLEStorage* storage );
    virtual ~OLEStreamReader();

    /**
     * Is this still a valid stream?
     */
    virtual bool isValid() const;

    /**
     * works like plain fseek
     */
    virtual bool seek( int offset, GSeekType whence = G_SEEK_SET );
    /**
     * works like plain ftell
     */
    virtual int tell() const;
    /**
     * The size of the stream
     */
    virtual size_t size() const;

    /**
     * Reading from the current position
     * Note: Modifies the current position!
     * All the read methods are endian-aware and convert
     * the contents from the file if necessary
     */
    U8 readU8();
    /**
     * @see readU8()
     */
    S8 readS8();
    /**
     * @see readU8()
     */
    U16 readU16();
    /**
     * @see readU8()
     */
    S16 readS16();
    /**
     * @see readU8()
     */
    U32 readU32();
    /**
     * @see readU8()
     */
    S32 readS32();

    /**
     * Reads a bunch of bytes w/o endian conversion to the
     * given buffer, at most length bytes.
     * Returns true on success
     */
    bool read( U8 *buffer, size_t length );

    /**
     * For debugging
     */
    void dumpStream( const std::string& fileName );

private:
    // we don't want to allow copying and assigning streams
    OLEStreamReader( const OLEStreamReader& rhs );
    OLEStreamReader& operator=( const OLEStreamReader& rhs );

    GsfInput* m_stream;
};


class OLEStreamWriter : public OLEStream
{
public:
    OLEStreamWriter( GsfOutput* stream, OLEStorage* storage );
    virtual ~OLEStreamWriter();

    /**
     * Is this still a valid stream?
     */
    virtual bool isValid() const;

    /**
     * works like plain fseek
     */
    virtual bool seek( int offset, GSeekType whence = G_SEEK_SET );
    /**
     * works like plain ftell
     */
    virtual int tell() const;
    /**
     * The size of the stream
     */
    virtual size_t size() const;

    /**
     * Writing to the current position
     * Note: Modifies the current position!
     * These write methods are endian-aware
     * and convert the contents to be LE in the file
     */
    void write( U8 data );
    /**
     * @see write(U8 data)
     */
    void write( S8 data );
    /**
     * @see write(U8 data)
     */
    void write( U16 data );
    /**
     * @see write(U8 data)
     */
    void write( S16 data );
    /**
     * @see write(U8 data)
     */
    void write( U32 data );
    /**
     * @see write(U8 data)
     */
    void write( S32 data );

    /**
     * Attention: This write method just writes out the
     * contents of the memory directly (w/o converting
     * to little-endian first!)
     */
    void write( U8* data, size_t length );

private:
    // we don't want to allow copying and assigning streams
    OLEStreamWriter( const OLEStreamWriter& rhs );
    OLEStreamWriter& operator=( const OLEStreamWriter& rhs );

    GsfOutput* m_stream;
};

} // namespace wvWare

#endif // OLESTREAM_H
