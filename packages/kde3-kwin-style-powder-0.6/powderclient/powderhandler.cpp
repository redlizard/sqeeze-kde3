//
// C++ Implementation: powderhandler
//
// Description:
//
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "powderhandler.h"
#include "powderclient.h"
#include "pixmaps.h"
#include "embeddata.h"

bool PowderHandler::m_initialized = false;
int PowderHandler::m_borderSize = 9;
int PowderHandler::m_cornerRadius = 6;
bool PowderHandler::m_maxCorner = false;
int PowderHandler::m_lightPos = 0;
int PowderHandler::m_gradientContrast = 5;
int PowderHandler::m_titleSize = 18; // + 5 +7 (top & mid spacers)
int PowderHandler::m_buttonSize = 18;
int PowderHandler::m_buttonSpacing = 6;
bool PowderHandler::m_centerTitle = false;
bool PowderHandler::m_styledMenu = true;
int PowderHandler::m_hoverStyle = 0;
//
int PowderHandler::m_closerGlow = 0;
int PowderHandler::m_menuGlow = 2;
int PowderHandler::m_maxGlow = 1;
int PowderHandler::m_minGlow = 3;
int PowderHandler::m_stickyGlow = 4;
int PowderHandler::m_abbeGlow = 5;
int PowderHandler::m_helpGlow = 4;
//
QColor PowderHandler::listColor0(255,0,0);
QColor PowderHandler::listColor1(0,255,0);
QColor PowderHandler::listColor2(0,192,255);
QColor PowderHandler::listColor3(255,160,0);
QColor PowderHandler::listColor4(255,0,255);
QColor PowderHandler::listColor5(255,255,0);
QColor PowderHandler::listColor6(0,0,255);

PowderHandler::PowderHandler()
{
	reset(0);
}

PowderHandler::~PowderHandler()
{
	Pixmaps::deletePixmaps();
	qCleanupImages_KWinPowder();
	m_initialized = false;
}

bool PowderHandler::reset(unsigned long) // unsigned long changed
{
	readConfig();

	Pixmaps::deletePixmaps();
	Pixmaps::createPixmaps();
	qInitImages_KWinPowder();

	m_initialized = true;
	return true;
}

KDecoration* PowderHandler::createDecoration(KDecorationBridge *bridge)
{
	return new PowderClient(bridge, this);
}

void PowderHandler::readConfig()
{
	KConfig config("kwinpowderrc");
	config.setGroup("General");

	m_borderSize = 9;	// Fixed.
	m_cornerRadius = config.readNumEntry("CornerRadius", 6);
	m_maxCorner = config.readBoolEntry("MaximizedCorner", false);
	m_lightPos = config.readNumEntry("LightPosition", 0);
	m_gradientContrast = config.readNumEntry("GradientContrast", 5);
	m_titleSize = config.readNumEntry("TitleSize", 18); // + 5 +7 (top & mid spacers)
	m_buttonSize = m_titleSize;
	m_buttonSpacing = m_titleSize/3;
	m_centerTitle = config.readBoolEntry("CenterTitle", false);
	m_styledMenu = config.readBoolEntry("StyledMenu", true);
	m_hoverStyle = config.readNumEntry("HoverStyle", 0);
	//
	m_closerGlow = config.readNumEntry("CloserGlow", 0);
	m_menuGlow = config.readNumEntry("MenuGlow", 2);
	m_maxGlow = config.readNumEntry("MaximizerGlow", 1);
	m_minGlow = config.readNumEntry("MinimizerGlow", 3);
	m_stickyGlow = config.readNumEntry("StickerGlow", 4);
	m_abbeGlow = config.readNumEntry("AboveBelowGlow", 5);
	m_helpGlow = config.readNumEntry("HelpGlow", 4);
	//
	QColor defaultColor0(255,0,0);
	QColor defaultColor1(0,255,0);
	QColor defaultColor2(0,192,255);
	QColor defaultColor3(255,160,0);
	QColor defaultColor4(255,0,255);
	QColor defaultColor5(255,255,0);
	QColor defaultColor6(0,0,255);
	//
	listColor0 = config.readColorEntry("Color0", &defaultColor0);
	listColor1 = config.readColorEntry("Color1", &defaultColor1);
	listColor2 = config.readColorEntry("Color2", &defaultColor2);
	listColor3 = config.readColorEntry("Color3", &defaultColor3);
	listColor4 = config.readColorEntry("Color4", &defaultColor4);
	listColor5 = config.readColorEntry("Color5", &defaultColor5);
	listColor6 = config.readColorEntry("Color6", &defaultColor6);
}

#include "powderhandler.moc"
