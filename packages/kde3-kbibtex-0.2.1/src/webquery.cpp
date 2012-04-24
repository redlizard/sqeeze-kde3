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
#include <qwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qframe.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>

#include <klocale.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include <kurllabel.h>
#include <klineedit.h>
#include <kapplication.h>
#include <kpushbutton.h>
#include <kprogress.h>
#include <kcompletion.h>

#include <settings.h>
#include <webqueryamatex.h>
#include <webqueryarxiv.h>
#include <webquerybibsonomy.h>
#include <webquerycitebase.h>
#include <webquerydblp.h>
#include <webquerygooglescholar.h>
#include <webquerypubmed.h>
#include <webqueryspireshep.h>
#include <webqueryzmath.h>
#include "webquery.h"

namespace KBibTeX
{
    KDialogBase *WebQueryWizard::singletonDlg = NULL;
    WebQueryWizard *WebQueryWizard::singletonWiz = NULL;

    WebQuery::WebQuery( QWidget *parent ): QObject(), m_parent( parent )
    {
        // nothing
    }

    WebQuery::~WebQuery()
    {
        // nothing
    }

    void WebQuery::cancelQuery()
    {
        // nothing
    }

    void WebQuery::slotCancelQuery()
    {
        m_aborted = TRUE;
        cancelQuery();
    }

    WebQueryWizard::WebQueryWizard( KDialogBase *dlg, const char* name ) : QWidget( dlg, name ), m_dlg( dlg ), m_progressDialog( NULL )
    {
        setupGUI( );

        WebQuery *query = new WebQueryArXiv( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        query = new WebQueryAmatex( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        query = new WebQueryBibSonomy( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        query = new WebQueryCitebase( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        query = new WebQueryDBLP( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        query = new WebQueryGoogleScholar( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        query = new WebQueryPubMed( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        query = new WebQuerySpiresHep( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        query = new WebQueryZMATH( this );
        m_webQueries.append( query );
        m_comboBoxEngines->insertItem( query->title() );

        Settings *settings = Settings::self( NULL );
        m_comboBoxEngines->setCurrentItem( settings->webQuery_LastEngine );
        otherEngineSelected( settings->webQuery_LastEngine );
        m_lineEditQuery->setText( settings->webQuery_LastSearchTerm );
        queryTextChanged( settings->webQuery_LastSearchTerm );
        m_spinBoxMaxHits->setValue( settings->webQuery_LastNumberOfResults );
        m_checkBoxImportAll->setChecked( settings->webQuery_ImportAll );
    }

    WebQueryWizard::~WebQueryWizard()
    {
        if ( m_progressDialog != NULL ) delete m_progressDialog;
    }

    int WebQueryWizard::execute( QWidget *parent, QValueList<BibTeX::Entry*> &results )
    {
        if ( singletonDlg == NULL )
        {
            singletonDlg = new KDialogBase( parent, "WebQueryWizard", TRUE, i18n( "Import" ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, TRUE );
            singletonWiz = new WebQueryWizard( singletonDlg, "WebQueryWizard" );
            singletonDlg->setButtonOK( KGuiItem( i18n( "&Import" ), "import", i18n( "Import selected items" ) ) );
            singletonDlg->setMainWidget( singletonWiz );
            connect( singletonWiz, SIGNAL( changeButtonOK( bool ) ), singletonDlg, SLOT( enableButtonOK( bool ) ) );
        }
        singletonDlg->enableButtonOK( FALSE );

        results.clear();
        int result = singletonDlg->exec();
        if ( result == QDialog::Accepted )
        {
            QListViewItemIterator it = singletonWiz->m_checkBoxImportAll->isChecked() ? QListViewItemIterator( singletonWiz->m_listViewResults ) : QListViewItemIterator( singletonWiz->m_listViewResults, QListViewItemIterator::Selected );
            while ( it.current() )
            {
                ResultsListViewItem *item = dynamic_cast<ResultsListViewItem*>( it.current() );
                results.append( new BibTeX::Entry( item->entry() ) );
                ++it;
            }
        }

        Settings *settings = Settings::self( NULL );
        settings->webQuery_LastSearchTerm = singletonWiz->m_lineEditQuery->text();
        settings->webQuery_LastEngine = singletonWiz->m_comboBoxEngines->currentItem();
        settings->webQuery_LastNumberOfResults = singletonWiz->m_spinBoxMaxHits->value();
        settings->webQuery_ImportAll = singletonWiz->m_checkBoxImportAll->isChecked();

        return result;
    }

    void WebQueryWizard::importEnableChanging( )
    {
        QListViewItemIterator it( m_listViewResults, QListViewItemIterator::Selected );

        emit changeButtonOK(( m_checkBoxImportAll->isChecked() && m_listViewResults->childCount() > 0 ) || it.current() != NULL );
    }

    void WebQueryWizard::otherEngineSelected( int index )
    {
        if ( index < 0 || index >= ( int )m_webQueries.size() ) return;

        m_pushButtonSearch->setCaption( QString( i18n( "Search %1" ) ).arg( m_webQueries[index]->title() ) );
        m_disclaimerLabel->setText( m_webQueries[index]->disclaimer() );
        m_disclaimerLabel->setURL( m_webQueries[index]->disclaimerURL() );
        QToolTip::remove( m_disclaimerLabel );
        QToolTip::add( m_disclaimerLabel, m_webQueries[index]->disclaimerURL() );
    }

    void WebQueryWizard::startSearch()
    {
        QString searchTerm = m_lineEditQuery->text().stripWhiteSpace();
        if ( searchTerm.isEmpty() ) return;

        int index = m_comboBoxEngines->currentItem();
        if ( m_progressDialog == NULL )
            m_progressDialog = new KProgressDialog( this, "WebQueryWizard_m_progressDialog", i18n( "Searching" ), QString( i18n( "Searching %1" ) ).arg( m_webQueries[index]->title() ) );
        else
        {
            m_progressDialog->setLabel( QString( i18n( "Searching %1" ) ).arg( m_webQueries[index]->title() ) );
        }

        setEnabled( FALSE );
        m_progressDialog->progressBar()->setValue( 0 );
        m_progressDialog->progressBar()->setMinimumWidth( 256 );
        m_progressDialog->setEnabled( TRUE );
        m_progressDialog->show();
        m_dlg->enableButtonCancel( FALSE );
        QApplication::setOverrideCursor( Qt::waitCursor );
        m_listViewResults->clear();
        connect( m_webQueries[index], SIGNAL( foundEntry( BibTeX::Entry* ) ), this, SLOT( addHit( BibTeX::Entry* ) ) );
        connect( m_webQueries[index], SIGNAL( endSearch( bool ) ), this, SLOT( endSearch( bool ) ) );
        connect( m_webQueries[index], SIGNAL( setProgress( int ) ), m_progressDialog->progressBar(), SLOT( setProgress( int ) ) );
        connect( m_webQueries[index], SIGNAL( setTotalSteps( int ) ), m_progressDialog->progressBar(), SLOT( setTotalSteps( int ) ) );
        connect( m_progressDialog, SIGNAL( cancelClicked() ), m_webQueries[index], SLOT( slotCancelQuery() ) );
        searchTerm.replace( '$', "" );
        m_webQueries[index]->query( searchTerm, m_spinBoxMaxHits->value() );
    }

    void WebQueryWizard::endSearch( bool error )
    {
        if ( error )
            m_progressDialog->cancel();

        int index = m_comboBoxEngines->currentItem();
        disconnect( m_webQueries[index], SIGNAL( setProgress( int ) ), m_progressDialog->progressBar(), SLOT( setProgress( int ) ) );
        disconnect( m_webQueries[index], SIGNAL( setTotalSteps( int ) ), m_progressDialog->progressBar(), SLOT( setTotalSteps( int ) ) );
        disconnect( m_progressDialog, SIGNAL( cancelClicked() ), m_webQueries[index], SLOT( slotCancelQuery() ) );
        disconnect( m_webQueries[index], SIGNAL( foundEntry( BibTeX::Entry* ) ), this, SLOT( addHit( BibTeX::Entry* ) ) );
        disconnect( m_webQueries[index], SIGNAL( endSearch( bool ) ), this, SLOT( endSearch( bool ) ) );
        setEnabled( TRUE );
        m_dlg->enableButtonCancel( TRUE );
        importEnableChanging();
        QApplication::restoreOverrideCursor();
    }

    void WebQueryWizard::addHit( BibTeX::Entry *entry )
    {
        new ResultsListViewItem( m_listViewResults, new BibTeX::Entry( entry ) );
    }

    void WebQueryWizard::queryTextChanged( const QString& text )
    {
        m_pushButtonSearch->setEnabled( !text.isEmpty() );
    }

    void WebQueryWizard::setupGUI()
    {
        setMinimumSize( 720, 384 );
        QVBoxLayout *verticalLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );
        m_toolbarContainer = new QFrame( this );
        verticalLayout->addWidget( m_toolbarContainer );
        QHBoxLayout *horizontalLayout = new QHBoxLayout( m_toolbarContainer, 0, KDialog::spacingHint() );

        QLabel *label = new QLabel( i18n( "&Engine:" ), m_toolbarContainer );
        horizontalLayout->addWidget( label );
        m_comboBoxEngines = new KComboBox( FALSE, m_toolbarContainer );
        label->setBuddy( m_comboBoxEngines );
        horizontalLayout->addWidget( m_comboBoxEngines );
        horizontalLayout->addSpacing( KDialog::spacingHint() * 2 );
        connect( m_comboBoxEngines, SIGNAL( activated( int ) ), this, SLOT( otherEngineSelected( int ) ) );

        KPushButton *clearSearchText = new KPushButton( m_toolbarContainer );
        clearSearchText->setIconSet( QIconSet( BarIcon( "locationbar_erase" ) ) );
        horizontalLayout->addWidget( clearSearchText );
        label = new QLabel( i18n( "Search &term:" ), m_toolbarContainer );
        horizontalLayout->addWidget( label );
        m_lineEditQuery = new KLineEdit( m_toolbarContainer );
        horizontalLayout->addWidget( m_lineEditQuery );
        label->setBuddy( m_lineEditQuery );
        horizontalLayout->addSpacing( KDialog::spacingHint() * 2 );
        connect( clearSearchText, SIGNAL( clicked() ), m_lineEditQuery, SLOT( clear() ) );
        connect( m_lineEditQuery, SIGNAL( textChanged( const QString& ) ), this, SLOT( queryTextChanged( const QString& ) ) );
        horizontalLayout->setStretchFactor( m_lineEditQuery, 4 );
        KCompletion *completionQuery = m_lineEditQuery->completionObject();

        label = new QLabel( i18n( "&Number of results:" ), m_toolbarContainer );
        horizontalLayout->addWidget( label );
        m_spinBoxMaxHits = new QSpinBox( 1, 500, 1, m_toolbarContainer );
        m_spinBoxMaxHits->setValue( 10 );
        horizontalLayout->addWidget( m_spinBoxMaxHits );
        label->setBuddy( m_spinBoxMaxHits );
        horizontalLayout->addSpacing( KDialog::spacingHint() * 2 );

        m_pushButtonSearch = new QPushButton( i18n( "&Search" ), m_toolbarContainer );
        horizontalLayout->addWidget( m_pushButtonSearch );
        m_pushButtonSearch->setIconSet( QIconSet( SmallIcon( "find" ) ) );
        m_pushButtonSearch->setEnabled( FALSE );

        m_listViewResults = new QListView( this );
        m_listViewResults->addColumn( i18n( "Year" ), 64 );
        m_listViewResults->addColumn( i18n( "Author" ), 128 );
        m_listViewResults->addColumn( i18n( "Title" ), 512 );
        m_listViewResults->setAllColumnsShowFocus( TRUE );
        m_listViewResults->setSelectionMode( QListView::Extended );
        verticalLayout->addWidget( m_listViewResults );

        horizontalLayout = new QHBoxLayout( verticalLayout, KDialog::spacingHint() * 4 );
        m_disclaimerLabel = new KURLLabel( this );
        horizontalLayout->addWidget( m_disclaimerLabel );
        horizontalLayout->setStretchFactor( m_disclaimerLabel, 4 );
        m_checkBoxImportAll = new QCheckBox( i18n( "Import all hits" ), this );
        m_checkBoxImportAll->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        horizontalLayout->addWidget( m_checkBoxImportAll );
        horizontalLayout->setStretchFactor( m_checkBoxImportAll, 0 );

        m_lineEditQuery->setFocus();

        connect( m_disclaimerLabel, SIGNAL( leftClickedURL( const QString& ) ), kapp, SLOT( invokeBrowser( const QString& ) ) );
        connect( m_listViewResults, SIGNAL( selectionChanged( ) ), this, SLOT( importEnableChanging( ) ) );
        connect( m_listViewResults, SIGNAL( clicked( QListViewItem* ) ), this, SLOT( importEnableChanging( ) ) );
        connect( m_checkBoxImportAll, SIGNAL( toggled( bool ) ), this, SLOT( importEnableChanging( ) ) );
        connect( m_pushButtonSearch, SIGNAL( clicked() ), this, SLOT( startSearch() ) );
        connect( m_lineEditQuery, SIGNAL( returnPressed() ), this, SLOT( startSearch() ) );
        connect( m_lineEditQuery, SIGNAL( returnPressed( const QString& ) ), completionQuery, SLOT( addItem( const QString& ) ) );
    }

    ResultsListViewItem::ResultsListViewItem( QListView * parent, BibTeX::Entry * entry ) : QListViewItem( parent ), m_entry( entry )
    {
        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftTitle );
        if ( field != NULL && field->value() != NULL )
            setText( 2, field ->value() ->text().replace( '{', "" ).replace( '}', "" ).replace( '~', ' ' ) );
        field = entry->getField( BibTeX::EntryField::ftAuthor );
        if ( field != NULL && field->value() != NULL )
        {
            BibTeX::PersonContainer* personContainer = dynamic_cast<BibTeX::PersonContainer*>( field->value()->items.first() );
            if ( personContainer != NULL )
            {
                QStringList authors;
                QValueList<BibTeX::Person*> list = personContainer->persons;
                for ( QValueList<BibTeX::Person*>::ConstIterator it = list.begin(); it != list.end(); ++it )
                    authors.append(( *it ) ->text() );
                setText( 1, authors.join( " and " ).replace( '{', "" ).replace( '}', "" ).replace( '~', ' ' ) );
            }
            else setText( 1, field ->value() ->text().replace( '{', "" ).replace( '}', "" ).replace( '~', ' ' ) );
        }
        field = entry->getField( BibTeX::EntryField::ftYear );
        if ( field != NULL && field->value() != NULL )
            setText( 0, field ->value() ->text().replace( '{', "" ).replace( '}', "" ).replace( '~', ' ' ) );
    }

    ResultsListViewItem::~ResultsListViewItem()
    {
        if ( m_entry != NULL )
            delete m_entry;
    }

    BibTeX::Entry* ResultsListViewItem::entry()
    {
        return m_entry;
    }

}
#include "webquery.moc"
