/* 
 *
 * $Id: k3bvideodvdimager.h 633751 2007-02-15 08:22:49Z trueg $
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


#ifndef _K3B_VIDEODVD_IMAGER_H_
#define _K3B_VIDEODVD_IMAGER_H_


#include <k3bisoimager.h>

class K3bVideoDvdDoc;


/**
 * Create VideoDVD images with mkisofs. The difference
 * to the IsoImager is the -dvd-video option and the fact
 * that all VIDEO_TS files need to be in one local folder since
 * otherwise mkisofs is not able to find the dvd structures.
 */
class K3bVideoDvdImager : public K3bIsoImager
{
  Q_OBJECT

 public:
  K3bVideoDvdImager( K3bVideoDvdDoc* doc, K3bJobHandler*, QObject* parent = 0, const char* name = 0 );
  virtual ~K3bVideoDvdImager();

 public slots:
  virtual void start();
  virtual void init();
  virtual void calculateSize();

 protected:
  bool addMkisofsParameters( bool printSize = false );
  int writePathSpec();
  void cleanup();
  int writePathSpecForDir( K3bDirItem* dirItem, QTextStream& stream );

 protected slots:
  virtual void slotReceivedStderr( const QString& );

 private:
  void fixVideoDVDSettings();

  class Private;
  Private* d;
};

#endif
