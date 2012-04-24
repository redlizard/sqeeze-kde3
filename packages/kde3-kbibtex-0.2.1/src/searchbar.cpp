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
#include <qlineedit.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qlabel.h>

#include <kcombobox.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

#include <settings.h>
#include "searchbar.h"

namespace KBibTeX
{

    SearchBar::SearchBar( QWidget *parent, const char *name )
            : QWidget( parent, name )
    {
        m_timerInput = new QTimer( this );
        connect( m_timerInput, SIGNAL( timeout() ), SLOT( slotTimeout() ) );

        setupGUI();
    }

    SearchBar::~SearchBar()
    {
        delete m_timerInput;
    }

    void SearchBar::setFactory( KXMLGUIFactory *factory, KXMLGUIClient *client )
    {
        KPopupMenu * menu = static_cast<KPopupMenu*>( factory -> container( "popup_newelements", client ) );
        Settings * settings = Settings::self();
        m_pushButtonAddElement->setPopup( menu );
        m_comboboxFilter->setHistoryItems( settings->editing_FilterHistory );
    }

    void SearchBar::restoreState()
    {
        Settings * settings = Settings::self();
        if ( settings->editing_UseSpecialFont )
            m_comboboxFilter->setFont( settings->editing_SpecialFont );
        else
            m_comboboxFilter->setFont( KGlobalSettings::generalFont() );
    }

    void SearchBar::setSearch( const QString&text, BibTeX::Element::FilterType filterType, BibTeX::EntryField::FieldType fieldType )
    {
        m_comboboxFilter->setCurrentText( text );
        switch ( filterType )
        {
        case BibTeX::Element::ftExact: m_comboboxFilterType->setCurrentItem( 0 ); break;
        case BibTeX::Element::ftEveryWord: m_comboboxFilterType->setCurrentItem( 1 ); break;
        case BibTeX::Element::ftAnyWord: m_comboboxFilterType->setCurrentItem( 2 ); break;
        }
        m_comboboxRestrictTo->setCurrentItem(( int ) fieldType + 1 );
    }

    void SearchBar::setupGUI()
    {
        QBoxLayout * layout = new QHBoxLayout( this, 3 /* KDialog::marginHint()*/, KDialog::spacingHint() );
        KIconLoader iconLoader = KIconLoader( "kbibtex" );

        m_pushButtonAddElement = new KPushButton( this );
        m_pushButtonAddElement->setIconSet( QIconSet( BarIcon( "add" ) ) );
        layout->addWidget( m_pushButtonAddElement );
        QToolTip::add( m_pushButtonAddElement, i18n( "Add a new BibTeX entry, comment or macro to this file" ) );

        m_pushButtonSearchOnlineDatabases = new KPushButton( this );
        m_pushButtonSearchOnlineDatabases->setIconSet( QIconSet( BarIcon( "network" ) ) );
        layout->addWidget( m_pushButtonSearchOnlineDatabases );
        QToolTip::add( m_pushButtonSearchOnlineDatabases, i18n( "Add a new BibTeX entry from an online database" ) );
        connect( m_pushButtonSearchOnlineDatabases, SIGNAL( clicked() ), this, SIGNAL( onlineSearch() ) );

        layout->insertSpacing( 2, KDialog::spacingHint() );

        m_pushButtonClearSearchText = new KPushButton( this );
        m_pushButtonClearSearchText->setIconSet( QIconSet( BarIcon( "locationbar_erase" ) ) );
        layout->addWidget( m_pushButtonClearSearchText );
        QToolTip::add( m_pushButtonClearSearchText, i18n( "Erase current search pattern" ) );
        m_pushButtonClearSearchText->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

        QLabel *label = new QLabel( i18n( "&Search:" ), this );
        layout->addWidget( label );

        m_comboboxFilter = new KHistoryCombo( TRUE, this, "search_combobox" );
        layout->addWidget( m_comboboxFilter );
        label->setBuddy( m_comboboxFilter );
        m_comboboxFilter->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
        m_comboboxFilter->setMaxCount( 256 );

        m_comboboxFilterType = new KComboBox( FALSE, this );
        m_comboboxFilterType->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
        layout->addWidget( m_comboboxFilterType );

        m_comboboxFilterType->insertItem( i18n( "Exact" ) );
        m_comboboxFilterType->insertItem( i18n( "Every word" ) );
        m_comboboxFilterType->insertItem( i18n( "Any word" ) );
        m_comboboxFilterType->setCurrentItem( 1 );

        label = new QLabel( i18n( "Restrict to:" ), this );
        layout->addWidget( label );
        m_comboboxRestrictTo = new KComboBox( FALSE, this );
        m_comboboxRestrictTo->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
        layout->addWidget( m_comboboxRestrictTo );
        label->setBuddy( m_comboboxRestrictTo );

        m_comboboxRestrictTo->insertItem( i18n( "All fields" ) );
        for ( int i = ( int ) BibTeX::EntryField::ftAbstract; i <= ( int ) BibTeX::EntryField::ftYear; i++ )
        {
            BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType ) i;
            m_comboboxRestrictTo->insertItem( Settings::fieldTypeToI18NString( fieldType ) );
        }

        connect( m_comboboxFilter->lineEdit(), SIGNAL( textChanged( const QString & ) ), this, SLOT( slotKeyPressed() ) );
        connect( m_comboboxFilter, SIGNAL( activated( const QString& ) ), m_comboboxFilter, SLOT( addToHistory( const QString& ) ) );
        connect( m_pushButtonClearSearchText, SIGNAL( clicked() ), this, SLOT( slotClear() ) );
        connect( m_comboboxFilterType, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotAnnounceDoSearch() ) );
        connect( m_comboboxFilter, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotAnnounceDoSearch() ) );
        connect( m_comboboxFilter->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( slotAnnounceDoSearch() ) );
        connect( m_comboboxFilterType, SIGNAL( activated( int ) ), this, SLOT( slotTimeout() ) );
        connect( m_comboboxRestrictTo, SIGNAL( activated( int ) ), this, SLOT( slotTimeout() ) );

        setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
    }

    void SearchBar::slotClear()
    {
        m_comboboxFilter->lineEdit() ->clear();
        m_comboboxRestrictTo->setCurrentItem( 0 );
        m_comboboxFilterType->setCurrentItem( 1 );
    }

    void SearchBar::slotTimeout()
    {
        BibTeX::EntryField::FieldType fieldType = BibTeX::EntryField::ftUnknown;
        if ( m_comboboxRestrictTo->currentItem() > 0 )
            fieldType = ( BibTeX::EntryField::FieldType )( m_comboboxRestrictTo->currentItem() - 1 + ( int ) BibTeX::EntryField::ftAbstract );

        Settings * settings = Settings::self();
        settings->editing_FilterHistory = m_comboboxFilter->historyItems();

        BibTeX::Element::FilterType filterType = BibTeX::Element::ftExact;
        if ( m_comboboxFilterType->currentItem() == 1 ) filterType = BibTeX::Element::ftEveryWord;
        else if ( m_comboboxFilterType->currentItem() == 2 ) filterType = BibTeX::Element::ftAnyWord;

        emit doSearch( m_comboboxFilter->currentText(), filterType, fieldType );
    }

    void SearchBar::slotAnnounceDoSearch()
    {
        if ( m_timerInput->isActive() )
            m_timerInput->stop();

        m_timerInput->start( 0, true );
    }

    void SearchBar::slotKeyPressed()
    {
        Settings * settings = Settings::self();
        if ( settings->editing_SearchBarClearField )
            m_comboboxRestrictTo->setCurrentItem( 0 );
    }

}
#include "searchbar.moc"
