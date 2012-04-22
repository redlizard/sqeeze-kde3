/*
	Serenity Style for KDE 3+
	
		Copyright (C) 2006 Remi Villatel <maxilys@tele2.fr>
	
	Based on Lipstik engine:
	
		Copyright (C) 2004 Patrice Tremblay <tremblaypatrice@yahoo.fr>
	
	Also based on numerous open source styles that helped me to not 
	re-invent the wheel for every line of code. Thanks to everyone.  ;-)
	
	This library is free software; you can redistribute it and/or 
	modify it under the terms of the GNU Library General Public 
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful, 
	but WITHOUT ANY WARRANTY; without even the implied warranty of 
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
	Library General Public License for more details.
	
	You should have received a copy of the GNU Library General Public 
	License along with this library. (See the file COPYING.) If not, 
	write to:
	The Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
	Boston, MA 02111-1307, USA.
*/

#ifndef __SERENITY_H
#define __SERENITY_H

#include <kpixmapeffect.h>
#include <kstyle.h>
#include <qbitmap.h>
//#include <qintcache.h>

//#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

class QSettings;
class QTab;
class QTimer;

// We use a bitmapped flag to trigger the various options
// to render a contour or a surface.
// This enum is (more than) global so that SereneShape class can access it.
enum RenderFlags
{
	No_Flags =		0x00000000, 
	Draw_Left =		0x00000001, 
	Draw_Right =		0x00000002, 
	Draw_Top =		0x00000004, 
	Draw_Bottom =		0x00000008, 
	Square_Box =		0x0000000F,	// Draw_(Left, Right, Top, Bottom)
	//
	Is_Sunken =		0x00000010, 
	Is_Hovered =		0x00000020, 
	Is_Disabled =		0x00000040, 
	Keep_Color =		0x00000080, 
	//
	Round_UpperLeft =	0x00000100, 
	Round_UpperRight =	0x00000200, 
	Round_BottomLeft =	0x00000400, 
	Round_BottomRight =	0x00000800, 
	//
	Round_Top =		0x00000300,	// Round_UpperLeft|Round_UpperRight
	Round_Bottom =		0x00000C00,	// Round_BottomLeft|Round_BottomRight
	Round_Left =		0x00000500,	// Round_UpperLeft|Round_BottomLeft
	Round_Right =		0x00000A00,	// Round_UpperRight|Round_BottomRight
	Round_All =		0x00000F00,	// Round_(all corners)
	//
	Button_Alike =		0x00000F0F,	// Square_Box|Round_All
	TopTab_Alike =		0x00000307,	// Round_Top|Draw_(Left, Top, Right)
	BottomTab_Alike =	0x00000C0B,	// Round_Bottom|Draw_(Left, Bottom, Right)
	//
	Draw_AlphaBlend =	0x00001000, 
	Serene_Drawing =	0x00002000	// Forced gradient contour
};

QColor globalHoverColor;
QColor globalHiliteColor;

class SerenityStyle : public KStyle
{
	Q_OBJECT

public:
	SerenityStyle();
	virtual ~SerenityStyle();
	
	void polish(QPalette& pal);
	void polish(QApplication* app);
	void polish(QWidget* widget);
	void unPolish(QWidget* widget);
	
	void drawKStylePrimitive(KStylePrimitive kpe, 
				QPainter* p, 
				const QWidget* widget, 
				const QRect &r, 
				const QColorGroup &cg, 
				SFlags flags = Style_Default, 
				const QStyleOption& = QStyleOption::Default) const;
	
	void drawPrimitive(PrimitiveElement pe, 
				QPainter* p, 
				const QRect &r, 
				const QColorGroup &cg, 
				SFlags flags = Style_Default, 
				const QStyleOption &opt = QStyleOption::Default) const;
	
	void drawControl(ControlElement element, 
				QPainter* p, 
				const QWidget* widget, 
				const QRect &r, 
				const QColorGroup &cg, 
				SFlags flags = Style_Default, 
				const QStyleOption& = QStyleOption::Default) const;
	
	void drawControlMask(ControlElement, QPainter*, const QWidget*, 
				const QRect &, const QStyleOption &) const;
	
	void drawComplexControl(ComplexControl control, 
				QPainter* p, 
				const QWidget* widget, 
				const QRect &r, 
				const QColorGroup &cg, 
				SFlags flags = Style_Default, 
				SCFlags controls = SC_All, 
				SCFlags active = SC_None, 
				const QStyleOption& = QStyleOption::Default) const;
	
	int pixelMetric(PixelMetric m, const QWidget* widget = 0) const;
	
	QRect subRect(SubRect r, const QWidget* widget) const;
	
	QRect querySubControlMetrics(ComplexControl control, 
				const QWidget* widget, 
				SubControl subcontrol, 
				const QStyleOption &opt = QStyleOption::Default) const;
	
	void drawComplexControlMask(QStyle::ComplexControl c, 
				QPainter* p, 
				const QWidget* w, 
				const QRect &r, 
				const QStyleOption &o=QStyleOption::Default) const;

	void drawItem(QPainter* p,
				const QRect &r, 
				int flags, 
				const QColorGroup &cg, 
				bool enabled, 
				const QPixmap* pixmap, 
				const QString &text, 
				int len = -1, 
				const QColor* penColor = 0) const;
	
	QSize sizeFromContents(QStyle::ContentsType t, 
				const QWidget* w, 
				const QSize &s, 
				const QStyleOption &o) const;
	
	int styleHint(StyleHint, const QWidget* = 0, 
				const QStyleOption & = QStyleOption::Default, 
				QStyleHintReturn* = 0) const;

	QPixmap stylePixmap(StylePixmap stylepixmap, 
				const QWidget* widget = 0, 
				const QStyleOption& = QStyleOption::Default) const;

protected:
	enum GlobalStyles
	{
		Tao = 0, Zen, Flat
	};
	
	enum TabPosition
	{
		First = 0, Middle, Last, Single
	};
	
	enum ActiveTabStyles
	{
		GradientTab = 0, InvGradTab, HilitedTab, FlatTab
	};
	
	enum PassiveTabStyles
	{
		InvGradPaTab = 0, GradPaTab, InvSolidPaTab, SolidPaTab
	};

	enum MenuGrooveStyles
	{
		MuGradient = 0, MuInvGradient, MuSolid, MuInvSolid, MuTwoLines, MuEmpty 
	};
	
	enum ProgressPattern
	{
		Arrows = 0, CandyBar, Checker, Waves
	};
	
	enum ColorType
	{
		DefaultGround, DefaultPen, 
		MixedSurface, LightSurface, 
		ButtonSurface, ButtonPen, 
		ComboMix, 
		FlatButtonSurface, 
		HoverColor, 
		ScrollerGroove, ScrollerSurface, ScrollerText, 
		FrameContour, 
		PanelContour, 
		Menubar_GradTop, Menubar_GradBottom, Menubar_Solid, Menubar_2lines, 
		PopupContour, 
		TabGroove_GradTop, TabGroove_GradBottom, TabGroove_Solid, 
		HeaderColor, 
		ExpanderColor, 
		TreeBranchColor, ModernCollapsed, ModernExpanded, ModernDotColor, 
		ProgressGroove, ProgressSurface, 
		SeparatorColor, 
		TipContour, TipSurface, TipBase
	};
	
	enum ExtendedFlags
	{
		Default_Button =	0x00000001, 
		ToolButton_Type =	0x00000002, 
		Scroller_Type =		0x00000004, 
		Slider_Type =		0x00000008, 
		Flat_Type =		0x00000010, 
		Header_Type =		0x00000020, 
		Expander_Type =		0x00000040, 
		TabAction_Type =	0x00000080, 
		MultiTab_Type =		0x00000100, 
		FlatButton_Type =	0x00000200
	};

	enum GradientDirections
	{
		HorizontalGradient = 1, VerticalGradient = 2
	};

	enum BitmapItem 
	{
		ARROWDOWN, ARROWUP, ARROWRIGHT, ARROWLEFT, TOOLPLUS, 
		MINIDOWN, MINIUP, MINIRIGHT, MINILEFT, 
		CHECKMARK, TRISTATE, RADIOMARK, APPLETMENU, 
		BIGPLUS, BIGMINUS, COLLAPSED, EXPANDED, 
		MODLEFTDECO, MODCOLLAPSED, MODTOPDECO, MODEXPANDED, MODDOT, 
		TINY_RADIOMARK, 
		DOCKCLOSE, DOCKIN, DOCKOUT, DOCKSTAY
	};

	enum HoverItems
	{
		HOVER_NONE, 
		HOVER_SUBLINE, HOVER_SUBLINE2, HOVER_ADDLINE, 
		HOVER_PAGES, HOVER_SLIDER, 
		HOVER_CHECKRADIO, 
		HOVER_COMBO, 
		HOVER_SPIN_UP, HOVER_SPIN_DOWN
	};

	void renderGradient(QPainter* p, 
				const QRect &r, 
				const QColor &startColor, 
				const QColor &endColor, 
				const int direction, 
				const bool convex = true) const;

	void renderGradLine(QPainter* p, 
				const QRect &r, 
				const QColor &startColor, 
				const QColor &endColor, 
				const int direction, 
				const bool convex = true) const;

	void renderDiagonalGradient(QPainter* p, 
				const QRect &r, 
				const QColor &startColor, 
				const QColor &middleColor, 
				const QColor &endColor, 
				const bool convex = true) const;
	
	void renderZenGradient(QPainter* p, 
				const QRect &r, 
				const QColor &startColor, 
				const QColor &middleColor, 
				const QColor &endColor, 
				const bool convex = true) const;

	void renderCircleGradient(QPainter* p, 
				const QRect &r, 
				const QColor &startColor, 
				const QColor &middleColor, 
				const QColor &endColor, 
				const bool convex = true) const;

	void renderBicoloreGradient(QPainter* p, 
				const QRect &r, 
				const QColorGroup &g, 
				const QColor &backgroundColor, 
				const QColor &startColor, 
				const QColor &middleColor, 
				const QColor &endColor,
				const int flags = No_Flags, 
				const bool convex = true) const;

	void renderMenuGradient(QPainter* p, 
					const QRect &r, 
					const QColor &startColor, 
					const QColor &middleColor, 
					const QColor &endColor,
					const bool convex = true) const;

	inline void renderPixel(QPainter* p, 
				const QPoint xy, 
				const QColor &color, 
				const int alpha = 128) const;

	void renderSereneContour(QPainter* p, 
				const QRect &r, 
				const QColor &backgroundColor, 
				const QColor &contourColor, 
				const uint flags = Button_Alike) const;

	void renderContour(QPainter* p, 
				const QRect &r, 
				const QColor &backgroundColor, 
				const QColor &contourColor, 
				const uint flags = Button_Alike) const;

	void renderFlatArea(QPainter* p, 
				const QRect &r, 
				const QColor &backgroundColor, 
				const QColor &surfaceColor, 
				const uint flags = Button_Alike) const;

	void renderSurface(QPainter* p, 
				const QRect &r, 
				const QColor &backgroundColor, 
				const QColor &surfaceColor, 
				const QColor &highlightColor, 
				const uint flags = Button_Alike) const;

	void renderZenCircle(QPainter* p, 
				const QRect &r, 
				const QColor &backgroundColor, 
				const QColor &surfaceColor, 
				const QColor &highlightColor, 
				const uint flags = Button_Alike) const;

	void renderMask(QPainter* p, 
				const QRect &r, 
				const QColor &color, 
				const uint flags = Button_Alike) const;

	void renderButton(QPainter* p, 
				const QRect &r, 
				const QColorGroup &g, 
				bool sunken = false, 
				bool mouseOver = false, 
				bool enabled = true, 
				bool khtmlMode = false, 
				const uint extdFlags = 0) const;

	void renderSereneEditFrame(QPainter* p, 
				const QRect &r, 
				const QColorGroup &g, 
				const bool enabled, 
				const bool focused, 
				const bool khtml = false) const;

	void renderGrip(QPainter* p, 
				const QRect &r, 
				const QColorGroup &g, 
				const QColor &color, 
				const bool horizontal, 
				const bool sunken = false, 
				const bool mouseOver = false, 
				const bool enabled = true, 
				const bool thick = true) const;

	void renderSereneTab(QPainter* p, 
				const QRect &r, 
				const QColorGroup &g, 
				const uint flags = 0, 
				const bool isBottom = false, 
				const TabPosition pos = Middle, 
				const bool triangular = false/*, 
				const bool cornerWidget = false*/) const;
	
	void renderHardMap(QPainter* p, 
				const QRect &r, 
				const QColor foreColor, 
				const BitmapItem littleMap) const;

	void renderSoftMap(QPainter* p, 
				const QRect &r, 
				const QColor backColor, 
				const QColor foreColor, 
				const BitmapItem bigMap) const;

	bool eventFilter(QObject*, QEvent*);
	
	QWidget* hoverWidget;
	bool hoverToolTab;
	int hoverThis;

	QRegion maskPattern(const int width, const int height, int shift) const;

	bool mouseWithin(const QWidget* hover, const QRect &here) const;

	int visibleLen(const QFontMetrics fm, const QString str, const int width);

	int limitedTo(int lowest, int highest, int variable);

	QListViewItem* nextVisibleSibling(QListViewItem* item) const;

	inline QColor colorMix(const QColor &bgColor, const QColor &fgColor, const int alpha = 128) const;

	inline QColor getColor(const QColorGroup &cg, const ColorType t, const bool enabled = true) const;
	
	inline QColor borderColor(const QColor baseColor, const bool top, const bool sunken = false) const;

	inline QColor gradientColor(const QColor baseColor, const bool top, const bool highContrast = false) const;

	inline QColor radioBorderColor(const QColor baseColor, const bool top, const bool sunken = false) const;

	inline QColor radioGradientColor(const QColor baseColor, const bool top) const;

 	inline QColor brighter(const QColor baseColor, const int factor) const;

 	inline QColor darker(const QColor baseColor, const int factor) const;

protected slots:
	void khtmlWidgetDestroyed(QObject* w);
	
	//Animation slots.
	void updateProgressPos();
	void progressBarDestroyed(QObject* bar);

private:
	// Disable copy constructor and = operator
	SerenityStyle( const SerenityStyle & );
	SerenityStyle& operator=( const SerenityStyle & );

	bool kickerMode, kornMode, kateMode, kontactMode;

	int _contrast;
	int LO_FACTOR, MED_FACTOR, HI_FACTOR, ADJUSTOR;
	//
	int _activeTabStyle;
	bool _alternateSinking;
	bool _animateProgressBar;
	bool _centerTabs;
	bool _customGlobalColor;
	bool _customOverHighlightColor;
	bool _drawFocusRect;
	bool _drawToolBarHandle;
	bool _drawToolBarSeparator;
	bool _drawTriangularExpander;
	bool _flatProgressBar;
	QColor _globalColor;
	int _globalStyle;
	bool _hiliteRubber;
	bool _menubarHack;
	int _menuGrooveStyle;
	//bool _mouseOverLabel;
	QColor _overHighlightColor;
	int _passiveTabStyle;
	int _progressPattern;
	bool _purerHover;
	int _resizerStyle;
	int _scrollBarStyle;
	int _scrollerScheme;
	bool _splitTabs;
	bool _subArrowFrame;
	int _submenuDelay;
	int _submenuSpeed;
	int _tabWarnType;
	bool _textboxHilite;
	int _tipTint;
	int _treeViewStyle;
	bool _wideSlider;

	QTab* hoverTab;

	// To track khtml widgets.
	QMap<const QWidget*, bool> khtmlWidgets;

	// Animation support.
	QMap<QWidget*, int> progAnimWidgets;

	// For progress bar animation
	QTimer* animationTimer;

	bool _reverseLayout;
	
	// For comboboxes, spinwidgets and alikes
	int _controlWidth;
};

#endif // __SERENITY_H
