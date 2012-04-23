/* 
 *
 * $Id: k3baudiocdtracksource.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_AUDIO_CD_TRACK_SOURCE_H_
#define _K3B_AUDIO_CD_TRACK_SOURCE_H_

#include "k3baudiodatasource.h"

#include <k3btoc.h>
#include <k3bcddbresult.h>

#include "k3b_export.h"


namespace K3bDevice {
  class Device;
}
class K3bCdparanoiaLib;


/**
 * Audio data source which reads it's data directly from an audio CD.
 *
 * Be aware that since GUI elements are not allowed in sources (other thread)
 * the source relies on the audio CD being inserted before any read operations.
 * It will search all available devices for the CD starting with the last used drive.
 */
class LIBK3B_EXPORT K3bAudioCdTrackSource : public K3bAudioDataSource
{
 public:
  /**
   * Default constructor to create a new source.
   */
  K3bAudioCdTrackSource( const K3bDevice::Toc& toc, int cdTrackNumber, const K3bCddbResultEntry& cddb, 
			 K3bDevice::Device* dev = 0 );

  /**
   * Constructor to create sources when loading from a project file without toc information
   */
  K3bAudioCdTrackSource( unsigned int discid, const K3b::Msf& length, int cdTrackNumber, 
			 const QString& artist, const QString& title,
			 const QString& cdartist, const QString& cdtitle );
  K3bAudioCdTrackSource( const K3bAudioCdTrackSource& );
  ~K3bAudioCdTrackSource();

  unsigned int discId() const { return m_discId; }
  int cdTrackNumber() const { return m_cdTrackNumber; }
  const K3bCddbResultEntry& metaInfo() const { return m_cddbEntry; }

  K3b::Msf originalLength() const;
  bool seek( const K3b::Msf& );
  int read( char* data, unsigned int max );
  QString type() const;
  QString sourceComment() const;
  K3bAudioDataSource* copy() const;

  /**
   * Searches for the corresponding Audio CD and returns the device in which it has
   * been found or 0 if it could not be found.
   */
  K3bDevice::Device* searchForAudioCD() const;

  /**
   * Set the device the source should start to look for the CD.
   */
  void setDevice( K3bDevice::Device* dev );

 private:
  bool initParanoia();
  void closeParanoia();
  bool searchForAudioCD( K3bDevice::Device* ) const;

  unsigned int m_discId;
  K3b::Msf m_length;
  K3bDevice::Toc m_toc;
  int m_cdTrackNumber;
  K3bCddbResultEntry m_cddbEntry;

  // ripping
  // we only save the device we last saw the CD in
  K3bDevice::Device* m_lastUsedDevice;
  K3bCdparanoiaLib* m_cdParanoiaLib;
  K3b::Msf m_position;
  bool m_initialized;
};

#endif
