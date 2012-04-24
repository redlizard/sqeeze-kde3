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
#include <qheader.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <klistview.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <file.h>
#include <entryfield.h>
#include <value.h>
#include <entry.h>
#include <settings.h>
#include "settingskeyword.h"

namespace KBibTeX
{

    SettingsKeyword::SettingsKeyword( QWidget *parent, const char *name ) : QWidget( parent, name ), m_keywords( NULL )
    {
        setupGUI();
    }

    SettingsKeyword::~SettingsKeyword()
    {
        // nothing
    }

    void SettingsKeyword::applyData()
    {
        Settings * settings = Settings::self();
        settings->keyword_GlobalList.clear();
        for ( QListViewItemIterator it = QListViewItemIterator( m_listKeywords ); it.current() != NULL; ++it )
            settings->keyword_GlobalList.append( it.current() ->text( 0 ) );
        settings->keyword_GlobalList.sort();
    }

    void SettingsKeyword::readData()
    {
        Settings * settings = Settings::self();
        m_listKeywords->clear();
        for ( QStringList::Iterator it = settings->keyword_GlobalList.begin(); it != settings->keyword_GlobalList.end(); ++it )
        {
            KListViewItem *item = new KListViewItem( m_listKeywords, *it );
            item->setPixmap( 0, SmallIcon( "package" ) );
        }

//         Settings * settings = Settings::self();
        BibTeX::File *bibtexFile = settings->currentBibTeXFile;
//         m_keywordsFromFile.clear();
        if ( bibtexFile != NULL )
//         {
            m_keywordsFromFile = bibtexFile->getAllValuesAsStringList( BibTeX::EntryField::ftKeywords );

        m_buttonImportKeywords->setEnabled( m_keywordsFromFile.size() > 0 );
    }

    void SettingsKeyword::slotNewKeyword()
    {
        KListViewItem * item = new KListViewItem( m_listKeywords, i18n( "New Keyword" ) );
        item->setPixmap( 0, SmallIcon( "package" ) );
        m_listKeywords->setSelected( item, TRUE );
        QTimer::singleShot( 100, this, SLOT( slotEditKeyword() ) );
    }

    void SettingsKeyword::slotEditKeyword()
    {
        KListViewItem * item = static_cast<KListViewItem*>( m_listKeywords->selectedItem() );
        if ( item != NULL )
        {
            m_listKeywords->rename( item, 0 );
            emit configChanged();
        }
        updateGUI();
    }

    void SettingsKeyword::slotDeleteKeyword()
    {
        KListViewItem * item = static_cast<KListViewItem*>( m_listKeywords->selectedItem() );
        if ( item != NULL )
        {
            delete item;
            emit configChanged();
        }
        updateGUI();
    }

    void SettingsKeyword::slotImportKeywords()
    {
        int numImported = m_keywordsFromFile.size();
        for ( QListViewItemIterator it = QListViewItemIterator( m_listKeywords ); it.current() != NULL; ++it )
        {
            QString text = it.current() ->text( 0 );
            if ( !m_keywordsFromFile.contains( text ) )
                m_keywordsFromFile.append( text );
            else
                numImported--;
        }

        m_keywordsFromFile.sort();
        m_listKeywords->clear();
        for ( QStringList::Iterator it = m_keywordsFromFile.begin(); it != m_keywordsFromFile.end(); ++it )
        {
            KListViewItem *item = new KListViewItem( m_listKeywords, *it );
            item->setPixmap( 0, SmallIcon( "package" ) );
        }

        KMessageBox::information( this, i18n( "1 keyword has been imported.", "%n keywords have been imported.", numImported ), i18n( "Keywords imported" ) );

        m_buttonImportKeywords->setEnabled( FALSE );
    }

    void SettingsKeyword::updateGUI()
    {
        bool selected = m_listKeywords->selectedItem() != NULL;
        m_buttonEditKeyword->setEnabled( selected );
        m_buttonDeleteKeyword->setEnabled( selected );
    }

    void SettingsKeyword::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 5, 2, 0, KDialog::spacingHint(), "gridLayout" );

        m_listKeywords = new KListView( this );
        m_listKeywords->addColumn( i18n( "Keywords" ) );
        m_listKeywords->header()->setClickEnabled( FALSE );
        gridLayout->addMultiCellWidget( m_listKeywords, 0, 4, 0, 0 );

        m_buttonNewKeyword = new QPushButton( i18n( "keyword", "New" ), this );
        m_buttonNewKeyword->setIconSet( QIconSet( SmallIcon( "add" ) ) );
        gridLayout->addWidget( m_buttonNewKeyword, 0, 1 );
        m_buttonEditKeyword = new QPushButton( i18n( "keyword", "Edit" ), this );
        m_buttonEditKeyword->setIconSet( QIconSet( SmallIcon( "edit" ) ) );
        gridLayout->addWidget( m_buttonEditKeyword, 1, 1 );
        m_buttonDeleteKeyword = new QPushButton( i18n( "keyword", "Delete" ), this );
        m_buttonDeleteKeyword->setIconSet( QIconSet( SmallIcon( "editdelete" ) ) );
        gridLayout->addWidget( m_buttonDeleteKeyword, 2, 1 );

        m_buttonImportKeywords = new QPushButton( i18n( "keyword", "Import" ), this );
        m_buttonImportKeywords->setIconSet( QIconSet( SmallIcon( "openfile" ) ) );
        QToolTip::add( m_buttonImportKeywords, "Import all keywords from the current BibTeX file" );
        gridLayout->addWidget( m_buttonImportKeywords, 4, 1 );
        connect( m_buttonImportKeywords, SIGNAL( clicked() ), this, SLOT( slotImportKeywords() ) );

        connect( m_buttonNewKeyword, SIGNAL( clicked() ), this, SLOT( slotNewKeyword() ) );
        connect( m_buttonEditKeyword, SIGNAL( clicked() ), this, SLOT( slotEditKeyword() ) );
        connect( m_buttonDeleteKeyword, SIGNAL( clicked() ), this, SLOT( slotDeleteKeyword() ) );
        connect( m_listKeywords, SIGNAL( selectionChanged() ), this, SLOT( updateGUI() ) );
        connect( m_listKeywords, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( updateGUI() ) );

        updateGUI();
    }

}
#include "settingskeyword.moc"
