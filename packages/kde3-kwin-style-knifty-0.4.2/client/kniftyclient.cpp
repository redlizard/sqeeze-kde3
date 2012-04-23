/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <klocale.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kdecoration.h>
#include <kdecorationfactory.h>

#include <qbitmap.h>
#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtooltip.h>

#include "kniftyclient.h"
#include "kniftyclient.moc"
#include "kniftybutton.h"
#include "misc.h"
#include "shadow.h"

using namespace KNifty;

// global constants
static const int TOPMARGIN       = 2; // do not change
static const int DECOHEIGHT      = 2; // do not change
static const int SIDETITLEMARGIN = 8;

// Default button layout
const char default_left[]  = "M";
const char default_right[] = "HIAX";

KniftyClient::KniftyClient (KDecorationBridge *b, KDecorationFactory *f)
    : KDecoration (b, f),
    mainLayout_(0),
    topSpacer_(0),
    titleSpacer_(0),
    leftTitleSpacer_(0), rightTitleSpacer_(0),
    decoSpacer_(0),
    leftSpacer_(0), rightSpacer_(0),
    bottomSpacer_(0),
    aCaptionBuffer(0), iCaptionBuffer(0),
    aTitleBarTile(0), iTitleBarTile(0),
    pixmaps_created(false),
    captionBufferDirty(true),
    closing(false),
    s_titleHeight(0),
    s_titleFont(QFont() )
{ ; } // All initialization code goes in init()

bool KniftyClient::isTool() const
{
    const int SUPPORTED_WINDOW_TYPES_MASK =
        NET::NormalMask | NET::DesktopMask | NET::DockMask | NET::ToolbarMask |
        NET::MenuMask | NET::DialogMask | NET::OverrideMask | NET::UtilityMask |
        NET::SplashMask;
        NET::WindowType type = windowType (SUPPORTED_WINDOW_TYPES_MASK);
    return type == NET::Toolbar || type == NET::Utility || type == NET::Menu;
}

void KniftyClient::init(void)
{
    createMainWidget(WResizeNoErase | WRepaintNoErase);
    widget()->installEventFilter(this);

    // store settings which maybe need to be changed "locally" or needed often
    s_titleHeight = isTool()?KniftyHandler::titleHeightTool():KniftyHandler::titleHeight();
    s_titleFont = isTool()?KniftyHandler::titleFontTool():KniftyHandler::titleFont();

    // for flicker-free redraws
    widget()->setBackgroundMode(NoBackground);

    _resetLayout();

    create_pixmaps();

    aCaptionBuffer = new QPixmap();
    iCaptionBuffer = new QPixmap();
    captionBufferDirty = true;
    widget()->repaint(titleSpacer_->geometry(), false);

}

KniftyClient::~KniftyClient()
{
    delete_pixmaps();

    delete aCaptionBuffer;
    delete iCaptionBuffer;

    for (int n=0; n<ButtonTypeCount; n++) {
        if (m_button[n]) delete m_button[n];
    }
}

void KniftyClient::resizeEvent(QResizeEvent *)
{
    doShape();
    widget()->repaint();
}

bool KniftyClient::eventFilter(QObject *obj, QEvent *e)
{
    if (obj != widget()) return false;

    switch (e->type()) {
      case QEvent::MouseButtonDblClick: {
          mouseDoubleClickEvent(static_cast<QMouseEvent *>(e));
          return true;
      }
      case QEvent::MouseButtonPress: {
          processMousePressEvent(static_cast<QMouseEvent *>(e));
          return true;
      }
      case QEvent::Paint: {
          paintEvent(static_cast<QPaintEvent *>(e));
          return true;
      }
      case QEvent::Resize: {
          resizeEvent(static_cast<QResizeEvent *>(e));
          return true;
      }
      case QEvent::Show: {
          showEvent(static_cast<QShowEvent *>(e));
          return true;
      }
      default: {
          return false;
      }
    }

    return false;
}

void KniftyClient::paintEvent(QPaintEvent*)
{
    if (!KniftyHandler::initialized()) return;

    if (captionBufferDirty)
        update_captionBuffer();

    bool active = isActive();

    QPainter painter(widget());

    QRegion mask;

    QColor border3 = KniftyHandler::bordersBlendColor() ?
                     options()->colorGroup(KDecoration::ColorTitleBlend, active).background()
                     : widget()->colorGroup().background();
    QColor border1 = border3.dark(155); // dark
    QColor border2 = border3.dark(112); // shadowed
    QColor titleBorder3 = KniftyHandler::useTitleColor() ?
                          options()->colorGroup(KDecoration::ColorTitleBar, active).background()
                          : widget()->colorGroup().background();
    QColor titleBorder1 = titleBorder3.dark(155); // dark...
    QColor titleBorder4 = hsvRelative(titleBorder3, 0, 0, 70); // light
    QColor deco1   = options()->colorGroup(KDecoration::ColorTitleBlend, active).background();

    QRect topRect(topSpacer_->geometry());
    QRect titleRect(titleSpacer_->geometry());
    QRect leftTitleRect(leftTitleSpacer_->geometry());
    QRect rightTitleRect(rightTitleSpacer_->geometry());
    QRect decoRect(decoSpacer_->geometry());
    QRect leftRect(leftSpacer_->geometry());
    QRect rightRect(rightSpacer_->geometry());
    QRect bottomRect(bottomSpacer_->geometry());
    QRect tempRect;

    // topSpacer
    if(topRect.height() > 0)
    {
        painter.setPen(titleBorder1 );
        painter.drawLine(topRect.left()+4, topRect.top(),
                         topRect.right()-4, topRect.top() );
        painter.drawLine(topRect.left()+2, topRect.top()+1,
                         topRect.left()+2+2, topRect.top()+1 );
        painter.drawLine(topRect.right()-2-2, topRect.top()+1,
                         topRect.right()-2, topRect.top()+1 );
        painter.setPen(titleBorder4 );
        painter.drawLine(topRect.left()+4, topRect.top()+1,
                         topRect.right()-4, topRect.top()+1 );
    }

    // leftTitleSpacer
    if(leftTitleRect.width() > 0)
    {
        painter.setPen(titleBorder1 );
        painter.drawLine(leftTitleRect.left(), leftTitleRect.top()+2,
                         leftTitleRect.left(), leftTitleRect.bottom() );
        painter.drawLine(leftTitleRect.left()+1, leftTitleRect.top(),
                         leftTitleRect.left()+1, leftTitleRect.top()+2 );

        mask  = QRegion(leftTitleRect);
        mask -= QRegion(leftTitleRect.left(), leftTitleRect.top(), 1, leftTitleRect.height() );
        mask -= QRegion(leftTitleRect.left()+1, leftTitleRect.top(), 1, 2 );
        painter.setClipRegion(mask);
        painter.setClipping(true);
        painter.drawTiledPixmap(leftTitleRect, active ? *aTitleBarTile : *iTitleBarTile );
        painter.setClipping(false);
    }

    // Space under the left button group
    painter.drawTiledPixmap(leftTitleRect.right()+1, titleRect.top(),
                            (titleRect.left()-1)-leftTitleRect.right(), titleRect.height(),
                            active ? *aTitleBarTile : *iTitleBarTile );


    // rightTitleSpacer
    if(rightTitleRect.width() > 0)
    {
        painter.setPen(titleBorder1 );
        painter.drawLine(rightTitleRect.right(), rightTitleRect.top()+2,
                         rightTitleRect.right(), rightTitleRect.bottom() );
        painter.drawLine(rightTitleRect.right()-1, rightTitleRect.top(),
                         rightTitleRect.right()-1, rightTitleRect.top()+2 );

        mask  = QRegion(rightTitleRect);
        mask -= QRegion(rightTitleRect.right(), rightTitleRect.top(), 1, leftTitleRect.height() );
        mask -= QRegion(rightTitleRect.right()-1, rightTitleRect.top(), 1, 2 );
        painter.setClipRegion(mask);
        painter.setClipping(true);
        painter.drawTiledPixmap(rightTitleRect, active ? *aTitleBarTile : *iTitleBarTile );
        painter.setClipping(false);
    }

    // Space under the right button group
    painter.drawTiledPixmap(titleRect.right()+1, titleRect.top(),
                            (rightTitleRect.left()-1)-titleRect.right(), titleRect.height(),
                            active ? *aTitleBarTile : *iTitleBarTile );

    // titleSpacer
    QPixmap *titleBfrPtr = active ? aCaptionBuffer : iCaptionBuffer;
    if(titleRect.width() > 0 && titleBfrPtr != 0)
    {
        const int titleMargin = 5; // 5 px betwee title and buttons

        int tX, tW;
        switch (KniftyHandler::titleAlign())
        {
        // AlignCenter
        case Qt::AlignHCenter:
            tX = (titleBfrPtr->width() > titleRect.width()-2*titleMargin) ?
                    (titleRect.left()+titleMargin)
                    : titleRect.left()+(titleRect.width()- titleBfrPtr->width() )/2;
            tW = (titleBfrPtr->width() > titleRect.width()-2*titleMargin) ?
                    (titleRect.width()-2*titleMargin)
                    : titleBfrPtr->width();
            break;
        // AlignRight
        case Qt::AlignRight:
            tX = (titleBfrPtr->width() > titleRect.width()-2*titleMargin) ?
                    (titleRect.left()+titleMargin)
                    : titleRect.right()-titleMargin-titleBfrPtr->width();
            tW = (titleBfrPtr->width() > titleRect.width()-2*titleMargin) ?
                    (titleRect.width()-2*titleMargin)
                    : titleBfrPtr->width();
            break;
        // AlignLeft
        default:
            tX = (titleRect.left()+titleMargin);
            tW = (titleBfrPtr->width() > titleRect.width()-2*titleMargin) ?
                    (titleRect.width()-2*titleMargin)
                    : titleBfrPtr->width();
        }

        if(tW > 0)
        {
            painter.drawTiledPixmap(tX, titleRect.top(),
                                    tW, titleRect.height(),
                                    *titleBfrPtr);
        }

        painter.drawTiledPixmap(titleRect.left(), titleRect.top(),
                                tX-titleRect.left(), titleRect.height(),
                                active ? *aTitleBarTile : *iTitleBarTile);

        painter.drawTiledPixmap(tX+tW, titleRect.top(),
                                titleRect.right()-(tX+tW)+1, titleRect.height(),
                                active ? *aTitleBarTile : *iTitleBarTile);
    }
    titleBfrPtr = 0;

    // decoSpacer
    if(decoRect.height() > 0)
    {
        painter.setPen(titleBorder1 );
        painter.drawLine(decoRect.left(), decoRect.top(), decoRect.left(), decoRect.bottom() );
        painter.drawLine(decoRect.right(), decoRect.top(), decoRect.right(), decoRect.bottom() );
        painter.setPen(deco1 );
        painter.drawLine(decoRect.left()+1, decoRect.bottom(), decoRect.right()-1, decoRect.bottom() );
//         if(!KniftyHandler::useTitleColor() && !KniftyHandler::bordersBlendColor())
//         {
            painter.setPen(hsvRelative(deco1, 0, -10, +20) );
//         }
        painter.drawLine(decoRect.left()+1, decoRect.top(), decoRect.right()-1, decoRect.top() );
    }

    if(leftRect.height() == 1 && rightRect.height() == 1) // the window is shaded... hopefully
    {
        // let's orient on the bottomSpacer which should have the prper x coords...
        painter.setPen(border1 );
        painter.drawPoint(bottomRect.left(), leftRect.top() );
        painter.drawPoint(bottomRect.right(), rightRect.top() );
        painter.setPen(border3 );
        painter.drawLine(bottomRect.left()+1, leftRect.top(), bottomRect.right()-1, rightRect.top() );

    }
    else
    {
        // leftSpacer
        if(leftRect.width() > 0 && leftRect.height() > 0)
        {
            painter.setPen(border1 );
            painter.drawLine(leftRect.left(), leftRect.top(),
                             leftRect.left(), leftRect.bottom() );
            tempRect.setCoords(leftRect.left()+1, leftRect.top(),
                               leftRect.right(), leftRect.bottom() );
            painter.fillRect(tempRect, border3 );
        }

        // rightSpacer
        if(rightRect.width() > 0 && rightRect.height() > 0)
        {
            painter.setPen(border1 );
            painter.drawLine(rightRect.right(), rightRect.top(),
                             rightRect.right(), rightRect.bottom() );
            tempRect.setCoords(rightRect.left(), rightRect.top(),
                               rightRect.right()-1, rightRect.bottom() );
            painter.fillRect(tempRect, border3 );
        }
    }

    // bottomSpacer
    if(bottomRect.height() > 0)
    {
        painter.setPen(border1 );
        painter.drawLine(bottomRect.left()+1, bottomRect.bottom(),
                         bottomRect.right()-1, bottomRect.bottom() );
        painter.drawLine(bottomRect.left(), bottomRect.top(),
                         bottomRect.left(), bottomRect.bottom()-1 );
        painter.drawLine(bottomRect.right(), bottomRect.top(),
                         bottomRect.right(), bottomRect.bottom()-1 );

        painter.setPen(border2 );
        painter.drawLine(bottomRect.left()+1, bottomRect.bottom()-1,
                         bottomRect.right()-1, bottomRect.bottom()-1 );

        tempRect.setCoords(bottomRect.left()+1, bottomRect.top(),
                           bottomRect.right()-1, bottomRect.bottom()-2);
        painter.fillRect(tempRect, border3 );
    }
}

void KniftyClient::showEvent(QShowEvent *)
{
    doShape();
    widget()->repaint();
}

void KniftyClient::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (titleSpacer_->geometry().contains(e->pos()))
        titlebarDblClickOperation();
}

void KniftyClient::doShape()
{
    QRegion mask(0, 0, width(), height());

    int r(width());
    int b(height());

    if(topSpacer_->geometry().height() > 0)
    {
        // Remove top-left corner.
        if(leftTitleSpacer_->geometry().width() > 0)
        {
            mask -= QRegion(0, 0, 1, 4);
            mask -= QRegion(1, 0, 3, 1);
            mask -= QRegion(1, 1, 1, 1);
        }
        // Remove top-right corner.
        if(rightTitleSpacer_->geometry().width() > 0)
        {
            mask -= QRegion(r-1, 0, 1, 4);
            mask -= QRegion(r-4, 0, 3, 1);
            mask -= QRegion(r-2, 1, 1, 1);
        }
    }

    // Remove bottom-left corner and bottom-right corner.
    if(bottomSpacer_->geometry().height() > 0)
    {
        mask -= QRegion(0, b-1, 1, 1);
        mask -= QRegion(r-1, b-1, 1, 1);
    }

    setMask(mask);
}

void KniftyClient::_resetLayout()
{
    // basic layout:
    //  _______________________________________________________________
    // |                         topSpacer                             |
    // |_______________________________________________________________|
    // | leftTitleSpacer | btns | titleSpacer | bts | rightTitleSpacer |
    // |_________________|______|_____________|_____|__________________|
    // |                         decoSpacer                            |
    // |_______________________________________________________________|
    // | |                                                           | |
    // | |                      windowWrapper                        | |
    // | |                                                           | |
    // |leftSpacer                                          rightSpacer|
    // |_|___________________________________________________________|_|
    // |                           bottomSpacer                        |
    // |_______________________________________________________________|
    //

    if (!KniftyHandler::initialized()) return;

    delete mainLayout_;

    delete topSpacer_;
    delete titleSpacer_;
    delete leftTitleSpacer_;
    delete rightTitleSpacer_;
    delete decoSpacer_;
    delete leftSpacer_;
    delete rightSpacer_;
    delete bottomSpacer_;

    mainLayout_ = new QVBoxLayout(widget(), 0, 0);

    topSpacer_        = new QSpacerItem(1, TOPMARGIN, QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleSpacer_      = new QSpacerItem(1, s_titleHeight,
                                        QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftTitleSpacer_  = new QSpacerItem(SIDETITLEMARGIN, s_titleHeight,
                                        QSizePolicy::Fixed, QSizePolicy::Fixed);
    rightTitleSpacer_ = new QSpacerItem(SIDETITLEMARGIN, s_titleHeight,
                                        QSizePolicy::Fixed, QSizePolicy::Fixed);
    decoSpacer_       = new QSpacerItem(1, DECOHEIGHT, QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftSpacer_       = new QSpacerItem(KniftyHandler::borderSize(), 1,
                                        QSizePolicy::Fixed, QSizePolicy::Expanding);
    rightSpacer_      = new QSpacerItem(KniftyHandler::borderSize(), 1,
                                        QSizePolicy::Fixed, QSizePolicy::Expanding);
    bottomSpacer_     = new QSpacerItem(1, KniftyHandler::borderSize(),
                                        QSizePolicy::Expanding, QSizePolicy::Fixed);

    // top
    mainLayout_->addItem(topSpacer_);

    // title
    QHBoxLayout *titleLayout_ = new QHBoxLayout(mainLayout_, 0, 0);

    // sizeof(...) is calculated at compile time
    memset(m_button, 0, sizeof(KniftyButton *) * ButtonTypeCount);

    titleLayout_->addItem(leftTitleSpacer_);
    addButtons(titleLayout_,
               options()->customButtonPositions() ? options()->titleButtonsLeft() : QString(default_left),
               s_titleHeight-1);
    titleLayout_->addItem(titleSpacer_);
    addButtons(titleLayout_,
               options()->customButtonPositions() ? options()->titleButtonsRight() : QString(default_right),
               s_titleHeight-1);
    titleLayout_->addItem(rightTitleSpacer_);

    // deco
    mainLayout_->addItem(decoSpacer_);

    //Mid
    QHBoxLayout * midLayout   = new QHBoxLayout(mainLayout_, 0, 0);
    midLayout->addItem(leftSpacer_);
    if( isPreview())
        midLayout->addWidget(new QLabel( i18n( "<center><b>Knifty preview</b></center>" ), widget()) );
    else
        midLayout->addItem( new QSpacerItem( 0, 0 ));
    midLayout->addItem(rightSpacer_);

    //Bottom
    mainLayout_->addItem(bottomSpacer_);


}

void KniftyClient::addButtons(QBoxLayout *layout, const QString& s, int buttonSize)
{
    if (s.length() > 0) {
        for (unsigned n=0; n < s.length(); n++) {
            switch (s[n]) {
              case 'M': // Menu button
                  if (!m_button[ButtonMenu]){
                      m_button[ButtonMenu] = new KniftyButton(this, "menu", i18n("Menu"), ButtonMenu, buttonSize, LeftButton|RightButton);
                      connect(m_button[ButtonMenu], SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
                      connect(m_button[ButtonMenu], SIGNAL(released()), this, SLOT(menuButtonReleased()));
                      layout->addWidget(m_button[ButtonMenu], 0, Qt::AlignHCenter | Qt::AlignTop);
                  }
                  break;
              case 'S': // On All Desktops button
                  if (!m_button[ButtonOnAllDesktops]){
                      const bool oad = isOnAllDesktops();
                      m_button[ButtonOnAllDesktops] = new KniftyButton(this, "on_all_desktops", oad?i18n("Not on all desktops"):i18n("On all desktops"), ButtonOnAllDesktops, buttonSize);
                      m_button[ButtonOnAllDesktops]->setSticky( oad );
                      connect(m_button[ButtonOnAllDesktops], SIGNAL(clicked()), this, SLOT(toggleOnAllDesktops()));
                      layout->addWidget(m_button[ButtonOnAllDesktops], 0, Qt::AlignHCenter | Qt::AlignTop);
                  }
                  break;
              case 'H': // Help button
                  if ((!m_button[ButtonHelp]) && providesContextHelp()){
                      m_button[ButtonHelp] = new KniftyButton(this, "help", i18n("Help"), ButtonHelp, buttonSize);
                      connect(m_button[ButtonHelp], SIGNAL(clicked()), this, SLOT(showContextHelp()));
                      layout->addWidget(m_button[ButtonHelp], 0, Qt::AlignHCenter | Qt::AlignTop);
                  }
                  break;
              case 'I': // Minimize button
                  if ((!m_button[ButtonMin]) && isMinimizable()){
                      m_button[ButtonMin] = new KniftyButton(this, "iconify", i18n("Minimize"), ButtonMin, buttonSize);
                      connect(m_button[ButtonMin], SIGNAL(clicked()), this, SLOT(minimize()));
                      layout->addWidget(m_button[ButtonMin], 0, Qt::AlignHCenter | Qt::AlignTop);
                  }
                  break;
              case 'A': // Maximize button
                  if ((!m_button[ButtonMax]) && isMaximizable()){
                      const bool max = maximizeMode()!=MaximizeRestore;
                      m_button[ButtonMax] = new KniftyButton(this, "maximize", max?i18n("Restore"):i18n("Maximize"), ButtonMax, buttonSize, LeftButton|MidButton|RightButton);
                      m_button[ButtonMax]->setMaximized( max );

                      connect(m_button[ButtonMax], SIGNAL(clicked()), this, SLOT(maxButtonPressed()));
                      layout->addWidget(m_button[ButtonMax], 0, Qt::AlignHCenter | Qt::AlignTop);
                  }
                  break;
              case 'X': // Close button
                  if ((!m_button[ButtonClose]) && isCloseable()){
                      m_button[ButtonClose] = new KniftyButton(this, "close", i18n("Close"), ButtonClose, buttonSize);
                      connect(m_button[ButtonClose], SIGNAL(clicked()), this, SLOT(closeWindow()));
                      layout->addWidget(m_button[ButtonClose], 0, Qt::AlignHCenter | Qt::AlignTop);
                  }
                  break;
              case '_': // Spacer item
                  layout->addSpacing(3); // add a 3 px spacing...
            }
	}
    }
}

void KniftyClient::captionChange(void)
{
    captionBufferDirty = true;
    widget()->repaint(titleSpacer_->geometry(), false);
}

KDecoration::Position KniftyClient::mousePosition(const QPoint &point) const
{
    const int corner = 24;
    Position pos = PositionCenter;

    // often needed coords..
    QRect topRect(topSpacer_->geometry());
    QRect decoRect(decoSpacer_->geometry());
    QRect leftRect(leftSpacer_->geometry());
    QRect leftTitleRect(leftTitleSpacer_->geometry());
    QRect rightRect(rightSpacer_->geometry());
    QRect rightTitleRect(rightTitleSpacer_->geometry());
    QRect bottomRect(bottomSpacer_->geometry());

    if(bottomRect.contains(point)) {
        if      (point.x() <= bottomRect.left()+corner)  pos = PositionBottomLeft;
        else if (point.x() >= bottomRect.right()-corner) pos = PositionBottomRight;
        else                                             pos = PositionBottom;
    }
    else if(leftRect.contains(point)) {
        if      (point.y() <= topRect.top()+corner)       pos = PositionTopLeft;
        else if (point.y() >= bottomRect.bottom()-corner) pos = PositionBottomLeft;
        else                                              pos = PositionLeft;
    }
    else if(leftTitleRect.contains(point)) {
        if      (point.y() <= topRect.top()+corner)       pos = PositionTopLeft;
        else                                              pos = PositionLeft;
    }
    else if(rightRect.contains(point)) {
        if      (point.y() <= topRect.top()+corner)       pos = PositionTopRight;
        else if (point.y() >= bottomRect.bottom()-corner) pos = PositionBottomRight;
        else                                              pos = PositionRight;
    }
    else if(rightTitleRect.contains(point)) {
        if      (point.y() <= topRect.top()+corner)       pos = PositionTopRight;
        else                                              pos = PositionRight;
    }
    else if(topRect.contains(point)) {
        if      (point.x() <= topRect.left()+corner)      pos = PositionTopLeft;
        else if (point.x() >= topRect.right()-corner)     pos = PositionTopRight;
        else                                              pos = PositionTop;
    }
    else if(decoRect.contains(point)) {
        if(point.x() <= leftTitleRect.right()) {
            if(point.y() <= topRect.top()+corner)         pos = PositionTopLeft;
            else                                          pos = PositionLeft;
        }
        else if(point.x() >= rightTitleRect.left()) {
            if(point.y() <= topRect.top()+corner)         pos = PositionTopRight;
            else                                          pos = PositionRight;
        }
    }

    return pos;
}

void KniftyClient::iconChange()
{
    if (m_button[ButtonMenu])
    {
        m_button[ButtonMenu]->repaint(false);
    }
}

void KniftyClient::activeChange()
{
    for (int n=0; n<ButtonTypeCount; n++)
        if (m_button[n]) m_button[n]->repaint(false);
    widget()->repaint(false);
}

QSize KniftyClient::minimumSize() const
{
    return QSize (100, 50);
}

void KniftyClient::borders(int &l, int &r, int &t, int &b) const
{
    if ((maximizeMode()==MaximizeFull) && !options()->moveResizeMaximizedWindows()) {
        t = s_titleHeight + DECOHEIGHT;
        b = l = r = 0;
    } else {
        t = s_titleHeight + TOPMARGIN + DECOHEIGHT;
        b = l = r = KniftyHandler::borderSize();
    }
}

void KniftyClient::maximizeChange()
{
    if (!KniftyHandler::initialized()) return;

    const bool m = (maximizeMode() == MaximizeFull);

    if( m && (maximizeMode() == MaximizeFull) && !options()->moveResizeMaximizedWindows() )
    {
        topSpacer_->changeSize(1, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
        bottomSpacer_->changeSize(1, 3, QSizePolicy::Expanding, QSizePolicy::Fixed);
        leftSpacer_->changeSize(0, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
        rightSpacer_->changeSize(0, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);

        leftTitleSpacer_->changeSize(0, s_titleHeight,
                    QSizePolicy::Fixed, QSizePolicy::Fixed);
        rightTitleSpacer_->changeSize(0, s_titleHeight,
                    QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    else
    {
        topSpacer_->changeSize(1, TOPMARGIN, QSizePolicy::Expanding, QSizePolicy::Fixed);
        bottomSpacer_->changeSize(1, KniftyHandler::borderSize(),
                    QSizePolicy::Expanding, QSizePolicy::Fixed);
        leftSpacer_->changeSize(KniftyHandler::borderSize(), 1,
                    QSizePolicy::Fixed, QSizePolicy::Expanding);
        rightSpacer_->changeSize(KniftyHandler::borderSize(), 1,
                    QSizePolicy::Fixed, QSizePolicy::Expanding);
        leftTitleSpacer_->changeSize(SIDETITLEMARGIN, s_titleHeight,
                    QSizePolicy::Fixed, QSizePolicy::Fixed);
        rightTitleSpacer_->changeSize(SIDETITLEMARGIN, s_titleHeight,
                    QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    widget()->layout()->activate();
    widget()->repaint( false );

    doShape();
}

void KniftyClient::desktopChange()
{
    bool s = isOnAllDesktops();
    if (m_button[ButtonOnAllDesktops])
    {
        m_button[ButtonOnAllDesktops]->update();
        m_button[ButtonOnAllDesktops]->setSticky(s);
        QToolTip::add(m_button[ButtonOnAllDesktops], s ? i18n("Not on all desktops") : i18n("On all desktops"));
    }
}

void KniftyClient::maxButtonPressed()
{
    if (m_button[ButtonMax]) {
        switch (m_button[ButtonMax]->lastMousePress())
        {
          case MidButton:
              maximize(maximizeMode() ^ MaximizeVertical );
              maximizeChange();
              break;
          case RightButton:
              maximize(maximizeMode() ^ MaximizeHorizontal );
              maximizeChange();
              break;
          default:
              maximize(maximizeMode() == MaximizeFull ? MaximizeRestore : MaximizeFull );
              maximizeChange();
        }

        m_button[ButtonMax]->setMaximized( maximizeMode()!=MaximizeRestore );
        QToolTip::add(m_button[ButtonMax], (maximizeMode()!=MaximizeRestore) ? i18n("Restore") : i18n("Maximize"));
    }
}

void KniftyClient::menuButtonPressed()
{
    static QTime* t = NULL;
    static KniftyClient* lastClient = NULL;
    if (t == NULL)
        t = new QTime;
    bool dbl = (lastClient == this && t->elapsed() <= QApplication::doubleClickInterval());
    lastClient = this;
    t->start();
    if( !dbl )
    {
        QPoint pos = m_button[ButtonMenu]->mapToGlobal(
            m_button[ButtonMenu]->rect().bottomLeft() );
        KDecorationFactory* f = factory();
        showWindowMenu( pos );
        if( !f->exists( this )) // 'this' was deleted
            return;
        m_button[ButtonMenu]->setDown(false);
    }
    else
       closing = true;
}

void KniftyClient::menuButtonReleased()
{
    if(closing)
        closeWindow();
}

void KniftyClient::create_pixmaps()
{
    if(pixmaps_created)
        return;

    QColor titleBorder3 = KniftyHandler::useTitleColor() ?
                          options()->colorGroup(KDecoration::ColorTitleBar, true).background()
                          : widget()->colorGroup().background();
    KPixmap tempPixmap;
    tempPixmap.resize(1, s_titleHeight );
    KPixmapEffect::gradient(tempPixmap,
                            hsvRelative(titleBorder3, 0, 0, 30),
                            hsvRelative(titleBorder3, 0, 0, -10),
                            KPixmapEffect::VerticalGradient);
    aTitleBarTile = new QPixmap(1, s_titleHeight );
    QPainter painter;
    painter.begin(aTitleBarTile);
    painter.drawPixmap(0, 0, tempPixmap);
    painter.end();

    titleBorder3 = KniftyHandler::useTitleColor() ?
                   options()->colorGroup(KDecoration::ColorTitleBar, false).background()
                   : widget()->colorGroup().background();
    tempPixmap.resize(1, s_titleHeight );
    KPixmapEffect::gradient(tempPixmap,
                            hsvRelative(titleBorder3, 0, 0, 5),
                            hsvRelative(titleBorder3, 0, 0, -10),
                            KPixmapEffect::VerticalGradient);
    iTitleBarTile = new QPixmap(1, s_titleHeight );
    painter.begin(iTitleBarTile);
    painter.drawPixmap(0, 0, tempPixmap);
    painter.end();

    pixmaps_created = true;
}

void KniftyClient::delete_pixmaps()
{
    delete aTitleBarTile;
    aTitleBarTile = 0;

    delete iTitleBarTile;
    iTitleBarTile = 0;

    pixmaps_created = false;
}

void KniftyClient::update_captionBuffer()
{
    if (!KniftyHandler::initialized()) return;

    const uint maxCaptionLength = 300; // truncate captions longer than this!
    QString captionText(caption() );
    if (captionText.length() > maxCaptionLength) {
        captionText.truncate(maxCaptionLength);
        captionText.append(" [...]");
    }

    QFontMetrics fm(s_titleFont);
    int captionWidth  = fm.width(captionText);

    QPixmap textPixmap;
    QPainter painter;
    if(KniftyHandler::titleShadow())
    {
        // prepare the shadow
        textPixmap = QPixmap(captionWidth+2*2, s_titleHeight ); // 2*2 px shadow space
        textPixmap.fill(QColor(0,0,0));
        textPixmap.setMask( textPixmap.createHeuristicMask(TRUE) );
        painter.begin(&textPixmap);
        painter.setFont(s_titleFont);
        painter.setPen(white);
        painter.drawText(textPixmap.rect(), AlignCenter, captionText );
        painter.end();
    }

    QImage shadow;
    ShadowEngine se;

    // active
    aCaptionBuffer->resize(captionWidth+4, s_titleHeight ); // 4 px shadow
    painter.begin(aCaptionBuffer);
    painter.drawTiledPixmap(aCaptionBuffer->rect(), *aTitleBarTile);
    if(KniftyHandler::titleShadow())
    {
        shadow = se.makeShadow(textPixmap, QColor(0, 0, 0));
        painter.drawImage(1, 1, shadow);
    }
    painter.setFont(s_titleFont);
    painter.setPen(options()->color(KDecoration::ColorFont, true));
    painter.drawText(aCaptionBuffer->rect(), AlignCenter, captionText );
    painter.end();


    // inactive
    iCaptionBuffer->resize(captionWidth+4, s_titleHeight );
    painter.begin(iCaptionBuffer);
    painter.drawTiledPixmap(iCaptionBuffer->rect(), *iTitleBarTile);
    if(KniftyHandler::titleShadow())
    {
        painter.drawImage(1, 1, shadow);
    }
    painter.setFont(s_titleFont);
    painter.setPen(options()->color(KDecoration::ColorFont, false));
    painter.drawText(iCaptionBuffer->rect(), AlignCenter, captionText );
    painter.end();

    captionBufferDirty = false;
}

void KniftyClient::resize (const QSize &size)
{
    widget()->resize(size);
}

void KniftyClient::shadeChange ()
{
    ; /* XXX: Needs reimplemented */
}
