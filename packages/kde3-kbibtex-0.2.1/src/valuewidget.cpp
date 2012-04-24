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
#include <qlistview.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qheader.h>
#include <qpushbutton.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kiconloader.h>

#include <value.h>
#include <settings.h>
#include "valuewidget.h"

namespace KBibTeX
{

    QDialog::DialogCode ValueWidget::execute( const QString& title, BibTeX::EntryField::FieldType fieldType, BibTeX::Value *value, bool isReadOnly, QWidget *parent, const char *name )
    {
        KDialogBase * dlg = new KDialogBase( parent, name, TRUE, QString( i18n( "Edit field '%1'" ) ).arg( title ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, TRUE );

        ValueWidget* valueWidget = new ValueWidget( fieldType, value, isReadOnly, dlg, "kbibtexvaluewidget" );
        dlg->setMainWidget( valueWidget );
        connect( dlg, SIGNAL( okClicked() ), valueWidget, SLOT( apply() ) );

        QDialog::DialogCode result = ( QDialog::DialogCode )dlg->exec();
        if ( isReadOnly ) result = QDialog::Rejected;

        delete( valueWidget );
        delete( dlg );

        return result;
    }

    ValueWidget::ValueWidget( BibTeX::EntryField::FieldType fieldType, BibTeX::Value *value, bool isReadOnly, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_fieldType( fieldType ), m_value( value ), m_newValueCounter( 0 ), m_isReadOnly( isReadOnly )
    {
        setupGUI();
        reset();
        updateGUI();
    }


    ValueWidget::~ValueWidget()
    {
        // nothing
    }

    void ValueWidget::applyList( QStringList& list )
    {
        if ( m_fieldType == BibTeX::EntryField::ftKeywords )
        {
            BibTeX::KeywordContainer *container = new  BibTeX::KeywordContainer();
            for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
                container->keywords.append( new BibTeX::Keyword( *it ) );
            m_value->items.append( container );
        }
        else if ( m_fieldType == BibTeX::EntryField::ftAuthor || m_fieldType == BibTeX::EntryField::ftEditor )
        {
            Settings * settings = Settings::self();
            BibTeX::PersonContainer *container = new  BibTeX::PersonContainer();
            for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
                container->persons.append( new BibTeX::Person( *it, settings->editing_FirstNameFirst ) );
            m_value->items.append( container );
        }
        else
        {
            for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
                m_value->items.append( new BibTeX::PlainText( *it ) );
        }

        list.clear();
    }

    void ValueWidget::apply( )
    {
        if ( m_isReadOnly ) return;

        QStringList list;
        m_value->items.clear();
        QListViewItem *item = m_listViewValue->firstChild();
        while ( item != NULL )
        {
            QCheckListItem * checkItem = dynamic_cast<QCheckListItem*>( item );
            QString text = checkItem->text( 0 );

            if ( checkItem->state() == QCheckListItem::On )
            {
                if ( !list.isEmpty() ) applyList( list );
                m_value->items.append( new BibTeX::MacroKey( text ) );
            }
            else
                list.append( text );
            item = item->nextSibling();
        }

        if ( !list.isEmpty() )
            applyList( list );
    }

    void ValueWidget::reset( )
    {
        m_listViewValue->clear();

        QCheckListItem *after = NULL;
        for ( QValueList<BibTeX::ValueItem*>::ConstIterator it = m_value->items.begin(); it != m_value->items.end(); ++it )
        {
            BibTeX::MacroKey *macroKey = dynamic_cast<BibTeX::MacroKey*>( *it );
            QCheckListItem *item = new QCheckListItem( m_listViewValue, after, ( *it )->text(), QCheckListItem::CheckBox );
            after = item;
            item->setState( macroKey != NULL ? QCheckListItem::On : QCheckListItem::Off );
            item->setRenameEnabled( 0, !m_isReadOnly );
        }
    }

    void ValueWidget::slotAdd()
    {
        QCheckListItem * item = new QCheckListItem( m_listViewValue, m_listViewValue->lastItem(), QString( i18n( "May only contain ASCII characters, in case of doubt keep English form", "NewValue%1" ) ).arg( m_newValueCounter++ ), QCheckListItem::CheckBox );
        item->setState( QCheckListItem::Off );
        item->setRenameEnabled( 0, !m_isReadOnly );
        m_listViewValue->setSelected( item, TRUE );
        slotEdit();
    }

    void ValueWidget::slotEdit()
    {
        QListViewItem * item = m_listViewValue->selectedItem();
        if ( item != NULL )
            item->startRename( 0 );
    }

    void ValueWidget::slotToggle()
    {
        QListViewItem * item = m_listViewValue->selectedItem();
        if ( item != NULL )
        {
            QCheckListItem * checkItem = dynamic_cast<QCheckListItem*>( item );
            if ( checkItem != NULL )
                checkItem->setOn( !checkItem->isOn() );
        }
    }

    void ValueWidget::slotDelete()
    {
        QListViewItem * item = m_listViewValue->selectedItem();
        if ( item != NULL )
            delete item;
        updateGUI();
    }

    void ValueWidget::slotUp()
    {
        QListViewItem * item = m_listViewValue->selectedItem();
        if ( item != NULL && item -> itemAbove() != NULL )
        {
            item->itemAbove() ->moveItem( item );
            updateGUI();
        }
    }

    void ValueWidget::slotDown()
    {
        QListViewItem * item = m_listViewValue->selectedItem();
        if ( item != NULL && item -> itemBelow() != NULL )
        {
            item->moveItem( item->itemBelow() );
            updateGUI();
        }
    }

    void ValueWidget::updateGUI()
    {
        bool isElementSelected = m_listViewValue->selectedItem() != NULL;
        m_pushButtonEdit->setEnabled( !m_isReadOnly && isElementSelected );
        m_pushButtonToggle->setEnabled( !m_isReadOnly && isElementSelected );
        m_pushButtonDelete->setEnabled( !m_isReadOnly && isElementSelected );
        m_pushButtonUp->setEnabled( !m_isReadOnly && isElementSelected && m_listViewValue->selectedItem() != m_listViewValue->firstChild() );
        m_pushButtonDown->setEnabled( !m_isReadOnly && isElementSelected && m_listViewValue->selectedItem() != m_listViewValue->lastItem() );
    }

    void ValueWidget::setupGUI()
    {
        QGridLayout * layout = new QGridLayout( this, 8, 2, 0, KDialog::marginHint() );
        layout->setRowStretch( 7, 1 );

        QLabel *label = new QLabel( i18n( "Checked entries are string keys, unchecked entries are quoted text." ), this );
        layout->addWidget( label, 0, 0 );

        m_listViewValue = new QListView( this );
        layout->addMultiCellWidget( m_listViewValue, 1, 7, 0, 0 );
        m_listViewValue->setDefaultRenameAction( m_isReadOnly ? QListView::Reject : QListView::Accept );
        m_listViewValue->addColumn( i18n( "Text" ) );
        m_listViewValue->setSorting( -1, TRUE );
        m_listViewValue->setAllColumnsShowFocus( TRUE );
        m_listViewValue->header() ->setClickEnabled( FALSE );
        m_listViewValue->header() ->setStretchEnabled( TRUE, 0 );
        m_listViewValue->setEnabled( !m_isReadOnly );
        connect( m_listViewValue, SIGNAL( selectionChanged() ), this, SLOT( updateGUI() ) );
        connect( m_listViewValue, SIGNAL( clicked( QListViewItem * ) ), this, SLOT( updateGUI() ) );
        connect( m_listViewValue, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( updateGUI() ) );

        m_pushButtonAdd = new QPushButton( i18n( "Add" ), this );
        layout->addWidget( m_pushButtonAdd, 1, 1 );
        m_pushButtonAdd->setIconSet( QIconSet( SmallIcon( "add" ) ) );
        m_pushButtonAdd->setEnabled( !m_isReadOnly );
        connect( m_pushButtonAdd, SIGNAL( clicked() ), this, SLOT( slotAdd() ) );

        m_pushButtonEdit = new QPushButton( i18n( "Edit" ), this );
        layout->addWidget( m_pushButtonEdit, 2, 1 );
        m_pushButtonEdit->setIconSet( QIconSet( SmallIcon( "edit" ) ) );
        connect( m_pushButtonEdit, SIGNAL( clicked() ), this, SLOT( slotEdit() ) );

        m_pushButtonToggle = new QPushButton( i18n( "Toggle" ), this );
        layout->addWidget( m_pushButtonToggle, 3, 1 );
        m_pushButtonToggle->setIconSet( QIconSet( SmallIcon( "flag" ) ) );
        connect( m_pushButtonToggle, SIGNAL( clicked() ), this, SLOT( slotToggle() ) );

        m_pushButtonDelete = new QPushButton( i18n( "Delete" ), this );
        layout->addWidget( m_pushButtonDelete, 4, 1 );
        m_pushButtonDelete->setIconSet( QIconSet( SmallIcon( "editdelete" ) ) );
        connect( m_pushButtonDelete, SIGNAL( clicked() ), this, SLOT( slotDelete() ) );

        m_pushButtonUp = new QPushButton( i18n( "Up" ), this );
        layout->addWidget( m_pushButtonUp, 5, 1 );
        m_pushButtonUp->setIconSet( QIconSet( SmallIcon( "up" ) ) );
        connect( m_pushButtonUp, SIGNAL( clicked() ), this, SLOT( slotUp() ) );

        m_pushButtonDown = new QPushButton( i18n( "Down" ), this );
        layout->addWidget( m_pushButtonDown, 6, 1 );
        m_pushButtonDown->setIconSet( QIconSet( SmallIcon( "down" ) ) );
        connect( m_pushButtonDown, SIGNAL( clicked() ), this, SLOT( slotDown() ) );
    }

}
#include "valuewidget.moc"
