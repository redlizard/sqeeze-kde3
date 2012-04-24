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

#include <fieldlineedit.h>
#include <value.h>
#include <entryfield.h>
#include "entrywidgettitle.h"

namespace KBibTeX
{

    EntryWidgetTitle::EntryWidgetTitle( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name )
            : EntryWidgetTab( bibtexfile, isReadOnly, parent, name )
    {
        setupGUI();
    }

    EntryWidgetTitle::~EntryWidgetTitle()
    {
        // nothing
    }

    bool EntryWidgetTitle::isModified()
    {
        return m_fieldLineEditTitle->isModified() || m_fieldLineEditBookTitle->isModified() || m_fieldLineEditSeries->isModified();
    }

    void EntryWidgetTitle::updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll )
    {
        bool enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftTitle ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditTitle->setEnabled( enableWidget );
        m_fieldLineEditTitle->setFieldType( BibTeX::EntryField::ftTitle );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftBookTitle ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditBookTitle->setEnabled( enableWidget );
        m_fieldLineEditBookTitle->setFieldType( BibTeX::EntryField::ftBookTitle );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftSeries ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditSeries->setEnabled( enableWidget );
        m_fieldLineEditSeries->setFieldType( BibTeX::EntryField::ftSeries );
    }

    void EntryWidgetTitle::apply(BibTeX::Entry *entry  )
    {
        BibTeX::Value * value = m_fieldLineEditTitle->value();
        setValue( entry, BibTeX::EntryField::ftTitle, value );
        delete value;

        value = m_fieldLineEditBookTitle->value();
        setValue( entry, BibTeX::EntryField::ftBookTitle, value );
        delete value;

        value = m_fieldLineEditSeries->value();
        setValue( entry, BibTeX::EntryField::ftSeries, value );
        delete value;
    }

    void EntryWidgetTitle::reset(BibTeX::Entry *entry  )
    {
        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftTitle );
        m_fieldLineEditTitle->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftBookTitle );
        m_fieldLineEditBookTitle->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftSeries );
        m_fieldLineEditSeries->setValue( field != NULL ? field->value() : NULL );
    }

    void EntryWidgetTitle::updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings )
    {
        bool crossRefValid = FALSE;

        addMissingWarning( entryType, BibTeX::EntryField::ftTitle, m_fieldLineEditTitle->caption(), !m_fieldLineEditTitle->isEmpty(), m_fieldLineEditTitle, listViewWarnings );

        crossRefValid = m_crossRefEntry != NULL &&  m_crossRefEntry->getField( BibTeX::EntryField::ftTitle ) != NULL;
        addMissingWarning( entryType, BibTeX::EntryField::ftBookTitle, m_fieldLineEditBookTitle->caption(), !m_fieldLineEditBookTitle->isEmpty() || crossRefValid, m_fieldLineEditBookTitle, listViewWarnings );
        if ( crossRefValid && m_fieldLineEditBookTitle->isEmpty() )
            addCrossRefInfo( m_fieldLineEditBookTitle->caption(), m_fieldLineEditBookTitle, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftSeries, m_fieldLineEditSeries->caption(), !m_fieldLineEditSeries->isEmpty(), m_fieldLineEditSeries, listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditTitle, m_fieldLineEditTitle->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditBookTitle, m_fieldLineEditBookTitle->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditSeries, m_fieldLineEditSeries->caption(), listViewWarnings );
    }

    void EntryWidgetTitle::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 4, 2, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );
        gridLayout->setRowStretch( 3, 1 );

        QLabel *label = new QLabel( QString( "%1:" ).arg( i18n( "Title" ) ), this );
        gridLayout->addWidget( label, 0, 0 );
        m_fieldLineEditTitle = new KBibTeX::FieldLineEdit( i18n( "Title" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditTitle" );
        label->setBuddy( m_fieldLineEditTitle );
        gridLayout->addWidget( m_fieldLineEditTitle, 0, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Book Title" ) ), this );
        gridLayout->addWidget( label, 1, 0 );
        m_fieldLineEditBookTitle = new KBibTeX::FieldLineEdit( i18n( "Book Title" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditBookTitle" );
        label->setBuddy( m_fieldLineEditBookTitle );
        gridLayout->addWidget( m_fieldLineEditBookTitle, 1, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Series" ) ), this );
        gridLayout->addWidget( label, 2, 0 );
        m_fieldLineEditSeries = new KBibTeX::FieldLineEdit( i18n( "Series" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditSeries" );
        label->setBuddy( m_fieldLineEditSeries );
        gridLayout->addWidget( m_fieldLineEditSeries, 2, 1 );
    }

}
#include "entrywidgettitle.moc"
