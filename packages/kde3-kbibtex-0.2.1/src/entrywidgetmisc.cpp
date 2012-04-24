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
#include "entrywidgetmisc.h"

namespace KBibTeX
{

    EntryWidgetMisc::EntryWidgetMisc( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name )
            : EntryWidgetTab( bibtexfile, isReadOnly, parent, name )
    {
        setupGUI();
    }

    EntryWidgetMisc::~EntryWidgetMisc()
    {
        // nothing
    }

    bool EntryWidgetMisc::isModified()
    {
        return m_fieldLineEditType->isModified() || m_fieldLineEditKey->isModified() || m_fieldLineEditNote->isModified() || m_fieldLineEditAnnote->isModified() || m_fieldLineEditAbstract->isModified();
    }

    void EntryWidgetMisc::updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll )
    {
        bool enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftType ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditType->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftKey ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditKey->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftNote ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditNote->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftAnnote ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditAnnote->setEnabled( enableWidget );
    }

    void EntryWidgetMisc::apply( BibTeX::Entry *entry )
    {
        BibTeX::Value * value = m_fieldLineEditType->value();
        setValue( entry, BibTeX::EntryField::ftType, value );
        delete value;

        value = m_fieldLineEditKey-> value();
        setValue( entry, BibTeX::EntryField::ftKey, value );
        delete value;

        value = m_fieldLineEditNote-> value();
        setValue( entry, BibTeX::EntryField::ftNote, value );
        delete value;

        value = m_fieldLineEditAnnote-> value();
        setValue( entry, BibTeX::EntryField::ftAnnote, value );
        delete value;

        value = m_fieldLineEditAbstract-> value();
        setValue( entry, BibTeX::EntryField::ftAbstract, value );
        delete value;
    }

    void EntryWidgetMisc::reset( BibTeX::Entry *entry )
    {
        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftType );
        m_fieldLineEditType->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftKey );
        m_fieldLineEditKey->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftNote );
        m_fieldLineEditNote->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftAnnote );
        m_fieldLineEditAnnote->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftAbstract );
        m_fieldLineEditAbstract->setValue( field != NULL ? field->value() : NULL );
    }

    void EntryWidgetMisc::updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings )
    {
        addMissingWarning( entryType, BibTeX::EntryField::ftType, m_fieldLineEditType->caption(), !m_fieldLineEditType->isEmpty(), m_fieldLineEditType, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftKey, m_fieldLineEditKey->caption(), !m_fieldLineEditKey->isEmpty(), m_fieldLineEditKey, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftNote, m_fieldLineEditNote->caption(), !m_fieldLineEditNote->isEmpty(), m_fieldLineEditNote, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftAnnote, m_fieldLineEditAnnote->caption(), !m_fieldLineEditAnnote->isEmpty(), m_fieldLineEditAnnote, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftAbstract, m_fieldLineEditAbstract->caption(), !m_fieldLineEditAbstract->isEmpty(), m_fieldLineEditAbstract, listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditType, m_fieldLineEditType->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditKey, m_fieldLineEditKey->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditNote, m_fieldLineEditNote->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditAnnote, m_fieldLineEditAnnote->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditAbstract, m_fieldLineEditAbstract->caption(), listViewWarnings );
    }

    void EntryWidgetMisc::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 5, 5, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );
        gridLayout->setColSpacing( 2, KDialog::spacingHint() * 4 );
        gridLayout->setRowStretch( 4, 1 );

        QLabel *label = new QLabel( QString( "%1:" ).arg( i18n( "Type" ) ), this );
        gridLayout->addWidget( label, 0, 0 );
        m_fieldLineEditType = new KBibTeX::FieldLineEdit( i18n( "Type" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditType" );
        m_fieldLineEditType->setFieldType( BibTeX::EntryField::ftType );
        label->setBuddy( m_fieldLineEditType );
        gridLayout->addWidget( m_fieldLineEditType, 0, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Key" ) ), this );
        gridLayout->addWidget( label, 0, 3 );
        m_fieldLineEditKey = new KBibTeX::FieldLineEdit( i18n( "Key" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditKey" );
        m_fieldLineEditKey->setFieldType( BibTeX::EntryField::ftKey );
        label->setBuddy( m_fieldLineEditKey );
        gridLayout->addWidget( m_fieldLineEditKey, 0, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Note" ) ), this );
        gridLayout->addWidget( label, 1, 0 );
        m_fieldLineEditNote = new KBibTeX::FieldLineEdit( i18n( "Note" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditNote" );
        m_fieldLineEditNote->setFieldType( BibTeX::EntryField::ftNote );
        label->setBuddy( m_fieldLineEditNote );
        gridLayout->addMultiCellWidget( m_fieldLineEditNote, 1, 1, 1, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Annote" ) ), this );
        gridLayout->addWidget( label, 2, 0 );
        m_fieldLineEditAnnote = new KBibTeX::FieldLineEdit( i18n( "Annote" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditAnnote" );
        m_fieldLineEditAnnote->setFieldType( BibTeX::EntryField::ftAnnote );
        label->setBuddy( m_fieldLineEditAnnote );
        gridLayout->addMultiCellWidget( m_fieldLineEditAnnote, 2, 2, 1, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Abstract" ) ), this );
        gridLayout->addWidget( label, 3, 0 );
        m_fieldLineEditAbstract = new KBibTeX::FieldLineEdit( i18n( "Abstract" ), KBibTeX::FieldLineEdit::itMultiLine, m_isReadOnly, this, "m_fieldLineEditAbstract" );
        m_fieldLineEditAbstract->setFieldType( BibTeX::EntryField::ftAbstract );
        label->setBuddy( m_fieldLineEditAbstract );
        gridLayout->addMultiCellWidget( m_fieldLineEditAbstract, 3, 4, 1, 4 );
    }

}
#include "entrywidgetmisc.moc"
