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
#include <klocale.h>

#include <entry.h>
#include <entrywidgetwarningsitem.h>
#include <fieldlineedit.h>
#include "entrywidgettab.h"

namespace KBibTeX
{
    BibTeX::Entry *EntryWidgetTab::m_crossRefEntry = NULL;

    EntryWidgetTab::EntryWidgetTab(BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_isReadOnly( isReadOnly ), m_bibtexfile( bibtexfile )
    {
        // nothing
    }

    EntryWidgetTab::~EntryWidgetTab()
    {
        // nothing
    }

    void EntryWidgetTab::setValue( BibTeX::Entry *entry, const BibTeX::EntryField::FieldType fieldType, BibTeX::Value *value )
    {
        BibTeX::EntryField * field = entry->getField( fieldType );

        if ( value != NULL )
        {
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( fieldType );
                entry->addField( field );
            }
            field->setValue( value );
        }
        else
        {
            if ( field != NULL )
                entry->deleteField( fieldType );
        }
    }

    void EntryWidgetTab::addMissingWarning( BibTeX::Entry::EntryType entryType, BibTeX::EntryField::FieldType fieldType, const QString& label, bool valid, QWidget *widget, QListView *listView )
    {
        if ( BibTeX::Entry::getRequireStatus( entryType, fieldType ) == BibTeX::Entry::frsRequired && !valid )
            new EntryWidgetWarningsItem( EntryWidgetWarningsItem::wlWarning, QString( i18n( "The field '%1' is required, but missing" ) ).arg( label ), widget, listView, "warning" );
    }

    void EntryWidgetTab::addCrossRefInfo( const QString& label, QWidget *widget, QListView *listView )
    {
        QString crossRefId = m_crossRefEntry != NULL ? m_crossRefEntry->id() : "???";
        new EntryWidgetWarningsItem( EntryWidgetWarningsItem::wlInformation, QString( i18n( "Using cross referenced entry '%1' for field '%2'" ) ).arg( crossRefId ).arg( label ), widget, listView, "information" );
    }

    void EntryWidgetTab::addFieldLineEditWarning( FieldLineEdit *fieldLineEdit, const QString& label, QListView *listView )
    {
        switch ( fieldLineEdit->error() )
        {
        case FieldLineEdit::etInvalidStringKey:
            new EntryWidgetWarningsItem( EntryWidgetWarningsItem::wlError, QString( i18n( "The field '%1' contains string keys with invalid characters" ) ).arg( label ), fieldLineEdit, listView, "error" );
            break;
        case FieldLineEdit::etNoError:
            {
                // nothing
            }
            break;
        default:
            new EntryWidgetWarningsItem( EntryWidgetWarningsItem::wlError, QString( i18n( "The field '%1' contains some unknown error" ) ).arg( label ), fieldLineEdit, listView, "error" );
        }
    }
}
#include "entrywidgettab.moc"
