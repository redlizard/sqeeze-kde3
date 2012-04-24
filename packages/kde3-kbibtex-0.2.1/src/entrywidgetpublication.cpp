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
#include <qregexp.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <klocale.h>

#include <fieldlineedit.h>
#include <entrywidgetwarningsitem.h>
#include "entrywidgetpublication.h"

namespace KBibTeX
{

    EntryWidgetPublication::EntryWidgetPublication( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name )
            : EntryWidgetTab( bibtexfile, isReadOnly, parent, name )
    {
        setupGUI();
    }


    EntryWidgetPublication::~EntryWidgetPublication()
    {
        // nothing
    }

    bool EntryWidgetPublication::isModified()
    {
        return m_fieldLineEditOrganization->isModified() || m_fieldLineEditPublisher->isModified() || m_fieldLineEditSchool->isModified() || m_fieldLineEditInstitution->isModified() || m_fieldLineEditLocation->isModified() || m_fieldLineEditAddress->isModified() || m_fieldLineEditJournal->isModified() || m_fieldLineEditPages->isModified() || m_fieldLineEditEdition->isModified() || m_fieldLineEditChapter->isModified() || m_fieldLineEditVolume->isModified() || m_fieldLineEditNumber->isModified() || m_fieldLineEditMonth->isModified() || m_fieldLineEditYear->isModified() || m_fieldLineEditISBN->isModified() || m_fieldLineEditISSN->isModified() || m_fieldLineEditCrossRef->isModified() || m_fieldLineEditHowPublished->isModified();
    }

    void EntryWidgetPublication::updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll )
    {
        bool enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftOrganization ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditOrganization->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftPublisher ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditPublisher->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftSchool ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditSchool->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftInstitution ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditInstitution->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftLocation ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditLocation->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftAddress ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditAddress->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftJournal ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditJournal->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftEdition ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditEdition->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftVolume ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditVolume->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftNumber ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditNumber->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftMonth ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditMonth->setEnabled( enableWidget );
        m_pushButtonMonths->setEnabled( enableWidget && !m_isReadOnly );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftYear ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditYear->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftISBN ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditISBN->setEnabled( enableWidget );
        int len = isbn().length();
        m_pushButtonISBN->setEnabled( len == 10 || len == 13 );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftISSN ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditISSN->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftCrossRef ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditCrossRef->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftHowPublished ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditHowPublished->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftChapter ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditChapter->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftPages ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditPages->setEnabled( enableWidget );
    }

    void EntryWidgetPublication::apply( BibTeX::Entry *entry )
    {
        BibTeX::Value * value = m_fieldLineEditHowPublished->value();
        setValue( entry, BibTeX::EntryField::ftHowPublished, value );
        delete value;

        value = m_fieldLineEditPublisher->value();
        setValue( entry, BibTeX::EntryField::ftPublisher, value );
        delete value;

        value = m_fieldLineEditOrganization->value();
        setValue( entry, BibTeX::EntryField::ftOrganization, value );
        delete value;

        value = m_fieldLineEditInstitution->value();
        setValue( entry, BibTeX::EntryField::ftInstitution, value );
        delete value;

        value = m_fieldLineEditSchool->value();
        setValue( entry, BibTeX::EntryField::ftSchool, value );
        delete value;

        value = m_fieldLineEditLocation->value();
        setValue( entry, BibTeX::EntryField::ftLocation, value );
        delete value;

        value = m_fieldLineEditAddress->value();
        setValue( entry, BibTeX::EntryField::ftAddress, value );
        delete value;

        value = m_fieldLineEditJournal->value();
        setValue( entry, BibTeX::EntryField::ftJournal, value );
        delete value;

        value = m_fieldLineEditPages->value();
        if ( value != NULL && value->items.first() != NULL )
        {
            BibTeX::PlainText *plainText = dynamic_cast<BibTeX::PlainText*>( value->items.first() );
            if ( plainText != NULL )
                plainText->setText( plainText->text().replace( QRegExp( "\\s*--?\\s*" ), QChar( 0x2013 ) ) );
        }
        setValue( entry, BibTeX::EntryField::ftPages, value );
        delete value;

        value = m_fieldLineEditEdition->value();
        setValue( entry, BibTeX::EntryField::ftEdition, value );
        delete value;

        value = m_fieldLineEditChapter->value();
        setValue( entry, BibTeX::EntryField::ftChapter, value );
        delete value;

        value = m_fieldLineEditVolume->value();
        setValue( entry, BibTeX::EntryField::ftVolume, value );
        delete value;

        value = m_fieldLineEditNumber->value();
        setValue( entry, BibTeX::EntryField::ftNumber, value );
        delete value;

        value = m_fieldLineEditMonth->value();
        setValue( entry, BibTeX::EntryField::ftMonth, value );
        delete value;

        value = m_fieldLineEditYear->value();
        setValue( entry, BibTeX::EntryField::ftYear, value );
        delete value;

        value = m_fieldLineEditISBN->value();
        setValue( entry, BibTeX::EntryField::ftISBN, value );
        delete value;

        value = m_fieldLineEditISSN->value();
        setValue( entry, BibTeX::EntryField::ftISSN, value );
        delete value;

        value = m_fieldLineEditCrossRef->value();
        setValue( entry, BibTeX::EntryField::ftCrossRef, value );
        delete value;
    }

    void EntryWidgetPublication::reset( BibTeX::Entry *entry )
    {
        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftHowPublished );
        m_fieldLineEditHowPublished->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftPublisher );
        m_fieldLineEditPublisher->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftOrganization );
        m_fieldLineEditOrganization->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftInstitution );
        m_fieldLineEditInstitution->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftSchool );
        m_fieldLineEditSchool->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftLocation );
        m_fieldLineEditLocation->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftAddress );
        m_fieldLineEditAddress->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftJournal );
        m_fieldLineEditJournal->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftPages );
        m_fieldLineEditPages->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftEdition );
        m_fieldLineEditEdition->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftChapter );
        m_fieldLineEditChapter->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftVolume );
        m_fieldLineEditVolume->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftNumber );
        m_fieldLineEditNumber->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftMonth );
        m_fieldLineEditMonth->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftYear );
        m_fieldLineEditYear->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftISBN );
        m_fieldLineEditISBN->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftISSN );
        m_fieldLineEditISSN->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftCrossRef );
        m_fieldLineEditCrossRef->setValue( field != NULL ? field->value() : NULL );
        slotSetCrossRefEntry();
    }

    void EntryWidgetPublication::updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings )
    {
        bool crossRefValid = FALSE;
        BibTeX::Value *value = NULL;

        addMissingWarning( entryType, BibTeX::EntryField::ftHowPublished, m_fieldLineEditHowPublished->caption(), !m_fieldLineEditHowPublished->isEmpty(), m_fieldLineEditHowPublished, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftPublisher, m_fieldLineEditPublisher->caption(), !m_fieldLineEditPublisher->isEmpty(), m_fieldLineEditPublisher, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftOrganization, m_fieldLineEditOrganization->caption(), !m_fieldLineEditOrganization->isEmpty(), m_fieldLineEditOrganization, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftInstitution, m_fieldLineEditInstitution->caption(), !m_fieldLineEditInstitution->isEmpty(), m_fieldLineEditInstitution, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftSchool, m_fieldLineEditSchool->caption(), !m_fieldLineEditSchool->isEmpty(), m_fieldLineEditSchool, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftLocation, m_fieldLineEditLocation->caption(), !m_fieldLineEditLocation->isEmpty(), m_fieldLineEditLocation, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftAddress, m_fieldLineEditAddress->caption(), !m_fieldLineEditAddress->isEmpty(), m_fieldLineEditAddress, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftJournal, m_fieldLineEditJournal->caption(), !m_fieldLineEditJournal->isEmpty(), m_fieldLineEditJournal, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftEdition, m_fieldLineEditEdition->caption(), !m_fieldLineEditEdition->isEmpty(), m_fieldLineEditEdition, listViewWarnings );

        crossRefValid = m_crossRefEntry != NULL && m_crossRefEntry->getField( BibTeX::EntryField::ftVolume ) != NULL;
        addMissingWarning( entryType, BibTeX::EntryField::ftVolume, m_fieldLineEditVolume->caption(), !m_fieldLineEditVolume->isEmpty() || crossRefValid, m_fieldLineEditVolume, listViewWarnings );
        if ( crossRefValid && m_fieldLineEditVolume->isEmpty() )
            addCrossRefInfo( m_fieldLineEditVolume->caption(), m_fieldLineEditVolume, listViewWarnings );

        crossRefValid = m_crossRefEntry != NULL && m_crossRefEntry->getField( BibTeX::EntryField::ftNumber ) != NULL;
        addMissingWarning( entryType, BibTeX::EntryField::ftNumber, m_fieldLineEditNumber->caption(), !m_fieldLineEditNumber->isEmpty() || crossRefValid, m_fieldLineEditNumber, listViewWarnings );
        if ( crossRefValid && m_fieldLineEditNumber->isEmpty() )
            addCrossRefInfo( m_fieldLineEditNumber->caption(), m_fieldLineEditNumber, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftMonth, m_fieldLineEditMonth->caption(), !m_fieldLineEditMonth->isEmpty(), m_fieldLineEditMonth, listViewWarnings );

        crossRefValid = m_crossRefEntry != NULL && m_crossRefEntry->getField( BibTeX::EntryField::ftYear ) != NULL;
        addMissingWarning( entryType, BibTeX::EntryField::ftYear, m_fieldLineEditYear->caption(), !m_fieldLineEditYear->isEmpty() || crossRefValid, m_fieldLineEditYear, listViewWarnings );
        if ( crossRefValid && m_fieldLineEditYear->isEmpty() )
            addCrossRefInfo( m_fieldLineEditYear->caption(), m_fieldLineEditYear, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftISBN, m_fieldLineEditISBN->caption(), !m_fieldLineEditISBN->isEmpty(), m_fieldLineEditISBN, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftISSN, m_fieldLineEditISSN->caption(), !m_fieldLineEditISSN->isEmpty(), m_fieldLineEditISSN, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftCrossRef, m_fieldLineEditCrossRef->caption(), !m_fieldLineEditCrossRef->isEmpty(), m_fieldLineEditCrossRef, listViewWarnings );

        if ( entryType == BibTeX::Entry::etInBook )
        {
            if ( m_fieldLineEditChapter->isEmpty() && m_fieldLineEditPages->isEmpty() )
                new EntryWidgetWarningsItem( EntryWidgetWarningsItem::wlWarning, i18n( "The fields 'Chapter' or 'Pages' are required, but both are missing" ), m_fieldLineEditPages, listViewWarnings, "warning" );
        }
        else
        {
            addMissingWarning( entryType, BibTeX::EntryField::ftPages, m_fieldLineEditPages->caption(), !m_fieldLineEditPages->isEmpty(), m_fieldLineEditPages, listViewWarnings );

            addMissingWarning( entryType, BibTeX::EntryField::ftChapter, m_fieldLineEditChapter->caption(), !m_fieldLineEditChapter->isEmpty(), m_fieldLineEditChapter, listViewWarnings );
        }

        value = m_fieldLineEditJournal->value();
        if ( value != NULL && !value->items.isEmpty() )
        {
            BibTeX::MacroKey *key = dynamic_cast<BibTeX::MacroKey*>( value->items[0] );
            if ( key != NULL )
            {
                QString jourText = key->text();
                if ( !jourText.isEmpty() && !m_bibtexfile->containsKey( jourText ) )
                    new EntryWidgetWarningsItem( EntryWidgetWarningsItem::wlWarning, i18n( "The field 'Journal' refers to a not existing entry" ), m_fieldLineEditJournal, listViewWarnings, "warning" );
            }
        }

        addFieldLineEditWarning( m_fieldLineEditHowPublished, m_fieldLineEditHowPublished->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditPublisher, m_fieldLineEditPublisher->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditOrganization, m_fieldLineEditOrganization->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditInstitution, m_fieldLineEditInstitution->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditSchool, m_fieldLineEditSchool->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditLocation, m_fieldLineEditLocation->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditAddress, m_fieldLineEditAddress->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditJournal, m_fieldLineEditJournal->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditPages, m_fieldLineEditPages->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditEdition, m_fieldLineEditEdition->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditChapter, m_fieldLineEditChapter->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditVolume, m_fieldLineEditVolume->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditNumber, m_fieldLineEditNumber->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditMonth, m_fieldLineEditMonth->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditYear, m_fieldLineEditYear->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditISBN, m_fieldLineEditISBN->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditISSN, m_fieldLineEditISSN->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditCrossRef, m_fieldLineEditCrossRef->caption(), listViewWarnings );

        /** misusing this function to enable/disable ISBN button */
        int len = isbn().length();
        m_pushButtonISBN->setEnabled( len == 10 || len == 13 );
    }

    void EntryWidgetPublication::slotSetMonth( int month )
    {
        BibTeX::MacroKey *item = new BibTeX::MacroKey( BibTeX::MonthsTriple[ month ] );
        BibTeX::Value * value = new BibTeX::Value();
        value->items.append( item );
        m_fieldLineEditMonth->setValue( value );
        delete value;
    }

    void EntryWidgetPublication::slotSetCrossRefEntry()
    {
        m_crossRefEntry = NULL;
        if ( m_fieldLineEditCrossRef->value() != NULL )
        {
            QString crossRefText = m_fieldLineEditCrossRef->value() ->text();
            if ( !crossRefText.isEmpty() )
                m_crossRefEntry = dynamic_cast<BibTeX::Entry*>( m_bibtexfile->containsKey( crossRefText ) );
        }
    }

    void EntryWidgetPublication::slotOpenISBN()
    {
        kapp->invokeBrowser( QString( i18n( "http://en.wikipedia.org/w/index.php?title=Special:Booksources&isbn=%1" ) ).arg( isbn() ) );
    }

    /**
     *
     */
    void EntryWidgetPublication::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 10, 5, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );
        gridLayout->setColSpacing( 2, KDialog::spacingHint() * 4 );
        gridLayout->setRowStretch( 9, 1 );

        QLabel *label = new QLabel( QString( "%1:" ).arg( i18n( "Journal" ) ), this );
        gridLayout->addWidget( label, 0, 0 );
        m_fieldLineEditJournal = new KBibTeX::FieldLineEdit( i18n( "Journal" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditJournal" );
        m_fieldLineEditJournal ->setFieldType( BibTeX::EntryField::ftJournal );
        label->setBuddy( m_fieldLineEditJournal );
        gridLayout->addWidget( m_fieldLineEditJournal, 0, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Volume" ) ), this );
        gridLayout->addWidget( label, 1, 0 );
        m_fieldLineEditVolume = new KBibTeX::FieldLineEdit( i18n( "Volume" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditVolume" );
        m_fieldLineEditVolume->setFieldType( BibTeX::EntryField::ftVolume );
        label->setBuddy( m_fieldLineEditVolume );
        gridLayout->addWidget( m_fieldLineEditVolume, 1, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Number" ) ), this );
        gridLayout->addWidget( label, 2, 0 );
        m_fieldLineEditNumber = new KBibTeX::FieldLineEdit( i18n( "Number" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditNumber" );
        m_fieldLineEditNumber ->setFieldType( BibTeX::EntryField::ftNumber );
        label->setBuddy( m_fieldLineEditNumber );
        gridLayout->addWidget( m_fieldLineEditNumber, 2, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Month" ) ), this );
        gridLayout->addWidget( label, 3, 0 );
        QHBoxLayout *innerLayout = new QHBoxLayout();
        gridLayout->addLayout( innerLayout, 3, 1 );
        m_pushButtonMonths = new QPushButton( this, "m_pushButtonMonths" );
        m_pushButtonMonths->setIconSet( QIconSet( SmallIcon( "vcalendar" ) ) );
        m_pushButtonMonths->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
        m_pushButtonMonths->setEnabled( !m_isReadOnly );
        innerLayout->addWidget( m_pushButtonMonths );
        m_fieldLineEditMonth = new KBibTeX::FieldLineEdit( i18n( "Month" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditMonth" );
        m_fieldLineEditMonth->setFieldType( BibTeX::EntryField::ftMonth );
        label->setBuddy( m_fieldLineEditMonth );
        innerLayout->addWidget( m_fieldLineEditMonth );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Year" ) ), this );
        gridLayout->addWidget( label, 4, 0 );
        m_fieldLineEditYear = new KBibTeX::FieldLineEdit( i18n( "Year" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditYear" );
        m_fieldLineEditYear->setFieldType( BibTeX::EntryField::ftYear );
        label->setBuddy( m_fieldLineEditYear );
        gridLayout->addWidget( m_fieldLineEditYear, 4, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Pages" ) ), this );
        gridLayout->addWidget( label, 5, 0 );
        m_fieldLineEditPages = new KBibTeX::FieldLineEdit( i18n( "Pages" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditPages" );
        m_fieldLineEditPages->setFieldType( BibTeX::EntryField::ftPages );
        label->setBuddy( m_fieldLineEditPages );
        gridLayout->addWidget( m_fieldLineEditPages, 5, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Edition" ) ), this );
        gridLayout->addWidget( label, 6, 0 );
        m_fieldLineEditEdition = new KBibTeX::FieldLineEdit( i18n( "Edition" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditEdition" );
        m_fieldLineEditEdition->setFieldType( BibTeX::EntryField::ftEdition );
        label->setBuddy( m_fieldLineEditEdition );
        gridLayout->addWidget( m_fieldLineEditEdition, 6, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Chapter" ) ), this );
        gridLayout->addWidget( label, 7, 0 );
        m_fieldLineEditChapter = new KBibTeX::FieldLineEdit( i18n( "Chapter" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditChapter" );
        m_fieldLineEditChapter ->setFieldType( BibTeX::EntryField::ftChapter );
        label->setBuddy( m_fieldLineEditChapter );
        gridLayout->addWidget( m_fieldLineEditChapter, 7, 1 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Cross Reference" ) ), this );
        gridLayout->addWidget( label, 8, 0 );
        m_fieldLineEditCrossRef = new KBibTeX::FieldLineEdit( i18n( "Cross Reference" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditCrossRef" );
        m_fieldLineEditCrossRef->setFieldType( BibTeX::EntryField::ftCrossRef );
        label->setBuddy( m_fieldLineEditCrossRef );
        gridLayout->addWidget( m_fieldLineEditCrossRef, 8, 1 );
        connect( m_fieldLineEditCrossRef, SIGNAL( textChanged() ), this, SLOT( slotSetCrossRefEntry() ) );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Organization" ) ), this );
        gridLayout->addWidget( label, 0, 3 );
        m_fieldLineEditOrganization = new KBibTeX::FieldLineEdit( i18n( "Organization" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditOrganization" );
        m_fieldLineEditOrganization->setFieldType( BibTeX::EntryField::ftOrganization );
        label->setBuddy( m_fieldLineEditOrganization );
        gridLayout->addWidget( m_fieldLineEditOrganization, 0, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Publisher" ) ), this );
        gridLayout->addWidget( label, 1, 3 );
        m_fieldLineEditPublisher = new KBibTeX::FieldLineEdit( i18n( "Publisher" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditPublisher" );
        m_fieldLineEditPublisher->setFieldType( BibTeX::EntryField::ftPublisher );
        label->setBuddy( m_fieldLineEditPublisher );
        gridLayout->addWidget( m_fieldLineEditPublisher, 1, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "School" ) ), this );
        gridLayout->addWidget( label, 2, 3 );
        m_fieldLineEditSchool = new KBibTeX::FieldLineEdit( i18n( "School" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditSchool" );
        m_fieldLineEditSchool->setFieldType( BibTeX::EntryField::ftSchool );
        label->setBuddy( m_fieldLineEditSchool );
        gridLayout->addWidget( m_fieldLineEditSchool, 2, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Institution" ) ), this );
        gridLayout->addWidget( label, 3, 3 );
        m_fieldLineEditInstitution = new KBibTeX::FieldLineEdit( i18n( "Institution" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditInstitution" );
        m_fieldLineEditInstitution->setFieldType( BibTeX::EntryField::ftInstitution );
        label->setBuddy( m_fieldLineEditInstitution );
        gridLayout->addWidget( m_fieldLineEditInstitution, 3, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Location" ) ), this );
        gridLayout->addWidget( label, 4, 3 );
        m_fieldLineEditLocation = new KBibTeX::FieldLineEdit( i18n( "Location" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditLocation" );
        m_fieldLineEditLocation->setFieldType( BibTeX::EntryField::ftLocation );
        label->setBuddy( m_fieldLineEditLocation );
        gridLayout->addWidget( m_fieldLineEditLocation, 4, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Address" ) ), this );
        gridLayout->addWidget( label, 5, 3 );
        m_fieldLineEditAddress = new KBibTeX::FieldLineEdit( i18n( "Address" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditAddress" );
        m_fieldLineEditAddress->setFieldType( BibTeX::EntryField::ftAddress );
        label->setBuddy( m_fieldLineEditAddress );
        gridLayout->addWidget( m_fieldLineEditAddress, 5, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "ISBN" ) ), this );
        gridLayout->addWidget( label, 6, 3 );
        innerLayout = new QHBoxLayout();
        gridLayout->addLayout( innerLayout, 6, 4 );
        m_pushButtonISBN = new QPushButton( this, "m_pushButtonMonths" );
        m_pushButtonISBN->setIconSet( QIconSet( SmallIcon( "find" ) ) );
        m_pushButtonISBN->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
        QToolTip::add( m_pushButtonISBN, i18n( "Lookup ISBN number in Wikipedia" ) );
        innerLayout->addWidget( m_pushButtonISBN );
        m_fieldLineEditISBN = new KBibTeX::FieldLineEdit( i18n( "ISBN" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditISBN" );
        m_fieldLineEditISBN ->setFieldType( BibTeX::EntryField::ftISBN );
        label->setBuddy( m_fieldLineEditISBN );
        innerLayout->addWidget( m_fieldLineEditISBN );
        connect( m_pushButtonISBN, SIGNAL( clicked() ), this, SLOT( slotOpenISBN() ) );

        label = new QLabel( QString( "%1:" ).arg( i18n( "ISSN" ) ), this );
        gridLayout->addWidget( label, 7, 3 );
        m_fieldLineEditISSN = new KBibTeX::FieldLineEdit( i18n( "ISSN" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditISSN" );
        m_fieldLineEditISSN->setFieldType( BibTeX::EntryField::ftISSN );
        label->setBuddy( m_fieldLineEditISSN );
        gridLayout->addWidget( m_fieldLineEditISSN, 7, 4 );

        label = new QLabel( QString( "%1:" ).arg( i18n( "How Published" ) ), this );
        gridLayout->addWidget( label, 8, 3 );
        m_fieldLineEditHowPublished = new KBibTeX::FieldLineEdit( i18n( "How Published" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditHowPublished" );
        m_fieldLineEditHowPublished->setFieldType( BibTeX::EntryField::ftHowPublished );
        label->setBuddy( m_fieldLineEditHowPublished );
        gridLayout->addWidget( m_fieldLineEditHowPublished, 8, 4 );

        QPopupMenu *menuMonths = new QPopupMenu( this, "menuMonths" );
        for ( int i = 0; i < 12; i++ )
            menuMonths->insertItem( BibTeX::Months[ i ], i );
        m_pushButtonMonths->setPopup( menuMonths );
        connect( menuMonths, SIGNAL( activated( int ) ), this, SLOT( slotSetMonth( int ) ) );
    }

    QString EntryWidgetPublication::isbn() const
    {
        BibTeX::Value *value = m_fieldLineEditISBN->value();
        if ( value == NULL )
            return "";

        return value->text().replace( QRegExp( "[^0-9X]" ), "" );
    }
}
#include "entrywidgetpublication.moc"
