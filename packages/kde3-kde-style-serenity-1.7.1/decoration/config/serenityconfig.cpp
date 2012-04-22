/**************************************************************************
*   Copyright (C) 2006 by Remi Villatel <maxilys@tele2.fr>                *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
**************************************************************************/

#include <kcolorbutton.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>

#include "serenityconfig.h"
#include "configdialog.h"

SerenityConfig::SerenityConfig(KConfig* config, QWidget* parent)
	: QObject(parent), m_config(0), m_dialog(0)
{
	// Create the configuration object.
	m_config = new KConfig("kwinserenityrc");
	KGlobal::locale()->insertCatalogue("kwin_serenity_config");
	
	// Create and show the configuration dialog.
	m_dialog = new ConfigDialog(parent);
	m_dialog->show();
	
	// Load the configuration.
	load(config);
	
	// Setup the connections.
	connect(m_dialog->m_alternateSinking, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_borderSize, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_buttonBaseColor, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_buttonStyle, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_buttonTint, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_buttonTintColor, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_buttonTintRate, SIGNAL(valueChanged(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_centerTitle, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_extraSpacing, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_frameColor, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_globalStyle, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_hidePolicy, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_singleHover, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_singleHoverColor, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_styledMenu, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_sunkenColor, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_symbolBaseColor, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_symbolTheme, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_symbolTint, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_symbolTintColor, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_symbolTintRate, SIGNAL(valueChanged(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_noMaxBorder, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_purerHover, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_titleFrame, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_titleGround, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_titleSize, SIGNAL(valueChanged(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_zenBorder, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	//
	connect(m_dialog->m_closerWide, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_menuWide, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_maxWide, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_minWide, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_stickyWide, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_aboveWide, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_belowWide, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	connect(m_dialog->m_helpWide, SIGNAL(toggled(bool)), this, SLOT(linkedChange()));
	//
	connect(m_dialog->m_closerGlow, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_menuGlow, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_maxGlow, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_minGlow, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_stickyGlow, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_aboveGlow, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_belowGlow, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	connect(m_dialog->m_helpGlow, SIGNAL(activated(int)), this, SLOT(linkedChange()));
	//
	connect(m_dialog->listColor0, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor1, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor2, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor3, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor4, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor5, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor6, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor7, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor8, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor9, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor10, SIGNAL(pressed()), this, SLOT(linkedChange()));
	connect(m_dialog->listColor11, SIGNAL(pressed()), this, SLOT(linkedChange()));
	
	if (m_dialog->m_borderSize->currentItem() != 0) 
	{
		m_dialog->m_zenBorder->setEnabled(true);
	}
	else
	{
		m_dialog->m_zenBorder->setEnabled(false);
	}
	if (m_dialog->m_buttonStyle->currentItem() == 1)
	{
		m_dialog->m_titleFrame->setEnabled(false);
		m_dialog->m_buttonBaseLabel->setEnabled(false);
		m_dialog->m_buttonBaseColor->setEnabled(false);
		m_dialog->m_buttonTint->setEnabled(false);
		m_dialog->m_allButtonTint->setEnabled(false);
	}
	else
	{
		m_dialog->m_titleFrame->setEnabled(true);
		m_dialog->m_buttonBaseLabel->setEnabled(true);
		m_dialog->m_buttonBaseColor->setEnabled(true);
		m_dialog->m_buttonTint->setEnabled(true);
		if (m_dialog->m_buttonTint->isChecked()) 
		{
			m_dialog->m_allButtonTint->setEnabled(true);
		}
		else
		{
			m_dialog->m_allButtonTint->setEnabled(false);
		}
	}
	if (m_dialog->m_symbolTint->isChecked()) 
	{
		m_dialog->m_allSymbolTint->setEnabled(true);
	}
	else
	{
		m_dialog->m_allSymbolTint->setEnabled(false);
	}
	if (m_dialog->m_singleHover->isChecked()) 
	{
		m_dialog->m_singleHoverColor->setEnabled(true);
		m_dialog->m_allHover->setEnabled(false);
	}
	else
	{
		m_dialog->m_singleHoverColor->setEnabled(false);
		m_dialog->m_allHover->setEnabled(true);
	}
}

SerenityConfig::~SerenityConfig()
{
	if (m_dialog) delete m_dialog;
	if (m_config) delete m_config;
}

void SerenityConfig::load(KConfig*)
{
	m_config->setGroup("General");
	
	bool alternateSinking = m_config->readBoolEntry("AlternateSunkenEffect", false);
	m_dialog->m_alternateSinking->setChecked(alternateSinking);
	int borderSize = limitedTo(0, 5, m_config->readNumEntry("BorderSize", 2));
	m_dialog->m_borderSize->setCurrentItem(borderSize);
	int buttonBaseColor = limitedTo(0, 1, m_config->readNumEntry("ButtonBaseColor", 0));
	m_dialog->m_buttonBaseColor->setCurrentItem(buttonBaseColor);
	int buttonStyle = limitedTo(0, 3, m_config->readNumEntry("ButtonStyle", 0));
	m_dialog->m_buttonStyle->setCurrentItem(buttonStyle);
	bool buttonTint = m_config->readBoolEntry("ButtonTint", false);
	m_dialog->m_buttonTint->setChecked(buttonTint);
	int buttonTintColor = limitedTo(0, 14, m_config->readNumEntry("ButtonTintColor", 0));
	m_dialog->m_buttonTintColor->setCurrentItem(buttonTintColor);
	int buttonTintRate = limitedTo(-8, 8, m_config->readNumEntry("ButtonTintRate", 0));
	m_dialog->m_buttonTintRate->setValue(buttonTintRate);
	bool centerTitle = m_config->readBoolEntry("CenterTitle", true);
	m_dialog->m_centerTitle->setChecked(centerTitle);
	bool extraSpacing = m_config->readBoolEntry("ExtraSpacing", false);
	m_dialog->m_extraSpacing->setChecked(extraSpacing);
	int frameColor = limitedTo(0, 2, m_config->readNumEntry("FrameColor", 1));
	m_dialog->m_frameColor->setCurrentItem(frameColor);
	int globalStyle = limitedTo(0, 2, m_config->readNumEntry("GlobalStyle", 1));
	m_dialog->m_globalStyle->setCurrentItem(globalStyle);
	int hidePolicy = m_config->readNumEntry("HidePolicy", 0);
	m_dialog->m_hidePolicy->setCurrentItem(hidePolicy);
	bool noMaxBorder = m_config->readBoolEntry("NoMaxBorder", false);
	m_dialog->m_noMaxBorder->setChecked(noMaxBorder);
	bool purerHover = m_config->readBoolEntry("PurerHover", false);
	m_dialog->m_purerHover->setChecked(purerHover);
	bool singleHover = m_config->readBoolEntry("SingleHover", false);
	m_dialog->m_singleHover->setChecked(singleHover);
	int singleHoverColor = limitedTo(0, 12, m_config->readNumEntry("SingleHoverColor", 0));
	m_dialog->m_singleHoverColor->setCurrentItem(singleHoverColor);
	bool styledMenu = m_config->readBoolEntry("StyledMenu", true);
	m_dialog->m_styledMenu->setChecked(styledMenu);
	int sunkenColor = limitedTo(0, 13, m_config->readNumEntry("ActivatedButtonColor", 0));
	m_dialog->m_sunkenColor->setCurrentItem(sunkenColor);
	int symbolBaseColor = limitedTo(0, 0, m_config->readNumEntry("SymbolBaseColor", 0));
	m_dialog->m_symbolBaseColor->setCurrentItem(symbolBaseColor);
	int symbolTheme = limitedTo(0, 4, m_config->readNumEntry("SymbolTheme", 0));
	m_dialog->m_symbolTheme->setCurrentItem(symbolTheme);
	bool symbolTint = m_config->readBoolEntry("SymbolTint", false);
	m_dialog->m_symbolTint->setChecked(symbolTint);
	int symbolTintColor = limitedTo(0, 13, m_config->readNumEntry("SymbolTintColor", 0));
	m_dialog->m_symbolTintColor->setCurrentItem(symbolTintColor);
	int symbolTintRate = limitedTo(-8, 8, m_config->readNumEntry("SymbolTintRate", 0));
	m_dialog->m_symbolTintRate->setValue(symbolTintRate);
	bool titleFrame = m_config->readBoolEntry("TitleFraming", false);
	m_dialog->m_titleFrame->setChecked(titleFrame);
	int titleGround = limitedTo(0, 13, m_config->readNumEntry("TitlebarBackground", 0));
	m_dialog->m_titleGround->setCurrentItem(titleGround);
	int titleSize = limitedTo(18, 36, m_config->readNumEntry("TitleSize", 20));
	if (titleSize & 1)
		titleSize++;	// Only odd values.
	m_dialog->m_titleSize->setValue(titleSize);
	bool zenBorder = m_config->readBoolEntry("ZenBorder", true);
	m_dialog->m_zenBorder->setChecked(zenBorder);
	//
	bool closerWide = m_config->readBoolEntry("WideCloser", false);
	m_dialog->m_closerWide->setChecked(closerWide);
	bool menuWide = m_config->readBoolEntry("WideMenu", false);
	m_dialog->m_menuWide->setChecked(menuWide);
	bool maxWide = m_config->readBoolEntry("WideMaximizer", false);
	m_dialog->m_maxWide->setChecked(maxWide);
	bool minWide = m_config->readBoolEntry("WideMinimizer", false);
	m_dialog->m_minWide->setChecked(minWide);
	bool stickyWide = m_config->readBoolEntry("WideSticker", false);
	m_dialog->m_stickyWide->setChecked(stickyWide);
	bool aboveWide = m_config->readBoolEntry("WideAbove", false);
	m_dialog->m_aboveWide->setChecked(aboveWide);
	bool belowWide = m_config->readBoolEntry("WideBelow", false);
	m_dialog->m_belowWide->setChecked(belowWide);
	bool helpWide = m_config->readBoolEntry("WideHelp", false);
	m_dialog->m_helpWide->setChecked(helpWide);
	//
	int closerGlow = limitedTo(0, 11, m_config->readNumEntry("CloserGlow", 0));
	m_dialog->m_closerGlow->setCurrentItem(closerGlow);
	int menuGlow = limitedTo(0, 11, m_config->readNumEntry("MenuGlow", 2));
	m_dialog->m_menuGlow->setCurrentItem(menuGlow);
	int maxGlow = limitedTo(0, 11, m_config->readNumEntry("MaximizerGlow", 1));
	m_dialog->m_maxGlow->setCurrentItem(maxGlow);
	int minGlow = limitedTo(0, 11, m_config->readNumEntry("MinimizerGlow", 3));
	m_dialog->m_minGlow->setCurrentItem(minGlow);
	int stickyGlow = limitedTo(0, 11, m_config->readNumEntry("StickerGlow", 5));
	m_dialog->m_stickyGlow->setCurrentItem(stickyGlow);
	int aboveGlow = limitedTo(0, 11, m_config->readNumEntry("AboveGlow", 6));
	m_dialog->m_aboveGlow->setCurrentItem(aboveGlow);
	int belowGlow = limitedTo(0, 11, m_config->readNumEntry("BelowGlow", 7));
	m_dialog->m_belowGlow->setCurrentItem(belowGlow);
	int helpGlow = limitedTo(0, 11, m_config->readNumEntry("HelpGlow", 4));
	m_dialog->m_helpGlow->setCurrentItem(helpGlow);
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
	QColor color0 = m_config->readColorEntry("Color0", &defaultColor0);
	m_dialog->listColor0->setColor(color0);
	QColor color1 = m_config->readColorEntry("Color1", &defaultColor1);
	m_dialog->listColor1->setColor(color1);
	QColor color2 = m_config->readColorEntry("Color2", &defaultColor2);
	m_dialog->listColor2->setColor(color2);
	QColor color3 = m_config->readColorEntry("Color3", &defaultColor3);
	m_dialog->listColor3->setColor(color3);
	QColor color4 = m_config->readColorEntry("Color4", &defaultColor4);
	m_dialog->listColor4->setColor(color4);
	QColor color5 = m_config->readColorEntry("Color5", &defaultColor5);
	m_dialog->listColor5->setColor(color5);
	QColor color6 = m_config->readColorEntry("Color6", &defaultColor6);
	m_dialog->listColor6->setColor(color6);
	QColor color7 = m_config->readColorEntry("Color7", &defaultColor7);
	m_dialog->listColor7->setColor(color7);
	QColor color8 = m_config->readColorEntry("Color8", &defaultColor8);
	m_dialog->listColor8->setColor(color8);
	QColor color9 = m_config->readColorEntry("Color9", &defaultColor9);
	m_dialog->listColor9->setColor(color9);
	QColor color10 = m_config->readColorEntry("Color10", &defaultColor10);
	m_dialog->listColor10->setColor(color10);
	QColor color11 = m_config->readColorEntry("Color11", &defaultColor11);
	m_dialog->listColor11->setColor(color11);
}

void SerenityConfig::save(KConfig*)
{
	m_config->setGroup("General");
	
	m_config->writeEntry("ActivatedButtonColor", m_dialog->m_sunkenColor->currentItem());
	m_config->writeEntry("AlternateSunkenEffect", m_dialog->m_alternateSinking->isChecked());
	m_config->writeEntry("BorderSize", m_dialog->m_borderSize->currentItem());
	m_config->writeEntry("ButtonBaseColor", m_dialog->m_buttonBaseColor->currentItem());
	m_config->writeEntry("ButtonStyle", m_dialog->m_buttonStyle->currentItem());
	m_config->writeEntry("ButtonTint", m_dialog->m_buttonTint->isChecked());
	m_config->writeEntry("ButtonTintColor", m_dialog->m_buttonTintColor->currentItem());
	m_config->writeEntry("ButtonTintRate", m_dialog->m_buttonTintRate->value());
	m_config->writeEntry("CenterTitle", m_dialog->m_centerTitle->isChecked());
	m_config->writeEntry("ExtraSpacing", m_dialog->m_extraSpacing->isChecked());
	m_config->writeEntry("FrameColor", m_dialog->m_frameColor->currentItem());
	m_config->writeEntry("GlobalStyle", m_dialog->m_globalStyle->currentItem());
	m_config->writeEntry("HidePolicy", m_dialog->m_hidePolicy->currentItem());
	m_config->writeEntry("NoMaxBorder", m_dialog->m_noMaxBorder->isChecked());
	m_config->writeEntry("PurerHover", m_dialog->m_purerHover->isChecked());
	m_config->writeEntry("SingleHover", m_dialog->m_singleHover->isChecked());
	m_config->writeEntry("SingleHoverColor", m_dialog->m_singleHoverColor->currentItem());
	m_config->writeEntry("StyledMenu", m_dialog->m_styledMenu->isChecked());
	m_config->writeEntry("SymbolBaseColor", m_dialog->m_symbolBaseColor->currentItem());
	m_config->writeEntry("SymbolTheme", m_dialog->m_symbolTheme->currentItem());
	m_config->writeEntry("SymbolTint", m_dialog->m_symbolTint->isChecked());
	m_config->writeEntry("SymbolTintColor", m_dialog->m_symbolTintColor->currentItem());
	m_config->writeEntry("SymbolTintRate", m_dialog->m_symbolTintRate->value());
	m_config->writeEntry("TitleFraming", m_dialog->m_titleFrame->isChecked());
	m_config->writeEntry("TitlebarBackground", m_dialog->m_titleGround->currentItem());
	m_config->writeEntry("TitleSize", m_dialog->m_titleSize->value());
	m_config->writeEntry("ZenBorder", m_dialog->m_zenBorder->isChecked());
	//
	m_config->writeEntry("WideCloser", m_dialog->m_closerWide->isOn());
	m_config->writeEntry("WideMenu", m_dialog->m_menuWide->isOn());
	m_config->writeEntry("WideMaximizer", m_dialog->m_maxWide->isOn());
	m_config->writeEntry("WideMinimizer", m_dialog->m_minWide->isOn());
	m_config->writeEntry("WideSticker", m_dialog->m_stickyWide->isOn());
	m_config->writeEntry("WideAbove", m_dialog->m_aboveWide->isOn());
	m_config->writeEntry("WideBelow", m_dialog->m_belowWide->isOn());
	m_config->writeEntry("WideHelp", m_dialog->m_helpWide->isOn());
	//
	m_config->writeEntry("CloserGlow", m_dialog->m_closerGlow->currentItem());
	m_config->writeEntry("MenuGlow", m_dialog->m_menuGlow->currentItem());
	m_config->writeEntry("MaximizerGlow", m_dialog->m_maxGlow->currentItem());
	m_config->writeEntry("MinimizerGlow", m_dialog->m_minGlow->currentItem());
	m_config->writeEntry("StickerGlow", m_dialog->m_stickyGlow->currentItem());
	m_config->writeEntry("AboveGlow", m_dialog->m_aboveGlow->currentItem());
	m_config->writeEntry("BelowGlow", m_dialog->m_belowGlow->currentItem());
	m_config->writeEntry("HelpGlow", m_dialog->m_helpGlow->currentItem());
	//
	m_config->writeEntry("Color0", m_dialog->listColor0->color());
	m_config->writeEntry("Color1", m_dialog->listColor1->color());
	m_config->writeEntry("Color2", m_dialog->listColor2->color());
	m_config->writeEntry("Color3", m_dialog->listColor3->color());
	m_config->writeEntry("Color4", m_dialog->listColor4->color());
	m_config->writeEntry("Color5", m_dialog->listColor5->color());
	m_config->writeEntry("Color6", m_dialog->listColor6->color());
	m_config->writeEntry("Color7", m_dialog->listColor7->color());
	m_config->writeEntry("Color8", m_dialog->listColor8->color());
	m_config->writeEntry("Color9", m_dialog->listColor9->color());
	m_config->writeEntry("Color10", m_dialog->listColor10->color());
	m_config->writeEntry("Color11", m_dialog->listColor11->color());
	//
	m_config->sync();
}

void SerenityConfig::defaults()
{
	m_dialog->m_alternateSinking->setChecked(false);
	m_dialog->m_borderSize->setCurrentItem(2);
	m_dialog->m_buttonBaseColor->setCurrentItem(0);
	m_dialog->m_buttonStyle->setCurrentItem(0);
	m_dialog->m_buttonTint->setChecked(false);
	m_dialog->m_buttonTintColor->setCurrentItem(0);
	m_dialog->m_buttonTintRate->setValue(0);
	m_dialog->m_centerTitle->setChecked(true);
	m_dialog->m_extraSpacing->setChecked(false);
	m_dialog->m_frameColor->setCurrentItem(1);
	m_dialog->m_globalStyle->setCurrentItem(1);
	m_dialog->m_hidePolicy->setCurrentItem(0);
	m_dialog->m_noMaxBorder->setChecked(false);
	m_dialog->m_purerHover->setChecked(false);
	m_dialog->m_singleHover->setChecked(false);
	m_dialog->m_singleHoverColor->setCurrentItem(12);
	m_dialog->m_styledMenu->setChecked(true);
	m_dialog->m_sunkenColor->setCurrentItem(0);
	m_dialog->m_symbolBaseColor->setCurrentItem(0);
	m_dialog->m_symbolTheme->setCurrentItem(0);
	m_dialog->m_symbolTint->setChecked(false);
	m_dialog->m_symbolTintColor->setCurrentItem(0);
	m_dialog->m_symbolTintRate->setValue(0);
	m_dialog->m_titleFrame->setChecked(false);
	m_dialog->m_titleGround->setCurrentItem(0);
	m_dialog->m_titleSize->setValue(20);
	m_dialog->m_zenBorder->setChecked(true);
	//
	m_dialog->m_closerWide->setChecked(false);
	m_dialog->m_menuWide->setChecked(false);
	m_dialog->m_maxWide->setChecked(false);
	m_dialog->m_minWide->setChecked(false);
	m_dialog->m_stickyWide->setChecked(false);
	m_dialog->m_aboveWide->setChecked(false);
	m_dialog->m_belowWide->setChecked(false);
	m_dialog->m_helpWide->setChecked(false);
	//
	m_dialog->m_closerGlow->setCurrentItem(0);
	m_dialog->m_menuGlow->setCurrentItem(2);
	m_dialog->m_maxGlow->setCurrentItem(1);
	m_dialog->m_minGlow->setCurrentItem(3);
	m_dialog->m_stickyGlow->setCurrentItem(5);
	m_dialog->m_aboveGlow->setCurrentItem(6);
	m_dialog->m_belowGlow->setCurrentItem(7);
	m_dialog->m_helpGlow->setCurrentItem(4);
	//
	m_dialog->listColor0->setColor(QColor(255, 0, 0));
	m_dialog->listColor1->setColor(QColor(0, 208, 0));
	m_dialog->listColor2->setColor(QColor(0, 192, 255));
	m_dialog->listColor3->setColor(QColor(255, 160, 0));
	m_dialog->listColor4->setColor(QColor(255, 0, 255));
	m_dialog->listColor5->setColor(QColor(255, 255, 0));
	m_dialog->listColor6->setColor(QColor(128, 128, 128));
	m_dialog->listColor7->setColor(QColor(255, 255, 255));
	m_dialog->listColor8->setColor(QColor(64, 64, 64));
	m_dialog->listColor9->setColor(QColor(128, 128, 128));
	m_dialog->listColor10->setColor(QColor(192, 192, 192));
	m_dialog->listColor11->setColor(QColor(255, 255, 255));
}

void SerenityConfig::linkedChange()
{
	if (m_dialog->m_borderSize->currentItem() != 0) 
	{
		m_dialog->m_zenBorder->setEnabled(true);
	}
	else
	{
		m_dialog->m_zenBorder->setEnabled(false);
	}
	if (m_dialog->m_buttonStyle->currentItem() == 1)
	{
		m_dialog->m_titleFrame->setEnabled(false);
		m_dialog->m_buttonBaseLabel->setEnabled(false);
		m_dialog->m_buttonBaseColor->setEnabled(false);
		m_dialog->m_buttonTint->setEnabled(false);
		m_dialog->m_allButtonTint->setEnabled(false);
	}
	else
	{
		m_dialog->m_titleFrame->setEnabled(true);
		m_dialog->m_buttonBaseLabel->setEnabled(true);
		m_dialog->m_buttonBaseColor->setEnabled(true);
		m_dialog->m_buttonTint->setEnabled(true);
		if (m_dialog->m_buttonTint->isChecked()) 
		{
			m_dialog->m_allButtonTint->setEnabled(true);
		}
		else
		{
			m_dialog->m_allButtonTint->setEnabled(false);
		}
	}
	if (m_dialog->m_symbolTint->isChecked()) 
	{
		m_dialog->m_allSymbolTint->setEnabled(true);
	}
	else
	{
		m_dialog->m_allSymbolTint->setEnabled(false);
	}
	if (m_dialog->m_singleHover->isChecked()) 
	{
		m_dialog->m_singleHoverColor->setEnabled(true);
		m_dialog->m_allHover->setEnabled(false);
	}
	else
	{
		m_dialog->m_singleHoverColor->setEnabled(false);
		m_dialog->m_allHover->setEnabled(true);
	}
	//
	emit changed();
}

int SerenityConfig::limitedTo(int lowest, int highest, int variable)
{
	if ( (variable < lowest) || (variable > highest) )
		return (lowest < 0) ? 0 : lowest;
	else
		return variable;
}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

extern "C"
{
	QObject* allocate_config(KConfig* config, QWidget* parent)
	{
		return (new SerenityConfig(config, parent));
	}
}

#include "serenityconfig.moc"
