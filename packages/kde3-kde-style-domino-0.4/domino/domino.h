/* Domino widget style for KDE 3
   Copyright (C) 2006 Michael Lentner <michaell@gmx.net>

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
   Copyright (C) 2001-2002 Karol Szwed		<gallium@kde.org>
		 (C) 2001-2002 Fredrik Höglund	<fredrik@kde.org>
   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley		<mosfet@kde.org>
		 (C) 2000 Dirk Mueller			<mueller@kde.org>
		 (C) 2001 Martijn Klingens		<klingens@kde.org>
   Progressbar code based on KStyle,
   Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __DOMINO_H
#define __DOMINO_H

#include <kstyle.h>
#include <qintcache.h>
#include <ktoolbarbutton.h>
#include "misc.h"
#include "data.h"
#include <qpixmapcache.h>


// #include <X11/extensions/Xrender.h>
#include <X11/Xft/Xft.h>
class ButtonContour;
class RubberWidget;
class Rubber;


	enum ContourType
{
	Contour_Sunken = 0,
	Contour_Raised = 1,
	Contour_Simple = 2
};

	enum ContourState
{
	Contour_Default = 0,
	Contour_Pressed = 1,
	Contour_MouseOver = 2,
	Contour_DefaultButton = 3
};

	enum TextEffect
{
	TextEffect_None = 0,
	TextEffect_OnlyButton = 1,
	TextEffect_Everywhere = 2
};

	struct DSurface {
		int numGradients;
		QColor g1Color1;
		QColor g1Color2;
		QColor g2Color1;
		QColor g2Color2;
		QColor background;
		int g1Top;
		int g1Bottom;
		int g2Top;
		int g2Bottom;
	};


class DominoStyle : public KStyle
{
	Q_OBJECT
	
public:
	
	DominoStyle();
	virtual ~DominoStyle();
	
	enum PaletteTyp
	{
		Palette_Application,
		Palette_Parent,
		Palette_Widget
	};
	
	void polish(QApplication* app );
	void polish(QWidget* widget );
	void polish( QPalette &p );
	void unPolish(QWidget* widget );
	
	void drawKStylePrimitive(KStylePrimitive kpe,
				QPainter* p,
				const QWidget* widget,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags = Style_Default,
				const QStyleOption& = QStyleOption::Default ) const;

	void drawPrimitive(PrimitiveElement pe,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags = Style_Default,
				const QStyleOption &opt = QStyleOption::Default ) const;

	void drawControl(ControlElement element,
				QPainter *p,
				const QWidget *widget,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags = Style_Default,
				const QStyleOption& = QStyleOption::Default ) const;

	void drawControlMask( ControlElement, QPainter *, const QWidget *, const QRect &, const QStyleOption &) const;

	void drawComplexControl(ComplexControl control,
				QPainter *p,
				const QWidget *widget,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags = Style_Default,
				SCFlags controls = SC_All,
				SCFlags active = SC_None,
				const QStyleOption& = QStyleOption::Default ) const;

	int pixelMetric(PixelMetric m,
				const QWidget *widget = 0 ) const;

	QRect subRect(SubRect r,
				const QWidget *widget ) const;

	QRect querySubControlMetrics(ComplexControl control,
				const QWidget *widget,
				SubControl subcontrol,
				const QStyleOption &opt = QStyleOption::Default ) const;

	QSize sizeFromContents(QStyle::ContentsType t,
				const QWidget *w,
				const QSize &s,
				const QStyleOption &o) const;

	int styleHint(StyleHint, const QWidget * = 0,
				const QStyleOption & = QStyleOption::Default,
				QStyleHintReturn * = 0 ) const;
	
	
	QPixmap stylePixmap(StylePixmap stylepixmap, const QWidget* widget, const QStyleOption& opt) const;
	
	void drawItem( QPainter *p, const QRect &r,
		       int flags, const QColorGroup &g, bool enabled,
		       const QPixmap *pixmap, const QString &text,
		       int len = -1, const QColor *penColor = 0) const;
	
	void dominoDrawItem( QPainter *p, const QRect &r,
		       int flags, const QColorGroup &g, bool enabled,
		       const QPixmap *pixmap, const QString &text,
		       int len = -1, const QColor *penColor = 0, bool buttonTextEffect = 0 ) const;
	
	void setDominoButtonPalette(QWidget* widget, PaletteTyp type);
	
	virtual void drawFocusIndicator(QPainter *p,
				const QRect &r,
				int flags,
				const QColorGroup cg,
				bool /*enabled*/,
				const QPixmap*  /*pixmap*/,
				const QString & text,
				int /*len = -1*/,
				const QColor& highlight = 0,
				bool drawLine = true,
				bool isButton = false) const;
	
	QPixmap renderOutlineIcon(const QPixmap* icon) const;
	
	void renderSurface(QPainter *p,
			   const QRect &r,
			   const QColor &backgroundColor,
			   const QColor &buttonColor,
			   const QColor &highlightColor,
			   const uint flags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
					   Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight|Is_Horizontal) const;
	
	inline QWidget::BackgroundOrigin setPixmapOrigin(QWidget::BackgroundOrigin origin);
	
	mutable Rubber* rubber;
	mutable QRect oldRubberRect;
	mutable bool viewPortPressed;
	mutable bool ignoreNextFocusRect;
	uint _argbRubberBandColor;
	mutable bool compositeManagerRunning;
	const QWidget* rubberViewPort;
	
protected:
	
	enum RubberType
	{
		ArgbRubber = 1,
		LineRubber = 2,
		DistRubber = 3
	};
	RubberType rubberBandType;
	
	enum TabPosition
	{
	First ,
	Middle,
	Last,
	Single
	};

	struct TextEffectSettings {
		TextEffect mode;
		QColor color;
		QColor buttonColor;
		int opacity;
		int buttonOpacity;
		QPoint pos;
		QPoint buttonPos;
	};

	struct GroupBoxSettings {
		bool tintBackground;
		int brightness;
		bool customColor;
		QColor color;
	};

	struct FocusIndicatorSettings {
		QColor color;
		int opacity;
		QColor buttonColor;
		int buttonOpacity;
		bool drawUnderline;
		bool indicateFocus;
	};

	
	mutable TextEffectSettings textEffectSettings;
	GroupBoxSettings groupBoxSettings;
	FocusIndicatorSettings focusIndicatorSettings;
	
	ButtonContour* buttonContour;
	
	enum SurfaceFlags {
	Draw_Left =			 0x00000001,
	Draw_Right =		 0x00000002,
	Draw_Top =			 0x00000004,
	Draw_Bottom =		 0x00000008,
	Is_Horizontal =		 0x00000200,
	Is_Highlight =		 0x00000400,
	Is_Default =			 0x00000800,
	Is_Disabled =		 0x00001000,
	Round_UpperLeft =	 0x00002000,
	Round_UpperRight =	 0x00004000,
	Round_BottomLeft =	 0x00008000,
	Round_BottomRight = 0x00010000,
	Rectangular_UpperLeft =	 0x00000010,
	Rectangular_UpperRight =	 0x10000020,
	Rectangular_BottomLeft =	 0x00000040,
	Rectangular_BottomRight = 0x00000080,
	
	Draw_AlphaBlend =	0x00020000,
	Is_ComboBox =		0x00080000,
	Is_Button =			0x00100000,
	Is_CheckItem =		0x00200000,
	Is_RadioButton =		0x00400000,
	Is_TabTop =			0x00000100,
	Is_TabBottom =		0x01000000,
	Is_ActiveTabTop =	0x00040000,
	Is_ActiveTabBottom =	0x00800000,
	Is_ScrollBarBtn =		0x02000000,
	Is_Header =			0x04000000,
	Is_ScrollBarGroove =	0x08000000,
	Is_Slider =			0x20000000,
	Is_Down =			0x40000000
	};
	
	enum DominoStyleFlags {
		Domino_noCache = 0x80000000
	};
	
	enum DominoPrimitiveElement {
		PE_ScrollBarSubLine2 = 0xf000001
	};
	
	void renderContour(QPainter *p,
			const QRect &r,
			const QColor &backgroundColor,
			const QColor &contourColor,
			const uint flags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
				Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight) const;
	
	void renderButtonShadow(QPainter *p, const QRect &r, const uint flags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
			Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight|Is_Horizontal) const;
	
	QPixmap* renderLineEditShadow(QWidget* widget, QRect rect, QColor bg, uint flags, bool reverse);
	
	inline void renderPixel(QPainter *p,
		const QPoint &pos,
		const int alpha,
		const QColor &color,
		const QColor &background = QColor(),
		bool fullAlphaBlend = true) const;

	void renderButton(QPainter *p,
			  const QRect &r,
			  const QColorGroup &g,
			  bool sunken = false,
			  bool mouseOver = false,
			  bool horizontal = true,
			  bool enabled = true,
			  bool khtmlMode = false) const;

	void renderHeader(QPainter *p,
			  const QRect &r,
			  const QColorGroup &g,
			  bool sunken = false,
			  bool mouseOver = false,
			  bool horizontal = true,
			  bool enabled = true) const;

	void renderGradient(QPainter *p,
			const QRect &r,
			const QColor &c1,
			const QColor &c2,
			bool horizontal = true) const;

	void renderTab(QPainter *p,
			const QRect &r,
			const QColorGroup &g,
			bool mouseOver = false,
			const bool selected = false,
			const bool bottom = false,
			const TabPosition pos = Middle,
			const bool triangular = false,
			const bool cornerWidget = false,
			const bool konqTab = false) const;
	
	DSurface btnSurface;
	DSurface tabTopSurface;
	DSurface activeTabTopSurface;
	DSurface tabBottomSurface;
	DSurface activeTabBottomSurface;
	DSurface scrollBarSurface;
	DSurface scrollBarGrooveSurface;
	DSurface headerSurface;
	DSurface checkItemSurface;
	
	bool eventFilter(QObject *, QEvent *);
	
	QString configMode;
	void removeCachedSbPix(const QColor &color) {
		QPixmapCache::remove("sHCap"+QString::number(color.pixel(), 16)+"1"+configMode);
		QPixmapCache::remove("sHCap"+QString::number(color.pixel(), 16)+"0"+configMode);
		QPixmapCache::remove("sVCap"+QString::number(color.pixel(), 16)+"1"+configMode);
		QPixmapCache::remove("sVCap"+QString::number(color.pixel(), 16)+"0"+configMode);
		QPixmapCache::remove(QString::number(color.pixel(), 16)+"sh"+configMode);
		QPixmapCache::remove(QString::number(color.pixel(), 16)+"sv"+configMode);
		QPixmapCache::remove(QString::number(color.pixel(), 16)+"ah"+configMode);
		QPixmapCache::remove(QString::number(color.pixel(), 16)+"av"+configMode);
	};
	
	QColor getGroupBoxColor(const QWidget* w) const;
	QPixmap* createCheckMark(const QColor color) const;
	QPixmap* createRadioIndicator(const QColor color) const;
	QPixmap disableIcon(const QPixmap* icon) const;;
	
	QWidget* hoverWidget;

	QPixmap* checkMark;
	QPixmap* radioIndicator;
	QPixmap* popupFrame;
	QPixmap* border1;
	mutable bool alphaButtonMode;
	bool KApplicationMode;
	QColor _checkMarkColor;
	
	QMap<const QGroupBox*, const QPixmap*> groupBoxPixmaps;
	uint preMultiplyColor(const uint &c) const;
	QToolButton* popupButton;
	QTimer* popupTimer;
	QPoint popupMousePressPos;

protected slots:
	void khtmlWidgetDestroyed(QObject* w);
	void groupBoxDestroyed(QObject* w);
	
	//Animation slots.
	void updateProgressPos();
	void updateScrollPos(); // smoothscroll
	
	void progressBarDestroyed(QObject* bar);
	inline QColor dGetColor(const QColor &color, const int value) const;
	inline QColor lightenColor(const QColor &color, const int value) const;
	inline QColor darkenColor(const QColor &color, const int value) const;
	void updateTabWidgetFrame();
	void spinBoxValueChanged(int);
	void toolPopupPressed();
	void popupToolMenu();
	void toolPopupReleased();
	
private:
	// Disable copy constructor and = operator
	DominoStyle( const DominoStyle & );
	DominoStyle& operator=( const DominoStyle & );

	bool kickerMode;
	bool konquerorMode;
	bool konsoleMode;
	
	mutable bool flatMode;
	bool sideRepaint;
	bool _animateProgressBar;
	bool _drawTriangularExpander;
	bool _customCheckMarkColor;
	bool _smoothScrolling;
	QColor _toolTipColor;
	int _buttonHeightAdjustment;
	bool _customPopupMenuColor;
	QColor _popupMenuColor;
	bool _customSelMenuItemColor;
	QColor _selMenuItemColor;
	bool _drawPopupMenuGradient;
	bool _indentPopupMenuItems;
	bool _centerTabs;
	bool _wheelSwitchesTabs;
	bool _customSliderHandleContourColor;
	bool _drawButtonSunkenShadow;
	bool _toolBtnAsBtn;
	bool _highlightToolBtnIcon;
	
	bool konqTab;
	int minTabLength;
	bool macStyleBar;
	int konqTabBarContrast;
	bool spinWidgetDown;
	
	mutable int sliderlen;
	
	// smoothscrolling
	int scrollDistance;
	bool scrollVertical;
	QScrollView* scrollWidget;
	QTimer* scrollTimer;
	
	bool hoveredToolBoxTab;
	
	// track khtml widgets.
	QMap<const QWidget*,bool> khtmlWidgets;
	
	//Animation support.
	QMap<QWidget*, int> progAnimWidgets;

	// pixmap cache.
	enum CacheEntryType {
		cSurface,
		cGradientTile,
		cAlphaDot
	};
	
	struct CacheEntry
	{
		CacheEntryType type;
		int width;
		int height;
		QRgb c1Rgb;
		QRgb c2Rgb;
		bool horizontal;
	
		QPixmap* pixmap;
	
		CacheEntry(CacheEntryType t, int w, int h, QRgb c1, QRgb c2 = 0,
			bool hor = false, QPixmap* p = 0 ):
			type(t), width(w), height(h), c1Rgb(c1), c2Rgb(c2), horizontal(hor), pixmap(p)
		{}
	
		~CacheEntry()
		{
			delete pixmap;
		}
	
		int key()
		{
			// create an int key from the properties which is used to refer to entries in the QIntCache.
			// the result may not be 100% correct as we don't have so much space in one integer -- use
			// == operator after find to make sure we got the right one. :)
			return horizontal ^ (type<<1) ^ (width<<5) ^ (height<<10) ^ (c1Rgb<<19) ^ (c2Rgb<<22);
		}
	
		bool operator == (const CacheEntry& other)
		{
			bool match = (type == other.type) &&
				(width	 == other.width) &&
				(height == other.height) &&
				(c1Rgb == other.c1Rgb) &&
				(c1Rgb == other.c1Rgb) &&
				(horizontal = other.horizontal);
// 				   if(!match) {
// 					   qDebug("operator ==: CacheEntries don't match!");
// 					   qDebug("width: %d\t\tother width: %d", width, other.width);
// 					   qDebug("height: %d\t\tother height: %d", height, other.height);
// 					   qDebug("fgRgb: %d\t\tother fgRgb: %d", fgRgb, other.fgRgb);
// 					   qDebug("bgRgb: %d\t\tother bgRgb: %d", bgRgb, other.bgRgb);
// 					   qDebug("surfaceFlags: %d\t\tother surfaceFlags: %d", surfaceFlags, other.surfaceFlags);
// 				   }
			return match;
		}
	};
	QIntCache<CacheEntry> *pixmapCache;

	// For KPE_ListViewBranch
	mutable QBitmap *verticalLine;
	mutable QBitmap *horizontalLine;
	
	// For progress bar animation
	QTimer *animationTimer;
	
	void renderTabSeparator(QPainter *p, const QRect &r, const QColor buttonColor, bool tabAbove) const;
	void renderGroupBox(QObject *obj, QEvent *ev);
};



class ButtonContour
{
	friend class DominoStyleConfig;
	friend class DominoStyle;
	
	public:
		ButtonContour();
		virtual ~ButtonContour();
		
		static const int num_states = 4;
		static const int num_types = 3;
		ContourType defaultType;
		ContourType type;
		ContourState state;
		bool alpha_mode;
		bool drawButtonSunkenShadow;
		int created[num_types][num_states];
		int shadowCreated[num_states];
		
		QColor contour[num_states];
		uint c1[num_types][num_states]; // top shadow
		uint c2[num_types][num_states]; // outer ring
		uint c3[num_types][num_states]; // inner ring
		uint c4[num_types][num_states]; // bottom first shadow line
		uint c5[num_types][num_states]; // bottom second shadow line
		uint c6[num_types][num_states]; // first left shadow line
		QPixmap* btnEdges[num_types][num_states];
		QPixmap* btnVLines[num_types][num_states];
		QPixmap* btnHLines[num_types][num_states];
		
		// for sunken type and sunken flat buttons
		QPixmap* buttonShadowRectangular[num_states];
		QPixmap* buttonShadowRound [num_states];
		QPixmap* buttonShadowHlines[num_states];
		QPixmap* buttonShadowVlines[num_states];
		
		
		bool alphaMode() {
			return alpha_mode;
		}
		uint color1() {
			return c1[type][state];
		}
		uint color2() {
			return c2[type][state];
		}
		uint color3() {
			return c3[type][state];
		}
		uint color4() {
			return c4[type][state];
		}
		uint color5() {
			return c5[type][state];
		}
		uint color6() {
			return c6[type][state];
		}
		QColor contourColor(int t) {
			return contour[t];
		}
		QPixmap* buttonEdges() {
			return btnEdges[type][state];
		}
		QPixmap* buttonVLines() {
			return btnVLines[type][state];
		}
		QPixmap* buttonHLines() {
			return btnHLines[type][state];
		}
		void setType(ContourType t = Contour_Sunken) {
			type = t;
			createPixmaps(type, state);
		}
		void setState(ContourState s = Contour_Default) {
			state = s;
			if(s == Contour_Pressed && type != Contour_Sunken)
				type = Contour_Simple;
			createPixmaps(type, state);
		}
		void setContour(ContourType t = Contour_Sunken, ContourState s = Contour_Default) {
			type = t;
			state = s;
			createPixmaps(type, state);
		}
		void setColor(ContourState s, QColor color) {
			contour[s] = color;
			
			uint c2_tmp;
			uint c3_tmp;
			if(defaultType == Contour_Sunken /*|| defaultType == Contour_Simple*/) {
				c2_tmp = qRgba(contour[s].red(), contour[s].green(), contour[s].blue(), 218);
				c3_tmp = qRgba(contour[s].red(), contour[s].green(), contour[s].blue(), 35);
			}
			else {
				c2_tmp = qRgba(contour[s].red(), contour[s].green(), contour[s].blue(), 203);
				
				QColor whiteColor(qRgba(Qt::white.red(), Qt::white.green(), Qt::white.blue(), 34));
				QColor blackColor(qRgba(contour[s].red(), contour[s].green(), contour[s].blue(), 35));
				
				c3_tmp = blendColors(whiteColor, blackColor).rgb();
			}
			
			for(int t = 0; t < num_types; t++) {
				c2[t][s] = c2_tmp; // outer ring
				c3[t][s] = c3_tmp; // inner ring
			}
		}
		void setDefaultType(ContourType t = Contour_Raised, bool create = true) {
			defaultType = t;
			type = t;
			if(create)
				createPixmaps(type, state);
		}
		void setAlphaMode() {
			alpha_mode = true;
		}
		void reset() {
			type = defaultType;
			state = Contour_Default;
			alpha_mode = false;
		}
		
		QPixmap* shadowRound() {
			return buttonShadowRound[state];
		}
		QPixmap* shadowRectangular() {
			return buttonShadowRectangular[state];
		}
		QPixmap* shadowHLines() {
			return buttonShadowHlines[state];
		}
		QPixmap* shadowVLines() {
			return buttonShadowVlines[state];
		}
		
		void createPixmaps(ContourType t = Contour_Sunken, ContourState s = Contour_Default) {
			type = t;
			state = s;
			
			
			// for pressed sunken buttons + pressed KMultiTabbarTabs
			if(drawButtonSunkenShadow && !shadowCreated[state]) {
				
				QColor shadowColor = alphaBlendColors(Qt::black, contour[state], 120);
				buttonShadowRectangular[state] = new QPixmap(tintImage(qembed_findImage("btnPressedShadowRectangular"), shadowColor));
				buttonShadowRound[state] = new QPixmap(tintImage(qembed_findImage("btnPressedShadowRound"), shadowColor));
				buttonShadowHlines[state] = new QPixmap(tintImage(qembed_findImage("btnPressedShadowHLines"), shadowColor));
				buttonShadowVlines[state] = new QPixmap(tintImage(qembed_findImage("btnPressedShadowVLines"), shadowColor));
				shadowCreated[state] = 1;
			}
			
			
			if(created[type][state])
				return;
			
			QImage btnVLines_img;
			QImage btnHLines_img;
			QImage btnShadow_img;
			
			switch(type) {
				case Contour_Simple: // now pressed raised!
					btnShadow_img = qembed_findImage("btnRSPr"); // the inner white ring
					blend(tintImage(qembed_findImage("btn"), contour[state]), btnShadow_img, btnShadow_img);
					btnHLines_img = qembed_findImage("btnHRSPr");
					blend(tintImage(qembed_findImage("btnH"), contour[state]), btnHLines_img, btnHLines_img);
					btnVLines_img = qembed_findImage("btnVRSPr");
					blend(tintImage(qembed_findImage("btnV"), contour[state]), btnVLines_img, btnVLines_img);
					break;
				case Contour_Sunken:
					btnShadow_img = qembed_findImage("btnSSh");
					blend(tintImage(qembed_findImage("btn"), contour[state]), btnShadow_img, btnShadow_img);
					btnHLines_img = qembed_findImage("btnHSSh");
					blend(tintImage(qembed_findImage("btnH"), contour[state]), btnHLines_img, btnHLines_img);
					btnVLines_img = qembed_findImage("btnVSSh");
					blend(tintImage(qembed_findImage("btnV"), contour[state]), btnVLines_img, btnVLines_img);
					break;
				case Contour_Raised:
					btnShadow_img = qembed_findImage("btn17RaisedShadow3"); // the inner white ring + outer shadow
					blend(tintImage(qembed_findImage("btn"), contour[state]), btnShadow_img, btnShadow_img);
					btnHLines_img = qembed_findImage("btnHRSh");
					blend(tintImage(qembed_findImage("btnH"), contour[state]), btnHLines_img, btnHLines_img);
					btnVLines_img = qembed_findImage("btnVRSh");
					blend(tintImage(qembed_findImage("btnV"), contour[state]), btnVLines_img, btnVLines_img);
				default:
					break;
			}
			
			btnEdges[type][state] = new QPixmap(btnShadow_img);
			btnVLines[type][state] = new QPixmap(btnVLines_img);
			btnHLines[type][state] = new QPixmap(btnHLines_img);
			created[type][state] = 1;
		}
};

class RubberWidget : public QWidget
{
	public:
		RubberWidget(Window window);
		~RubberWidget();
};

class Rubber
{
	friend class DominoStyle;
	
	public:
		Rubber(uint col);
		~Rubber();
		
		Visual* visual;
		Colormap colormap;
		XSetWindowAttributes wsa;
		Window window;
		
		RubberWidget* parent;
		RubberWidget* rubber;
		QRegion mask;
		uint color;
		
		void destroy() {
			if(parent) {
				delete rubber;
				delete parent;
				window = 0;
				parent = 0;
				rubber = 0;
			}
		}
		void create(QRect& r, QRegion& mask);
		void updateMask(QRegion& mask);
};


// DominoXXX classes to access protected (or private...) members
class DominoQGroupBox : public QGroupBox
{
	friend class DominoStyle;
	public:
		DominoQGroupBox( QWidget* parent=0, const char* name=0 )
	: QGroupBox( parent, name ) {}
};

class DominoQWidget : public QWidget
{
	friend class DominoStyleConfig;
	friend class DominoStyle;
	public:
		DominoQWidget( QWidget* parent=0, const char* name=0 )
	: QWidget( parent, name ) {}
	
};

class DominoQFrame : public QFrame
{
	friend class DominoStyle;
	public:
		DominoQFrame( QFrame* parent=0, const char* name=0 )
	: QFrame( parent, name ) {}
};

// QWhatsThis
class DominoQWhatsThat : public QWidget
{
	friend class DominoStyle;
	public:
		DominoQWhatsThat(): QWidget() {}
	private:
		QString text;
};

class DominoKToolBarButton : public KToolBarButton
{
	friend class DominoStyle;
	public:
		DominoKToolBarButton(): KToolBarButton() {}
};

class DominoQListBoxItem : public QListBoxItem
{
	friend class DominoStyle;
	public:
		DominoQListBoxItem(): QListBoxItem() {}
};

class DominoQTabWidget : public QTabWidget
{
	friend class DominoStyleConfig;
	friend class DominoStyle;
	public:
		DominoQTabWidget( QWidget* parent=0, const char* name=0 )
	: QTabWidget( parent, name ) {}
};

class DominoQSpinBox : public QSpinBox
{
	friend class DominoStyle;
	public:
		DominoQSpinBox( QFrame* parent=0, const char* name=0 )
	: QSpinBox( parent, name ) {}
};

class DominoQPixmap : public QPixmap
{
	friend class DominoStyle;
	public:
		DominoQPixmap( int w, int h,  int depth = -1) : QPixmap(w, h, depth) {}
};

class DominoQButton : public QButton
{
	friend class DominoStyle;
	public:
		DominoQButton(): QButton() {}
};

#endif // __DOMINO_H
