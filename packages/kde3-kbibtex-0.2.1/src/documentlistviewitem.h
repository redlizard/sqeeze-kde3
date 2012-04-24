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
#ifndef DOCUMENTLISTVIEWITEM_H
#define DOCUMENTLISTVIEWITEM_H

#include <klistview.h>

#include <element.h>
#include "documentlistview.h"

namespace KBibTeX
{
    class DocumentListViewItem : public KListViewItem
    {
    public:
        DocumentListViewItem( BibTeX::File *file, BibTeX::Element *element, DocumentListView *parent );
        DocumentListViewItem( BibTeX::File *file, BibTeX::Element *element, DocumentListView *parent, QListViewItem *after );

        ~DocumentListViewItem();

        BibTeX::Element* element();
        void updateItem();
        void setUnreadStatus( bool unread );

    protected:
        void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );

    private:
        BibTeX::Element *m_element;
        BibTeX::File *m_bibtexFile;
        DocumentListView *m_parent;
        bool m_unreadStatus;

        void setTexts();
    };
}

#endif // DOCUMENTLISTVIEWITEM_H
