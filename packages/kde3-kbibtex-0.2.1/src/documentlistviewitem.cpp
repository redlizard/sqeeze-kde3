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
#include <qlistview.h>
#include <qpainter.h>
#include <qpen.h>

#include <klocale.h>

#include "documentlistviewitem.h"
#include "entry.h"
#include "comment.h"
#include <preamble.h>
#include "macro.h"

namespace KBibTeX
{
    DocumentListViewItem::DocumentListViewItem( BibTeX::File *file, BibTeX::Element *element, DocumentListView *parent ) : KListViewItem( parent, parent->lastItem() ), m_element( element ), m_bibtexFile( file ), m_parent( parent ), m_unreadStatus( FALSE )
    {
        setTexts();
    }

    DocumentListViewItem::DocumentListViewItem( BibTeX::File *file, BibTeX::Element *element, DocumentListView *parent, QListViewItem *after ) : KListViewItem( parent, after ), m_element( element ), m_bibtexFile( file ), m_parent( parent )
    {
        setTexts();
    }

    DocumentListViewItem::~DocumentListViewItem()
    {
        // nothing
    }

    BibTeX::Element* DocumentListViewItem::element()
    {
        return m_element;
    }

    void DocumentListViewItem::updateItem()
    {
        setTexts();
    }

    void DocumentListViewItem::setUnreadStatus( bool unread )
    {
        m_unreadStatus = unread;
    }

    void DocumentListViewItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
    {
        if ( !p )
            return ;

        // make unread items bold
        if ( m_unreadStatus )
        {
            QFont f = p->font();
            f.setBold( TRUE );
            p->setFont( f );
        }

        KListViewItem::paintCell( p, cg, column, width, align );
    }


    void DocumentListViewItem::setTexts()
    {
        BibTeX::Entry * entry = dynamic_cast<BibTeX::Entry*>( m_element );

        if ( entry )
        {
            entry = new BibTeX::Entry( entry );
            m_bibtexFile->completeReferencedFields( entry );

            if ( entry->entryType() != BibTeX::Entry::etUnknown )
                setText( 0, BibTeX::Entry::entryTypeToString( entry->entryType() ) );
            else
                setText( 0, entry->entryTypeString() );
            setText( 1, entry->id() );

            for ( int i = 2; i < m_parent->columns(); i++ )
            {
                BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType )( i - 2 + ( int ) BibTeX::EntryField::ftAbstract );
                BibTeX::EntryField *field = entry->getField( fieldType );
                BibTeX::Value *value = NULL;
                if ( field != NULL && (( value = field->value() ) != NULL ) )
                    setText( i, value->text().replace( '{', "" ).replace( '}', "" ).replace( '~', "" ) );
                else
                    setText( i, "" );
            }

            delete entry;
        }
        else
        {
            BibTeX::Comment *comment = dynamic_cast<BibTeX::Comment*>( m_element );
            if ( comment )
            {
                setText( 0, i18n( "Comment" ) );
                QString text = comment->text();
                text.replace( '\n', ' ' );
                setText(( int ) BibTeX::EntryField::ftTitle - ( int ) BibTeX::EntryField::ftAbstract + 2, text );
            }
            else
            {
                BibTeX::Macro * macro = dynamic_cast<BibTeX::Macro*>( m_element );
                if ( macro )
                {
                    setText( 0, i18n( "Macro" ) );
                    setText( 1, macro->key() );
                    BibTeX::Value *value = macro->value();
                    int i = ( int ) BibTeX::EntryField::ftTitle - ( int ) BibTeX::EntryField::ftAbstract + 2;
                    if ( value )
                        setText( i, macro->value() ->text() );
                    else
                        setText( i, "" );
                }
                else
                {
                    BibTeX::Preamble *preamble = dynamic_cast<BibTeX::Preamble*>( m_element );
                    if ( preamble )
                    {
                        setText( 0, i18n( "Preamble" ) );
                        BibTeX::Value *value = preamble->value();
                        int i = ( int ) BibTeX::EntryField::ftTitle - ( int ) BibTeX::EntryField::ftAbstract + 2;
                        if ( value )
                            setText( i, preamble->value() ->text() );
                        else
                            setText( i, "" );
                    }

                }
            }
        }
    }
}
