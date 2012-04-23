//
// C++ Interface: powderhandler
//
// Description:
//
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef POWDERHANDLER_H
#define POWDERHANDLER_H

#include <kdecorationfactory.h>
#include <kdecoration.h>
#include <qobject.h>
#include <kconfig.h>

// Default button layout
const char default_left[]  = "M";
const char default_right[] = "HIAX";

class PowderHandler : public QObject, public KDecorationFactory
{
Q_OBJECT
public:
	PowderHandler();
	~PowderHandler();
	virtual bool reset(unsigned long changed);

	virtual KDecoration* createDecoration(KDecorationBridge*);

	static bool initialized() { return m_initialized; }
	static int borderSize() { return m_borderSize; }	// Doesn't exist.
	static int cornerRadius() { return m_cornerRadius; }
	static bool maxCorner() { return m_maxCorner; }
	static int lightPos() { return m_lightPos; }
	static int gradientContrast() { return m_gradientContrast; }
	static int titleSize() { return m_titleSize; }
	static int buttonSpacing() { return m_buttonSpacing; }	// Doesn't exist.
	static int buttonSize() { return m_titleSize; }		// This is on purpose!
	static bool centerTitle() { return m_centerTitle; }
	static bool styledMenu() { return m_styledMenu; }
	static int hoverStyle() { return m_hoverStyle; }
	//
	static int closeGlow() { return m_closerGlow; }
	static int menuGlow() { return m_menuGlow; }
	static int maxGlow() { return m_maxGlow; }
	static int minGlow() { return m_minGlow; }
	static int stickyGlow() { return m_stickyGlow; }
	static int abbeGlow() { return m_abbeGlow; }
	static int helpGlow() { return m_helpGlow; }
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
			default: return listColor0;
		}
	}

private:
	void readConfig();
	static bool m_initialized;
	static int m_cornerRadius;
	static bool m_maxCorner;
	static int m_lightPos;
	static int m_gradientContrast;
	static int m_titleSize;
	static int m_buttonSize;
	static bool m_centerTitle;
	static bool m_styledMenu;
	static int m_hoverStyle;
	//
	static int m_borderSize;
	static int m_buttonSpacing;
	//
	static int m_closerGlow;
	static int m_menuGlow;
	static int m_maxGlow;
	static int m_minGlow;
	static int m_stickyGlow;
	static int m_abbeGlow;
	static int m_helpGlow;
	//
	static QColor listColor0;
	static QColor listColor1;
	static QColor listColor2;
	static QColor listColor3;
	static QColor listColor4;
	static QColor listColor5;
	static QColor listColor6;
};

#endif
