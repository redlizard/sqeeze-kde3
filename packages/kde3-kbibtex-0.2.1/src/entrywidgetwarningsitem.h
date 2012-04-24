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
#ifndef KBIBTEXENTRYWIDGETWARNINGSITEM_H
#define KBIBTEXENTRYWIDGETWARNINGSITEM_H

#include <qlistview.h>

class QString;

namespace KBibTeX
{

    class EntryWidgetWarningsItem : public QListViewItem
    {
    public:
        enum WarningLevel {wlInformation = 1, wlWarning = 2, wlError = 3};

        EntryWidgetWarningsItem( WarningLevel level, const QString &message, QWidget *widget, QListView *parent, const char* name = NULL );
        ~EntryWidgetWarningsItem();

        QWidget *widget();

    private:
        QWidget *m_widget;
    };

}

#endif
