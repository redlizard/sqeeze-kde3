/* 
 *
 * $Id: k3baudiozerodata.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "k3baudiozerodata.h"
#include "k3baudiotrack.h"

#include <klocale.h>

#include <string.h>


K3bAudioZeroData::K3bAudioZeroData( const K3b::Msf& len )
  : K3bAudioDataSource(),
    m_length(len),
    m_writtenData(0)
{
}


K3bAudioZeroData::K3bAudioZeroData( const K3bAudioZeroData& zero )
  : K3bAudioDataSource( zero ),
    m_length( zero.m_length ),
    m_writtenData( 0 )
{
}


K3bAudioZeroData::~K3bAudioZeroData()
{
}


void K3bAudioZeroData::setLength( const K3b::Msf& msf )
{
  if( msf > 0 )
    m_length = msf;
  else
    m_length = 1; // 1 frame

  m_writtenData = 0;

  emitChange();
}


QString K3bAudioZeroData::type() const
{
  return i18n("Silence");
}


QString K3bAudioZeroData::sourceComment() const
{
  return QString::null;
}


bool K3bAudioZeroData::seek( const K3b::Msf& msf )
{
  if( msf < length() ) {
    m_writtenData = msf.audioBytes();
    return true;
  }
  else
    return false;
}


int K3bAudioZeroData::read( char* data, unsigned int max )
{
  if( m_writtenData + max > length().audioBytes() )
    max = length().audioBytes() - m_writtenData;

  m_writtenData += max;

  ::memset( data, 0, max );

  return max;
}


K3bAudioDataSource* K3bAudioZeroData::copy() const
{
  return new K3bAudioZeroData( *this );
}


void K3bAudioZeroData::setStartOffset( const K3b::Msf& pos )
{
  if( pos >= length() )
    setLength( 1 );
  else
    setLength( length() - pos );
}


void K3bAudioZeroData::setEndOffset( const K3b::Msf& pos )
{
  if( pos < 1 )
    setLength( 1 );
  else
    setLength( pos );
}
