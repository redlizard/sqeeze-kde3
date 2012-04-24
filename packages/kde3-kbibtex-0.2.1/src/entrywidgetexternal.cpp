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
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtooltip.h>

#include <krun.h>
#include <kdialog.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <kapplication.h>

#include <fieldlineedit.h>
#include <settings.h>
#include "entrywidgetexternal.h"

namespace KBibTeX
{

    EntryWidgetExternal::EntryWidgetExternal( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name )
            : EntryWidgetTab( bibtexfile, isReadOnly, parent, name )
    {
        setupGUI();
    }

    EntryWidgetExternal::~EntryWidgetExternal()
    {
        // nothing
    }

    bool EntryWidgetExternal::isModified()
    {
        return m_fieldLineEditURL->isModified() || m_fieldLineEditDoi->isModified() || m_fieldLineEditLocalFile->isModified();
    }

    void EntryWidgetExternal::updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll )
    {
        bool enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftURL ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditURL->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftDoi ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditDoi->setEnabled( enableWidget );

        enableWidget = enableAll || BibTeX::Entry::getRequireStatus( entryType, BibTeX::EntryField::ftLocalFile ) != BibTeX::Entry::frsIgnored;
        m_fieldLineEditLocalFile->setEnabled( enableWidget );
        m_pushButtonBrowseLocalFile->setEnabled( !m_isReadOnly && enableWidget );

        updateGUI();
    }

    void EntryWidgetExternal::apply( BibTeX::Entry *entry )
    {
        BibTeX::Value * value = m_fieldLineEditURL->value();
        setValue( entry, BibTeX::EntryField::ftURL, value );
        delete value;

        value = m_fieldLineEditDoi->value();
        setValue( entry, BibTeX::EntryField::ftDoi, value );
        delete value;

        value = m_fieldLineEditLocalFile->value();
        setValue( entry, BibTeX::EntryField::ftLocalFile, value );
        delete value;
    }

    void EntryWidgetExternal::reset( BibTeX::Entry *entry )
    {
        disconnect( m_fieldLineEditURL, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );
        disconnect( m_fieldLineEditDoi, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );
        disconnect( m_fieldLineEditLocalFile, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );


        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftURL );
        m_fieldLineEditURL->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftDoi );
        m_fieldLineEditDoi->setValue( field != NULL ? field->value() : NULL );

        field = entry->getField( BibTeX::EntryField::ftLocalFile );
        m_fieldLineEditLocalFile->setValue( field != NULL ? field->value() : NULL );

        updateGUI();

        connect( m_fieldLineEditURL, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );
        connect( m_fieldLineEditDoi, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );
        connect( m_fieldLineEditLocalFile, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );
    }

    void EntryWidgetExternal::updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings )
    {
        addMissingWarning( entryType, BibTeX::EntryField::ftURL, m_fieldLineEditURL->caption(), !m_fieldLineEditURL->isEmpty(), m_fieldLineEditURL, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftDoi, m_fieldLineEditDoi->caption(), !m_fieldLineEditDoi->isEmpty(), m_fieldLineEditDoi, listViewWarnings );

        addMissingWarning( entryType, BibTeX::EntryField::ftLocalFile, m_fieldLineEditDoi->caption(), !m_fieldLineEditDoi->isEmpty(), m_fieldLineEditLocalFile, listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditURL, m_fieldLineEditURL->caption(), listViewWarnings );

        addFieldLineEditWarning( m_fieldLineEditDoi, m_fieldLineEditDoi->caption(), listViewWarnings );

        updateGUI();
    }

    void EntryWidgetExternal::openURL()
    {
        BibTeX::Value * value = m_fieldLineEditURL->value();
        KURL url = Settings::locateFile( value->text(), m_bibtexfile->fileName, this );
        if ( url.isValid() )
            new KRun( url, this );
        else
            m_pushButtonOpenURL->setEnabled( FALSE );
    }

    void EntryWidgetExternal::openDoi()
    {
        BibTeX::Value * value = m_fieldLineEditDoi->value();
        KURL url = Settings::doiURL( value->text() );
        if ( url.isValid() )
            new KRun( url, this );
        else
            m_pushButtonOpenDoi->setEnabled( FALSE );
    }

    void EntryWidgetExternal::openLocalFile()
    {
        BibTeX::Value * value = m_fieldLineEditLocalFile->value();
        KURL url = Settings::locateFile( value->text(), m_bibtexfile->fileName, this );
        if ( url.isValid() )
            new KRun( url, this );
        else
            m_pushButtonOpenLocalFile->setEnabled( FALSE );
    }

    void EntryWidgetExternal::browseLocalFile()
    {
        QString filename = KFileDialog::getOpenFileName();

        if ( !filename.isEmpty() )
        {
            BibTeX::Value * value = new BibTeX::Value();
            value->items.append( new BibTeX::PlainText( filename ) );
            m_fieldLineEditLocalFile->setValue( value );
        }
    }

    void EntryWidgetExternal::updateGUI()
    {
        BibTeX::Value * value = m_fieldLineEditURL->value();
        m_pushButtonOpenURL->setEnabled( value != NULL && Settings::locateFile( value->text(), m_bibtexfile->fileName, this ).isValid() );

        value = m_fieldLineEditLocalFile->value();
        m_pushButtonOpenLocalFile->setEnabled( value != NULL && Settings::locateFile( value->text(), m_bibtexfile->fileName, this ).isValid() );

        value = m_fieldLineEditDoi->value();
        m_pushButtonOpenDoi->setEnabled( value != NULL && Settings::doiURL( value->text() ).isValid() );
    }

    void EntryWidgetExternal::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 5, 3, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );
        gridLayout->setRowStretch( 4, 10 );

        QLabel *label = new QLabel( QString( "%1:" ).arg( i18n( "URL" ) ), this );
        gridLayout->addWidget( label, 0, 0 );
        m_fieldLineEditURL = new KBibTeX::FieldLineEdit( i18n( "URL" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditURL" );
        m_fieldLineEditURL->setFieldType( BibTeX::EntryField::ftURL );
        gridLayout->addWidget( m_fieldLineEditURL, 0, 1 );
        label->setBuddy( m_fieldLineEditURL );
        connect( m_fieldLineEditURL, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );

        m_pushButtonOpenURL = new QPushButton( this );
        gridLayout->addWidget( m_pushButtonOpenURL, 0, 2 );
        m_pushButtonOpenURL->setIconSet( QIconSet( SmallIcon( "fileopen" ) ) );
        m_pushButtonOpenURL->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        QToolTip::add( m_pushButtonOpenURL, QString( i18n( "Open %1" ) ).arg( i18n( "URL" ) ) );
        connect( m_pushButtonOpenURL, SIGNAL( clicked() ), this, SLOT( openURL() ) );

        KURLLabel *urllabel = new KURLLabel( "http://www.doi.org/", QString( "%1:" ).arg( i18n( "DOI" ) ), this );
        QToolTip::add( urllabel, i18n( "Digital Object Identifier" ) );
        urllabel->setFocusPolicy( QWidget::NoFocus );
        gridLayout->addWidget( urllabel, 1, 0 );
        m_fieldLineEditDoi = new KBibTeX::FieldLineEdit( i18n( "DOI" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditDoi" );
        m_fieldLineEditDoi->setFieldType( BibTeX::EntryField::ftDoi );
        gridLayout->addWidget( m_fieldLineEditDoi, 1, 1 );
        urllabel->setBuddy( m_fieldLineEditDoi );
        connect( m_fieldLineEditDoi, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );
        connect( urllabel, SIGNAL( leftClickedURL( const QString& ) ), kapp, SLOT( invokeBrowser( const QString& ) ) );

        m_pushButtonOpenDoi = new QPushButton( this );
        gridLayout->addWidget( m_pushButtonOpenDoi, 1, 2 );
        m_pushButtonOpenDoi->setIconSet( QIconSet( SmallIcon( "fileopen" ) ) );
        m_pushButtonOpenDoi->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        QToolTip::add( m_pushButtonOpenDoi, i18n( "Open DOI" ) );
        connect( m_pushButtonOpenDoi, SIGNAL( clicked() ), this, SLOT( openDoi() ) );

        label = new QLabel( QString( "%1:" ).arg( i18n( "Local File" ) ), this );
        gridLayout->addWidget( label, 2, 0 );
        m_fieldLineEditLocalFile = new KBibTeX::FieldLineEdit( i18n( "Local File" ), KBibTeX::FieldLineEdit::itSingleLine, m_isReadOnly, this, "m_fieldLineEditLocalFile" );
        m_fieldLineEditLocalFile->setFieldType( BibTeX::EntryField::ftLocalFile );
        gridLayout->addWidget( m_fieldLineEditLocalFile, 2, 1 );
        label->setBuddy( m_fieldLineEditLocalFile );
        connect( m_fieldLineEditLocalFile, SIGNAL( textChanged() ), this, SLOT( updateGUI() ) );

        m_pushButtonOpenLocalFile = new QPushButton( this );
        gridLayout->addWidget( m_pushButtonOpenLocalFile, 2, 2 );
        m_pushButtonOpenLocalFile->setIconSet( QIconSet( SmallIcon( "fileopen" ) ) );
        m_pushButtonOpenLocalFile->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        QToolTip::add( m_pushButtonOpenLocalFile, QString( i18n( "Open %1" ) ).arg( i18n( "Local File" ) ) );
        connect( m_pushButtonOpenLocalFile, SIGNAL( clicked() ), this, SLOT( openLocalFile() ) );

        QWidget *container = new QWidget( this );
        QHBoxLayout *containerLayout = new QHBoxLayout( container );
        gridLayout->addWidget( container, 3, 1 );
        m_pushButtonBrowseLocalFile = new QPushButton( i18n( "&Browse..." ), container );
        QToolTip::add( m_pushButtonBrowseLocalFile, i18n( "Browse for a local file" ) );
        m_pushButtonBrowseLocalFile->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        containerLayout->addWidget( m_pushButtonBrowseLocalFile );
        containerLayout->addStretch( 10 );
        m_pushButtonBrowseLocalFile->setIconSet( QIconSet( SmallIcon( "fileopen" ) ) );
        connect( m_pushButtonBrowseLocalFile, SIGNAL( clicked() ), this, SLOT( browseLocalFile() ) );
    }
}
#include "entrywidgetexternal.moc"
