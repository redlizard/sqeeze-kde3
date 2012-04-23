/* 
 *
 * $Id: k3boggvorbisdecoder.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_OGGVORBIS_DECODER_H_
#define _K3B_OGGVORBIS_DECODER_H_


#include <k3baudiodecoder.h>

class KURL;


class K3bOggVorbisDecoderFactory : public K3bAudioDecoderFactory
{
  Q_OBJECT

 public:
  K3bOggVorbisDecoderFactory( QObject* parent = 0, const char* name = 0 );
  ~K3bOggVorbisDecoderFactory();

  bool canDecode( const KURL& filename );

  int pluginSystemVersion() const { return 3; }

  K3bAudioDecoder* createDecoder( QObject* parent = 0, 
				  const char* name = 0 ) const;
};


/**
  *@author Sebastian Trueg
  */
class K3bOggVorbisDecoder : public K3bAudioDecoder
{
  Q_OBJECT

 public: 
  K3bOggVorbisDecoder( QObject* parent = 0, const char* name = 0 );
  ~K3bOggVorbisDecoder();

  void cleanup();

  QString fileType() const;

 protected:
  bool analyseFileInternal( K3b::Msf& frames, int& samplerate, int& ch );
  bool initDecoderInternal();
  bool seekInternal( const K3b::Msf& );

  int decodeInternal( char* _data, int maxLen );

 private:
  bool openOggVorbisFile();

  class Private;
  Private* d;
};

#endif
