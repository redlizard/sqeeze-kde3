/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

#include "config.h"
#include "configdialog.h"

KniftyConfig::KniftyConfig(KConfig* config, QWidget* parent)
    : QObject(parent), m_config(0), m_dialog(0)
{
    // create the configuration object
    m_config = new KConfig("kwinkniftyrc");
    KGlobal::locale()->insertCatalogue("kwin_knifty_config");

    // create and show the configuration dialog
    m_dialog = new ConfigDialog(parent);
    m_dialog->show();

    // load the configuration
    load(config);

    // setup the connections
    connect(m_dialog->titleAlign, SIGNAL(clicked(int)),
            this, SIGNAL(changed()));
    connect(m_dialog->highContrastButtons, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
    connect(m_dialog->designatedTitlebarColor, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
    connect(m_dialog->bordersBlendColor, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
    connect(m_dialog->titleShadow, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
}

KniftyConfig::~KniftyConfig()
{
    if (m_dialog) delete m_dialog;
    if (m_config) delete m_config;
}

void KniftyConfig::load(KConfig*)
{
    m_config->setGroup("General");


    QString value = m_config->readEntry("TitleAlignment", "AlignHCenter");
    QRadioButton *button = (QRadioButton*)m_dialog->titleAlign->child(value);
    if (button) button->setChecked(true);
    bool highContrastButtons = m_config->readBoolEntry("UseHighContrastHoveredButtons", false);
    m_dialog->highContrastButtons->setChecked(highContrastButtons);
    bool designatedTitlebarColor = m_config->readBoolEntry("UseTitleBarColor", false);
    m_dialog->designatedTitlebarColor->setChecked(designatedTitlebarColor);
    bool bordersBlendColor = m_config->readBoolEntry("BordersBlendColor", false);
    m_dialog->bordersBlendColor->setChecked(bordersBlendColor);
    bool titleShadow = m_config->readBoolEntry("TitleShadow", true);
    m_dialog->titleShadow->setChecked(titleShadow);
}

void KniftyConfig::save(KConfig*)
{
    m_config->setGroup("General");

    QRadioButton *button = (QRadioButton*)m_dialog->titleAlign->selected();
    if (button) m_config->writeEntry("TitleAlignment", QString(button->name()));
    m_config->writeEntry("UseHighContrastHoveredButtons", m_dialog->highContrastButtons->isChecked() );
    m_config->writeEntry("UseTitleBarColor", m_dialog->designatedTitlebarColor->isChecked() );
    m_config->writeEntry("BordersBlendColor", m_dialog->bordersBlendColor->isChecked() );
    m_config->writeEntry("TitleShadow", m_dialog->titleShadow->isChecked() );
    m_config->sync();
}

void KniftyConfig::defaults()
{
    QRadioButton *button =
        (QRadioButton*)m_dialog->titleAlign->child("AlignHCenter");
    if (button) button->setChecked(true);
    m_dialog->highContrastButtons->setChecked(false);
    m_dialog->designatedTitlebarColor->setChecked(false);
    m_dialog->bordersBlendColor->setChecked(false);
    m_dialog->titleShadow->setChecked(true);
}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

extern "C"
{
    QObject* allocate_config(KConfig* config, QWidget* parent) {
        return (new KniftyConfig(config, parent));
    }
}

#include "config.moc"
