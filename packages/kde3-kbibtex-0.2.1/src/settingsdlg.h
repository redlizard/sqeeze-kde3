/***************************************************************************
*   Copyright (C) 2004-2006 by Thomas Fischer                             *
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
#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include <kdialogbase.h>

namespace KBibTeX
{
    class SettingsFileIO;
    class SettingsEditing;
    class SettingsSearchURL;
    class SettingsKeyword;
    class SettingsIdSuggestions;
    class SettingsUserDefinedInput;

    class SettingsDlg : public KDialogBase
    {
        Q_OBJECT
    public:
        SettingsDlg( QWidget *parent = 0, const char *name = "SETTINGsdlg" );

        ~SettingsDlg();

    public slots:
        virtual int exec();

    protected slots:
        void slotConfigChanged();
        void slotApplySettings();
        virtual void accept();

    private:
        SettingsFileIO *m_fileIO;
        SettingsEditing *m_editing;
        SettingsSearchURL *m_searchURL;
        SettingsKeyword *m_keyword;
        SettingsIdSuggestions *m_idSuggestions;
        SettingsUserDefinedInput *m_userDefinedInput;
    };
}

#endif
