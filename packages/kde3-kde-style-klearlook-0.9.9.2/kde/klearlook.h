/* $Id: klearlook.h,v 1.8 2006/04/26 18:55:41 jck Exp $
*/

/*
 
  Klearlook (C) Joerg C. Koenig, 2005 jck@gmx.org
 
----
 
  Based upon QtCurve (C) Craig Drummond, 2003 Craig.Drummond@lycos.co.uk
      Bernhard Rosenkränzer <bero@r?dh?t.com>
      Preston Brown <pbrown@r?dh?t.com>
      Than Ngo <than@r?dh?t.com>
 
  Released under the GNU General Public License (GPL) v2.
 
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files(the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#ifndef __KLEARLOOK_H__
#define __KLEARLOOK_H__


#include <kdeversion.h>
#include <kstyle.h>
#include <qcolor.h>
#include <qpoint.h>
#include "config.h"
#include "common.h"

class KlearlookStyle : public KStyle {
        Q_OBJECT

    public:

        enum EApp
        {
            APP_KICKER,
            APP_KORN,
            APP_OPENOFFICE,
            APP_OTHER
    };

        enum
        {
            GRADIENT_BASE,
            GRADIENT_LIGHT,
            GRADIENT_DARK,
            GRADIENT_TOP,
            GRADIENT_BOTTOM,
            GRADIENT_NUM_COLS
    };

        enum ERound
        {
            ROUNDED_NONE,
            ROUNDED_TOP,
            ROUNDED_BOTTOM,
            ROUNDED_LEFT,
            ROUNDED_RIGHT,
            ROUNDED_ALL
    };

        enum EHover
        {
            HOVER_NONE,
            HOVER_CHECK,
            HOVER_RADIO,
            HOVER_SB_ADD,
            HOVER_SB_SUB,
            HOVER_SB_SLIDER,
            HOVER_HEADER,
            HOVER_KICKER
    };

#ifdef USE_SINGLE_STYLE

        KlearlookStyle();
#else

        KlearlookStyle( bool gpm, bool bb = false, bool bf = false, bool round = false,
            EGroove st = GROOVE_RAISED, h = GROOVE_RAISED,
            bool ge = false, bool va = true, bool bdt = false, bool crlh = true, EDefBtnIndicator dbi = IND_BORDER,
            int tbb = 5, ELvExpander lve = LV_EXP_PM, lvl = LV_LINES_DOTTED, bool lvd = true, bool icon = true,
            int popupmenuHighlightLevel = 130 );
#endif

        virtual ~KlearlookStyle() {}

        void polish( QApplication *app );
        void polish( QPalette &pal );
        void polish( QWidget *widget );
        void unPolish( QWidget *widget );
        void drawLightBevel( QPainter *p, const QRect &r, const QColorGroup &cg, QStyle::SFlags flags, bool useGrad, ERound round,
                             const QColor &fill, const QColor *custom = NULL, bool light = false ) const;
        void drawLightBevelButton( QPainter *p, const QRect &r, const QColorGroup &cg, QStyle::SFlags flags, bool useGrad, ERound round,
                                   const QColor &fill, const QColor *custom = NULL, bool light = false ) const;
        void drawArrow( QPainter *p, const QRect &r, const QColorGroup &cg, QStyle::SFlags flags, QStyle::PrimitiveElement pe,
                        bool small = false, bool checkActive = false ) const;
        void drawPrimitive( PrimitiveElement, QPainter *, const QRect &, const QColorGroup &, SFlags = Style_Default,
                            const QStyleOption & = QStyleOption::Default ) const;
        void drawPrimitiveMenu( PrimitiveElement, QPainter *, const QRect &, const QColorGroup &, SFlags = Style_Default,
                                const QStyleOption & = QStyleOption::Default ) const;
        void drawKStylePrimitive( KStylePrimitive kpe, QPainter* p, const QWidget* widget, const QRect &r,
                                  const QColorGroup &cg, SFlags flags, const QStyleOption &opt ) const;
        void drawControl( ControlElement, QPainter *, const QWidget *, const QRect &, const QColorGroup &,
                          SFlags = Style_Default, const QStyleOption & = QStyleOption::Default ) const;
        void drawControlMask( ControlElement, QPainter *, const QWidget *, const QRect &,
                              const QStyleOption & = QStyleOption::Default ) const;
        void drawComplexControlMask( ComplexControl control, QPainter *p, const QWidget *widget, const QRect &r,
                                     const QStyleOption &data ) const;
        QRect subRect( SubRect, const QWidget * ) const;
        void drawComplexControl( ComplexControl, QPainter *, const QWidget *, const QRect &, const QColorGroup &,
                                 SFlags = Style_Default, SCFlags = SC_All, SCFlags = SC_None,
                                 const QStyleOption & = QStyleOption::Default ) const;
        QRect querySubControlMetrics( ComplexControl, const QWidget *, SubControl,
                                      const QStyleOption & = QStyleOption::Default ) const;
        int pixelMetric( PixelMetric, const QWidget *widget = 0 ) const;
        int kPixelMetric( KStylePixelMetric kpm, const QWidget *widget ) const;
        QSize sizeFromContents( ContentsType, const QWidget *, const QSize &,
                                const QStyleOption & = QStyleOption::Default ) const;
        int styleHint( StyleHint, const QWidget *widget = 0, const QStyleOption & = QStyleOption::Default,
                       QStyleHintReturn *returnData = 0 ) const;

    protected:

        bool eventFilter( QObject *object, QEvent *event );
        void drawPBarOrMenu( QPainter *p, QRect const &r, bool horiz, const QColorGroup &cg, bool menu = false ) const;
        void drawPBarOrMenu2( QPainter *p, QRect const &r, bool horiz, const QColorGroup &cg, bool menu = false ) const;
        void drawGradientWithBorder( QPainter *p, QRect const &r, bool horiz = true ) const;
        void drawBevelGradient( const QColor &base, bool increase, int border, QPainter *p, QRect const &r, bool horiz,
                                double shadeTop, double shadeBot ) const;
        void drawGradient( const QColor &top, const QColor &bot, bool increase, int border, QPainter *p, QRect const &r,
                           bool horiz = true ) const;
        void drawSliderHandle( QPainter *p, const QRect &r, const QColorGroup &cg, QStyle::SFlags flags ) const;
	void drawPopupRect( QPainter *p, const QRect &r, const QColorGroup &cg) const ;

        void drawSliderGroove( QPainter *p, const QRect &r, QStyle::SFlags flags, const QWidget *widget ) const;


    private:

        void shadeColors( const QColor &base, QColor *vals ) const;
        const QColor * buttonColors( const QColorGroup &cg ) const;
        const QColor * backgroundColors( const QColorGroup &cg ) const;
        bool redrawHoverWidget();

    private:

    QColor menuPbar[ GRADIENT_NUM_COLS < NUM_SHADES + 1 ? NUM_SHADES + 1 : GRADIENT_NUM_COLS ],
        gray[ NUM_SHADES + 1 ],
        button[ NUM_SHADES + 1 ];   // Last color = base color, for comparisons!
        mutable QColor buttonColoured[ NUM_SHADES + 1 ];
        mutable QColor backgroundColoured[ NUM_SHADES + 1 ];
        EApp themedApp;
        int popupmenuHighlightLevel;
        bool borderButton,
        menuIcons,
        darkMenubar,
        borderFrame,
        rounded,
        vArrow,
        boldDefText,
        crLabelHighlight,
        lvDark,
        borderSplitter;
        EDefBtnIndicator defBtnIndicator;
        EGroove sliderThumbs,
        handles;
        ETBarBorder toolbarBorders;
        ELvExpander lvExpander;
        ELvLines lvLines;
        EProfile pmProfile;
        EAppearance appearance;
#if KDE_VERSION >= 0x30200

        bool isTransKicker;
#endif

        EHover hover;
        int contrast;
        QPoint oldCursor;
        mutable bool formMode;
        QWidget *hoverWidget;
        int hoverSect;
};

#endif
