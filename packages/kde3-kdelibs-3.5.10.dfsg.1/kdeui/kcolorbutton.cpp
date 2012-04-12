/*  This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 1999 Cristian Tibirna (ctibirna@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <qpainter.h>
#include <qdrawutil.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qstyle.h>
#include <kglobalsettings.h>
#include <kstdaccel.h>
#include "kcolordialog.h"
#include "kcolorbutton.h"
#include "kcolordrag.h"

class KColorButton::KColorButtonPrivate
{
public:
    bool m_bdefaultColor;
    QColor m_defaultColor;
};

KColorButton::KColorButton( QWidget *parent, const char *name )
  : QPushButton( parent, name )
{
  d = new KColorButtonPrivate;
  d->m_bdefaultColor = false;
  d->m_defaultColor = QColor();
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, SIGNAL(clicked()), this, SLOT(chooseColor()));
}

KColorButton::KColorButton( const QColor &c, QWidget *parent,
			    const char *name )
  : QPushButton( parent, name ), col(c)
{
  d = new KColorButtonPrivate;
  d->m_bdefaultColor = false;
  d->m_defaultColor = QColor();
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, SIGNAL(clicked()), this, SLOT(chooseColor()));
}

KColorButton::KColorButton( const QColor &c, const QColor &defaultColor, QWidget *parent,
			    const char *name )
  : QPushButton( parent, name ), col(c)
{
  d = new KColorButtonPrivate;
  d->m_bdefaultColor = true;
  d->m_defaultColor = defaultColor;
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, SIGNAL(clicked()), this, SLOT(chooseColor()));
}

KColorButton::~KColorButton()
{
  delete d;
}

void KColorButton::setColor( const QColor &c )
{
  if ( col != c ) {
    col = c;
    repaint( false );
    emit changed( col );
  }
}

QColor KColorButton::defaultColor() const
{
  return d->m_defaultColor;
}

void KColorButton::setDefaultColor( const QColor &c )
{
  d->m_bdefaultColor = c.isValid();
  d->m_defaultColor = c;
}


void KColorButton::drawButtonLabel( QPainter *painter )
{
  int x, y, w, h;
  QRect r = style().subRect( QStyle::SR_PushButtonContents, this );
  r.rect(&x, &y, &w, &h);

  int margin = style().pixelMetric( QStyle::PM_ButtonMargin, this );
  x += margin;
  y += margin;
  w -= 2*margin;
  h -= 2*margin;

  if (isOn() || isDown()) {
    x += style().pixelMetric( QStyle::PM_ButtonShiftHorizontal, this );
    y += style().pixelMetric( QStyle::PM_ButtonShiftVertical, this );
  }

  QColor fillCol = isEnabled() ? col : backgroundColor();
  qDrawShadePanel( painter, x, y, w, h, colorGroup(), true, 1, NULL);
  if ( fillCol.isValid() )
    painter->fillRect( x+1, y+1, w-2, h-2, fillCol );

  if ( hasFocus() ) {
    QRect focusRect = style().subRect( QStyle::SR_PushButtonFocusRect, this );
    style().drawPrimitive( QStyle::PE_FocusRect, painter, focusRect, colorGroup() );
  }
}

QSize KColorButton::sizeHint() const
{
  return style().sizeFromContents(QStyle::CT_PushButton, this, QSize(40, 15)).
	  	expandedTo(QApplication::globalStrut());
}

void KColorButton::dragEnterEvent( QDragEnterEvent *event)
{
  event->accept( KColorDrag::canDecode( event) && isEnabled());
}

void KColorButton::dropEvent( QDropEvent *event)
{
  QColor c;
  if( KColorDrag::decode( event, c)) {
    setColor(c);
  }
}

void KColorButton::keyPressEvent( QKeyEvent *e )
{
  KKey key( e );

  if ( KStdAccel::copy().contains( key ) ) {
    QMimeSource* mime = new KColorDrag( color() );
    QApplication::clipboard()->setData( mime, QClipboard::Clipboard );
  }
  else if ( KStdAccel::paste().contains( key ) ) {
    QColor color;
    KColorDrag::decode( QApplication::clipboard()->data( QClipboard::Clipboard ), color );
    setColor( color );
  }
  else
    QPushButton::keyPressEvent( e );
}

void KColorButton::mousePressEvent( QMouseEvent *e)
{
  mPos = e->pos();
  QPushButton::mousePressEvent(e);
}

void KColorButton::mouseMoveEvent( QMouseEvent *e)
{
  if( (e->state() & LeftButton) &&
    (e->pos()-mPos).manhattanLength() > KGlobalSettings::dndEventDelay() )
  {
    // Drag color object
    KColorDrag *dg = new KColorDrag( color(), this);
    dg->dragCopy();
    setDown(false);
  }
}

void KColorButton::chooseColor()
{
  QColor c = color();
  if ( d->m_bdefaultColor )
  {
      if( KColorDialog::getColor( c, d->m_defaultColor, this ) != QDialog::Rejected ) {
          setColor( c );
      }
  }
  else
  {
      if( KColorDialog::getColor( c, this ) != QDialog::Rejected ) {
          setColor( c );
      }
  }
}

void KColorButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcolorbutton.moc"
