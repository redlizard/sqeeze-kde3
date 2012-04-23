/* 
 *
 * $Id: k3bmpcwrapper.h 630384 2007-02-05 09:33:17Z mlaurent $
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

#ifndef _K3B_MPC_WRAPPER_H_
#define _K3B_MPC_WRAPPER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>

#include <k3bmsf.h>

#include "mpcdec/mpcdec.h"

class QFile;


class K3bMpcWrapper
{
 public:
  K3bMpcWrapper();
  ~K3bMpcWrapper();

  bool open( const QString& filename );
  void close();

  int decode( char*, int max );

  bool seek( const K3b::Msf& );

  K3b::Msf length() const;
  int samplerate() const;
  unsigned int channels() const;

  QFile* input() const { return m_input; }

 private:
  QFile* m_input;
  mpc_reader* m_reader;
  mpc_decoder* m_decoder;
  mpc_streaminfo* m_info;
};

#endif
