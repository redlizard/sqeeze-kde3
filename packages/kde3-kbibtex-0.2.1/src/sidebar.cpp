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
#include <qcombobox.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qstring.h>
#include <qtoolbutton.h>

#include <kdebug.h>
#include <kpopupmenu.h>
#include <klistview.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>

#include <settings.h>
#include <entry.h>
#include "sidebar.h"

namespace KBibTeX
{

    const BibTeX::EntryField::FieldType SideBar::importantFields[ 6 ] =
    {
        BibTeX::EntryField::ftAuthor, BibTeX::EntryField::ftBookTitle, BibTeX::EntryField::ftJournal, BibTeX::EntryField::ftTitle, BibTeX::EntryField::ftYear, BibTeX::EntryField::ftKeywords
    };

    SideBar::SideBar( bool isReadOnly, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_bibtexFile( NULL ), m_isReadOnly( isReadOnly ), m_popupMenu( NULL )
    {
        setupGUI();
    }

    SideBar::~SideBar()
    {
        // nothing
    }

    void SideBar::restoreState()
    {
        Settings * settings = Settings::self();
        if ( settings->editing_UseSpecialFont )
            m_listAvailableItems->setFont( settings->editing_SpecialFont );
        else
            m_listAvailableItems->setFont( KGlobalSettings::generalFont() );
    }

    void SideBar::refreshLists( BibTeX::File *bibtexFile )
    {
        if ( bibtexFile != NULL )
            m_bibtexFile = bibtexFile;

        QApplication::setOverrideCursor( Qt::waitCursor );
        setEnabled( FALSE );

        BibTeX::EntryField::FieldType fieldType = m_buttonToggleShowAll->isOn() ? ( BibTeX::EntryField::FieldType )( m_listTypeList->currentItem() + ( int ) BibTeX::EntryField::ftAbstract ) : importantFields[ m_listTypeList->currentItem()];

        m_listAvailableItems->clear();
        if ( m_bibtexFile != NULL )
        {
            QMap<QString, int> allValues = m_bibtexFile->getAllValuesAsStringListWithCount( fieldType );
            for ( QMap<QString, int>::ConstIterator it = allValues.constBegin(); it != allValues.constEnd();++it )
            {
                QString text = it.key();
                if ( !text.startsWith( "other" ) )
                    new SideBarListViewItem( m_listAvailableItems, QString::number( it.data() ), text );
            }
        }

        setEnabled( TRUE );
        QApplication::restoreOverrideCursor();
    }

    void SideBar::resizeEvent( QResizeEvent *event )
    {
        QSize widgetSize = size();
        setEnabled( widgetSize.width() > 3 && widgetSize.height() > 3 );
        QWidget::resizeEvent( event );
    }

    void SideBar::setupGUI()
    {
        QGridLayout * layout = new QGridLayout( this, 2, 2, 0, KDialog::spacingHint() );
        layout->setColStretch( 0, 0 );
        layout->setColStretch( 1, 10 );

        m_buttonToggleShowAll = new QToolButton( this );
        layout->addWidget( m_buttonToggleShowAll, 0, 0 );
        QIconSet showAllPixmap = KGlobal::iconLoader() ->loadIconSet( "taskbar", KIcon::Small );
        m_buttonToggleShowAll->setIconSet( showAllPixmap );
        m_buttonToggleShowAll->setToggleButton( TRUE );
        QToolTip::add( m_buttonToggleShowAll, i18n( "Toggle between showing all fields or only important fields" ) );

        m_listTypeList = new QComboBox( FALSE, this );
        layout->addWidget( m_listTypeList, 0, 1 );
        QToolTip::add( m_listTypeList, i18n( "Filter this sidebar for a given field" ) );

        m_listAvailableItems = new KListView( this );
        m_listAvailableItems->addColumn( i18n( "#" ) );
        m_listAvailableItems->addColumn( i18n( "Items" ) );
        m_listAvailableItems->setAllColumnsShowFocus( TRUE );
        m_listAvailableItems->setSorting( 1 );
        layout->addMultiCellWidget( m_listAvailableItems, 1, 1, 0, 1 );

        m_popupMenu = new KPopupMenu( m_listAvailableItems );
        m_popupMenu->insertItem( i18n( "Rename all occurrences" ), this, SLOT( startRenaming() ) );

        connect( m_listAvailableItems, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( prepareSearch( QListViewItem * ) ) );
        connect( m_listTypeList, SIGNAL( activated( int ) ), this, SLOT( refreshLists() ) );
        connect( m_buttonToggleShowAll, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowAll( bool ) ) );
        connect( m_listAvailableItems, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ), this, SLOT( showContextMenu( QListViewItem*, const QPoint& ) ) );
        connect( m_listAvailableItems, SIGNAL( itemRenamed( QListViewItem*, int, const QString& ) ), this, SLOT( endRenaming( QListViewItem*, int, const QString& ) ) );

        toggleShowAll( FALSE );
    }

    void SideBar::setReadOnly( bool isReadOnly )
    {
        m_isReadOnly = isReadOnly;
    }


    void SideBar::prepareSearch( QListViewItem *item )
    {
        if ( item != NULL )
        {
            BibTeX::EntryField::FieldType fieldType = m_buttonToggleShowAll->isOn() ? ( BibTeX::EntryField::FieldType )( m_listTypeList->currentItem() + ( int ) BibTeX::EntryField::ftAbstract ) : importantFields[ m_listTypeList->currentItem()];
            emit selected( item->text( 1 ), BibTeX::Element::ftExact, fieldType );
        }
    }

    void SideBar::toggleShowAll( bool showAll )
    {
        m_listTypeList->clear();

        if ( showAll )
        {
            for ( int i = ( int ) BibTeX::EntryField::ftAbstract; i <= ( int ) BibTeX::EntryField::ftYear; i++ )
            {
                BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType ) i;
                m_listTypeList->insertItem( Settings::fieldTypeToI18NString( fieldType ) );
            }
            m_listTypeList->setCurrentItem(( int ) BibTeX::EntryField::ftAuthor );
        }
        else
        {
            for ( int i = 0; i < 6;i++ )
                m_listTypeList->insertItem( Settings::fieldTypeToI18NString( importantFields[ i ] ) );
            m_listTypeList->setCurrentItem( 0 );
        }

        refreshLists();
    }

    void SideBar::showContextMenu( QListViewItem * item, const QPoint & pos )
    {
        if ( item != NULL && !m_isReadOnly && m_popupMenu != NULL )
            m_popupMenu->exec( pos );
    }

    void SideBar::startRenaming()
    {
        QListViewItem * item = m_listAvailableItems->selectedItem();
        if ( item != NULL )
        {
            m_oldText = item->text( 1 );
            item->setRenameEnabled( 1, TRUE );
            item->startRename( 1 );
        }
    }

    void SideBar::endRenaming( QListViewItem * item, int , const QString & text )
    {
        item->setRenameEnabled( 1, FALSE );
        BibTeX::EntryField::FieldType fieldType = m_buttonToggleShowAll->isOn() ? ( BibTeX::EntryField::FieldType )( m_listTypeList->currentItem() + ( int ) BibTeX::EntryField::ftAbstract ) : importantFields[ m_listTypeList->currentItem()];
        m_bibtexFile->replaceValue( m_oldText, text, fieldType );

        prepareSearch( item );

        emit valueRenamed();
    }

    SideBarListViewItem::SideBarListViewItem( QListView *parent, QString label1, QString label2 ) : QListViewItem( parent, label1, label2 )
    {
// nothing
    }

    int SideBarListViewItem::compare( QListViewItem* item, int column, bool ascending ) const
    {
        if ( column == 0 )
        {
            bool ok = FALSE;
            int ownValue = text( 0 ).toInt( &ok );
            if ( ok )
            {
                int otherValue = item->text( 0 ).toInt( &ok );
                if ( ok )
                {
                    return ownValue < otherValue ? -1 : ( ownValue > otherValue ? 1 : 0 );
                }
            }
        }

        return QListViewItem::compare( item, column, ascending );
    }
}
#include "sidebar.moc"
