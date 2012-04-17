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
			 (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org>
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

#ifndef __COMIX_H
#define __COMIX_H

#include <kstyle.h>
#include <qbitmap.h>
#include <qintcache.h>

#define u_arrow -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2
#define d_arrow -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1
#define l_arrow 0,-3, 0,3,-1,-2,-1,2,-2,-1,-2,1,-3,0
#define r_arrow -2,-3,-2,3,-1,-2, -1,2,0,-1,0,1,1,0

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

struct PixelMarks {
	int x;
	int y;
	int alpha;
};

enum circleType
{
	ActiveCap=0, InactiveCap, HoverCap,
	ActiveEdge, InactiveEdge, HoverEdge, HighlightEdge, ToolbuttonEdge,
	ActiveBall, InactiveBall, HoverBall,
	CustomColor,
	CircleCount
};

enum circleSize
{
	SizeCap=0,
	SizeEdge,
	SizeBall
};

class QSettings;
class ComixCircle;

/* -------------- ComixCache -------------- */
class ComixCache
{

public:
	ComixCache();
	~ComixCache();
	
	QPixmap* getCirclePixmap( const uint &type,
					const uint &size,
					const QColor &color );

private:

	ComixCircle		*capCircle;
	ComixCircle		*edgeCircle;
	ComixCircle		*ballCircle;
	QPixmap			*circlePixmap[CircleCount];
	QColor			circleColor[CircleCount];
	uint			circleHeight[CircleCount];
	
};

/* -------------- ComixStyle -------------- */
class ComixStyle : public KStyle
{
	Q_OBJECT

public:
	ComixStyle();
	virtual ~ComixStyle();

	void polish(QApplication* app );
	void polish(QWidget* widget );
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

	void drawControlMask( ControlElement, QPainter *, 
					const QWidget *, 
					const QRect &, 
					const QStyleOption &) const;

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

	int styleHint(QStyle::StyleHint sh,
					const QWidget *w,
					const QStyleOption &opt,
					QStyleHintReturn *shr) const;

	QRect subRect(SubRect r,
				  const QWidget *widget ) const;

	QRect querySubControlMetrics(ComplexControl control,
					const QWidget *widget,
					SubControl subcontrol,
					const QStyleOption &opt = QStyleOption::Default ) const;

	void drawComplexControlMask(QStyle::ComplexControl c,
					QPainter *p,
					const QWidget *w,
					const QRect &r,
					const QStyleOption &o=QStyleOption::Default) const;

	QSize sizeFromContents(QStyle::ContentsType t,
					const QWidget *w,
					const QSize &s,
					const QStyleOption &o) const;

protected:
	enum TabPosition
	{
		First = 0,
		Middle,
		Last,
		Single // only one tab!
	};

	enum ColorType
	{
		ButtonContour,
		ButtonToolContour,
		HighlightContour,
		HighlightButton,
		PanelContour,
		TextColor
	};

	enum WidgetState
	{
		IsEnabled,
		IsPressed,	 // implies IsEnabled
		IsHighlighted, // implies IsEnabled
		IsDisabled
	};

	// the only way i see to provide all these options
	// to renderContour/renderSurface...
	enum SurfaceFlags {
		Draw_Edge =			0x00000001,
		Draw_Top =			0x00000002,
		Draw_Bottom =		0x00000004,
		Is_Sunken =			0x00000008,
		Is_Disabled =		0x00000010,
		No_Shrink =			0x00000020,
		Flat_Color =		0x00000040,
		Cut_Left =			0x00000080,
		Cut_Right =			0x00000100,
		Toggle_Button =		0x00000200,
		Draw_Ball =			0x00000400,
		Draw_Cap =			0x00000800,
		Force_Cap =			0x00001000,
		Cut_Top =			0x00002000,
		Cut_Bottom =		0x00004000,
		Tool_Button = 		0x00008000,
		Custom_Color = 		0x00010000,
		Draw_Hover = 		0x00020000,
		Draw_Highlight = 	0x00040000
	};

	void renderContour(QPainter *p,
					const QRect &r,
					const QColor &contourColor,
					const bool &enabled,
					const uint flags = Draw_Edge) const;

	void renderSurface(QPainter *p,
					const QRect &r,
					const QColor &buttonColor,
					const uint flags = Draw_Edge) const;

	void renderButton(QPainter *p,
					const QRect &r,
					const QColorGroup &g,
					bool sunken = false,
					bool edgeButton = false,
					bool mouseOver = false,
					bool enabled = true) const;

	void renderHeader(QPainter *p,
					const QRect &r,
					const QColorGroup &g,
					bool sunken = false,
					bool mouseOver = false,
					bool enabled = true) const;

	void renderPanel(QPainter *p,
					const QRect &r,
					const QColorGroup &g,
					const bool nokicker = true,
					const bool sunken = true,
					const bool noshrink = false) const;

	void renderTab(QPainter *p,
					const QRect &r,
					const QColorGroup &g,
					const QColor &innerColor,
					bool mouseOver = false,
					const bool selected = false,
					const bool bottom = false,
					const TabPosition pos = Middle,
					const bool konsole = false,
					const bool focus = false) const;

	virtual void renderMenuBlendPixmap( KPixmap& pix, const QColorGroup& cg, 
					const QPopupMenu* popup ) const;

	void renderColorButtonLabel( QPainter *p, 
					const QRect &r,
					const QColor &labelColor ) const;

	void renderCheckFrame( QPainter *p, 
					const QRect &r,
					const QColor &frameColor ) const;

	bool eventFilter(QObject *, QEvent *);
	
	void insetRect(QRect *rect, int inset) const;

	QWidget			*hoverWidget;
	ComixCache		*circleCache;

protected slots:
	void khtmlWidgetDestroyed(QObject* w);

	inline QColor getColor(const QColorGroup &cg, const ColorType t, const bool enabled = true)const;
	inline QColor getColor(const QColorGroup &cg, const ColorType t, const WidgetState s)const;
	
private:

	// Disable copy constructor and = operator
	ComixStyle( const ComixStyle & );
	ComixStyle& operator=( const ComixStyle & );

	bool kickerMode, kornMode, kAlarmMode;
	bool htmlLineEdit;

	int _contrast;
	bool _drawToolBarSeparator;
	bool _drawToolBarItemSeparator;
	bool _centerTabs;
	bool _hideFocusRect;
	bool _konsoleTabColor;
	bool _buttonSurfaceHighlight;
	bool _useCustomBrushColor;
	bool _useMenuFrame;
	QColor _customBrushColor;
	QString _scrollBarStyle;

	bool _hoverColorButton, _sunkenColorButton;

	// track khtml widgets.
	QMap<const QWidget*,bool> khtmlWidgets;


};

#endif // __COMIX_H
