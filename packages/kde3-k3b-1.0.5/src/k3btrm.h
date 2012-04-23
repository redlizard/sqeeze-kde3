/* 
 *
 * $Id: k3btrm.h 630384 2007-02-05 09:33:17Z mlaurent $
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

#ifndef _K3B_TRM_H_
#define _K3B_TRM_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MUSICBRAINZ

#include <k3bmsf.h>

/**
 * This class is a wrapper around the trm part of libmusicbrainz.
 * It handles proxy settings automatically through KDE.
 *
 * K3bTRM always treats audio data as 44100, 2 channel, 16 bit data.
 */
class K3bTRM
{
 public:
  K3bTRM();
  ~K3bTRM();

  void start( const K3b::Msf& length );

  /**
   * \return true if no more data is needed
   */
  bool generate( char* data, int len );

  /**
   * \return true on success, false on error.
   */
  bool finalize();

  const QCString& rawSignature() const;
  const QCString& signature() const;

 private:
  class Private;
  Private* d;
};

#endif
#endif
