/* 
 *
 * $Id: skeleton.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3b<name>decoder.h"

#include <k3bpluginfactory.h>


K_EXPORT_COMPONENT_FACTORY( libk3b<name>decoder, K3bPluginFactory<K3b<name>DecoderFactory>( "libk3b<name>decoder" ) )


K3b<name>DecoderFactory::K3b<name>DecoderFactory( QObject* parent, const char* name )
  : K3bAudioDecoderFactory( parent, name )
{
}


K3b<name>DecoderFactory::~K3b<name>DecoderFactory()
{
}


K3bAudioDecoder* K3b<name>DecoderFactory::createDecoder( QObject* parent, 
							 const char* name ) const
{
  return new K3b<name>Decoder( parent, name );
}


bool K3b<name>DecoderFactory::canDecode( const KURL& url )
{
  // PUT YOUR CODE HERE
  return false;
}






K3b<name>Decoder::K3b<name>Decoder( QObject* parent, const char* name )
  : K3bAudioDecoder( parent, name )
{
}


K3b<name>Decoder::~K3b<name>Decoder()
{
}


QString K3b<name>Decoder::fileType() const
{
  // PUT YOUR CODE HERE
}


bool K3b<name>Decoder::analyseFileInternal( K3b::Msf& frames, int& samplerate, int& ch )
{
  // PUT YOUR CODE HERE
  // call addTechnicalInfo and addMetaInfo here
  return false;
}


bool K3b<name>Decoder::initDecoderInternal()
{
  // PUT YOUR CODE HERE
  return false;
}


bool K3b<name>Decoder::seekInternal( const K3b::Msf& )
{
  // PUT YOUR CODE HERE
  return false;
}


int K3b<name>Decoder::decodeInternal( char* _data, int maxLen )
{
  // PUT YOUR CODE HERE
  return -1;
}


#include "k3b<name>decoder.moc"
