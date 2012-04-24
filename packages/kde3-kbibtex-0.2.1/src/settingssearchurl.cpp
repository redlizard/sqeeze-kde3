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
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qheader.h>
#include <qcombobox.h>

#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include <settings.h>
#include "settingssearchurl.h"

namespace KBibTeX
{
    SettingsSearchURL::SettingsSearchURL( QWidget *parent, const char *name )
            : QWidget( parent, name ), m_counter( 1 )
    {
        setupGUI();
    }


    SettingsSearchURL::~SettingsSearchURL()
    {
        // nothing
    }

    void SettingsSearchURL::applyData()
    {
        Settings * settings = Settings::self();
        settings->searchURLs.clear();
        for ( QListViewItemIterator it( m_listviewSearchURLs ); it.current(); it++ )
        {
            Settings::SearchURL *searchURL = new Settings::SearchURL;
            searchURL->description = it.current() ->text( 0 );
            searchURL->includeAuthor = it.current() ->text( 1 ) == i18n( "Yes" );
            searchURL->url = it.current() ->text( 2 );
            settings->searchURLs.append( searchURL );
        }
    }

    void SettingsSearchURL::readData()
    {
        Settings * settings = Settings::self();

        m_listviewSearchURLs->clear();
        for ( QValueList<Settings::SearchURL*>::ConstIterator it = settings->searchURLs.begin(); it != settings->searchURLs.end(); ++it )
        {
            KListViewItem *item = new KListViewItem( m_listviewSearchURLs, ( *it ) ->description, ( *it ) ->includeAuthor ? i18n( "Yes" ) : i18n( "No" ), ( *it ) ->url );
            item->setPixmap( 0, SmallIcon( "html" ) );
        }
    }

    void SettingsSearchURL::slotNew()
    {
        urlDialog();
        emit configChanged();
        updateGUI();
    }

    void SettingsSearchURL::slotDelete()
    {
        QListViewItem * item = m_listviewSearchURLs->selectedItem();
        if ( item != NULL )
        {
            m_listviewSearchURLs->removeItem( item );
            emit configChanged();
        }
        updateGUI();
    }

    void SettingsSearchURL::slotEdit()
    {
        QListViewItem * item = m_listviewSearchURLs->selectedItem();
        if ( item != NULL )
        {
            urlDialog( item );
            emit configChanged();
        }
        updateGUI();
    }

    void SettingsSearchURL::slotReset()
    {
        if ( KMessageBox:: warningContinueCancel( this, i18n( "The list of URLs will be checked and known entries will be replaced by the program standards. Search entries you have defined by yourself will be kept most likely." ), i18n( "Reset list of URLs" ), KGuiItem( i18n( "Reset" ), "reload" ) ) == KMessageBox::Continue )
        {
            Settings::self() ->restoreDefaultSearchURLs();
            readData();
            emit configChanged();
        }
        updateGUI();
    }

    void SettingsSearchURL::updateGUI()
    {
        bool enable = m_listviewSearchURLs->selectedItem() != NULL;
        m_pushbuttonEdit->setEnabled( enable );
        m_pushbuttonDelete->setEnabled( enable );
    }

    void SettingsSearchURL::setupGUI()
    {
        QGridLayout * layout = new QGridLayout( this, 5, 2, 0, KDialog::spacingHint() );

        m_listviewSearchURLs = new KListView( this );
        layout->addMultiCellWidget( m_listviewSearchURLs, 0, 4, 0, 0 );
        m_listviewSearchURLs->setAllColumnsShowFocus( TRUE );
        m_listviewSearchURLs->addColumn( i18n( "Description" ) );
        m_listviewSearchURLs->addColumn( i18n( "Author" ) );
        m_listviewSearchURLs->addColumn( i18n( "URL" ) );
        m_listviewSearchURLs->header()->setClickEnabled( FALSE );
        m_listviewSearchURLs->setResizeMode( QListView::LastColumn );
        m_listviewSearchURLs->setMinimumWidth( 384 );

        m_pushbuttonNew = new QPushButton( i18n( "search url", "New" ), this );
        m_pushbuttonNew->setIconSet( QIconSet( SmallIcon( "add" ) ) );
        layout->addWidget( m_pushbuttonNew, 0, 1 );

        m_pushbuttonEdit = new QPushButton( i18n( "search url", "Edit" ), this );
        m_pushbuttonEdit->setIconSet( QIconSet( SmallIcon( "edit" ) ) );
        layout->addWidget( m_pushbuttonEdit, 1, 1 );

        m_pushbuttonDelete = new QPushButton( i18n( "search url", "Delete" ), this );
        m_pushbuttonDelete->setIconSet( QIconSet( SmallIcon( "editdelete" ) ) );
        layout->addWidget( m_pushbuttonDelete, 2, 1 );

        m_pushbuttonReset = new QPushButton( i18n( "search url", "Reset" ), this );
        m_pushbuttonReset->setIconSet( QIconSet( SmallIcon( "reload" ) ) );
        layout->addWidget( m_pushbuttonReset, 4, 1 );

        connect( m_pushbuttonNew, SIGNAL( clicked() ), this, SLOT( slotNew() ) );
        connect( m_pushbuttonEdit, SIGNAL( clicked() ), this, SLOT( slotEdit() ) );
        connect( m_listviewSearchURLs, SIGNAL( doubleClicked( QListViewItem *, const QPoint &, int ) ), this, SLOT( slotEdit() ) );
        connect( m_pushbuttonDelete, SIGNAL( clicked() ), this, SLOT( slotDelete() ) );
        connect( m_pushbuttonReset, SIGNAL( clicked() ), this, SLOT( slotReset() ) );
        connect( m_listviewSearchURLs, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( updateGUI() ) );
        connect( m_listviewSearchURLs, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( updateGUI() ) );
        connect( m_listviewSearchURLs, SIGNAL( onItem( QListViewItem * ) ), this, SLOT( updateGUI() ) );

        updateGUI();
    }

    void SettingsSearchURL::urlDialog( QListViewItem * item )
    {
        KDialogBase * dlg = new KDialogBase( this, "urldialog", TRUE, item == NULL ? i18n( "New URL" ) : i18n( "Edit URL" ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, TRUE );
        QWidget *container = new QWidget( dlg, "container" );
        QGridLayout *layout = new QGridLayout( container, 3, 2, 0, KDialog::marginHint() );
        QLabel *label = new QLabel( i18n( "Description:" ), container );
        layout->addWidget( label, 0, 0 );
        QLineEdit *descr = new QLineEdit( container );
        label->setBuddy( descr );
        layout->addWidget( descr, 0, 1 );
        label = new QLabel( i18n( "URL:" ), container );
        layout->addWidget( label, 1, 0 );
        QLineEdit *url = new QLineEdit( container );
        layout->addWidget( url, 1, 1 );
        label->setBuddy( url );
        url->setMinimumWidth( 384 );
        QToolTip::add( url, i18n( "Within the URL, '%1' will be replaced by the search term." ) );
        label = new QLabel( i18n( "Include Author:" ), container );
        layout->addWidget( label, 2, 0 );
        QComboBox *cbIncludeAuthor = new QComboBox( FALSE, container );
        layout->addWidget( cbIncludeAuthor, 2, 1 );
        label->setBuddy( cbIncludeAuthor );
        cbIncludeAuthor->insertItem( i18n( "Yes" ) );
        cbIncludeAuthor->insertItem( i18n( "No" ) );

        dlg->setMainWidget( container );

        if ( item != NULL )
        {
            descr->setText( item->text( 0 ) );
            url->setText( item->text( 2 ) );
            cbIncludeAuthor->setCurrentItem( item->text( 1 ) == i18n( "Yes" ) ? 0 : 1 );
        }

        if ( dlg->exec() == QDialog::Accepted )
        {
            if ( item == NULL )
            {
                KListViewItem *item = new KListViewItem( m_listviewSearchURLs, descr->text(), cbIncludeAuthor->currentItem() == 0 ? i18n( "Yes" ) : i18n( "No" ), url->text() );
                item->setPixmap( 0, SmallIcon( "html" ) );
            }
            else
            {
                item->setText( 0, descr->text() );
                item->setText( 1, cbIncludeAuthor->currentItem() == 0 ? i18n( "Yes" ) : i18n( "No" ) );
                item->setText( 2, url->text() );
            }
        }

        delete dlg;
    }
}

#include "settingssearchurl.moc"
