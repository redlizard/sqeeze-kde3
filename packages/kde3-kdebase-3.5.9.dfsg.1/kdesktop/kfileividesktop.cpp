/* This file is proposed to be part of the KDE base.
 * Copyright (C) 2003 Laur Ivan <laurivan@eircom.net>
 *
 * Many thanks to:
 *  - Bernardo Hung <deciare@gta.igs.net> for the enhanced shadow
 *  algorithm (currently used)
 *  - Tim Jansen <tim@tjansen.de> for the API updates and fixes.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>

#include <qcolor.h>
#include <qpalette.h>
#include <qstring.h>
#include <qpainter.h>
#include <qstyle.h>
#include <kwordwrap.h>
#include <kiconview.h>
#include <kdebug.h>

#include <kshadowengine.h>
#include "kdesktopshadowsettings.h"
#include "kfileividesktop.h"

//#define DEBUG

/* Changelog:
 */

KFileIVIDesktop::KFileIVIDesktop(KonqIconViewWidget *iconview, KFileItem* fileitem,
				 int size, KShadowEngine *shadow) :
  KFileIVI(iconview, fileitem, size),
  m_selectedImage(0L),
  m_normalImage(0L),
  _selectedUID(0),
  _normalUID(0)
{
  m_shadow = shadow;
  oldText = "";

  calcRect( text() ); // recalculate rect including shadow
}

KFileIVIDesktop::~KFileIVIDesktop()
{
  delete m_selectedImage;
  delete m_normalImage;
}

void KFileIVIDesktop::calcRect( const QString& _text )
{
    KIconViewItem::calcRect( _text );

    if ( !iconView() || !m_shadow ||
         !wordWrap() || !( static_cast<KDesktopShadowSettings *>
             ( m_shadow->shadowSettings() ) )->isEnabled() )
        return;

    int spread = shadowThickness();
    QRect itemTextRect = textRect();
    QRect itemRect = rect();

    itemTextRect.setBottom( itemTextRect.bottom() + spread );
    itemTextRect.setRight( itemTextRect.right() + spread );
    itemRect.setBottom( itemRect.bottom() + spread );
    itemRect.setRight( itemRect.right() + spread );

    setTextRect( itemTextRect );
    setItemRect( itemRect );
}

void KFileIVIDesktop::paintItem( QPainter *p, const QColorGroup &cg)
{
  QColorGroup colors = updateColors(cg);

  QIconView* view = iconView();
  Q_ASSERT( view );

  if ( !view )
    return;

  if ( !wordWrap() )
    return;

  p->save();

  // draw the pixmap as in KIconViewItem::paintItem(...)
  paintPixmap(p, colors);

  //
  // Paint the text as shadowed if the shadow is available
  //
  if (m_shadow != 0L && (static_cast<KDesktopShadowSettings *> (m_shadow->shadowSettings()))->isEnabled())
    drawShadowedText(p, colors);
  else {
    paintFontUpdate(p);
    paintText(p, colors);
  }

  p->restore();

  paintOverlay(p);
}

bool KFileIVIDesktop::shouldUpdateShadow(bool selected)
{
  unsigned long uid = (static_cast<KDesktopShadowSettings *> (m_shadow->shadowSettings()))->UID();
  QString wrapped = wordWrap()->wrappedString();

  if (wrapped != oldText){
    oldText = wrapped;
    _selectedUID = _normalUID = 0;
  }

  if (selected == true)
    return (uid != _selectedUID);
  else
    return (uid != _normalUID);

  return false;
}

void KFileIVIDesktop::paintFocus( QPainter *p, const QColorGroup &cg )
{
    if ( !iconView() )
        return;

    if ( !m_shadow || !wordWrap() ||
         !( static_cast<KDesktopShadowSettings *>
             ( m_shadow->shadowSettings() ) )->isEnabled() ) {
        QIconViewItem::paintFocus( p, cg );
        return;
    }

    int spread = shadowThickness();

    iconView()->style().drawPrimitive( QStyle::PE_FocusRect, p,
        QRect( textRect( false ).x(), textRect( false ).y(),
               textRect( false ).width() - spread,
               textRect( false ).height() - spread + 1 ),
        cg,
        isSelected() ? QStyle::Style_FocusAtBorder : QStyle::Style_Default,
        QStyleOption( isSelected() ? cg.highlight() : cg.base() ) );

    if ( this != iconView()->currentItem() ) {
        iconView()->style().drawPrimitive( QStyle::PE_FocusRect, p,
            QRect( pixmapRect( false ).x(), pixmapRect( false ).y(),
            pixmapRect( false ).width(), pixmapRect( false ).height() ),
            cg, QStyle::Style_Default, QStyleOption( cg.base() ) );
    }
}


void KFileIVIDesktop::drawShadowedText( QPainter *p, const QColorGroup &cg )
{
  int textX = textRect( FALSE ).x() + 2;
  int textY = textRect( FALSE ).y();
  int align = ((KIconView *) iconView())->itemTextPos() == QIconView::Bottom
    ? AlignHCenter : AlignAuto;
  bool rebuild = shouldUpdateShadow(isSelected());

  KDesktopShadowSettings *settings = (KDesktopShadowSettings *) (m_shadow->shadowSettings());

  unsigned long uid = settings->UID();

  p->setFont(iconView()->font());
  paintFontUpdate(p);
  QColor shadow;
  QColor text;
  int spread = shadowThickness();

  if ( isSelected() && settings->selectionType() != KShadowSettings::InverseVideoOnSelection ) {
    // select using a filled rect
    text = cg.highlightedText();
    QRect rect = textRect( false );
    rect.setRight( rect.right() - spread );
    rect.setBottom( rect.bottom() - spread + 1 );
    p->fillRect( rect, cg.highlight() );
  }
  else {
    // use shadow
    if ( isSelected() ) {
      // inverse text and shadow colors
      shadow = settings->textColor();
      text = settings->bgColor();
      if ( rebuild ) {
        setSelectedImage( buildShadow( p, align, shadow ) );
        _selectedUID = uid;
      }
    }
    else {
      text = settings->textColor();
      shadow = ( settings->bgColor().isValid() ) ? settings->bgColor() :
               ( qGray( text.rgb() ) > 127 ) ? black : white;
      if (rebuild) {
        setNormalImage(buildShadow(p, align, shadow));
        _normalUID = uid;
      }
    }

    // draw the shadow
    int shadowX = textX - spread + settings->offsetX();
    int shadowY = textY - spread + settings->offsetY();

    p->drawImage(shadowX, shadowY,
      (isSelected()) ? *selectedImage() : *normalImage(),
      0, 0, -1, -1, DITHER_FLAGS);
  }

  // draw the text
  p->setPen(text);
  wordWrap()->drawText( p, textX, textY, align | KWordWrap::Truncate );
}


QImage *KFileIVIDesktop::buildShadow( QPainter *p, const int align,
                                      QColor &shadowColor )
{
  QPainter pixPainter;
  int spread = shadowThickness();

  QPixmap textPixmap(textRect( FALSE ).width() + spread * 2 + 2,
    textRect( FALSE ).height() + spread * 2 + 2);

  textPixmap.fill(QColor(0,0,0));
  textPixmap.setMask( textPixmap.createHeuristicMask(TRUE) );

  pixPainter.begin(&textPixmap);
  pixPainter.setPen(white);    // get the pen from the root painter
  pixPainter.setFont(p->font()); // get the font from the root painter
  wordWrap()->drawText( &pixPainter, spread, spread, align | KWordWrap::Truncate );
  pixPainter.end();

  return new QImage(m_shadow->makeShadow(textPixmap, shadowColor));
}

int KFileIVIDesktop::shadowThickness() const
{
  return ( ( m_shadow->shadowSettings()->thickness() + 1 ) >> 1 ) + 1;
}

