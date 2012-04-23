/* 
 *
 * $Id: k3blibsndfiledecoder.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Matthieu Bedouet <mbedouet@no-log.org>
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

#include "k3blibsndfiledecoder.h"

#include <k3bpluginfactory.h>

#include <qfile.h>
#include <qstringlist.h>

#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinstance.h>


#include <math.h>
#include <stdio.h>
#include <sndfile.h>


K_EXPORT_COMPONENT_FACTORY( libk3blibsndfiledecoder, K3bPluginFactory<K3bLibsndfileDecoderFactory>( "libk3blibsndfiledecoder" ) )


class K3bLibsndfileDecoder::Private
{
public:
  Private(): 
    isOpen(false),
    buffer(0),
    bufferSize(0) {
    format_info.name = 0;
  }
  
  SNDFILE *sndfile;
  SF_INFO sndinfo;
  SF_FORMAT_INFO format_info;
  bool isOpen;
  float* buffer;
  int bufferSize;
};



K3bLibsndfileDecoder::K3bLibsndfileDecoder( QObject* parent, const char* name )
  : K3bAudioDecoder( parent, name )
{
  d = new Private();
}


K3bLibsndfileDecoder::~K3bLibsndfileDecoder()
{
  delete d;
}


QString K3bLibsndfileDecoder::fileType() const
{
  if( d->format_info.name )
    return QString::fromLocal8Bit(d->format_info.name);
  else
    return "-";
}
	


bool K3bLibsndfileDecoder::openFile()
{
  if( !d->isOpen ) {
    
    cleanup();
    
    d->sndinfo.format = 0;
    d->sndfile = sf_open (QFile::encodeName(filename()), SFM_READ, &d->sndinfo);
    if ( !d->sndfile ) {
      kdDebug() << "(K3bLibsndfileDecoder::openLibsndfileFile) : " << sf_strerror(d->sndfile) << endl;
      return false;
    }
    else {
      //retrieve infos (name, extension) about the format:
      d->format_info.format = d->sndinfo.format & SF_FORMAT_TYPEMASK ;
      sf_command (d->sndfile, SFC_GET_FORMAT_INFO, &d->format_info, sizeof (SF_FORMAT_INFO)) ;
      
      d->isOpen = true;
      kdDebug() << "(K3bLibsndfileDecoder::openLibsndfileFile) " << d->format_info.name << " file opened " << endl;
      return true;
    }
  }

  return d->isOpen;
}


bool K3bLibsndfileDecoder::analyseFileInternal( K3b::Msf& frames, int& samplerate, int& ch )
{
  cleanup();
  
  if( openFile() ) {
    // check length of track
    if ( d->sndinfo.frames <= 0 ) {
      kdDebug() << "(K3bLibsndfileDecoder::analyseFileInternal) Could not determine length of file "
		<< filename() << endl;
      cleanup();
      return false;
    }
    else {
      addMetaInfo( META_TITLE, sf_get_string(d->sndfile, SF_STR_TITLE) );
      addMetaInfo( META_ARTIST, sf_get_string(d->sndfile, SF_STR_ARTIST) );
      addMetaInfo( META_COMMENT, sf_get_string(d->sndfile, SF_STR_COMMENT) );
      
      addTechnicalInfo( i18n("Channels"), QString::number(d->sndinfo.channels) );
      addTechnicalInfo( i18n("Sampling Rate"), i18n("%1 Hz").arg(d->sndinfo.samplerate) );
      
      frames = (unsigned long)ceil(d->sndinfo.frames / d->sndinfo.samplerate * 75.0); 
      samplerate = d->sndinfo.samplerate;
      ch = d->sndinfo.channels;

      kdDebug() << "(K3bLibsndfileDecoder) successfully analysed file: " << frames << " frames." << endl;

      cleanup();
      return true;
    }
  }
  else
    return false;
}



bool K3bLibsndfileDecoder::initDecoderInternal()
{	
  cleanup();
  return openFile();
}



int K3bLibsndfileDecoder::decodeInternal( char* data, int maxLen )
{
  if( !d->buffer ) {
    d->buffer = new float[maxLen];
    d->bufferSize = maxLen/2;
  }
  
  int read = (int) sf_read_float(d->sndfile, d->buffer,d->bufferSize) ;
  fromFloatTo16BitBeSigned( d->buffer, data, read );
  read = read * 2;
  
  if( read < 0 ) {
    kdDebug() << "(K3bLibsndfileDecoder::decodeInternal) Error: " << read << endl;
    return -1;
  }
  else if( read == 0 ) {
    kdDebug() << "(K3bLibsndfileDecoder::decodeInternal) successfully finished decoding." << endl;
    return 0;
  }
  else if( read != maxLen ) {
    kdDebug() << "(K3bLibsndfileDecoder::decodeInternal) read:" << read << " expected:" << maxLen << endl;
    return -1;
  }
  else 
    return read;
 }



bool K3bLibsndfileDecoder::seekInternal( const K3b::Msf& pos)
{
  return ( sf_seek( d->sndfile, pos.pcmSamples(), SEEK_SET ) == 0 );
}




void K3bLibsndfileDecoder::cleanup()
{
  if( d->isOpen ) {
    kdDebug() << "(K3bLibsndfileDecoder) cleaning up." << endl;
    sf_close( d->sndfile );
    d->isOpen = false;
  }
}



/********************************************************/


K3bLibsndfileDecoderFactory::K3bLibsndfileDecoderFactory( QObject* parent, const char* name )
  : K3bAudioDecoderFactory( parent, name )
{
}


K3bLibsndfileDecoderFactory::~K3bLibsndfileDecoderFactory()
{
}


K3bAudioDecoder* K3bLibsndfileDecoderFactory::createDecoder( QObject* parent, 
							     const char* name ) const
{
  return new K3bLibsndfileDecoder( parent, name );
}


bool K3bLibsndfileDecoderFactory::canDecode( const KURL& url )
{
  SF_INFO infos;
  infos.format = 0;
  SNDFILE* sndfile = sf_open (QFile::encodeName(url.path()), SFM_READ, &infos);
  
  //is it supported by libsndfile?
  if ( !sndfile ) {
    kdDebug() << "(K3bLibsndfileDecoder) " << sf_strerror(sndfile) << endl;
    return false;
  }
  //we exclude only WAVE as there is another plugin for this
  else if ( infos.format && ((infos.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) )  {
    
    //retrieve infos (name) about the format:
    SF_FORMAT_INFO format_info;
    format_info.format = infos.format & SF_FORMAT_TYPEMASK ;
    sf_command (sndfile, SFC_GET_FORMAT_INFO, &format_info, sizeof (format_info)) ;
    
    kdDebug() << "(K3bLibsndfileDecoder) " << format_info.name << " file === OK === "  << endl;
    sf_close( sndfile );
    return true;
  }
  else {
    kdDebug() << "(K3bLibsndfileDecoder) " << url.path() << "not supported" << endl;
    sf_close( sndfile );
    return false;
  }
  return false;
}

#include "k3blibsndfiledecoder.moc"
