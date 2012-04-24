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
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <entry.h>
#include <entryfield.h>
#include <file.h>
#include <settings.h>
#include <entrywidgettab.h>
#include <entrywidgettitle.h>
#include <entrywidgetauthor.h>
#include <entrywidgetkeyword.h>
#include <entrywidgetpublication.h>
#include <entrywidgetmisc.h>
#include <entrywidgetexternal.h>
#include <entrywidgetuserdefined.h>
#include <entrywidgetother.h>
#include <entrywidgetsource.h>
#include <entrywidgetwarningsitem.h>
#include <fieldlistview.h>
#include <idsuggestions.h>
#include "entrywidget.h"

namespace KBibTeX
{
    QDialog::DialogCode EntryWidget::execute( BibTeX::Entry *entry, BibTeX::File *bibtexfile, bool isReadOnly, bool isNew, QWidget *parent, const char *name )
    {
        EntryWidgetDialog *dlg = new EntryWidgetDialog( parent, name, TRUE, i18n( "Edit BibTeX Entry" ), KDialogBase::Ok | KDialogBase::Cancel );
        EntryWidget *entryWidget = new EntryWidget( entry, bibtexfile, isReadOnly, isNew, dlg, "entryWidget" );
        dlg->setMainWidget( entryWidget );

        QDialog::DialogCode result = ( QDialog::DialogCode ) dlg->exec();

        delete entryWidget;
        delete dlg;

        return result;
    }

    EntryWidget::EntryWidget( BibTeX::File *bibtexfile, bool isReadOnly, bool isNew, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_originalEntry( NULL ), m_bibtexfile( bibtexfile ), m_isReadOnly( isReadOnly ), m_isNew( isNew ), m_lastPage( NULL )
    {
        setupGUI( parent, FALSE );

        Settings * settings = Settings::self();
        m_checkBoxEnableAll->setChecked( settings->editing_EnableAllFields );
        m_defaultIdSuggestionAvailable = settings->idSuggestions_default >= 0;
        m_pushButtonForceDefaultIdSuggestion->setEnabled( !m_isReadOnly && m_defaultIdSuggestionAvailable );
        m_pushButtonIdSuggestions->setEnabled( !m_isReadOnly );
    }

    EntryWidget::EntryWidget( BibTeX::Entry *entry, BibTeX::File *bibtexfile, bool isReadOnly, bool isNew, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_originalEntry( entry ), m_bibtexfile( bibtexfile ), m_isReadOnly( isReadOnly ), m_isNew( isNew ), m_lastPage( NULL )
    {
        setupGUI( parent );

        Settings * settings = Settings::self();
        m_checkBoxEnableAll->setChecked( settings->editing_EnableAllFields );
        m_defaultIdSuggestionAvailable = settings->idSuggestions_default >= 0;
        m_pushButtonForceDefaultIdSuggestion->setEnabled( !m_isReadOnly && m_defaultIdSuggestionAvailable );
        m_pushButtonIdSuggestions->setEnabled( !m_isReadOnly );

        reset();
    }

    EntryWidget::~EntryWidget()
    {
        m_updateWarningsTimer->stop();
        delete m_updateWarningsTimer;
    }

    void EntryWidget::showEvent( QShowEvent *showev )
    {
        QWidget::showEvent( showev );
        EntryWidgetTitle *ewt = dynamic_cast<EntryWidgetTitle*>( m_tabWidget->page( 0 ) );
        if ( ewt != NULL )
            ewt->m_fieldLineEditTitle->setFocus();
    }

    void EntryWidget::closeEvent( QCloseEvent * e )
    {
        bool isModified = m_lineEditID->isModified();

        for ( QValueList<KBibTeX::EntryWidgetTab*>::iterator it( m_internalEntryWidgets.begin() ); !isModified && it != m_internalEntryWidgets.end(); ++it )
            isModified = ( *it ) ->isModified();
        isModified |= m_sourcePage->isModified();

        KGuiItem discardBtn = KGuiItem( i18n( "Discard" ), "editshred" );
        if ( isModified && KMessageBox::warningContinueCancel( this, i18n( "The current entry has been modified. Do you want do discard your changes?" ), i18n( "Discard changes" ), discardBtn ) == KMessageBox::Cancel )
            e->ignore();
        else
            e->accept();
    }

    void EntryWidget::apply()
    {
        if ( !m_isReadOnly )
            apply( m_originalEntry );
    }

    void EntryWidget::apply( BibTeX::Entry *entry )
    {
        internalApply( entry );
        if ( m_tabWidget->currentPage() == m_sourcePage )
            m_sourcePage->apply( entry );
        else
        {
            for ( QValueList<KBibTeX::EntryWidgetTab*>::iterator it( m_internalEntryWidgets.begin() ); it != m_internalEntryWidgets.end(); ++it )
                ( *it ) ->apply( entry );

            Settings * settings = Settings::self();
            settings->addToCompletion( entry );
        }
    }

    void EntryWidget::internalApply( BibTeX::Entry *entry )
    {
        BibTeX::Entry::EntryType entryType = BibTeX::Entry::entryTypeFromString( m_comboBoxEntryType->currentText() );
        if ( entryType == BibTeX::Entry::etUnknown )
            entry->setEntryTypeString( m_comboBoxEntryType->currentText() );
        else
            entry->setEntryType( entryType );

        QString id = m_lineEditID->text();
        entry->setId( id );
    }

    void EntryWidget::reset()
    {
        reset( m_originalEntry );
    }

    void EntryWidget::reset( BibTeX::Entry *entry )
    {
        internalReset( entry );
        m_sourcePage->reset( entry );
        for ( QValueList<KBibTeX::EntryWidgetTab*>::iterator it( m_internalEntryWidgets.begin() ); it != m_internalEntryWidgets.end(); ++it )
            ( *it ) ->reset( entry );

        updateGUI();
    }

    void EntryWidget::internalReset( BibTeX::Entry *entry )
    {
        m_lineEditID->setText( entry->id() );
        Settings * settings = Settings::self();
        m_pushButtonForceDefaultIdSuggestion->setOn( m_defaultIdSuggestionAvailable && settings->idSuggestions_forceDefault && m_isNew );
        m_pushButtonForceDefaultIdSuggestion->setEnabled( !m_isReadOnly && m_defaultIdSuggestionAvailable );

        bool foundEntryType = FALSE;
        for ( int i = 0; !foundEntryType && i < m_comboBoxEntryType->count(); i++ )
            if (( BibTeX::Entry::EntryType ) i + BibTeX::Entry::etArticle == entry->entryType() )
            {
                m_comboBoxEntryType->setCurrentItem( i );
                foundEntryType = TRUE;
            }
        if ( !foundEntryType )
            m_comboBoxEntryType->setCurrentText( entry->entryTypeString() );
    }

    void EntryWidget::slotEnableAllFields( )
    {
        updateGUI();
    }

    void EntryWidget::slotForceDefaultIdSuggestion()
    {
        m_isNew = FALSE;
        m_lineEditID->setEnabled( !m_pushButtonForceDefaultIdSuggestion->isOn() );
        m_pushButtonIdSuggestions->setEnabled( !m_pushButtonForceDefaultIdSuggestion->isOn() && !m_isReadOnly );
        updateWarnings();
    }

    void EntryWidget::slotEntryTypeChanged( )
    {
        updateGUI();
    }

    void EntryWidget::slotCurrentPageChanged( QWidget* newPage )
    {
        BibTeX::Entry temporaryEntry;

        if ( newPage == m_sourcePage )
        {
            // switching to source tab
            m_updateWarningsTimer->stop();
            internalApply( &temporaryEntry );
            for ( QValueList<KBibTeX::EntryWidgetTab*>::iterator it( m_internalEntryWidgets.begin() ); it != m_internalEntryWidgets.end(); ++it )
                ( *it ) ->apply( &temporaryEntry );
            m_sourcePage->reset( &temporaryEntry );

            m_comboBoxEntryType->setEnabled( FALSE );
            m_lineEditID->setEnabled( FALSE );
            m_pushButtonIdSuggestions->setEnabled( FALSE );
            m_pushButtonForceDefaultIdSuggestion->setEnabled( FALSE );
        }
        else if ( m_lastPage == m_sourcePage )
        {
            // switching from source tab away
            m_sourcePage->apply( &temporaryEntry );
            internalReset( &temporaryEntry );
            for ( QValueList<KBibTeX::EntryWidgetTab*>::iterator it( m_internalEntryWidgets.begin() ); it != m_internalEntryWidgets.end(); ++it )
                ( *it ) ->reset( &temporaryEntry );
            updateWarnings();

            m_comboBoxEntryType->setEnabled( TRUE );
            m_lineEditID->setEnabled( !m_defaultIdSuggestionAvailable || !m_pushButtonForceDefaultIdSuggestion->isOn() );
            m_pushButtonIdSuggestions->setEnabled(( !m_defaultIdSuggestionAvailable || !m_pushButtonForceDefaultIdSuggestion->isOn() ) && !m_isReadOnly );
            m_pushButtonForceDefaultIdSuggestion->setEnabled( !m_isReadOnly && m_defaultIdSuggestionAvailable );
            if ( !m_isReadOnly )
                m_updateWarningsTimer->start( 500 );
        }

        m_lastPage = newPage;
    }

    void EntryWidget::warningsExecute( QListViewItem* item )
    {
        EntryWidgetWarningsItem * ewwi = dynamic_cast<KBibTeX::EntryWidgetWarningsItem*>( item );
        if ( ewwi != NULL && ewwi->widget() != NULL )
        {
            ewwi->widget() ->setFocus();
            // find and activate corresponding tab page
            QObject *parent = ewwi->widget();
            KBibTeX::EntryWidgetTab *ewt = dynamic_cast<KBibTeX::EntryWidgetTab*>( parent );
            while ( ewt == NULL && parent != NULL )
            {
                parent = parent->parent();
                ewt = dynamic_cast<KBibTeX::EntryWidgetTab*>( parent );
            }
            m_tabWidget->setCurrentPage( m_tabWidget-> indexOf( ewt ) );
        }
    }

    void EntryWidget::setupGUI( QWidget *parent, bool showWarnings )
    {
        QGridLayout * layout = new QGridLayout( this, 4, 6, 0, KDialog::marginHint() );

        // in the top row on the left, put an entry type label and combobox
        QLabel *label = new QLabel( i18n( "E&ntry Type:" ), this );
        layout->addWidget( label, 0, 0 );
        m_comboBoxEntryType = new QComboBox( TRUE, this, "m_comboBoxEntryType" );
        label->setBuddy( m_comboBoxEntryType );
        m_comboBoxEntryType->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
        m_comboBoxEntryType->setEnabled( !m_isReadOnly );
        layout->addWidget( m_comboBoxEntryType, 0, 1 );
        setupEntryTypes();

        // in the top row on the left, put an identifier label and combobox
        label = new QLabel( i18n( "&Identifier" ), this );
        layout->addWidget( label, 0, 2 );
        m_lineEditID = new QLineEdit( this, "m_lineEditID" );
        label->setBuddy( m_lineEditID );
        m_lineEditID->setReadOnly( m_isReadOnly );
        m_lineEditID->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
        layout->addWidget( m_lineEditID, 0, 3 );

        m_pushButtonIdSuggestions = new QPushButton( QIconSet( BarIcon( "wizard" ) ), "", this, "m_pushButtonIdSuggestions" );
        m_menuIdSuggestions = new QPopupMenu( m_pushButtonIdSuggestions );
        connect( m_menuIdSuggestions, SIGNAL( activated( int ) ), this, SLOT( insertIdSuggestion( int ) ) );
        m_pushButtonIdSuggestions->setPopup( m_menuIdSuggestions );
        layout->addWidget( m_pushButtonIdSuggestions, 0, 4 );

        m_pushButtonForceDefaultIdSuggestion = new QPushButton( QIconSet( BarIcon( "favorites" ) ), "", this, "m_pushButtonForceDefaultIdSuggestion" );
        m_pushButtonForceDefaultIdSuggestion->setToggleButton( TRUE );
        layout->addWidget( m_pushButtonForceDefaultIdSuggestion, 0, 5 );
        QToolTip::add( m_pushButtonForceDefaultIdSuggestion, i18n( "Use the default id suggestion to set the entry id" ) );
        QWhatsThis::add( m_pushButtonForceDefaultIdSuggestion, i18n( "Use the default id suggestion to set the entry id.\nYou can edit and select the default id suggestion in the configuration dialog." ) );

        // central tab widget for all the tabs
        m_tabWidget = new QTabWidget( this );
        layout->addMultiCellWidget( m_tabWidget, 1, 1, 0, 5 );
        addTabWidgets();

        // a check box if the user want to edit all fields
        m_checkBoxEnableAll = new QCheckBox( i18n( "Enable all &fields for editing" ), this );
        layout->addMultiCellWidget( m_checkBoxEnableAll, 2, 2, 0, 5 );

        if ( showWarnings )
        {
            // list view for warnings, errors and suggestions
            m_listViewWarnings = new QListView( this );
            m_listViewWarnings->addColumn( i18n( "Message" ) );
            m_listViewWarnings->setAllColumnsShowFocus( true );
            layout->addMultiCellWidget( m_listViewWarnings, 3, 3, 0, 5 );
            connect( m_listViewWarnings, SIGNAL( doubleClicked( QListViewItem*, const QPoint&, int ) ), this, SLOT( warningsExecute( QListViewItem* ) ) );
        }
        else
            m_listViewWarnings = NULL;

        connect( m_checkBoxEnableAll, SIGNAL( toggled( bool ) ), this, SLOT( slotEnableAllFields( ) ) );
        connect( m_comboBoxEntryType, SIGNAL( activated( int ) ), this, SLOT( slotEntryTypeChanged( ) ) );
        connect( m_pushButtonForceDefaultIdSuggestion, SIGNAL( toggled( bool ) ), this, SLOT( slotForceDefaultIdSuggestion() ) );
        connect( m_comboBoxEntryType, SIGNAL( textChanged( const QString & ) ), this, SLOT( slotEntryTypeChanged() ) );
        connect( m_tabWidget, SIGNAL( currentChanged( QWidget* ) ), this, SLOT( slotCurrentPageChanged( QWidget* ) ) );
        connect( parent, SIGNAL( okClicked() ), this, SLOT( apply() ) );
        connect( m_menuIdSuggestions, SIGNAL( aboutToShow() ), this, SLOT( updateIdSuggestionsMenu() ) );

        m_updateWarningsTimer = new QTimer( this );
        connect( m_updateWarningsTimer, SIGNAL( timeout() ), this, SLOT( updateWarnings() ) );
        if ( !m_isReadOnly )
            m_updateWarningsTimer->start( 500 );
    }

    void EntryWidget::addTabWidgets()
    {
        addTabWidget( new KBibTeX::EntryWidgetTitle( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetTitle" ), i18n( "Title" ) );
        addTabWidget( new KBibTeX::EntryWidgetAuthor( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetAuthor" ), i18n( "Author/Editor" ) );
        addTabWidget( new KBibTeX::EntryWidgetPublication( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetPublication" ), i18n( "Publication" ) );
        addTabWidget( new KBibTeX::EntryWidgetMisc( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetMisc" ), i18n( "Misc" ) );
        addTabWidget( new KBibTeX::EntryWidgetKeyword( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetKeyword" ), i18n( "Keywords" ) );
        addTabWidget( new KBibTeX::EntryWidgetExternal( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetExternal" ), i18n( "External" ) );
        addTabWidget( new KBibTeX::EntryWidgetUserDefined( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetUserDefined" ), i18n( "User Defined" ) );
        addTabWidget( new KBibTeX::EntryWidgetOther( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetOther" ), i18n( "Other Fields" ) );

        m_sourcePage = new KBibTeX::EntryWidgetSource( m_bibtexfile, m_isReadOnly, m_tabWidget, "EntryWidgetSource" );
        m_tabWidget->insertTab( m_sourcePage, i18n( "Source" ) );
    }

    void EntryWidget::addTabWidget( EntryWidgetTab *widget, const QString& title )
    {
        m_tabWidget->insertTab( widget, title );
        m_internalEntryWidgets.append( widget );
    }

    void EntryWidget::setupEntryTypes()
    {
        int i = ( int ) BibTeX::Entry::etArticle;
        BibTeX::Entry::EntryType entryType = ( BibTeX::Entry::EntryType ) i;
        while ( entryType != BibTeX::Entry::etUnknown )
        {
            QString currentString = BibTeX::Entry::entryTypeToString( entryType );
            m_comboBoxEntryType->insertItem( currentString );
            entryType = ( BibTeX::Entry::EntryType ) ++i;
        }
    }

    void EntryWidget::updateGUI()
    {
        BibTeX::Entry::EntryType entryType = BibTeX::Entry::entryTypeFromString( m_comboBoxEntryType->currentText() );
        for ( QValueList<KBibTeX::EntryWidgetTab*>::iterator it( m_internalEntryWidgets.begin() ); it != m_internalEntryWidgets.end(); ++it )
            ( *it ) ->updateGUI( entryType, m_checkBoxEnableAll->isChecked() );
        updateWarnings( );
    }

    void EntryWidget::updateWarnings()
    {
        if ( m_listViewWarnings == NULL )
            return;

        m_listViewWarnings->clear();

        int p = 0;
        if ( m_defaultIdSuggestionAvailable && m_pushButtonForceDefaultIdSuggestion->isOn() )
        {
            BibTeX::Entry temporaryEntry;
            apply( &temporaryEntry );
            QString id = IdSuggestions::createDefaultSuggestion( m_bibtexfile, &temporaryEntry );
            if ( id.isNull() || id.isEmpty() )
                new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlWarning, i18n( "Please supply more fields to use the default id" ), m_pushButtonForceDefaultIdSuggestion, m_listViewWarnings );
            else
            {
                QString conflictFreeId = id;
                int nr = 0;
                while ( m_bibtexfile->containsKey( conflictFreeId ) != NULL )
                    conflictFreeId = QString( "%1-%2" ).arg( id ).arg( ++nr );

                new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlInformation, QString( i18n( "Using '%1' as entry id" ) ).arg( conflictFreeId ), m_pushButtonForceDefaultIdSuggestion, m_listViewWarnings );
                m_lineEditID->setText( conflictFreeId );
            }
        }
        else if ( m_lineEditID->text().isEmpty() )
            new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlError, i18n( "An entry has to have an identifier" ), m_lineEditID, m_listViewWarnings );
        else if (( p = m_lineEditID->text().find( QRegExp( "[^-.:/+_a-zA-Z0-9]" ) ) ) > 0 )
            new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlError, QString( i18n( "The identifier contains invalid characters at position %1" ) ).arg( p + 1 ), m_lineEditID, m_listViewWarnings );

        BibTeX::Entry::EntryType entryType = BibTeX::Entry::entryTypeFromString( m_comboBoxEntryType->currentText() );
        for ( QValueList<KBibTeX::EntryWidgetTab*>::iterator it( m_internalEntryWidgets.begin() ); it != m_internalEntryWidgets.end(); ++it )
            ( *it ) ->updateWarnings( entryType, m_listViewWarnings );

        QString text = m_lineEditID->text();
        for ( unsigned int i = 0; i < text.length(); i++ )
            if ( text.at( i ).unicode() > 127 )
            {
                new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlWarning, QString( i18n( "The identifier contains non-ascii characters, first one is '%1'" ) ).arg( text.at( i ) ), m_lineEditID, m_listViewWarnings );
                break;
            }

        BibTeX::Entry temporaryEntry;
        /** fetch data from GUI elements */
        for ( QValueList<KBibTeX::EntryWidgetTab*>::iterator it( m_internalEntryWidgets.begin() ); it != m_internalEntryWidgets.end(); ++it )
            ( *it ) ->apply( &temporaryEntry );
        internalApply( &temporaryEntry );

        BibTeX::Entry *crossRefEntry = NULL;
        QString crossRefText = "";
        BibTeX::EntryField *crossRef = temporaryEntry.getField( BibTeX::EntryField::ftCrossRef );
        if ( crossRef != NULL )
        {
            crossRefText = crossRef->value()->text();
            crossRefEntry = dynamic_cast<BibTeX::Entry*>( m_bibtexfile->containsKey( crossRefText ) );
        }

        switch ( temporaryEntry.entryType() )
        {
        case BibTeX::Entry::etProceedings:
            if ( temporaryEntry.getField( BibTeX::EntryField::ftEditor ) == NULL && temporaryEntry.getField( BibTeX::EntryField::ftOrganization ) == NULL && temporaryEntry.getField( BibTeX::EntryField::ftKey ) == NULL )
            {
                new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlWarning, i18n( "Require either 'Editor', 'Organization', or 'Key'" ), NULL, m_listViewWarnings );
            }
            break;
        case BibTeX::Entry::etInProceedings:
            {
                if ( crossRefEntry != NULL )
                {
                    if ( crossRefEntry->getField( BibTeX::EntryField::ftEditor ) == NULL && crossRefEntry->getField( BibTeX::EntryField::ftKey ) == NULL && crossRefEntry->getField( BibTeX::EntryField::ftBookTitle ) == NULL )
                    {
                        new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlWarning, QString( i18n( "Cross referenced entry '%1' must contain either 'Editor', 'Key', or 'Book Title'" ) ).arg( crossRefText ), NULL, m_listViewWarnings );
                    }
                }
            }
            break;
        case BibTeX::Entry::etInBook:
            {
                if ( crossRefEntry != NULL )
                {
                    if ( crossRefEntry->getField( BibTeX::EntryField::ftVolume ) == NULL )
                        new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlWarning, QString( i18n( "Cross referenced entry '%1' must contain 'Volume'" ) ).arg( crossRefText ), NULL, m_listViewWarnings );
                    if ( crossRefEntry->getField( BibTeX::EntryField::ftEditor ) == NULL && crossRefEntry->getField( BibTeX::EntryField::ftKey ) == NULL && crossRefEntry->getField( BibTeX::EntryField::ftSeries ) == NULL )
                    {
                        new KBibTeX::EntryWidgetWarningsItem( KBibTeX::EntryWidgetWarningsItem::wlWarning, QString( i18n( "Cross referenced entry '%1' must contain either 'Editor', 'Key', or 'Series'" ) ).arg( crossRefText ), NULL, m_listViewWarnings );
                    }
                }
            }
            break;
        default:
            {
// nothing
            }
        }
    }

    void EntryWidget::updateIdSuggestionsMenu()
    {
        BibTeX::Entry temporaryEntry;
        m_menuIdSuggestions->clear();
        m_idToSuggestion.clear();

        apply( &temporaryEntry );
        QStringList suggestions = IdSuggestions::createSuggestions( m_bibtexfile, &temporaryEntry );
        for ( QStringList::ConstIterator it = suggestions.begin(); it != suggestions.end(); ++it )
            m_idToSuggestion.insert( m_menuIdSuggestions->insertItem( *it ), *it );

        if ( m_idToSuggestion.count() == 0 )
            m_menuIdSuggestions->setItemEnabled( m_menuIdSuggestions->insertItem( i18n( "No suggestions available" ) ), FALSE );
    }

    void EntryWidget::insertIdSuggestion( int id )
    {
        m_lineEditID->setText( m_idToSuggestion[id] );
    }
}
#include "entrywidget.moc"
