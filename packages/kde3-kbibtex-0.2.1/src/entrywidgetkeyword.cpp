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
#include <qregexp.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qlabel.h>

#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <kguiitem.h>
#include <kdebug.h>

#include <settings.h>
#include "entrywidgetkeyword.h"

namespace KBibTeX
{
    KeywordListViewItem::KeywordListViewItem( KListView * parent, const QString & text, bool global ) : QCheckListItem( parent, text, QCheckListItem::CheckBox )
    {
        setGlobal( global );
    }

    KeywordListViewItem::~KeywordListViewItem()
    {
// nothing
    }

    void KeywordListViewItem::setGlobal( bool global )
    {
        m_isGlobal = global;
        if ( m_isGlobal )
        {
            setText( 1, i18n( "Global" ) );
            setPixmap( 1, SmallIcon( "package" ) );
        }
        else
        {
            setText( 1, i18n( "In this file only" ) );
            setPixmap( 1, SmallIcon( "editcopy" ) );
        }
    }

    EntryWidgetKeyword::EntryWidgetKeyword( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name ) : EntryWidgetTab( bibtexfile, isReadOnly, parent, name ), m_bibtexfile( bibtexfile ), m_isModified( FALSE ), m_numKeywords( 0 )
    {
        setupGUI();
    }

    EntryWidgetKeyword::~EntryWidgetKeyword()
    {
        // nothing
    }

    bool EntryWidgetKeyword::isModified()
    {
        return m_isModified;
    }

    void EntryWidgetKeyword::updateGUI( BibTeX::Entry::EntryType /*entryType*/, bool /*enableAll*/ )
    {
        // nothing
    }

    void EntryWidgetKeyword::apply( BibTeX::Entry *entry )
    {
        readListView();

        /**
         * Update entry
        */
        if ( m_usedKeywords.empty() )
            entry->deleteField( BibTeX::EntryField::ftKeywords );
        else
        {
            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftKeywords );
            BibTeX::Value *value = NULL;
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( BibTeX::EntryField::ftKeywords );
                entry->addField( field );
            }
            value = field->value();
            value->items.clear();
            BibTeX::KeywordContainer *keywordContainer = new BibTeX::KeywordContainer( m_usedKeywords );
            value->items.append( keywordContainer );
        }

        /**
         * Update global keyword list
        */
        KBibTeX::Settings * settings = KBibTeX::Settings::self();
        settings->keyword_GlobalList = m_globalKeywords;
    }

    void EntryWidgetKeyword::reset( BibTeX::Entry *entry )
    {
        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftKeywords );
        BibTeX::KeywordContainer *keywordContainer = NULL;
        if ( field != NULL && ( keywordContainer = dynamic_cast<BibTeX::KeywordContainer*>( field->value()->items.first() ) ) != NULL )
            for ( QValueList<BibTeX::Keyword*>::Iterator it = keywordContainer->keywords.begin(); it != keywordContainer->keywords.end(); ++it )
                m_usedKeywords.append(( *it )->text() );

        m_fileKeywords = m_bibtexfile->getAllValuesAsStringList( BibTeX::EntryField::ftKeywords );

        KBibTeX::Settings * settings = KBibTeX::Settings::self();
        m_globalKeywords = settings->keyword_GlobalList;

        m_availableKeywords = QStringList( m_globalKeywords );
        for ( QStringList::Iterator it = m_fileKeywords.begin(); it !=  m_fileKeywords.end(); ++it )
            if ( !m_availableKeywords.contains( *it ) )
                m_availableKeywords.append( *it );
        for ( QStringList::Iterator it = m_usedKeywords.begin(); it !=  m_usedKeywords.end(); ++it )
            if ( !m_availableKeywords.contains( *it ) )
                m_availableKeywords.append( *it );

        setListView();
    }

    void EntryWidgetKeyword::updateWarnings( BibTeX::Entry::EntryType /*entryType*/, QListView */*listViewWarnings*/ )
    {
        // nothing
    }

    void EntryWidgetKeyword::slotSelectionChanged()
    {
        bool hasCurrent = m_listviewKeywords->selectedItem() != NULL;

        m_buttonEdit->setEnabled( hasCurrent );
        m_buttonToggleGlobal->setEnabled( hasCurrent );
    }

    void EntryWidgetKeyword::slotKeywordRenamed( QListViewItem * item, const QString & text, int /*col*/ )
    {
        KeywordListViewItem *kwlvi = dynamic_cast<KeywordListViewItem*>( item );
        if ( text.isEmpty() )
        {
            item->setText( 0, m_beforeRenaming );
            kwlvi->setOn( FALSE );
        }
        else if ( text != m_beforeRenaming )
        {
            if ( m_availableKeywords.contains( text ) )
            {
                item->setText( 0, m_beforeRenaming );
                KMessageBox::error( this, QString( i18n( "The keyword '%1' does already exist in the list of keywords.\nThe old name has been restored." ) ).arg( text ), i18n( "Renaming keyword failed" ) );
            }
            else
            {
                m_availableKeywords.remove( m_beforeRenaming );
                m_availableKeywords.append( text );
                if ( kwlvi->isGlobal() )
                {
                    m_globalKeywords.remove( m_beforeRenaming );
                    m_globalKeywords.append( text );
                }
                else
                {
                    m_fileKeywords.remove( m_beforeRenaming );
                    m_fileKeywords.append( text );
                }
                QCheckListItem *checkedItem = dynamic_cast<QCheckListItem*>( item );
                if ( checkedItem != NULL )
                    checkedItem->setOn( TRUE );
            }
        }
    }

    void EntryWidgetKeyword::slotNewKeyword()
    {
        KeywordListViewItem * item = new KeywordListViewItem( m_listviewKeywords, QString( i18n( "May only contain ASCII characters, in case of doubt keep English form", "NewKeyword%1" ) ).arg( ++m_numKeywords ), FALSE );
        m_listviewKeywords->setSelected( item, TRUE );
        QTimer::singleShot( 100, this, SLOT( slotEditKeyword() ) );
    }

    void EntryWidgetKeyword::slotEditKeyword()
    {
        QListViewItem * item = m_listviewKeywords->selectedItem();
        if ( item != NULL )
        {
            m_beforeRenaming = item->text( 0 );
            m_listviewKeywords->rename( item, 0 );
        }
    }

    void EntryWidgetKeyword::slotToggleGlobal()
    {
        KeywordListViewItem *item = dynamic_cast<KeywordListViewItem*>( m_listviewKeywords->selectedItem() );
        if ( item != NULL )
        {
            bool isGlobal = item->isGlobal();
            if ( isGlobal )
                m_globalKeywords.remove( item->text( 0 ) );
            else
                m_globalKeywords.append( item->text( 0 ) );

            item->setGlobal( !isGlobal );
        }
    }

    void EntryWidgetKeyword::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 6, 2, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );
        gridLayout->setRowStretch( 4, 1 );

        m_listviewKeywords = new KListView( this );
        m_listviewKeywords->setEnabled( !m_isReadOnly );
        m_listviewKeywords->addColumn( i18n( "Keyword" ) );
        m_listviewKeywords->addColumn( i18n( "Origin" ) );
        gridLayout->addMultiCellWidget( m_listviewKeywords, 0, 4, 0, 0 );
        m_listviewKeywords->setAllColumnsShowFocus( TRUE );
        connect( m_listviewKeywords, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotSelectionChanged() ) );
        connect( m_listviewKeywords, SIGNAL( clicked( QListViewItem * ) ), this, SLOT( slotSelectionChanged() ) );
        connect( m_listviewKeywords, SIGNAL( itemRenamed( QListViewItem*, const QString&, int ) ), this, SLOT( slotKeywordRenamed( QListViewItem*, const QString&, int ) ) );

        m_buttonNew = new QPushButton( i18n( "keyword", "New" ), this );
        m_buttonNew->setEnabled( !m_isReadOnly );
        m_buttonNew->setIconSet( QIconSet( SmallIcon( "add" ) ) );
        QToolTip::add( m_buttonNew, i18n( "Add a new keyword to the list" ) );
        gridLayout->addWidget( m_buttonNew, 0, 1 );
        connect( m_buttonNew, SIGNAL( clicked() ), this, SLOT( slotNewKeyword() ) );

        m_buttonEdit = new QPushButton( i18n( "keyword", "Edit" ), this );
        m_buttonEdit->setIconSet( QIconSet( SmallIcon( "edit" ) ) );
        QToolTip::add( m_buttonEdit, i18n( "Edit the selected keyword" ) );
        gridLayout->addWidget( m_buttonEdit, 1, 1 );
        m_buttonEdit->setEnabled( FALSE );
        connect( m_buttonEdit, SIGNAL( clicked() ), this, SLOT( slotEditKeyword() ) );

        m_buttonToggleGlobal = new QPushButton( i18n( "keyword", "Toggle &global" ), this );
        m_buttonToggleGlobal->setIconSet( QIconSet( SmallIcon( "package" ) ) );
        QToolTip::add( m_buttonToggleGlobal, i18n( "Add or remove the selected keyword to or from the global list" ) );
        gridLayout->addWidget( m_buttonToggleGlobal, 2, 1 );
        m_buttonToggleGlobal->setEnabled( FALSE );
        connect( m_buttonToggleGlobal, SIGNAL( clicked() ), this, SLOT( slotToggleGlobal() ) );

        QLabel *label = new QLabel( i18n( "There is no need to delete keywords. Simply uncheck unwanted keywords and make them non-global.\nGlobal keywords can also be edited in the settings dialog." ), this );
        label->setAlignment( QLabel::WordBreak | QLabel::AlignTop );
        gridLayout->addMultiCellWidget( label, 5, 5, 0, 1 );
    }

    void EntryWidgetKeyword::setListView()
    {
        m_availableKeywords.sort();
        m_listviewKeywords->clear();
        for ( QStringList::Iterator it = m_availableKeywords.begin(); it != m_availableKeywords.end(); ++it )
        {
            KeywordListViewItem *item = new KeywordListViewItem( m_listviewKeywords, *it,  m_globalKeywords.contains( *it ) );
            if ( m_usedKeywords.contains( *it ) )
                item->setOn( TRUE );
        }
    }

    void EntryWidgetKeyword::readListView()
    {
        m_usedKeywords.clear();
        for ( QListViewItemIterator it = QListViewItemIterator( m_listviewKeywords, QListViewItemIterator::Checked ); it.current() != NULL; ++it )
            m_usedKeywords.append(( *it ) ->text( 0 ) );
    }
}
#include "entrywidgetkeyword.moc"
