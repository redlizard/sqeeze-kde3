/***************************************************************************
*   Copyright (C) 2004-2008 by Thomas Fischer                             *
*   fischer@unix-ag.uni-kl.de                                             *
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

#include <qlayout.h>

#include <klocale.h>
#include <kiconloader.h>

#include <settingsfileio.h>
#include <settingsediting.h>
#include <settingssearchurl.h>
#include <settingsidsuggestions.h>
#include <settingskeyword.h>
#include <settingsuserdefinedinput.h>
#include "settingsdlg.h"

namespace KBibTeX
{
    SettingsDlg::SettingsDlg( QWidget *parent, const char *name )
            : KDialogBase( Tabbed, i18n( "Configure" ), Ok | Apply | Cancel, Ok, parent, name, true )
    {
        QFrame * page = addPage( i18n( "&Editing" ) );
        QVBoxLayout *layout = new QVBoxLayout( page, 0, spacingHint() );
        m_editing = new SettingsEditing( page );
        layout->addWidget( m_editing );
        connect( m_editing, SIGNAL( configChanged() ), this, SLOT( slotConfigChanged() ) );

        page = addPage( i18n( "&File Open&&Save" ) );
        layout = new QVBoxLayout( page, 0, spacingHint() );
        m_fileIO = new SettingsFileIO( page );
        layout->addWidget( m_fileIO );
        connect( m_fileIO, SIGNAL( configChanged() ), this, SLOT( slotConfigChanged() ) );

        page = addPage( i18n( "&Search URLs" ) );
        layout = new QVBoxLayout( page, 0, spacingHint() );
        m_searchURL = new SettingsSearchURL( page );
        layout->addWidget( m_searchURL );
        connect( m_searchURL, SIGNAL( configChanged() ), this, SLOT( slotConfigChanged() ) );

        page = addPage( i18n( "Global &Keywords" ), QString::null, SmallIcon( "package" ) );
        layout = new QVBoxLayout( page, 0, spacingHint() );
        m_keyword = new SettingsKeyword( page );
        layout->addWidget( m_keyword );
        connect( m_keyword, SIGNAL( configChanged() ), this, SLOT( slotConfigChanged() ) );

        page = addPage( i18n( "Id Suggestions" ) );
        layout = new QVBoxLayout( page, 0, spacingHint() );
        m_idSuggestions = new SettingsIdSuggestions( page );
        layout->addWidget( m_idSuggestions );
        connect( m_idSuggestions, SIGNAL( configChanged() ), this, SLOT( slotConfigChanged() ) );

        page = addPage( i18n( "User Defined Fields" ) );
        layout = new QVBoxLayout( page, 0, spacingHint() );
        m_userDefinedInput = new SettingsUserDefinedInput( page );
        layout->addWidget( m_userDefinedInput );
        connect( m_userDefinedInput, SIGNAL( configChanged() ), this, SLOT( slotConfigChanged() ) );

        connect( this, SIGNAL( applyClicked() ), SLOT( slotApplySettings() ) );
    }


    SettingsDlg::~SettingsDlg()
    {
        // nothing
    }

    int SettingsDlg::exec()
    {
        m_fileIO->readData();
        m_editing->readData();
        m_searchURL->readData();
        m_keyword->readData();
        m_idSuggestions->readData();
        m_userDefinedInput->readData();
        enableButton( Apply, FALSE );

        return KDialogBase::exec();
    }

    void SettingsDlg::slotConfigChanged()
    {
        enableButton( Apply, TRUE );
    }

    void SettingsDlg::slotApplySettings()
    {
        m_fileIO->applyData();
        m_editing->applyData();
        m_searchURL->applyData();
        m_keyword->applyData();
        m_idSuggestions->applyData();
        m_userDefinedInput->applyData();
        enableButton( Apply, FALSE );
    }

    void SettingsDlg::accept()
    {
        slotApplySettings();
        KDialogBase::accept();
    }
}

#include "settingsdlg.moc"
