/***************************************************************************
 *   Copyright (C) 2004 by Jens Luetkens                                   *
 *   j.luetkens@limitland.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <math.h>

#include <qcursor.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qimage.h>
#include <qtooltip.h>
#include <qsettings.h>
#include <qimage.h>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <klocale.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>

#include <kdecoration.h>
#include <kdecorationfactory.h>

#include "comixClient.h"
#include "comixButton.h"

using namespace COMIX;

// Default button layout
const char default_left[]  = "M";
const char default_right[] = "HIAX";

//
// Constructor
//
ComixClient::ComixClient( KDecorationBridge *b, KDecorationFactory *f )
	    : KDecoration( b, f ),
		mainLayout_(0),
		clientHandler((ComixHandler *)f)
{
}

//
// Destructor
//
ComixClient::~ComixClient()
{

    deletePixmaps();

    for (int n=0; n<ButtonCount; n++) {
        if (m_buttons[n]) delete m_buttons[n];
    }

}

//
// init(void)
//
void ComixClient::init( void )
{
    createMainWidget(WResizeNoErase | WRepaintNoErase);
    
	widget()->installEventFilter(this);

    widget()->setBackgroundMode(NoBackground);
	
	readConfig();

    createLayout();

    createPixmaps();

	titleGeometry( false );

}

//
// readConfig()
//
bool ComixClient::readConfig()
{

    // create a config object
    KConfig config("kwincomixrc");
    config.setGroup("General");

    QString value = config.readEntry("TitleAlignment", "AlignLeft");
    if (value == "AlignLeft")         m_titleAlign = Qt::AlignLeft;
    else if (value == "AlignHCenter") m_titleAlign = Qt::AlignHCenter;
    else if (value == "AlignRight")   m_titleAlign = Qt::AlignRight;

    m_titleExpand = config.readBoolEntry("TitleExpand", false);

    m_windowSeparator = config.readBoolEntry("WindowSeparator", false);

	// contour color
	QSettings settings;
	m_contrast 				= settings.readNumEntry("/Qt/KDE/contrast", 4);

	aWindowBackgroundColor 	= options()->color(KDecoration::ColorHandle, true);
	iWindowBackgroundColor 	= options()->color(KDecoration::ColorHandle, false);
	aWindowContourColor 	= options()->color(KDecoration::ColorFrame, true);
	iWindowContourColor 	= options()->color(KDecoration::ColorFrame, false);

	aTitleBarColor 			= options()->color(KDecoration::ColorTitleBar, true);
	iTitleBlendColor 		= options()->color(KDecoration::ColorTitleBar, false);
	aTitleBarColor 			= options()->color(KDecoration::ColorTitleBlend, true);
	iTitleBlendColor 		= options()->color(KDecoration::ColorTitleBlend, false);

	someGeometry();

    return true;
}

//
// createPixmaps()
//
void ComixClient::createPixmaps()
{
	if( hasMenu ) {
		menuIcon = icon().pixmap(QIconSet::Small, QIconSet::Normal);
		menuIcon.convertFromImage( menuIcon.convertToImage().smoothScale(iconHeight, iconHeight));
	}
	
	activeCaption = new QPixmap(0,0);
	inactiveCaption = new QPixmap(0,0);
	
}

//
// deletePixmaps()
//
void ComixClient::deletePixmaps()
{
	if( activeCaption ) delete activeCaption;
	if( inactiveCaption ) delete inactiveCaption;
}

//
// eventFilter( QObject *obj, QEvent *e )
//
bool ComixClient::eventFilter( QObject *obj, QEvent *e )
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

//
// paintEvent(QPaintEvent *)
//
void ComixClient::paintEvent(QPaintEvent *)
{
	QPainter p(widget());
	
	QRect windowRect(0,0,width(),height());
	QRect captionRect(titleSpacer_->geometry());

	// frame
	if ( isShade() ) {
		windowRect.setHeight(boxHeight + borderSize + borderSize);
	}	
	setMask(windowRect);
	frameWindowRect(windowRect, &p);

	// top background
	insetRect(&windowRect, borderSize);
	if( isShade() ) windowRect.setHeight(boxHeight);
	else if( m_windowSeparator ) windowRect.setHeight(boxHeight+borderSize+borderSize);
	else windowRect.setHeight(boxHeight+borderSize);
	
	p.fillRect(windowRect, isActive() ? aWindowBackgroundColor : iWindowBackgroundColor);
	
	// optional separator
	if( m_windowSeparator && !isShade() ) {
		p.setPen(isActive() ? aWindowContourColor : iWindowContourColor);
		int offset = windowRect.bottom()-borderSize-penWidth;
		int i = 1;
		while( i <= penWidth ) {
			p.drawLine(penWidth,offset+i,width()-penWidth,offset+i);
			i++;
		}
	}

	// caption alignment
	int maxTitleWidth = captionRect.width() - borderSize;
	int maxMoveBy = maxTitleWidth-titleRect.width();
	int centerMoveBy = (windowRect.width() - titleRect.width() - windowRect.left() - titleRect.left()) / 2;
	
	QRect alignRect = titleRect;

	switch( m_titleAlign ) {
        case Qt::AlignHCenter:
			alignRect.moveBy(QMIN(maxMoveBy,centerMoveBy),0);
			break;
        case Qt::AlignRight:
			alignRect.moveBy(maxMoveBy,0);
			break;
		default:
			break;
	}

	p.drawPixmap(alignRect.left(),alignRect.top(),isActive() ? *activeCaption : *inactiveCaption );

}

//
// drawButton( QRect rect, QPainter * p, bool sunken )
//
void ComixClient::drawButton( QRect rect, QPainter * p, bool sunken )
{
	const QPixmap *button;
	
	if( isActive() ) {
		if( sunken )
			button = clientHandler->getPixmap( SunkenButton );
		else 
			button = clientHandler->getPixmap( ActiveButton );
	}
	else {
		if( sunken )
			button = clientHandler->getPixmap( InactiveSunkenButton );
		else 
			button = clientHandler->getPixmap( InactiveButton );
	}
	
	p->drawPixmap(rect.left(),rect.top(),*button);

}

//
// frameWindowRect( QRect rect, QPainter * p 
//
void ComixClient::frameWindowRect( QRect rect, QPainter * p )
{

	p->setPen(isActive() ? aWindowContourColor : iWindowContourColor);

	int i = 0;
	while( i < (penWidth) ) {
		p->drawRect(rect);
		insetRect(&rect,1);
		i++;
	}
	
	p->setPen(isActive() ? aWindowBackgroundColor : iWindowBackgroundColor );

	i = 0;
	while( i < (borderSize - penWidth) ) {
		p->drawRect(rect);
		insetRect(&rect,1);
		i++;
	}
	

}

//
// insetRect( QRect * rect, int inset )
//
void ComixClient::insetRect( QRect * rect, int inset )
{

	rect->setWidth(rect->width()-inset-inset);
	rect->setHeight(rect->height()-inset-inset);
	rect->moveBy(inset,inset);

}

//
// mouseDoubleClickEvent(QMouseEvent * e)
//
void ComixClient::mouseDoubleClickEvent(QMouseEvent * e)
{
	if (iconRect.contains(e->pos())) {
		closeWindow();
    }
	else if (titleSpacer_->geometry().contains(e->pos()))
		titlebarDblClickOperation();
}

//
// activeChange()
//
void ComixClient::activeChange()
{
	widget()->repaint(false);
	repaintButtons();
}

//
// desktopChange()
//
void ComixClient::desktopChange()
{
    bool s = isOnAllDesktops();
    if (m_buttons[ButtonOnAllDesktops])
    {
        m_buttons[ButtonOnAllDesktops]->update();
        m_buttons[ButtonOnAllDesktops]->setOnAllDesktops(s);
        QToolTip::add(m_buttons[ButtonOnAllDesktops], s ? i18n("Not On All Desktops") : i18n("On All Desktops"));
    }
}


//
// maximizeChange()
//
void ComixClient::maximizeChange()
{
    widget()->repaint( false );
}

//
// iconChange()
//
void ComixClient::iconChange()
{
    if (hasMenu)
    {
		updateCaptionBuffer(titleRect.width(),titleRect.height());
	    widget()->repaint(titleSpacer_->geometry(), false);
    }
}

//
// shadeChange ()
//
void ComixClient::shadeChange ()
{
    if ( m_buttons[ButtonShade] ) {
        bool shaded = isSetShade();
        m_buttons[ButtonShade]->setOn( shaded );
        QToolTip::add(m_buttons[ButtonShade], shaded ? i18n("Unshade") : i18n("Shade"));
    }
}

//
// captionChange()
//
void ComixClient::captionChange()
{
    titleGeometry( true );
	widget()->repaint(titleSpacer_->geometry(), false);
}

//
// resize (const QSize &size)
//
void ComixClient::resize (const QSize &size)
{
    widget()->resize(size);
}

//
// borders(int &l, int &r, int &t, int &b) const
//
void ComixClient::borders(int &l, int &r, int &t, int &b) const
{

	t = boxHeight + borderSize + borderSize - penWidth;
	if( m_windowSeparator ) t += borderSize;
	b = l = r = borderSize;
	
}

//
// createButtons()
//
void ComixClient::createButtons(QBoxLayout *layout, const QString& buttons)
{
	for (unsigned int i = 0; i < buttons.length(); ++i)
	{
		switch (buttons[i].latin1())
		{
		case 'H': // Help
			if ((!m_buttons[ButtonHelp]) && providesContextHelp()){
				m_buttons[ButtonHelp] = new ComixButton(this, "help", i18n("Help"), ButtonHelp, false);
				connect(m_buttons[ButtonHelp], SIGNAL(clicked()), this, SLOT(showContextHelp()));
				layout->addWidget(m_buttons[ButtonHelp], 0,  Qt::AlignTop);
				layout->setStretchFactor(m_buttons[ButtonHelp], 0);
				m_buttons[ButtonHelp]->setFixedSize(boxHeight, boxHeight);
				countButtons++;
            }
			break;
		case 'M': // Menu
			hasMenu = true;
			break;
		case 'S': // On All Desktops (Sticky)
			m_buttons[ButtonOnAllDesktops] = new ComixButton(this, "onalldesktops", i18n("On All Desktops"), ButtonOnAllDesktops, true);
			connect(m_buttons[ButtonOnAllDesktops], SIGNAL(clicked()), this, SLOT(toggleOnAllDesktops()));
			layout->addWidget(m_buttons[ButtonOnAllDesktops], 0,  Qt::AlignTop);
			layout->setStretchFactor(m_buttons[ButtonOnAllDesktops], 0);
			m_buttons[ButtonOnAllDesktops]->setFixedSize(boxHeight, boxHeight);
			countButtons++;
			break;
		case 'I': // Minimize
			if (isMinimizable()){
				m_buttons[ButtonIconify] = new ComixButton(this, "iconify", i18n("Minimize"), ButtonIconify, false);
				connect(m_buttons[ButtonIconify], SIGNAL(clicked()), this, SLOT(minimize()));
				layout->addWidget(m_buttons[ButtonIconify], 0,  Qt::AlignTop);
				layout->setStretchFactor(m_buttons[ButtonIconify], 0);
				m_buttons[ButtonIconify]->setFixedSize(boxHeight, boxHeight);
				countButtons++;
			}
			break;
		case 'A': // Maximize
			if (isMaximizable()){
				m_buttons[ButtonMaximize] = new ComixButton(this, "maximize", i18n("Maximize"), ButtonMaximize, true);
				connect(m_buttons[ButtonMaximize], SIGNAL(clicked()), this, SLOT(maxButtonPressed()));
				layout->addWidget(m_buttons[ButtonMaximize], 0,  Qt::AlignTop);
				layout->setStretchFactor(m_buttons[ButtonMaximize], 0);
				m_buttons[ButtonMaximize]->setFixedSize(boxHeight, boxHeight);
				countButtons++;
			}
			break;
		case 'X': // Close
			if (isCloseable()){
				m_buttons[ButtonClose] = new ComixButton(this, "close", i18n("Close"), ButtonClose, false);
				connect(m_buttons[ButtonClose], SIGNAL(clicked()), this, SLOT(closeWindow()));
				layout->addWidget(m_buttons[ButtonClose], 0, Qt::AlignTop);
				layout->setStretchFactor(m_buttons[ButtonClose], 0);
				m_buttons[ButtonClose]->setFixedSize(boxHeight,boxHeight);
				countButtons++;
			}
			break;
		case 'L': // Shade
			m_buttons[ButtonShade] = new ComixButton(this, "shade", i18n("Shade"), ButtonShade, true);
			connect(m_buttons[ButtonShade], SIGNAL(clicked()), this, SLOT(slotShade()));
			layout->addWidget(m_buttons[ButtonShade], 0, Qt::AlignTop);
			layout->setStretchFactor(m_buttons[ButtonShade], 0);
			m_buttons[ButtonShade]->setFixedSize(boxHeight, boxHeight);
			countButtons++;
			break;
		case 'F': // Above all others
			m_buttons[ButtonAbove] = new ComixButton(this, "above", i18n("Keep Above Others"), ButtonAbove, true);
			connect(m_buttons[ButtonAbove], SIGNAL(clicked()), this, SLOT(slotKeepAbove()));
			layout->addWidget(m_buttons[ButtonAbove], 0, Qt::AlignTop);
			layout->setStretchFactor(m_buttons[ButtonAbove], 0);
			m_buttons[ButtonAbove]->setFixedSize(boxHeight, boxHeight);
			countButtons++;
			break;
		case 'B': // Below all others
			m_buttons[ButtonBelow] = new ComixButton(this, "below", i18n("Keep Below Others"), ButtonBelow, true);
			connect(m_buttons[ButtonBelow], SIGNAL(clicked()), this, SLOT(slotKeepBelow()));
			layout->addWidget(m_buttons[ButtonBelow], 0, Qt::AlignTop);
			layout->setStretchFactor(m_buttons[ButtonBelow], 0);
			m_buttons[ButtonBelow]->setFixedSize(boxHeight, boxHeight);
			countButtons++;
			break;
		/*
		case 'R': // Resize
			m_buttons[ButtonResize] = new ComixButton(this, "resize", i18n("Resize window"), ButtonResize, false);
			connect(m_buttons[ButtonResize], SIGNAL(clicked()), this, SLOT(slotResize()));
			layout->addWidget(m_buttons[ButtonResize], 0, Qt::AlignTop);
			layout->setStretchFactor(m_buttons[ButtonResize], 0);
			m_buttons[ButtonResize]->setFixedSize(boxHeight, boxHeight);
			m_buttons[ButtonResize]->setAutoRepeat(true);
			countButtons++;
			break;
		*/
		case '_': // Spacer item
			layout->addSpacing(borderSize);
			countSpacer++;
			break;
		}

	}

}

//
// someGeometry()
//
void ComixClient::someGeometry() 
{
	
	textMargin 	= clientHandler->gTextMargin();
	boxHeight 	= clientHandler->gBoxHeight();
	boxRadius 	= boxHeight / 2;
	iconHeight 	= clientHandler->gFontHeight() * 3 / 4;
	
	borderSize 	= clientHandler->borderSize();
	penWidth 	= clientHandler->penWidth();
	
}

//
// titleGeometry( bool forceUpdate )
//
void ComixClient::titleGeometry( bool forceUpdate ) 
{
	
	titleRect = titleSpacer_->geometry();
	
	if( titleRect.width() > 0 ) {
	
		QFontMetrics fm(options()->font(isActive(), false));

		titleRect.setHeight(boxHeight);
		titleRect.setWidth(titleRect.width()-borderSize);

		int maxTitleWidth = titleRect.width();
		int drawWidth = fm.width(caption()) + textMargin + textMargin + penWidth + penWidth + penWidth;

		if ( hasMenu ) drawWidth += textMargin + menuIcon.width();

		if( m_titleExpand ) {
			titleWidth = titleRect.width();
		}
		else {
			titleWidth = QMIN(drawWidth, maxTitleWidth);
		}
		titleRect.setWidth(titleWidth);

		if( forceUpdate || titleWidth != activeCaption->width() ) {

			updateCaptionBuffer( titleWidth, boxHeight );

		}
	
	}
	
}

//
// updateCaptionBuffer()
//
void ComixClient::updateCaptionBuffer( int captionWidth, int captionHeight )
{
	
	delete activeCaption;
	delete inactiveCaption;
	
	activeCaption = new QPixmap( captionWidth, captionHeight );
	inactiveCaption = new QPixmap( captionWidth, captionHeight );
	QPixmap tmp(captionHeight-10,captionHeight);
	
	QPainter tp(&tmp);
	QPainter ap(activeCaption);
	QPainter ip(inactiveCaption);

	tp.drawPixmap(0,0,*clientHandler->getPixmap(ActiveButton),5,0,captionHeight-5,-1);
	
	ap.drawPixmap(0,0,*clientHandler->getPixmap(ActiveButton),0,0,5,-1);
	ap.drawTiledPixmap(5,0,captionWidth-10,captionHeight,tmp);
	ap.drawPixmap(captionWidth-5,0,*clientHandler->getPixmap(ActiveButton),captionHeight-5,0,5,-1);

	tp.drawPixmap(0,0,*clientHandler->getPixmap(InactiveButton),5,0,captionHeight-5,-1);
	
	ip.drawPixmap(0,0,*clientHandler->getPixmap(InactiveButton),0,0,5,-1);
	ip.drawTiledPixmap(5,0,captionWidth-10,captionHeight,tmp);
	ip.drawPixmap(captionWidth-5,0,*clientHandler->getPixmap(InactiveButton),captionHeight-5,0,5,-1);

	// draw icon and text
	int 	inset = textMargin + penWidth;
	QRect 	textRect = QRect(0,-2,captionWidth,captionHeight+3);
	insetRect(&textRect,inset);
	
	QFontMetrics fm(options()->font(isActive(), false));
	int textWidth = fm.width(caption()) + inset + inset;
	if ( hasMenu ) textWidth += menuIcon.width() + textMargin;
	
	int textMoveBy = 0;
	
	switch( m_titleAlign ) {
        case Qt::AlignHCenter:
			textMoveBy = (titleRect.width() - textWidth) / 2;
			break;
        case Qt::AlignRight:
			textMoveBy = titleRect.width() - textWidth;
			break;
		default:
			break;
	}
	
	if( textMoveBy < 0 ) textMoveBy = 0;
	
	if ( hasMenu ) {
		iconRect.setRect(inset, inset, menuIcon.width(), menuIcon.height());
		iconRect.moveBy(textMoveBy,0);
		
	    ap.drawPixmap(iconRect.left(), iconRect.top(), menuIcon);
	    ip.drawPixmap(iconRect.left(), iconRect.top(), menuIcon);
		
		textRect.setLeft(textRect.left() + menuIcon.width() + textMargin);
	}

	textRect.moveBy(textMoveBy,0);

	ap.setFont(options()->font(true, false));
	ap.setPen(options()->color(KDecoration::ColorFont, true));
	ap.drawText(textRect, AlignLeft, caption());

	ip.setFont(options()->font(false, false));
	ip.setPen(options()->color(KDecoration::ColorFont, false));
	ip.drawText(textRect, AlignLeft, caption());

}

//
// createLayout()
//
void ComixClient::createLayout()
{

	int tSpacerHeight = boxHeight - penWidth;

    // basic layout:
    //  ________________________________________________________________
    // |  _______  ___________________________________________  _______ |
    // | | btns | |         titleSpacer                      | | btns | |
    // | |______| |__________________________________________| |___ __| |
    // |  _____________________________________________________________ |
    // | |                                                            | |
    // | |                      contentsFake                          | |
    // | |                                                            | |
    // | |____________________________________________________________| |
    // |________________________________________________________________|
    //

    if( mainLayout_ ) delete mainLayout_;

    mainLayout_ = new QVBoxLayout(widget(), borderSize, (borderSize - penWidth));

	if( m_windowSeparator ) {
		tSpacerHeight += borderSize;
    }

	titleSpacer_      = new QSpacerItem(32, tSpacerHeight,  QSizePolicy::Expanding, QSizePolicy::Fixed);

    // title
    QHBoxLayout *titleLayout = new QHBoxLayout(mainLayout_, borderSize, 0);

    // sizeof(...) is calculated at compile time
    memset(m_buttons, 0, sizeof(ComixButton *) * ButtonCount);

    createButtons(titleLayout, options()->customButtonPositions() ? options()->titleButtonsLeft() : QString(default_left));
	
    titleLayout->addItem(titleSpacer_);
	
    createButtons(titleLayout, options()->customButtonPositions() ? options()->titleButtonsRight() : QString(default_right));
    //Mid
    QHBoxLayout * midLayout   = new QHBoxLayout(mainLayout_, 0, 0);
    if( isPreview())
        midLayout->addWidget(new QLabel( i18n( "<center><b>comix preview</b></center>" ), widget()) );
    else
        midLayout->addItem( new QSpacerItem( 0, 0 ));

	// Make sure that topLayout doesn't stretch - midLayout should take
	// all spare space.
	mainLayout_->setStretchFactor(titleLayout, 0);
	mainLayout_->setStretchFactor(midLayout, 1);
	
}

//
// repaintButtons(void)
//
void ComixClient::repaintButtons(void)
{
	//force all buttons to repaint
    for (int n=0; n<ButtonCount; n++)
        if (m_buttons[n]) m_buttons[n]->repaint(false);

}

//
// maxButtonPressed()
//
void ComixClient::maxButtonPressed()
{
    if (m_buttons[ButtonMaximize]) {
        switch (m_buttons[ButtonMaximize]->lastMousePress())
        {
          case MidButton:
              maximize(maximizeMode() ^ MaximizeVertical );
              maximizeChange();
			  m_buttons[ButtonMaximize]->setOn(false);
              break;
          case RightButton:
              maximize(maximizeMode() ^ MaximizeHorizontal );
              maximizeChange();
			  m_buttons[ButtonMaximize]->setOn(false);
              break;
          default:
              maximize(maximizeMode() == MaximizeFull ? MaximizeRestore : MaximizeFull );
              maximizeChange();
        }

        m_buttons[ButtonMaximize]->setMaximized( maximizeMode()!=MaximizeRestore );
        QToolTip::add(m_buttons[ButtonMaximize], (maximizeMode()!=MaximizeRestore) ? i18n("Restore") : i18n("Maximize"));
    }
}

//
// mousePosition(const QPoint &point) const
//
KDecoration::Position ComixClient::mousePosition(const QPoint &point) const
{
	return KDecoration::mousePosition(point);
}

//
// minimumSize() const
//
QSize ComixClient::minimumSize() const
{
    return QSize (200, 50);
}

//
// slotShade()
//
void ComixClient::slotShade()
{
	
	setShade( !isShade() );
	
}

//
// slotKeepAbove()
//
void ComixClient::slotKeepAbove()
{
    bool above = !keepAbove();
    setKeepAbove( above );
    if (m_buttons[ButtonAbove])
    {
        m_buttons[ButtonAbove]->setOn(above);
        m_buttons[ButtonAbove]->setAbove(above);
        QToolTip::add(m_buttons[ButtonAbove], above ? i18n("Do Not Keep Above Others") : i18n("Keep Above Others"));
    }

    if (m_buttons[ButtonBelow] && m_buttons[ButtonBelow]->isOn())
    {
        m_buttons[ButtonBelow]->setOn(false);
        m_buttons[ButtonBelow]->setBelow(false);
        QToolTip::add(m_buttons[ButtonBelow], i18n("Keep Below Others"));
    }
}

//
// slotKeepBelow()
//
void ComixClient::slotKeepBelow()
{
    bool below = !keepBelow();
    setKeepBelow( below );
    if (m_buttons[ButtonBelow])
    {
        m_buttons[ButtonBelow]->setOn(below);
        m_buttons[ButtonBelow]->setBelow(below);
        QToolTip::add(m_buttons[ButtonBelow], below ? i18n("Do Not Keep Below Others") : i18n("Keep Below Others"));
    }

    if (m_buttons[ButtonAbove] && m_buttons[ButtonAbove]->isOn())
    {
        m_buttons[ButtonAbove]->setOn(false);
        m_buttons[ButtonAbove]->setAbove(false);
        QToolTip::add(m_buttons[ButtonAbove], i18n("Keep Above Others"));
    }
}

//
// slotResize()
//
void ComixClient::slotResize()
{
	//	performWindowOperation(UnrestrictedResizeOp);
}

/*
//
// setResizeMouse( QPoint pos )
//
void ComixClient::setResizeMouse( QPoint pos )
{
    resizeMouse = pos;
}
*/

//
// resizeEvent(QResizeEvent *)
//
void ComixClient::resizeEvent(QResizeEvent *)
{
	titleGeometry( false );
    widget()->repaint();
}

//
// showEvent(QShowEvent *)
//
void ComixClient::showEvent(QShowEvent *)
{
    widget()->repaint();
}

//
// isTool() const
//
bool ComixClient::isTool() const
{
    const int SUPPORTED_WINDOW_TYPES_MASK =
        NET::NormalMask | NET::DesktopMask | NET::DockMask | NET::ToolbarMask |
        NET::MenuMask | NET::DialogMask | NET::OverrideMask | NET::UtilityMask |
        NET::SplashMask;
        NET::WindowType type = windowType (SUPPORTED_WINDOW_TYPES_MASK);
    return type == NET::Toolbar || type == NET::Utility || type == NET::Menu;
}

