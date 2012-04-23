/* 
 *
 * $Id: k3bpipebuffer.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_PIPE_BUFFER_H_
#define _K3B_PIPE_BUFFER_H_


#include <k3bthreadjob.h>

/**
 * the pipebuffer uses the signal percent to show it's status.
 */
class K3bPipeBuffer : public K3bThreadJob
{
 public:
  K3bPipeBuffer( K3bJobHandler*, QObject* parent = 0, const char* name = 0 );
  ~K3bPipeBuffer();

  /**
   * Set the buffer size in MB. The default value is 4 MB.
   */
  void setBufferSize( int );

  /**
   * If this is set to -1 (which is the default) the pipebuffer
   * will create a fd pair which can be obtained by inFd() after
   * the buffer has been started.
   */
  void readFromFd( int fd );
  void writeToFd( int fd );

  /**
   * This is only valid after the piepbuffer has been started and no fd
   * has been set with readFromFd.
   */
  int inFd() const;

 public slots:
  void start();
  void cancel();

 private:
  class WorkThread;
  WorkThread* m_thread;
};

#endif
