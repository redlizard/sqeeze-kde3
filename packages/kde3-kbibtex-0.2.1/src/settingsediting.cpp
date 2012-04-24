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
#include <qlabel.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <kpushbutton.h>
#include <kdirselectdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdialog.h>
#include <kfontdialog.h>

#include <documentlistview.h>
#include "settingsediting.h"

namespace KBibTeX
{
    SettingsEditing::SettingsEditing( QWidget *parent, const char *name )
            : QWidget( parent, name )
    {
        QGroupBox * group = NULL;
        QVBoxLayout *layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

        group = new QGroupBox( 2, Qt::Horizontal, i18n( "Main List" ), this );
        layout->addWidget( group );
        QLabel *label = new QLabel( i18n( "&Sorting:" ), group );
        m_comboBoxSortingColumn = new QComboBox( FALSE, group );
        m_comboBoxSortingColumn->insertItem( i18n( "Element Type" ) );
        m_comboBoxSortingColumn->insertItem( i18n( "Entry Id" ) );
        for ( int i = 0; i <= ( int ) BibTeX::EntryField::ftYear - ( int ) BibTeX::EntryField::ftAbstract; i++ )
        {
            BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType )( i + ( int ) BibTeX::EntryField::ftAbstract );
            QString label = Settings::fieldTypeToI18NString( fieldType );
            m_comboBoxSortingColumn->insertItem( label );
        }
        label->setBuddy( m_comboBoxSortingColumn );

        label = new QLabel( i18n( "So&rting order:" ), group );
        m_comboBoxSortingOrder = new QComboBox( FALSE, group );
        m_comboBoxSortingOrder->insertItem( i18n( "Ascending" ) );
        m_comboBoxSortingOrder->insertItem( i18n( "Descending" ) );
        label->setBuddy( m_comboBoxSortingOrder );

        label = new QLabel( i18n( "&Double click action:" ), group );
        m_comboBoxDoubleClickAction = new QComboBox( FALSE, group );
        m_comboBoxDoubleClickAction->insertItem( i18n( "Edit element" ) );
        m_comboBoxDoubleClickAction->insertItem( i18n( "Open document" ) );
        label->setBuddy( m_comboBoxDoubleClickAction );

        label = new QLabel( i18n( "On dragging with mouse:" ), group );
        m_comboBoxDragAction = new QComboBox( FALSE, group );
        m_comboBoxDragAction->insertItem( i18n( "Copy reference (\\cite{...})" ) );
        m_comboBoxDragAction->insertItem( i18n( "Copy BibTeX text (@article{...})" ) );
        label->setBuddy( m_comboBoxDragAction );

        group = new QGroupBox( 1, Qt::Vertical, i18n( "Entry Editing" ), this );
        layout->addWidget( group );
        m_checkBoxEnableAllFields = new QCheckBox( i18n( "Enable all &fields for editing" ), group );

        group = new QGroupBox( 1, Qt::Vertical, i18n( "Search Bar" ), this );
        layout->addWidget( group );
        m_checkBoxSearchBarClearField = new QCheckBox( i18n( "Reset field filter when changing filter text" ), group );

        group = new QGroupBox( 2, Qt::Horizontal, i18n( "Presentation" ), this );
        layout->addWidget( group );
        m_checkBoxUseSpecialFont = new QCheckBox( i18n( "Use special &font" ), group );
        m_pushButtonSpecialFont = new QPushButton( group );
        label = new QLabel( i18n( "Author and editor names:" ), group );
        m_comboBoxNameOrder = new QComboBox( group );
        label->setBuddy( m_comboBoxNameOrder );
        m_comboBoxNameOrder->insertItem( i18n( "John Doe" ) );
        m_comboBoxNameOrder->insertItem( i18n( "Doe, John" ) );
        QToolTip::add( m_comboBoxNameOrder, i18n( "Show names as 'John Doe' instead of 'Doe, John'" ) );
        QWhatsThis::add( m_comboBoxNameOrder, i18n( "Show names as 'John Doe' instead of 'Doe, John'.\n\nTakes only effect after the next start of KBibTeX." ) );

        group = new QGroupBox( 2, Qt::Vertical, i18n( "Document Search Path" ), this );
        layout->addWidget( group );
        label = new QLabel( i18n( "Default search path for documents (e.g. PDF files) associated with an BibTeX entry:" ), group );
        QWidget *container = new QWidget( group );
        QHBoxLayout *containerLayout = new QHBoxLayout( container, 0, KDialog::spacingHint() );
        m_lineEditDocumentSearchPath = new KLineEdit( container );
        containerLayout->addWidget( m_lineEditDocumentSearchPath );
        KPushButton *btnSelectDocumentSearchPath = new KPushButton( SmallIcon( "fileopen" ), "", container );
        containerLayout->addWidget( btnSelectDocumentSearchPath );
        label->setBuddy( m_lineEditDocumentSearchPath );

        layout->addStretch();

        connect( m_checkBoxSearchBarClearField, SIGNAL( toggled( bool ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_checkBoxEnableAllFields, SIGNAL( toggled( bool ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxDoubleClickAction, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxDragAction, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxSortingColumn, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxSortingOrder, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_pushButtonSpecialFont, SIGNAL( clicked() ), this, SLOT( slotSelectSpecialFont() ) );
        connect( m_checkBoxUseSpecialFont, SIGNAL( toggled( bool ) ), m_pushButtonSpecialFont, SLOT( setEnabled( bool ) ) );
        connect( m_checkBoxUseSpecialFont, SIGNAL( toggled( bool ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_lineEditDocumentSearchPath, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotConfigChanged() ) );
        connect( btnSelectDocumentSearchPath, SIGNAL( clicked() ), this, SLOT( slotSelectDocumentSearchPath() ) );
    }


    SettingsEditing::~SettingsEditing()
    {
        // nothing
    }

    void SettingsEditing::applyData()
    {
        Settings * settings = Settings::self();

        settings->editing_SearchBarClearField = m_checkBoxSearchBarClearField->isChecked();
        settings->editing_EnableAllFields = m_checkBoxEnableAllFields->isChecked();
        settings->editing_MainListSortingColumn = m_comboBoxSortingColumn->currentItem();
        settings->editing_MainListSortingOrder = m_comboBoxSortingOrder->currentItem() == 0 ? 1 : -1;
        settings->editing_MainListDoubleClickAction = m_comboBoxDoubleClickAction->currentItem();
        settings->editing_DragAction = m_comboBoxDragAction->currentItem() == 0 ? Settings::COPYREFERENCE : Settings::COPYBIBTEX;

        settings->editing_UseSpecialFont = m_checkBoxUseSpecialFont->isChecked();
        settings->editing_SpecialFont = m_specialFont;

        settings->editing_FirstNameFirst = m_comboBoxNameOrder->currentItem() == 0;

        settings->editing_DocumentSearchPath = m_lineEditDocumentSearchPath->text();
    }

    void SettingsEditing::readData()
    {
        Settings * settings = Settings::self();

        m_checkBoxSearchBarClearField->setChecked( settings->editing_SearchBarClearField );
        m_checkBoxEnableAllFields->setChecked( settings->editing_EnableAllFields );
        m_comboBoxSortingColumn->setCurrentItem( settings->editing_MainListSortingColumn );
        m_comboBoxSortingOrder->setCurrentItem( settings->editing_MainListSortingOrder == 1 ? 0 : 1 );
        m_comboBoxDoubleClickAction->setCurrentItem( settings->editing_MainListDoubleClickAction );
        m_comboBoxDragAction->setCurrentItem( settings->editing_DragAction == Settings::COPYREFERENCE ? 0 : 1 );

        m_checkBoxUseSpecialFont->setChecked( settings->editing_UseSpecialFont );
        m_specialFont = settings->editing_SpecialFont;
        updateFontData();
        m_pushButtonSpecialFont->setEnabled( m_checkBoxUseSpecialFont->isChecked() );

        m_comboBoxNameOrder->setCurrentItem( settings->editing_FirstNameFirst ? 0 : 1 );

        m_lineEditDocumentSearchPath->setText( settings->editing_DocumentSearchPath );
    }

    void SettingsEditing::slotConfigChanged()
    {
        emit configChanged();
    }

    void SettingsEditing::slotSelectSpecialFont()
    {
        int result = KFontDialog::getFont( m_specialFont );
        if ( result == KFontDialog::Accepted )
        {
            updateFontData();
            emit configChanged();
        }
    }

    void SettingsEditing::slotSelectDocumentSearchPath()
    {
        KURL url( KDirSelectDialog::selectDirectory() );
        if ( url.isValid() && !url.isEmpty() )
            m_lineEditDocumentSearchPath->setText( url.prettyURL() );
    }

    void SettingsEditing::updateFontData()
    {
        m_pushButtonSpecialFont->setText( m_specialFont.family() );
        m_pushButtonSpecialFont->setFont( m_specialFont );
    }
}

#include "settingsediting.moc"
