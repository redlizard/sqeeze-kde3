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
#include <kiconloader.h>
#include <klocale.h>

#include "entrywidgetwarningsitem.h"

namespace KBibTeX
{

    EntryWidgetWarningsItem::EntryWidgetWarningsItem( WarningLevel level, const QString &message, QWidget *widget, QListView *parent, const char* name )
            : QListViewItem( parent, name ), m_widget( widget )
    {
        KIconLoader * loader = KGlobal::instance() ->iconLoader();
        switch ( level )
        {
        case wlInformation:
            setPixmap( 0, loader->loadIcon( "messagebox_info", KIcon::NoGroup, KIcon::SizeSmall ) );
            break;
        case wlWarning:
            setPixmap( 0, loader->loadIcon( "messagebox_warning", KIcon::NoGroup, KIcon::SizeSmall ) );
            break;
        case wlError:
            setPixmap( 0, loader->loadIcon( "messagebox_critical", KIcon::NoGroup, KIcon::SizeSmall ) );
            break;
        default:
            {
                // nothing
            }
        }
        setText( 0, message );
    }


    EntryWidgetWarningsItem::~EntryWidgetWarningsItem()
    {
        // nothing
    }

    QWidget *EntryWidgetWarningsItem::widget()
    {
        return m_widget;
    }

}
