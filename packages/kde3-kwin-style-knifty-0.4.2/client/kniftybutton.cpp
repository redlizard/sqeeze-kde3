/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qbutton.h>

#include "kniftybutton.h"
#include "kniftyclient.h"
#include "misc.h"
#include "shadow.h"

using namespace KNifty;

// button bitmaps
static unsigned char close_bits[] =
{
   0x42, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0x42};
// static unsigned char maximize_bits[] = {
//    0x00, 0x18, 0x3c, 0x7e, 0xff, 0x00, 0x00, 0x00
// };
// static unsigned char maximize_bits[] = {
//    0x7e, 0xff, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff
// };
static unsigned char maximize_bits[] = {
   0xff, 0xff, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff};
// static unsigned char restore_bits[] = {
//    0x02, 0x06, 0x0e, 0x1e, 0x3e, 0x7e, 0xfc, 0x00
// };
static unsigned char restore_bits[] = {
   0xfc, 0x84, 0xbf, 0xbf, 0xa1, 0xa1, 0xe1, 0x3f};
// static unsigned char minimize_bits[] = {
//    0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00
// };
static unsigned char minimize_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff};
static unsigned char empty_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char help_bits[] = {
   0x7e, 0xe7, 0xc0, 0x70, 0x18, 0x00, 0x18, 0x18};
static unsigned char sticky_bits[] = {
   0x18, 0x7e, 0x66, 0xc3, 0xc3, 0x66, 0x7e, 0x18};
static unsigned char unsticky_bits[] = {
   0x18, 0x7e, 0x66, 0xdb, 0xdb, 0x66, 0x7e, 0x18};

static const int DECOSIZE   = 8;

KniftyButton::KniftyButton(KniftyClient *parent, const char *name, const QString& tip, ButtonType type, int size, int btns)
    : QButton(parent->widget(), name),
    m_client(parent),
    m_lastMouse(0),
    m_realizeButtons(btns),
    m_size(size),
    m_type(type),
    hover(false),
    isSticky(false),
    isMaximized(false)
{
    setBackgroundMode(NoBackground);

    // the app icon should fit the button...
    if(m_size < 10) { m_size = 10; }

    setFixedSize(m_size, m_size);
    QToolTip::add(this, tip);
}

KniftyButton::~KniftyButton()
{
}

QSize KniftyButton::sizeHint() const
{
    return QSize(m_size, m_size);
}

void KniftyButton::enterEvent(QEvent *e)
{
    hover = true;
    repaint(false);

    QButton::enterEvent(e);
}

void KniftyButton::leaveEvent(QEvent *e)
{
    hover = false;
    repaint(false);

    QButton::leaveEvent(e);
}

void KniftyButton::mousePressEvent(QMouseEvent* e)
{
    m_lastMouse = e->button();
    // pass on event after changing button to LeftButton
    QMouseEvent me(e->type(), e->pos(), e->globalPos(),
                   (e->button()&m_realizeButtons)?LeftButton:NoButton, e->state());
    QButton::mousePressEvent(&me);
}

void KniftyButton::mouseReleaseEvent(QMouseEvent* e)
{
    m_lastMouse = e->button();
    // pass on event after changing button to LeftButton
    QMouseEvent me(e->type(), e->pos(), e->globalPos(),
                   (e->button()&m_realizeButtons)?LeftButton:NoButton, e->state());
    QButton::mouseReleaseEvent(&me);
}

int KniftyButton::inverseBwColor(QColor color)
{
    return ((color.red()*0.299 + color.green()*0.587 + color.blue()*0.114) < 128 ) * 255;
}

void KniftyButton::drawButton(QPainter *painter)
{
    if (!KniftyHandler::initialized())
        return;

    bool active = m_client->isActive();
    QPixmap backgroundTile = m_client->getTitleBarTile(active);

    QColor titleBorder3 = KniftyHandler::useTitleColor() ?
                          m_client->options()->colorGroup(KDecoration::ColorTitleBar, active).background()
                          : colorGroup().background();
    QColor titleBorder1 = titleBorder3.dark(155); // dark

    QPixmap buffer;
    buffer.resize(width(), height());
    QPainter bufferPainter(&buffer);

    bufferPainter.drawTiledPixmap(0, 0, width(), width(), backgroundTile);
    if (!KniftyHandler::useHighContrastHoveredButtons())
    {
        if (hover)
        {
            if (isDown())
            {
            bufferPainter.setPen(titleBorder3.dark(150));
            }
            else {
            bufferPainter.setPen(titleBorder3.dark(120));
            }
            bufferPainter.drawLine(1, 0, width()-2, 0);
            bufferPainter.drawLine(0, 1, 0, height()-2);
            bufferPainter.drawLine(1, height()-1, width()-2, height()-1);
            bufferPainter.drawLine(width()-1, 1, width()-1, height()-2);
        }
    }

    if (m_type == ButtonMenu)
    {
        QPixmap menuIcon = m_client->icon().pixmap(QIconSet::Small, QIconSet::Normal);
        if (width() < menuIcon.width() || height() < menuIcon.height() ) {
            menuIcon.convertFromImage( menuIcon.convertToImage().smoothScale(width()-2, height()-2));
        }
        bufferPainter.drawPixmap((width()-menuIcon.width())/2, (height()-menuIcon.height())/2, menuIcon);
    }
    else
    {
        QBitmap deco;

        switch (m_type) {
        case ButtonClose:
            deco = QBitmap(DECOSIZE, DECOSIZE, close_bits, true);
            break;
        case ButtonMax:
            deco = QBitmap(DECOSIZE, DECOSIZE,
                           isMaximized ? restore_bits : maximize_bits,
                           true);
            break;
        case ButtonMin:
            deco = QBitmap(DECOSIZE, DECOSIZE, minimize_bits, true);
            break;
        case ButtonHelp:
            deco = QBitmap(DECOSIZE, DECOSIZE, help_bits, true);
            break;
        case ButtonOnAllDesktops:
            deco = QBitmap(DECOSIZE, DECOSIZE,
                           isSticky ? unsticky_bits : sticky_bits,
                           true);
            break;
        default:
            deco = QBitmap(DECOSIZE, DECOSIZE, empty_bits, true);
        }
        deco.setMask(deco);

        int dX = (width()-DECOSIZE)/2;
        int dY = (height()-DECOSIZE)/2;

        if (isDown())
        {
            dY++;
            if (hover)
            {
                bufferPainter.setPen(titleBorder1.dark(120));
            }
            else
            {
                bufferPainter.setPen(titleBorder1);
            }
        }
        else
        {
            bufferPainter.setPen(titleBorder3.light(120) );
            bufferPainter.drawPixmap((width()-DECOSIZE)/2, (height()-DECOSIZE)/2+1, deco);
            if(hover)
            {
                if (!KniftyHandler::useHighContrastHoveredButtons())
                    bufferPainter.setPen(titleBorder1.light(120));
                else
                {
                    int bw = inverseBwColor(titleBorder3);
                    bufferPainter.setPen(QColor(bw,bw,bw));
                }
            }
            else
            {
                bufferPainter.setPen(titleBorder1);
            }
        }

        bufferPainter.drawPixmap(dX, dY, deco);
    }

    bufferPainter.end();
    painter->drawPixmap(0, 0, buffer);
}
