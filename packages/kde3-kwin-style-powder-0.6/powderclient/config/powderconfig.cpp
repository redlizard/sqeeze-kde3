/***************************************************************************
*   Copyright (C) 2005 by Remi Villatel <maxilys@tele2.fr>                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcolorbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>

#include "powderconfig.h"
#include "configdialog.h"

PowderConfig::PowderConfig(KConfig* config, QWidget* parent)
    : QObject(parent), m_config(0), m_dialog(0)
{
    // Create the configuration object.
    m_config = new KConfig("kwinpowderrc");
    KGlobal::locale()->insertCatalogue("kwin_powder_config");

    // Create and show the configuration dialog.
    m_dialog = new ConfigDialog(parent);
    m_dialog->show();

    // Load the configuration.
    load(config);

    // Setup the connections.
    connect(m_dialog->m_cornerRadius, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_maxCorner, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(m_dialog->m_lightPos, SIGNAL(activated(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_gradientContrast, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_titleSize, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_centerTitle, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(m_dialog->m_styledMenu, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(m_dialog->m_hoverStyle, SIGNAL(activated(int)), this, SIGNAL(changed()));

    connect(m_dialog->m_closerGlow, SIGNAL(activated(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_menuGlow, SIGNAL(activated(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_maxGlow, SIGNAL(activated(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_minGlow, SIGNAL(activated(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_stickyGlow, SIGNAL(activated(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_abbeGlow, SIGNAL(activated(int)), this, SIGNAL(changed()));
    connect(m_dialog->m_helpGlow, SIGNAL(activated(int)), this, SIGNAL(changed()));

    connect(m_dialog->listColor0, SIGNAL(changed(const QColor&)),this,SLOT(colorChanged(const QColor&)));
    connect(m_dialog->listColor1, SIGNAL(changed(const QColor&)),this,SLOT(colorChanged(const QColor&)));
    connect(m_dialog->listColor2, SIGNAL(changed(const QColor&)),this,SLOT(colorChanged(const QColor&)));
    connect(m_dialog->listColor3, SIGNAL(changed(const QColor&)),this,SLOT(colorChanged(const QColor&)));
    connect(m_dialog->listColor4, SIGNAL(changed(const QColor&)),this,SLOT(colorChanged(const QColor&)));
    connect(m_dialog->listColor5, SIGNAL(changed(const QColor&)),this,SLOT(colorChanged(const QColor&)));
    connect(m_dialog->listColor6, SIGNAL(changed(const QColor&)),this,SLOT(colorChanged(const QColor&)));
}

PowderConfig::~PowderConfig()
{
    if (m_dialog) delete m_dialog;
    if (m_config) delete m_config;
}

void PowderConfig::load(KConfig*)
{
    m_config->setGroup("General");

    int cornerRadius = m_config->readNumEntry("CornerRadius", 6);
    m_dialog->m_cornerRadius->setValue(cornerRadius);
    bool maxCorner = m_config->readBoolEntry("MaximizedCorner", false);
    m_dialog->m_maxCorner->setChecked(maxCorner);
    int lightPos = m_config->readNumEntry("LightPosition", 0);
    m_dialog->m_lightPos->setCurrentItem(lightPos);
    int gradientContrast = m_config->readNumEntry("GradientContrast", 5);
    m_dialog->m_gradientContrast->setValue(gradientContrast);
    int titleSize = m_config->readNumEntry("TitleSize", 18);
    m_dialog->m_titleSize->setValue(titleSize);
    bool centerTitle = m_config->readBoolEntry("CenterTitle", false);
    m_dialog->m_centerTitle->setChecked(centerTitle);
    bool styledMenu = m_config->readBoolEntry("StyledMenu", true);
    m_dialog->m_styledMenu->setChecked(styledMenu);
    int hoverStyle = m_config->readNumEntry("HoverStyle", 0);
    m_dialog->m_hoverStyle->setCurrentItem(hoverStyle);
    //
    int closerGlow = m_config->readNumEntry("CloserGlow", 0);
    m_dialog->m_closerGlow->setCurrentItem(closerGlow);
    int menuGlow = m_config->readNumEntry("MenuGlow", 2);
    m_dialog->m_menuGlow->setCurrentItem(menuGlow);
    int maxGlow = m_config->readNumEntry("MaximizerGlow", 1);
    m_dialog->m_maxGlow->setCurrentItem(maxGlow);
    int minGlow = m_config->readNumEntry("MinimizerGlow", 3);
    m_dialog->m_minGlow->setCurrentItem(minGlow);
    int stickyGlow = m_config->readNumEntry("StickerGlow", 4);
    m_dialog->m_stickyGlow->setCurrentItem(stickyGlow);
    int abbeGlow = m_config->readNumEntry("AboveBelowGlow", 5);
    m_dialog->m_abbeGlow->setCurrentItem(abbeGlow);
    int helpGlow = m_config->readNumEntry("HelpGlow", 4);
    m_dialog->m_helpGlow->setCurrentItem(helpGlow);
    //
    QColor defaultColor0(255,0,0);
    QColor defaultColor1(0,255,0);
    QColor defaultColor2(0,192,255);
    QColor defaultColor3(255,160,0);
    QColor defaultColor4(255,0,255);
    QColor defaultColor5(255,255,0);
    QColor defaultColor6(0,0,255);
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
}

void PowderConfig::save(KConfig*)
{
    m_config->setGroup("General");

    m_config->writeEntry("CornerRadius", m_dialog->m_cornerRadius->value());
    m_config->writeEntry("MaximizedCorner", m_dialog->m_maxCorner->isChecked());
    m_config->writeEntry("LightPosition", m_dialog->m_lightPos->currentItem());
    m_config->writeEntry("GradientContrast", m_dialog->m_gradientContrast->value());
    m_config->writeEntry("TitleSize", m_dialog->m_titleSize->value());
    m_config->writeEntry("CenterTitle", m_dialog->m_centerTitle->isChecked());
    m_config->writeEntry("StyledMenu", m_dialog->m_styledMenu->isChecked());
    m_config->writeEntry("HoverStyle", m_dialog->m_hoverStyle->currentItem());
    //
    m_config->writeEntry("CloserGlow", m_dialog->m_closerGlow->currentItem());
    m_config->writeEntry("MenuGlow", m_dialog->m_menuGlow->currentItem());
    m_config->writeEntry("MaximizerGlow", m_dialog->m_maxGlow->currentItem());
    m_config->writeEntry("MinimizerGlow", m_dialog->m_minGlow->currentItem());
    m_config->writeEntry("StickerGlow", m_dialog->m_stickyGlow->currentItem());
    m_config->writeEntry("AboveBelowGlow", m_dialog->m_abbeGlow->currentItem());
    m_config->writeEntry("HelpGlow", m_dialog->m_helpGlow->currentItem());
    //
    m_config->writeEntry("Color0", m_dialog->listColor0->color());
    m_config->writeEntry("Color1", m_dialog->listColor1->color());
    m_config->writeEntry("Color2", m_dialog->listColor2->color());
    m_config->writeEntry("Color3", m_dialog->listColor3->color());
    m_config->writeEntry("Color4", m_dialog->listColor4->color());
    m_config->writeEntry("Color5", m_dialog->listColor5->color());
    m_config->writeEntry("Color6", m_dialog->listColor6->color());
    //
    m_config->sync();
}

void PowderConfig::defaults()
{
    m_dialog->m_cornerRadius->setValue(6);
    m_dialog->m_maxCorner->setChecked(false);
    m_dialog->m_lightPos->setCurrentItem(0);
    m_dialog->m_gradientContrast->setValue(5);
    m_dialog->m_titleSize->setValue(18);
    m_dialog->m_centerTitle->setChecked(false);
    m_dialog->m_styledMenu->setChecked(true);
    m_dialog->m_hoverStyle->setCurrentItem(0);
    //
    m_dialog->m_closerGlow->setCurrentItem(0);
    m_dialog->m_menuGlow->setCurrentItem(2);
    m_dialog->m_maxGlow->setCurrentItem(1);
    m_dialog->m_minGlow->setCurrentItem(3);
    m_dialog->m_stickyGlow->setCurrentItem(4);
    m_dialog->m_abbeGlow->setCurrentItem(5);
    m_dialog->m_helpGlow->setCurrentItem(4);
    //
    m_dialog->listColor0->setColor(QColor(255,0,0));
    m_dialog->listColor1->setColor(QColor(0,255,0));
    m_dialog->listColor2->setColor(QColor(0,192,255));
    m_dialog->listColor3->setColor(QColor(255,160,0));
    m_dialog->listColor4->setColor(QColor(255,0,255));
    m_dialog->listColor5->setColor(QColor(255,255,0));
    m_dialog->listColor6->setColor(QColor(0,0,255));
}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

extern "C"
{
    QObject* allocate_config(KConfig* config, QWidget* parent) {
        return (new PowderConfig(config, parent));
    }
}

#include "powderconfig.moc"
