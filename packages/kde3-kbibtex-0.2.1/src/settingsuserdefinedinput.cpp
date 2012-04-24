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
#include <qheader.h>
#include <qlabel.h>
#include <qtooltip.h>

#include <kdialog.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <klistview.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <settings.h>
#include <fieldlineedit.h>
#include "settingsuserdefinedinput.h"

namespace KBibTeX
{

    SettingsUserDefinedInput::SettingsUserDefinedInput( QWidget *parent, const char *name )            : QWidget( parent, name )
    {
        setupGUI();
    }

    SettingsUserDefinedInput::~SettingsUserDefinedInput()
    {
// TODO
    }

    void SettingsUserDefinedInput::applyData()
    {
        Settings *settings = Settings::self( NULL );
        settings->userDefinedInputFields.clear();
        for ( QListViewItemIterator it( m_listFields ); it.current(); ++it )
        {
            Settings::UserDefinedInputFields *udif = new Settings::UserDefinedInputFields();
            udif->name = it.current()->text( 0 );
            udif->label = it.current()->text( 1 );
            udif->inputType = it.current()->text( 2 ) == i18n( "Single line" ) ? FieldLineEdit::itSingleLine : FieldLineEdit::itMultiLine;
            settings->userDefinedInputFields << udif;
        }
    }

    void SettingsUserDefinedInput::readData()
    {
        m_listFields->clear();
        Settings *settings = Settings::self( NULL );
        KListViewItem *prev = NULL;
        for ( QValueList<Settings::UserDefinedInputFields*>::iterator it = settings->userDefinedInputFields.begin(); it != settings->userDefinedInputFields.end(); ++it )
        {
            prev = new KListViewItem( m_listFields, prev, ( *it )->name, ( *it )->label, ( *it )->inputType == FieldLineEdit::itMultiLine ? i18n( "Multiple lines" ) : i18n( "Single line" ) );
        }
    }

    void SettingsUserDefinedInput::slotNewField()
    {
        fieldDialog();
        emit configChanged();
        updateGUI();
    }

    void SettingsUserDefinedInput::slotEditField()
    {
        QListViewItem * item = m_listFields->selectedItem();
        if ( item != NULL )
        {
            fieldDialog( item );
            emit configChanged();
        }
        updateGUI();
    }

    void SettingsUserDefinedInput::slotDeleteField()
    {
        QListViewItem * item = m_listFields->selectedItem();
        if ( item != NULL )
        {
            m_listFields->removeItem( item );
            emit configChanged();
        }
        updateGUI();
    }

    void SettingsUserDefinedInput::slotMoveUpField()
    {
        QListViewItem *item = m_listFields->selectedItem();
        if ( item != NULL && item -> itemAbove() != NULL )
        {
            QListViewItem *itemAbove = item->itemAbove();
            for ( int i = 0; i < 3; ++i )
            {
                QString text = item->text( i );
                item->setText( i, itemAbove->text( i ) );
                itemAbove->setText( i, text );
            }
            m_listFields->setCurrentItem( itemAbove );
            m_listFields->ensureItemVisible( itemAbove );
            updateGUI();
        }
    }

    void SettingsUserDefinedInput::slotMoveDownField()
    {
        QListViewItem *item = m_listFields->selectedItem();
        if ( item != NULL && item -> itemBelow() != NULL )
        {
            QListViewItem *itemBelow = item->itemBelow();
            for ( int i = 0; i < 3; ++i )
            {
                QString text = item->text( i );
                item->setText( i, itemBelow->text( i ) );
                itemBelow->setText( i, text );
            }
            m_listFields->setCurrentItem( itemBelow );
            m_listFields->ensureItemVisible( itemBelow );
            updateGUI();
        }
    }

    void SettingsUserDefinedInput::updateGUI()
    {
        QListViewItem *item = m_listFields->selectedItem();
        bool selected = item != NULL;
        m_buttonEditField->setEnabled( selected );
        m_buttonDeleteField->setEnabled( selected );
        m_buttonMoveDownField->setEnabled( selected && item->itemBelow() != NULL );
        m_buttonMoveUpField->setEnabled( selected && item->itemAbove() != NULL );
    }

    void SettingsUserDefinedInput::updateDialogGUI()
    {
        bool enable = !m_lineEditName->text().isEmpty() && !m_lineEditLabel->text().isEmpty();
        fieldDlg->enableButtonOK( enable );
        fieldDlg->enableButtonApply( enable );
    }

    void SettingsUserDefinedInput::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 6, 2, 0, KDialog::spacingHint(), "gridLayout" );
        gridLayout->setRowStretch( 5, 1 );
        gridLayout->setColStretch( 0, 1 );

        m_listFields = new KListView( this );
        m_listFields->setSorting( -1, FALSE );
        m_listFields->addColumn( i18n( "Name" ) );
        m_listFields->header()->setClickEnabled( FALSE );
        m_listFields->addColumn( i18n( "Label" ) );
        m_listFields->header()->setClickEnabled( FALSE );
        m_listFields->addColumn( i18n( "Type" ) );
        m_listFields->header()->setClickEnabled( FALSE );
        m_listFields->header()->setClickEnabled( FALSE );
        m_listFields->setResizeMode( QListView::LastColumn );
        m_listFields->setMinimumWidth( 128 );
        m_listFields->setAllColumnsShowFocus( TRUE );
        gridLayout->addMultiCellWidget( m_listFields, 0, 5, 0, 0 );

        m_buttonNewField = new KPushButton( i18n( "user-defined input", "New" ), this );
        m_buttonNewField->setIconSet( QIconSet( SmallIcon( "add" ) ) );
        gridLayout->addWidget( m_buttonNewField, 0, 1 );
        m_buttonEditField = new KPushButton( i18n( "user-defined input",  "Edit" ), this );
        m_buttonEditField->setIconSet( QIconSet( SmallIcon( "edit" ) ) );
        gridLayout->addWidget( m_buttonEditField, 1, 1 );
        m_buttonDeleteField = new KPushButton( i18n( "user-defined input",  "Delete" ), this );
        m_buttonDeleteField->setIconSet( QIconSet( SmallIcon( "editdelete" ) ) );
        gridLayout->addWidget( m_buttonDeleteField, 2, 1 );
        m_buttonMoveUpField = new KPushButton( i18n( "user-defined input", "Up" ), this );
        m_buttonMoveUpField->setIconSet( QIconSet( SmallIcon( "up" ) ) );
        gridLayout->addWidget( m_buttonMoveUpField, 3, 1 );
        connect( m_buttonMoveUpField, SIGNAL( clicked() ), this, SLOT( slotMoveUpField() ) );
        m_buttonMoveDownField = new KPushButton( i18n( "user-defined input",  "Down" ), this );
        m_buttonMoveDownField->setIconSet( QIconSet( SmallIcon( "down" ) ) );
        gridLayout->addWidget( m_buttonMoveDownField, 4, 1 );
        connect( m_buttonMoveDownField, SIGNAL( clicked() ), this, SLOT( slotMoveDownField() ) );

        connect( m_buttonNewField, SIGNAL( clicked() ), this, SLOT( slotNewField() ) );
        connect( m_buttonEditField, SIGNAL( clicked() ), this, SLOT( slotEditField() ) );
        connect( m_buttonDeleteField, SIGNAL( clicked() ), this, SLOT( slotDeleteField() ) );
        connect( m_listFields, SIGNAL( selectionChanged() ), this, SLOT( updateGUI() ) );
        connect( m_listFields, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( updateGUI() ) );

        updateGUI();
    }

    void SettingsUserDefinedInput::fieldDialog( QListViewItem * item )
    {
        fieldDlg = new KDialogBase( this, "fieldDialog", TRUE, item == NULL ? i18n( "New Field" ) : i18n( "Edit Field" ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, TRUE );
        QWidget *container = new QWidget( fieldDlg, "container" );
        QGridLayout *layout = new QGridLayout( container, 3, 2, 0, KDialog::marginHint() );
        QLabel *label = new QLabel( i18n( "Name:" ), container );
        layout->addWidget( label, 0, 0 );
        m_lineEditName = new KLineEdit( container );
        label->setBuddy( m_lineEditName );
        layout->addWidget( m_lineEditName, 0, 1 );
        label = new QLabel( i18n( "Label:" ), container );
        layout->addWidget( label, 1, 0 );
        m_lineEditLabel = new KLineEdit( container );
        layout->addWidget( m_lineEditLabel, 1, 1 );
        label->setBuddy( m_lineEditLabel );
        m_lineEditLabel->setMinimumWidth( 384 );
        label = new QLabel( i18n( "Input Type:" ), container );
        layout->addWidget( label, 2, 0 );
        KComboBox *comboBoxInputType = new KComboBox( FALSE, container );
        layout->addWidget( comboBoxInputType, 2, 1 );
        label->setBuddy( comboBoxInputType );
        comboBoxInputType->insertItem( i18n( "Single line" ) );
        comboBoxInputType->insertItem( i18n( "Multiple lines" ) );

        fieldDlg->setMainWidget( container );

        if ( item != NULL )
        {
            m_lineEditName->setText( item->text( 0 ) );
            m_lineEditLabel->setText( item->text( 1 ) );
            comboBoxInputType->setCurrentItem( item->text( 2 ) == i18n( "Single line" ) ? 0 : 1 );
        }

        connect( m_lineEditName, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateDialogGUI() ) );
        connect( m_lineEditLabel, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateDialogGUI() ) );
        if ( fieldDlg->exec() == QDialog::Accepted )
        {
            if ( item == NULL )
            {
                /*KListViewItem *item = */
                new KListViewItem( m_listFields, m_lineEditName->text(), m_lineEditLabel->text(), comboBoxInputType->currentItem() == 0 ? i18n( "Single line" ) : i18n( "Multiple lines" ) );
            }
            else
            {
                item->setText( 0, m_lineEditName->text() );
                item->setText( 1, m_lineEditLabel->text() );
                item->setText( 2, comboBoxInputType->currentItem() == 0 ? i18n( "Single line" ) : i18n( "Multiple lines" ) );
            }
        }

        delete fieldDlg;
    }
}
#include "settingsuserdefinedinput.moc"
