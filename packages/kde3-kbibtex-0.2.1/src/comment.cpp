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
#include <qregexp.h>
#include <qstringlist.h>

#include <comment.h>
#include <xsltransform.h>

namespace BibTeX
{

    Comment::Comment( const QString& text, bool useCommand ) : Element(), m_text( text ), m_useCommand( useCommand )
    {
        // nothing
    }

    Comment::Comment( Comment *other )
    {
        m_text = other->m_text;
        m_useCommand = other->m_useCommand;
    }

    Comment::~Comment()
    {
        // nothing
    }

    QString Comment::text() const
    {
        return m_text;
    }

    void Comment::setText( const QString &text )
    {
        m_text = text;
    }

    bool Comment::useCommand() const
    {
        return m_useCommand;
    }

    void Comment::setUseCommand( bool useCommand )
    {
        m_useCommand = useCommand;
    }

    bool Comment::containsPattern( const QString& pattern, EntryField::FieldType fieldType, FilterType filterType, bool caseSensitive ) const
    {
        if ( filterType == ftExact )
        {
            /** check for exact match */
            return fieldType == EntryField::ftUnknown && m_text.contains( pattern, caseSensitive );
        }
        else
        {
            /** for each word in the search pattern ... */
            QStringList words = QStringList::split( QRegExp( "\\s+" ), pattern );
            unsigned int hits = 0;
            for ( QStringList::Iterator it = words.begin(); it != words.end(); ++it )
            {
                /** check if word is contained in text */
                if ( fieldType == EntryField::ftUnknown && m_text.contains( *it, caseSensitive ) )
                    ++hits;
            }

            /** return success depending on filter type and number of hits */
            return ( filterType == ftAnyWord && hits > 0 || filterType == ftEveryWord && hits == words.count() );
        }
    }

    Element* Comment::clone()
    {
        return new Comment( this );
    }
}
