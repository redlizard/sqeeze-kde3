/* SuSE KWin window decoration
  Copyright (C) 2005 Gerd Fleischer <gerdfleischer@web.de>
  Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA.
*/

#ifndef SUSE2BUTTON_H
#define SUSE2BUTTON_H

#include <qbutton.h>
#include <qpixmap.h>

#include <kcommondecoration.h>

#include "SUSE2.h"

class QTimer;

namespace KWinSUSE2 {

static const uint TIMERINTERVAL = 50; // msec
static const uint ANIMATIONSTEPS = 4;

class SUSE2Client;

class SUSE2Button : public KCommonDecorationButton
{
    Q_OBJECT
public:
    SUSE2Button(ButtonType type, SUSE2Client *parent, const char *name);
    ~SUSE2Button();

    void reset(unsigned long changed);
    SUSE2Client *client() { return m_client; }

protected slots:
    void animate();

private:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void drawButton(QPainter *painter);
    void drawPlastikBtn(QPainter *painter);
    void drawLipstikBtn(QPainter *painter);
    void renderBtnContour(QPainter *p, const QRect &r);
    void renderBtnSurface(QPainter *p, const QRect &r);
    void renderPixel(QPainter *p, const QPoint &pos, const int alpha, const QColor &color,
                     const int pixelPos, const int active);
    void renderGradient(QPainter *p, const QRect &r, const QColor &c1, const QColor &c2,
                        const int gradient, const int active);


private:
    SUSE2Client *m_client;

    ButtonIcon m_iconType;
    bool hover;

    QTimer *animTmr;
    uint animProgress;

    QPixmap *lipstikCache[8+3*(ANIMATIONSTEPS+1)][2];
};

/**
 * This class creates bitmaps which can be used as icons on buttons. The icons
 * are "hardcoded".
 * Over the previous "Gimp->xpm->QImage->recolor->SmoothScale->QPixmap" solution
 * it has the important advantage that icons are more scalable and at the same
 * time sharp and not blurred.
 */
class IconEngine
{
    public:
        static QBitmap icon(ButtonIcon icon, int size);

    private:
        enum Object {
            HorizontalLine,
            VerticalLine,
            DiagonalLine,
            CrossDiagonalLine
        };

        static void drawObject(QPainter &p, Object object, int x, int y, int length, int lineWidth);
};

} // namespace KWinSUSE2

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
