/* 
 *
 * $Id: k3blibdvdcss.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_LIBDVDCSS_H_
#define _K3B_LIBDVDCSS_H_

#include "k3b_export.h"

namespace K3bDevice {
  class Device;
}


/**
 * Wrapper class for libdvdcss. dynamically openes the library if it
 * is available on the system.
 */
class LIBK3B_EXPORT K3bLibDvdCss
{
 public:
  ~K3bLibDvdCss();

  static const int DVDCSS_BLOCK_SIZE = 2048;
  static const int DVDCSS_NOFLAGS = 0;
  static const int DVDCSS_READ_DECRYPT = (1 << 0);
  static const int DVDCSS_SEEK_MPEG = (1 << 0);
  static const int DVDCSS_SEEK_KEY = (1 << 1);

  /**
   * Try to open a Video DVD and authenticate it.
   * @return true if the Video DVD could be authenticated successfully, false otherwise.
   */
  bool open( K3bDevice::Device* dev );
  void close();

  int seek( int sector, int flags );
  int read( void* buffer, int sectors, int flags );

  /**
   * This method optimized the seek calls to maximize reading performance.
   * It also makes sure we never read unscrambled and scrambled data at the same time.
   *
   * You have to call crackAllKeys() before using this. Do never call this in combination
   * with seek or read!
   */
  int readWrapped( void* buffer, int firstSector, int sectors );

  /**
   * Cache all CSS keys to guarantee smooth reading further on.
   * This method also creates a title offset list which is needed by readWrapped.
   */
  bool crackAllKeys();

  /**
   * returns 0 if the libdvdcss could not
   * be found on the system.
   * Otherwise you have to take care of
   * deleting.
   */
  static K3bLibDvdCss* create();

 private:
  class Private;
  Private* d;

  K3bLibDvdCss();

  static void* s_libDvdCss;
  static int s_counter;
};

#endif
