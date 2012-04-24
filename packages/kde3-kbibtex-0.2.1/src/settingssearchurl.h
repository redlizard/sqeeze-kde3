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
#ifndef SETTINGSSEARCHURL_H
#define SETTINGSSEARCHURL_H

#include <qwidget.h>

class QLineEdit;
class QPushButton;
class KListView;
class QListViewItem;

namespace KBibTeX
{
    class SettingsSearchURL : public QWidget
    {
        Q_OBJECT
    public:
        SettingsSearchURL( QWidget *parent = 0, const char *name = 0 );
        ~SettingsSearchURL();

        void applyData();
        void readData();

    signals:
        void configChanged();

    private slots:
        void slotNew();
        void slotEdit();
        void slotDelete();
        void slotReset();
        void updateGUI();

    private:
        QPushButton *m_pushbuttonNew;
        QPushButton *m_pushbuttonEdit;
        QPushButton *m_pushbuttonDelete;
        KListView *m_listviewSearchURLs;
        QPushButton *m_pushbuttonReset;

        int m_counter;

        void setupGUI();

        void urlDialog( QListViewItem * item = NULL );

    };
}

#endif
