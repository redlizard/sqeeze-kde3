/* 
 *
 * $Id: k3baudioclient.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3baudioclient.h"
#include "k3baudioserver.h"


K3bAudioClient::K3bAudioClient()
  : m_attached(false)
{
}


K3bAudioClient::~K3bAudioClient()
{
}


void K3bAudioClient::startStreaming()
{
  if( !m_attached ) {
    K3bAudioServer::instance()->attachClient( this );
    m_attached = true;
  }
}


void K3bAudioClient::stopStreaming()
{
  if( m_attached ) {
    K3bAudioServer::instance()->detachClient( this );
    m_attached = false;
  }
}
