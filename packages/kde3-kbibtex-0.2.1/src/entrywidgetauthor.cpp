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
#include <qlayout.h>
#include <qlabel.h>

#include <kdialog.h>
#include <klocale.h>

#include <fieldlistview.h>
#include <entrywidgetwarningsitem.h>
#include "entrywidgetauthor.h"

namespace KBibTeX
{

    EntryWidgetAuthor::EntryWidgetAuthor( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name )
            : EntryWidgetTab( bibtexfile, isReadOnly, parent, name )
    {
        setupGUI();
    }

    EntryWidgetAuthor::~EntryWidgetAuthor()
    {
        // nothing
    }

    bool EntryWidgetAuthor::isModified()
    {
        return m_fieldListViewAuthor->isModified() || m_fieldListViewEditor->isModified();
    }

    void EntryWidgetAuthor::updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll )
    {
        bool enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftAuthor ) != BibTeX::Entry::frsIgnored;
        m_fieldListViewAuthor->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftEditor ) != BibTeX::Entry::frsIgnored;
        m_fieldListViewEditor->setEnabled( enableWidget );
    }

    void EntryWidgetAuthor::apply( BibTeX::Entry *entry )
    {
        BibTeX::Value * value = m_fieldListViewAuthor->value();
        setValue( entry, BibTeX::EntryField::ftAuthor, value );
        if ( value != NULL ) delete value;

        value = m_fieldListViewEditor->value();
        setValue( entry, BibTeX::EntryField::ftEditor, value );
        if ( value != NULL ) delete value;
    }

    void EntryWidgetAuthor::reset( BibTeX::Entry *entry )
    {
        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftAuthor );
        m_fieldListViewAuthor->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftEditor );
        m_fieldListViewEditor->setValue( field != NULL ? field->value() : NULL );
    }

    void EntryWidgetAuthor::updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings )
    {
        if ( entryType == BibTeX::Entry::etBook || entryType == BibTeX::Entry::etInBook )
        {
            if ( m_fieldListViewAuthor->isEmpty() && m_fieldListViewEditor->isEmpty() )
                new EntryWidgetWarningsItem( EntryWidgetWarningsItem::wlWarning, i18n( "The fields 'Author' or 'Editor' are required, but both are missing" ), m_fieldListViewAuthor, listViewWarnings, "warning" );
        }
        else
        {
            addMissingWarning( entryType, BibTeX::EntryField::ftAuthor, m_fieldListViewAuthor->caption(), !m_fieldListViewAuthor->isEmpty(), m_fieldListViewAuthor, listViewWarnings );

            addMissingWarning( entryType, BibTeX::EntryField::ftEditor, m_fieldListViewEditor->caption(), !m_fieldListViewEditor->isEmpty(), m_fieldListViewEditor, listViewWarnings );
        }
    }

    void EntryWidgetAuthor::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 2, 3, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );
        gridLayout->setColSpacing( 1, KDialog::spacingHint() * 4 );

        QLabel *label = new QLabel( QString( "%1:" ).arg( i18n( "Author" ) ), this );
        gridLayout->addWidget( label, 0, 0 );
        m_fieldListViewAuthor = new KBibTeX::FieldListView( i18n( "Author" ), i18n( "May only contain ASCII characters, in case of doubt keep English form", "NewAuthor" ), m_isReadOnly, this );
        m_fieldListViewAuthor->setFieldType( BibTeX::EntryField::ftAuthor );
        gridLayout->addWidget( m_fieldListViewAuthor, 1, 0 );
        label->setBuddy( m_fieldListViewAuthor );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Editor" ) ), this );
        gridLayout->addWidget( label, 0, 2 );
        m_fieldListViewEditor = new KBibTeX::FieldListView( i18n( "Editor" ), i18n( "May only contain ASCII characters, in case of doubt keep English form", "NewEditor" ), m_isReadOnly, this );
        m_fieldListViewEditor->setFieldType( BibTeX::EntryField::ftEditor );
        gridLayout->addWidget( m_fieldListViewEditor, 1, 2 );
        label->setBuddy( m_fieldListViewEditor );
    }

}
#include "entrywidgetauthor.moc"
