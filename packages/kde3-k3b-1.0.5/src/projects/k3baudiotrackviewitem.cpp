/* 
 *
 * $Id: k3baudiotrackviewitem.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3baudiotrackviewitem.h"
#include "k3baudiodatasourceviewitem.h"
#include "k3baudiodatasource.h"
#include "k3baudiotrackview.h"
#include "k3baudiotrack.h"
#include <k3bcdtextvalidator.h>

#include <kiconloader.h>

#include <qpainter.h>


K3bAudioTrackViewItem::K3bAudioTrackViewItem( K3bAudioTrackView* parent, 
					      K3bAudioTrackViewItem* after, 
					      K3bAudioTrack* track )
  : K3bListViewItem( parent, after ),
    m_track( track ),
    m_alreadyRemoved(false),
    m_showingSources(false),
    m_animationCounter(1)
{
  // columns
  // 0 - No.
  // 1 - Artist (CD-Text)
  // 2 - Title (CD-Text)
  // 3 - Type
  // 4 - Pregap
  // 5 - Length
  // 6 - Filename

  //  animationIconNumber = 1;
  setEditor( 1, LINE );
  setEditor( 2, LINE );
  setValidator( 1, new K3bCdTextValidator() );
  setValidator( 2, validator(1) );

  //  setMarginVertical( 5 );

  // italic type
  QFont f(listView()->font());
  f.setItalic( true );
  setFont( 3, f );

  // gray out filename
  setForegroundColor( 5, listView()->palette().disabled().foreground() );

  // smaller filename
  f = listView()->font();
  f.setPointSize( f.pointSize() - 2 );
  setFont( 5, f );

  updateSourceItems();
}


K3bAudioTrackViewItem::~K3bAudioTrackViewItem()
{
  delete validator(1);
}


void K3bAudioTrackViewItem::paintCell( QPainter* p, const QColorGroup& cg, int col, int width, int align )
{
  K3bListViewItem::paintCell( p, cg, col, width, align );

  // draw the separator
  if( listView()->firstChild() != this ) {
    p->save();
    // FIXME: modify the value from palette().disabled().foreground() to be lighter (or darker, depending on the background color )
    p->setPen( Qt::lightGray );
    p->drawLine( 0, 0, width, 0 );
    p->restore();
  }
}


void K3bAudioTrackViewItem::paintBranches( QPainter* p, const QColorGroup& cg, int w, int, int h )
{
  // we just want empty space
  p->fillRect( QRect( 0, 0, w, h ), cg.base() );
}


QString K3bAudioTrackViewItem::text(int i) const
{
  // to avoid crashes when the track has been deleted and this viewitem is still around
  if( m_alreadyRemoved )
    return QString::null;

  //
  // We add two spaces after all strings (except the once renameable)
  // to increase readability
  //

  switch( i )
    {
    case 0:
      return QString::number( m_track->trackNumber() ).rightJustify( 2, ' ' );
    case 1:
      return m_track->performer();
    case 2:
      return m_track->title();
    case 3:
      if( m_showingSources )
	return QString::null;
      else
	return m_track->firstSource()->type();
    case 4:
      return m_track->length().toString();
    case 5:
      if( m_showingSources )
	return QString::null;
      else
	return m_track->firstSource()->sourceComment();
    default:
      return KListViewItem::text(i);
    }
}

void K3bAudioTrackViewItem::setText( int col, const QString& text )
{
  //
  // Stupid QListViewItem actually calls setText in paintCell. Thus, once a new item
  // is created setText is called and in turn the doc is marked as modified since
  // we call setArtist or setPerformer here! :(
  //
  // Quick fix: check if the field actually changed
  //
  if( col == 1 ) {
    // this is the cd-text artist field
    if( text != m_track->performer() )
      m_track->setPerformer( text );
  }
  else if( col == 2 ) {
    // this is the cd-text title field
    if( text != m_track->title() )
      m_track->setTitle( text );
  }

  KListViewItem::setText( col, text );
}


void K3bAudioTrackViewItem::showSources( bool show )
{
  setOpen(show);
  m_showingSources = show;
}


void K3bAudioTrackViewItem::updateSourceItems()
{
  while( firstChild() )
    delete firstChild();

  K3bAudioDataSource* source = track()->firstSource();
  K3bAudioDataSourceViewItem* sourceItem = 0;
  while( source ) {
    sourceItem = new K3bAudioDataSourceViewItem( this, sourceItem, source );
    sourceItem->animate();
    source = source->next();
  }
}


bool K3bAudioTrackViewItem::animate()
{
  //
  // We animate if one of the sources have length == 0
  // otherwise we set the led
  //
  bool animate = false;
  bool valid = true;
  QListViewItem* item = firstChild();
  while( item ) {
    K3bAudioDataSourceViewItem* sourceItem = dynamic_cast<K3bAudioDataSourceViewItem*>( item );
    animate = animate || sourceItem->animate();
    valid = valid && sourceItem->source()->isValid();
    item = item->nextSibling();
  }
  if( animate ) {
    QString icon = QString( "kde%1" ).arg( m_animationCounter );
    setPixmap( 4, SmallIcon( icon ) );
    m_animationCounter++;
    if ( m_animationCounter > 6 )
      m_animationCounter = 1;
  }
  else {
    // set status icon
    setPixmap( 4, ( valid ? SmallIcon( "greenled" ) : SmallIcon( "redled" ) ) );
  }
  return animate;
}


void K3bAudioTrackViewItem::setSelected( bool s )
{
  K3bListViewItem::setSelected(s);

  // we also select or unselect all source items
  QListViewItem* item = firstChild();
  while( item ) {
    item->setSelected(s);
    item = item->nextSibling();
  }
}


void K3bAudioTrackViewItem::insertItem( QListViewItem* item )
{
  K3bListViewItem::insertItem( item );
  if( isSelected() )
    item->setSelected(true);
}
