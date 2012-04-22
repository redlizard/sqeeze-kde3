//
// C++ Interface: serenityhandler
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SERENITYHANDLER_H
#define SERENITYHANDLER_H

#include <kdecorationfactory.h>
#include <kdecoration.h>
#include <qobject.h>
#include <kconfig.h>

#include <qapplication.h>

// Default button layout
const char default_left[]  = "M";
const char default_right[] = "HIAX";

class SerenityHandler : public QObject, public KDecorationFactory
{
	Q_OBJECT
	public:
		SerenityHandler();
		~SerenityHandler();
		virtual bool reset(unsigned long changed);
	
		virtual KDecoration* createDecoration(KDecorationBridge*);
#if KDE_IS_VERSION(3, 4, 0)
		virtual bool supports( Ability ability );
#endif
		///
		static void set_ground(QColor ground) { std_background = ground; }
		static void set_pen(QColor pen) { std_foreground = pen; }
		static void set_hilite(QColor hilite) { std_highlight = hilite; }
		static QColor std_ground() { return std_background; }
		static QColor std_pen() { return std_foreground; }
		static QColor std_hilite() { return std_highlight; }

		static bool initialized() { return m_initialized; }
		//
		static bool alternateSinking() { return m_alternateSinking; }
		static int borderSize() { return 2 + m_borderSize*2; }	// From 2 to 12px.
		static int titleBorderSize() { return ((m_borderSize==0) ? 3 : 2) + m_borderSize*2; }
		static QColor buttonNormalColor(int button, bool active);
		static int buttonSize() { return m_titleSize; }		// This is on purpose!
		static int buttonSpacing() { return borderSize(); }
		static int buttonStyle() { return m_buttonStyle; }
		static bool buttonTint() { return m_buttonTint; }
		static int buttonTintRate()
		{
			return 144 - ((m_buttonTint) ? m_buttonTintRate*10 : 0);
		}
		static bool centerTitle() { return m_centerTitle; }
		static int extraSpacing() { return m_extraSpacing ? 2 : 0; }
		static int frameColor() { return m_frameColor; }
		static int globalStyle() { return m_globalStyle; }
		static int gradientContrast() { return m_gradientContrast; }
		static bool hiddenCloser() { return (m_hidePolicy == 2 || m_hidePolicy == 4); }
		static bool hiddenMenu() { return (m_hidePolicy == 1 || m_hidePolicy == 4); }
		static int hidePolicy() { return m_hidePolicy; }
		static QColor hoverColor(int button);
		static bool maxBorder() { return m_noMaxBorder == false; }
		static bool purerHover() {return m_purerHover; }
		static bool solidBar() { return m_solidBar; } /// Remove
		static bool styledMenu() { return m_styledMenu; }
		static QColor sunkenColor(int button);
		static QColor symbolNormalColor(int button, bool active);
		static int symbolTheme() { return m_symbolTheme; }
		static bool symbolTint() { return m_symbolTint; }
		static int symbolTintRate()
		{
			return 128 - ((m_symbolTint) ? m_symbolTintRate*8 : 0);
		}
		static bool titleFrame() { return m_titleFrame; }
		static QColor titleGroundTop(bool active);
		static QColor titleGroundBottom(bool active);
		static QColor titleNormalColor(bool active);
		static int titleSize() { return m_titleSize; }
 		static int titleStyle() { return m_titleGround; }
		static bool zenBorder() { return m_borderSize ? m_zenBorder : false; } // Disabled if 2px border.
		//
		static bool closeWide() { return m_closerWide; }
		static bool menuWide() { return m_menuWide; }
		static bool maxWide() { return m_maxWide; }
		static bool minWide() { return m_minWide; }
		static bool stickyWide() { return m_stickyWide; }
		static bool aboveWide() { return m_aboveWide; }
		static bool belowWide() { return m_belowWide; }
		static bool helpWide() { return m_helpWide; }
	
		static int closeGlow() { return m_closerGlow; }
		static int menuGlow() { return m_menuGlow; }
		static int maxGlow() { return m_maxGlow; }
		static int minGlow() { return m_minGlow; }
		static int stickyGlow() { return m_stickyGlow; }
		static int aboveGlow() { return m_aboveGlow; }
		static int belowGlow() { return m_belowGlow; }
		static int helpGlow() { return m_helpGlow; }
		//
		static int glowIndex(int button)
		{
			switch (button)
			{
				case 0: return m_helpGlow;
				case 1: return m_maxGlow;
				case 2: return m_minGlow;
				case 3: return m_closerGlow;
				case 4: return m_stickyGlow;
				case 5: return m_aboveGlow;
				case 6: return m_belowGlow;
				case 7:
				default:
					return m_menuGlow;
			}
		}
		//
		static QColor listColor(int idx)
		{
			switch(idx)
			{
				case 0: return listColor0;
				case 1: return listColor1;
				case 2: return listColor2;
				case 3: return listColor3;
				case 4: return listColor4;
				case 5: return listColor5;
				case 6: return listColor6;
				case 7: return listColor7;
				case 8: return listColor8;
				case 9: return listColor9;
				case 10: return listColor10;
				case 11: return listColor11;
				default: return listColor0;
			}
		}

		static QColor colorMix(QColor backgroundColor, 
				QColor foregroundColor, 
				int alpha = 128);
	
	private:
		static QColor std_background;
		static QColor std_foreground;
		static QColor std_highlight;
		//
		static bool m_initialized;
		//
		static bool m_alternateSinking;
		static int m_borderSize;
		static int m_buttonBaseColor;
		//static int m_buttonSize;
		static int m_buttonStyle;
		static bool m_buttonTint;
		static int m_buttonTintColor;
		static int m_buttonTintRate;
		static bool m_centerTitle;
		static bool m_extraSpacing;
		static int m_frameColor;
		static int m_gradientContrast;
		static int m_globalStyle;
		static int m_hidePolicy;
		static bool m_noMaxBorder;
		static bool m_purerHover;
		static bool m_singleHover;
		static int m_singleHoverColor;
		static bool m_solidBar; /// Remove
		static bool m_styledMenu;
		static int m_sunkenColor;
		static int m_symbolBaseColor;
		static int m_symbolTheme;
		static bool m_symbolTint;
		static int m_symbolTintColor;
		static int m_symbolTintRate;
		static bool m_titleFrame;
		static int m_titleGround;
		static int m_titleSize;
		static bool m_zenBorder;
		//
		static bool m_closerWide;
		static bool m_menuWide;
		static bool m_maxWide;
		static bool m_minWide;
		static bool m_stickyWide;
		static bool m_aboveWide;
		static bool m_belowWide;
		static bool m_helpWide;
	
		static int m_closerGlow;
		static int m_menuGlow;
		static int m_maxGlow;
		static int m_minGlow;
		static int m_stickyGlow;
		static int m_aboveGlow;
		static int m_belowGlow;
		static int m_helpGlow;
		//
		static QColor listColor0;
		static QColor listColor1;
		static QColor listColor2;
		static QColor listColor3;
		static QColor listColor4;
		static QColor listColor5;
		static QColor listColor6;
		static QColor listColor7;
		static QColor listColor8;
		static QColor listColor9;
		static QColor listColor10;
		static QColor listColor11;
		//
		void readConfig();
	
	protected:
		int limitedTo(int lowest, int highest, int variable);
};

#endif
