//
// C++ Implementation: serenityhandler
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "serenityhandler.h"
#include "serenityclient.h"
#include "pixmaps.h"
#include "embeddata.h"

bool SerenityHandler::m_initialized = false;
//
bool SerenityHandler::m_alternateSinking = false;
int SerenityHandler::m_borderSize = 2;	// Actually 2 + borderSize*2
//int SerenityHandler::m_buttonSize = 20;
int SerenityHandler::m_buttonBaseColor = 0;
int SerenityHandler::m_buttonStyle = 0;
bool SerenityHandler::m_buttonTint = false;
int SerenityHandler::m_buttonTintColor = 0;
int SerenityHandler::m_buttonTintRate = 0;
bool SerenityHandler::m_centerTitle = true;
bool SerenityHandler::m_extraSpacing = false;
int SerenityHandler::m_frameColor = 1;
int SerenityHandler::m_globalStyle = 1;
int SerenityHandler::m_gradientContrast = 5;
int SerenityHandler::m_hidePolicy = 0;
bool SerenityHandler::m_noMaxBorder = false;
bool SerenityHandler::m_purerHover = false;
bool SerenityHandler::m_singleHover = false;
int SerenityHandler::m_singleHoverColor = 0;
bool SerenityHandler::m_solidBar = false; /// Remove
bool SerenityHandler::m_styledMenu = true;
int SerenityHandler::m_sunkenColor = 0;
int SerenityHandler::m_symbolBaseColor = 0;
int SerenityHandler::m_symbolTheme = 0;
bool SerenityHandler::m_symbolTint = false;
int SerenityHandler::m_symbolTintColor = 0;
int SerenityHandler::m_symbolTintRate = 0;
bool SerenityHandler::m_titleFrame = false;
int SerenityHandler::m_titleGround = 0;
int SerenityHandler::m_titleSize = 20;		// + borderSize * 2
bool SerenityHandler::m_zenBorder = true;
//
bool SerenityHandler::m_closerWide = false;
bool SerenityHandler::m_menuWide = false;
bool SerenityHandler::m_maxWide = false;
bool SerenityHandler::m_minWide = false;
bool SerenityHandler::m_stickyWide = false;
bool SerenityHandler::m_aboveWide = false;
bool SerenityHandler::m_belowWide = false;
bool SerenityHandler::m_helpWide = false;
//
int SerenityHandler::m_closerGlow = 0;
int SerenityHandler::m_menuGlow = 2;
int SerenityHandler::m_maxGlow = 1;
int SerenityHandler::m_minGlow = 3;
int SerenityHandler::m_stickyGlow = 5;
int SerenityHandler::m_aboveGlow = 6;
int SerenityHandler::m_belowGlow = 7;
int SerenityHandler::m_helpGlow = 4;
//
QColor SerenityHandler::listColor0(255, 0, 0);
QColor SerenityHandler::listColor1(0, 255, 0);
QColor SerenityHandler::listColor2(0, 192, 255);
QColor SerenityHandler::listColor3(255, 160, 0);
QColor SerenityHandler::listColor4(255, 0, 255);
QColor SerenityHandler::listColor5(255, 255, 0);
QColor SerenityHandler::listColor6(128, 128, 128);
QColor SerenityHandler::listColor7(255, 255, 255);
QColor SerenityHandler::listColor8(64, 64, 64);
QColor SerenityHandler::listColor9(128, 128, 128);
QColor SerenityHandler::listColor10(192, 192, 192);
QColor SerenityHandler::listColor11(255, 255, 255);

QPalette pal = QApplication::palette();
QColor SerenityHandler::std_background = pal.color(QPalette::Active, QColorGroup::Background);
QColor SerenityHandler::std_foreground = pal.color(QPalette::Active, QColorGroup::Foreground);
QColor SerenityHandler::std_highlight = pal.color(QPalette::Active, QColorGroup::Highlight);

SerenityHandler::SerenityHandler()
{
	reset(0);
}

SerenityHandler::~SerenityHandler()
{
	Pixmaps::deletePixmaps();
	qCleanupImages_KWinSerenity();
	m_initialized = false;
}

bool SerenityHandler::reset(unsigned long) // unsigned long changed
{
	readConfig();

	Pixmaps::deletePixmaps();
	Pixmaps::createPixmaps();
	qInitImages_KWinSerenity();

	m_initialized = true;
	return true;
}

KDecoration* SerenityHandler::createDecoration(KDecorationBridge *bridge)
{
	return new SerenityClient(bridge, this);
}

#if KDE_IS_VERSION(3, 4, 0)
bool SerenityHandler::supports(Ability ability)
{
	switch (ability)
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
		{
			return true;
		}
		case AbilityButtonShade:
		default:
		{
			return false;
		}
	};
}
#endif

void SerenityHandler::readConfig()
{
	KConfig config("kwinserenityrc");
	m_gradientContrast = config.readNumEntry("/Qt/KDE/contrast", 5);
	if (m_gradientContrast<0 || m_gradientContrast>10)
		m_gradientContrast = 5;

	config.setGroup("General");
	m_alternateSinking = config.readBoolEntry("AlternateSunkenEffect", false);
	m_borderSize = limitedTo(0, 5, config.readNumEntry("BorderSize", 2));
	m_buttonBaseColor = limitedTo(0, 1, config.readNumEntry("ButtonBaseColor", 0));
	// m_buttonSize = m_titleSize;
	m_buttonStyle = limitedTo(0, 3, config.readNumEntry("ButtonStyle", 0));
	m_buttonTint = config.readBoolEntry("ButtonTint", false);
	m_buttonTintColor = limitedTo(0, 14, config.readNumEntry("ButtonTintColor", 0));
	m_buttonTintRate = limitedTo(-8, 8, config.readNumEntry("ButtonTintRate", 0));
	m_centerTitle = config.readBoolEntry("CenterTitle", true);
	m_extraSpacing = config.readBoolEntry("ExtraSpacing", false);
	m_frameColor = limitedTo(0, 2, config.readNumEntry("FrameColor", 1));
	m_globalStyle = limitedTo(0, 2, config.readNumEntry("GlobalStyle", 1));
	m_hidePolicy = limitedTo(0, 4, config.readNumEntry("HidePolicy", 0));
	m_noMaxBorder = config.readBoolEntry("NoMaxBorder", false);
	m_purerHover = config.readBoolEntry("PurerHover", false);
	m_singleHover = config.readBoolEntry("SingleHover", false);
	m_singleHoverColor = limitedTo(0, 12, config.readNumEntry("SingleHoverColor", 0));
	m_solidBar = config.readBoolEntry("SolidBar", false); /// Remove
	m_styledMenu = config.readBoolEntry("StyledMenu", true);
	m_sunkenColor = limitedTo(0, 13, config.readNumEntry("ActivatedButtonColor", 0));
	m_symbolBaseColor = limitedTo(0, 0, config.readNumEntry("SymbolBaseColor", 0));
	m_symbolTheme = limitedTo(0, 4, config.readNumEntry("SymbolTheme", 0));
	m_symbolTint = config.readBoolEntry("SymbolTint", false);
	m_symbolTintColor = limitedTo(0, 13, config.readNumEntry("SymbolTintColor", 0));
	m_symbolTintRate = limitedTo(-8, 8, config.readNumEntry("SymbolTintRate", 0));
	m_titleFrame = config.readBoolEntry("TitleFraming", false);
	m_titleGround = limitedTo(0, 3, config.readNumEntry("TitlebarBackground", 0));
	m_titleSize = limitedTo(18, 36, config.readNumEntry("TitleSize", 20));
	if (m_titleSize & 1)	// If odd size
		m_titleSize++;
	m_zenBorder = config.readBoolEntry("ZenBorder", true);
	//
	m_closerWide = config.readBoolEntry("WideCloser", false);
	m_menuWide = config.readBoolEntry("WideMenu", false);
	m_maxWide = config.readBoolEntry("WideMaximizer", false);
	m_minWide = config.readBoolEntry("WideMinimizer", false);
	m_stickyWide = config.readBoolEntry("WideSticker", false);
	m_aboveWide = config.readBoolEntry("WideAbove", false);
	m_belowWide = config.readBoolEntry("WideBelow", false);
	m_helpWide = config.readBoolEntry("WideHelp", false);
	//
	m_closerGlow = limitedTo(0, 11, config.readNumEntry("CloserGlow", 0));
	m_menuGlow = limitedTo(0, 11, config.readNumEntry("MenuGlow", 2));
	m_maxGlow = limitedTo(0, 11, config.readNumEntry("MaximizerGlow", 1));
	m_minGlow = limitedTo(0, 11, config.readNumEntry("MinimizerGlow", 3));
	m_stickyGlow = limitedTo(0, 11, config.readNumEntry("StickerGlow", 5));
	m_aboveGlow = limitedTo(0, 11, config.readNumEntry("AboveGlow", 6));
	m_belowGlow = limitedTo(0, 11, config.readNumEntry("BelowGlow", 7));
	m_helpGlow = limitedTo(0, 11, config.readNumEntry("HelpGlow", 4));
	//
	QColor defaultColor0(255, 0, 0);
	QColor defaultColor1(0, 255, 0);
	QColor defaultColor2(0, 192, 255);
	QColor defaultColor3(255, 160, 0);
	QColor defaultColor4(255, 0, 255);
	QColor defaultColor5(255, 255, 0);
	QColor defaultColor6(128, 128, 128);
	QColor defaultColor7(255, 255, 255);
	QColor defaultColor8(64, 64, 64);
	QColor defaultColor9(128, 128, 128);
	QColor defaultColor10(192, 192, 192);
	QColor defaultColor11(255, 255, 255);
	//
	listColor0 = config.readColorEntry("Color0", &defaultColor0);
	listColor1 = config.readColorEntry("Color1", &defaultColor1);
	listColor2 = config.readColorEntry("Color2", &defaultColor2);
	listColor3 = config.readColorEntry("Color3", &defaultColor3);
	listColor4 = config.readColorEntry("Color4", &defaultColor4);
	listColor5 = config.readColorEntry("Color5", &defaultColor5);
	listColor6 = config.readColorEntry("Color6", &defaultColor6);
	listColor7 = config.readColorEntry("Color7", &defaultColor7);
	listColor8 = config.readColorEntry("Color8", &defaultColor8);
	listColor9 = config.readColorEntry("Color9", &defaultColor9);
	listColor10 = config.readColorEntry("Color10", &defaultColor10);
	listColor11 = config.readColorEntry("Color11", &defaultColor11);
}

int SerenityHandler::limitedTo(int lowest, int highest, int variable)
{
	if ( (variable < lowest) || (variable > highest) )
		return (lowest < 0) ? 0 : lowest;
	else
		return variable;
}

QColor SerenityHandler::buttonNormalColor(int button, bool active)
{
	QColor result = m_buttonBaseColor
			? KDecoration::options()->color(KDecoration::ColorTitleBar, active) 
			: std_background;
	if (m_buttonTint)
	{
		QColor tint;
		if (m_buttonTintColor == 0)
			tint = KDecoration::options()->color(KDecoration::ColorTitleBlend, active);
		else if (m_buttonTintColor == 1)
		{
			if ((button == -1) && (!m_singleHover))
				tint = result;
			else
				tint = hoverColor(button);
		}
		else if (m_buttonTintColor == 2)
			tint = KDecoration::options()->color(KDecoration::ColorFont, active);
		else if (m_buttonTintColor >= 3)
			tint = listColor(m_buttonTintColor - 3);
		//
		result = colorMix(result, tint, buttonTintRate());
	}
	return result;
}

QColor SerenityHandler::hoverColor(int button)
{
	if (m_singleHover)
	{
		return (m_singleHoverColor > 0) 
			? listColor(m_singleHoverColor - 1)
			: std_highlight;
	}
	else
	{
		return listColor(glowIndex(button));
	} 
}

QColor SerenityHandler::sunkenColor(int button)
{
	if (m_sunkenColor == 0)
		return std_highlight;
	else if (m_sunkenColor == 1)
		return hoverColor(button);
	else
		return listColor(m_sunkenColor - 2);
}

QColor SerenityHandler::symbolNormalColor(int button, bool active)
{
	QColor result = KDecoration::options()->color(KDecoration::ColorFont, active);
	if (m_symbolTint)
	{
		QColor tint;
		if (m_symbolTintColor == 0)
			tint = KDecoration::options()->color(KDecoration::ColorButtonBg, active);
		else if (m_symbolTintColor == 1)
			tint = hoverColor(button);
		else
			tint = listColor(m_symbolTintColor - 2);
		//
		result = colorMix(result, tint, symbolTintRate());
	}
	return result;
}

QColor SerenityHandler::titleGroundTop(bool active)
{
	if (m_titleGround == 0)
		return std_background;
	else if (m_titleGround == 3)
		return colorMix(std_background, Pixmaps::brighter(std_background, m_gradientContrast*3));
	else // (m_titleGround == 1 || 2)
		return KDecoration::options()->color(KDecoration::ColorTitleBar, active);
}

QColor SerenityHandler::titleGroundBottom(bool active)
{
	if (m_titleGround == 1)
		return KDecoration::options()->color(KDecoration::ColorTitleBar, active);
	else // (m_titleGround == 0 || 2 || 3)
		return std_background;
}

QColor SerenityHandler::titleNormalColor(bool active)
{
	QColor result; 
	if ((m_titleGround == 1) || (m_titleGround == 2))
	{ 
		result = KDecoration::options()->color(KDecoration::ColorTitleBar, active);
	}
	else 
	{
		return buttonNormalColor(-1, active);
	}
	if (m_buttonTint)
	{
		QColor tint = result;
		if (m_buttonTintColor == 0)
			tint = KDecoration::options()->color(KDecoration::ColorTitleBlend, active);
		else if (m_buttonTintColor == 2)
			tint = KDecoration::options()->color(KDecoration::ColorFont, active);
		//
		result = colorMix(result, tint, buttonTintRate());
	}
	return result;
}

QColor SerenityHandler::colorMix(QColor bgColor, QColor fgColor, int alpha)
{
	if (alpha > 255) alpha = 255;
	if (alpha < 0) alpha = 0;

	int bred, bgrn, bblu, fred, fgrn, fblu;
	bgColor.getRgb(&bred, &bgrn, &bblu);
	fgColor.getRgb(&fred, &fgrn, &fblu);
	return QColor( fred + ((bred-fred)*alpha)/255,
			fgrn + ((bgrn-fgrn)*alpha)/255,
			fblu + ((bblu-fblu)*alpha)/255 );
}

#include "serenityhandler.moc"
