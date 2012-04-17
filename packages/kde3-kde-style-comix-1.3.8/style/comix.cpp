/* Comix widget style for KDE 3
   Copyright (C) 2004 Jens Luetkens <j.luetkens@limitland.de>

   based on the KDE style "Plastik":

   Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

   based on the KDE style "dotNET":

   Copyright (C) 2001-2002, Chris Lee <clee@kde.org>
							Carsten Pfeiffer <pfeiffer@kde.org>
							Karol Szwed <gallium@kde.org>
   Drawing routines completely reimplemented from KDE3 HighColor, which was
   originally based on some stuff from the KDE2 HighColor.

   based on drawing routines of the style "Keramik":

   Copyright (c) 2002 Malte Starostik <malte@kde.org>
			 (c) 2002,2003 Maksim Orlovich <mo002j@mail.rochester.edu>
   based on the KDE3 HighColor Style
   Copyright (C) 2001-2002 Karol Szwed	  <gallium@kde.org>
			 (C) 2001-2002 Fredrik H�lund  <fredrik@kde.org>
   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley	   <mosfet@kde.org>
			 (C) 2000 Dirk Mueller		  <mueller@kde.org>
			 (C) 2001 Martijn Klingens	  <klingens@kde.org>
   Progressbar code based on KStyle,
   Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcleanuphandler.h>
#include <qdrawutil.h>
#include <qheader.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qpen.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qregion.h>
#include <qslider.h>
#include <qsettings.h>
#include <qscrollbar.h>
#include <qstylefactory.h>
#include <qstyleplugin.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qvariant.h>

#include <kpixmap.h>
#include <klistbox.h>
#include <kwin.h>

#include "comix.h"
#include "comix.moc"
#include "comixCircle.h"
#include "misc.h"

#include <kdebug.h>
#include <kcolorbutton.h>

// popupmenu item constants...
static const int itemHMargin = 6;
static const int itemFrame = 2;
static const int arrowHMargin = 6;
static const int rightBorder = 12;

static const int capRadius = 12;
static const int edgeRadius = 5;
static const int ballRadius = 7;
static const int penWidth = 2;

static const int capHeight = capRadius + capRadius;
static const int edgeHeight = edgeRadius + edgeRadius;
static const int ballHeight = ballRadius + ballRadius;

// radiomarks and checkbox frame
static const PixelMarks	radioMarks[] = {{3,0,224},{2,0,153},{1,0,64},{3,1,31},{2,1,102},{1,1,191}};
static const PixelMarks	checkFrame[] = {{2,0,161},{1,0,97},{0,0,18},{2,1,255},{1,1,255},{2,2,94}};
static const int capFill[]  = {6,3,2,1,1,1,0,0,0,0};
static const int edgeFill[] = {1,0,0};
static const int ballFill[] = {1,0,0,0,0};
static const int capGrad[]  = {10,9,8,7,6,5,4,3,2,1,0};
static const int edgeGrad[] = {10,6,1};
static const int ballGrad[] = {9,7,5,3,1};

static bool translucentMenus = false; // thanks to Matias

// -- Style Plugin Interface -------------------------
class ComixStylePlugin : public QStylePlugin
{
	public:
		ComixStylePlugin() {}
		~ComixStylePlugin() {}

		QStringList keys() const {
			return QStringList() << "Comix";
		}

		QStyle* create( const QString& key ) {
			if (key.lower() == "comix")
				return new ComixStyle;
			return 0;
		}
};

Q_EXPORT_PLUGIN( ComixStylePlugin )
// -- end --

// ---------------------------------------------------------------------------
//
// cache Constructor
//
ComixCache::ComixCache()
{
	// create the circles
	capCircle = new ComixCircle(capRadius,penWidth);
	edgeCircle = new ComixCircle(edgeRadius,penWidth);
	ballCircle = new ComixCircle(ballRadius,penWidth);
	
	// init the pixmaps to null
    for (int n=0; n<CircleCount; n++) {
        circlePixmap[n] = NULL;
        circleColor[n] = QColor(0,0,0);
        circleHeight[n] = 0;
    }

}

	
ComixCache::~ComixCache()
{
	delete capCircle;
	delete edgeCircle;
	delete ballCircle;
	
    for (int n=0; n<CircleCount; n++) {
        if (circlePixmap[n]) delete circlePixmap[n];
    }
}

QPixmap* ComixCache::getCirclePixmap(const uint &type,
					const uint &size,
					const QColor &color )
{

	if( circlePixmap[type] ) {
		if( color.rgb() != circleColor[type].rgb() || size != circleHeight[type] ) {
			delete circlePixmap[type];
			circlePixmap[type] = NULL;
		}
	}
	
	if( !circlePixmap[type] ) {
		switch( size ) {
			case SizeCap:
				circlePixmap[type] = capCircle->circlePixmap(color);
				break;
			case SizeBall:
				circlePixmap[type] = ballCircle->circlePixmap(color);
				break;
			default:
				circlePixmap[type] = edgeCircle->circlePixmap(color);
				break;
		}
		circleColor[type] = color;
		circleHeight[type] = size;
	}
	
	return circlePixmap[type];
	
}

// ---------------------------------------------------------------------------
//
// main Constructor
//
ComixStyle::ComixStyle() : KStyle( AllowMenuTransparency, ThreeButtonScrollBar ),
	kickerMode(false),
	kornMode(false),
	kAlarmMode(false),
	htmlLineEdit(false)
{
	hoverWidget = 0;

	QSettings settings;
	_contrast = settings.readNumEntry("/Qt/KDE/contrast", 4);

    settings.beginGroup("/comixstyle/Settings");
	_drawToolBarSeparator = settings.readBoolEntry("/drawToolBarSeparator", false);
	_drawToolBarItemSeparator = settings.readBoolEntry("/drawToolBarItemSeparator", false);
	_centerTabs = settings.readBoolEntry("/centerTabs", false);
	_hideFocusRect = settings.readBoolEntry("/hideFocusRect", true);
	_konsoleTabColor = settings.readBoolEntry("/konsoleTabColor", true);
	_buttonSurfaceHighlight = settings.readBoolEntry("/buttonSurfaceHighlight", false);
	_useCustomBrushColor = settings.readBoolEntry("/useCustomBrushColor", false);
	_useMenuFrame = settings.readBoolEntry("/useMenuFrame", false);
	_customBrushColor = QColor( settings.readEntry("/customBrushColor", "#808080") );
	_scrollBarStyle = settings.readEntry("/scrollBarStyle", "ThreeButtonScrollBar");
	settings.endGroup();
	
	// GUIStyle will become compatibility layer in QT4
	if( !strcmp(_scrollBarStyle,"WindowsStyleScrollBar" ) )
		this->setScrollBarType(KStyle::WindowsStyleScrollBar);
	else if( !strcmp(_scrollBarStyle,"PlatinumStyleScrollBar" ) )
		this->setScrollBarType(KStyle::PlatinumStyleScrollBar);
	else if( !strcmp(_scrollBarStyle,"NextStyleScrollBar" ) )
		this->setScrollBarType(KStyle::NextStyleScrollBar);

	_hoverColorButton = false;
	_sunkenColorButton = false;
	
	circleCache = new ComixCache();
	
}



ComixStyle::~ComixStyle()
{
	delete circleCache;
}

void ComixStyle::polish(QApplication* app)
{

	if (!qstrcmp(app->argv()[0], "kicker"))
		kickerMode = true;
	else if (!qstrcmp(app->argv()[0], "korn"))
		kornMode = true;
	else if (!qstrcmp(app->argv()[0], "kalarm"))
		kAlarmMode = true;

}

void ComixStyle::polish(QWidget* widget)
{
	// BackgroundMode will become compatibility layer in QT4
	if( !strcmp(widget->name(), "__khtml") ) { // is it a khtml widget...?
		khtmlWidgets[widget] = true;
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(khtmlWidgetDestroyed(QObject*)));
	}

	// use qt_cast where possible to check if the widget inheits one of the classes. might improve
	// performance compared to QObject::inherits()
	if ( ::qt_cast<QPushButton*>(widget) || ::qt_cast<QComboBox*>(widget) ||
			::qt_cast<QSpinWidget*>(widget) || ::qt_cast<QSlider*>(widget) ||
			::qt_cast<QCheckBox*>(widget) || ::qt_cast<QRadioButton*>(widget) ||
			::qt_cast<QToolButton*>(widget) 
	) {
		widget->installEventFilter(this);
	} else if( ::qt_cast<QLineEdit*>(widget) ) {
		widget->installEventFilter(this);
	} else if( ::qt_cast<QPopupMenu*>(widget) || ::qt_cast<QTabBar*>(widget) ) {
		widget->setBackgroundMode( NoBackground );
	}

	if ( ::qt_cast<QButton*>(widget) || ::qt_cast<QComboBox*>(widget) ) {
		widget->setBackgroundMode( PaletteBackground );
	}

	if( !qstrcmp(widget->name(), "kde toolbar widget") ) {
		// This sets the konqueror draggable label "Location: "
		// "button" background to standard background.
		// We will remove the event filter immediately afterwards.
		widget->installEventFilter(this);
	}
	else if( !qstrcmp(widget->className(), "KToolBarSeparator") ) {
		// This sets the toolbar separators background to 
		// standard background.
		widget->setBackgroundMode( PaletteBackground );
	}
	else if( !qstrcmp(widget->className(), "KonqFrameStatusBar") ) {
		// This disables the white background of the KonquerorFrameStatusBar.
		// When the green led is shown the background is set to 
		// applications cg.midlight() so we override it to standard background.
		// Same for inactive cg.mid().
		QPalette kPalette = QApplication::palette();
		kPalette.setColor( QColorGroup::Midlight,kPalette.active().background() );
		kPalette.setColor( QColorGroup::Mid,kPalette.active().background() );
		QApplication::setPalette( kPalette );
	}
	
	KStyle::polish(widget);
}

void ComixStyle::unPolish(QWidget* widget)
{
	if( !strcmp(widget->name(), "__khtml") ) { // is it a khtml widget...?
		khtmlWidgets.remove(widget);
	}

	// use qt_cast to check if the widget inheits one of the classes.
	if ( ::qt_cast<QPushButton*>(widget) || ::qt_cast<QComboBox*>(widget) ||
			::qt_cast<QSpinWidget*>(widget) || ::qt_cast<QSlider*>(widget) ||
			::qt_cast<QCheckBox*>(widget) || ::qt_cast<QRadioButton*>(widget) ||
			::qt_cast<QToolButton*>(widget) || ::qt_cast<QLineEdit*>(widget) )
	{
		widget->removeEventFilter(this);
	} else if (::qt_cast<QPopupMenu*>(widget)) {
		widget->setBackgroundMode( PaletteBackground );
	} else if( widget->inherits("KonqFrameStatusBar")
				|| widget->inherits("QFrame")
				|| widget->inherits("QViewportWidget")
	) {
		widget->removeEventFilter(this);
	}

	KStyle::unPolish(widget);
}

void ComixStyle::khtmlWidgetDestroyed(QObject* obj)
{
	khtmlWidgets.remove(static_cast<QWidget*>(obj));
}

void ComixStyle::renderContour(QPainter *p,
								const QRect &ro,
								const QColor &contour,
								const bool &enabled,
								const uint flags) const
{
	if((ro.width() <= 0)||(ro.height() <= 0))
		return;
		
	bool drawEdge = flags&Draw_Edge;
	bool drawBall = flags&Draw_Ball;
	bool drawCap = flags&Draw_Cap;
	
	bool noShrink = flags&No_Shrink;
	const bool forceCap = flags&Force_Cap;
	const bool cutTop = flags&Cut_Top;
	const bool cutBottom = flags&Cut_Bottom;
	const bool drawTop = flags&Draw_Top;
	const bool drawBottom = flags&Draw_Bottom;
	const bool toolButton = flags&Tool_Button;
	const bool customColor = flags&Custom_Color;
	const bool hover = flags&Draw_Hover;
	const bool highlight = flags&Draw_Highlight;
	
	QRect r = QRect(ro);
	QPixmap *contourPixmap = NULL;
	int isize = capRadius;
	
	if( drawBall ) {
		drawEdge = false;
		drawCap = false;
		isize = ballRadius;
		noShrink = true;
	}
	else if( drawCap && ro.height() < (capHeight - 2) && !forceCap ) {
		drawEdge = true;
		drawCap = false;
		isize = edgeRadius;
	}
	else if( !drawCap ) {
		drawEdge = true;
		drawCap = false;
		isize = edgeRadius;
	}
	
	// shrink rect by 1 pixel
	if( !noShrink ) {
		insetRect( &r, 1 );
	}

	p->setPen( contour );
	
	if( drawEdge || drawBall ) {
		p->drawLine(r.left(), r.top()+isize, r.left(), r.bottom()-isize);
		p->drawLine(r.left()+1, r.top()+isize, r.left()+1, r.bottom()-isize);
		p->drawLine(r.right(), r.top()+isize, r.right(), r.bottom()-isize);
		p->drawLine(r.right()-1, r.top()+isize, r.right()-1, r.bottom()-isize);
		p->drawLine(r.left()+isize, r.top(), r.right()-isize, r.top());
		p->drawLine(r.left()+isize, r.top()+1, r.right()-isize, r.top()+1);
		p->drawLine(r.left()+isize, r.bottom(), r.right()-isize, r.bottom());
		p->drawLine(r.left()+isize, r.bottom()-1, r.right()-isize, r.bottom()-1);
		
	}
	else if( drawCap ) {
		if( cutTop ) {
			if( drawTop ) {
				p->drawLine(r.left(), r.top(), r.right(), r.top());
				p->drawLine(r.left(), r.top()+1, r.right(), r.top()+1);
			}
			// left and right
			p->drawLine(r.left(), r.top(), r.left(), r.bottom()-capRadius);
			p->drawLine(r.left()+1, r.top(), r.left()+1, r.bottom()-capRadius);
			p->drawLine(r.right(), r.top(), r.right(), r.bottom()-capRadius);
			p->drawLine(r.right()-1, r.top(), r.right()-1, r.bottom()-capRadius);
		} else {
			p->drawLine(r.left()+capRadius, r.top(), r.right()-capRadius, r.top());
			p->drawLine(r.left()+capRadius, r.top()+1, r.right()-capRadius, r.top()+1);
		}
		if( cutBottom ) {
			if( drawBottom ) {
				p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
				p->drawLine(r.left(), r.bottom()-1, r.right(), r.bottom()-1);
			}
			// left and right
			p->drawLine(r.left(), r.top()+capRadius, r.left(), r.bottom());
			p->drawLine(r.left()+1, r.top()+capRadius, r.left()+1, r.bottom());
			p->drawLine(r.right(), r.top()+capRadius, r.right(), r.bottom());
			p->drawLine(r.right()-1, r.top()+capRadius, r.right()-1, r.bottom());
		} else {
			p->drawLine(r.left()+capRadius, r.bottom(), r.right()-capRadius, r.bottom());
			p->drawLine(r.left()+capRadius, r.bottom()-1, r.right()-capRadius, r.bottom()-1);
		}
		if( !cutTop && !cutBottom ) {
			if( r.height() > capHeight )  {
				// left and right
				p->drawLine(r.left(), r.top()+capRadius, r.left(), r.bottom()-capRadius);
				p->drawLine(r.left()+1, r.top()+capRadius, r.left()+1, r.bottom()-capRadius);
				p->drawLine(r.right(), r.top()+capRadius, r.right(), r.bottom()-capRadius);
				p->drawLine(r.right()-1, r.top()+capRadius, r.right()-1, r.bottom()-capRadius);
			}
		}
	}

	// draw corner pixmaps
	if( drawEdge ) {
		if( customColor ) contourPixmap = circleCache->getCirclePixmap(CustomColor,SizeEdge,contour);
		else if( highlight ) contourPixmap = circleCache->getCirclePixmap(HighlightEdge,SizeEdge,contour);
		else if( hover ) contourPixmap = circleCache->getCirclePixmap(HoverEdge,SizeEdge,contour);
		else if( toolButton ) contourPixmap = circleCache->getCirclePixmap(ToolbuttonEdge,SizeEdge,contour);
		else if( enabled ) contourPixmap = circleCache->getCirclePixmap(ActiveEdge,SizeEdge,contour);
		else contourPixmap = circleCache->getCirclePixmap(InactiveEdge,SizeEdge,contour);
	}
	else if( drawBall ) {
		if( customColor ) contourPixmap = circleCache->getCirclePixmap(CustomColor,SizeBall,contour);
		else if( enabled ) contourPixmap = circleCache->getCirclePixmap(ActiveBall,SizeBall,contour);
		else contourPixmap = circleCache->getCirclePixmap(InactiveBall,SizeBall,contour);
	}
	else if( drawCap ) {
		if( hover ) contourPixmap = circleCache->getCirclePixmap(HoverCap,SizeCap,contour);
		else if( enabled ) contourPixmap = circleCache->getCirclePixmap(ActiveCap,SizeCap,contour);
		else contourPixmap = circleCache->getCirclePixmap(InactiveCap,SizeCap,contour);
	}
	
	if( !cutTop ) {
		p->drawPixmap(r.left(), r.top(), *contourPixmap, 0, 0, isize, isize);
		p->drawPixmap(r.right()+1-isize, r.top(), *contourPixmap, isize, 0, isize, isize);
	}
	if( !cutBottom ) {
		p->drawPixmap(r.left(), r.bottom()+1-isize, *contourPixmap, 0, isize, isize, isize);
		p->drawPixmap(r.right()+1-isize, r.bottom()+1-isize, *contourPixmap, isize, isize, isize, isize);
	}
	
}

void ComixStyle::renderSurface(QPainter *p,
								 const QRect &rect,
								 const QColor &buttonColor,
								 const uint flags) const
{

	if((rect.width() <= 0)||(rect.height() <= 0))
		return;
	
	bool drawEdge = flags&Draw_Edge;
	bool drawBall = flags&Draw_Ball;
	bool drawCap = flags&Draw_Cap;
	const bool noShrink = flags&No_Shrink;
	const bool cutLeft = flags&Cut_Left;
	const bool cutRight = flags&Cut_Right;
	const bool flatColor = flags&Flat_Color;
	const bool sunken = flags&Is_Sunken;
	const bool forceCap = flags&Force_Cap;
	const bool cutTop = flags&Cut_Top;
	const bool cutBottom = flags&Cut_Bottom;

	QRect r = QRect(rect);

	int grad = 0;
	const int *fillMap;
	const int *fillGrad;
	
	if( drawBall ) {
		drawEdge = false;
		drawCap = false;
	}
	else if( drawCap && rect.height() < capHeight && !forceCap ) {
		drawEdge = true;
		drawCap = false;
	}
	else if( !drawCap ) {
		drawEdge = true;
		drawCap = false;
	}

	if( drawEdge ) {
		grad = edgeRadius;
		fillMap = edgeFill;
		fillGrad = edgeGrad;
		// shrink rect by 1 pixel
		if( !noShrink ) {
			insetRect( &r, 1 );
		}
	}
	else if( drawBall ) {
		grad = ballRadius;
		fillMap = ballFill;
		fillGrad = ballGrad;
	}
	else {
		grad = capRadius;
		fillMap = capFill;
		fillGrad = capGrad;
		if( !noShrink ) {
			insetRect( &r, 1 );
		}
	}

	QRect fill_Rect = QRect(r.left()+penWidth,r.top(),r.width()-penWidth-penWidth,r.height());
	if( !cutTop ) fill_Rect.setTop(fill_Rect.top()+grad);
	if( !cutBottom ) fill_Rect.setBottom(fill_Rect.bottom()-grad);

	p->setPen(buttonColor);
	insetRect( &r, penWidth );

	if(fill_Rect.height() > 0) {
		p->fillRect(fill_Rect,buttonColor);
	}
	
	int max = grad - penWidth;
	int left = 0;
	int right = 0;
	int val = 0;
	for( int i=0; i < max; i++ ) {
	
		if( !cutLeft ) left = *fillMap;
		if( !cutRight ) right = *fillMap;
		
		if( !flatColor ) {
			// the fillGrad imp is not too beatutiful but much faster than calculating
			val = 100 + (*fillGrad * _contrast / 2);
			p->setPen(sunken ? buttonColor.dark(val) : buttonColor.light(val));
		}
		
		if( !cutTop )
			p->drawLine(r.left()+left,r.top()+i,r.right()-right,r.top()+i);

		if( !flatColor ) {
			p->setPen(sunken ? buttonColor.light(val) : buttonColor.dark(val));
		}
		
		if( !cutBottom )
			p->drawLine(r.left()+left,r.bottom()-i,r.right()-right,r.bottom()-i);
		
		fillMap++;
		fillGrad++;
	}

}


void ComixStyle::renderButton(QPainter *p,
							   const QRect &r,
							   const QColorGroup &cg,
							   bool sunken,
							   bool edgeButton,
							   bool mouseOver,
							   bool enabled) const
{
	
	QColor contour;
	QColor surface;
	
	uint contourFlags;
	
	// small fix for the kicker buttons...
	if( kickerMode ) enabled = true;

	if( edgeButton ) {
		contourFlags = Draw_Edge;
	}
	else if( r.width() == capHeight && r.height() == capHeight ) {
		contourFlags = Draw_Cap|No_Shrink;
	}
	else if( r.width() < capHeight + 2 || r.height() < capHeight + 2 ) {
		contourFlags = Draw_Edge;
	}
	else {
		contourFlags = Draw_Cap;
	}
	
	if(!enabled) {
		contourFlags|=Is_Disabled;
		surface = sunken?alphaBlendColors(cg.background(), cg.button().dark(110), 127):alphaBlendColors(cg.background(), cg.button(), 127);
		contour = alphaBlendColors(cg.background(), getColor(cg, ButtonContour), 127);
	}
	else if( mouseOver ) {
		surface = sunken?cg.button().dark(110):getColor(cg, HighlightButton);
		contour = getColor(cg, HighlightContour);
		contourFlags|=Draw_Hover;
	}
	else {
		surface = sunken?cg.button().dark(110):cg.button();
		contour = getColor(cg, PanelContour);
	}
	
	if( sunken ) {
		contourFlags|=Is_Sunken;
	}
	
	renderSurface(p, r, surface, contourFlags);

	renderContour(p, r, contour, enabled, contourFlags);

}

void ComixStyle::renderHeader(QPainter *p,
								const QRect &r,
								const QColorGroup &cg,
								bool sunken,
								bool mouseOver,
								bool enabled) const
{
	if(kickerMode) enabled = true;
	
	QColor surface;
	QColor contour;

	uint contourFlags = Draw_Edge;
	if(!enabled) {
		contourFlags|=Is_Disabled;
		surface = sunken?alphaBlendColors(cg.background(), cg.button().dark(110), 127):alphaBlendColors(cg.background(), cg.button(), 127);
		contour = alphaBlendColors(cg.background(), getColor(cg, ButtonContour), 127);
	}
	else if( mouseOver ) {
		surface = sunken?cg.button().dark(110):cg.button();
		contour = getColor(cg, HighlightContour);
		contourFlags |= Draw_Hover;
	}
	else {
		surface = sunken?cg.button().dark(110):cg.button();
		contour = getColor(cg, PanelContour);
	}
	if( sunken ) {
		contourFlags|=Is_Sunken;
	}

	if( !translucentMenus ) {
		p->setPen(enabled?cg.base():cg.background());
		p->drawRect(r);
		p->drawRect(QRect(r.left()+1,r.top()+1,r.width()-2,r.height()-2));
		p->drawPoint(r.left()+3,r.top()+3);
		p->drawPoint(r.left()+3,r.bottom()-3);
		p->drawPoint(r.right()-3,r.top()+3);
		p->drawPoint(r.right()-3,r.bottom()-3);
	}
	
	renderSurface(p, r, surface, contourFlags);

	renderContour(p, r, contour, enabled, contourFlags);

}

void ComixStyle::renderPanel(QPainter *p,
							  const QRect &r,
							  const QColorGroup &cg,
							  const bool nokicker,
							  const bool sunken,
							  const bool noshrink) const
{
	int x, x2, y, y2, w, h;
	r.rect(&x,&y,&w,&h);
	r.coords(&x, &y, &x2, &y2);
	
#if (QT_VERSION < 0x030300) // pre Qt 3.4 (no KDE version flag?)
	// is this still necessary for the new kicker? no.
	nokicker = true;
#endif
	if( kickerMode && !nokicker ) {

		if( sunken ) {
			p->setPen(cg.background().light());
			p->drawRect(r);
		} else {
			p->setPen(cg.background().dark());
			p->drawRect(r);
		}

	} else {
		renderContour(p, r, getColor(cg, PanelContour), true, Draw_Edge|noshrink );
	}
	
}

void ComixStyle::renderMenuBlendPixmap( KPixmap &pix, const QColorGroup &cg,
	const QPopupMenu* /* popup */ ) const
{
	translucentMenus = true;
	pix.fill( cg.background().light(105) );
}

void ComixStyle::renderColorButtonLabel( QPainter *p, const QRect &r, const QColor &labelColor ) const
{
	// shrink to difference
	int diff;
	uint flags = Flat_Color|Custom_Color;
	QRect labelRect = r;
	
	if( r.height() < capHeight + 2 ) {
		diff = ballRadius - edgeRadius + 1;
		flags |= Draw_Edge;
	}
	else {
		diff = capRadius - ballRadius + 1;	
		flags |= Draw_Ball;
	}

	insetRect( &labelRect, diff );
	
	renderSurface( p, labelRect, labelColor, flags );

	renderContour( p, labelRect, labelColor, true, flags );

}

void ComixStyle::renderCheckFrame( QPainter *p, const QRect &r, const QColor &frameColor ) const
{

	QImage aImg(1,1,32); // 1x1@32
	aImg.setAlphaBuffer(true);
	QRgb rgb = frameColor.rgb();
	QPixmap result;
	
	p->setPen(frameColor);

	if( r.width() > 6 ) {
		p->drawLine(r.left()+3,r.top(),r.right()-3,r.top());
		p->drawLine(r.left()+3,r.top()+1,r.right()-3,r.top()+1);
		p->drawLine(r.left()+3,r.bottom(),r.right()-3,r.bottom());
		p->drawLine(r.left()+3,r.bottom()-1,r.right()-3,r.bottom()-1);
	}
	if( r.height() > 6 ) {
		p->drawLine(r.left(),r.top()+3,r.left(),r.bottom()-3);
		p->drawLine(r.left()+1,r.top()+3,r.left()+1,r.bottom()-3);
		p->drawLine(r.right(),r.top()+3,r.right(),r.bottom()-3);
		p->drawLine(r.right()-1,r.top()+3,r.right()-1,r.bottom()-3);
	}

	int i = 0;
	while( i < 6 ) {
		aImg.setPixel(0,0,qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),checkFrame[i].alpha));
		result.convertFromImage(aImg);

		// top left
		p->drawPixmap(r.left()+checkFrame[i].x,r.top()+checkFrame[i].y, result);
		p->drawPixmap(r.left()+checkFrame[i].y,r.top()+checkFrame[i].x, result);
		// bottom left
		p->drawPixmap(r.left()+checkFrame[i].x,r.bottom()-checkFrame[i].y, result);
		p->drawPixmap(r.left()+checkFrame[i].y,r.bottom()-checkFrame[i].x, result);
		// top right
		p->drawPixmap(r.right()-checkFrame[i].x,r.top()+checkFrame[i].y, result);
		p->drawPixmap(r.right()-checkFrame[i].y,r.top()+checkFrame[i].x, result);
		// bottom right
		p->drawPixmap(r.right()-checkFrame[i].x,r.bottom()-checkFrame[i].y, result);
		p->drawPixmap(r.right()-checkFrame[i].y,r.bottom()-checkFrame[i].x, result);

		i++;
	}

}

void ComixStyle::renderTab(QPainter *p,
							const QRect &r,
							const QColorGroup &cg,
							const QColor &innerColor,
							bool mouseOver,
							const bool selected,
							const bool bottom,
							const TabPosition pos,
							const bool,
							const bool konsole) const
{

	const bool reverseLayout = QApplication::reverseLayout();

	const bool isFirst = (pos == First) || (pos == Single);
	const bool isLast = (pos == Last);

	QColor surface, contour;
	uint contourFlags = Draw_Cap|Force_Cap|No_Shrink;

	if( selected ) {
		contour = getColor(cg, PanelContour);
	}
	else if( mouseOver ) {
		surface = getColor(cg, HighlightButton);
		contour = getColor(cg, HighlightContour);
		contourFlags |= Draw_Hover;
	}
	else {
		surface = cg.button();
		contour = getColor(cg, PanelContour);
	}

	QRect rc = QRect( r );
	if( reverseLayout ) rc.setLeft(rc.left() + penWidth);
	else rc.setRight(rc.right() - penWidth);
	
	//if( isFirst ) rc.setLeft(rc.left() + 4);

	if (selected) {

		if ( !bottom ) {
			contourFlags |= Cut_Bottom;
		} else {
			contourFlags |= Cut_Top;
		}
		
		if( konsole )
			renderSurface(p, rc, innerColor, contourFlags|Flat_Color);

		renderContour(p, rc, getColor(cg, PanelContour), true,  contourFlags);

		p->setPen(getColor(cg, PanelContour));
		
		if( reverseLayout ) {
			if( bottom ) {
				p->drawLine(r.left(),r.top(),r.left(),r.top()+1);
				p->drawLine(r.left()+1,r.top(),r.left()+1,r.top()+1);
			}
			else {
				p->drawLine(r.left(),r.bottom(),r.left(),r.bottom()-1);
				p->drawLine(r.left()+1,r.bottom(),r.left()+1,r.bottom()-1);
			}
		}
		else {
			if( bottom ) {
				p->drawLine(r.right(),r.top(),r.right(),r.top()+1);
				p->drawLine(r.right()-1,r.top(),r.right()-1,r.top()+1);
			}
			else {
				p->drawLine(r.right(),r.bottom(),r.right(),r.bottom()-1);
				p->drawLine(r.right()-1,r.bottom(),r.right()-1,r.bottom()-1);
			}
		}

	} else {
		// inactive tabs

		p->setPen(getColor(cg, PanelContour));

		if( !bottom ) {
			contourFlags |= Cut_Bottom|Draw_Bottom;
			rc.setHeight(rc.height()-penWidth-penWidth);
			
			// draw "bottom" line
			if( (isFirst && !reverseLayout) || (isLast && reverseLayout) ) {
				p->drawLine(r.left(),r.bottom(),r.right()+penWidth,r.bottom());
				p->drawLine(r.left(),r.bottom()-1,r.right()+penWidth,r.bottom()-1);
			}
			else if( (isFirst && reverseLayout) || (isLast && reverseLayout) ) {
				p->drawLine(r.left()-penWidth,r.bottom(),r.right(),r.bottom());
				p->drawLine(r.left()-penWidth,r.bottom()-1,r.right(),r.bottom()-1);
			}
			else {
				p->drawLine(r.left()-penWidth,r.bottom(),r.right()+penWidth,r.bottom());
				p->drawLine(r.left()-penWidth,r.bottom()-1,r.right()+penWidth,r.bottom()-1);
			}
		} else {
			contourFlags |= Cut_Top|Draw_Top;
			rc.setTop(rc.top()+penWidth+penWidth);

			// draw "bottom" line
			if( (isFirst && !reverseLayout) || (isLast && reverseLayout) ) {
				p->drawLine(r.left(),r.top(),r.right()+penWidth,r.top());
				p->drawLine(r.left(),r.top()+1,r.right()+penWidth,r.top()+1);
			}
			else if( (isFirst && reverseLayout) || (isLast && reverseLayout) ) {
				p->drawLine(r.left()-penWidth,r.top(),r.right(),r.top());
				p->drawLine(r.left()-penWidth,r.top()+1,r.right(),r.top()+1);
			}
			else {
				p->drawLine(r.left()-penWidth,r.top(),r.right()+penWidth,r.top());
				p->drawLine(r.left()-penWidth,r.top()+1,r.right()+penWidth,r.top()+1);
			}
		}

		renderSurface(p, rc, surface, contourFlags);

		renderContour(p, rc, contour, true, contourFlags);

	}

}


void ComixStyle::drawKStylePrimitive(KStylePrimitive kpe,
									  QPainter *p,
									  const QWidget* widget,
									  const QRect &r,
									  const QColorGroup &cg,
									  SFlags flags,
									  const QStyleOption& opt) const
{
	const bool enabled = (flags & Style_Enabled);
	const bool mouseOver = flags & Style_MouseOver;
	
//  SLIDER
//  ------
	switch( kpe ) {
		case KPE_SliderGroove: {
			const QSlider* slider = (const QSlider*)widget;
			bool horizontal = slider->orientation() == Horizontal;

			QColor contour;
			if( !enabled ) {
				contour = alphaBlendColors(cg.background(), getColor(cg, ButtonContour), 127);
			}
			else {
				contour = getColor(cg, PanelContour);
			}

			const bool heightDividable = ((r.height()%2) == 0);
			const bool widthDividable = ((r.width()%2) == 0);
			
			p->setPen( contour );
			
			if (horizontal) {
				int center = r.y()+r.height()/2;
				p->drawLine(r.left(), center, r.right(), center);
				if( heightDividable )
					p->drawLine(r.left(), center-1, r.right(), center-1);
				else 
					p->drawLine(r.left(), center+1, r.right(), center+1);
			} else {
				int center = r.x()+r.width()/2;
				p->drawLine(center, r.top(), center, r.bottom());
				if( widthDividable )
					p->drawLine(center-1, r.top(), center-1, r.bottom());
				else 
					p->drawLine(center+1, r.top(), center+1, r.bottom());
			}
			break;
		}

		case KPE_SliderHandle: {

			const bool pressed = (flags&Style_Active);

			QColor contour;
			QColor surface;
			uint contourFlags = Draw_Ball;

			if( pressed || mouseOver || (hoverWidget == widget) ) {
				contour = getColor(cg, HighlightContour);
				surface = getColor(cg, HighlightButton);
				contourFlags |= Draw_Hover;
			}
			else if( enabled ) {
				contour = getColor(cg, PanelContour);
				surface = cg.button();
			}
			else {
				contour = alphaBlendColors(cg.background(), getColor(cg, ButtonContour), 127);
				surface = alphaBlendColors(cg.background(), cg.button(), 127);
			}

			int xcenter = (r.left()+r.right()) / 2;
			int ycenter = (r.top()+r.bottom()) / 2;

			QRect cr = QRect(xcenter-ballRadius+1,ycenter-ballRadius+1,ballRadius+ballRadius,ballRadius+ballRadius);

			if( pressed ) contourFlags |= Is_Sunken;

			renderSurface(p, cr, surface, contourFlags);

			renderContour(p, cr, contour, enabled, contourFlags);

			break;
		}

		case KPE_DockWindowHandle:
		case KPE_ToolBarHandle:
		case KPE_GeneralHandle: {
			break;
		}

		case KPE_ListViewExpander: {
			// expand one pixel
			QRect r2 = QRect( r.x()-1, r.y()-1, r.width()+2, r.height()+2);
			int radius = (r.width() - 4) / 2;
			int centerx = r.x() + r.width()/2;
			int centery = r.y() + r.height()/2;

			renderContour(p, r2, getColor(cg,PanelContour), true, Draw_Edge|No_Shrink );

			// plus or minus
			p->setPen( cg.text() );
			p->drawLine( centerx - radius, centery, centerx + radius, centery );
			if ( flags & Style_On ) // Collapsed = On
				p->drawLine( centerx, centery - radius, centerx, centery + radius );

			break;
		}

		// copied and slightly modified from KStyle.
		case KPE_ListViewBranch: {
			// Typical Windows style listview branch element (dotted line).

			static QBitmap *verticalLine = 0, *horizontalLine = 0;
			static QCleanupHandler<QBitmap> qlv_cleanup_bitmap;

			// Create the dotline pixmaps if not already created
			if ( !verticalLine )
			{
				// make 128*1 and 1*128 bitmaps that can be used for
				// drawing the right sort of lines.
				verticalLine   = new QBitmap( 1, 129, true );
				horizontalLine = new QBitmap( 128, 1, true );
				QPointArray a( 64 );
				QPainter p2;
				p2.begin( verticalLine );

				int i;
				for( i=0; i < 64; i++ )
					a.setPoint( i, 0, i*2+1 );
				p2.setPen( color1 );
				p2.drawPoints( a );
				p2.end();
				QApplication::flushX();
				verticalLine->setMask( *verticalLine );

				p2.begin( horizontalLine );
				for( i=0; i < 64; i++ )
					a.setPoint( i, i*2+1, 0 );
				p2.setPen( color1 );
				p2.drawPoints( a );
				p2.end();
				QApplication::flushX();
				horizontalLine->setMask( *horizontalLine );

				qlv_cleanup_bitmap.add( &verticalLine );
				qlv_cleanup_bitmap.add( &horizontalLine );
			}

			p->setPen( cg.mid() );

			if (flags & Style_Horizontal)
			{
				int point = r.x();
				int other = r.y();
				int end = r.x()+r.width();
				int thickness = r.height();

				while( point < end )
				{
					int i = 128;
					if ( i+point > end )
						i = end-point;
					p->drawPixmap( point, other, *horizontalLine, 0, 0, i, thickness );
					point += i;
				}

			} else {
				int point = r.y();
				int other = r.x();
				int end = r.y()+r.height();
				int thickness = r.width();
				int pixmapoffset = (flags & Style_NoChange) ? 0 : 1;	// ### Hackish

				while( point < end )
				{
					int i = 128;
					if ( i+point > end )
						i = end-point;
					p->drawPixmap( other, point, *verticalLine, 0, pixmapoffset, thickness, i );
					point += i;
				}
			}

			break;
		}

		default:
			KStyle::drawKStylePrimitive(kpe, p, widget, r, cg, flags, opt);
	}

}


void ComixStyle::drawPrimitive(PrimitiveElement pe,
								QPainter *p,
								const QRect &r,
								const QColorGroup &cg,
								SFlags flags,
								const QStyleOption &opt ) const
{
	bool down   = flags & Style_Down;
	bool on	 = flags & Style_On;
	bool sunken = flags & Style_Sunken;
	bool horiz  = flags & Style_Horizontal;
	const bool enabled = flags & Style_Enabled;
	const bool mouseOver = flags & Style_MouseOver;
	const bool toggleButton = flags & Toggle_Button;
	
	bool hasFocus = flags & Style_HasFocus;
	
	bool noErase = false;
	if( p->device() && dynamic_cast<QPixmap*>(p->device() ) ) {
		noErase = true; // no background
	}
	

	int x = r.x();
	int y = r.y();
	int w = r.width();
	int h = r.height();

	int x2, y2;
	r.coords(&x, &y, &x2, &y2);
	
	QColor contour;
	QColor surface;
	if( mouseOver || hasFocus ) {
		contour = getColor(cg, HighlightContour);
		surface = getColor(cg, HighlightButton);
	}
	else if( enabled ) {
		contour = getColor(cg, PanelContour);
		surface = cg.button();
	}
	else {
		contour = getColor(cg, PanelContour).light(120);
		surface = alphaBlendColors(cg.background(), cg.button(), 127);
	}
	
	switch(pe) {

	
		case PE_Separator: {

			p->setPen(getColor(cg, PanelContour));
			p->drawLine(r.left(),r.top(),r.right(),r.top());

			break;
		}

#if (QT_VERSION >= 0x030300) // requires Qt 3.3
		case PE_RubberBand: {
			p->drawWinFocusRect( r );
			break;
		}
#endif

		case PE_FocusRect: {
			if( !_hideFocusRect )
				p->drawWinFocusRect( r );
			break;
		}

		case PE_HeaderSection: {
			// the taskbar buttons seems to be painted with PE_HeaderSection but I
			// want them look like normal buttons (at least for now. :) )
			bool nokicker = true;
			QWidget *mySelf = dynamic_cast<QWidget*>(p->device());
			if( !mySelf ) nokicker = false;
			
			if( nokicker ) {
				renderHeader(p, r, cg, (on||down), mouseOver||hasFocus, enabled );
				break;
			}
		}

	// BUTTONS
	// -------
		case PE_ButtonBevel: {

			if( !noErase ) p->eraseRect( r );
			renderButton(p, r, cg, (on||down), toggleButton, mouseOver||hasFocus, enabled );

			break;
		}
		case PE_ButtonDropDown:
		case PE_ButtonCommand: {

			renderButton(p, r, cg, (on||down), false, mouseOver||hasFocus, enabled );

			break;
		}

		case PE_ButtonTool: {

			uint contourFlags = Draw_Edge|No_Shrink|Tool_Button;
			if(!enabled) contourFlags|=Is_Disabled;
			
			QColor contourTool = getColor(cg, ButtonToolContour);

			renderContour(p, r, contourTool, enabled, contourFlags);
			
			break;
		}
		case PE_ButtonDefault: {

			uint contourFlags = Draw_Hover;
			
			if( r.height() < capHeight ) {

				contourFlags |= Draw_Edge;

				renderButton(p, r, cg, (on||down), false, true, enabled );

			}
			else {

				contourFlags |= Draw_Cap|Force_Cap;

				renderButton(p, QRect(r.left()-1,r.top()-1,r.width()+2,r.height()+2), cg, (on||down), false, true, enabled );

			}

			renderContour(p, r, getColor(cg, HighlightContour, enabled), enabled, contourFlags);
			
			break;
		}

		case PE_SpinWidgetPlus:
		case PE_SpinWidgetMinus: {

			p->setPen( cg.buttonText() );

			int l = QMIN( w-2, h-2 );
			// make the length even so that we get a nice symmetric plus...
			if(l%2 != 0)
				--l;
			QPoint c = r.center();

			p->drawLine( c.x()-l/2, c.y(), c.x()+l/2, c.y() );
			if ( pe == PE_SpinWidgetPlus ) {
				p->drawLine( c.x(), c.y()-l/2, c.x(), c.y()+l/2 );
			}

			break;
		}

		case PE_ScrollBarSlider: {
			
			QColor fillColor = cg.base();

			if( p->device() ) {
				const QScrollBar* sb = dynamic_cast<QScrollBar*>(p->device());
				if ( sb ) {
					QWidget* parent = sb->parentWidget(true);
					if( parent ) {
						if ( !parent->erasePixmap() )
							fillColor = parent->backgroundColor();
						else 
							fillColor = cg.background();
					}
				}
			}

			p->fillRect(r, fillColor);

			// shrink 1 pixel;
			QRect cr = QRect(r.left()+1,r.top()+1,r.width()-2,r.height()-2);
			uint contourFlags = Draw_Ball;
			if( down ) contourFlags |= Is_Sunken;

			renderSurface(p, cr, surface, contourFlags);

			renderContour(p, cr, contour, enabled, contourFlags);

			// connect do slider "groove" (AddPage, SubPage)
			p->setPen(contour);

			if( horiz ) {
				int half = r.height()/2;
					p->drawPoint(r.left(),r.top()+half);
					p->drawPoint(r.left(),r.top()+half-1);
					p->drawPoint(r.right(),r.top()+half);
					p->drawPoint(r.right(),r.top()+half-1);
					p->drawPoint(r.right()+1,r.top()+half);
					p->drawPoint(r.right()+1,r.top()+half-1);
			}
			else {
				int half = r.width()/2;
					p->drawPoint(r.left()+half,r.top());
					p->drawPoint(r.left()+half-1,r.top());
					p->drawPoint(r.left()+half,r.bottom());
					p->drawPoint(r.left()+half-1,r.bottom());
					p->drawPoint(r.left()+half,r.bottom()+1);
					p->drawPoint(r.left()+half-1,r.bottom()+1);
			}

			break;
		}

		case PE_ScrollBarAddPage:
		case PE_ScrollBarSubPage: {

			QColor fillColor = cg.base();

			if( p->device() ) {
				const QScrollBar* sb = dynamic_cast<QScrollBar*>(p->device());
				if ( sb ) {
					QWidget* parent = sb->parentWidget(true);
					if( parent ) {
						if ( !parent->erasePixmap() ) {
							fillColor = parent->backgroundColor();
						}
						else 
							fillColor = cg.background();
					}
				}
			}

			p->fillRect(r, fillColor);
			
			p->setPen(contour);

			if( horiz ) {
				int half = r.height()/2;
				p->drawLine(r.left(), r.top()+half, r.right()+1, r.top()+half);
				p->drawLine(r.left(), r.top()+half-1, r.right()+1, r.top()+half-1);
			} else {
				int half = r.width()/2;
				p->drawLine(r.left()+half, r.top(), r.left()+half, r.bottom()+1);
				p->drawLine(r.left()+half-1, r.top(), r.left()+half-1, r.bottom()+1);
			}

			break;
		}

	// SCROLLBAR BUTTONS
	// -----------------
		case PE_ScrollBarSubLine: {

			QColor fillColor = cg.base();

			if( p->device() ) {
				const QScrollBar* sb = dynamic_cast<QScrollBar*>(p->device());
				if ( sb ) {
					QWidget* parent = sb->parentWidget(true);
					if( parent ) {
						if ( !parent->erasePixmap() )
							fillColor = parent->backgroundColor();
						else 
							fillColor = cg.background();
					}
				}
			}

			p->fillRect(r, fillColor);

			// scroll up or left
			// shrink 1 pixel
			QRect cr = QRect(r.left()+1,r.top()+1,r.width()-2,r.height()-2);

			uint contourFlags = Draw_Ball;
			if( down ) contourFlags |= Is_Sunken;

			renderSurface(p, cr, surface, contourFlags);

			renderContour(p, cr, contour, enabled, contourFlags);

			// connect 
			p->setPen(contour);

			if( horiz ) {
				int half = r.height()/2;
				p->drawPoint(r.right(),r.top()+half);
				p->drawPoint(r.right(),r.top()+half-1);
			}
			else {
				int half = r.width()/2;
				p->drawPoint(r.left()+half,r.bottom());
				p->drawPoint(r.left()+half-1,r.bottom());
			}

			p->setPen(cg.foreground());
			drawPrimitive((horiz ? PE_ArrowLeft : PE_ArrowUp), p, r, cg, flags);

			break;
		}

		case PE_ScrollBarAddLine: {
			
			QColor fillColor = cg.base();

			if( p->device() ) {
				const QScrollBar* sb = dynamic_cast<QScrollBar*>(p->device());
				if ( sb ) {
					QWidget* parent = sb->parentWidget(true);
					if( parent ) {
						if ( !parent->erasePixmap() )
							fillColor = parent->backgroundColor();
						else 
							fillColor = cg.background();
					}
				}
			}

			p->fillRect(r, fillColor);
			
			// scroll down or right
			// shrink 1 pixel
			QRect cr = QRect(r.left()+1,r.top()+1,r.width()-2,r.height()-2);
			
			uint contourFlags = Draw_Ball;
			if( down ) contourFlags |= Is_Sunken;

			renderSurface(p, cr, surface, contourFlags);

			renderContour(p, cr, contour, enabled, contourFlags);

			// connect 
			p->setPen(contour);

			if( horiz ) {
				int half = r.height()/2;
				p->drawPoint(r.left(),r.top()+half);
				p->drawPoint(r.left(),r.top()+half-1);
			}
			else {
				int half = r.width()/2;
				p->drawPoint(r.left()+half,r.top());
				p->drawPoint(r.left()+half-1,r.top());
			}

			p->setPen(cg.foreground());
			drawPrimitive((horiz ? PE_ArrowRight : PE_ArrowDown), p, r, cg, flags);
			break;
		}

	// CHECKBOXES
	// ----------
		case PE_Indicator: {

			QColor contentColor = enabled?cg.base():cg.background();

			uint contourFlags = Draw_Edge|No_Shrink;
			if(!enabled)  contourFlags |= Is_Disabled;
			if( down )  contourFlags |= Is_Sunken;
			
			renderSurface(p, r, contentColor, contourFlags);
			
			renderCheckFrame(p, r, contour);

			drawPrimitive(PE_CheckMark, p, r, cg, flags);

			break;
		}

/*
		case PE_IndicatorMask: {

			//p->fillRect (r, Qt::white);

			break;
		}
*/
	// RADIOBUTTONS
	// ------------
		case PE_ExclusiveIndicator: {
			
			QColor contentColor = enabled?cg.base():cg.background();
			
			uint contourFlags = Draw_Ball;

			// first the surface...
			if(!enabled)  contourFlags |= Is_Disabled;
			if( down )  contourFlags |= Is_Sunken;

			renderSurface(p, r, contentColor, contourFlags);

			renderContour(p, r, contour, enabled, contourFlags);


			break;
		}

/*
		case PE_ExclusiveIndicatorMask: {

			p->fillRect(r, Qt::white);

			break;
		}
*/
	// GENERAL PANELS
	// --------------
		case PE_Splitter: {
			p->fillRect(r,cg.background());
			break;
		}

		case PE_PanelGroupBox:
		case PE_GroupBoxFrame: {
		
			bool nokicker = true;
			QWidget *mySelf = dynamic_cast<QWidget*>(p->device());
			if( !mySelf ) nokicker = false;
			
			renderPanel(p, r, cg, nokicker);

			break;
		}

		case PE_WindowFrame: {

			p->setPen(getColor(cg,PanelContour));
			p->drawRect(r);
			p->drawRect(QRect(r.left()+1,r.top()+1,r.width()-2,r.height()-2));

			break;
		}
		case PE_Panel: {
		
			bool nokicker = true;

			QWidget *mySelf = dynamic_cast<QWidget*>(p->device());
			if( !mySelf ) nokicker = false;
			else if( !strcmp(mySelf->className(),"FittsLawFrame") ) nokicker = false;

			//if( !kickerMode && mySelf ) {
			if( mySelf ) {
			
				// and be shure of the background
				p->setPen(cg.background());
				QRect frameRect = QRect(r.left()+1,r.top()+1,r.width()-2,r.height()-2);
				QRect frameRect2 = QRect(r.left()+2,r.top()+2,r.width()-4,r.height()-4);
				p->drawRect(r);
				p->drawRect(frameRect);
				p->drawRect(frameRect2);

				// top, bottom, left, right
				p->drawLine(r.left()+penWidth+1, r.top()+penWidth+1, r.right()-penWidth-1, r.top()+penWidth+1);
				p->drawLine(r.left()+penWidth+1, r.bottom()-penWidth-1, r.right()-penWidth-1, r.bottom()-penWidth-1);
				p->drawLine(r.left()+penWidth+1, r.top()+penWidth+1, r.left()+penWidth+1, r.bottom()-penWidth-1);
				p->drawLine(r.right()-penWidth-1, r.top()+penWidth+1, r.right()-penWidth-1, r.bottom()-penWidth-1);

			}

			renderPanel(p, r, cg, nokicker, sunken, false);
			
			break;
		}

		case PE_PanelLineEdit: {

			bool isReadOnly = false;
			bool isEnabled = true;
			
			// panel is highlighted by default if it has focus, but if we have access to the
			// widget itself we can try to avoid highlighting in case it's readOnly or disabled.
			if (p->device() && dynamic_cast<QLineEdit*>(p->device()))
			{
				QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(p->device());

				isReadOnly = lineEdit->isReadOnly();
				isEnabled = lineEdit->isEnabled();
				
			}
			// thanks to ceebx for the hack
			if( !noErase ) {
				QRect frameRect1 = QRect(r.left()+1,r.top()+1,r.width()-2,r.height()-2);
				QRect frameRect2 = QRect(r.left()+2,r.top()+2,r.width()-4,r.height()-4);
				p->setPen(cg.background());
				p->drawRect(r);
				p->drawRect(frameRect1);
				p->drawRect(frameRect2);
			}

			QRect whiteRect = QRect(r.left()+penWidth+1,r.top()+penWidth+1,r.width()-penWidth-penWidth-2,r.height()-penWidth-penWidth-2);
			if( isEnabled && !isReadOnly ) {
				p->setPen( cg.base() );
			}
			else {
				p->setPen( cg.background() );
			}
			p->drawRect( whiteRect );
			

			QColor contour;
			if( !enabled ) {
				contour = alphaBlendColors(cg.background(), getColor(cg, ButtonContour), 127);
			} else if( hasFocus ) {
				contour = getColor(cg, HighlightContour);
			} else {
				contour = getColor(cg, PanelContour);
			}
			
			//
			// todo: you left side panel outer frame
			//
			
			renderContour(p, r, contour, enabled, Draw_Edge );
			
			break;
		}

		case PE_StatusBarSection: {
		
			// No effect on konqueror status bar *frame* in file-browsng mode.
			// The items' background are taken from parent, so we don't
			// need to draw the background here.

			break;
		}

		case PE_PanelTabWidget: {
		
			QColor innerColor = cg.background();

			if( _konsoleTabColor ) {
				QWidget *mySelf = dynamic_cast<QWidget*>(p->device());
				if( mySelf ) 
				if( mySelf->parentWidget() ) {
					QWidget *parent = mySelf->parentWidget();
					if( parent ) 
					if( parent->parentWidget() ) 
					if( !strncmp(parent->parentWidget()->name(),"konsole",7)) {
						QWidget *innerWidget = 0;
						innerWidget = mySelf->childAt(r.left()+penWidth+2,r.top()+penWidth+2);
						if( innerWidget ) {
							if ( !innerWidget->erasePixmap() ) {
								innerColor = innerWidget->backgroundColor();
							}
						}
					}
				}
			}
			
			p->setPen(innerColor);
			// top, bottom, left, right
			p->drawLine(r.left()+penWidth+1, r.top()+penWidth+1, r.right()-penWidth-1, r.top()+penWidth+1);
			p->drawLine(r.left()+penWidth+1, r.bottom()-penWidth-1, r.right()-penWidth-1, r.bottom()-penWidth-1);
			p->drawLine(r.left()+penWidth+1, r.top()+penWidth+1, r.left()+penWidth+1, r.bottom()-penWidth-1);
			p->drawLine(r.right()-penWidth-1, r.top()+penWidth+1, r.right()-penWidth-1, r.bottom()-penWidth-1);

			renderPanel(p, r, cg, true, sunken);
					
			break;
		}
		
		case PE_PanelPopup: {

			if( !translucentMenus ) {
				// need to fill 1 pixel inner border
				p->setPen(cg.background().light(105));
				// top, bottom, left, right
				p->drawLine(r.left()+penWidth, r.top()+penWidth, r.right()-penWidth, r.top()+penWidth);
				p->drawLine(r.left()+penWidth, r.bottom()-penWidth, r.right()-penWidth, r.bottom()-penWidth);
				p->drawLine(r.left()+penWidth, r.top()+penWidth, r.left()+penWidth, r.bottom()-penWidth);
				p->drawLine(r.right()-penWidth, r.top()+penWidth, r.right()-penWidth, r.bottom()-penWidth);
			}
			
			renderContour(p, r, getColor(cg, PanelContour, enabled), enabled, Draw_Edge|No_Shrink);

			break;
		}

	// MENU / TOOLBAR PANEL
	// --------------------
		case PE_PanelMenuBar:
		case PE_PanelDockWindow: {

			// fix for toolbar lag (from Mosfet Liquid) 
			QWidget* w = dynamic_cast<QWidget*>(p->device());
			if(w && w->backgroundMode() == PaletteButton) 
				w->setBackgroundMode(PaletteBackground);
			
			// the tool Bar itself
			if( !kickerMode ) {
				p->setPen( cg.background() );
				p->drawRect( r );
			}
			
			if ( _drawToolBarSeparator ) {
				p->setPen( getColor(cg, PanelContour) );
				if ( r.width() > r.height() ) {
					p->drawLine( r.left(), r.top(), r.right(), r.top() );
					p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
					//kdDebug() << "horizaontal\n";
				}
				else {
					p->drawLine( r.left(), r.top(), r.left(), r.bottom() );
					p->drawLine( r.right(), r.top(), r.right(), r.bottom() );
					//kdDebug() << "vertical\n";
				}
			}

			break;
		}

	// TOOLBAR/DOCK WINDOW HANDLE
	// --------------------------
		case PE_DockWindowResizeHandle: {
			renderButton(p, r, cg);
			break;
		}

		case PE_DockWindowHandle: {

			// tear-off handles
			if(!kickerMode) {
				p->fillRect(r,cg.background());
				renderContour(p, r, getColor(cg, ButtonToolContour), enabled, Draw_Edge|Tool_Button);
			}
			break;
		}

	// TOOLBAR SEPARATOR
	// -----------------
		case PE_DockWindowSeparator: {

			// dock window (buttons) vertical separator
			if ( _drawToolBarItemSeparator ) {

				p->setPen( getColor(cg, PanelContour) );

				QPoint center = r.center();
				if( horiz ) { // dock window is horizontal
					p->drawLine(center.x(),r.top()+penWidth,center.x(),r.bottom()-penWidth);
					p->drawLine(center.x()+1,r.top()+penWidth,center.x()+1,r.bottom()-penWidth);
				}
				else {
					p->drawLine(r.left()+penWidth,center.y(),r.right()-penWidth,center.y());
					p->drawLine(r.left()+penWidth,center.y()+1,r.right()-penWidth,center.y()+1);
				}
			}
			break;
		}

		case PE_CheckMark: {
			const QColor contentColor = enabled?cg.base():cg.background();
	   		QImage aImg(1,1,32); // 1x1@32
			aImg.setAlphaBuffer(true);
			QPixmap result;

			QColor checkmarkColor = enabled?cg.highlight():cg.background();
			QRgb rgb = contour.rgb();

			int x = r.center().x() - 3, y = r.center().y() - 3;
			QBitmap bmp;
			if( flags & Style_On ) {

				// frame
				p->setPen(contour);
				p->drawLine(x+1,y,x+6,y);
				p->drawLine(x,y+1,x,y+6);
				p->drawLine(x+1,y+7,x+6,y+7);
				p->drawLine(x+7,y+1,x+7,y+6);

				// full alpha
				aImg.setPixel(0,0,qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),127));
				result.convertFromImage(aImg);
				p->drawPixmap(x,y, result);
				p->drawPixmap(x+7,y, result);
				p->drawPixmap(x,y+7, result);
				p->drawPixmap(x+7,y+7, result);
				// fill
				p->setPen(checkmarkColor.light(100 + (_contrast * 3)));
				p->drawLine(x+1,y+1,x+6,y+1);
				p->setPen(checkmarkColor.light(100 + (_contrast * 2)));
				p->drawLine(x+1,y+2,x+6,y+2);
				p->setPen(checkmarkColor.light(100 + _contrast ));
				p->drawLine(x+1,y+3,x+6,y+3);
				p->setPen(checkmarkColor.dark(100 + _contrast ));
				p->drawLine(x+1,y+4,x+6,y+4);
				p->setPen(checkmarkColor.dark(100 + (_contrast * 2)));
				p->drawLine(x+1,y+5,x+6,y+5);
				p->setPen(checkmarkColor.dark(100 + (_contrast * 3)));
				p->drawLine(x+1,y+6,x+6,y+6);

			} else if ( flags & Style_Off ) {
				// empty
			} else { // tristate

				p->setPen(contour);

				p->drawLine(x+1,y+2,x+6,y+2);
				p->drawLine(x+1,y+4,x+6,y+4);
				p->drawLine(x+1,y+6,x+6,y+6);

			}

			break;
		}

		case PE_SpinWidgetUp:
		case PE_SpinWidgetDown:
		case PE_HeaderArrow:
		case PE_ArrowUp:
		case PE_ArrowDown:
		case PE_ArrowLeft:
		case PE_ArrowRight: {
		
			QPointArray a;
			
			p->setPen(getColor(cg, TextColor, enabled));
			
			switch (pe) {
				case PE_SpinWidgetUp:
				case PE_ArrowUp: {
					a.setPoints(7, u_arrow);
					break;
				}
				case PE_SpinWidgetDown:
				case PE_ArrowDown: {
					a.setPoints(7, d_arrow);
					break;
				}
				case PE_ArrowLeft: {
					a.setPoints(7, l_arrow);
					break;
				}
				case PE_ArrowRight: {
					a.setPoints(7, r_arrow);
					break;
				}
				default: {
					if (flags & Style_Up) {
						a.setPoints(7, u_arrow);
					} else {
						a.setPoints(7, d_arrow);
					}
				}
			}

			a.translate((r.x()+r.width()/2), (r.y()+r.height()/2));
			// extra-pixel-shift, correcting some visual tics...
			switch(pe) {
				case PE_ArrowLeft:
				case PE_ArrowRight:
					a.translate(0, -1);
					break;
				case PE_SpinWidgetUp:
				case PE_SpinWidgetDown:
					a.translate(+1, 0);
					break;
				default:
					a.translate(0, 0);
			}

			if (p->pen() == QPen::NoPen) {
				if (flags & Style_Enabled) {
					p->setPen(cg.buttonText());
				} else {
					p->setPen(cg.highlightedText());
				}
			}
			p->drawLineSegments(a, 0, 3);
			p->drawPoint(a[6]);
			
			break;
		}

		default: {
			return KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
		}
	}
	
}


void ComixStyle::drawControl(ControlElement element,
							  QPainter *p,
							  const QWidget *widget,
							  const QRect &r,
							  const QColorGroup &cg,
							  SFlags flags,
							  const QStyleOption& opt) const
{
	const bool reverseLayout = QApplication::reverseLayout();

	const bool enabled = (flags & Style_Enabled);
	
	switch (element) {

	// PROGRESSBAR
	// -----------
		case CE_ProgressBarGroove: {
			break;
		}

		case CE_ProgressBarContents: {

			const uint contourFlags = Draw_Cap|No_Shrink;
			int whichRadius;
			if( r.height() < capHeight ) 
				whichRadius = edgeRadius;
			else
				whichRadius = capRadius;

			const QProgressBar *pb = dynamic_cast<const QProgressBar*>(widget);
			int steps = pb->totalSteps();

			const QColor bg = cg.background(); // background
			const QColor fg = enabled?cg.highlight():cg.background(); // foreground

			if( steps == 0 ) { // Busy indicator

				renderSurface(p, r, bg, contourFlags|Is_Sunken);

			} else {
				double percent = static_cast<double>(pb->progress()) / static_cast<double>(steps);

				int w = static_cast<int>(r.width() * percent);
				int w2 = r.width()-w;

				if( w < whichRadius && percent > 0.00 ) w = whichRadius;
				if( w > (r.width()-whichRadius) && w2 > 0 ) {
					w = r.width()-whichRadius;
					w2 = whichRadius;
				}
				uint surfaceFlags = contourFlags;

				QRect Rsurface(reverseLayout?r.right()-w:r.left(), r.top(), w+2, r.height());
				QRect Rempty(reverseLayout?r.left():r.left()+w, r.top(), w2, r.height() );

				if( w < 1 ) {
					renderSurface(p, Rempty, bg, surfaceFlags|Is_Sunken);
				}
				else if( w2 < 1 ) {
					renderSurface(p, Rsurface, fg, surfaceFlags);
				}
				else {
					renderSurface(p, Rsurface, fg, surfaceFlags|(reverseLayout?Cut_Left:Cut_Right));
					renderSurface(p, Rempty, bg, surfaceFlags|Is_Sunken|(reverseLayout?Cut_Right:Cut_Left));
				}					
				// additional vertical line
				if( w > 0 ) {
					p->setPen(getColor(cg, PanelContour, enabled));
					if( reverseLayout ) {
						p->drawLine(r.right()-w,r.top()+penWidth,r.right()-w,r.bottom()-penWidth);
						p->drawLine(r.right()-w-1,r.top()+penWidth,r.right()-w-1,r.bottom()-penWidth);
					}
					else {
						p->drawLine(r.left()+w,r.top()+penWidth,r.left()+w,r.bottom()-penWidth);
						p->drawLine(r.left()+w+1,r.top()+penWidth,r.left()+w+1,r.bottom()-penWidth);
					}
				}
				
			}

			renderContour(p, r, getColor(cg, PanelContour, enabled), enabled, contourFlags );
			
			break;
		}

	// RADIOBUTTONS
	// ------------
		case CE_RadioButton: {
			
			drawPrimitive(PE_ExclusiveIndicator, p, r, cg, flags);

			const QColor contentColor = enabled?cg.base():cg.background();
			QColor checkmarkColor = enabled?cg.highlight():cg.background();
			QColor contour;
			if( widget == hoverWidget ) contour = getColor(cg,HighlightContour);
			else if( enabled ) contour = getColor(cg,PanelContour);
			else contour = alphaBlendColors(cg.background(), getColor(cg,ButtonContour), 127);

			if (flags & Style_On || flags & Style_Down) {
				
				QRect ir = QRect(r.left()+3,r.top()+3,r.width()-6,r.height()-6);
				int x = r.left()+3;
				int y = r.top()+3;

		   		QImage aImg(1,1,32); // 1x1@32
				aImg.setAlphaBuffer(true);
				QRgb rgb = contour.rgb();
				QPixmap result;

				p->setPen(checkmarkColor.light(100 + (_contrast * 3)));
				p->drawLine(x+1,y+1,x+6,y+1);
				p->setPen(checkmarkColor.light(100 + (_contrast * 2)));
				p->drawLine(x+1,y+2,x+6,y+2);
				p->setPen(checkmarkColor.light(100 + _contrast));
				p->drawLine(x+1,y+3,x+6,y+3);
				p->setPen(checkmarkColor.dark(100 + _contrast ));
				p->drawLine(x+1,y+4,x+6,y+4);
				p->setPen(checkmarkColor.dark(100 + (_contrast * 2)));
				p->drawLine(x+1,y+5,x+6,y+5);
				p->setPen(checkmarkColor.dark(100 + (_contrast * 3)));
				p->drawLine(x+1,y+6,x+6,y+6);

				int i = 0;
				while( i < 6 ) {
					aImg.setPixel(0,0,qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),radioMarks[i].alpha));
					result.convertFromImage(aImg);
					// top left
					p->drawPixmap(ir.left()+radioMarks[i].x,ir.top()+radioMarks[i].y, result);
					p->drawPixmap(ir.left()+radioMarks[i].y,ir.top()+radioMarks[i].x, result);
					// bottom left
					p->drawPixmap(ir.left()+radioMarks[i].x,ir.bottom()-radioMarks[i].y, result);
					p->drawPixmap(ir.left()+radioMarks[i].y,ir.bottom()-radioMarks[i].x, result);
					// top right
					p->drawPixmap(ir.right()-radioMarks[i].x,ir.top()+radioMarks[i].y, result);
					p->drawPixmap(ir.right()-radioMarks[i].y,ir.top()+radioMarks[i].x, result);
					// bottom right
					p->drawPixmap(ir.right()-radioMarks[i].x,ir.bottom()-radioMarks[i].y, result);
					p->drawPixmap(ir.right()-radioMarks[i].y,ir.bottom()-radioMarks[i].x, result);
					
					i++;
				}
				
			}

			break;
		}

	// TABS
	// ----
		case CE_TabBarTab: {

			const QTabBar *tb = dynamic_cast<const QTabBar *>(widget);
			QTabBar::Shape tbs = tb->shape();

			const bool focus = tb->hasFocus();
			bool selected = false;
			if (flags & Style_Selected) selected = true;

			TabPosition pos;
			if (tb->count() == 1) {
				pos = Single;
			} else if ((tb->indexOf(opt.tab()->identifier()) == 0)) {
				pos = First;
			} else if (tb->indexOf(opt.tab()->identifier()) == tb->count() - 1) {
				pos = Last;
			} else {
				pos = Middle;
			}

			bool tabsBelow = false;
			switch (tbs) {
				case QTabBar::TriangularAbove:
				case QTabBar::RoundedAbove:
					tabsBelow = false;
					break;
				case QTabBar::TriangularBelow:
				case QTabBar::RoundedBelow:
					tabsBelow = true;
					break;
			}

			QColor innerColor = cg.background();
			bool konsole = false;

			if( _konsoleTabColor ) {
				QWidget *parent = tb->parentWidget();
				if( parent ) 
				if( parent->parentWidget() ) 
				if( !strncmp(parent->parentWidget()->name(),"konsole",7)) {
					QRect pr = parent->rect();
					QWidget *firstChild = parent->childAt(pr.left()+4,tabsBelow?pr.top()+4:pr.bottom()-6);
					if( firstChild )
					if( !firstChild->erasePixmap() ) { 
						innerColor = firstChild->paletteBackgroundColor();
						konsole = true;
					}
				}
			}

			renderTab(p, r, cg, innerColor, (flags & Style_MouseOver), selected, tabsBelow, pos, focus, konsole);
			
			if( selected && konsole && _konsoleTabColor ) {
				p->setPen(innerColor);
				if( tabsBelow ) {
					if( reverseLayout ) {
						p->drawLine(r.left()+penWidth+penWidth,r.top(),r.right()-penWidth,r.top());
						p->drawLine(r.left()+penWidth+penWidth,r.top()+1,r.right()-penWidth,r.top()+1);
					}
					else {
						p->drawLine(r.left()+penWidth,r.top(),r.right()-penWidth-penWidth,r.top());
						p->drawLine(r.left()+penWidth,r.top()+1,r.right()-penWidth-penWidth,r.top()+1);
					}
				}
				else {
					if( reverseLayout ) {
						p->drawLine(r.left()+penWidth+penWidth,r.bottom(),r.right()-penWidth,r.bottom());
						p->drawLine(r.left()+penWidth+penWidth,r.bottom()-1,r.right()-penWidth,r.bottom()-1);
					}
					else {
						p->drawLine(r.left()+penWidth,r.bottom(),r.right()-penWidth-penWidth,r.bottom());
						p->drawLine(r.left()+penWidth,r.bottom()-1,r.right()-penWidth-penWidth,r.bottom()-1);
					}
				}

			}

			break;
		}
		case CE_PushButton: {
			QPushButton *button = (QPushButton *)widget;

			const bool isDefault = enabled && button->isDefault();
			
			if (widget == hoverWidget)
				flags |= Style_MouseOver;

			if( button && button->isToggleButton() )
				flags |= Toggle_Button;
			
			if( isDefault ) {
				drawPrimitive(PE_ButtonDefault, p, r, cg, flags);
			}
			else {
				drawPrimitive(PE_ButtonBevel, p, r, cg, flags, QStyleOption(button) );
			}

			break;
		}

		case CE_PushButtonLabel:
		{
		
			const QPushButton* button = dynamic_cast<const QPushButton *>( widget );
			if( button && button->isToggleButton() ) {

				// konqueror side bar
				KStyle::drawControl(element, p, widget, r, cg, flags, opt);
				break;

			}

			int x, y, w, h;
			r.rect( &x, &y, &w, &h );

			bool active = button->isOn() || button->isDown();
			bool cornArrow = false;

			// Shift button contents if pushed.
			if ( active )
			{
				x += pixelMetric(PM_ButtonShiftHorizontal, widget);
				y += pixelMetric(PM_ButtonShiftVertical, widget);
				flags |= Style_Sunken;
			}
			
			// Does the button have a popup menu?
			if ( button->isMenuButton() )
			{
				int dx = pixelMetric( PM_MenuButtonIndicator, widget );
				if ( button->iconSet() && !button->iconSet()->isNull()  &&
					(dx + button->iconSet()->pixmap (QIconSet::Small, QIconSet::Normal, QIconSet::Off ).width()) >= w )
				{
					cornArrow = true; //To little room. Draw the arrow in the corner, don't adjust the widget
				}
				else
				{
					drawPrimitive( PE_ArrowDown, p, visualRect( QRect(x + w - dx - 2, y + 2, dx, h - 4), r ),
								cg, flags, opt );
					w -= dx;
				}
			}

			// Draw the icon if there is one
			if ( button->iconSet() && !button->iconSet()->isNull() )
			{
				QIconSet::Mode  mode  = QIconSet::Disabled;
				QIconSet::State state = QIconSet::Off;
				
				if (button->isEnabled())
					mode = button->hasFocus() ? QIconSet::Active : QIconSet::Normal;
				if (button->isToggleButton() && button->isOn())
					state = QIconSet::On;

				QPixmap pixmap = button->iconSet()->pixmap( QIconSet::Small, mode, state );

				if (button->text().isEmpty() && !button->pixmap())
					p->drawPixmap( x + w/2 - pixmap.width()/2, y + h / 2 - pixmap.height() / 2,
									pixmap );
				else
					p->drawPixmap( x + capRadius, y + h / 2 - pixmap.height() / 2, pixmap );

				if (cornArrow) {
					//Draw over the icon
					drawPrimitive( PE_ArrowDown, p, visualRect( QRect(x + w - 4, x + h - 3, 7, 7), r ), cg, flags, opt );
				}

				int  pw = pixmap.width();
				x += pw + edgeRadius;
				w -= pw + edgeRadius;
			}

			// Make the label indicate if the button is a default button or not
			drawItem( p, QRect(x, y, w, h), AlignCenter|ShowPrefix, button->colorGroup(),
						button->isEnabled(), button->pixmap(), button->text(), -1,
						&button->colorGroup().buttonText() );


			if ( flags & Style_HasFocus )
				drawPrimitive( PE_FocusRect, p,
								visualRect( subRect( SR_PushButtonFocusRect, widget ), widget ),
								cg, flags );
			break;
		}

	// MENUBAR ITEM (sunken panel on mouse over)
	// -----------------------------------------
		case CE_MenuBarItem: {
			QMenuItem *mi = opt.menuItem();
			bool active  = flags & Style_Active;
			bool focused = flags & Style_HasFocus;
			bool down = flags & Style_Down;
			const int text_flags =
				AlignVCenter | AlignHCenter | ShowPrefix | DontClip | SingleLine;

			// menu bar buttons
			p->fillRect(r, cg.background());

			if (active && focused) {
				if (down) {
					drawPrimitive(PE_ButtonTool, p, r, cg, flags|Style_Down|Draw_Ball, opt);
				} else {
					drawPrimitive(PE_ButtonTool, p, r, cg, flags|Draw_Ball, opt);
				}
			}
			
			p->setPen(cg.foreground() );
			p->drawText(r, text_flags, mi->text());
			break;
		}

	// POPUPMENU ITEM (highlighted on mouseover)
	// ------------------------------------------
		case CE_PopupMenuItem: {
			const QPopupMenu *popupmenu = static_cast< const QPopupMenu * >( widget );
			QMenuItem *mi = opt.menuItem();

			if ( !mi )
			{
				// Don't leave blank holes if we set NoBackground for the QPopupMenu.
				// This only happens when the popupMenu spans more than one column.
				if (! ( widget->erasePixmap() && !widget->erasePixmap()->isNull() ) )
					p->fillRect( r, cg.background().light( 105 ) );

				break;
			}

			int  tab		= opt.tabWidth();
			int  checkcol   = opt.maxIconWidth();
			bool enabled	= mi->isEnabled();
			bool checkable  = popupmenu->isCheckable();
			bool active	 = flags & Style_Active;
			bool etchtext   = true;
			bool reverse	= QApplication::reverseLayout();
			int darken = 100 + _contrast;
			
			if ( checkable )
				checkcol = QMAX( checkcol, 20 );

			// Draw the menu item background
			if (active) {
				if (enabled) {
					
					//renderSurface(p, r, cg.highlight(), Draw_Edge|No_Shrink|Flat_Color);
					if( !_useMenuFrame )
						renderSurface(p, r, cg.highlight(), Draw_Edge|No_Shrink|Flat_Color);
				
					renderContour(p, r, cg.highlight().dark(darken), enabled, Draw_Edge|No_Shrink|Draw_Highlight);
				
				}
				else {
					if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() )
						p->drawPixmap( r.topLeft(), *widget->erasePixmap(), r );
					else 
						p->fillRect( r, cg.background().light(105) );
				}
			}
			// Draw the transparency pixmap
			else if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() )
				p->drawPixmap( r.topLeft(), *widget->erasePixmap(), r );
			// Draw a solid background
			else
				p->fillRect( r, cg.background().light( 105 ) );

			// Are we a menu item separator?
			if ( mi->isSeparator() )
			{
				p->setPen( cg.mid() );
				p->drawLine( r.x()+5, r.y() + 1, r.right()-5, r.y() + 1 );
				p->drawLine( r.x()+5, r.y() + 2, r.right()-5 , r.y() + 2 );
				break;
			}

			QRect cr = visualRect( QRect( r.x() + 2, r.y() + 2, checkcol - 1, r.height() - 4 ), r );
			// Do we have an icon?
			if ( mi->iconSet() )
			{
				QIconSet::Mode mode;

				// Select the correct icon from the iconset
				if (active)
					mode = enabled?QIconSet::Active:QIconSet::Disabled;
				else
					mode = enabled?QIconSet::Normal:QIconSet::Disabled;

				// Do we have an icon and are checked at the same time?
				// Then draw a "pressed" background behind the icon
				if ( checkable && /*!active &&*/ mi->isChecked() )
					qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(),
										cg, true, 1, &cg.brush(QColorGroup::Midlight) );
				// Draw the icon
				QPixmap pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
				QRect pmr( 0, 0, pixmap.width(), pixmap.height() );
				pmr.moveCenter( cr.center() );
				p->drawPixmap( pmr.topLeft(), pixmap );
			}

			// Are we checked? (This time without an icon)
			else if ( checkable && mi->isChecked() )
			{
				// We only have to draw the background if the menu item is inactive -
				// if it's active the "pressed" background is already drawn

				// Draw the checkmark
				SFlags cflags = Style_Default;
				cflags |= active ? Style_Enabled : Style_On;

				drawPrimitive( PE_CheckMark, p, cr, cg, cflags );
			}

			// Time to draw the menu item label...
			int xm = 2 + checkcol + 2; // X position margin

			int xp = reverse ? // X position
					r.x() + tab + rightBorder + itemHMargin + itemFrame - 1 :
					r.x() + xm;

			int offset = reverse ? -1 : 1; // Shadow offset for etched text

			// Label width (minus the width of the accelerator portion)
			int tw = r.width() - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1;

			// Set the color for enabled and disabled text
			// (used for both active and inactive menu items)
			p->setPen( enabled ? cg.buttonText() : cg.mid() );

			// This color will be used instead of the above if the menu item
			// is active and disabled at the same time. (etched text)
			QColor discol = cg.mid();

			// Does the menu item draw it's own label?
			if ( mi->custom() ) {
				int m = 2;
				p->save();
				// Draw etched text if we're inactive and the menu item is disabled
				if ( etchtext && !enabled && !active ) {
					p->setPen( cg.foreground() );
					mi->custom()->paint( p, cg, active, enabled, xp+offset, r.y()+m+1, tw, r.height()-2*m );
					p->setPen( discol );
				}
				mi->custom()->paint( p, cg, active, enabled, xp, r.y()+m, tw, r.height()-2*m );
				p->restore();
			}
			else {
				// The menu item doesn't draw it's own label
				QString s = mi->text();
				// Does the menu item have a text label?
				if ( !s.isNull() ) {
					int t = s.find( '\t' );
					int m = 2;
					int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
					text_flags |= reverse ? AlignRight : AlignLeft;

					//QColor draw = cg.text();
					QColor draw = (active && enabled) ? cg.highlightedText () : cg.foreground();
					p->setPen(draw);

					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						int tabx = reverse ? r.x() + rightBorder + itemHMargin + itemFrame :
							r.x() + r.width() - tab - rightBorder - itemHMargin - itemFrame;

						// Draw the right part of the label (accelerator text)
						if ( etchtext && !enabled ) {
							// Draw etched text if we're inactive and the menu item is disabled
							p->setPen( cg.light() );
							p->drawText( tabx+offset, r.y()+m+1, tab, r.height()-2*m, text_flags, s.mid( t+1 ) );
							p->setPen( discol );
						}
			
						p->drawText( tabx, r.y()+m, tab, r.height()-2*m, text_flags, s.mid( t+1 ) );
						s = s.left( t );
					}

					// Draw the left part of the label (or the whole label
					// if there's no accelerator)
					if ( etchtext && !enabled ) {
						// Etched text again for inactive disabled menu items...
						p->setPen( cg.light() );
						p->drawText( xp+offset, r.y()+m+1, tw, r.height()-2*m, text_flags, s, t );
						p->setPen( discol );
					}

					p->drawText( xp, r.y()+m, tw, r.height()-2*m, text_flags, s, t );

					p->setPen(cg.text());

				}

				// The menu item doesn't have a text label
				// Check if it has a pixmap instead
				else if ( mi->pixmap() ) {
					QPixmap *pixmap = mi->pixmap();

					// Draw the pixmap
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( OpaqueMode );

					int diffw = ( ( r.width() - pixmap->width() ) / 2 )
									+ ( ( r.width() - pixmap->width() ) % 2 );
					p->drawPixmap( r.x()+diffw, r.y()+1, *pixmap );

					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( TransparentMode );
				}
			}

			// Does the menu item have a submenu?
			if ( mi->popup() ) {
				PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
				int dim = pixelMetric(PM_MenuButtonIndicator) - 1;
				QRect vr = visualRect( QRect( r.x() + r.width() - 5 - 1 - dim,
							r.y() + r.height() / 2 - dim / 2, dim, dim), r );

				// Draw an arrow at the far end of the menu item
				if ( active ) {
					if ( enabled )
						discol = cg.buttonText();

					QColorGroup g2( discol, cg.highlight(), white, white,
									enabled ? white : discol, discol, white );

					drawPrimitive( arrow, p, vr, g2, Style_Enabled );
				} else
					drawPrimitive( arrow, p, vr, cg,
							enabled ? Style_Enabled : Style_Default );
			}

			break;
		}

	// Menu and dockwindow empty space
	//
		case CE_DockWindowEmptyArea:

			p->fillRect(r, cg.background());

			break;

		case CE_MenuBarEmptyArea:

			// menu bar empty content
			p->fillRect(r, cg.background());

			break;

		default:
		  KStyle::drawControl(element, p, widget, r, cg, flags, opt);
	}
	
}

void ComixStyle::drawControlMask(ControlElement element,
								  QPainter *p,
								  const QWidget *w,
								  const QRect &r,
								  const QStyleOption &opt) const
{
	switch (element) {
		case CE_PushButton: {
			// don't need this
			break;
		}

		default: {
			KStyle::drawControlMask (element, p, w, r, opt);
		}
	}
}

void ComixStyle::drawComplexControlMask(ComplexControl c,
										 QPainter *p,
										 const QWidget *w,
										 const QRect &r,
										 const QStyleOption &o) const
{

	KStyle::drawComplexControlMask (c, p, w, r, o);

}

void ComixStyle::drawComplexControl(ComplexControl control,
									 QPainter *p,
									 const QWidget *widget,
									 const QRect &r,
									 const QColorGroup &cg,
									 SFlags flags,
									 SCFlags controls,
									 SCFlags active,
									 const QStyleOption& opt) const
{
	const bool reverseLayout = QApplication::reverseLayout();

	const bool enabled = (flags & Style_Enabled);
	const bool mouseOver = (widget == hoverWidget) || (flags & Style_HasFocus);
	
	//kdDebug() << "complexControl\n";
	
	switch(control) {
	// COMBOBOX
	// --------
		case CC_ComboBox: {
			//static const unsigned int capHeight = 15;
			const QComboBox *cb = dynamic_cast<const QComboBox *>(widget);
			// at the moment cb is only needed to check if the combo box is editable or not.
			// if cb doesn't exist, just assume true and the app (gideon! ;) ) at least doesn't crash.
			// same with kdevelop, assume it's editable.
			bool editable = true;
			bool hasFocus = false;
			QColor button, contour;
			uint surfaceFlags;
			uint contourFlags = Draw_Cap;

			QColor input = enabled?cg.base():cg.background();

			if (cb) {
				editable = cb->editable();
				hasFocus = cb->hasFocus();

				QLineEdit * le = cb->lineEdit();
				if( le && editable && enabled ) {
					// konqueror https background color...
					// this would be too hacky, better get konqueror to update the combobox...
					//if( ! strncmp(le->text(), "https", 5) ) input = QColor(245, 246, 190);
					//else input = le->backgroundColor();
					input = le->backgroundColor();
				}
				else {
					input = cg.background();
				}
			}
			
			if( !enabled ) {
				button = alphaBlendColors(cg.background(), cg.button(), 127);
				contour = alphaBlendColors(cg.background(), getColor(cg, ButtonContour), 127);
			}
			else if( hasFocus || mouseOver ) {
				button = getColor(cg, HighlightButton);
				contour = getColor(cg, HighlightContour);
				contourFlags |= Draw_Hover;
			}
			else {
				button = cg.button();
				contour = getColor(cg, PanelContour);
			}
			
			if( !khtmlWidgets.contains(cb) && !kickerMode ) {
				// kicker mode for the "run command" combobox in kicker :-P
				p->fillRect(r, cg.background() );
			}
			
			const QRect RbuttonSurface(reverseLayout?r.left():r.right()-capHeight-2, r.top(),
										capHeight+3, r.height());

			const QRect RcontentSurface(reverseLayout?r.left()+capHeight:r.left(), r.top(),
										 r.width()-capHeight+1, r.height());

			surfaceFlags = contourFlags;
			if( reverseLayout ) surfaceFlags |= Cut_Right;
			else surfaceFlags |= Cut_Left;
			
			renderSurface(p, RbuttonSurface, button, surfaceFlags);

			surfaceFlags = contourFlags;
			if( reverseLayout ) surfaceFlags |= Cut_Left;
			else surfaceFlags |= Cut_Right;
			if( editable ) surfaceFlags |= Flat_Color;
			
			renderSurface(p, RcontentSurface, input, surfaceFlags);
			
			p->setPen(cg.foreground());
			drawPrimitive(PE_SpinWidgetDown, p, RbuttonSurface, cg, Style_Default|Style_Enabled|Style_Raised);

			renderContour(p, r, contour, enabled, contourFlags);

			//extend the contour: between input and handler...
			p->setPen( contour );
			if(reverseLayout) {
				p->drawLine(r.left()+capHeight, r.top()+penWidth, r.left()+capHeight, r.bottom()-penWidth);
				p->drawLine(r.left()+capHeight+1, r.top()+penWidth, r.left()+capHeight+1, r.bottom()-penWidth);
			} else {
				p->drawLine(r.right()-capHeight, r.top()+penWidth, r.right()-capHeight, r.bottom()-penWidth);
				p->drawLine(r.right()-capHeight-1, r.top()+penWidth, r.right()-capHeight-1, r.bottom()-penWidth);
			}

			// QComboBox draws the text using cg.text(), we can override this
			// from here
			if( enabled ) p->setPen( cg.buttonText() );
			else p->setPen( alphaBlendColors(cg.background(), cg.buttonText(), 127) );
			p->setBackgroundColor( cg.button() );

			break;
		}

	// TOOLBUTTON
	// ----------
		case CC_ToolButton: {
			const QToolButton *tb = (const QToolButton *) widget;

			QRect button, menuarea;
			button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
			menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

			SFlags bflags = flags,
					mflags = flags;

			if (kornMode) {
				drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);
				break;
			} else {
				// don't want to have the buttoncolor as the background...
				// done before.
				bflags &= ~Style_MouseOver;
			}

			if (active & SC_ToolButton)
				bflags |= Style_Down;

			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;

			if (controls & SC_ToolButton) {
			// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised) || widget==hoverWidget ) {
					drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);
				} else if (tb->parentWidget() &&
							tb->parentWidget()->backgroundPixmap() &&
							!tb->parentWidget()->backgroundPixmap()->isNull()) {
					QPixmap pixmap = *(tb->parentWidget()->backgroundPixmap());
					p->drawTiledPixmap( r, pixmap, tb->pos() );
				}
			}

			// Draw a toolbutton menu indicator if required
			if (controls & SC_ToolButtonMenu) {
				if (mflags & (Style_Down | Style_On | Style_Raised)) {
					drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
				}
				drawPrimitive(PE_ArrowDown, p, menuarea, cg, mflags, opt);
			}

			if (tb->hasFocus() && !tb->focusProxy()) {
				QRect fr = tb->rect();
				fr.addCoords(2, 2, -2, -2);
				drawPrimitive(PE_FocusRect, p, fr, cg);
			}

			break;
		}


	// SPINWIDGETS
	// -----------
		case CC_SpinWidget: {
			//static const unsigned int capHeight = 15;
			
			bool alarmpatch = false;
			if( kAlarmMode && r.width() == 100 ) {
				alarmpatch = true;
			}
			
			const QSpinWidget *sw = dynamic_cast<const QSpinWidget *>(widget);
			SFlags sflags = flags;

			uint contourFlags = Draw_Ball;
			uint surfaceFlags = contourFlags;
			
			int heights;

			PrimitiveElement pe;

			bool hasFocus = mouseOver;
			
			const QColor input = enabled?cg.base():cg.background();
			QColor button, contour;
			
			if( !enabled ) {
				button = alphaBlendColors(cg.background(), cg.button(), 127);
				contour = alphaBlendColors(cg.background(), getColor(cg, ButtonContour), 127);
			}
			else if( hasFocus ) {
				button = cg.button();
				contour = getColor(cg, HighlightContour);
				contourFlags |= Draw_Hover;
			}
			else {
				button = cg.button();
				contour = getColor(cg, PanelContour);
			}
		  
			// contour
			const bool heightDividable = ((r.height()%2) == 0);
			heights = QMAX(8, (r.height()-2)/2);

			// surfaces
			QRect inputRect = QRect(reverseLayout?r.right()-heights:r.left(),
					r.top(), r.width()-ballHeight, r.height() );
			
			QRect upRect, downRect, fillRect;
			if( alarmpatch ) {
				// no reverse Layout yet
				upRect = QRect(r.right()-ballHeight+3, r.top()+penWidth,
						ballHeight-penWidth-penWidth, heights-penWidth);
				downRect = QRect(r.right()-ballHeight+3, r.top()+heights+penWidth, 
						ballHeight-penWidth-penWidth, heightDividable?heights-penWidth:heights-penWidth+1 );
				fillRect = QRect(upRect.left(),upRect.top()+3,upRect.width(),upRect.height()-6);
			}
			else {
				upRect = QRect(reverseLayout?r.left()+penWidth:r.right()-ballHeight+1, r.top()+penWidth,
						ballHeight-penWidth, heights-penWidth);
				downRect = QRect(reverseLayout?r.left()+penWidth:r.right()-ballHeight+1,
						r.top()+heights+penWidth, ballHeight-penWidth, heightDividable?heights-penWidth:heights-penWidth+1 );
				fillRect = QRect(upRect.left(),upRect.top()+3,upRect.width(),upRect.height()-6);
			}
			
			if (active==SC_SpinWidgetUp) surfaceFlags|=Is_Sunken;
			if(!enabled) surfaceFlags|=Is_Disabled;

			if( reverseLayout ) surfaceFlags = contourFlags|Cut_Right;
			else surfaceFlags = contourFlags|Cut_Left;
			
			p->fillRect(r, cg.background() );
			
			QColor sc = cg.button();
			if( widget == hoverWidget )
				sc = getColor(cg,HighlightButton);

			// manually draw the surface
			// upRect
			if (active == SC_SpinWidgetUp) {
				// top
				p->setPen(sc.dark(100 + (_contrast * 3)));
				p->drawLine(upRect.left(),upRect.top(),upRect.right(),upRect.top());
				p->setPen(sc.dark(100 + (_contrast * 2)));
				p->drawLine(upRect.left(),upRect.top()+1,upRect.right(),upRect.top()+1);
				p->setPen(sc.dark(100 + _contrast));
				p->drawLine(upRect.left(),upRect.top()+2,upRect.right(),upRect.top()+2);
				// fillRect
				p->fillRect(fillRect,sc);
				// bottom
				p->setPen(sc.light(100 + _contrast));
				p->drawLine(upRect.left(),upRect.bottom()-2,upRect.right(),upRect.bottom()-2);
				p->setPen(sc.light(100 + (_contrast * 2)));
				p->drawLine(upRect.left(),upRect.bottom()-1,upRect.right(),upRect.bottom()-1);
				p->setPen(sc.light(100 + (_contrast * 3)));
				p->drawLine(upRect.left(),upRect.bottom(),upRect.right(),upRect.bottom());
			}
			else {
				// top
				p->setPen(sc.light(100 + (_contrast * 3)));
				p->drawLine(upRect.left(),upRect.top(),upRect.right(),upRect.top());
				p->setPen(sc.light(100 + (_contrast * 2)));
				p->drawLine(upRect.left(),upRect.top()+1,upRect.right(),upRect.top()+1);
				p->setPen(sc.light(100 + _contrast));
				p->drawLine(upRect.left(),upRect.top()+2,upRect.right(),upRect.top()+2);
				// fillRect
				p->fillRect(fillRect,sc);
				// bottom
				p->setPen(sc.dark(100 + _contrast));
				p->drawLine(upRect.left(),upRect.bottom()-2,upRect.right(),upRect.bottom()-2);
				p->setPen(sc.dark(100 + (_contrast * 2)));
				p->drawLine(upRect.left(),upRect.bottom()-1,upRect.right(),upRect.bottom()-1);
				p->setPen(sc.dark(100 + (_contrast * 3)));
				p->drawLine(upRect.left(),upRect.bottom(),upRect.right(),upRect.bottom());
			}
			// downRect
			if (active == SC_SpinWidgetDown) {
				// top
				p->setPen(sc.dark(100 + (_contrast * 3)));
				p->drawLine(downRect.left(),downRect.top(),downRect.right(),downRect.top());
				p->setPen(sc.dark(100 + (_contrast * 2)));
				p->drawLine(downRect.left(),downRect.top()+1,downRect.right(),downRect.top()+1);
				p->setPen(sc.dark(100 + _contrast));
				p->drawLine(downRect.left(),downRect.top()+2,downRect.right(),downRect.top()+2);
				// fillRect
				fillRect.moveBy(0,heights);
				if( !heightDividable ) fillRect.setHeight(fillRect.height()+1);
				p->fillRect(fillRect,sc);
				// bottom
				p->setPen(sc.light(100 + _contrast));
				p->drawLine(downRect.left(),downRect.bottom()-2,downRect.right(),downRect.bottom()-2);
				p->setPen(sc.light(100 + (_contrast * 2)));
				p->drawLine(downRect.left(),downRect.bottom()-1,downRect.right(),downRect.bottom()-1);
				p->setPen(sc.light(100 + (_contrast * 3)));
				p->drawLine(downRect.left(),downRect.bottom(),downRect.right(),downRect.bottom());
			}
			else {
				// top
				p->setPen(sc.light(100 + (_contrast * 3)));
				p->drawLine(downRect.left(),downRect.top(),downRect.right(),downRect.top());
				p->setPen(sc.light(100 + (_contrast * 2)));
				p->drawLine(downRect.left(),downRect.top()+1,downRect.right(),downRect.top()+1);
				p->setPen(sc.light(100 + _contrast));
				p->drawLine(downRect.left(),downRect.top()+2,downRect.right(),downRect.top()+2);
				// fillRect
				fillRect.moveBy(0,heights);
				if( !heightDividable ) fillRect.setHeight(fillRect.height()+1);
				p->fillRect(fillRect,sc);
				// bottom
				p->setPen(sc.dark(100 + _contrast));
				p->drawLine(downRect.left(),downRect.bottom()-2,downRect.right(),downRect.bottom()-2);
				p->setPen(sc.dark(100 + (_contrast * 2)));
				p->drawLine(downRect.left(),downRect.bottom()-1,downRect.right(),downRect.bottom()-1);
				p->setPen(sc.dark(100 + (_contrast * 3)));
				p->drawLine(downRect.left(),downRect.bottom(),downRect.right(),downRect.bottom());
			}
			
			if( reverseLayout ) surfaceFlags = contourFlags|Cut_Left|Flat_Color;
			else surfaceFlags = contourFlags|Cut_Right|Flat_Color;
			
			renderSurface(p, inputRect, input, surfaceFlags);

			// icons...
			sflags = Style_Default | Style_Enabled;
			if (active == SC_SpinWidgetUp) {
				sflags |= Style_On;
				sflags |= Style_Sunken;
			} else
				sflags |= Style_Raised;
			if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
				pe = PE_SpinWidgetPlus;
			else
				pe = PE_SpinWidgetUp;
			p->setPen(cg.foreground());
			drawPrimitive(pe, p, upRect, cg, sflags);

			sflags = Style_Default | Style_Enabled;
			if (active == SC_SpinWidgetDown) {
				sflags |= Style_On;
				sflags |= Style_Sunken;
			} else
				sflags |= Style_Raised;
			if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
				pe = PE_SpinWidgetMinus;
			else
				pe = PE_SpinWidgetDown;
			p->setPen(cg.foreground());
			drawPrimitive(pe, p, downRect, cg, sflags);

			if( alarmpatch ) {
				// no reverse layout yet
				renderContour(p, QRect(86,0,21,26), contour, enabled, contourFlags );
				p->setPen( contour );
				p->drawLine(98,0,98,26);
				p->drawLine(99,0,99,26);

			}
			else {
				renderContour(p, querySubControlMetrics(control, widget, SC_SpinWidgetFrame),
					contour, enabled, contourFlags );
			}
			
			// extend the contour: between input and handler...
			p->setPen( contour );
			if(reverseLayout) {
				p->drawLine(r.left()+ballHeight, r.top()+penWidth, r.left()+ballHeight, r.bottom()-penWidth);
				p->drawLine(r.left()+ballHeight+1, r.top()+penWidth, r.left()+ballHeight+1, r.bottom()-penWidth);
				p->drawLine(r.left()+penWidth, heights, r.left()+ballHeight, heights);
				p->drawLine(r.left()+penWidth, heights+1, r.left()+ballHeight, heights+1);
			} else {
				p->drawLine(r.right()-ballHeight, r.top()+penWidth, r.right()-ballHeight, r.bottom()-penWidth);
				p->drawLine(r.right()-ballHeight-1, r.top()+penWidth, r.right()-ballHeight-1, r.bottom()-penWidth);
				p->drawLine(r.right()-ballHeight, heights, r.right()-penWidth, heights);
				p->drawLine(r.right()-ballHeight, heights+1, r.right()-penWidth, heights+1);
			}

			break;
		}

		default:
			KStyle::drawComplexControl(control, p, widget,
										r, cg, flags, controls,
										active, opt);
			break;
	}

}


QRect ComixStyle::subRect(SubRect r, const QWidget *widget) const
{

	switch (r) {

		case SR_ComboBoxFocusRect: {
			return querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxEditField );
		}

		case SR_ProgressBarContents: {
			return QRect(widget->rect());
		}

		default: {
			return KStyle::subRect(r, widget);
		}
	}
}

QRect ComixStyle::querySubControlMetrics(ComplexControl control,
										  const QWidget *widget,
										  SubControl subcontrol,
										  const QStyleOption &opt) const
{
	if (!widget) {
		return QRect();
	}

	QRect r(widget->rect());
	switch (control) {

		case CC_ComboBox: {
			switch (subcontrol) {
				case SC_ComboBoxEditField: {
					return QRect(r.left()+capRadius, r.top()+penWidth+1, r.width()-capHeight-capRadius-penWidth, r.height()-penWidth-penWidth-2 );
				}
				default: {
					return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
				}
			}
			break;
		}
		case CC_SpinWidget: {

			const bool heightDividable = ((r.height()%2) == 0);

			QSize bs;
			if(heightDividable) {
				bs.setHeight(QMAX(8, (r.height()-2)/2));
			} else {
				bs.setHeight(QMAX(8, (r.height()-2-1)/2));
			}
			bs.setWidth(ballHeight);
			const int buttonsLeft = r.width()-bs.width();
			
			switch (subcontrol) {
				case SC_SpinWidgetUp: {
					return QRect(buttonsLeft, r.top(), bs.width(), bs.height() );
				}
				case SC_SpinWidgetDown: {
					return QRect(buttonsLeft, r.top()+bs.height(), bs.width(), bs.height() );
				}
				case SC_SpinWidgetFrame: {
					return QRect(r.left(), r.top(), r.width(), r.height() );
				}
				case SC_SpinWidgetEditField: {
					return QRect(r.left()+ballRadius, r.top()+penWidth, r.width()-ballHeight-ballRadius-penWidth, r.height()-penWidth-penWidth );
				}
				case SC_SpinWidgetButtonField: {
					return QRect(buttonsLeft, r.top()+1, bs.width(), r.height()-2);
				}
				default: {
					return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
				}
			}
			break;
		}

		default: {
			return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
		}
	}

}

int ComixStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
	switch(m) {
	// TABS
		case PM_TabBarBaseHeight: {
			return 0;
			// 1 pixel below top of PE_PanelTabWidget
		}
		case PM_TabBarTabVSpace: {
			return capRadius;
		}
		case PM_TabBarTabHSpace: {
				return capHeight + 4; // expand for TabVSpace
		}
		case PM_TabBarTabOverlap: {
			return 0;
			// negative values will mess up the tab layout here
		}
		case PM_TabBarBaseOverlap: {
			if(widget) {
				QTabWidget * tw = ::qt_cast<QTabWidget*>(widget);
				if( tw && tw->tabPosition() == QTabWidget::Bottom )
					return -3;
				else if( ! (widget->mapToGlobal(widget->pos()).x() || widget->mapToGlobal(widget->pos()).y()) )
					// fix for a hidden fullscreen frame like kaffeine does
					return 0;
				else
					return -1;
			}
		}
		case PM_TabBarTabShiftHorizontal: {
			return 0;
		}
		case PM_TabBarTabShiftVertical: {
			if(widget) {
				QTabBar * tb = ::qt_cast<QTabBar*>(widget);
				QTabBar::Shape tbs = tb->shape();
				switch (tbs) {
					case QTabBar::TriangularAbove:
					case QTabBar::RoundedAbove:
						return 0;
					case QTabBar::TriangularBelow:
					case QTabBar::RoundedBelow:
						return 2;
					default:
						return 0;
				}
			}
			else 
				return 0;
		}

#if (QT_VERSION >= 0x030300) // requires Qt 3.3
	// extra space between menubar items
		case PM_MenuBarItemSpacing: {
			return edgeRadius;
		}
#endif

// #if (QT_VERSION >= 0x030300) // requires Qt 3.3
//	 // extra space between toolbar items
//		 case PM_ToolBarItemSpacing: {
//			 return 4;
//		 }
// #endif

	// SCROLL BAR
		case PM_ScrollBarSliderMin: {
			return ballHeight + ballHeight;
		}
		case PM_ScrollBarExtent: {
			return ballHeight + 4;
		}

		case PM_DockWindowSeparatorExtent:
			return edgeRadius;

		case PM_DockWindowHandleExtent:
			return 11;

	// SPLITTERS
	// ---------
		case PM_SplitterWidth: {
			return edgeRadius;
		}

	// PROGRESSBARS
	// ------------
		case PM_ProgressBarChunkWidth:
			return 1;

	// SLIDER
	// ------
		case PM_SliderLength:
			return ballHeight;

	// MENU INDICATOR
	// --------------
		case PM_MenuButtonIndicator:
			return 7;

	// CHECKBOXES / RADIO BUTTONS
	// --------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
		case PM_ExclusiveIndicatorHeight:
		case PM_IndicatorWidth:				// Checkbox size
		case PM_IndicatorHeight:
			return ballHeight;

	// FRAMES
	// ------
		case PM_SpinBoxFrameWidth:
			return 1;

		case PM_MenuBarFrameWidth:
			return 1;

		case PM_DefaultFrameWidth: {
			if(widget && ::qt_cast<QPopupMenu*>(widget))
				// 2 pixel border + 1 pixel inner frame
				return 3;
			else
				// 1 pixel shrink + 2 pixel border + 1 pixel inner frame
				return 4;
		}
	
	// BUTTONS
	// -------
		case PM_ButtonDefaultIndicator: {
			return 0;
		}

		case PM_ButtonMargin: {
			return edgeRadius;
		}

		case PM_ButtonShiftVertical: 
		case PM_ButtonShiftHorizontal: {
			return 1;
		}

		default:
			return KStyle::pixelMetric(m, widget);
	}
}


int ComixStyle::styleHint(StyleHint sh,
							const QWidget * w,
							const QStyleOption & opt,
							QStyleHintReturn * shr) const
{
	switch (sh){

		case QStyle::SH_TabBar_Alignment:
			if (_centerTabs){
				return AlignHCenter;
			}
			break;

		default:
			return KStyle::styleHint(sh, w, opt, shr);
	}
	return KStyle::styleHint(sh, w, opt, shr);
}

QSize ComixStyle::sizeFromContents(ContentsType t,
									const QWidget *widget,
									const QSize &s,
									const QStyleOption &opt) const
{

	switch (t) {
		case CT_PopupMenuItem: {
			if (!widget || opt.isDefault())
				return s;

			const QPopupMenu *popup = dynamic_cast<const QPopupMenu *>(widget);
			QMenuItem *mi = opt.menuItem();
			int maxpmw = opt.maxIconWidth();
			int w = s.width(), h = s.height();
			bool checkable = popup->isCheckable();

			if (mi->custom()) {
				w = mi->custom()->sizeHint().width();
				h = mi->custom()->sizeHint().height();
				if (!mi->custom()->fullSpan() )
					h += 4;
			} else if (mi->widget()) {
			// don't change the size in this case.
			} else if (mi->isSeparator()) {
				w = 20;
				h = 2;
			} else {
				if (mi->pixmap()) {
					h = QMAX(h, mi->pixmap()->height() + 2);
				} else {
					h = QMAX(h, 16 + 2 );
					h = QMAX(h, popup->fontMetrics().height() + 4 );
				}

				if (mi->iconSet()) {
					h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small, QIconSet::Normal).height() + 2);
				}
			}

			if (!mi->text().isNull() && (mi->text().find('\t') >= 0)) {
				w += itemHMargin + itemFrame*2 + 7;
			} else if (mi->popup()) {
				w += 2 * arrowHMargin;
			}

			if (maxpmw) {
				w += maxpmw + 6;
			}
			if (checkable && maxpmw < 20) {
				w += 20 - maxpmw;
			}
			if (checkable || maxpmw > 0) {
				w += 12;
			}

			w += rightBorder;

			return QSize(w, h);
		}

		case CT_PushButton:
		{
			const QPushButton* btn = static_cast<const QPushButton*>(widget);
			int w = s.width() + 2 * pixelMetric(PM_ButtonMargin, widget);
			int h = s.height() + 2 * pixelMetric(PM_ButtonMargin, widget);

			if( h < capHeight + 2 ) h = capHeight + 2;
			if( w < capHeight + 2 ) w = capHeight + 2;
			
			if ( btn->text().isEmpty() ) return QSize(w, h);
			return QSize( w+capHeight, h );
		}

		case CT_ProgressBar:
		{
			// force Cap Height
			int h = s.height();
			if( h < capHeight ) h = capHeight;
			return QSize( s.width(), h );
		}

		case CT_SpinBox:
		{
			// force Cap Height
			int h = s.height();
			if( h < capHeight ) h = capHeight;
			return QSize( s.width()+penWidth+penWidth, h );
		}

		case CT_ToolButton:
		{
			if(widget->parent() && ::qt_cast<QToolBar*>(widget->parent()) )
				return QSize(  s.width()+edgeHeight, s.height()+edgeHeight );
			else
				return KStyle::sizeFromContents (t, widget, s, opt);
		}

		case CT_ComboBox:
		{
			int h = s.height() + 2 * pixelMetric(PM_ButtonMargin, widget);
			return QSize(s.width()+capHeight+capHeight, QMAX(capHeight,h));
		}

		default:
			return KStyle::sizeFromContents (t, widget, s, opt);
	}

	return KStyle::sizeFromContents (t, widget, s, opt);
}

bool ComixStyle::eventFilter(QObject *obj, QEvent *ev)
{
	
	if (KStyle::eventFilter(obj, ev) )
		return true;

	if (!obj->isWidgetType() ) return false;
	
	// custom KColorButton
	if ( !strcmp(obj->className(),"KColorButton") ) {
		if ((ev->type() == QEvent::Paint) || (ev->type() == QEvent::WindowActivate) 
			|| (ev->type() == QEvent::Enter) || (ev->type() == QEvent::Leave) 
			|| (ev->type() == QEvent::MouseButtonPress) 
			|| (ev->type() == QEvent::DragLeave) || (ev->type() == QEvent::DragEnter) 
			|| (ev->type() == QEvent::ChildInserted) 
		) {
			
			// Keramik made my day
			static bool recursion = false;
			if (recursion )
				return false;
			recursion = true;

			KColorButton* kcbutton = static_cast<KColorButton*>(obj);
			bool enabled = kcbutton->isEnabled();

			if (ev->type() == QEvent::MouseButtonPress) {
				_sunkenColorButton = true;
				obj->event( ev );
			} else if (ev->type() == QEvent::DragLeave) {
				_sunkenColorButton = false;
				obj->event( ev );
			}
			
			if ( (ev->type() == QEvent::Enter) || (ev->type() == QEvent::DragEnter) ) {
				_hoverColorButton = true;
				obj->event( ev );
			} else if ( (ev->type() == QEvent::Leave) || (ev->type() == QEvent::DragLeave) ) {
				_hoverColorButton = false;
				obj->event( ev );
			}
			
			if (ev->type() == QEvent::ChildInserted) {
				_hoverColorButton = false;
				_sunkenColorButton = false;
				obj->event( ev );
			}

			bool sunken = (_sunkenColorButton && _hoverColorButton && enabled);

			QPainter newp( kcbutton );
			const QRect newr = kcbutton->rect();
			const QColorGroup newcg = kcbutton->colorGroup();
			const QColor labelColor = enabled?kcbutton->color():alphaBlendColors(newcg.background(), kcbutton->color(), 127);

			newp.eraseRect(newr);

			renderButton(&newp, newr, newcg, sunken, false, _hoverColorButton, enabled);

			renderColorButtonLabel(&newp, newr, labelColor);

			recursion = false;
		    return true;
		}
	}

	//Hover highlight... use qt_cast to check if the widget inheits one of the classes.
	else if ( ::qt_cast<QPushButton*>(obj) || ::qt_cast<QComboBox*>(obj) ||
			::qt_cast<QSpinWidget*>(obj) || ::qt_cast<QCheckBox*>(obj) ||
			::qt_cast<QRadioButton*>(obj) || ::qt_cast<QToolButton*>(obj) || 
			::qt_cast<QSlider*>(obj) )
	{

		if ((ev->type() == QEvent::Enter) && static_cast<QWidget*>(obj)->isEnabled())
		{
			QWidget* button = static_cast<QWidget*>(obj);
			hoverWidget = button;
			button->repaint(false);
		}
		else if ((ev->type() == QEvent::Leave) && (obj == hoverWidget) )
		{
			QWidget* button = static_cast<QWidget*>(obj);
			hoverWidget = 0;
			button->repaint(false);
		}
		return false;
	}
	// focus highlight
	else if ( ::qt_cast<QLineEdit*>(obj) ) {
		QWidget* widget = static_cast<QWidget*>(obj);

		if ( ::qt_cast<QSpinWidget*>(widget->parentWidget()) )
		{
			QWidget* spinbox = widget->parentWidget();
			if ((ev->type() == QEvent::FocusIn) || (ev->type() == QEvent::FocusOut))
			{
				spinbox->repaint(false);
			}
			return false;
		}

		if ((ev->type() == QEvent::FocusIn) || (ev->type() == QEvent::FocusOut))
		{
			widget->repaint(false);
		}
		return false;
	}
	else if ( !qstrcmp(obj->className(), "KonqDraggableLabel") ) {
		// Konqueror "Location:" label
		QWidget* label = static_cast<QWidget*>(obj);
		label->setBackgroundMode(Qt::PaletteBackground);
		label->removeEventFilter(this);
	}

	return false;
}

QColor ComixStyle::getColor(const QColorGroup &cg, const ColorType t, const bool enabled)const
{
	return getColor(cg, t, enabled?IsEnabled:IsDisabled);
}

QColor ComixStyle::getColor(const QColorGroup &cg, const ColorType t, const WidgetState s)const
{
	const bool enabled = (s != IsDisabled) && ((s == IsEnabled) || (s == IsPressed) || (s == IsHighlighted));

	switch(t) {

		case ButtonContour:
			if( enabled ) {
				if( _useCustomBrushColor ) 
					return _customBrushColor;
				else 
					return cg.button().dark(130+_contrast*8);
			}
			else {
				if( _useCustomBrushColor ) 
					return alphaBlendColors(cg.background(), _customBrushColor, 127);
				else 
					return alphaBlendColors(cg.background(), cg.button().dark(130+_contrast*8), 127);
			}

		case ButtonToolContour:
			if( _useCustomBrushColor ) 
				return _customBrushColor;
			else 
				return cg.background().dark(120+_contrast*2);

		case HighlightContour:
			if( _useCustomBrushColor ) 
				return _customBrushColor.light(127);
			else 
				return alphaBlendColors(cg.highlight(), cg.button().dark(130+_contrast*8), 127);

		case HighlightButton:
			if( _buttonSurfaceHighlight )
				return alphaBlendColors(cg.highlight(), cg.button(), 127);
			else
				return cg.button();
			
		case PanelContour:
			if( _useCustomBrushColor ) 
				return _customBrushColor;
			else 
				return cg.background().dark(150+_contrast*8);

		case TextColor:
			if( enabled ) {
				return cg.text();
			}
			else {
				// disabled contour
				return alphaBlendColors(cg.background(), cg.button().dark(130+_contrast*8), 127);
			}

		default:
			return cg.background();
	}
}

//
// insetRect( QRect * rect, int inset )
//
void ComixStyle::insetRect( QRect * rect, int inset ) const
{

	rect->setWidth(rect->width()-inset);
	rect->setHeight(rect->height()-inset);
	rect->setLeft(rect->left()+inset);
	rect->setTop(rect->top()+inset);

}

