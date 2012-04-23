/* SuSE KWin window decoration
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

#include <klocale.h>
#include <kpixmap.h>
#include <kimageeffect.h>
#include <kpixmapeffect.h>
#include <kstandarddirs.h>
#include <kdecoration.h>

#include <qbitmap.h>
#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>

#include "SUSE2client.h"
#include "SUSE2button.h"
#include "misc.h"
#include "shadow.h"

namespace KWinSUSE2
{
SUSE2Client::SUSE2Client(KDecorationBridge* bridge, KDecorationFactory* factory)
    : KCommonDecoration(bridge, factory),
    aCaptionBuffer(0), iCaptionBuffer(0),
    aTitleBarTile(0), iTitleBarTile(0),
    pixmaps_created(false),
    captionBufferDirty(true),
    s_titleFont(QFont())
{}

SUSE2Client::~SUSE2Client()
{
    delete_pixmaps();

    delete aCaptionBuffer;
    delete iCaptionBuffer;
}

// pure virtual methods from KCommonDecoration
QString SUSE2Client::visibleName() const
{
    return i18n("SUSE2 (Version %1)").arg("0.4");
}

QString SUSE2Client::defaultButtonsLeft() const
{
    return "M";
}

QString SUSE2Client::defaultButtonsRight() const
{
    return "HIA___X";
}

bool SUSE2Client::decorationBehaviour(DecorationBehaviour behaviour) const
{
    switch (behaviour) {
        case DB_MenuClose:
            return Handler()->menuClose();

        case DB_WindowMask:
            return true;

        default:
            return KCommonDecoration::decorationBehaviour(behaviour);
    }
}

KCommonDecorationButton *SUSE2Client::createButton(ButtonType type)
{
    switch (type) {
        case MenuButton:
            return new SUSE2Button(MenuButton, this, "menu");

        case OnAllDesktopsButton:
            return new SUSE2Button(OnAllDesktopsButton, this, "on_all_desktops");

        case HelpButton:
            return new SUSE2Button(HelpButton, this, "help");

        case MinButton:
            return new SUSE2Button(MinButton, this, "minimize");

        case MaxButton:
            return new SUSE2Button(MaxButton, this, "maximize");

        case CloseButton:
            return new SUSE2Button(CloseButton, this, "close");

        case AboveButton:
            return new SUSE2Button(AboveButton, this, "above");

        case BelowButton:
            return new SUSE2Button(BelowButton, this, "below");

        case ShadeButton:
            return new SUSE2Button(ShadeButton, this, "shade");

        default:
            return 0;
    }
}

int SUSE2Client::layoutMetric(LayoutMetric lm, bool respectWindowState, const KCommonDecorationButton *btn) const
{
    bool maximized = maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows();

    switch (lm) {
        case LM_BorderLeft:
        case LM_BorderRight:
        case LM_BorderBottom:
            if (respectWindowState && maximized)
                return 0;
            return Handler()->borderSize();

        case LM_TitleEdgeTop:
            if (respectWindowState && maximized) {
                return 0;
            } else {
                return TOPMARGIN;
            }

        case LM_TitleEdgeBottom:
             if (respectWindowState && maximized) {
                 return 1;
             } else {
                return DECOHEIGHT;
             }

        case LM_TitleEdgeLeft:
        case LM_TitleEdgeRight:
            if (respectWindowState && maximized) {
                return 0;
            } else {
                return 6;
            }

        case LM_TitleBorderLeft:
        case LM_TitleBorderRight:
            return 5;

        case LM_ButtonWidth:
        case LM_ButtonHeight:
        case LM_TitleHeight:
            if (respectWindowState && isToolWindow()) {
                return Handler()->titleHeightTool();
            } else {
                return Handler()->titleHeight();
            }

        case LM_ButtonSpacing:
            return 1;

        case LM_ButtonMarginTop:
            return 0;

        case LM_ExplicitButtonSpacer:
            return 3;

        default:
            return KCommonDecoration::layoutMetric(lm, respectWindowState, btn);
    }
}

void SUSE2Client::init()
{
    s_titleFont = isToolWindow() ?
                  Handler()->titleFontTool()
                  : Handler()->titleFont();

    create_pixmaps();

    aCaptionBuffer = new QPixmap();
    iCaptionBuffer = new QPixmap();
    captionBufferDirty = true;

    KCommonDecoration::init();
}

QRegion SUSE2Client::cornerShape(WindowCorner corner)
{
    int w = widget()->width();
    int h = widget()->height();

    switch (corner) {
        case WC_TopLeft:
            if (layoutMetric(LM_TitleEdgeLeft) > 0 &&
                (Handler()->roundCorners() == 1 ||
                (Handler()->roundCorners() == 2 && maximizeMode() != MaximizeFull))) {
                return QRegion(0, 0, 1, 5) + QRegion(0, 0, 2, 3) + QRegion(0, 0, 3, 2) + QRegion(0, 0, 5, 1);
            } else {
                return QRegion(0, 0, 1, 1);
            }

        case WC_TopRight:
            if (layoutMetric(LM_TitleEdgeRight) > 0 &&
                (Handler()->roundCorners() == 1 ||
                (Handler()->roundCorners() == 2 && maximizeMode() != MaximizeFull))) {
                return QRegion(w-1, 0, 1, 5) + QRegion(w-2, 0, 2, 3) + QRegion(w-3, 0, 3, 2) + QRegion(w-5, 0, 5, 1);
            } else {
                return QRegion(w-1, 0, 1, 1);
            }

        case WC_BottomLeft:
            if (layoutMetric(LM_BorderLeft) > 0)
                return QRegion(0, h-1, 1, 1);
            else
                return QRegion();

        case WC_BottomRight:
            if (layoutMetric(LM_BorderRight) > 0)
                return QRegion(w-1, h-1, 1, 1);
            else
                return QRegion();

        default:
            return QRegion();
    }
}

void SUSE2Client::paintEvent(QPaintEvent *e)
{
    QRegion region = e->region();

    if (captionBufferDirty) update_captionBuffer();

    bool active = isActive();

    QPainter painter(widget());

    // colors...
    const QColor windowContour = Handler()->getColor(WindowContour, active);
    const QColor innerWindowContour = Handler()->getColor(TitleGradientTo, active);
    const QColor deco = Handler()->getColor(TitleGradientTo, active);
    const QColor aBorder = Handler()->getColor(Border, active);
    const QColor darkAntiAlias = alphaBlendColors(innerWindowContour, active ? aAntialiasBase : iAntialiasBase, 140);
    const QColor midAntiAlias = alphaBlendColors(innerWindowContour, active ? aAntialiasBase : iAntialiasBase, 120);
    const QColor brightAntiAlias = alphaBlendColors(innerWindowContour, active ? aAntialiasBase : iAntialiasBase, 80);
    const QColor innerAntiAlias = alphaBlendColors(innerWindowContour, active ? aAntialiasBase : iAntialiasBase, 170);

    QRect r = widget()->rect();

    int r_w = r.width();
    int r_x, r_y, r_x2, r_y2;
    r.coords(&r_x, &r_y, &r_x2, &r_y2);
    const int borderLeft = layoutMetric(LM_BorderLeft);
    const int borderRight = layoutMetric(LM_BorderRight);
    const int borderBottom = layoutMetric(LM_BorderBottom);
    const int titleHeight = layoutMetric(LM_TitleHeight);
    const int titleEdgeTop = layoutMetric(LM_TitleEdgeTop);
    const int titleEdgeBottom = layoutMetric(LM_TitleEdgeBottom);
    const int titleEdgeLeft = layoutMetric(LM_TitleEdgeLeft);
    const int titleEdgeRight = layoutMetric(LM_TitleEdgeRight);
    const int btnMarginTop = layoutMetric(LM_ButtonMarginTop);
    const int titleMargin = layoutMetric(LM_TitleBorderLeft);

    const int borderBottomTop = r_y2-borderBottom+1;
    const int borderLeftRight = r_x+borderLeft-1;
    const int borderRightLeft = r_x2-borderRight+1;
    const int titleEdgeBottomBottom = r_y+titleEdgeTop+titleHeight+titleEdgeBottom-1;

    const int sideHeight = borderBottomTop-titleEdgeBottomBottom-1;

     // the title background over full width
    if (maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows()) {
        painter.drawTiledPixmap(r_x, r_y,
                                r_w, titleHeight + titleEdgeTop + titleEdgeBottom,
                                active ? *aTitleBarTile : *iTitleBarTile, 0, TOPMARGIN - btnMarginTop);
    } else {
        painter.drawTiledPixmap(r_x, r_y,
                                r_w, titleHeight + titleEdgeTop + titleEdgeBottom,
                                active ? *aTitleBarTile : *iTitleBarTile);
    }

    QRect Rtitle = QRect(r_x+titleEdgeLeft+buttonsLeftWidth(), r_y+titleEdgeTop,
                         r_x2-titleEdgeRight-buttonsRightWidth()-(r_x+titleEdgeLeft+buttonsLeftWidth()),
                         titleEdgeBottomBottom-(r_y+titleEdgeTop) );

    QRect tmpRect;

    // topSpacer
    if(titleEdgeTop > 0) {
        tmpRect.setRect(r_x, r_y, r_w, titleEdgeTop );
        if (tmpRect.isValid() && region.contains(tmpRect) ) {
            painter.setPen(windowContour);
            if (Handler()->roundCorners() == 1 ||
                (Handler()->roundCorners() == 2 && maximizeMode() != MaximizeFull)) {
                // top line
                if (maximizeMode() != MaximizeFull || options()->moveResizeMaximizedWindows()) {
                    painter.fillRect(tmpRect.left(), tmpRect.top(), 2, titleEdgeTop+1, windowContour);
                    painter.fillRect(tmpRect.left()+2, tmpRect.top(), 3, 3, windowContour);
                    painter.fillRect(tmpRect.right()-1, tmpRect.top(), 2, titleEdgeTop+1, windowContour);
                    painter.fillRect(tmpRect.right()-4, tmpRect.top(), 3, 3, windowContour);
                    painter.drawLine(tmpRect.left()+5, tmpRect.top(), tmpRect.right()-5, tmpRect.top());
                    painter.setPen(innerWindowContour);
                    painter.drawLine(tmpRect.left()+5, tmpRect.top()+1, tmpRect.right()-5, tmpRect.top()+1);
                    painter.setPen(windowContour);
                }

                painter.setPen(innerWindowContour);
                // left inner corner
                painter.drawPoint(tmpRect.left()+3, tmpRect.top()+2);
                painter.drawPoint(tmpRect.left()+2, tmpRect.top()+3);
                // right inner corner
                painter.drawPoint(tmpRect.right()-3, tmpRect.top()+2);
                painter.drawPoint(tmpRect.right()-2, tmpRect.top()+3);
                painter.setPen(midAntiAlias);
                painter.drawPoint(tmpRect.left()+3, tmpRect.top()+3);
                painter.drawPoint(tmpRect.right()-3, tmpRect.top()+3);
                painter.setPen(brightAntiAlias);
                painter.drawPoint(tmpRect.left()+5, tmpRect.top()+2);
                painter.drawPoint(tmpRect.right()-5, tmpRect.top()+2);
                painter.setPen(innerAntiAlias);
                painter.drawPoint(tmpRect.left()+4, tmpRect.top()+2);
                painter.drawPoint(tmpRect.left()+2, tmpRect.top()+4);
                painter.drawPoint(tmpRect.right()-4, tmpRect.top()+2);
                painter.drawPoint(tmpRect.right()-2, tmpRect.top()+4);
            } else {
                // top line
                if (maximizeMode() != MaximizeFull || options()->moveResizeMaximizedWindows()) {
                    painter.fillRect(tmpRect.left(), tmpRect.top(), 2, titleEdgeTop+1, windowContour);
                    painter.fillRect(tmpRect.left()+2, tmpRect.top(), 3, 2, windowContour);
                    painter.fillRect(tmpRect.right()-1, tmpRect.top(), 2, titleEdgeTop+1, windowContour);
                    painter.fillRect(tmpRect.right()-4, tmpRect.top(), 3, 2, windowContour);
                    painter.drawLine(tmpRect.left()+2, tmpRect.top(), tmpRect.right()-2, tmpRect.top() );
                    painter.setPen(innerWindowContour);
                    painter.drawLine(tmpRect.left()+1, tmpRect.top()+1, tmpRect.right()-1, tmpRect.top()+1);
                    painter.setPen(windowContour);
                }

                painter.setPen(alphaBlendColors(innerWindowContour, windowContour, 110));
                // left corner
                painter.drawLine(tmpRect.left(), tmpRect.top(), tmpRect.left()+1, tmpRect.top());
                painter.drawPoint(tmpRect.left(), tmpRect.top()+1);
                // right corner
                painter.drawLine(tmpRect.right(), tmpRect.top(), tmpRect.right()-1, tmpRect.top());
                painter.drawPoint(tmpRect.right(), tmpRect.top()+1);

                painter.setPen(windowContour);
                // left side
                painter.drawLine(tmpRect.left(), tmpRect.top()+2, tmpRect.left(), tmpRect.bottom());
                // right side
                painter.drawLine(tmpRect.right(), tmpRect.top()+2, tmpRect.right(), tmpRect.bottom());

                painter.setPen(innerWindowContour);
                // left inner corner
                painter.drawLine(tmpRect.left()+1, tmpRect.top()+2, tmpRect.left()+1, tmpRect.top()+4);
                // right inner corner
                painter.drawLine(tmpRect.right()-1, tmpRect.top()+2, tmpRect.right()-1, tmpRect.top()+4);
            }
        }
    }

    // leftTitleSpacer
    if(titleEdgeLeft > 1)
    {
        tmpRect.setRect(r_x, titleEdgeTop, borderLeft, titleEdgeTop+titleHeight+titleEdgeBottom);
        if (tmpRect.isValid() && region.contains(tmpRect) ) {
            painter.setPen(windowContour);
            painter.drawLine(tmpRect.left(), tmpRect.top(),
                             tmpRect.left(), tmpRect.bottom() );

            painter.setPen(innerWindowContour);
            painter.drawLine(tmpRect.left()+1, tmpRect.top()+1,
                             tmpRect.left()+1, tmpRect.bottom() );
            if (Handler()->roundCorners() == 1 ||
                (Handler()->roundCorners() == 2 && maximizeMode() != MaximizeFull)) {
                painter.setPen(darkAntiAlias);
                painter.drawPoint(tmpRect.left()+2, tmpRect.top()+1);
            }
        }
    }

    // rightTitleSpacer
    if(titleEdgeRight > 1)
    {
        tmpRect.setRect(borderRightLeft, titleEdgeTop, borderRight, titleEdgeTop+titleHeight+titleEdgeBottom);
        if (tmpRect.isValid() && region.contains(tmpRect) ) {
            painter.setPen(windowContour );
            painter.drawLine(tmpRect.right(), tmpRect.top(),
                            tmpRect.right(), tmpRect.bottom() );

            painter.setPen(innerWindowContour );
            painter.drawLine(tmpRect.right()-1, tmpRect.top()+1,
                            tmpRect.right()-1, tmpRect.bottom() );
            if (Handler()->roundCorners() == 1 ||
                (Handler()->roundCorners() == 2 && maximizeMode() != MaximizeFull)) {
                painter.setPen(darkAntiAlias);
                painter.drawPoint(tmpRect.right()-2, tmpRect.top()+1);
            }
        }
    }

    // titleSpacer
    QPixmap *titleBfrPtr = active ? aCaptionBuffer : iCaptionBuffer;
    if(Rtitle.width() > 0 && titleBfrPtr != 0) {

        int tX, tW;
        switch (Handler()->titleAlign()) {
            // AlignCenter
            case Qt::AlignHCenter:
                tX = (titleBfrPtr->width() > Rtitle.width()-2 * titleMargin) ?
                        (Rtitle.left()+titleMargin)
                        : Rtitle.left()+(Rtitle.width()- titleBfrPtr->width())/2;
                tW = (titleBfrPtr->width() > Rtitle.width()-2 * titleMargin) ?
                        (Rtitle.width()-2 * titleMargin)
                        : titleBfrPtr->width();
                break;
            // AlignRight
            case Qt::AlignRight:
                tX = (titleBfrPtr->width() > Rtitle.width()-2 * titleMargin) ?
                        (Rtitle.left()+titleMargin)
                        : Rtitle.right()-titleMargin-titleBfrPtr->width();
                tW = (titleBfrPtr->width() > Rtitle.width()-2 * titleMargin) ?
                        (Rtitle.width()-2 * titleMargin)
                        : titleBfrPtr->width();
                break;
            // AlignLeft
            default:
                tX = (Rtitle.left()+titleMargin);
                tW = (titleBfrPtr->width() > Rtitle.width()-2 * titleMargin) ?
                        (Rtitle.width()-2 * titleMargin)
                        : titleBfrPtr->width();
        }

        if(tW > 0) {
            painter.drawTiledPixmap(tX, Rtitle.top() - TOPMARGIN + btnMarginTop+2,
                                    tW, Rtitle.height() + titleEdgeBottom + titleEdgeTop,
                                    *titleBfrPtr, 0, 2);
        }
    }
    titleBfrPtr = 0;

     // leftSpacer
     if (borderLeft > 0 && sideHeight > 0) {
         tmpRect.setCoords(r_x, titleEdgeBottomBottom+1, borderLeftRight, borderBottomTop-1);
         if (tmpRect.isValid() && region.contains(tmpRect) ) {
             if (tmpRect.width() > 2) {
                 painter.fillRect(tmpRect, active ? aGradientBottom : iGradientBottom);
             }
             painter.setPen(windowContour);
             painter.drawLine(tmpRect.left(), tmpRect.top(), tmpRect.left(), tmpRect.bottom());
             painter.drawLine(tmpRect.right(), tmpRect.top(), tmpRect.right(), tmpRect.bottom());

             painter.setPen(innerWindowContour);
             painter.drawLine(tmpRect.left()+1, tmpRect.top(), tmpRect.left()+1, tmpRect.bottom());
         }
     }

    // rightSpacer
    if (borderRight > 0 && sideHeight > 0) {
        tmpRect.setCoords(borderRightLeft, titleEdgeBottomBottom+1, r_x2, borderBottomTop-1);
        if (tmpRect.isValid() && region.contains(tmpRect) ) {
            if (tmpRect.width() > 2) {
                painter.fillRect(tmpRect, active ? aGradientBottom : iGradientBottom);
            }
            painter.setPen(windowContour);
            painter.drawLine(tmpRect.right(), tmpRect.top(), tmpRect.right(), tmpRect.bottom());
            painter.drawLine(tmpRect.left(), tmpRect.top(), tmpRect.left(), tmpRect.bottom());

            painter.setPen(innerWindowContour);
            painter.drawLine(tmpRect.right()-1, tmpRect.top(), tmpRect.right()-1, tmpRect.bottom());
        }
    }

    // bottomSpacer
    if (borderBottom > 0) {
        tmpRect.setCoords(r_x, borderBottomTop, r_x2, r_y2);
        if (tmpRect.isValid() && region.contains(tmpRect) ) {

            if (borderBottomTop > 2) {
                painter.fillRect(tmpRect, active ? aGradientBottom : iGradientBottom);
            }

            // bottom lines
            painter.setPen(innerWindowContour);
            painter.drawLine(r_x, r_y2-1, r_x2, r_y2-1);

            painter.setPen(windowContour);
            painter.drawLine(r_x, r_y2, r_x2, r_y2); //bottom line
            //bottom upper line
            painter.drawLine(borderLeft-1, borderBottomTop, r_x2-borderRight+1, borderBottomTop);

            if (borderLeft > 0) {
                painter.setPen(innerWindowContour);
                painter.drawLine(tmpRect.left()+1, tmpRect.top(), tmpRect.left()+1, tmpRect.bottom()-1);

                painter.setPen(windowContour);
                // rounded left bottom corner - side
                painter.drawLine(tmpRect.left(), tmpRect.top(), tmpRect.left(), tmpRect.bottom()-2);

                // anti-alias for the window contour...
                painter.setPen(alphaBlendColors(innerWindowContour, windowContour, 90) );
                painter.drawLine(tmpRect.left(), tmpRect.bottom()-1, tmpRect.left(), tmpRect.bottom());
                painter.drawPoint(tmpRect.left()+1, tmpRect.bottom());
            }

            if (borderRight > 0) {
                painter.setPen(innerWindowContour);
                painter.drawLine(tmpRect.right()-1, tmpRect.top(), tmpRect.right()-1, tmpRect.bottom()-1);

                painter.setPen(windowContour);
                // rounded right bottom corner - side
                painter.drawLine(tmpRect.right(), tmpRect.top(), tmpRect.right(), tmpRect.bottom()-2);

                // anti-alias for the window contour...
                painter.setPen(alphaBlendColors(innerWindowContour, windowContour, 110) );
                painter.drawLine(tmpRect.right(), tmpRect.bottom()-1, tmpRect.right(), tmpRect.bottom());
                painter.drawPoint(tmpRect.right()-1, tmpRect.bottom());
            }
        }
    }
}

void SUSE2Client::updateCaption()
{
    captionBufferDirty = true;

    const int titleHeight = layoutMetric(LM_TitleHeight);
    const int titleEdgeBottom = layoutMetric(LM_TitleEdgeBottom);
    const int titleEdgeTop = layoutMetric(LM_TitleEdgeTop);
    const int titleEdgeLeft = layoutMetric(LM_TitleEdgeLeft);
    const int marginLeft = layoutMetric(LM_TitleBorderLeft);
    const int marginRight = layoutMetric(LM_TitleBorderRight);

    QRect g = QRect(titleEdgeLeft, titleEdgeTop, (widget()->rect()).width()-marginLeft-marginRight, titleHeight + titleEdgeBottom);
    widget()->update(g);
}

void SUSE2Client::reset(unsigned long changed)
{
    if (changed & SettingColors) {
        // repaint the whole thing
        delete_pixmaps();
        create_pixmaps();
        captionBufferDirty = true;
        widget()->update();
        updateButtons();
    } else if (changed & SettingFont) {
        s_titleFont = isToolWindow() ?
                      Handler()->titleFontTool()
                      : Handler()->titleFont();

        // then repaint
        delete_pixmaps();
        create_pixmaps();
        captionBufferDirty = true;
        widget()->update();
        updateButtons();
    }

    KCommonDecoration::reset(changed);
}

void SUSE2Client::create_pixmaps()
{
    if(pixmaps_created)
        return;

    KPixmap tempPixmap;
    QPainter painter;

    const int titleHeight = layoutMetric(LM_TitleHeight);

    // aTitleBarTile
    tempPixmap.resize(1, titleHeight + TOPMARGIN + DECOHEIGHT);
    KPixmapEffect::gradient(tempPixmap,
                            Handler()->getColor(TitleGradientFrom, true),
                            Handler()->getColor(TitleGradientTo, true),
                            KPixmapEffect::VerticalGradient);
    aTitleBarTile = new QPixmap(1, titleHeight + TOPMARGIN + DECOHEIGHT);
    painter.begin(aTitleBarTile);
    painter.drawPixmap(0, 0, tempPixmap);

    QImage t;
    if (Handler()->titlebarStyle() == 0) { // new, Toplight
        t = QImage(1, (titleHeight + TOPMARGIN + DECOHEIGHT)/3 + 1, 32 );
        t = KImageEffect::gradient(QSize(1, t.height()),
                                Handler()->getColor(TitleGradientFrom, true).light(130),
                                Handler()->getColor(TitleGradientTo, true),
                                KImageEffect::VerticalGradient/*, 100, 100*/);
        painter.drawImage(0, 2, t, 0, 0, -1, tempPixmap.height()-2);
        t.create(t.width(), t.height()*2, t.depth());
        t = KImageEffect::unbalancedGradient(QSize(1, t.height()),
                                Handler()->getColor(TitleGradientTo, true),
                                Handler()->getColor(TitleGradientFrom, true),
                                KImageEffect::VerticalGradient, 100, 100);
        painter.drawImage(0, t.height()/2, t, 0, 0, -1, t.height());
    } else { // older, Balanced
        t = QImage(1, (titleHeight + TOPMARGIN + DECOHEIGHT)/2 + 1, 32 );
        t = KImageEffect::gradient(QSize(1, t.height()),
                                Handler()->getColor(TitleGradientFrom, true).light(150),
                                Handler()->getColor(TitleGradientTo, true).light(110),
                                KImageEffect::VerticalGradient);
        painter.drawImage(0, 2, t, 0, 0, -1, tempPixmap.height()-2);
        t = KImageEffect::gradient(QSize(1, t.height()),
                                Handler()->getColor(TitleGradientTo, true),
                                Handler()->getColor(TitleGradientFrom, true),
                                KImageEffect::VerticalGradient);
        painter.drawImage(0, t.height(), t, 0, 0, -1, t.height());
    }

    painter.end();

    // iTitleBarTile
    tempPixmap.resize(1, titleHeight + TOPMARGIN + DECOHEIGHT);
    KPixmapEffect::gradient(tempPixmap,
                            Handler()->getColor(TitleGradientFrom, false),
                            Handler()->getColor(TitleGradientTo, false),
                            KPixmapEffect::VerticalGradient);
    iTitleBarTile = new QPixmap(1, titleHeight + TOPMARGIN + DECOHEIGHT);
    painter.begin(iTitleBarTile);
    painter.drawPixmap(0, 0, tempPixmap);
    painter.end();

    QImage aTempImage = aTitleBarTile->convertToImage();
    aGradientBottom = QColor::QColor(aTempImage.pixel(0, aTempImage.height()-1));
    aAntialiasBase = QColor::QColor(aTempImage.pixel(0, 2));
    QImage iTempImage = iTitleBarTile->convertToImage();
    iGradientBottom = QColor::QColor(iTempImage.pixel(0, iTempImage.height()-1));
    iAntialiasBase = QColor::QColor(iTempImage.pixel(0, 2));

    pixmaps_created = true;
}

void SUSE2Client::delete_pixmaps()
{
    delete aTitleBarTile;
    aTitleBarTile = 0;

    delete iTitleBarTile;
    iTitleBarTile = 0;

    pixmaps_created = false;
}

void SUSE2Client::update_captionBuffer()
{
    const uint maxCaptionLength = 110; // truncate captions longer than this!
    QString c(caption());
    if (c.length() > maxCaptionLength) {
        QString tmpLeft = c.left(50);
        QString tmpRight = c.right(50);
        c = tmpLeft + QString::fromUtf8("[...]") + tmpRight;
    }

    QImage logo(Handler()->titleLogoURL());
    int logoOffset = Handler()->titleLogoOffset();
    QFontMetrics fm(s_titleFont);
    int captionWidth  = fm.width(c);
    int logoWidth = 0;

    if (Handler()->titleLogo()) {
        logoWidth = logo.width() + logoOffset;
        captionWidth += logoWidth;
        if (logo.height()+1 > fm.height())
            logo.scaleHeight(fm.height()-1);
    }

    const int titleEdgeTop = layoutMetric(LM_TitleEdgeTop);
    const int titleEdgeBottom = layoutMetric(LM_TitleEdgeBottom);
    const int titleHeight = layoutMetric(LM_TitleHeight);
    QPixmap textPixmap;
    QPainter painter;
    if(Handler()->titleShadow()) {
        // prepare the shadow
        textPixmap = QPixmap(captionWidth+4, titleHeight + TOPMARGIN + DECOHEIGHT); // 4 px shadow space
        textPixmap.fill(QColor(0,0,0));
        textPixmap.setMask(textPixmap.createHeuristicMask(true));
        painter.begin(&textPixmap);
        painter.setFont(s_titleFont);
        painter.setPen(white);
        if (Handler()->titleLogo()) {
            painter.drawText(textPixmap.rect().left(), textPixmap.rect().top()+titleEdgeTop,
                             textPixmap.rect().width()-logo.width() - logoOffset, textPixmap.rect().height()-titleEdgeTop-titleEdgeBottom,
                             AlignCenter, c);
            painter.drawImage(captionWidth - logo.width(), textPixmap.rect().top() + TOPMARGIN, logo);
        } else {
            painter.drawText(textPixmap.rect().left(), textPixmap.rect().top()+titleEdgeTop,
                             textPixmap.rect().width(), textPixmap.rect().height()-titleEdgeTop-titleEdgeBottom,
                             AlignCenter, c);
        }
        painter.end();
    }

    QImage shadow;
    ShadowEngine se;

    // active
    aCaptionBuffer->resize(captionWidth+4, titleHeight + TOPMARGIN + DECOHEIGHT); // 4 px shadow
    painter.begin(aCaptionBuffer);
    painter.drawTiledPixmap(aCaptionBuffer->rect(), *aTitleBarTile);
    if(Handler()->titleShadow()) {
        shadow = se.makeShadow(textPixmap, QColor(0, 0, 0));
        painter.drawImage(1, 1, shadow);
    }
    painter.setFont(s_titleFont);
    painter.setPen(Handler()->getColor(TitleFont,true));
    painter.drawText(aCaptionBuffer->rect().left(), aCaptionBuffer->rect().top() + titleEdgeTop,
            aCaptionBuffer->rect().width()-logoWidth, aCaptionBuffer->rect().height() - titleEdgeTop-titleEdgeBottom,
            AlignCenter, c);
    if (Handler()->titleLogo())
        painter.drawImage(captionWidth - logo.width(), aCaptionBuffer->rect().top() + TOPMARGIN, logo);
    painter.end();

    // inactive -> no shadow and no logo
    iCaptionBuffer->resize(captionWidth+4, titleHeight + TOPMARGIN + DECOHEIGHT);
    painter.begin(iCaptionBuffer);
    painter.drawTiledPixmap(iCaptionBuffer->rect(), *iTitleBarTile);
    painter.setFont(s_titleFont);
    painter.setPen(Handler()->getColor(TitleFont,false));
    painter.drawText(iCaptionBuffer->rect().left(), iCaptionBuffer->rect().top() + titleEdgeTop,
                     iCaptionBuffer->rect().width() - logoWidth, iCaptionBuffer->rect().height() - titleEdgeTop - titleEdgeBottom,
                     AlignCenter, c);
    painter.end();

    captionBufferDirty = false;
}

} // KWinSUSE2

// kate: space-indent on; indent-width 4; replace-tabs on;

