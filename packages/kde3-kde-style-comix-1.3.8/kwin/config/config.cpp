/***************************************************************************
 *   Copyright (C) 2004 by Jens Luetkens                                   *
 *   j.luetkens@limitland.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

#include "config.h"
#include "configdialog.h"

//
// Constructor
//
ComixConfig::ComixConfig(KConfig* config, QWidget* parent)
    : QObject(parent), m_config(0), m_dialog(0)
{
    // create the configuration object
    m_config = new KConfig("kwincomixrc");
    KGlobal::locale()->insertCatalogue("kwin_clients");

    // create and show the configuration dialog
    m_dialog = new ConfigDialog(parent);
    m_dialog->show();

    // load the configuration
    load(config);

    // setup the connections
    connect(m_dialog->titleAlign, SIGNAL(clicked(int)),
            this, SIGNAL(changed()));
    connect(m_dialog->titleExpand, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
    connect(m_dialog->windowSeparator, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));


}

//
// Destructor
//
ComixConfig::~ComixConfig()
{
    if (m_dialog) delete m_dialog;
    if (m_config) delete m_config;
}

//
// load(KConfig*)
//
void ComixConfig::load(KConfig*)
{

    m_config->setGroup("General");

    QString value = m_config->readEntry("TitleAlignment", "AlignLeft");
    QRadioButton *button = (QRadioButton*)m_dialog->titleAlign->child(value.latin1());
    if (button) button->setChecked(true);

    bool titleExpand = m_config->readBoolEntry("TitleExpand", false);
    m_dialog->titleExpand->setChecked(titleExpand);

    bool windowSeparator = m_config->readBoolEntry("WindowSeparator", false);
    m_dialog->windowSeparator->setChecked(windowSeparator);

}

//
// save(KConfig*)
//
void ComixConfig::save(KConfig*)
{
    m_config->setGroup("General");

    QRadioButton *button = (QRadioButton*)m_dialog->titleAlign->selected();
    if (button) m_config->writeEntry("TitleAlignment", QString(button->name()));

    m_config->writeEntry("TitleExpand", m_dialog->titleExpand->isChecked() );

    m_config->writeEntry("WindowSeparator", m_dialog->windowSeparator->isChecked() );

    m_config->sync();
}

//
// defaults()
//
void ComixConfig::defaults()
{

    QRadioButton *button =
        (QRadioButton*)m_dialog->titleAlign->child("AlignLeft");
    if (button) button->setChecked(true);

    m_dialog->titleExpand->setChecked(false);

    m_dialog->windowSeparator->setChecked(false);

}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

extern "C"
{
    QObject* allocate_config(KConfig* config, QWidget* parent) {
        return (new ComixConfig(config, parent));
    }
}

//#include "config.moc"
