/* SUSE KWin window decoration
  Copyright (C) 2005 Gerd Fleischer <gerdfleischer@web.de>
  Copyright (C) 2005 Adrian Schroeter <adrian@suse.de>

  based on the window decoration "Plastik" and "Web":
  Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>
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

#include <kdebug.h>

#include <math.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kdecoration.h>

#include "misc.h"
#include "shadow.h"
#include "SUSE2client.h"
#include "SUSE2button.h"
#include "SUSE2button.moc"

#define PLASTIK_FLAT 0
#define LIPSTIK_FLAT 1
#define PLASTIK_3D 2
#define LIPSTIK_3D 3
#define LIPSTIK_BRIGHT 4

namespace KWinSUSE2
{

SUSE2Button::SUSE2Button(ButtonType type, SUSE2Client *parent, const char *name)
    : KCommonDecorationButton(type, parent, name),
    m_client(parent),
    m_iconType(NumButtonIcons),
    hover(false)
{
    memset(lipstikCache, 0, sizeof(QPixmap*)*(8 + 3*(ANIMATIONSTEPS+1))*2); // set elements to 0

    setBackgroundMode(NoBackground);

    animTmr = new QTimer(this);
    connect(animTmr, SIGNAL(timeout() ), this, SLOT(animate() ) );
    animProgress = 0;
}

SUSE2Button::~SUSE2Button()
{
    for (uint t = 0; t < (8 + 3*ANIMATIONSTEPS+1); ++t)
        for (int i = 0; i < 2; ++i)
            delete lipstikCache[t][i];
}

void SUSE2Button::reset(unsigned long changed)
{
    if (changed&DecorationReset || changed&ManualReset || changed&SizeChange || changed&StateChange) {
        switch (type()) {
            case CloseButton:
                m_iconType = CloseIcon;
                break;
            case HelpButton:
                m_iconType = HelpIcon;
                break;
            case MinButton:
                m_iconType = MinIcon;
                break;
            case MaxButton:
                if (isOn()) {
                    m_iconType = MaxRestoreIcon;
                } else {
                    m_iconType = MaxIcon;
                }
                break;
            case OnAllDesktopsButton:
                if (isOn()) {
                    m_iconType = NotOnAllDesktopsIcon;
                } else {
                    m_iconType = OnAllDesktopsIcon;
                }
                break;
            case ShadeButton:
                if (isOn()) {
                    m_iconType = UnShadeIcon;
                } else {
                    m_iconType = ShadeIcon;
                }
                break;
            case AboveButton:
                if (isOn()) {
                    m_iconType = NoKeepAboveIcon;
                } else {
                    m_iconType = KeepAboveIcon;
                }
                break;
            case BelowButton:
                if (isOn()) {
                    m_iconType = NoKeepBelowIcon;
                } else {
                    m_iconType = KeepBelowIcon;
                }
                break;
            default:
                m_iconType = NumButtonIcons;
                break;
        }

        for (uint t = 0; t < (8 + 3*(ANIMATIONSTEPS+1)); ++t) {
            for (int i = 0; i < 2; ++i) {
                if (lipstikCache[t][i]) {
                    delete lipstikCache[t][i];
                    lipstikCache[t][i] = 0;
               }
           }
        }

        this->update();
    }
}

void SUSE2Button::animate()
{
    animTmr->stop();

    if(hover) {
        if(animProgress < ANIMATIONSTEPS) {
            if (Handler()->animateButtons() ) {
                animProgress++;
            } else {
                animProgress = ANIMATIONSTEPS;
            }
            animTmr->start(TIMERINTERVAL, true); // single-shot
        }
    } else {
        if(animProgress > 0) {
            if (Handler()->animateButtons() ) {
                animProgress--;
            } else {
                animProgress = 0;
            }
            animTmr->start(TIMERINTERVAL, true); // single-shot
        }
    }

    repaint(false);
}

void SUSE2Button::enterEvent(QEvent *e)
{
    QButton::enterEvent(e);

    hover = true;
    animate();
//     repaint(false);
}

void SUSE2Button::leaveEvent(QEvent *e)
{
    QButton::leaveEvent(e);

    hover = false;
    animate();
//     repaint(false);
}

void SUSE2Button::drawButton(QPainter *painter)
{
    int type = Handler()->buttonType();

    switch (type) {
        case PLASTIK_FLAT:
        case LIPSTIK_FLAT:
        case LIPSTIK_BRIGHT:
            drawPlastikBtn(painter);
            break;
        case PLASTIK_3D:
        case LIPSTIK_3D:
            drawLipstikBtn(painter);
            break;
        default:
            drawPlastikBtn(painter);
            break;
    }
}

void SUSE2Button::drawPlastikBtn(QPainter *painter)
{
    QRect r(0, 0, width(), height());

    bool active = m_client->isActive();
    bool down = isDown();
    KPixmap backgroundTile = m_client->getTitleBarTile(active);
    KPixmap tempKPixmap;

    QColor highlightColor;
    if(type() == CloseButton) {
        highlightColor = QColor(255,64,0);
    } else {
        highlightColor = Qt::white;
    }

    QColor contourTop = alphaBlendColors(Handler()->getColor(TitleGradientFrom, active),
            Qt::black, 220);
    QColor contourBottom = alphaBlendColors(Handler()->getColor(TitleGradientTo, active),
            Qt::black, 220);
    QColor surfaceTop = alphaBlendColors(Handler()->getColor(TitleGradientFrom, active),
            Qt::white, 220);
    QColor surfaceBottom = alphaBlendColors(Handler()->getColor(TitleGradientTo, active),
            Qt::white, 220);

    if (type() == CloseButton && active && Handler()->redCloseButton()) {
        contourTop = QColor(170,70,70);
        contourBottom = QColor(120,50,50);
        surfaceTop = QColor(255,70,70);
        surfaceBottom = QColor(170,50,50);
        highlightColor = QColor(140,10,10);
    } else if (Handler()->customColors()) {
        contourTop = alphaBlendColors(Handler()->getColor(BtnBg, active), Qt::black, 185);
        contourBottom = alphaBlendColors(Handler()->getColor(BtnBg, active), Qt::black, 135);
        surfaceTop = Handler()->getColor(BtnBg, active);
        surfaceBottom = alphaBlendColors(Handler()->getColor(BtnBg, active), Qt::black, 185);
    }


    int highlightAlpha = static_cast<int>(255-((60/static_cast<double>(ANIMATIONSTEPS))*
                                          static_cast<double>(animProgress) ) );
    contourTop = alphaBlendColors(contourTop, highlightColor, highlightAlpha );
    contourBottom = alphaBlendColors(contourBottom, highlightColor, highlightAlpha);
    surfaceTop = alphaBlendColors(surfaceTop, highlightColor, highlightAlpha);
    surfaceBottom = alphaBlendColors(surfaceBottom, highlightColor, highlightAlpha);

    if (down) {
        contourTop = alphaBlendColors(contourTop, Qt::black, 200);
        contourBottom = alphaBlendColors(contourBottom, Qt::black, 200);
        surfaceTop = alphaBlendColors(surfaceTop, Qt::black, 200);
        surfaceBottom = alphaBlendColors(surfaceBottom, Qt::black, 200);
    }

    KPixmap buffer;
    buffer.resize(width(), height());
    QPainter bP(&buffer);

    // fill with the titlebar background
    bP.drawTiledPixmap(0, 0, width(), width(), backgroundTile, 0, TOPMARGIN);

    if (type() == MenuButton) {
        KPixmap menuIcon(m_client->icon().pixmap( QIconSet::Small, QIconSet::Normal));
        if (width() < menuIcon.width() || height() < menuIcon.height() ) {
            menuIcon.convertFromImage( menuIcon.convertToImage().smoothScale(width(), height()));
        }
        double fade = animProgress * 0.09;
        KPixmapEffect::fade(menuIcon, fade, QColor(240, 240, 240));
        bP.drawPixmap((width()-menuIcon.width())/2, (height()-menuIcon.height())/2, menuIcon);
    } else {
        // contour
        bP.setPen(contourTop);
        bP.drawLine(r.x()+2, r.y(), r.right()-2, r.y() );
        bP.drawPoint(r.x()+1, r.y()+1);
        bP.drawPoint(r.right()-1, r.y()+1);
        bP.setPen(contourBottom);
        bP.drawLine(r.x()+2, r.bottom(), r.right()-2, r.bottom() );
        bP.drawPoint(r.x()+1, r.bottom()-1);
        bP.drawPoint(r.right()-1, r.bottom()-1);
        // sides of the contour
        tempKPixmap.resize(1, r.height()-2*2);
        KPixmapEffect::gradient(tempKPixmap,
                                contourTop,
                                contourBottom,
                                KPixmapEffect::VerticalGradient);
        bP.drawPixmap(r.x(), r.y()+2, tempKPixmap);
        bP.drawPixmap(r.right(), r.y()+2, tempKPixmap);
        // sort of anti-alias for the contour
        bP.setPen(alphaBlendColors(Handler()->getColor(TitleGradientFrom, active),
                contourTop, 150) );
        bP.drawPoint(r.x()+1, r.y());
        bP.drawPoint(r.right()-1, r.y());
        bP.drawPoint(r.x(), r.y()+1);
        bP.drawPoint(r.right(), r.y()+1);
        bP.setPen(alphaBlendColors(Handler()->getColor(TitleGradientTo, active),
                contourBottom, 150) );
        bP.drawPoint(r.x()+1, r.bottom());
        bP.drawPoint(r.right()-1, r.bottom());
        bP.drawPoint(r.x(), r.bottom()-1);
        bP.drawPoint(r.right(), r.bottom()-1);

        // surface
        // fill top and bottom
        if (Handler()->buttonType() == PLASTIK_FLAT) {
		bP.setPen(surfaceTop);
		bP.drawLine(r.x()+2, r.y()+1, r.right()-2, r.y()+1 );
		bP.setPen(surfaceBottom);
		bP.drawLine(r.x()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
        } else if (Handler()->buttonType() == LIPSTIK_FLAT || down) {
		bP.setPen(surfaceBottom);
		bP.drawLine(r.x()+2, r.y()+1, r.right()-2, r.y()+1 );
		bP.setPen(surfaceTop);
		bP.drawLine(r.x()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
        } else {
		bP.setPen(surfaceBottom.light(117));
		bP.drawLine(r.x()+2, r.y()+1, r.right()-2, r.y()+1 );
		bP.setPen(surfaceTop.dark(122));
		bP.drawLine(r.x()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
	}

        // fill the rest! :)
        tempKPixmap.resize(1, r.height()-2*2);
        if (Handler()->buttonType() == PLASTIK_FLAT) {
		KPixmapEffect::gradient(tempKPixmap,
					surfaceTop,
					surfaceBottom,
					KPixmapEffect::VerticalGradient);
        } else {
		KPixmapEffect::gradient(tempKPixmap,
					surfaceBottom,
					surfaceTop,
					KPixmapEffect::VerticalGradient);
        }
        bP.drawTiledPixmap(r.x()+1, r.y()+2, r.width()-2, r.height()-4, tempKPixmap);

        int dX,dY;
        KPixmap deco;
        int s = lroundf(r.height()*Handler()->iconSize());
        if ((s + r.height())%2 != 0) --s;

        if (down) {
            deco = active ? Handler()->buttonPixmap(m_iconType, s, ActiveDown) : Handler()->buttonPixmap(m_iconType, s, InactiveDown);
        } else {
            deco = active ? Handler()->buttonPixmap(m_iconType, s, ActiveUp) : Handler()->buttonPixmap(m_iconType, s, InactiveUp);
        }
        dX = r.x()+(r.width()-deco.width())/2;
        dY = r.y()+(r.height()-deco.height())/2;
        if (down) {
            dY++;
        }

        if(active && !down &&
           (Handler()->useTitleProps() && Handler()->titleShadow() || Handler()->iconShadow())) {
            bP.drawPixmap(dX+1, dY+1, Handler()->buttonPixmap(m_iconType, s, Shadow));
        }

        bP.drawPixmap(dX, dY, deco);
    }

    bP.end();
    painter->drawPixmap(0, 0, buffer);
}

void SUSE2Button::drawLipstikBtn(QPainter *painter)
{
    QRect r(0, 0, width(), height());

    bool active = m_client->isActive();
    bool down = isDown();
    KPixmap backgroundTile = m_client->getTitleBarTile(active);

    KPixmap buffer;
    buffer.resize(width(), height());
    QPainter bP(&buffer);

    // fill with the titlebar background
    bP.drawTiledPixmap(0, 0, width(), width(), backgroundTile, 0, TOPMARGIN);

    if (type() == MenuButton)
    {
        KPixmap menuIcon(m_client->icon().pixmap( QIconSet::Small, QIconSet::Normal));
        if (width() < menuIcon.width() || height() < menuIcon.height() ) {
            menuIcon.convertFromImage( menuIcon.convertToImage().smoothScale(width(), height()));
        }
        double fade = animProgress * 0.09;
        KPixmapEffect::fade(menuIcon, fade, QColor(240, 240, 240));
        bP.drawPixmap((width()-menuIcon.width())/2, (height()-menuIcon.height())/2, menuIcon);
    } else {
        renderBtnContour(&bP, r);
        if (down) {
            QColor downColor;
            if (type() == CloseButton && active && Handler()->redCloseButton()) {
                downColor = QColor(220,65,65).dark(115);
            } else if (Handler()->customColors()) {
                downColor = Handler()->getColor(BtnBg, active).dark(115);
            } else {
                downColor = alphaBlendColors(Handler()->getColor(TitleGradientFrom, active), Qt::black, 220).dark(115);
            }
            bP.fillRect(r.left()+1, r.top()+2, r.width()-2, r.height()-4, downColor);
            bP.setPen(downColor);
            // top line
            bP.drawLine(r.left()+2, r.top()+1, r.right()-2, r.top()+1);
            // bottom and right lines
            bP.setPen(downColor.light(106));
            bP.drawLine(r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1);
            bP.drawLine(r.right()-1, r.top()+2, r.right()-1, r.bottom()-2);
        } else {
            renderBtnSurface(&bP, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2));
        }

        int dX,dY;
        KPixmap deco;
        int s = lroundf(r.height() * Handler()->iconSize());
        if ((s + r.height())%2 != 0) --s;

        if (down) {
            deco = active ? Handler()->buttonPixmap(m_iconType, s, ActiveDown) : Handler()->buttonPixmap(m_iconType, s, InactiveDown);
        } else {
            deco = active ? Handler()->buttonPixmap(m_iconType, s, ActiveUp) : Handler()->buttonPixmap(m_iconType, s, InactiveUp);
        }

        dX = r.x()+(r.width()-deco.width())/2;
        dY = r.y()+(r.height()-deco.height())/2;
        if (down) {
            dY++;
        }

        if(active && !down &&
           (Handler()->useTitleProps() && Handler()->titleShadow() || Handler()->iconShadow())) {
            bP.drawPixmap(dX+1, dY+1, Handler()->buttonPixmap(m_iconType, s, Shadow));
        }

        bP.drawPixmap(dX, dY, deco);
    }

    bP.end();
    painter->drawPixmap(0, 0, buffer);
}

void SUSE2Button::renderBtnContour(QPainter *p, const QRect &r)
{
    if((r.width() <= 0)||(r.height() <= 0))
        return;

    bool active = m_client->isActive();

    QColor backgroundColor;
    if (type() == CloseButton && active && Handler()->redCloseButton()) {
        backgroundColor = QColor(220,65,65);
    } else if (Handler()->customColors()) {
        backgroundColor = Handler()->getColor(BtnBg, active);
    } else {
        backgroundColor = alphaBlendColors(Handler()->getColor(TitleGradientFrom, active), Qt::black, 220);
    }

    QColor contourColor = backgroundColor.dark(135);

// sides
    p->setPen(contourColor);
    p->drawLine(r.left(), r.top()+2, r.left(), r.bottom()-2);
    p->drawLine(r.right(), r.top()+2, r.right(), r.bottom()-2);
    p->drawLine(r.left()+2, r.top(), r.right()-2, r.top());
    p->drawLine(r.left()+2, r.bottom(), r.right()-2, r.bottom());

// edges
    const int alphaAA = 110; // the alpha value for anti-aliasing...

    // first part...
    p->setPen(contourColor);
    p->drawPoint(r.left()+1, r.top()+1);
    p->drawPoint(r.left()+1, r.bottom()-1);
    p->drawPoint(r.right()-1, r.top()+1);
    p->drawPoint(r.right()-1, r.bottom()-1);

    // second part... fill edges in case we don't paint alpha-blended
    p->setPen( backgroundColor );

    // third part... anti-aliasing...
    int intActive = active ? 0 : 1;
    renderPixel(p,QPoint(r.left()+1,r.top()),alphaAA,contourColor, 0, intActive);
    renderPixel(p,QPoint(r.left(),r.top()+1),alphaAA,contourColor, 1, intActive);
    renderPixel(p,QPoint(r.left()+1,r.bottom()),alphaAA,contourColor, 2, intActive);
    renderPixel(p,QPoint(r.left(),r.bottom()-1),alphaAA,contourColor, 3, intActive);
    renderPixel(p,QPoint(r.right()-1,r.top()),alphaAA,contourColor, 4, intActive);
    renderPixel(p,QPoint(r.right(),r.top()+1),alphaAA,contourColor, 5, intActive);
    renderPixel(p,QPoint(r.right()-1,r.bottom()),alphaAA,contourColor, 6, intActive);
    renderPixel(p,QPoint(r.right(),r.bottom()-1),alphaAA,contourColor, 7, intActive);
}

void SUSE2Button::renderBtnSurface(QPainter *p, const QRect &r)
{
    if((r.width() <= 0)||(r.height() <= 0))
        return;

    bool active = m_client->isActive();

    QColor backgroundColor;
    if (type() == CloseButton && active && Handler()->redCloseButton()) {
        backgroundColor = QColor(220,65,65);
    } else if (Handler()->customColors()) {
        backgroundColor = Handler()->getColor(BtnBg, active);
    } else {
        backgroundColor = alphaBlendColors(Handler()->getColor(TitleGradientFrom, active), Qt::black, 220);
    }

    QColor highlightColor;
    if(type() == CloseButton) {
        highlightColor = QColor(255,0,0);
    } else {
        highlightColor = Qt::white;
    }

    int highlightAlpha = static_cast<int>(255-((60/static_cast<double>(ANIMATIONSTEPS)) *
                                          static_cast<double>(animProgress) ) );

    QColor buttonColor, bottomColor, topLineColor, bottomLineColor;

    if (Handler()->buttonType() == LIPSTIK_3D) { // Lipstik
        buttonColor = backgroundColor;
        bottomColor = buttonColor.light(112);
        topLineColor = buttonColor.light(112);
        bottomLineColor = buttonColor.dark(102);
    } else { // Plastik
        bottomColor = backgroundColor;
        buttonColor = bottomColor.light(130);
        topLineColor = buttonColor.light(108);
        bottomLineColor = bottomColor.dark(108);
    }

    buttonColor = alphaBlendColors(buttonColor, highlightColor, highlightAlpha);
    bottomColor = alphaBlendColors(bottomColor, highlightColor, highlightAlpha);
    topLineColor = alphaBlendColors(topLineColor, highlightColor, highlightAlpha);
    bottomLineColor = alphaBlendColors(bottomLineColor, highlightColor, highlightAlpha);

    int intActive = active ? 0 : 1;

// sides,left
    int height = r.height() - 2;
    renderGradient(p, QRect(r.left(), r.top()+1, 1, height), bottomColor, buttonColor, 8+animProgress, intActive);
//right
    renderGradient(p, QRect(r.right(), r.top()+1, 1, height), bottomColor, buttonColor, 9+ANIMATIONSTEPS+animProgress, intActive);
//top
    p->setPen(topLineColor);
    p->drawLine(r.left()+1, r.top(), r.right()-1, r.top() );
//bottom
    p->setPen(bottomLineColor);
    p->drawLine(r.left()+1, r.bottom(), r.right()-1, r.bottom() );

// button area...
    int width = r.width();
    height = r.height();
    width-=2;
    height-=2;
    renderGradient(p, QRect(r.left()+1, r.top()+1, width, height),  bottomColor, buttonColor,
                   10 + 2 * ANIMATIONSTEPS+animProgress, intActive);
}

void SUSE2Button::renderPixel(QPainter *p, const QPoint &pos, const int alpha, const QColor &color,
                              const int pixelPos, const int active)
{
    if (lipstikCache[pixelPos][active]) {
        p->drawPixmap(pos, *(lipstikCache[pixelPos][active]));
    } else {
        QRgb rgb = color.rgb();
        QImage aImg(1,1,32); // 1x1
        aImg.setAlphaBuffer(true);
        aImg.setPixel(0,0,qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),alpha));
        QPixmap *result = new QPixmap(aImg);

        p->drawPixmap(pos, *result);

        // add to the cache...
        lipstikCache[pixelPos][active] = result;
    }
}

void SUSE2Button::renderGradient(QPainter *painter, const QRect &rect, const QColor &c1, const QColor &c2,
                                 const int gradient, const int active)
{
    if((rect.width() <= 0)||(rect.height() <= 0))
        return;

    if (lipstikCache[gradient][active]) {
        painter->drawTiledPixmap(rect, *(lipstikCache[gradient][active]));
    } else {
        // there wasn't anything matching in the cache, create the pixmap now...
        QPixmap *result = new QPixmap(10, rect.height());
        QPainter p(result);

        int r_h = result->rect().height();
        int r_x, r_y, r_x2, r_y2;
        result->rect().coords(&r_x, &r_y, &r_x2, &r_y2);

        int rDiff, gDiff, bDiff;
        int rc, gc, bc;

        register int y;

        rDiff = ( c1.red())   - (rc = c2.red());
        gDiff = ( c1.green()) - (gc = c2.green());
        bDiff = ( c1.blue())  - (bc = c2.blue());

        register int rl = rc << 16;
        register int gl = gc << 16;
        register int bl = bc << 16;

        int rdelta = ((1<<16) / r_h) * rDiff;
        int gdelta = ((1<<16) / r_h) * gDiff;
        int bdelta = ((1<<16) / r_h) * bDiff;

        // these for-loops could be merged, but the if's in the inner loop
        // would make it slow
        for ( y = 0; y < r_h; y++ ) {
            rl += rdelta;
            gl += gdelta;
            bl += bdelta;

            p.setPen(QColor(rl>>16, gl>>16, bl>>16));
            p.drawLine(r_x, r_y+y, r_x2, r_y+y);
        }

        p.end();

        // draw the result...
        painter->drawTiledPixmap(rect, *result);

        // add to the cache...
        lipstikCache[gradient][active] = result;
    }
}

QBitmap IconEngine::icon(ButtonIcon icon, int size)
{
    QBitmap bitmap(size,size);
    bitmap.fill(Qt::color0);
    QPainter p(&bitmap);

    p.setPen(Qt::color1);

    QRect r = bitmap.rect();

    // line widths
    int lwTitleBar = 1;
    if (r.width() > 16) {
        lwTitleBar = 4;
    } else if (r.width() > 4) {
        lwTitleBar = 2;
    }
    int lwArrow = 1;
    if (r.width() > 16) {
        lwArrow = 4;
    } else if (r.width() > 7) {
        lwArrow = 2;
    }

    switch(icon) {
        case CloseIcon:
        {
            int lineWidth = 1;
            if (r.width() > 16) {
                lineWidth = 3;
            } else if (r.width() > 4) {
                lineWidth = 2;
            }

            drawObject(p, DiagonalLine, r.x(), r.y(), r.width(), lineWidth);
            drawObject(p, CrossDiagonalLine, r.x(), r.bottom(), r.width(), lineWidth);

            break;
        }

        case MaxIcon:
        {
            int lineWidth2 = 1; // frame
            if (r.width() > 16) {
                lineWidth2 = 2;
            } else if (r.width() > 4) {
                lineWidth2 = 1;
            }

            drawObject(p, HorizontalLine, r.x(), r.top(), r.width(), lwTitleBar);
            drawObject(p, HorizontalLine, r.x(), r.bottom()-(lineWidth2-1), r.width(), lineWidth2);
            drawObject(p, VerticalLine, r.x(), r.top(), r.height(), lineWidth2);
            drawObject(p, VerticalLine, r.right()-(lineWidth2-1), r.top(), r.height(), lineWidth2);

            break;
        }

        case MaxRestoreIcon:
        {
            int lineWidth2 = 1; // frame
            if (r.width() > 16) {
                lineWidth2 = 2;
            } else if (r.width() > 4) {
                lineWidth2 = 1;
            }

            int margin1, margin2;
            margin1 = margin2 = lineWidth2 * 2;
            if (r.width() < 8)
                margin1 = 1;

            // background window
            drawObject(p, HorizontalLine, r.x()+margin1, r.top(), r.width()-margin1, lineWidth2);
            drawObject(p, HorizontalLine, r.right()-margin2, r.bottom()-(lineWidth2 - 1)-margin1, margin2, lineWidth2);
            drawObject(p, VerticalLine, r.x()+margin1, r.top(), margin2, lineWidth2);
            drawObject(p, VerticalLine, r.right()-(lineWidth2 - 1), r.top(), r.height()-margin1, lineWidth2);

            // foreground window
            drawObject(p, HorizontalLine, r.x(), r.top()+margin2, r.width()-margin2, lwTitleBar);
            drawObject(p, HorizontalLine, r.x(), r.bottom()-(lineWidth2 - 1), r.width()-margin2, lineWidth2);
            drawObject(p, VerticalLine, r.x(), r.top()+margin2, r.height(), lineWidth2);
            drawObject(p, VerticalLine, r.right()-(lineWidth2 - 1)-margin2, r.top()+margin2, r.height(), lineWidth2);

            break;
        }

        case MinIcon:
        {
            drawObject(p, HorizontalLine, r.x(), r.bottom()-(lwTitleBar - 1), r.width(), lwTitleBar);

            break;
        }

        case HelpIcon:
        {
            int center = r.x()+r.width() / 2 - 1;
            int side = r.width() / 4;

            // paint a question mark... code is quite messy, to be cleaned up later...! :o

            if (r.width() > 16) {
                int lineWidth = 3;

                // top bar
                drawObject(p, HorizontalLine, center-side + 3, r.y(), 2 * side - 3 - 1, lineWidth);
                // top bar rounding
                drawObject(p, CrossDiagonalLine, center - side - 1, r.y() + 5, 6, lineWidth);
                drawObject(p, DiagonalLine, center+side-3, r.y(), 5, lineWidth);
                // right bar
                drawObject(p, VerticalLine, center+side+2-lineWidth, r.y()+3, r.height()-(2*lineWidth+side+2+1), lineWidth);
                // bottom bar
                drawObject(p, CrossDiagonalLine, center, r.bottom()-2*lineWidth, side+2, lineWidth);
                drawObject(p, HorizontalLine, center, r.bottom()-3*lineWidth+2, lineWidth, lineWidth);
                // the dot
                drawObject(p, HorizontalLine, center, r.bottom()-(lineWidth-1), lineWidth, lineWidth);
            } else if (r.width() > 8) {
                int lineWidth = 2;

                // top bar
                drawObject(p, HorizontalLine, center-(side-1), r.y(), 2*side-1, lineWidth);
                // top bar rounding
                if (r.width() > 9) {
                    drawObject(p, CrossDiagonalLine, center-side-1, r.y()+3, 3, lineWidth);
                } else {
                    drawObject(p, CrossDiagonalLine, center-side-1, r.y()+2, 3, lineWidth);
                }
                drawObject(p, DiagonalLine, center+side-1, r.y(), 3, lineWidth);
                // right bar
                drawObject(p, VerticalLine, center+side+2-lineWidth, r.y()+2, r.height()-(2*lineWidth+side+1), lineWidth);
                // bottom bar
                drawObject(p, CrossDiagonalLine, center, r.bottom()-2*lineWidth+1, side+2, lineWidth);
                // the dot
                drawObject(p, HorizontalLine, center, r.bottom()-(lineWidth-1), lineWidth, lineWidth);
            } else {
                int lineWidth = 1;

                // top bar
                drawObject(p, HorizontalLine, center-(side-1), r.y(), 2*side, lineWidth);
                // top bar rounding
                drawObject(p, CrossDiagonalLine, center-side-1, r.y()+1, 2, lineWidth);
                // right bar
                drawObject(p, VerticalLine, center+side+1, r.y(), r.height()-(side+2+1), lineWidth);
                // bottom bar
                drawObject(p, CrossDiagonalLine, center, r.bottom()-2, side+2, lineWidth);
                // the dot
                drawObject(p, HorizontalLine, center, r.bottom(), 1, 1);
            }

            break;
        }

        case NotOnAllDesktopsIcon:
        {
            int lwMark = r.width()-lwTitleBar*2-2;
            if (lwMark < 1)
                lwMark = 3;

            drawObject(p, HorizontalLine, r.x()+(r.width()-lwMark)/2, r.y()+(r.height()-lwMark)/2, lwMark, lwMark);

            // Fall through to OnAllDesktopsIcon intended!
        }
        case OnAllDesktopsIcon:
        {
            // horizontal bars
            drawObject(p, HorizontalLine, r.x()+lwTitleBar, r.y(), r.width()-2*lwTitleBar, lwTitleBar);
            drawObject(p, HorizontalLine, r.x()+lwTitleBar, r.bottom()-(lwTitleBar-1), r.width()-2*lwTitleBar, lwTitleBar);
            // vertical bars
            drawObject(p, VerticalLine, r.x(), r.y()+lwTitleBar, r.height()-2*lwTitleBar, lwTitleBar);
            drawObject(p, VerticalLine, r.right()-(lwTitleBar-1), r.y()+lwTitleBar, r.height()-2*lwTitleBar, lwTitleBar);


            break;
        }

        case NoKeepAboveIcon:
        {
            int center = r.x()+r.width()/2;

            // arrow
            drawObject(p, CrossDiagonalLine, r.x(), center+2*lwArrow, center-r.x(), lwArrow);
            drawObject(p, DiagonalLine, r.x()+center, r.y()+1+2*lwArrow, center-r.x(), lwArrow);
            if (lwArrow>1)
                drawObject(p, HorizontalLine, center-(lwArrow-2), r.y()+2*lwArrow, (lwArrow-2)*2, lwArrow);

            // Fall through to KeepAboveIcon intended!
        }
        case KeepAboveIcon:
        {
            int center = r.x()+r.width()/2;

            // arrow
            drawObject(p, CrossDiagonalLine, r.x(), center, center-r.x(), lwArrow);
            drawObject(p, DiagonalLine, r.x()+center, r.y()+1, center-r.x(), lwArrow);
            if (lwArrow>1)
                drawObject(p, HorizontalLine, center-(lwArrow-2), r.y(), (lwArrow-2)*2, lwArrow);

            break;
        }

        case NoKeepBelowIcon:
        {
            int center = r.x()+r.width()/2;

            // arrow
            drawObject(p, DiagonalLine, r.x(), center-2*lwArrow, center-r.x(), lwArrow);
            drawObject(p, CrossDiagonalLine, r.x()+center, r.bottom()-1-2*lwArrow, center-r.x(), lwArrow);
            if (lwArrow>1)
                drawObject(p, HorizontalLine, center-(lwArrow-2), r.bottom()-(lwArrow-1)-2*lwArrow, (lwArrow-2)*2, lwArrow);

            // Fall through to KeepBelowIcon intended!
        }
        case KeepBelowIcon:
        {
            int center = r.x()+r.width()/2;

            // arrow
            drawObject(p, DiagonalLine, r.x(), center, center-r.x(), lwArrow);
            drawObject(p, CrossDiagonalLine, r.x()+center, r.bottom()-1, center-r.x(), lwArrow);
            if (lwArrow>1)
                drawObject(p, HorizontalLine, center-(lwArrow-2), r.bottom()-(lwArrow-1), (lwArrow-2)*2, lwArrow);

            break;
        }

        case ShadeIcon:
        {
            drawObject(p, HorizontalLine, r.x(), r.y(), r.width(), lwTitleBar);

            break;
        }

        case UnShadeIcon:
        {
            int lw1 = 1;
            int lw2 = 1;
            if (r.width() > 16) {
                lw1 = 4;
                lw2 = 2;
            } else if (r.width() > 7) {
                lw1 = 2;
                lw2 = 1;
            }

            int h = QMAX( (r.width()/2), (lw1+2*lw2) );

            // horizontal bars
            drawObject(p, HorizontalLine, r.x(), r.y(), r.width(), lw1);
            drawObject(p, HorizontalLine, r.x(), r.x()+h-(lw2-1), r.width(), lw2);
            // vertical bars
            drawObject(p, VerticalLine, r.x(), r.y(), h, lw2);
            drawObject(p, VerticalLine, r.right()-(lw2-1), r.y(), h, lw2);

            break;
        }

        default:
            break;
    }

    p.end();

    bitmap.setMask(bitmap);

    return bitmap;
}

void IconEngine::drawObject(QPainter &p, Object object, int x, int y, int length, int lineWidth)
{
    switch(object) {
        case DiagonalLine:
            if (lineWidth <= 1) {
                for (int i = 0; i < length; ++i) {
                    p.drawPoint(x+i,y+i);
                }
            } else if (lineWidth <= 2) {
                for (int i = 0; i < length; ++i) {
                    p.drawPoint(x+i,y+i);
                }
                for (int i = 0; i < (length-1); ++i) {
                    p.drawPoint(x+1+i,y+i);
                    p.drawPoint(x+i,y+1+i);
                }
            } else {
                for (int i = 1; i < (length-1); ++i) {
                    p.drawPoint(x+i,y+i);
                }
                for (int i = 0; i < (length-1); ++i) {
                    p.drawPoint(x+1+i,y+i);
                    p.drawPoint(x+i,y+1+i);
                }
                for (int i = 0; i < (length-2); ++i) {
                    p.drawPoint(x+2+i,y+i);
                    p.drawPoint(x+i,y+2+i);
                }
            }
            break;
        case CrossDiagonalLine:
            if (lineWidth <= 1) {
                for (int i = 0; i < length; ++i) {
                    p.drawPoint(x+i,y-i);
                }
            } else if (lineWidth <= 2) {
                for (int i = 0; i < length; ++i) {
                    p.drawPoint(x+i,y-i);
                }
                for (int i = 0; i < (length-1); ++i) {
                    p.drawPoint(x+1+i,y-i);
                    p.drawPoint(x+i,y-1-i);
                }
            } else {
                for (int i = 1; i < (length-1); ++i) {
                    p.drawPoint(x+i,y-i);
                }
                for (int i = 0; i < (length-1); ++i) {
                    p.drawPoint(x+1+i,y-i);
                    p.drawPoint(x+i,y-1-i);
                }
                for (int i = 0; i < (length-2); ++i) {
                    p.drawPoint(x+2+i,y-i);
                    p.drawPoint(x+i,y-2-i);
                }
            }
            break;
        case HorizontalLine:
            for (int i = 0; i < lineWidth; ++i) {
                p.drawLine(x,y+i, x+length-1, y+i);
            }
            break;
        case VerticalLine:
            for (int i = 0; i < lineWidth; ++i) {
                p.drawLine(x+i,y, x+i, y+length-1);
            }
            break;
        default:
            break;
    }
}

} // KWinSUSE2

// kate: space-indent on; indent-width 4; replace-tabs on;
