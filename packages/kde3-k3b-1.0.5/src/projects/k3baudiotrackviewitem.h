/* 
 *
 * $Id: k3baudiotrackviewitem.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_AUDIO_TRACK_VIEWITEM_H_
#define _K3B_AUDIO_TRACK_VIEWITEM_H_

#include <k3blistview.h>

class K3bAudioTrackView;
class K3bAudioTrack;


class K3bAudioTrackViewItem : public K3bListViewItem
{
 public:
  K3bAudioTrackViewItem( K3bAudioTrackView* parent, 
			 K3bAudioTrackViewItem* after, 
			 K3bAudioTrack* track );
  ~K3bAudioTrackViewItem();

  /**
   * If one of the sources still have length 0 we animate.
   */
  bool animate();

  K3bAudioTrack* track() const { return m_track; }

  void updateSourceItems();
  bool showingSources() const { return m_showingSources; }
  void showSources( bool show );
  void setText( int col, const QString& text );
  QString text( int i ) const;

  /**
   * @reimpl
   */
  void setSelected( bool s );
  /**
   * @reimpl
   */
  void insertItem( QListViewItem* item );

  void paintBranches( QPainter*, const QColorGroup &, int, int, int );
  void paintCell( QPainter* p, const QColorGroup& cg, int col, int width, int align );

 private:
  K3bAudioTrack* m_track;
  bool m_alreadyRemoved;
  bool m_showingSources;

  int m_animationCounter;
};

#endif
