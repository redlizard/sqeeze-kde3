/* 
 *
 * $Id: k3bdvdrecordwriter.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_DVDRECORD_WRITER_H_
#define _K3B_DVDRECORD_WRITER_H_

#include "k3bcdrecordwriter.h"


class K3bDevice::Device;

/**
 * Basically this is just a wrapper around K3bCdrecordWriter
 * which uses another K3bExternalBin and ignores the writingMode setting.
 */
class K3bDvdrecordWriter : public K3bCdrecordWriter
{
  Q_OBJECT

 public:
  K3bDvdrecordWriter( K3bDevice::Device*, QObject* parent = 0, const char* name = 0 );
  ~K3bDvdrecordWriter();

 protected:
  void prepareProcess();
};

#endif
