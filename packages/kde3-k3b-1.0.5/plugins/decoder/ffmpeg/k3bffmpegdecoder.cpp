/* 
 *
 * $Id: k3bffmpegdecoder.cpp 641798 2007-03-12 16:07:10Z trueg $
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

#include "k3bffmpegdecoder.h"
#include "k3bffmpegwrapper.h"

#include <kdebug.h>
#include <k3bpluginfactory.h>

extern "C" {
#include <ffmpeg/avcodec.h>
}

#include <math.h>


K_EXPORT_COMPONENT_FACTORY( libk3bffmpegdecoder, K3bPluginFactory<K3bFFMpegDecoderFactory>( "k3bffmpegdecoder" ) )


K3bFFMpegDecoderFactory::K3bFFMpegDecoderFactory( QObject* parent, const char* name )
  : K3bAudioDecoderFactory( parent, name )
{
}


K3bFFMpegDecoderFactory::~K3bFFMpegDecoderFactory()
{
}


K3bAudioDecoder* K3bFFMpegDecoderFactory::createDecoder( QObject* parent, 
							 const char* name ) const
{
  return new K3bFFMpegDecoder( parent, name );
}


bool K3bFFMpegDecoderFactory::canDecode( const KURL& url )
{
  K3bFFMpegFile* file = K3bFFMpegWrapper::instance()->open( url.path() );
  if( file ) {
    delete file;
    return true;
  }
  else {
    return false;
  }
}






K3bFFMpegDecoder::K3bFFMpegDecoder( QObject* parent, const char* name )
  : K3bAudioDecoder( parent, name ),
    m_file(0)
{
}


K3bFFMpegDecoder::~K3bFFMpegDecoder()
{
}


QString K3bFFMpegDecoder::fileType() const
{
  return m_type;
}


bool K3bFFMpegDecoder::analyseFileInternal( K3b::Msf& frames, int& samplerate, int& ch )
{
  m_file = K3bFFMpegWrapper::instance()->open( filename() );
  if( m_file ) {

    // TODO: call addTechnicalInfo

    addMetaInfo( META_TITLE, m_file->title() );
    addMetaInfo( META_ARTIST, m_file->author() );
    addMetaInfo( META_COMMENT, m_file->comment() );

    samplerate = m_file->sampleRate();
    ch = m_file->channels();
    m_type = m_file->typeComment();
    frames = m_file->length();

    // ffmpeg's length information is not reliable at all
    // so we have to decode the whole file in order to get the correct length
//     char buffer[10*2048];
//     int len = 0;
//     unsigned long long bytes = 0;
//     while( ( len = m_file->read( buffer, 10*2048 ) ) > 0 )
//       bytes += len;
    
//     frames = (unsigned long)ceil((double)bytes/2048.0);

    // cleanup;
    delete m_file;
    m_file = 0;

    return true;
  }
  else
    return false;
}


bool K3bFFMpegDecoder::initDecoderInternal()
{
  if( !m_file )
    m_file = K3bFFMpegWrapper::instance()->open( filename() );

  return (m_file != 0);
}


void K3bFFMpegDecoder::cleanup()
{
  delete m_file;
  m_file = 0;
}


bool K3bFFMpegDecoder::seekInternal( const K3b::Msf& msf )
{
  if( msf == 0 )
    return initDecoderInternal();
  else
    return m_file->seek( msf );
}


int K3bFFMpegDecoder::decodeInternal( char* _data, int maxLen )
{
  return m_file->read( _data, maxLen );
}


#include "k3bffmpegdecoder.moc"
