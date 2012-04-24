/***************************************************************************
 *   Copyright (C) 2004-2005 by Thomas Fischer                             *
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
#include <qcheckbox.h>
#include <qtimer.h>
#include <qvalidator.h>
#include <qheader.h>
#include <qlabel.h>

#include <kinputdialog.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kdialog.h>

#include <idsuggestions.h>
#include <idsuggestionswidget.h>
#include <settings.h>
#include <fileimporter.h>
#include <fileimporterbibtex.h>
#include "settingsidsuggestions.h"

namespace KBibTeX
{
    QString SettingsIdSuggestions::exampleBibTeXEntry = "@Article{ dijkstra1983terminationdetect,\nauthor = {Edsger W. Dijkstra and W. H. J. Feijen and A. J. M. {van Gasteren}},\ntitle = {{Derivation of a Termination Detection Algorithm for Distributed Computations}},\njournal = {Information Processing Letters},\nvolume = 16,\nnumber = 5,\npages = {217--219},\nmonth = jun,\nyear = 1983\n}";

    IdSuggestionsListViewItem::IdSuggestionsListViewItem( KListView *list, const QString& plainText, BibTeX::Entry *example ): KListViewItem( list ), m_original( plainText ), m_example( example )
    {
        KListViewItem::setText( 0, parse( plainText ) );
        KListViewItem::setMultiLinesEnabled( TRUE );
    }

    IdSuggestionsListViewItem::IdSuggestionsListViewItem( KListView *list, KListViewItem *prev, const QString& plainText, BibTeX::Entry *example ): KListViewItem( list, prev ), m_original( plainText ), m_example( example )
    {
        KListViewItem::setText( 0, parse( plainText ) );
        KListViewItem::setMultiLinesEnabled( TRUE );
    }

    void IdSuggestionsListViewItem::setText( int col, const QString& text )
    {
        if ( col == 0 )
            KListViewItem::setText( 0, parse( text ) );
        else
            KListViewItem::setText( col, text );
    }

    int IdSuggestionsListViewItem::width( const QFontMetrics & fm, const QListView *, int c ) const
    {
        int max = 100;
        QStringList lines = QStringList::split( '\n', text( c ) );
        for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it )
        {
            int w = fm.width( *it );
            if ( w > max ) max = w;
        }

        return max + 36;
    }

    QString IdSuggestionsListViewItem::originalText() const
    {
        return m_original;
    }

    QString IdSuggestionsListViewItem::parse( const QString&plainText )
    {
        m_original = plainText;
        QString result = IdSuggestions::formatStrToHuman( plainText );
        if ( m_example != NULL )
        {
            result.append( QString( i18n( "\nExample: %1" ) ).arg( IdSuggestions::formatId( m_example, plainText ) ) );
        }
        return result;
    }

    SettingsIdSuggestions::SettingsIdSuggestions( QWidget *parent, const char *name )
            : QWidget( parent, name )
    {
        m_validator = new QRegExpValidator( QRegExp( "[^\\s]+" ), this );
        setupGUI();

        BibTeX::FileImporter *importer = new BibTeX::FileImporterBibTeX( false );
        BibTeX::File *file = importer->load( exampleBibTeXEntry );
        m_example = new BibTeX::Entry( dynamic_cast<BibTeX::Entry*>( *( file->begin() ) ) );
        delete file;
        delete importer;
    }

    SettingsIdSuggestions::~SettingsIdSuggestions()
    {
        delete m_example;
        delete m_validator;
    }

    void SettingsIdSuggestions::applyData()
    {
        Settings * settings = Settings::self();

        settings->idSuggestions_formatStrList.clear();
        settings->idSuggestions_default = -1;
        settings->idSuggestions_forceDefault = m_checkBoxForceDefault->isChecked();
        int i = 0;
        for ( QListViewItemIterator it( m_listIdSuggestions ); it.current(); ++i, ++it )
        {
            IdSuggestionsListViewItem *item = dynamic_cast<IdSuggestionsListViewItem*>( *it );
            settings->idSuggestions_formatStrList.append( item->originalText() );
            if ( item == m_defaultSuggestionItem )
                settings->idSuggestions_default = i;
        }
    }

    void SettingsIdSuggestions::readData()
    {
        Settings * settings = Settings::self();

        IdSuggestionsListViewItem *prev = NULL;
        m_listIdSuggestions->clear();
        m_defaultSuggestionItem = NULL;
        m_checkBoxForceDefault->setChecked( settings->idSuggestions_forceDefault );
        m_checkBoxForceDefault->setEnabled( settings->idSuggestions_default >= 0 );
        int i = 0;
        for ( QStringList::ConstIterator it = settings->idSuggestions_formatStrList.begin(); it != settings->idSuggestions_formatStrList.end(); ++i, ++it )
        {
            prev = new IdSuggestionsListViewItem( m_listIdSuggestions, prev, *it, m_example );
            prev->setPixmap( 0, SmallIcon( "filter" ) );
            if ( i == settings->idSuggestions_default )
                m_defaultSuggestionItem = prev;
        }

        if ( m_defaultSuggestionItem != NULL )
            m_defaultSuggestionItem->setPixmap( 0, SmallIcon( "favorites" ) );

        updateGUI();
    }

    void SettingsIdSuggestions::slotConfigChanged()
    {
        emit configChanged();
    }

    void SettingsIdSuggestions::slotNewIdSuggestion()
    {
        IdSuggestionsListViewItem * item = new IdSuggestionsListViewItem( m_listIdSuggestions, "a|Y|T", m_example );
        item->setPixmap( 0, SmallIcon( "filter" ) );
        m_listIdSuggestions->setSelected( item, TRUE );
        QTimer::singleShot( 100, this, SLOT( slotEditIdSuggestion() ) );
    }

    void SettingsIdSuggestions::slotEditIdSuggestion()
    {
        IdSuggestionsListViewItem * item = static_cast<IdSuggestionsListViewItem*>( m_listIdSuggestions->selectedItem() );
        if ( item != NULL )
        {
            QString formatStr = item->originalText();
            if ( IdSuggestionsWidget::execute( formatStr, this ) == QDialog::Accepted )
            {
                item->setText( 0, formatStr );
                emit configChanged();
            }
        }
        updateGUI();
    }

    void SettingsIdSuggestions::slotDeleteIdSuggestion()
    {
        IdSuggestionsListViewItem * item = static_cast<IdSuggestionsListViewItem*>( m_listIdSuggestions->selectedItem() );
        if ( item != NULL )
        {
            if ( m_defaultSuggestionItem == item )
                m_defaultSuggestionItem = NULL;
            m_checkBoxForceDefault->setEnabled( m_defaultSuggestionItem != NULL );

            delete item;
            emit configChanged();
        }
        updateGUI();
    }

    void SettingsIdSuggestions::slotMoveUpIdSuggestion()
    {
        IdSuggestionsListViewItem * item = dynamic_cast<IdSuggestionsListViewItem*>( m_listIdSuggestions->selectedItem() );
        if ( item != NULL && item -> itemAbove() != NULL )
        {
            IdSuggestionsListViewItem *itemAbove = dynamic_cast<IdSuggestionsListViewItem*>( item->itemAbove() );
            QString text = item->originalText();
            item->setText( 0, itemAbove->originalText( ) );
            itemAbove->setText( 0, text );
            m_listIdSuggestions->setCurrentItem( itemAbove );
            m_listIdSuggestions->ensureItemVisible( itemAbove );

            if ( m_defaultSuggestionItem == itemAbove )
            {
                itemAbove->setPixmap( 0, SmallIcon( "filter" ) );
                m_defaultSuggestionItem = item;
            }
            else if ( m_defaultSuggestionItem == item )
            {
                item->setPixmap( 0, SmallIcon( "filter" ) );
                m_defaultSuggestionItem = itemAbove;
            }
            m_defaultSuggestionItem->setPixmap( 0, SmallIcon( "favorites" ) );
        }
    }

    void SettingsIdSuggestions::slotMoveDownIdSuggestion()
    {
        IdSuggestionsListViewItem * item = dynamic_cast<IdSuggestionsListViewItem*>( m_listIdSuggestions->selectedItem() );
        if ( item != NULL && item -> itemBelow() != NULL )
        {
            IdSuggestionsListViewItem *itemBelow = dynamic_cast<IdSuggestionsListViewItem*>( item->itemBelow() );
            QString text = item->originalText();
            item->setText( 0, itemBelow->originalText( ) );
            itemBelow->setText( 0, text );
            m_listIdSuggestions->setCurrentItem( itemBelow );
            m_listIdSuggestions->ensureItemVisible( itemBelow );

            if ( m_defaultSuggestionItem == itemBelow )
            {
                itemBelow->setPixmap( 0, SmallIcon( "filter" ) );
                m_defaultSuggestionItem = item;
            }
            else if ( m_defaultSuggestionItem == item )
            {
                item->setPixmap( 0, SmallIcon( "filter" ) );
                m_defaultSuggestionItem = itemBelow;
            }
            m_defaultSuggestionItem->setPixmap( 0, SmallIcon( "favorites" ) );
        }
    }

    void SettingsIdSuggestions::slotToggleDefault()
    {
        if ( m_defaultSuggestionItem != NULL )
            m_defaultSuggestionItem->setPixmap( 0, SmallIcon( "filter" ) );

        QListViewItem *item = m_listIdSuggestions->selectedItem();
        if ( item == m_defaultSuggestionItem )
            m_defaultSuggestionItem = NULL;
        else
        {
            m_defaultSuggestionItem = item;
            m_defaultSuggestionItem->setPixmap( 0, SmallIcon( "favorites" ) );
        }
        m_checkBoxForceDefault->setEnabled( m_defaultSuggestionItem != NULL );
    }

    void SettingsIdSuggestions::updateGUI()
    {
        QListViewItem *item = m_listIdSuggestions->selectedItem();
        bool selected = item != NULL;
        m_buttonEditIdSuggestion->setEnabled( selected );
        m_buttonDeleteIdSuggestion->setEnabled( selected );
        m_buttonMoveDownIdSuggestion->setEnabled( selected && item->itemBelow() != NULL );
        m_buttonMoveUpIdSuggestion->setEnabled( selected && item->itemAbove() != NULL );
        m_buttonToggleDefault->setEnabled( selected );
    }

    void SettingsIdSuggestions::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 8, 2, 0, KDialog::spacingHint(), "gridLayout" );
        gridLayout->setRowStretch( 6, 1 );
        gridLayout->setColStretch( 0, 1 );

        m_listIdSuggestions = new KListView( this );
        m_listIdSuggestions->setSorting( -1, FALSE );
        m_listIdSuggestions->addColumn( i18n( "Id Suggestions" ) );
        m_listIdSuggestions->header()->setClickEnabled( FALSE );
        gridLayout->addMultiCellWidget( m_listIdSuggestions, 0, 6, 0, 0 );
        connect( m_listIdSuggestions, SIGNAL( selectionChanged() ), this, SLOT( updateGUI() ) );
        connect( m_listIdSuggestions, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( updateGUI() ) );
        connect( m_listIdSuggestions, SIGNAL( doubleClicked( QListViewItem*, const QPoint &, int ) ), this, SLOT( slotEditIdSuggestion() ) );

        m_buttonNewIdSuggestion = new KPushButton( i18n( "id suggestion",  "New" ), this );
        m_buttonNewIdSuggestion->setIconSet( QIconSet( SmallIcon( "add" ) ) );
        gridLayout->addWidget( m_buttonNewIdSuggestion, 0, 1 );
        connect( m_buttonNewIdSuggestion, SIGNAL( clicked() ), this, SLOT( slotNewIdSuggestion() ) );
        m_buttonEditIdSuggestion = new KPushButton( i18n( "id suggestion", "Edit" ), this );
        m_buttonEditIdSuggestion->setIconSet( QIconSet( SmallIcon( "edit" ) ) );
        gridLayout->addWidget( m_buttonEditIdSuggestion, 1, 1 );
        connect( m_buttonEditIdSuggestion, SIGNAL( clicked() ), this, SLOT( slotEditIdSuggestion() ) );
        m_buttonDeleteIdSuggestion = new KPushButton( i18n( "id suggestion", "Delete" ), this );
        m_buttonDeleteIdSuggestion->setIconSet( QIconSet( SmallIcon( "editdelete" ) ) );
        gridLayout->addWidget( m_buttonDeleteIdSuggestion, 2, 1 );
        connect( m_buttonDeleteIdSuggestion, SIGNAL( clicked() ), this, SLOT( slotDeleteIdSuggestion() ) );
        m_buttonMoveUpIdSuggestion = new KPushButton( i18n( "id suggestion", "Up" ), this );
        m_buttonMoveUpIdSuggestion->setIconSet( QIconSet( SmallIcon( "up" ) ) );
        gridLayout->addWidget( m_buttonMoveUpIdSuggestion, 3, 1 );
        connect( m_buttonMoveUpIdSuggestion, SIGNAL( clicked() ), this, SLOT( slotMoveUpIdSuggestion() ) );
        m_buttonMoveDownIdSuggestion = new KPushButton( i18n( "id suggestion", "Down" ), this );
        m_buttonMoveDownIdSuggestion->setIconSet( QIconSet( SmallIcon( "down" ) ) );
        gridLayout->addWidget( m_buttonMoveDownIdSuggestion, 4, 1 );
        connect( m_buttonMoveDownIdSuggestion, SIGNAL( clicked() ), this, SLOT( slotMoveDownIdSuggestion() ) );
        m_buttonToggleDefault = new KPushButton( i18n( "Toogle default" ), this );
        m_buttonToggleDefault->setIconSet( QIconSet( SmallIcon( "favorites" ) ) );
        gridLayout->addWidget( m_buttonToggleDefault, 5, 1 );
        connect( m_buttonToggleDefault, SIGNAL( clicked() ), this, SLOT( slotToggleDefault() ) );

        m_checkBoxForceDefault = new QCheckBox( i18n( "Use default id suggestion when editing new entries" ), this );
        gridLayout->addMultiCellWidget( m_checkBoxForceDefault, 7, 7, 0, 1 );
    }
}

#include "settingsidsuggestions.moc"
