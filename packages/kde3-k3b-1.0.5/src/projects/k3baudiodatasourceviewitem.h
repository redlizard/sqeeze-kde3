/* 
 *
 * $Id: k3baudiodatasourceviewitem.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_AUDIO_DATA_SOURCE_VIEWITEM_H_
#define _K3B_AUDIO_DATA_SOURCE_VIEWITEM_H_

#include <k3blistview.h>

class K3bAudioTrack;
class K3bAudioDataSource;
class K3bAudioTrackViewItem;

class K3bAudioDataSourceViewItem : public K3bListViewItem
{
 public:
  K3bAudioDataSourceViewItem( K3bAudioTrackViewItem* parent, 
			      K3bAudioDataSourceViewItem* after, 
			      K3bAudioDataSource* );

  K3bAudioDataSource* source() const { return m_source; }
  K3bAudioTrackViewItem* trackViewItem() const { return m_trackViewItem; }

  QString text( int i ) const;
  void setText( int col, const QString& text );

  bool animate();

  void setSelected( bool s );

  /**
   * Does nothing becasue we don't want no branches here.
   */
  void paintBranches( QPainter*, const QColorGroup &,
		      int, int, int ) {}

 private:
  K3bAudioTrackViewItem* m_trackViewItem;
  K3bAudioDataSource* m_source;
  int m_animationCounter;
};

#endif
