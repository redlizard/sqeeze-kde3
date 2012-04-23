/* 
 *
 * $Id: k3baudioeditorwidget.cpp 630444 2007-02-05 12:43:19Z trueg $
 * Copyright (C) 2004-2007 Sebastian Trueg <trueg@k3b.org>
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

#include "k3baudioeditorwidget.h"

#include <qpainter.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qtooltip.h>



class K3bAudioEditorWidget::Range
{
public:
  Range( int i,
	 const K3b::Msf& s,
	 const K3b::Msf& e,
	 bool sf,
	 bool ef,
	 const QString& t,
	 const QBrush& b )
    : id(i),
      start(s),
      end(e),
      startFixed(sf),
      endFixed(ef),
      brush(b),
      toolTip(t) {
  }

  int id;
  K3b::Msf start;
  K3b::Msf end;
  bool startFixed;
  bool endFixed;
  QBrush brush;
  QString toolTip;

  bool operator<( const K3bAudioEditorWidget::Range& r ) {
    return start < r.start;
  }
  bool operator>( const K3bAudioEditorWidget::Range& r ) {
    return start > r.start;
  }
  bool operator==( const K3bAudioEditorWidget::Range& r ) {
    return start == r.start;
  }
};


class K3bAudioEditorWidget::Marker
{
public:
  Marker( int i,
	  const K3b::Msf& msf,
	  bool f,
	  const QColor& c, 
	  const QString& t )
    : id(i),
      pos(msf),
      fixed(f),
      color(c),
      toolTip(t) {
  }

  int id;
  K3b::Msf pos;
  bool fixed;
  QColor color;
  QString toolTip;

  operator K3b::Msf& () { return pos; }
};


class K3bAudioEditorWidget::RangeList : public QPtrList<K3bAudioEditorWidget::Range>
{
public:
  RangeList()
    : QPtrList<Range>() {
  }

  RangeList( const RangeList& l )
    : QPtrList<Range>( l ) {
  }

  int compareItems( QPtrCollection::Item item1, QPtrCollection::Item item2 ) {
    if( *static_cast<Range*>(item1) > *static_cast<Range*>(item2) )
      return 1;
    else if( *static_cast<Range*>(item1) < *static_cast<Range*>(item2) )
      return -1;
    else
      return 0;
  }
};

class K3bAudioEditorWidget::ToolTip : public QToolTip
{
public:
  ToolTip( K3bAudioEditorWidget* w ) 
    : QToolTip( w ),
      m_editorWidget( w ) {
  }

protected:
  void maybeTip( const QPoint& p ) {
    QRect r = m_editorWidget->contentsRect();
    Marker* m = m_editorWidget->findMarker( p );
    if( m ) {
      r.setLeft( p.x() - 1 );
      r.setRight( p.x() + 1 );
      tip( r, m->toolTip.isEmpty() ? m->pos.toString() : QString("%1 (%2)").arg(m->toolTip).arg(m->pos.toString()) );
    }
    else {
      Range* range = m_editorWidget->findRange( p );
      if( range ) {
	r.setLeft( m_editorWidget->msfToPos( range->start ) );
	r.setRight( m_editorWidget->msfToPos( range->end ) );
	tip( r, 
	     range->toolTip.isEmpty()
	     ? QString("%1 - %2").arg(range->start.toString()).arg(range->end.toString())
	     : QString("%1 (%2 - %3)").arg(range->toolTip).arg(range->start.toString()).arg(range->end.toString()) );
      }
    }
  }

private:
  K3bAudioEditorWidget* m_editorWidget;
};


class K3bAudioEditorWidget::Private
{
public:
  Private()
    : allowOverlappingRanges(true),
      rangeSelectionEnabled(false),
      selectedRange(0),
      movedRange(0) {
  }

  QBrush selectedRangeBrush;

  bool allowOverlappingRanges;
  bool rangeSelectionEnabled;

  Range* selectedRange;
  Range* movedRange;
  K3b::Msf lastMovePosition;

  RangeList ranges;
};


K3bAudioEditorWidget::K3bAudioEditorWidget( QWidget* parent, const char* name )
  : QFrame( parent, name, Qt::WNoAutoErase ),
    m_maxMarkers(1),
    m_idCnt(1),
    m_mouseAt(true),
    m_draggedRange(0),
    m_draggedMarker(0)
{
  d = new Private;
  d->selectedRangeBrush = colorGroup().highlight();

  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
  setFrameStyle( StyledPanel|Sunken );
  setMouseTracking(true);
  setCursor( Qt::PointingHandCursor );

  m_margin = 5;

  m_toolTip = new ToolTip( this );
}


K3bAudioEditorWidget::~K3bAudioEditorWidget()
{
  d->ranges.setAutoDelete(true);
  m_markers.setAutoDelete(true);
  d->ranges.clear();
  m_markers.clear();

  delete d;
}


QSize K3bAudioEditorWidget::minimumSizeHint() const
{
  constPolish();
  // some fixed height minimum and enough space for a tickmark every minute
  // But never exceed 2/3 of the the screen width, otherwise it just looks ugly
  // FIXME: this is still bad for long sources and there might be 60 minutes sources!

  int maxWidth = QApplication::desktop()->width()*2/3;
  int wantedWidth = 2*m_margin + 2*frameWidth() + (m_length.totalFrames()/75/60 + 1) * fontMetrics().width( "000" );
  return QSize( QMIN( maxWidth, wantedWidth ), 
		2*m_margin + 12 + 6 /*12 for the tickmarks and 6 for the markers */ + fontMetrics().height() + 2*frameWidth() );
}


QSize K3bAudioEditorWidget::sizeHint() const
{
  return minimumSizeHint();
}


void K3bAudioEditorWidget::setLength( const K3b::Msf& length )
{
  m_length = length;
  // TODO: remove markers beyond length
  // TODO: shorten ranges if nesseccary
  update();
}


const K3b::Msf K3bAudioEditorWidget::length() const
{
  return m_length;
}


void K3bAudioEditorWidget::setSelectedRangeBrush( const QBrush& b )
{
  d->selectedRangeBrush = b;
}


const QBrush& K3bAudioEditorWidget::selectedRangeBrush() const
{
  return d->selectedRangeBrush;
}


void K3bAudioEditorWidget::setAllowOverlappingRanges( bool b )
{
  d->allowOverlappingRanges = b;
}


bool K3bAudioEditorWidget::allowOverlappingRanges() const
{
  return d->allowOverlappingRanges;
}


void K3bAudioEditorWidget::enableRangeSelection( bool b )
{
  d->rangeSelectionEnabled = b;
  repaint( false );
}


bool K3bAudioEditorWidget::rangeSelectedEnabled() const
{
  return d->selectedRange;
}


void K3bAudioEditorWidget::setSelectedRange( int id )
{
  setSelectedRange( getRange( id ) );
}


void K3bAudioEditorWidget::setSelectedRange( K3bAudioEditorWidget::Range* r )
{
  d->selectedRange = r;
  if( rangeSelectedEnabled() ) {
    repaint( false );
    emit selectedRangeChanged( d->selectedRange ? d->selectedRange->id : 0 );
  }
}


int K3bAudioEditorWidget::selectedRange() const
{
  if( d->selectedRange )
    return d->selectedRange->id;
  else
    return 0;
}


int K3bAudioEditorWidget::addRange( const K3b::Msf& start, const K3b::Msf& end, 
				    bool startFixed, bool endFixed,
				    const QString& toolTip,
				    const QBrush& brush )
{
  if( start > end || end > m_length-1 )
    return -1;

  Range* r = new Range( m_idCnt++, start, end, startFixed, endFixed, toolTip,
			brush.style() != QBrush::NoBrush ? brush : QBrush(colorGroup().background()) );
  d->ranges.inSort( r );

  // only update the changed range
  QRect rect = contentsRect();
  rect.setLeft( msfToPos( start ) );
  rect.setRight( msfToPos( end ) );
  update( rect );

  return r->id;
}


int K3bAudioEditorWidget::findRange( int pos ) const
{
  Range* r = findRange( QPoint( pos, 0 ) );
  if( r )
    return r->id;
  else
    return 0;
}


int K3bAudioEditorWidget::findRangeEdge( int pos, bool* end ) const
{
  Range* r = findRangeEdge( QPoint( pos, 0 ), end );
  if( r )
    return r->id;
  else
    return 0;
}


bool K3bAudioEditorWidget::modifyRange( int identifier, const K3b::Msf& start, const K3b::Msf& end )
{
  Range* range = getRange( identifier );
  if( range ) {
    if( start > end )
      return false;

    if( end > m_length )
      return false;

    range->start = start;
    range->end = end;

    if( !d->allowOverlappingRanges )
      fixupOverlappingRanges( range );
    
    repaint( false );

    return true;
  }
  else
    return false;
}


bool K3bAudioEditorWidget::removeRange( int identifier )
{
  if( Range* range = getRange( identifier ) ) {
    d->ranges.removeRef( range );

    emit rangeRemoved( identifier );

    // repaint only the part of the range
    QRect rect = contentsRect();
    rect.setLeft( msfToPos( range->start ) );
    rect.setRight( msfToPos( range->end ) );

    if( d->selectedRange == range )
      setSelectedRange( 0 );

    delete range;

    update( rect );

    return true;
  }
  else
    return false;
}


K3b::Msf K3bAudioEditorWidget::rangeStart( int identifier ) const
{
  if( Range* range = getRange( identifier ) )
    return range->start;
  else
    return 0;
}


K3b::Msf K3bAudioEditorWidget::rangeEnd( int identifier ) const
{
  if( Range* range = getRange( identifier ) )
    return range->end;
  else
    return 0;
}


QValueList<int> K3bAudioEditorWidget::allRanges() const
{
  QValueList<int> l;
  d->ranges.sort();
  for( QPtrListIterator<Range> it( d->ranges ); *it; ++it )
    l.append( (*it)->id );
  return l;
}


void K3bAudioEditorWidget::setMaxNumberOfMarkers( int i )
{
  m_maxMarkers = i;

  // remove last markers
  while( m_markers.count() > QMAX( 1, m_maxMarkers ) ) {
    removeMarker( m_markers.getLast()->id );
  }
}


int K3bAudioEditorWidget::addMarker( const K3b::Msf& pos, bool fixed, const QString& toolTip, const QColor& color )
{
  if( pos < m_length ) {
    Marker* m = new Marker( m_idCnt++, pos, fixed, color.isValid() ? color : colorGroup().foreground(), toolTip );
    m_markers.inSort( m );
    return m->id;
  }
  else
    return -1;
}


bool K3bAudioEditorWidget::removeMarker( int identifier )
{
  if( Marker* m = getMarker( identifier ) ) {
    m_markers.removeRef( m );

    emit markerRemoved( identifier );

    // TODO: in case a marker is bigger than one pixel this needs to be changed
    QRect rect = contentsRect();
    rect.setLeft( msfToPos( m->pos ) );
    rect.setRight( msfToPos( m->pos ) );
    delete m;

    update( rect );

    return true;
  }
  else
    return false;
}


bool K3bAudioEditorWidget::moveMarker( int identifier, const K3b::Msf& pos )
{
  if( pos < m_length )
    if( Marker* m = getMarker( identifier ) ) {
      QRect rect = contentsRect();
      rect.setLeft( QMIN( msfToPos( pos ), msfToPos( m->pos ) ) );
      rect.setRight( QMAX( msfToPos( pos ), msfToPos( m->pos ) ) );

      m->pos = pos;

      // TODO: in case a marker is bigger than one pixel this needs to be changed
      update( rect );

      return true;
    }

  return false;
}


void K3bAudioEditorWidget::drawContents( QPainter* p )
{
  // double buffering
  QPixmap pix( contentsRect().size() );
  pix.fill( colorGroup().base() );

  QPainter pixP;
  pixP.begin( &pix, this );

  QRect drawRect( contentsRect() );
  drawRect.setLeft( drawRect.left() + m_margin );
  drawRect.setRight( drawRect.right() - m_margin );

  // from minimumSizeHint()
//   int neededHeight = fontMetrics().height() + 12 + 6;

//   drawRect.setTop( drawRect.top() + (drawRect.height() - neededHeight)/2 );
//   drawRect.setHeight( neededHeight );

  drawRect.setTop( drawRect.top() + m_margin );
  drawRect.setBottom( drawRect.bottom() - m_margin );

  drawAll( &pixP, drawRect );

  pixP.end();

  QRect rect = p->clipRegion().boundingRect();
  QRect pixRect = rect;
  pixRect.moveBy( -1*frameWidth(), -1*frameWidth() );
  bitBlt( this, rect.topLeft(), &pix, pixRect );
}


void K3bAudioEditorWidget::drawAll( QPainter* p, const QRect& drawRect )
{
  // we simply draw the ranges one after the other.
  for( QPtrListIterator<Range> it( d->ranges ); *it; ++it )
    drawRange( p, drawRect, *it );

  // Hack to make sure the currently selected range is always on top
  if( d->selectedRange )
    drawRange( p, drawRect, d->selectedRange );
  
  for( QPtrListIterator<Marker> it( m_markers ); *it; ++it )
    drawMarker( p, drawRect, *it );


  // left vline
  p->drawLine( drawRect.left(), drawRect.top(), 
	       drawRect.left(), drawRect.bottom() );

  // timeline
  p->drawLine( drawRect.left(), drawRect.bottom(), 
	       drawRect.right(), drawRect.bottom() );

  // right vline
  p->drawLine( drawRect.right(), drawRect.top(), 
	       drawRect.right(), drawRect.bottom() );

  // draw minute markers every minute
  int minute = 1;
  int minuteStep = 1;
  int markerVPos = drawRect.bottom();
  int maxMarkerWidth = fontMetrics().width( QString::number(m_length.minutes()) );
  int minNeededSpace = maxMarkerWidth + 1;
  int x = 0;
  while( minute*60*75 < m_length ) {
    int newX = msfToPos( minute*60*75 );

    // only draw the mark if we have anough space
    if( newX - x >= minNeededSpace ) {
      p->drawLine( newX, markerVPos, newX, markerVPos-5 );
      QRect txtRect( newX-(maxMarkerWidth/2), 
		     markerVPos - 6 - fontMetrics().height(), 
		     maxMarkerWidth, 
		     fontMetrics().height() );
      p->drawText( txtRect, Qt::AlignCenter, QString::number(minute) );

      // FIXME: draw second markers if we have enough space

      x = newX;
    }
    else {
      minute -= minuteStep;
      if( minuteStep == 1 )
	minuteStep = 5;
      else
	minuteStep *= 2;
    }

    minute += minuteStep;
  }
}


void K3bAudioEditorWidget::drawRange( QPainter* p, const QRect& drawRect, K3bAudioEditorWidget::Range* r )
{
  p->save();

  int start = msfToPos( r->start );
  int end = msfToPos( r->end );

  if( rangeSelectedEnabled() && r == d->selectedRange )
    p->fillRect( start, drawRect.top() + 6 , end-start+1, drawRect.height() - 6, selectedRangeBrush() );
  else
    p->fillRect( start, drawRect.top() + 6 , end-start+1, drawRect.height() - 6, r->brush );

  p->drawRect( start, drawRect.top() + 6 , end-start+1, drawRect.height() - 6 );

  p->restore();
}


void K3bAudioEditorWidget::drawMarker( QPainter* p, const QRect& drawRect, K3bAudioEditorWidget::Marker* m )
{
  p->save();

  p->setPen( m->color );
  p->setBrush( m->color );

  int x = msfToPos( m->pos );
  p->drawLine( x, drawRect.bottom(), x, drawRect.top() );

  QPointArray points( 3 );
  points.setPoint( 0, x, drawRect.top() + 6 );
  points.setPoint( 1, x-3, drawRect.top() );
  points.setPoint( 2, x+3, drawRect.top() );
  p->drawPolygon( points );

  p->restore();
}


void K3bAudioEditorWidget::fixupOverlappingRanges( Range* r )
{
  // copy the list to avoid problems with the iterator
  RangeList ranges( d->ranges );
  for( QPtrListIterator<Range> it( ranges ); *it; ++it ) {
    Range* range = *it;
    if( range != r ) {
      // remove the range if it is covered completely
      if( range->start >= r->start &&
	  range->end <= r->end ) {
	d->ranges.removeRef( range );
	emit rangeRemoved( range->id );
	if( d->selectedRange == range )
	  setSelectedRange( 0 );
	delete range;
      }
      // split the range if it contains r completely
      else if( r->start >= range->start &&
	       r->end <= range->end ) {
	// create a new range that spans the part after r
	addRange( r->end+1, range->end, 
		  range->startFixed, range->endFixed,
		  range->toolTip,
		  range->brush );

	// modify the old range to only span the part before r
	range->end = r->start-1;
	emit rangeChanged( range->id, range->start, range->end );
      }
      else if( range->start >= r->start && range->start <= r->end ) {
	range->start = r->end+1;
	emit rangeChanged( range->id, range->start, range->end );
      }
      else if( range->end >= r->start && range->end <= r->end ) {
	range->end = r->start-1;
	emit rangeChanged( range->id, range->start, range->end );
      }
    }
  }
}


void K3bAudioEditorWidget::mousePressEvent( QMouseEvent* e )
{
  m_draggedRange = 0;
  m_draggedMarker = 0;

  bool end;
  if( Range* r = findRangeEdge( e->pos(), &end ) ) {
    m_draggedRange = r;
    m_draggingRangeEnd = end;
    setSelectedRange( r );
  }
  else {
    Range* r = findRange( e->pos() );
    d->movedRange = r;
    d->lastMovePosition = posToMsf( e->pos().x() );
    setSelectedRange( r );
    m_draggedMarker = findMarker( e->pos() );
  }

  QFrame::mousePressEvent(e);
}


void K3bAudioEditorWidget::mouseReleaseEvent( QMouseEvent* e )
{
  if( !d->allowOverlappingRanges ) {
    //
    // modify and even delete ranges that we touched
    //
    if( m_draggedRange ) {
      fixupOverlappingRanges( m_draggedRange );
      repaint( false );
    }
    else if( d->movedRange ) {
      fixupOverlappingRanges( d->movedRange );
      repaint( false );
    }
  }

  m_draggedRange = 0;
  m_draggedMarker = 0;
  d->movedRange = 0;

  QFrame::mouseReleaseEvent(e);
}


void K3bAudioEditorWidget::mouseDoubleClickEvent( QMouseEvent* e )
{
  QFrame::mouseDoubleClickEvent(e);
}


void K3bAudioEditorWidget::mouseMoveEvent( QMouseEvent* e )
{
  if( m_mouseAt )
    emit mouseAt( posToMsf( e->pos().x() ) );
  
  if( e->state() & Qt::LeftButton ) {
    if( m_draggedRange ) {
      // determine the position the range's end was dragged to and its other end
      K3b::Msf msfPos = QMAX( 0, QMIN( posToMsf( e->pos().x() ), m_length-1 ) );
      K3b::Msf otherEnd = ( m_draggingRangeEnd ? m_draggedRange->start : m_draggedRange->end );

      // move it to the new pos
      if( m_draggingRangeEnd )
	m_draggedRange->end = msfPos;
      else
	m_draggedRange->start = msfPos;

      // if we pass the other end switch them
      if( m_draggedRange->start > m_draggedRange->end ) {
	K3b::Msf buf = m_draggedRange->start;
	m_draggedRange->start = m_draggedRange->end;
	m_draggedRange->end = buf;
	m_draggingRangeEnd = !m_draggingRangeEnd;
      }

      emit rangeChanged( m_draggedRange->id, m_draggedRange->start, m_draggedRange->end );

      repaint( false );
    }
    else if( m_draggedMarker ) {
      m_draggedMarker->pos = posToMsf( e->pos().x() );
      emit markerMoved( m_draggedMarker->id, m_draggedMarker->pos );

      repaint( false );
    }
    else if( d->movedRange ) {
      int diff = posToMsf( e->pos().x() ).lba() - d->lastMovePosition.lba();
      if( d->movedRange->end + diff >= m_length )
	diff = m_length.lba() - d->movedRange->end.lba() - 1;
      else if( d->movedRange->start - diff < 0 )
	diff = -1 * d->movedRange->start.lba();
      d->movedRange->start += diff;
      d->movedRange->end += diff;

//       if( !d->allowOverlappingRanges )
// 	fixupOverlappingRanges( d->movedRange );

      d->lastMovePosition = posToMsf( e->pos().x() );

      emit rangeChanged( d->movedRange->id, d->movedRange->start, d->movedRange->end );

      repaint( false );
    }
  }
  else if( findRangeEdge( e->pos() ) || findMarker( e->pos() ) )
    setCursor( Qt::SizeHorCursor );
  else
    setCursor( Qt::PointingHandCursor );

  QFrame::mouseMoveEvent(e);
}


K3bAudioEditorWidget::Range* K3bAudioEditorWidget::getRange( int i ) const
{
  for( QPtrListIterator<Range> it( d->ranges ); *it; ++it )
    if( (*it)->id == i )
      return *it;

  return 0;
}


K3bAudioEditorWidget::Range* K3bAudioEditorWidget::findRange( const QPoint& p ) const
{
  // TODO: binary search; maybe store start and end positions in sorted lists for quick searching
  // this might be a stupid approach but we do not have many ranges anyway
  for( QPtrListIterator<Range> it( d->ranges ); *it; ++it ) {
    Range* range = *it;
    int start = msfToPos( range->start );
    int end = msfToPos( range->end );

    if( p.x() >= start && p.x() <= end ) {
      return range;
    }
  }
  return 0;
}


K3bAudioEditorWidget::Range* K3bAudioEditorWidget::findRangeEdge( const QPoint& p, bool* isEnd ) const
{
  // TODO: binary search
  // this might be a stupid approach but we do not have many ranges anyway
  for( QPtrListIterator<Range> it( d->ranges ); *it; ++it ) {
    Range* range = *it;
    int start = msfToPos( range->start );
    int end = msfToPos( range->end );

    //
    // In case two ranges meet at one point moving the mouse cursor deeper into one
    // range allows for grabbing that end
    //

    if( p.x() - 3 <= start && p.x() >= start && !range->startFixed ) {
      if( isEnd )
	*isEnd = false;
      return range;
    }
    else if( p.x() <= end && p.x() + 3 >= end && !range->endFixed ) {
      if( isEnd )
	*isEnd = true;
      return range;
    }
  }
  return 0;
}


K3bAudioEditorWidget::Marker* K3bAudioEditorWidget::getMarker( int i ) const
{
  for( QPtrListIterator<Marker> it( m_markers ); *it; ++it )
    if( (*it)->id == i )
      return *it;

  return 0;
}


K3bAudioEditorWidget::Marker* K3bAudioEditorWidget::findMarker( const QPoint& p ) const
{
  // TODO: binary search
  for( QPtrListIterator<Marker> it( m_markers ); *it; ++it ) {
    Marker* marker = *it;
    int start = msfToPos( marker->pos );

    if( p.x() - 1 <= start && p.x() + 1 >= start && !marker->fixed )
      return marker;
  }

  return 0;
}


// p is in widget coordinates
K3b::Msf K3bAudioEditorWidget::posToMsf( int p ) const
{
  int w = contentsRect().width() - 2*m_margin;
  int x = QMIN( p-frameWidth()-m_margin, w );
  return ( (int)((double)(m_length.lba()-1) / (double)w * (double)x) );
}


// returns widget coordinates
int K3bAudioEditorWidget::msfToPos( const K3b::Msf& msf ) const
{
  int w = contentsRect().width() - 2*m_margin;
  int pos = (int)((double)w / (double)(m_length.lba()-1) * (double)msf.lba());
  return frameWidth() + m_margin + QMIN( pos, w-1 );
}


#include "k3baudioeditorwidget.moc"
