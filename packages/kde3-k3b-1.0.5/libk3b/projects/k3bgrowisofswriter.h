/* 
 *
 * $Id: k3bgrowisofswriter.h 679276 2007-06-23 13:25:21Z trueg $
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

#ifndef _K3B_GROWISOFS_WRITER_H_
#define _K3B_GROWISOFS_WRITER_H_

#include "k3babstractwriter.h"


namespace K3bDevice {
  class Device;
  class DeviceHandler;
}
class KProcess;



class K3bGrowisofsWriter : public K3bAbstractWriter
{
  Q_OBJECT

 public:
  K3bGrowisofsWriter( K3bDevice::Device*, K3bJobHandler*,
		      QObject* parent = 0, const char* name = 0 );
  ~K3bGrowisofsWriter();

  bool active() const;

  int fd() const;
  bool closeFd();

 public slots:
  void start();
  void cancel();

  void setWritingMode( int );

  /**
   * If true the growisofs parameter -M is used in favor of -Z.
   */
  void setMultiSession( bool b );

  /**
   * Only used in DAO mode and only supported with growisofs >= 5.15
   * @param size size in blocks
   */
  void setTrackSize( long size );

  /**
   * Use this in combination with setTrackSize when writing double layer media.
   * @param lb The number of data sectors in the first layer. It needs to be less or equal
   *           to tracksize/2. The writer will pad the second layer with zeros if
   *           break < tracksize/2.
   *           If set to 0 this setting will be ignored.
   */
  void setLayerBreak( long lb );

  /**
   * Close the DVD to enable max DVD compatibility (uses the growisofs --dvd-compat parameter)
   * This will also be used in DAO mode and when the layerBreak has been set.
   */
  void setCloseDvd( bool );

  /**
   * set this to QString::null or an empty string to let the writer
   * read it's data from fd()
   */
  void setImageToWrite( const QString& );

  /**
   * While reading the image from stdin growisofs needs 
   * a valid -C parameter for multisession.
   */
  void setMultiSessionInfo( const QString& );

  void setForceNoEject( bool );

 protected:
  bool prepareProcess();

 protected slots:
  void slotReceivedStderr( const QString& );
  void slotProcessExited( KProcess* );
  void slotEjectingFinished( K3bDevice::DeviceHandler* dh );
  void slotThroughput( int t );
  void slotFlushingCache();
  void slotRingBufferFinished( bool );
  
 private:
  class Private;
  Private* d;
};

#endif
