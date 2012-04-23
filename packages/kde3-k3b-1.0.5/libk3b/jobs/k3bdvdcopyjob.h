/* 
 *
 * $Id: k3bdvdcopyjob.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_DVD_COPY_JOB_H_
#define _K3B_DVD_COPY_JOB_H_

#include <k3bjob.h>
#include "k3b_export.h"
#include <qstring.h>


namespace K3bDevice {
  class Device;
  class DeviceHandler;
}


class LIBK3B_EXPORT K3bDvdCopyJob : public K3bBurnJob
{
  Q_OBJECT

 public:
  K3bDvdCopyJob( K3bJobHandler* hdl, QObject* parent = 0, const char* name = 0 );
  ~K3bDvdCopyJob();

  K3bDevice::Device* writer() const { return m_onlyCreateImage ? 0 : m_writerDevice; }
  K3bDevice::Device* readingDevice() const { return m_readerDevice; }

  QString jobDescription() const;
  QString jobDetails() const;

 public slots:
  void start();
  void cancel();

  void setWriterDevice( K3bDevice::Device* w ) { m_writerDevice = w; }
  void setReaderDevice( K3bDevice::Device* w ) { m_readerDevice = w; }
  void setImagePath( const QString& p ) { m_imagePath = p; }
  void setRemoveImageFiles( bool b ) { m_removeImageFiles = b; }
  void setOnlyCreateImage( bool b ) { m_onlyCreateImage = b; }
  void setSimulate( bool b ) { m_simulate = b; }
  void setOnTheFly( bool b ) { m_onTheFly = b; }
  void setWriteSpeed( int s ) { m_speed = s; }
  void setCopies( int c ) { m_copies = c; }
  void setWritingMode( int w ) { m_writingMode = w; }
  void setIgnoreReadErrors( bool b ) { m_ignoreReadErrors = b; }
  void setReadRetries( int i ) { m_readRetries = i; }
  void setVerifyData( bool b );

 private slots:
  void slotDiskInfoReady( K3bDevice::DeviceHandler* );
  void slotReaderProgress( int );
  void slotReaderProcessedSize( int, int ); 
  void slotWriterProgress( int );
  void slotReaderFinished( bool );
  void slotWriterFinished( bool );
  void slotVerificationFinished( bool );
  void slotVerificationProgress( int p );

 private:
  bool waitForDvd();
  void prepareReader();
  void prepareWriter();
  void removeImageFiles();

  K3bDevice::Device* m_writerDevice;
  K3bDevice::Device* m_readerDevice;
  QString m_imagePath;

  bool m_onTheFly;
  bool m_removeImageFiles;

  bool m_simulate;
  int m_speed;
  int m_copies;
  bool m_onlyCreateImage;
  bool m_ignoreReadErrors;
  int m_readRetries;

  int m_writingMode;

  class Private;
  Private* d;
};


#endif
