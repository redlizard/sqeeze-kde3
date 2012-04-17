/*
	Copyright (C) 2006 Michael Lentner <michaell@gmx.net>
	
	based on KDE2 Default KWin client:
	Copyright (C) 1999, 2001 Daniel Duley <mosfet@kde.org>
	Matthias Ettrich <ettrich@kde.org>
	Karol Szwed <gallium@kde.org>
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Library General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "dominoclient.h"
#include "clientData.h"

#include <kconfig.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <X11/extensions/shape.h>
#include <qwmatrix.h>
#include <qpainter.h>
#include <qsettings.h>


namespace Domino
{

static DominoHandler* clientHandler;
static bool Domino_initialized = false;
const int titleEdgeTop = 3;
const int titleEdgeBottom = 1;
static int titleHeight;

static QPixmap* buttonPix;
static QPixmap* buttonHidePix;
static QPixmap* titleBarPix;
static QPixmap* borderTopLeftPix;
static QPixmap* borderTopRightPix;
static QPixmap* borderBottomPix;
static QPixmap* borderBottomLeftPix;
static QPixmap* borderBottomRightPix;
static QPixmap* borderLeftPix;
static QPixmap* borderRightPix;


static QPixmap* closeButtonIcon;
static QPixmap* minButtonIcon;
static QPixmap* maxButtonIcon;
static QPixmap* helpButtonIcon;
static QPixmap* onAllDesktopsButtonIcon;
static QPixmap* shadeButtonIcon;
static QPixmap* aboveButtonIcon;
static QPixmap* belowButtonIcon;

static QPixmap* pressedCloseButtonIcon;
static QPixmap* pressedMinButtonIcon;
static QPixmap* pressedMaxButtonIcon;
static QPixmap* pressedHelpButtonIcon;
static QPixmap* pressedOnAllDesktopsButtonIcon;
static QPixmap* pressedShadeButtonIcon;
static QPixmap* pressedAboveButtonIcon;
static QPixmap* pressedBelowButtonIcon;

static QPixmap* mouseOverCloseButtonIcon;
static QPixmap* mouseOverMinButtonIcon;
static QPixmap* mouseOverMaxButtonIcon;
static QPixmap* mouseOverHelpButtonIcon;
static QPixmap* mouseOverOnAllDesktopsButtonIcon;
static QPixmap* mouseOverShadeButtonIcon;
static QPixmap* mouseOverAboveButtonIcon;
static QPixmap* mouseOverBelowButtonIcon;

static bool buttonInputShape;
static QBitmap* buttonShapeBitmap;

static QColor background;
static QColor topGradientColor;
static QColor bottomGradientColor;
static QColor topBorderGradientColor;
static QColor bottomBorderGradientColor;
static bool showButtonIcons;
static bool showInactiveButtons;

const int titleGradientHeight = 10;
const int bottomBorderHeight = 8;
const int borderWidth = 5;

// ===========================================================================

DominoHandler::DominoHandler()
{
	int ignore, maj, min;
	
	if(XShapeQueryExtension(qt_xdisplay(), &ignore, &ignore) &&
		  XShapeQueryVersion(qt_xdisplay(), &maj, &min) &&
		  maj >= 1 && min >= 1) {
		
		buttonInputShape = true;
	}
	else
		buttonInputShape = false;
	
	clientHandler = this;
	readConfig( false );
	createPixmaps();
	Domino_initialized = true;
}


DominoHandler::~DominoHandler()
{
	Domino_initialized = false;
	freePixmaps();
	clientHandler = NULL;
}

KDecoration* DominoHandler::createDecoration( KDecorationBridge* b )
{
		return new DominoClient( b, this );
}

bool DominoHandler::reset( unsigned long changed )
{
	Domino_initialized = false;
	changed |= readConfig( true );
	if( changed & SettingColors )
	{ // pixmaps need to be recreated
		freePixmaps();
		createPixmaps();
	}
	Domino_initialized = true;
	// SettingButtons is handled by KCommonDecoration
	bool need_recreate = ( changed & ( SettingDecoration | SettingFont | SettingBorder )) != 0;
	if( need_recreate )  // something else than colors changed
		return true;
	resetDecorations( changed );
	return false;
}


unsigned long DominoHandler::readConfig( bool update )
{
	unsigned long changed = 0;
	KConfig conf("kwindominorc");
	conf.setGroup("General");
	
	titleHeight = QMAX(QFontMetrics(options()->font(true)).height(), 16);
	background = qApp->palette().active().background();
	
	customBorderColor = conf.readBoolEntry("customBorderColor", true);
	borderColor = customBorderColor ? conf.readEntry("borderColor", "#777B7F") : background;
	
	customButtonColor = conf.readBoolEntry("customButtonColor", false);
	buttonColor = customButtonColor ? conf.readEntry("buttonColor", "#212121") : background;
	customButtonIconColor = conf.readBoolEntry("customButtonIconColor", false);
	buttonIconColor = customButtonIconColor ? conf.readEntry("buttonIconColor", "#000000") : "#000000";
	showInactiveButtons = conf.readBoolEntry("showInactiveButtons", false);
	showButtonIcons = conf.readBoolEntry("showButtonIcons", false);
	darkLines = conf.readBoolEntry("darkFrame", false);
	
	if(!conf.readBoolEntry( "customGradientColors", false)) {
		topGradientColor = alphaBlendColors(background, Qt::white, 180);
		bottomGradientColor = alphaBlendColors(background, Qt::black, 230);
		topBorderGradientColor = alphaBlendColors(borderColor, Qt::white, 180);
		bottomBorderGradientColor = alphaBlendColors(borderColor, Qt::black, 230);
	}
	else {
		topGradientColor = conf.readEntry("topGradientColor", "#ffffff");
		bottomGradientColor = conf.readEntry("bottomGradientColor", "#000000");
		topBorderGradientColor = topGradientColor;
		bottomBorderGradientColor = bottomGradientColor;
	}

	if( update ) {
		changed |= SettingColors; // just recreate the pixmaps and repaint
	}
	
	if(conf.readBoolEntry("useDominoStyleContourColors", true)) {
		QSettings s;
		buttonContourColor = s.readEntry("/domino/Settings/buttonContourColor", background.dark(250).name());
		buttonMouseOverContourColor = s.readEntry("/domino/Settings/buttonMouseOverContourColor", background.dark(250).name());
		buttonPressedContourColor = s.readEntry("/domino/Settings/buttonPressedContourColor", background.dark(250).name());
	}
	else {
		buttonContourColor = conf.readEntry("buttonContourColor", background.dark(250).name());
		buttonMouseOverContourColor = conf.readEntry("buttonMouseOverContourColor", background.dark(250).name());
		buttonPressedContourColor= conf.readEntry("buttonPressedContourColor", background.dark(250).name());
	}
	
	return changed;
}


// This paints the button pixmaps upon loading the style.
void DominoHandler::createPixmaps()
{
	
	QImage edges;
	QImage topLines;
	QImage rightLines;
	QImage bottomLines;
	
	if(darkLines) {
		edges = qembed_findImage("client_edges_dark");
		topLines = qembed_findImage("client_topLines_dark");
		rightLines = qembed_findImage("client_rightLines_dark");
		bottomLines = qembed_findImage("client_bottomLines_dark");
	}
	else {
		edges = qembed_findImage("client_edges");
		topLines = qembed_findImage("client_topLines");
		rightLines = qembed_findImage("client_rightLines");
		bottomLines = qembed_findImage("client_bottomLines");
	}
	
	
	
	
	int titleBarHeight = titleHeight+titleEdgeTop+titleEdgeBottom;
	QWMatrix m;
	m.rotate(180);
	
	titleBarPix = new QPixmap(QSize(10, titleBarHeight));
	titleBarPix->fill(background);
	QRect r(titleBarPix->rect());
	QPainter p(titleBarPix);
	QRect rect(r.x(), r.y(), r.width(), titleGradientHeight);
	renderGradient(&p, rect, topGradientColor, background, "titleBar");
	p.drawPixmap(0, 0, topLines);
	p.end();
	
	
	
	///////// borders
	// sides
	QPixmap sideLinesRight =  rightLines;
	
	borderRightPix = new QPixmap(5, 10);
	borderRightPix->fill(borderColor);
	p.begin(borderRightPix);
	p.drawPixmap(3, 0, sideLinesRight);
	p.end();
	
	borderLeftPix = new QPixmap(borderRightPix->xForm(m));
	// edges
	borderTopLeftPix = new QPixmap(QSize(5, titleBarHeight));
	borderTopLeftPix->fill(borderColor);
	p.begin(borderTopLeftPix);
	r = QRect(0, 0, 5, titleGradientHeight);
	// if the gradient color is equal to the background color, we make the border also flat.
	renderGradient(&p, r, topGradientColor == background ? borderColor : topBorderGradientColor, borderColor, "borderTopLeftPix");
	borderTopRightPix = new QPixmap(*borderTopLeftPix);
	p.drawPixmap(0, 1, edges, 0, 0, 5, 6);
	p.drawTiledPixmap(0, 7, 2, titleBarHeight, sideLinesRight.xForm(m));
	p.end();
	p.begin(borderTopRightPix);
	p.drawPixmap(0, 1, edges, 5, 0, 5, 6);
	p.drawTiledPixmap(3, 7, 2, titleBarHeight, sideLinesRight);
	p.end();
	
	
	borderBottomRightPix = new QPixmap(QSize(5, bottomBorderHeight));
	borderBottomRightPix->fill(borderColor);
	p.begin(borderBottomRightPix);
	r = QRect(0, bottomBorderHeight-6, 5, 6);
	renderGradient(&p, r, borderColor, bottomGradientColor == background ? borderColor : bottomBorderGradientColor, "borderBottomRightPix");
	borderBottomLeftPix = new QPixmap(*borderBottomRightPix);
	p.drawPixmap(0, bottomBorderHeight-7, edges, 5, 6, 5, 6);
	p.drawPixmap(3, 0, sideLinesRight, 0, 0, 2, borderBottomLeftPix->height()-7);
	p.end();
	
	
	p.begin(borderBottomLeftPix);
	p.drawPixmap(0, bottomBorderHeight-7, edges, 0, 6, 5 ,6);
	p.drawPixmap(0, 0, sideLinesRight.xForm(m),  0, 0, 2, borderBottomLeftPix->height()-7);
	p.end();
	
	borderBottomPix = new QPixmap(QSize(10, bottomBorderHeight));
	borderBottomPix->fill(background);
	p.begin(borderBottomPix);
	r = QRect(0, bottomBorderHeight-6, 10, 6);
	renderGradient(&p, r, background, bottomGradientColor, "borderBottom");
	p.drawPixmap(0, bottomBorderHeight-2, bottomLines);
	p.end();
	
	
	
	///////// buttons
	////////////////////////////////////////////////////////////////////////
	
	if(buttonInputShape) {
		QRegion buttonClip(6,2,6,1);
		buttonClip += QRegion(5,3,8,1);
		buttonClip += QRegion(4,4,10,1);
		buttonClip += QRegion(3,5,12,1);
		buttonClip += QRegion(2,6,14,6);
		buttonClip += QRegion(3,12,12,1);
		buttonClip += QRegion(4,13,10,1);
		buttonClip += QRegion(5,14,8,1);
		buttonClip += QRegion(6,15,6,1);
		
		buttonShapeBitmap = new QBitmap(16, titleBarHeight, true);
		QPainter p(buttonShapeBitmap);
		p.setClipRegion(buttonClip);
		p.fillRect(QRect(0, 0, 16, titleBarHeight), Qt::color1);
	}
	
	QPixmap* buttonShadow = new QPixmap(qembed_findImage("clientButtonShadow"));
	QPixmap* buttonPressed = new QPixmap(qembed_findImage("clientButtonPressed"));
	
	buttonPix = new QPixmap(QSize(16, titleBarHeight));
	p.begin(buttonPix);
	p.drawTiledPixmap(0, 0, titleBarHeight, titleBarHeight, *titleBarPix, 0, titleEdgeTop);
	
	//////////// custom color
	r = QRect(buttonPix->rect());
	QRegion buttonClip(5,2,6,1);
	buttonClip += QRegion(4,3,8,1);
	buttonClip += QRegion(3,4,10,1);
	buttonClip += QRegion(2,5,12,6);
	buttonClip += QRegion(3,11,10,1);
	buttonClip += QRegion(4,12,8,1);
	buttonClip += QRegion(5,13,6,1);
	
	p.setClipRegion(buttonClip);
	p.fillRect(QRect(0, 0, r.width(), r.height()), buttonColor);
	QRect gr(0, -3, r.width(), titleGradientHeight);
	renderGradient(&p, gr, topGradientColor, buttonColor, "titleBar");
	p.setClipping(false);
	///////////
	p.drawPixmap(0, 0, *buttonShadow);
	p.end();
	
	
	QPixmap* pressedButtonPix = new QPixmap(*buttonPix);
	p.begin(pressedButtonPix);
	p.drawPixmap(0, 0, tintImage(qembed_findImage("clientButtonContour"), buttonPressedContourColor));
	p.drawPixmap(0, 0, *buttonPressed);
	p.end();
	
	
	QPixmap* mouseOverButtonPix = new QPixmap(*buttonPix);
	p.begin(mouseOverButtonPix);
	p.drawPixmap(0, 0, tintImage(qembed_findImage("clientButtonContour"), buttonMouseOverContourColor));
	p.end();
	
	
	p.begin(buttonPix);
	p.drawPixmap(0, 0, tintImage(qembed_findImage("clientButtonContour"), buttonContourColor));
	p.end();
	
	
	buttonHidePix = new QPixmap(QSize(buttonPix->width(), titleBarHeight));
	p.begin(buttonHidePix);
	p.drawTiledPixmap(0, 0, buttonHidePix->width(), buttonHidePix->height(), *titleBarPix, 0, titleEdgeTop);
	p.end();
	
	
	
	
	///////// icons
	///////////////////////////////////////////////////////////////////////
	
	QPixmap* icon = new QPixmap(customButtonIconColor ? tintImage(qembed_findImage("closeButtonIcon"), buttonIconColor) : qembed_findImage("closeButtonIcon"));
	closeButtonIcon = new QPixmap(*buttonPix);
	bitBlt(closeButtonIcon, 5, 5, icon);
	pressedCloseButtonIcon = new QPixmap(*pressedButtonPix);
	bitBlt(pressedCloseButtonIcon, 5, 5, icon);
	mouseOverCloseButtonIcon = new QPixmap(*mouseOverButtonPix);
	bitBlt(mouseOverCloseButtonIcon, 5, 5, icon);
	delete icon;
	
	icon = new QPixmap(customButtonIconColor ? tintImage(qembed_findImage("maxButtonIcon"), buttonIconColor) : qembed_findImage("maxButtonIcon"));
	maxButtonIcon = new QPixmap(*buttonPix);
	bitBlt(maxButtonIcon, 4, 4, icon);
	pressedMaxButtonIcon = new QPixmap(*pressedButtonPix);
	bitBlt(pressedMaxButtonIcon, 4, 4, icon);
	mouseOverMaxButtonIcon = new QPixmap(*mouseOverButtonPix);
	bitBlt(mouseOverMaxButtonIcon, 4, 4, icon);
	delete icon;
	
	icon = new QPixmap(customButtonIconColor ? tintImage(qembed_findImage("minButtonIcon"), buttonIconColor) : qembed_findImage("minButtonIcon"));
	minButtonIcon = new QPixmap(*buttonPix);
	bitBlt(minButtonIcon, 5, 9, icon);
	pressedMinButtonIcon = new QPixmap(*pressedButtonPix);
	bitBlt(pressedMinButtonIcon, 5, 9, icon);
	mouseOverMinButtonIcon = new QPixmap(*mouseOverButtonPix);
	bitBlt(mouseOverMinButtonIcon, 5, 9, icon);
	delete icon;
	
	icon = new QPixmap(customButtonIconColor ? tintImage(qembed_findImage("helpButtonIcon"), buttonIconColor) : qembed_findImage("helpButtonIcon"));
	helpButtonIcon = new QPixmap(*buttonPix);
	bitBlt(helpButtonIcon, 5, 4, icon);
	pressedHelpButtonIcon = new QPixmap(*pressedButtonPix);
	bitBlt(pressedHelpButtonIcon, 5, 4, icon);
	mouseOverHelpButtonIcon = new QPixmap(*mouseOverButtonPix);
	bitBlt(mouseOverHelpButtonIcon, 5, 4, icon);
	delete icon;
	
	icon = new QPixmap(customButtonIconColor ? tintImage(qembed_findImage("onAllDesktopsButtonIcon"), buttonIconColor) : qembed_findImage("onAllDesktopsButtonIcon"));
	onAllDesktopsButtonIcon = new QPixmap(*buttonPix);
	bitBlt(onAllDesktopsButtonIcon, 6, 6, icon);
	pressedOnAllDesktopsButtonIcon = new QPixmap(*pressedButtonPix);
	bitBlt(pressedOnAllDesktopsButtonIcon, 6, 6, icon);
	mouseOverOnAllDesktopsButtonIcon = new QPixmap(*mouseOverButtonPix);
	bitBlt(mouseOverOnAllDesktopsButtonIcon, 6, 6, icon);
	delete icon;
	
	icon = new QPixmap(customButtonIconColor ? tintImage(qembed_findImage("aboveButtonIcon"), buttonIconColor) : qembed_findImage("aboveButtonIcon"));
	aboveButtonIcon = new QPixmap(*buttonPix);
	bitBlt(aboveButtonIcon, 5, 4, icon);
	pressedAboveButtonIcon = new QPixmap(*pressedButtonPix);
	bitBlt(pressedAboveButtonIcon, 5, 4, icon);
	mouseOverAboveButtonIcon = new QPixmap(*mouseOverButtonPix);
	bitBlt(mouseOverAboveButtonIcon, 5, 4, icon);
	delete icon;
	
	icon = new QPixmap(customButtonIconColor ? tintImage(qembed_findImage("aboveButtonIcon").xForm(m), buttonIconColor) : qembed_findImage("aboveButtonIcon").xForm(m));
	belowButtonIcon = new QPixmap(*buttonPix);
	bitBlt(belowButtonIcon, 5, 4, icon);
	pressedBelowButtonIcon = new QPixmap(*pressedButtonPix);
	bitBlt(pressedBelowButtonIcon, 5, 4, icon);
	mouseOverBelowButtonIcon = new QPixmap(*mouseOverButtonPix);
	bitBlt(mouseOverBelowButtonIcon, 5, 4, icon);
	delete icon;
	
	icon = new QPixmap(customButtonIconColor ? tintImage(qembed_findImage("minButtonIcon"), buttonIconColor) : qembed_findImage("minButtonIcon"));
	shadeButtonIcon = new QPixmap(*buttonPix);
	bitBlt(shadeButtonIcon, 5, 5, icon);
	pressedShadeButtonIcon = new QPixmap(*pressedButtonPix);
	bitBlt(pressedShadeButtonIcon, 5, 5, icon);
	mouseOverShadeButtonIcon = new QPixmap(*mouseOverButtonPix);
	bitBlt(mouseOverShadeButtonIcon, 5, 5, icon);
	delete icon;
	
	delete mouseOverButtonPix;
	delete pressedButtonPix;
}

void DominoHandler::freePixmaps()
{
	
	delete buttonPix;
	delete buttonHidePix;
	buttonPix = 0;
	delete titleBarPix;
	delete borderTopRightPix;
	delete borderTopLeftPix;
	delete borderBottomRightPix;
	delete borderBottomLeftPix;
	delete borderLeftPix;
	delete borderRightPix;
	delete borderBottomPix;
	
	delete closeButtonIcon;
	delete maxButtonIcon;
	delete minButtonIcon;
	delete helpButtonIcon;
	delete onAllDesktopsButtonIcon;
	delete aboveButtonIcon;
	delete belowButtonIcon;
	delete shadeButtonIcon;
	
	delete pressedCloseButtonIcon;
	delete pressedMaxButtonIcon;
	delete pressedMinButtonIcon;
	delete pressedHelpButtonIcon;
	delete pressedOnAllDesktopsButtonIcon;
	delete pressedAboveButtonIcon;
	delete pressedBelowButtonIcon;
	delete pressedShadeButtonIcon;
	
	delete mouseOverCloseButtonIcon;
	delete mouseOverMinButtonIcon;
	delete mouseOverMaxButtonIcon;
	delete mouseOverHelpButtonIcon;
	delete mouseOverOnAllDesktopsButtonIcon;
	delete mouseOverShadeButtonIcon;
	delete mouseOverAboveButtonIcon;
	delete mouseOverBelowButtonIcon;
	
	
	
	if(buttonInputShape)
		delete buttonShapeBitmap;
	
}

bool DominoHandler::supports( Ability ability )
{
	switch( ability )
		{
		case AbilityAnnounceButtons:
		case AbilityButtonMenu:
		case AbilityButtonOnAllDesktops:
		case AbilityButtonSpacer:
		case AbilityButtonHelp:
		case AbilityButtonMinimize:
		case AbilityButtonMaximize:
		case AbilityButtonClose:
		case AbilityButtonAboveOthers:
		case AbilityButtonBelowOthers:
		case AbilityButtonShade:
			return true;
		default:
			return false;
		};
}

// ===========================================================================

DominoButton::DominoButton(ButtonType type, DominoClient *parent, const char *name)
	: KCommonDecorationButton(type, parent, name)
{
	isMouseOver = false;
	client = parent;
#ifdef ShapeInput
	if(buttonInputShape && buttonShapeBitmap ) {
		XShapeCombineMask (qt_xdisplay(),
				   winId(),
				   ShapeInput,
				   0,
				   0,
				   buttonShapeBitmap->handle(),
				   ShapeSet);
	}
#endif
}


DominoButton::~DominoButton()
{
}


void DominoButton::reset(unsigned long changed)
{
	if (changed&SizeChange || changed&ManualReset) {
		setBackgroundOrigin(QWidget::WidgetOrigin);
		setErasePixmap(showInactiveButtons ? *buttonPix : *buttonHidePix);
	}
	
	
	if (changed&DecorationReset || changed&ManualReset || changed&SizeChange || changed&StateChange) {
		this->update();
	}

	
}


void DominoButton::drawButton(QPainter *p)
{
	if (!Domino_initialized)
		return;
	
	bool down = isDown() || isOn();
	if(down || (( isMouseOver || showButtonIcons) && (showInactiveButtons || decoration()->isActive() || client->titleBarMouseOver)) ) {
		switch (type()) {
			case CloseButton:
				p->drawPixmap(0, 0, down ? *pressedCloseButtonIcon : isMouseOver ? *mouseOverCloseButtonIcon : *closeButtonIcon);
				break;
			case HelpButton:
				p->drawPixmap(0, 0, down ? *pressedHelpButtonIcon : isMouseOver ? *mouseOverHelpButtonIcon : *helpButtonIcon);
				break;
			case MinButton:
				p->drawPixmap(0, 0, down ? *pressedMinButtonIcon : isMouseOver ? *mouseOverMinButtonIcon : *minButtonIcon);
				break;
			case MaxButton:
				p->drawPixmap(0, 0, down ? *pressedMaxButtonIcon : isMouseOver ? *mouseOverMaxButtonIcon : *maxButtonIcon);
				break;
			case OnAllDesktopsButton:
				p->drawPixmap(0, 0, down ? *pressedOnAllDesktopsButtonIcon : isMouseOver ? *mouseOverOnAllDesktopsButtonIcon : *onAllDesktopsButtonIcon);
				break;
			case ShadeButton:
				p->drawPixmap(0, 0, down ? *pressedShadeButtonIcon : isMouseOver ? *mouseOverShadeButtonIcon : *shadeButtonIcon);
				break;
			case AboveButton:
				p->drawPixmap(0, 0, down ? *pressedAboveButtonIcon : isMouseOver ? *mouseOverAboveButtonIcon : *aboveButtonIcon);
				break;
			case BelowButton:
				p->drawPixmap(0, 0, down ? *pressedBelowButtonIcon : isMouseOver ? *mouseOverBelowButtonIcon : *belowButtonIcon);
				break;
			case MenuButton:
			default:
				break;
		}
	}
	else {
		if((client->titleBarMouseOver && type() != MenuButton) || (decoration()->isActive() && !showInactiveButtons && type() != MenuButton)) {
			p->drawPixmap(0, 0, *buttonPix);
		}
	}
}



void DominoButton::enterEvent(QEvent */*e*/)
{
	
	if(!client->isActive() && !showInactiveButtons)
		client->titleBarMouseOver = true;
	
	isMouseOver=true;
	repaint(false);
}


void DominoButton::leaveEvent(QEvent */*e*/)
{
	isMouseOver=false;
	repaint(false);
	client->titleBarMouseOver = false;
}


// ===========================================================================

DominoClient::DominoClient( KDecorationBridge* b, KDecorationFactory* f )
	: KCommonDecoration( b, f )
{
}

DominoClient::~DominoClient()
{
	
	delete titleBar;
	delete borderTopLeft;
	delete borderTopRight;
	delete borderBottomLeft;
	delete borderBottomRight;
	delete borderLeft;
	delete borderRight;
	delete borderBottom;
}

QString DominoClient::visibleName() const
{
	return "Domino";
}

QString DominoClient::defaultButtonsLeft() const
{
	return "MF";
}

QString DominoClient::defaultButtonsRight() const
{
	return "HIAX";
}

bool DominoClient::decorationBehaviour(DecorationBehaviour behaviour) const
{
	switch (behaviour) {
		case DB_MenuClose:
			return true;
		case DB_WindowMask:
			return true;
		case DB_ButtonHide:
			return true;
		default:
			return KCommonDecoration::decorationBehaviour(behaviour);
	}
}

int DominoClient::layoutMetric(LayoutMetric lm, bool respectWindowState, const KCommonDecorationButton *btn) const
{
	bool maximized = maximizeMode() == MaximizeFull && !options()->moveResizeMaximizedWindows();
	
	switch (lm) {
		case LM_BorderLeft:
		case LM_BorderRight:
			if(respectWindowState && maximized)
				return 0;
			return borderWidth;

		case LM_BorderBottom: {
			if(respectWindowState && maximized)
				return 0;
			return bottomBorderHeight; // = 8
		}
			
		case LM_TitleEdgeLeft:
		case LM_TitleEdgeRight:
			return borderWidth + 3;

		case LM_TitleEdgeTop:
			return titleEdgeTop; // = 3

		case LM_TitleEdgeBottom:
			return titleEdgeBottom; // = 1

		case LM_TitleBorderLeft:
		case LM_TitleBorderRight:
			return 1;

		case LM_TitleHeight: {
			return titleHeight;
		}

		case LM_ButtonWidth:
			return 16; // pixmap width
			
		case LM_ButtonHeight:
			return titleHeight;

		case LM_ButtonSpacing:
			return 3;

		case LM_ExplicitButtonSpacer:
			return 2;

		default:
			return KCommonDecoration::layoutMetric(lm, respectWindowState, btn);
	}
}

KCommonDecorationButton *DominoClient::createButton(ButtonType type)
{
	switch (type) {
		case MenuButton: {
			DominoButton* menu = new DominoButton(MenuButton, this, "menu");
			menuButton = menu;
			connect(menu, SIGNAL(destroyed()), this, SLOT(menuButtonDestroyed()));
			return menu;
			break;
		}
		case OnAllDesktopsButton: {
			return new DominoButton(OnAllDesktopsButton, this, "on_all_desktops");
		}
		case HelpButton: {
			return new DominoButton(HelpButton, this, "help");
		}
		case MinButton: {
			return new DominoButton(MinButton, this, "minimize");
		}
		case MaxButton: {
			return new DominoButton(MaxButton, this, "maximize");
		}
		case CloseButton: {
			return new DominoButton(CloseButton, this, "close");
		}
		case AboveButton: {
			return new DominoButton(AboveButton, this, "above");
		}
		case BelowButton: {
			return new DominoButton(BelowButton, this, "below");
		}
		case ShadeButton: {
			return new DominoButton(ShadeButton, this, "shade");
		}
		default:
			return 0;
	}
}


void DominoClient::init()
{
	
	menuButton = 0;
	globalMouseTracking = false;
	titleBarMouseOver = false;
	QWidget* w = new QWidget(KDecoration::initialParentWidget(), "clientMainWidget", WNoAutoErase|WStaticContents);
	setMainWidget(w);
	
	widget()->setBackgroundMode(NoBackground);
	widget()->installEventFilter(this);
	createLayout();

	KCommonDecoration::reset(SettingButtons);
	iconChange();
	connect(this, SIGNAL(keepAboveChanged(bool) ), SLOT(keepAboveChange(bool) ) );
	connect(this, SIGNAL(keepBelowChanged(bool) ), SLOT(keepBelowChange(bool) ) );
}

void DominoClient::createLayout()
{
	
	int titleBarHeight = titleHeight+titleEdgeTop+titleEdgeBottom;
	
	mainlayout = new QGridLayout(widget(), 3, 3, 0);
	titleBar = new TitleBar(widget(), "titlebar");
	titleBar->client = this;
	titleBar->setMouseTracking(true);
	
	borderTopLeft = new QWidget(widget(), "borderTopLeft", Qt::WNoAutoErase);
	borderTopLeft->setMouseTracking(true);
	borderTopRight = new QWidget(widget(), "borderTopRight", Qt::WNoAutoErase);
	borderTopRight->setMouseTracking(true);
	borderBottomLeft = new QWidget(widget(), "borderBottomLeft", Qt::WNoAutoErase);
	borderBottomLeft->setMouseTracking(true);
	borderBottomRight = new QWidget(widget(), "borderBottomRight", Qt::WNoAutoErase);
	borderBottomRight->setMouseTracking(true);
	borderLeft = new QWidget(widget(), "borderLeft", Qt::WNoAutoErase);
	borderLeft->setMouseTracking(true);
	borderRight = new QWidget(widget(), "borderRight", Qt::WNoAutoErase);
	borderRight->setMouseTracking(true);
	borderBottom = new QWidget(widget(), "borderBottom", Qt::WNoAutoErase);
	borderBottom->setMouseTracking(true);
	
	
	titleBar->setFixedHeight(titleBarHeight);
	titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	borderTopLeft->setFixedSize(borderWidth, titleBarHeight);
	borderTopRight->setFixedSize(borderWidth, titleBarHeight);
	
	borderLeft->setFixedWidth(borderWidth);
	borderLeft->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	borderRight->setFixedWidth(borderWidth);
	borderRight->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	
	borderBottomLeft->setFixedSize(borderWidth, bottomBorderHeight);
	borderBottomRight->setFixedSize(borderWidth, bottomBorderHeight);
	borderBottom->setFixedHeight(bottomBorderHeight);
	borderBottom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	mainlayout->addWidget(titleBar, 0,1);
	mainlayout->addWidget(borderTopLeft, 0,0);
	mainlayout->addWidget(borderTopRight, 0,2);
	
	mainlayout->addWidget(borderLeft, 1,0);
	mainlayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	mainlayout->addWidget(borderRight, 1,2);
	
	mainlayout->addWidget(borderBottomLeft, 2,0);
	mainlayout->addWidget(borderBottom, 2,1);
	mainlayout->addWidget(borderBottomRight, 2,2);
	
	setPixmaps();
	
}

void DominoClient::setPixmaps()
{
	borderTopLeft->setErasePixmap(*borderTopLeftPix);
	borderTopRight->setErasePixmap(*borderTopRightPix);
	borderBottomLeft->setErasePixmap(*borderBottomLeftPix);
	borderBottomRight->setErasePixmap(*borderBottomRightPix);
	borderLeft->setErasePixmap(*borderLeftPix);
	borderRight->setErasePixmap(*borderRightPix);
	borderBottom->setErasePixmap(*borderBottomPix);
	
	borderTopLeft->erase();
	borderTopRight->erase();
	borderBottomLeft->erase();
	borderBottomRight->erase();
	borderLeft->erase();
	borderRight->erase();
	borderBottom->erase();
}

void DominoClient::reset( unsigned long changed)
{
	
	widget()->repaint();
	KCommonDecoration::reset(changed);
	KCommonDecoration::resetButtons();
	updateCaption();
	setPixmaps();
	iconChange();
}


void DominoClient::updateMask()
{
	if(maximizeMode() == MaximizeFull && !options()->moveResizeMaximizedWindows()) {
		clearMask();
		return;
	}
	QRect r(widget()->rect());
	
	QRegion mask(r.x()+5, r.y(), r.width()-10, r.height());
	mask += QRect(r.x()+3, r.y()+1, r.width()-6, r.height()-2);
	mask += QRect(r.x()+2, r.y()+2, r.width()-4, r.height()-4);
	mask += QRect(r.x()+1, r.y()+3, r.width()-2, r.height()-6);
	mask += QRect(r.x(), r.y()+5, r.width(), r.height()-10);
	setMask(mask);
	
}

void DominoClient::updateCaption()
{
	QRect r(titleBar->rect());
	
	QPixmap* titleBar_pix = new QPixmap(r.size());
	QPainter painter(titleBar_pix);
	painter.drawTiledPixmap(0, 0, r.width(), r.height(), *titleBarPix);
	
	int lw = buttonsLeftWidth()+3;
	int rw = buttonsRightWidth();

	painter.setFont(options()->font(true,false));
	QFontMetrics fm = painter.fontMetrics();
	int captionWidth = fm.width(caption());
	int titleWidth = titleBar->width()-(lw+rw);
	
	
	QRect re(titleWidth > captionWidth ? (titleWidth-captionWidth)/2+lw : lw, 0, titleWidth, r.height());
	painter.setClipRect(re);
	painter.setPen(options()->color(KDecorationDefines::ColorFont, isActive()));
	painter.drawText(QRect(lw, r.y(), r.width()-(rw+lw), r.height()), titleWidth > captionWidth ? Qt::AlignCenter : Qt::AlignLeft|Qt::AlignVCenter, caption());
	painter.end();
	
	titleBar->setErasePixmap(*titleBar_pix);
	titleBar->erase();
	delete titleBar_pix;
}

void DominoClient::activeChange()
{
	updateButtons();
	updateCaption();
}

void DominoClient::iconChange()
{
	if(menuButton) {
		QPixmap pix(*buttonHidePix);
		QPixmap icon = KDecoration::icon().pixmap(QIconSet::Small, QIconSet::Normal);
		if(!icon.isNull())
			bitBlt(&pix, 0, 0, &icon, 0, 0, -1, -1, Qt::CopyROP);
		menuButton->setErasePixmap(pix);
	}
	
}

void DominoClient::menuButtonDestroyed()
{
	menuButton = 0;
}

void DominoClient::slotShade()
{
	setShade( !isSetShade() );
}


void DominoClient::borders( int& left, int& right, int& top, int& bottom ) const
{
	
	left = layoutMetric(LM_BorderLeft);
	right = layoutMetric(LM_BorderRight);
	bottom = layoutMetric(LM_BorderBottom);
	top = layoutMetric(LM_TitleHeight) +
			layoutMetric(LM_TitleEdgeTop) +
			layoutMetric(LM_TitleEdgeBottom);
	
	widget()->layout()->activate();
	
}

void DominoClient::shadeChange()
{
	if(isSetShade()) {
		mainlayout->remove(borderLeft);
		mainlayout->remove(borderRight);
		borderLeft->resize(borderLeft->width(), 0);
		borderRight->resize(borderLeft->width(), 0);
	}
	else {
		mainlayout->addWidget(borderLeft,1,0);
		mainlayout->addWidget(borderRight,1,2);
	}
	
	widget()->layout()->activate();
	
}

void DominoClient::resize( const QSize& s )
{
	widget()->resize( s );
	if(maximizeMode() == MaximizeFull && !options()->moveResizeMaximizedWindows()) {
		borderLeft->hide();
		borderRight->hide();
		borderTopLeft->hide();
		borderTopRight->hide();
		borderBottomLeft->hide();
		borderBottomRight->hide();
	}
	else if(borderLeft->isHidden()) {
		borderLeft->show();
		borderRight->show();
		borderTopLeft->show();
		borderTopRight->show();
		borderBottomLeft->show();
		borderBottomRight->show();
	}
}


void DominoClient::resizeEvent( QResizeEvent* ev )
{
	
	if (!Domino_initialized || ev->size() == ev->oldSize())
		return;
	KCommonDecoration::resizeEvent(ev); // which calls calcHiddenButtons()
	
	updateMask();
	updateCaption();
}
void DominoClient::updateWindowShape()
{
}

void DominoClient::showEvent(QShowEvent *)
{
}

void DominoClient::paintEvent( QPaintEvent* )
{
}

bool DominoClient::eventFilter(QObject*, QEvent* e)
{
	switch(e->type()) {
		case QEvent::Resize:
			resizeEvent(static_cast<QResizeEvent*>(e));
			return true;
		case QEvent::ApplicationPaletteChange:
			return false;
		case QEvent::Paint:
			paintEvent(static_cast<QPaintEvent*>(e));
			return true;
		case QEvent::MouseButtonDblClick:
			mouseDoubleClickEvent(static_cast<QMouseEvent*>(e));
			return true;
		case QEvent::Wheel:
			if(titleBar->hasMouse() || isSetShade())
				wheelEvent(static_cast<QWheelEvent*>(e));
			return true;
		case QEvent::MouseButtonPress:
			processMousePressEvent(static_cast<QMouseEvent*>(e));
			return true;
		case QEvent::Show:
			return true;
		case QEvent::Enter:
			updateButtons();
			return true;
		case QEvent::Leave:
			if(globalMouseTracking && !showInactiveButtons) {
				globalMouseTracking = false;
				qApp->setGlobalMouseTracking( false );
				qApp->removeEventFilter(titleBar);
				titleBarMouseOver = false;
			}
			updateButtons();
			return true;
		default:
			return false;
	}
	return false;
}


void TitleBar::enterEvent(QEvent*)
{
	if(!client->isActive() && !showInactiveButtons) {
		if(!client->globalMouseTracking) {
			client->globalMouseTracking = true;
			qApp->setGlobalMouseTracking( true );
			qApp->installEventFilter(this);
			client->titleBarMouseOver = true;
			client->updateButtons();
		}
	}
}


bool TitleBar::eventFilter(QObject*o, QEvent* e) {
	
	if(e->type() == QEvent::MouseMove && !showInactiveButtons && !client->isActive()) {
		QPoint pos = ((QMouseEvent*)e)->pos();
		if((dynamic_cast<QWidget*>(o) && geometry().contains(pos)) || dynamic_cast<QButton*>(o)) {
			client->titleBarMouseOver = true;
		}
		else {
			client->titleBarMouseOver = false;
			client->updateButtons();
		}
	}
	return false;
}



/////////////

void renderGradient(QPainter *painter, QRect &rect,
		    QColor &c1, QColor &c2, const char* /*name*/)
{
	if((rect.width() <= 0)||(rect.height() <= 0))
		return;
	
	QPixmap *result;
	
	result = new QPixmap(10, rect.height());
	QPainter p(result);
	
	int r_h = result->rect().height();
	int r_x, r_y, r_x2, r_y2;
	result->rect().coords(&r_x, &r_y, &r_x2, &r_y2);

	int rDiff, gDiff, bDiff;
	int rc, gc, bc;

	register int y;

	rDiff = ( c2.red())	  - (rc = c1.red());
	gDiff = ( c2.green()) - (gc = c1.green());
	bDiff = ( c2.blue())  - (bc = c1.blue());

	register int rl = rc << 16;
	register int gl = gc << 16;
	register int bl = bc << 16;

	int rdelta = ((1<<16) / r_h) * rDiff;
	int gdelta = ((1<<16) / r_h ) * gDiff;
	int bdelta = ((1<<16) / r_h ) * bDiff;

	
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
	
}


QColor alphaBlendColors(const QColor &bgColor, const QColor &fgColor, const int a)
{

	QRgb rgb = bgColor.rgb();
	QRgb rgb_b = fgColor.rgb();
	int alpha = a;
	if(alpha>255) alpha = 255;
	if(alpha<0) alpha = 0;
	int inv_alpha = 255 - alpha;

	QColor result  = QColor( qRgb(qRed(rgb_b)*inv_alpha/255 + qRed(rgb)*alpha/255,
				 qGreen(rgb_b)*inv_alpha/255 + qGreen(rgb)*alpha/255,
				 qBlue(rgb_b)*inv_alpha/255 + qBlue(rgb)*alpha/255) );

	return result;
}

QImage tintImage(const QImage &img, const QColor &tintColor) {
	
	QImage *result = new QImage(img.width(), img.height(), 32, 0, QImage::IgnoreEndian);
	unsigned int *data = (unsigned int*) img.bits();
	unsigned int *resultData = (unsigned int*) result->bits();
	result->setAlphaBuffer( true );
	int alpha;
	int total = img.width()*img.height();
	for ( int current = 0 ; current < total ; ++current ) {
		alpha = qAlpha( data[ current ] );
		resultData[ current ] = qRgba( tintColor.red(), tintColor.green(), tintColor.blue(), alpha );
	}
	return *result;
}



} // namespace

// Extended KWin plugin interface
extern "C" KDE_EXPORT KDecorationFactory* create_factory()
{
	return new Domino::DominoHandler();
}

#include "dominoclient.moc"

