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
#ifndef KBIBTEXSETTINGSKEYWORD_H
#define KBIBTEXSETTINGSKEYWORD_H

#include <qwidget.h>

class KListView;
class QPushButton;

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class SettingsKeyword : public QWidget
    {
        Q_OBJECT
    public:
        SettingsKeyword( QWidget *parent = 0, const char *name = 0 );
        ~SettingsKeyword();

        void applyData();
        void readData();

    signals:
        void configChanged();

    private slots:
        void slotNewKeyword();
        void slotEditKeyword();
        void slotDeleteKeyword();
        void slotImportKeywords();
        void updateGUI();

    private:
        QStringList *m_keywords;
        QStringList m_keywordsFromFile;
        KListView *m_listKeywords;
        QPushButton *m_buttonNewKeyword;
        QPushButton *m_buttonEditKeyword;
        QPushButton *m_buttonDeleteKeyword;
        QPushButton *m_buttonImportKeywords;

        void setupGUI();
    };
}

#endif
