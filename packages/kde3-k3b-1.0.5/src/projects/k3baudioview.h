/* 
 *
 * $Id: k3baudioview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BAUDIOVIEW_H
#define K3BAUDIOVIEW_H

#include <k3bview.h>
#include "k3baudiotrackview.h"

#include <qstringlist.h>


class K3bAudioDoc;
class K3bAudioTrack;
class K3bAudioTrackView;


/**
  *@author Sebastian Trueg
  */
class K3bAudioView : public K3bView
{
  Q_OBJECT
	
 public: 
  K3bAudioView( K3bAudioDoc* pDoc, QWidget* parent, const char *name = 0 );
  ~K3bAudioView();

  K3bAudioTrackPlayer* player() const { return m_songlist->player(); }

 public slots:
  void addUrls( const KURL::List& );

 protected:
  K3bProjectBurnDialog* newBurnDialog( QWidget* parent = 0, const char* name = 0 );

  void init();

 private slots:
  void slotAudioConversion();

 private:
  K3bAudioDoc* m_doc;
	
  K3bAudioTrackView* m_songlist;
};

#endif
