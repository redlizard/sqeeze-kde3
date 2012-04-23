/* 
 *
 * $Id: k3bisoimageverificationjob.h 597651 2006-10-21 08:04:01Z trueg $
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

#ifndef _K3B_VERIFICATION_JOB_H_
#define _K3B_VERIFICATION_JOB_H_

#include <k3bjob.h>

namespace K3bDevice {
  class Device;
  class DeviceHandler;
}


/**
 * Generic verification job. Add tracks to be verified via addTrack.
 * The job will then verifiy the tracks set against the set checksums.
 *
 * The different track types are handled as follows:
 * \li Data/DVD tracks: Read the track with a 2048 bytes sector size.
 *     Tracks length on DVD+RW media will be read from the iso9660
 *     descriptor.
 * \li Audio tracks: Rip the track with a 2352 bytes sector size. 
 *     In the case of audio tracks the job will not fail if the checksums
 *     differ becasue audio CD tracks do not contain error correction data.
 *     In this case only a warning will be emitted.
 *
 * Other sector sizes than 2048 bytes for data tracks are not supported yet,
 * i.e. Video CDs cannot be verified.
 *
 * TAO written tracks have two run-out sectors that are not read.
 *
 * The VerificationJob will also reload the medium before starting.
 */
class K3bVerificationJob : public K3bJob
{
  Q_OBJECT

 public:
  K3bVerificationJob( K3bJobHandler*, QObject* parent = 0, const char* name = 0 );
  ~K3bVerificationJob();

 public slots:
  void start();
  void cancel();
  void setDevice( K3bDevice::Device* dev );

  void clear();

  /**
   * Add a track to be verified.
   * \param tracknum The number of the track. If \a tracknum is 0
   *        the last track will be verified.
   * \param length Set to override the track length from the TOC. This may be
   *        useful when writing to DVD+RW media and the iso descriptor does not
   *        contain the exact image size (as true for many commercial Video DVDs)
   */
  void addTrack( int tracknum, const QCString& checksum, const K3b::Msf& length = K3b::Msf() );

  /**
   * Handle the special case of iso session growing
   */
  void setGrownSessionSize( const K3b::Msf& );

 private slots:
  void slotMediaReloaded( bool success );
  void slotDiskInfoReady( K3bDevice::DeviceHandler* dh );
  void readTrack( int trackIndex );
  void slotMd5JobFinished( bool success );
  void slotReaderProgress( int p );
  void slotReaderFinished( bool success );

 private:
  K3b::Msf trackLength( int trackNum );

  class Private;
  Private* d;
};

#endif
