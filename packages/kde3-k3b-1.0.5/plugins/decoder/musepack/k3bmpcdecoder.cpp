/* 
 *
 * $Id: k3bmpcdecoder.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
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

#include <config.h>

#include "k3bmpcdecoder.h"
#include "k3bmpcwrapper.h"

#include <k3bpluginfactory.h>

#include <klocale.h>


K_EXPORT_COMPONENT_FACTORY( libk3bmpcdecoder, K3bPluginFactory<K3bMpcDecoderFactory>( "libk3bmpcdecoder" ) )


K3bMpcDecoderFactory::K3bMpcDecoderFactory( QObject* parent, const char* name )
  : K3bAudioDecoderFactory( parent, name )
{
}


K3bMpcDecoderFactory::~K3bMpcDecoderFactory()
{
}


K3bAudioDecoder* K3bMpcDecoderFactory::createDecoder( QObject* parent, 
							 const char* name ) const
{
  return new K3bMpcDecoder( parent, name );
}


bool K3bMpcDecoderFactory::canDecode( const KURL& url )
{
  K3bMpcWrapper w;
  return w.open( url.path() );
}






K3bMpcDecoder::K3bMpcDecoder( QObject* parent, const char* name )
  : K3bAudioDecoder( parent, name )
{
  m_mpc = new K3bMpcWrapper();
}


K3bMpcDecoder::~K3bMpcDecoder()
{
  delete m_mpc;
}


QString K3bMpcDecoder::fileType() const
{
  return i18n("Musepack");
}


bool K3bMpcDecoder::analyseFileInternal( K3b::Msf& frames, int& samplerate, int& ch )
{
  if( m_mpc->open( filename() ) ) {
    frames = m_mpc->length();
    samplerate = m_mpc->samplerate();
    ch = m_mpc->channels();

    // call addTechnicalInfo and addMetaInfo here

    return true;
  }
  else
    return false;
}


bool K3bMpcDecoder::initDecoderInternal()
{
  return m_mpc->open( filename() );
}


bool K3bMpcDecoder::seekInternal( const K3b::Msf& msf )
{
  return m_mpc->seek( msf );
}


int K3bMpcDecoder::decodeInternal( char* _data, int maxLen )
{
  return m_mpc->decode( _data, maxLen );
}


#include "k3bmpcdecoder.moc"
