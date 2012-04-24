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
#include <qstring.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qheader.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <klistview.h>
#include <klineedit.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <value.h>
#include <valuewidget.h>
#include <settings.h>
#include "fieldlistview.h"

namespace KBibTeX
{

    FieldListView::FieldListView( const QString& caption, const QString& prefixNew, bool isReadOnly, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_value( new BibTeX::Value() ), m_caption( caption ), m_prefixNew( prefixNew ), m_isReadOnly( isReadOnly ), m_enabled( TRUE ), m_isComplex( FALSE ), m_isModified( FALSE ), m_newValueCounter( 0 ), m_fieldType( BibTeX::EntryField::ftUnknown )
    {
        setupGUI();
        m_listViewElements->installEventFilter( this );
        m_listViewElements->renameLineEdit() ->installEventFilter( this );

        m_value = new BibTeX::Value();
    }


    FieldListView::~FieldListView()
    {
        delete m_value;
    }

    void FieldListView::setValue( const BibTeX::Value *value )
    {
        if ( value != m_value )
        {
            if ( m_value != NULL )
                delete m_value;

            if ( value != NULL )
                m_value = new BibTeX::Value( value );
            else
                m_value = new BibTeX::Value( );

            reset();
            updateGUI();

            m_isModified = FALSE;
        }
    }

    BibTeX::Value *FieldListView::value()
    {
        if ( m_value->items.isEmpty() )
            return NULL;
        else
            return new BibTeX::Value( m_value );
    }

    void FieldListView::setEnabled( bool enabled )
    {
        m_enabled = enabled;
        updateGUI();
    }

    void FieldListView::setFieldType( BibTeX::EntryField::FieldType fieldType )
    {
        m_fieldType = fieldType;

        Settings * settings = Settings::self();
        m_listViewElements->renameLineEdit() ->setCompletionObject( settings->completion( m_fieldType ) );
        QToolTip::add( m_listViewElements, QString( i18n( "BibTeX field '%1'" ) ).arg( BibTeX::EntryField::fieldTypeToString( fieldType ) ) );
        QWhatsThis::add( m_listViewElements, QString( i18n( "BibTeX field '%1'" ) ).arg( BibTeX::EntryField::fieldTypeToString( fieldType ) ) );

        m_value->items.clear();
    }

    QString FieldListView::caption()
    {
        return m_caption;
    }

    bool FieldListView::isEmpty()
    {
        return m_value != NULL ? m_value->items.isEmpty() : TRUE;
    }

    bool FieldListView::isModified()
    {
        return m_isModified;
    }

    bool FieldListView::eventFilter( QObject *o, QEvent * e )
    {
        if ( o == m_listViewElements->renameLineEdit() )
        {
            if ( e->type() == QEvent::Hide )
                itemRenameDone();
        }
        else if ( e->type() == QEvent::AccelOverride )
        {
            QKeyEvent * ke = static_cast<QKeyEvent*>( e );
            //override delete action
            if ( ke->key() == Key_Delete && ke->state() == NoButton )
            {
                slotDelete();
                ke->accept();
                return true;
            }
            else if ( ke->key() == Key_F2 && ke->state() == NoButton )
            {
                slotEdit();
                ke->accept();
                return true;
            }
            else if ( ke->key() == Key_A && ke->state() == ControlButton )
            {
                slotAdd();
                ke->accept();
                return true;
            }
            else if ( ke->key() == Key_Up && ke->state() == ControlButton )
            {
                slotUp();
                ke->accept();
                return true;
            }
            else if ( ke->key() == Key_Down && ke->state() == ControlButton )
            {
                slotDown();
                ke->accept();
                return true;
            }
            else if ( ke->key() == Key_C && ke->state() == ControlButton | AltButton )
            {
                slotComplex();
                ke->accept();
                return true;
            }
        }
        return false;
    }


    void FieldListView::updateGUI()
    {
        disconnect( m_checkBoxEtAl, SIGNAL( toggled( bool ) ), this, SLOT( apply() ) );
        if ( m_value != NULL && !m_isComplex )
        {
            bool isElementSelected = m_listViewElements->selectedItem() != NULL;
            m_pushButtonAdd->setEnabled( !m_isReadOnly );
            m_pushButtonEdit->setEnabled( !m_isReadOnly && isElementSelected );
            m_pushButtonDelete->setEnabled( !m_isReadOnly && isElementSelected );
            m_pushButtonUp->setEnabled( !m_isReadOnly && isElementSelected && m_listViewElements->selectedItem() != m_listViewElements->firstChild() );
            m_pushButtonDown->setEnabled( !m_isReadOnly && isElementSelected && m_listViewElements->selectedItem() != m_listViewElements->lastItem() );
            m_listViewElements->setEnabled( !m_isReadOnly );
            m_checkBoxEtAl->setEnabled( !m_isReadOnly );
        }
        else
        {
            m_pushButtonAdd->setEnabled( FALSE );
            m_pushButtonEdit->setEnabled( FALSE );
            m_pushButtonDelete->setEnabled( FALSE );
            m_pushButtonUp->setEnabled( FALSE );
            m_pushButtonDown->setEnabled( FALSE );
            m_listViewElements->setEnabled( FALSE );
            m_checkBoxEtAl->setEnabled( FALSE );
        }
        connect( m_checkBoxEtAl, SIGNAL( toggled( bool ) ), this, SLOT( apply() ) );
    }

    void FieldListView::slotAdd()
    {
        if ( isSimple() )
        {
            KListViewItem * item = new KListViewItem( m_listViewElements, m_listViewElements->lastItem(), QString( "%1%2" ).arg( m_prefixNew ).arg( ++m_newValueCounter ) );
            m_listViewElements->setSelected( item, TRUE );
            updateGUI();
            QTimer::singleShot( 100, this, SLOT( slotEdit() ) );
        }
    }

    void FieldListView::slotEdit()
    {
        if ( isSimple() )
        {
            KListViewItem * item = static_cast<KListViewItem*>( m_listViewElements->selectedItem() );
            if ( item != NULL )
                m_listViewElements->rename( item, 0 );
        }
    }

    void FieldListView::slotDelete()
    {
        QListViewItem * item = m_listViewElements->selectedItem();
        if ( isSimple() && item != NULL )
        {
            delete item;
            apply();
            updateGUI();

            m_isModified = TRUE;
        }
    }

    void FieldListView::slotUp()
    {
        QListViewItem * item = m_listViewElements->selectedItem();
        if ( isSimple() && !m_listViewElements->isRenaming() && item != NULL && item -> itemAbove() != NULL )
        {
            item->itemAbove() ->moveItem( item );
            apply();
            updateGUI();

            m_isModified = TRUE;
        }
    }

    void FieldListView::slotDown()
    {
        QListViewItem * item = m_listViewElements->selectedItem();
        if ( isSimple() && !m_listViewElements->isRenaming() && item != NULL && item -> itemBelow() != NULL )
        {
            item->moveItem( item->itemBelow() );
            apply();
            updateGUI();

            m_isModified = TRUE;
        }
    }

    void FieldListView::slotComplex()
    {
        if ( !m_listViewElements->isRenaming() && ValueWidget::execute( m_caption, m_fieldType, m_value, m_isReadOnly, this ) == QDialog::Accepted )
        {
            reset();
            updateGUI();

            m_isModified = TRUE;
        }
    }

    void FieldListView::slotListViewDoubleClicked( QListViewItem * lvi )
    {
        if ( lvi == NULL )
            slotAdd();
    }

    void FieldListView::slotItemRenamed( QListViewItem * item, int /*col*/, const QString & text )
    {
        if ( text.isEmpty() && isSimple() && item != NULL )
        {
            delete item;
            updateGUI();
        }

        apply();
        m_isModified = TRUE;
    }

    void FieldListView::setupGUI()
    {
        Settings * settings = Settings::self();

        QGridLayout * layout = new QGridLayout( this, 8, 2, 0, KDialog::marginHint() );
        layout->setRowStretch( 5, 1 );

        m_listViewElements = new KListView( this );
        layout->addMultiCellWidget( m_listViewElements, 0, 6, 0, 0 );
        m_listViewElements->renameLineEdit() ->setCompletionObject( settings->completion( m_fieldType ) );
        m_listViewElements->renameLineEdit() ->setCompletionMode( KGlobalSettings::CompletionPopup );
        m_listViewElements->renameLineEdit() ->completionObject() ->setIgnoreCase( true );
        m_listViewElements->setDefaultRenameAction( QListView::Accept );
        m_listViewElements->addColumn( m_caption );
        m_listViewElements->setSorting( -1, TRUE );
        m_listViewElements->setItemsRenameable( TRUE );
        if ( settings->editing_UseSpecialFont )
            m_listViewElements->setFont( settings->editing_SpecialFont );
        m_listViewElements->header() ->setFont( KGlobalSettings::generalFont() );

        m_listViewElements->header() ->setClickEnabled( FALSE );
        m_listViewElements->header() ->setStretchEnabled( TRUE, 0 );
        connect( m_listViewElements, SIGNAL( selectionChanged() ), this, SLOT( updateGUI() ) );
        connect( m_listViewElements, SIGNAL( clicked( QListViewItem * ) ), this, SLOT( updateGUI() ) );
        connect( m_listViewElements, SIGNAL( doubleClicked( QListViewItem * ) ), this, SLOT( slotListViewDoubleClicked( QListViewItem * ) ) );
        connect( m_listViewElements, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( updateGUI() ) );
        connect( m_listViewElements, SIGNAL( itemRenamed( QListViewItem*, int, const QString& ) ), this, SLOT( slotItemRenamed( QListViewItem*, int, const QString& ) ) );

        m_pushButtonAdd = new QPushButton( i18n( "Add" ), this );
        layout->addWidget( m_pushButtonAdd, 0, 1 );
        m_pushButtonAdd->setIconSet( QIconSet( SmallIcon( "add" ) ) );
        connect( m_pushButtonAdd, SIGNAL( clicked() ), this, SLOT( slotAdd() ) );
        QToolTip::add( m_pushButtonAdd, QString( i18n( "Add new '%1' item (Ctrl+A)" ) ).arg( m_caption ) );

        m_pushButtonEdit = new QPushButton( i18n( "Edit" ), this );
        layout->addWidget( m_pushButtonEdit, 1, 1 );
        m_pushButtonEdit->setIconSet( QIconSet( SmallIcon( "edit" ) ) );
        connect( m_pushButtonEdit, SIGNAL( clicked() ), this, SLOT( slotEdit() ) );
        QToolTip::add( m_pushButtonEdit, QString( i18n( "Edit current '%1' item (F2)" ) ).arg( m_caption ) );

        m_pushButtonDelete = new QPushButton( i18n( "Delete" ), this );
        layout->addWidget( m_pushButtonDelete, 2, 1 );
        m_pushButtonDelete->setIconSet( QIconSet( SmallIcon( "editdelete" ) ) );
        connect( m_pushButtonDelete, SIGNAL( clicked() ), this, SLOT( slotDelete() ) );
        QToolTip::add( m_pushButtonDelete, QString( i18n( "Delete current '%1' item (Del)" ) ).arg( m_caption ) );

        m_pushButtonUp = new QPushButton( i18n( "Up" ), this );
        layout->addWidget( m_pushButtonUp, 3, 1 );
        m_pushButtonUp->setIconSet( QIconSet( SmallIcon( "up" ) ) );
        connect( m_pushButtonUp, SIGNAL( clicked() ), this, SLOT( slotUp() ) );
        QToolTip::add( m_pushButtonUp, QString( i18n( "Move current '%1' item up (Ctrl+Up)" ) ).arg( m_caption ) );

        m_pushButtonDown = new QPushButton( i18n( "Down" ), this );
        layout->addWidget( m_pushButtonDown, 4, 1 );
        m_pushButtonDown->setIconSet( QIconSet( SmallIcon( "down" ) ) );
        connect( m_pushButtonDown, SIGNAL( clicked() ), this, SLOT( slotDown() ) );
        QToolTip::add( m_pushButtonDown, QString( i18n( "Move current '%1' item down (Ctrl+Down)" ) ).arg( m_caption ) );

        m_pushButtonComplexEdit = new QPushButton( i18n( "Complex..." ), this );
        layout->addWidget( m_pushButtonComplexEdit, 6, 1 );
        m_pushButtonComplexEdit->setIconSet( QIconSet( SmallIcon( "leftjust" ) ) );
        connect( m_pushButtonComplexEdit, SIGNAL( clicked() ), this, SLOT( slotComplex() ) );
        QToolTip::add( m_pushButtonComplexEdit, QString( i18n( "Edit current '%1' item as a concatenated string (Ctrl+Alt+C)" ) ).arg( m_caption ) );

        m_checkBoxEtAl = new QCheckBox( i18n( "... and others (et al.)" ), this );
        layout->addMultiCellWidget( m_checkBoxEtAl, 7, 7, 0, 1 );
        connect( m_checkBoxEtAl, SIGNAL( toggled( bool ) ), this, SLOT( apply() ) );
    }

    void FieldListView::apply()
    {
        QStringList elements;
        Settings * settings = Settings::self();

        m_value->items.clear();

        for ( QListViewItemIterator it( m_listViewElements ); it.current(); it++ )
        {
            QString text = it.current() ->text( 0 );
            elements.append( text );
        }

        if ( elements.count() > 0 )
        {
            m_value->items.clear();
            BibTeX::PersonContainer *container = new BibTeX::PersonContainer( settings->editing_FirstNameFirst );

            switch ( m_fieldType )
            {
            case BibTeX::EntryField::ftAuthor:
            case BibTeX::EntryField::ftEditor:
                {
                    for ( QStringList::ConstIterator it = elements.constBegin(); it != elements.constEnd(); ++it )
                    {
                        BibTeX::Person *person = new BibTeX::Person( *it, settings->editing_FirstNameFirst );
                        container->persons.append( person );
                    }
                }
                break;
            default:
                kdDebug() << "Don't know how to handle entries of type " << BibTeX::EntryField::fieldTypeToString( m_fieldType ) << endl;
            }

            if ( m_checkBoxEtAl->isChecked() )
                container->persons.append( new BibTeX::Person( "others", settings->editing_FirstNameFirst ) );

            if ( !container->persons.isEmpty() )
                m_value->items.append( container );
            else
                delete container;

            settings->addToCompletion( m_value, m_fieldType );
        }
    }

    void FieldListView::reset()
    {
        disconnect( m_checkBoxEtAl, SIGNAL( toggled( bool ) ), this, SLOT( apply() ) );
        m_listViewElements->clear();
        m_checkBoxEtAl->setChecked( FALSE );

        m_isComplex = FALSE;
        for ( QValueList<BibTeX::ValueItem*>::ConstIterator it = m_value->items.constBegin(); !m_isComplex && it != m_value->items.constEnd(); ++it )
        {
            BibTeX::MacroKey *macroKey = dynamic_cast<BibTeX::MacroKey *>( *it );
            m_isComplex = macroKey != NULL;
        }

        if ( !m_isComplex )
            switch ( m_fieldType )
            {
            case BibTeX::EntryField::ftAuthor:
            case BibTeX::EntryField::ftEditor:
                {
                    for ( QValueList<BibTeX::ValueItem*>::ConstIterator it = m_value->items.constBegin(); it != m_value->items.constEnd(); ++it )
                    {
                        BibTeX::PersonContainer *container = dynamic_cast<BibTeX::PersonContainer *>( *it );
                        if ( container != NULL )
                            for ( QValueList<BibTeX::Person*>::ConstIterator pit = container->persons.constBegin(); pit != container->persons.constEnd(); ++pit )
                            {
                                QString text = ( *pit )->text();
                                if ( text == "others" )
                                    m_checkBoxEtAl->setChecked( TRUE );
                                else
                                    new QListViewItem( m_listViewElements, m_listViewElements->lastItem(), text );
                            }
                    }
                }
                break;
            default:
                kdDebug() << "Don't know how to handle entries of type " << BibTeX::EntryField::fieldTypeToString( m_fieldType ) << endl;
            }

        connect( m_checkBoxEtAl, SIGNAL( toggled( bool ) ), this, SLOT( apply() ) );
    }

    bool FieldListView::isSimple()
    {
        return m_value->items.count() == 0 || ( m_value->items.count() == 1 && dynamic_cast<BibTeX::MacroKey*>( m_value->items.first() ) == NULL );
    }

    void FieldListView::itemRenameDone()
    {
        apply();
    }
}
#include "fieldlistview.moc"
