/* 
 *
 * $Id: k3bvideodvdjob.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_VIDEO_DVD_JOB_H_
#define _K3B_VIDEO_DVD_JOB_H_

#include <k3bdvdjob.h>


class K3bVideoDvdDoc;

/**
 * This class heavily depends on K3bDvdJob and uses some of it's internals.
 */
class K3bVideoDvdJob : public K3bDvdJob
{
  Q_OBJECT

 public:
  K3bVideoDvdJob( K3bVideoDvdDoc*, K3bJobHandler*, QObject* parent = 0 );
  virtual ~K3bVideoDvdJob();

  virtual QString jobDescription() const;
  virtual QString jobDetails() const;

 private:
  bool prepareWriterJob();
  void prepareImager();

  K3bVideoDvdDoc* m_doc;
};

#endif
