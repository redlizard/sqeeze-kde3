/* 
 *
 * $Id: k3baudioclient.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_AUDIO_CLIENT_H_
#define _K3B_AUDIO_CLIENT_H_

#include "k3b_export.h"
/**
 * Interface for all K3b audio client classes which may attach to 
 * a K3b Audio Server to play 44100 16bit stereo audio data.
 */
class LIBK3B_EXPORT K3bAudioClient
{
 public:
  virtual ~K3bAudioClient();

  /**
   * if this method returns a value below 0 streaming is stopped.
   */
  virtual int read( char* data, int maxlen ) = 0;

 protected:
  K3bAudioClient();

  /**
   * This will start the streaming.
   */
  void startStreaming();

  /**
   * This stops the streaming,
   */
  void stopStreaming();

 private:
  bool m_attached;
};

#endif
