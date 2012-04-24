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
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include <krun.h>
#include <kdialog.h>
#include <kapplication.h>
#include <klistview.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kio/netaccess.h>

#include <fieldlineedit.h>
#include <settings.h>
#include "entrywidgetother.h"

namespace KBibTeX
{

    EntryWidgetOther::EntryWidgetOther( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name )
            : EntryWidgetTab( bibtexfile, isReadOnly, parent, name ), m_isModified( FALSE )
    {
        setupGUI();
    }

    EntryWidgetOther::~EntryWidgetOther()
    {
        m_listViewFields->clear();
    }

    bool EntryWidgetOther::isModified()
    {
        return m_isModified;
    }

    void EntryWidgetOther::updateGUI( BibTeX::Entry::EntryType /*entryType*/, bool /*enableAll*/ )
    {
        // nothing
    }

    void EntryWidgetOther::apply( BibTeX::Entry *entry )
    {
        for ( QStringList::iterator it = m_deletedFields.begin(); it != m_deletedFields.end(); it++ )
            entry->deleteField( *it );
        m_deletedFields.clear();

        for ( QListViewItemIterator it( m_listViewFields ); it.current(); it++ )
        {
            ValueListViewItem *vlvi = dynamic_cast<KBibTeX::ValueListViewItem*>( it.current() );
            if ( vlvi != NULL )
            {
                BibTeX::EntryField * field = entry->getField( vlvi->title() );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( vlvi->title() );
                    entry->addField( field );
                }

                field->setValue( vlvi->value() );
            }
        }

        m_isModified = FALSE;
    }

    void EntryWidgetOther::reset( BibTeX::Entry *entry )
    {
        m_listViewFields->clear();
        m_deletedFields.clear();

        for ( QValueList<BibTeX::EntryField*>::const_iterator it = entry->begin(); it != entry->end(); it++ )
        {
            BibTeX::EntryField *field = *it;
            bool doAdd = field->fieldType() == BibTeX::EntryField::ftUnknown;

            Settings * settings = Settings::self();
            QString ftn = field->fieldTypeName().lower();
            for ( unsigned int i = 0; doAdd && i < settings->userDefinedInputFields.count(); ++i )
                doAdd &= settings->userDefinedInputFields[i]->name.lower() != ftn;

            if ( doAdd )
                new ValueListViewItem( field->fieldTypeName(), field->value(), m_listViewFields );
        }

        m_isModified = FALSE;
    }

    void EntryWidgetOther::updateWarnings( BibTeX::Entry::EntryType /*entryType*/, QListView * /*listViewWarnings*/ )
    {
        // no warnings neccessary for user fields
    }

    void EntryWidgetOther::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 5, 3, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );

        m_lineEditKey = new QLineEdit( this, "m_lineEditKey" );
        m_lineEditKey->setReadOnly( m_isReadOnly );
        gridLayout->addWidget( m_lineEditKey, 0, 1 );
        QToolTip::add( m_lineEditKey, i18n( "Name of the user-defined field" ) );
        QWhatsThis::add( m_lineEditKey, i18n( "The name of the user-defined field. Should only contain letters and numbers." ) );
        QLabel *label = new QLabel( i18n( "&Name:" ), this );
        label->setBuddy( m_lineEditKey );
        gridLayout->addWidget( label, 0, 0 );

        m_pushButtonAdd = new QPushButton( i18n( "&Add" ), this, "m_pushButtonAdd" );
        gridLayout->addWidget( m_pushButtonAdd, 0, 2 );
        m_pushButtonAdd->setIconSet( QIconSet( SmallIcon( "add" ) ) );

        m_fieldLineEditValue = new KBibTeX::FieldLineEdit( i18n( "Value" ), KBibTeX::FieldLineEdit::itMultiLine, m_isReadOnly, this, "m_fieldLineEditValue" );
        gridLayout->addMultiCellWidget( m_fieldLineEditValue, 1, 2, 1, 2 );
        QToolTip::add( m_fieldLineEditValue, i18n( "Content of the user-defined field" ) );
        QWhatsThis::add( m_fieldLineEditValue, i18n( "The content of the user-defined field. May contain any text." ) );
        label = new QLabel( i18n( "&Content:" ), this );
        label->setBuddy( m_fieldLineEditValue );
        gridLayout->addWidget( label, 1, 0 );

        QSpacerItem* spacer = new QSpacerItem( 20, 110, QSizePolicy::Minimum, QSizePolicy::Expanding );
        gridLayout->addItem( spacer, 2, 0 );

        m_listViewFields = new KListView( this, "m_listViewFields" );
        m_listViewFields->addColumn( i18n( "Key" ) );
        m_listViewFields->addColumn( i18n( "Value" ) );
        m_listViewFields->setAllColumnsShowFocus( true );
        m_listViewFields->setFullWidth( TRUE );
        gridLayout->addMultiCellWidget( m_listViewFields, 3, 5, 1, 1 );

        label = new QLabel( i18n( "&List:" ), this );
        label->setBuddy( m_listViewFields );
        label->setAlignment( Qt::AlignTop );
        gridLayout->addWidget( label, 3, 0 );

        m_pushButtonDelete = new QPushButton( i18n( "&Delete" ), this, "m_pushButtonDelete" );
        gridLayout->addWidget( m_pushButtonDelete, 3, 2 );
        m_pushButtonDelete->setIconSet( QIconSet( SmallIcon( "delete" ) ) );

        m_pushButtonOpen = new QPushButton( i18n( "Op&en" ), this, "m_pushButtonOpen" );
        gridLayout->addWidget( m_pushButtonOpen, 4, 2 );
        m_pushButtonOpen->setIconSet( QIconSet( SmallIcon( "fileopen" ) ) );

        spacer = new QSpacerItem( 20, 110, QSizePolicy::Minimum, QSizePolicy::Expanding );
        gridLayout->addItem( spacer, 5, 2 );

        m_pushButtonOpen->setEnabled( FALSE );
        m_pushButtonAdd->setEnabled( FALSE );
        m_pushButtonDelete->setEnabled( FALSE );

        connect( m_listViewFields, SIGNAL( clicked( QListViewItem*, const QPoint&, int ) ), this, SLOT( fieldExecute( QListViewItem* ) ) );
        connect( m_lineEditKey, SIGNAL( textChanged( const QString& ) ), this, SLOT( updateGUI() ) );
        connect( m_pushButtonAdd, SIGNAL( clicked( ) ), this, SLOT( addClicked( ) ) );
        connect( m_pushButtonDelete, SIGNAL( clicked( ) ), this, SLOT( deleteClicked( ) ) );
        connect( m_pushButtonOpen, SIGNAL( clicked() ), this, SLOT( openClicked() ) );
    }

    void EntryWidgetOther::updateGUI()
    {
        QString text = m_lineEditKey->text();
        QListViewItem * item = m_listViewFields->findItem( text, 0 ) ;
        bool contains = item != NULL;
        bool isUnknown = BibTeX::EntryField::fieldTypeFromString( text ) == BibTeX::EntryField::ftUnknown;

        if ( contains )
            m_listViewFields->setSelected( item, TRUE );
        else
            m_listViewFields->clearSelection();

        m_pushButtonDelete->setEnabled( !m_isReadOnly && contains );
        m_pushButtonAdd->setEnabled( !m_isReadOnly && !text.isEmpty() && isUnknown );
        m_pushButtonAdd->setText( contains ? i18n( "&Apply" ) : i18n( "&Add" ) );
        m_pushButtonAdd->setIconSet( QIconSet( SmallIcon( contains ? "apply" : "fileopen" ) ) );

        bool validURL = FALSE;
        if ( contains )
        {
            KURL url = Settings::locateFile( item->text( 1 ), m_bibtexfile->fileName, this );
            validURL = url.isValid();
            m_internalURL = url;
        }
        m_pushButtonOpen->setEnabled( validURL );
    }

    void EntryWidgetOther::fieldExecute( QListViewItem * item )
    {
        ValueListViewItem * vlvi = dynamic_cast<KBibTeX::ValueListViewItem*>( item );
        if ( vlvi != NULL )
        {
            m_lineEditKey->setText( vlvi->title() );
            m_fieldLineEditValue->setValue( vlvi->value() );
        }
    }

    void EntryWidgetOther::addClicked()
    {
        QString text = m_lineEditKey->text();
        QListViewItem * item = m_listViewFields->findItem( text, 0 ) ;
        ValueListViewItem * vlvi = item != NULL ? dynamic_cast<KBibTeX::ValueListViewItem*>( item ) : NULL;

        if ( vlvi != NULL )
            vlvi->setValue( m_fieldLineEditValue->value() );
        else
            new ValueListViewItem( text, m_fieldLineEditValue->value(), m_listViewFields );

        updateGUI();

        m_isModified = TRUE;
    }

    void EntryWidgetOther::deleteClicked()
    {
        QListViewItem * item = m_listViewFields->findItem( m_lineEditKey->text(), 0 );

        if ( item != NULL )
        {
            m_deletedFields.append( item->text( 0 ) );
            delete item;
            m_lineEditKey->setText( "" );
            m_fieldLineEditValue->setValue( new BibTeX::Value() );
            updateGUI();
        }

        m_isModified = TRUE;
    }

    void EntryWidgetOther::openClicked()
    {
        if ( m_internalURL.isValid() )
        {
            BibTeX::Value * value = m_fieldLineEditValue->value();
            new KRun( m_internalURL, this );
            delete value;
        }
        else
            m_fieldLineEditValue->setEnabled( FALSE );
    }

    ValueListViewItem::ValueListViewItem( const QString& title, BibTeX::Value *value, QListView *parent )
            : QListViewItem( parent ), m_title( title )
    {
        m_value = new BibTeX::Value( value );
        setTexts( m_title );
    }

    ValueListViewItem::~ValueListViewItem()
    {
        delete m_value;
    }

    const BibTeX::Value *ValueListViewItem::value()
    {
        return m_value;
    }

    QString ValueListViewItem::title()
    {
        return m_title;
    }

    void ValueListViewItem::setValue( BibTeX::Value *value )
    {
        if ( value != m_value )
        {
            if ( m_value != NULL )
                delete m_value;

            if ( value != NULL )
                m_value = new BibTeX::Value( value );
            else
                m_value = new BibTeX::Value();

        }
        setTexts( m_title );
    }

    void ValueListViewItem::setTitle( const QString &title )
    {
        m_title = title;
        setTexts( title );
    }

    void ValueListViewItem::setTexts( const QString& title )
    {
        setText( 0, title );
        if ( m_value != NULL )
            setText( 1, m_value->text() );
    }

}
#include "entrywidgetother.moc"
