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
#ifndef BIBTEXCOMMENT_H
#define BIBTEXCOMMENT_H

#include <element.h>

namespace BibTeX
{

    class Comment : public Element
    {
    public:
        Comment( const QString &text, bool useCommand = false );
        Comment( Comment *other );
        virtual ~Comment();

        QString text() const;
        void setText( const QString &text );
        bool useCommand() const;
        void setUseCommand( bool useCommand );

        bool containsPattern( const QString& pattern, EntryField::FieldType fieldType = EntryField::ftUnknown, FilterType filterType = BibTeX::Element::ftExact, bool caseSensitive = FALSE ) const;
        Element* clone();

    private:
        QString m_text;
        bool m_useCommand;
    };

}
#endif
