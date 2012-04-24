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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qevent.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qvaluelist.h>
#include <qcursor.h>
#include <qbuffer.h>
#include <qlistview.h>
#include <qclipboard.h>
#include <qheader.h>
#include <qtextstream.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <klocale.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kactionclasses.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kiconloader.h>
#include <krun.h>

#include <kbibtex_part.h>
#include <documentlistviewitem.h>
#include <file.h>
#ifdef HAVE_LIBBIBUTILS
#include <fileimporterbibutils.h>
#else // HAVE_LIBBIBUTILS
#include <fileimporterris.h>
#endif // HAVE_LIBBIBUTILS
#include <fileimporterbibtex.h>
#include <fileexporterbibtex.h>
#include <element.h>
#include <entry.h>
#include <macro.h>
#include <comment.h>
#include <preamblewidget.h>
#include <preamble.h>
#include <entrywidget.h>
#include <commentwidget.h>
#include <macrowidget.h>
#include <settings.h>
#include <encoderlatex.h>

#include "documentlistview.h"

namespace KBibTeX
{
    DocumentListView::DocumentListView( KBibTeX::DocumentWidget *docWidget, bool isReadOnly, QWidget *parent, const char *name )
            : KListView( parent, name ), m_docWidget( docWidget ), m_bibtexFile( NULL ), m_contextMenu( NULL ), m_headerMenu( NULL ), m_isReadOnly( isReadOnly ), m_newElementCounter( 1 )
    {
        setAllColumnsShowFocus( true );
        setShowSortIndicator( true );
        setSelectionMode( QListView::Extended );
        header() ->setClickEnabled( TRUE );
        header() ->setMovingEnabled( TRUE );
        buildColumns();

        setDragEnabled( true );
        //         setDragAutoScroll( true );
        setAcceptDrops( TRUE );
        setDropVisualizer( TRUE );

        connect( header(), SIGNAL( clicked( int ) ), this, SLOT( setSortingColumn( int ) ) );
        connect( this, SIGNAL( contextMenu( KListView *, QListViewItem *, const QPoint & ) ), this, SLOT( showBibtexListContextMenu( KListView *, QListViewItem *, const QPoint & ) ) );
        connect( this, SIGNAL( doubleClicked( QListViewItem*, const QPoint&, int ) ), this, SLOT( slotDoubleClick( QListViewItem* ) ) );
        connect( this, SIGNAL( dropped( QDropEvent*, QListViewItem* ) ), this, SLOT( slotDropped( QDropEvent*, QListViewItem* ) ) );
    }

    DocumentListView::~DocumentListView()
    {
        // nothing
    }

    void DocumentListView::setFactory( KXMLGUIFactory *factory, KXMLGUIClient *client )
    {
        m_contextMenu = static_cast<KPopupMenu*>( factory -> container( "popup_bibtexlist", client ) );
    }

    bool DocumentListView::setBibTeXFile( BibTeX::File *bibtexFile )
    {
        m_bibtexFile = bibtexFile;
        setItems();

        return TRUE;
    }

    BibTeX::File* DocumentListView::getBibTeXFile( )
    {
        return m_bibtexFile;
    }

    void DocumentListView::setItems()
    {
        QApplication::setOverrideCursor( Qt::waitCursor );

        KProgressDialog *prgDlg = new KProgressDialog( this, "prgDlg", i18n( "Updating" ), i18n( "Updating main view" ), TRUE );
        prgDlg->show();
        KProgress *progress = prgDlg->progressBar();
        progress->setTotalSteps( m_bibtexFile->count() );

        bool update = viewport()->isUpdatesEnabled();
        viewport()->setUpdatesEnabled( FALSE );

        if ( m_bibtexFile->count() > 250 )
            qDebug( "This BibTeX source code contains many elements (%d) which thwarts KBibTeX when updating the main list.", m_bibtexFile->count() );

        clear();
        for ( unsigned int i = 0; i < m_bibtexFile->count(); i++ )
        {
            BibTeX::Element *element = m_bibtexFile->at( i );
            new DocumentListViewItem( m_bibtexFile, element, this );
            progress->setProgress( i );

            if ( i % 43 == 23 )
                kapp->processEvents();
        }

        viewport()->setUpdatesEnabled( update );
        triggerUpdate();
        delete prgDlg;

        updateVisiblity();

        QApplication::restoreOverrideCursor();
    }

    void DocumentListView::insertItems( BibTeX::File *items, KBibTeX::DocumentListViewItem *after )
    {
        for ( BibTeX::File::ElementList::iterator it = items->begin(); it != items->end(); it++ )
            after = insertItem( *it, after );
    }

    KBibTeX::DocumentListViewItem * DocumentListView::insertItem( BibTeX::Element *item, KBibTeX::DocumentListViewItem *after )
    {
        if ( m_bibtexFile == NULL )
            m_bibtexFile = new BibTeX::File();

        BibTeX::Element *element = NULL;
        BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry *>( item );
        if ( entry != NULL )
        {
            BibTeX::Entry *newEntry = new BibTeX::Entry( entry );
            if ( m_bibtexFile->containsKey( newEntry->id() ) )
            {
                int counter = 0;
                QString newId = QString( newEntry->id() ).append( '_' ).append( QString::number( ++counter ) );
                while ( m_bibtexFile->containsKey( newId ) )
                    newId = QString( newEntry->id() ).append( '_' ).append( QString::number( ++counter ) );
                newEntry->setId( newId );
            }
            element = newEntry;
        }
        else
            element = item->clone();


        m_bibtexFile->appendElement( element, after == NULL ? NULL : after->element() );
        after = new DocumentListViewItem( m_bibtexFile, element, this, after );
        after->setUnreadStatus( TRUE );
        updateVisiblity( after );
        m_unreadItems.append( after );

        QTimer::singleShot( 3500, this, SLOT( makeNewItemsUnread() ) );

        return after;
    }

    void DocumentListView::insertItem( BibTeX::Element *item )
    {
        insertItem( item, NULL );
    }

    void DocumentListView::updateVisiblity()
    {
        QListViewItemIterator it( this );
        while ( it.current() )
        {
            DocumentListViewItem * kblvi = dynamic_cast<DocumentListViewItem*>( it.current() );
            updateVisiblity( kblvi );
            it++;
        }
    }

    void DocumentListView::updateVisiblity( KBibTeX::DocumentListViewItem *item )
    {
        Settings * settings = Settings::self( m_bibtexFile );
        BibTeX::Element *element = item->element();
        bool notFiltered = m_filter.isEmpty() || element ->containsPattern( m_filter, m_filterFieldType, m_filterType );

        BibTeX::Macro *macro = dynamic_cast<BibTeX::Macro*>( element );
        if ( macro != NULL )
            item->setVisible( notFiltered && settings->editing_ShowMacros );
        else
        {
            BibTeX::Comment *comment = dynamic_cast<BibTeX::Comment*>( element );
            if ( comment != NULL )
                item->setVisible( notFiltered && settings->editing_ShowComments );
            else
                item->setVisible( notFiltered );
        }
    }

    void DocumentListView::deferredInitialization()
    {
        connect( header(), SIGNAL( sizeChange( int, int, int ) ), this, SLOT( saveColumnWidths() ) );
        connect( header(), SIGNAL( indexChange( int, int, int ) ), this, SLOT( saveColumnIndex() ) );
    }

    void DocumentListView::restoreState()
    {
        Settings * settings = Settings::self( m_bibtexFile );
        if ( settings->editing_UseSpecialFont )
            setFont( settings->editing_SpecialFont );
        else
            setFont( KGlobalSettings::generalFont() );
        header() ->setFont( KGlobalSettings::generalFont() );

        restoreColumnIndex();
        restoreColumnWidths();
        restoreSortingColumn();
    }

    void DocumentListView::setViewShowColumnsMenu( KActionMenu *actionMenu )
    {
        if ( m_headerMenu == NULL )
        {
            m_headerMenu = actionMenu->popupMenu();
            m_headerMenu->insertTitle( i18n( "Show Columns" ) );
            m_headerMenu->setCheckable( TRUE );
            connect( m_headerMenu, SIGNAL( activated( int ) ), this, SLOT( activateShowColumnMenu( int ) ) );

            Settings * settings = Settings::self( m_bibtexFile );

            int item = m_headerMenu->insertItem( i18n( "Element Type" ), 0 );
            m_headerMenu->setItemChecked( item, settings->editing_MainListColumnsWidth[ 0 ] > 0 );
            m_headerMenu->insertSeparator();

            for ( int i = 0; i <= ( int ) BibTeX::EntryField::ftYear - ( int ) BibTeX::EntryField::ftAbstract; i++ )
            {
                BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType )( i + ( int ) BibTeX::EntryField::ftAbstract );
                QString label = Settings::fieldTypeToI18NString( fieldType );
                item = m_headerMenu->insertItem( label, ( int ) fieldType + 2 );
                m_headerMenu->setItemChecked( item, settings->editing_MainListColumnsWidth[ i + 2 ] > 0 );
            }
        }
    }

    void DocumentListView::deleteSelected()
    {
        QListViewItemIterator it( this, QListViewItemIterator::Selected | QListViewItemIterator::Visible );
        if ( it.current() == NULL ) return;

        QListViewItem *above = it.current() ->itemAbove();
        QValueList<DocumentListViewItem*> toBeDeleted;
        while ( it.current() )
        {
            DocumentListViewItem * kblvi = dynamic_cast<DocumentListViewItem*>( it.current() );
            toBeDeleted.append( kblvi );
            it++;
        }

        for ( QValueList<DocumentListViewItem*>::Iterator it = toBeDeleted.begin(); it != toBeDeleted.end(); ++it )
        {
            m_bibtexFile->deleteElement(( *it )->element() );
            takeItem( *it );
            delete( *it );
        }

        if ( above )
            ensureItemVisible( above );

        emit modified();
    }

    const QValueList<BibTeX::Element*> DocumentListView::selectedItems()
    {
        QValueList<BibTeX::Element*> result;

        QListViewItemIterator it( this, QListViewItemIterator::Selected );
        while ( it.current() )
        {
            DocumentListViewItem * kblvi = dynamic_cast<DocumentListViewItem*>( it.current() );
            if ( kblvi->isVisible() )
                result.append( kblvi->element() );
            it++;
        }

        return result;
    }

    QString DocumentListView::selectedToBibTeXText()
    {
        BibTeX::FileExporterBibTeX *exporter = new BibTeX::FileExporterBibTeX();
        exporter->setEncoding( BibTeX::File::encLaTeX );

        QBuffer buffer;
        buffer.open( IO_WriteOnly );
        QValueList<BibTeX::Element*> selectedElements = selectedItems();
        for ( QValueList<BibTeX::Element*>::iterator it = selectedElements.begin(); it != selectedElements.end(); ++it )
            exporter->save( &buffer, *it );
        buffer.close();
        delete exporter;

        buffer.open( IO_ReadOnly );
        QTextStream in( &buffer );
        in.setEncoding( QTextStream::UnicodeUTF8 );
        QString result = in.read();
        buffer.close();

        return result;
    }

    QString DocumentListView::selectedToBibTeXRefs()
    {
        QString refs;
        QValueList<BibTeX::Element*> selectedElements = selectedItems();
        for ( QValueList<BibTeX::Element*>::iterator it = selectedElements.begin(); it != selectedElements.end(); ++it )
        {
            BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *it );
            if ( entry == NULL ) continue;

            if ( !refs.isEmpty() )
                refs.append( "," );
            refs.append( entry->id() );
        }
        return QString( "\\cite{%1}" ).arg( refs );
    }

    void DocumentListView::copy()
    {
        kapp->clipboard() ->setText( selectedToBibTeXText() );
    }

    void DocumentListView::copyReferences()
    {
        kapp->clipboard() ->setText( selectedToBibTeXRefs() );
    }

    void DocumentListView::cut()
    {
        copy();
        deleteSelected();
    }

    bool DocumentListView::paste()
    {
        DocumentListViewItem * dlvi =  dynamic_cast<KBibTeX::DocumentListViewItem *>( selectedItem() );
        if ( dlvi == NULL )
            dlvi = dynamic_cast<KBibTeX::DocumentListViewItem *>( currentItem() );

        QString clipboardText = kapp->clipboard() ->text();

        /** check if clipboard contains BibTeX content */
        if ( BibTeX::FileImporterBibTeX::guessCanDecode( clipboardText ) )
        {
            Settings * settings = Settings::self( m_bibtexFile );
            BibTeX::FileImporter *importer = new BibTeX::FileImporterBibTeX( settings->editing_FirstNameFirst );
            BibTeX::File *clipboardData = importer->load( clipboardText );
            delete importer;

            if ( clipboardData != NULL )
            {
                insertItems( clipboardData, dlvi );
                delete clipboardData;
                return TRUE;
            }
            else
                return FALSE;
        }
#ifdef HAVE_LIBBIBUTILS
        else if ( BibTeX::FileImporterBibUtils::guessCanDecode( clipboardText ) )
        {
            BibTeX::FileImporterBibUtils::InputFormat inputFormat = BibTeX::FileImporterBibUtils::guessInputFormat( clipboardText );
            BibTeX::FileImporter *importer = new BibTeX::FileImporterBibUtils( inputFormat );
            BibTeX::File *clipboardData = importer->load( clipboardText );
            delete importer;

            if ( clipboardData != NULL )
            {
                insertItems( clipboardData, dlvi );
                delete clipboardData;
                return TRUE;
            }
            else
                return FALSE;
        }
#else // HAVE_LIBBIBUTILS
        else if ( BibTeX::FileImporterRIS::guessCanDecode( clipboardText ) )
        {
            BibTeX::FileImporter *importer = new BibTeX::FileImporterRIS( );
            BibTeX::File *clipboardData = importer->load( clipboardText );
            delete importer;

            if ( clipboardData != NULL )
            {
                insertItems( clipboardData, dlvi );
                delete clipboardData;
                return TRUE;
            }
            else
                return FALSE;
        }
#endif // HAVE_LIBBIBUTILS
        else
        {
            /** Decoding the paste text as bibtex failed. Maybe the user wants
                to paste the text as link address, abstract, etc... */
            if ( !dlvi ) // no list view item selected to add data to
                return FALSE;

            // fetch BibTeX element from current list view item
            BibTeX::Entry * element = dynamic_cast<BibTeX::Entry*>( dlvi->element() );
            if ( ! element )
                return FALSE;

            // build popup menu
            KPopupMenu * popup = new KPopupMenu( this, "pastePopup" );
            popup->insertTitle( i18n( "Paste text as..." ) );
            for ( int i = ( int ) BibTeX::EntryField::ftAuthor; i <= ( int ) BibTeX::EntryField::ftYear; i++ )
            {
                BibTeX::EntryField::FieldType ft = ( BibTeX::EntryField::FieldType ) i;
                popup->insertItem( Settings::fieldTypeToI18NString( ft ), i );
            }
            popup->insertSeparator();
            QIconSet cancelPixmap = KGlobal::iconLoader() ->loadIconSet( "cancel", KIcon::Small );
            int cancelId = popup->insertItem( cancelPixmap, i18n( "Cancel" ) );

            // show popup menu
            int selectedId = popup->exec( QCursor::pos() );
            if ( selectedId == cancelId || selectedId == -1 )
                return FALSE; // cancel menu

            // determine field to add clipboard value to
            BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType ) selectedId;
            BibTeX::EntryField * field = element->getField( fieldType );
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( fieldType );
                element->addField( field );
            }
            else if ( field->value() != NULL )
                delete field->value();

            clipboardText = BibTeX::EncoderLaTeX::currentEncoderLaTeX() ->encode( clipboardText );

            // create new value from clipboard's content
            BibTeX::Value * value = new BibTeX::Value();
            if ( fieldType == BibTeX::EntryField::ftAuthor || fieldType == BibTeX::EntryField::ftEditor )
            {
                Settings * settings = Settings::self( m_bibtexFile );
                value->items.append( new BibTeX::PersonContainer( clipboardText, settings->editing_FirstNameFirst ) );
            }
            else if ( fieldType == BibTeX::EntryField::ftKeywords )
                value->items.append( new BibTeX::KeywordContainer( clipboardText ) );
            else
                value->items.append( new BibTeX::PlainText( clipboardText ) );

            field->setValue( value );

            return TRUE;
        }
    }

    void DocumentListView::selectAll()
    {
        QListView::selectAll( true );
    }

    /*        void DocumentListView::sendSelectedToLyx()
        {
            QStringList refsToSend;
            QListViewItemIterator it( this, QListViewItemIterator::Selected );
            while ( it.current() )
            {
                DocumentListViewItem * kblvi = dynamic_cast<DocumentListViewItem*>( it.current() );
                BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( kblvi->element() );
                if ( entry != NULL && kblvi->isVisible() )
                    refsToSend.append( entry->id() );
                it++;
            }

            Settings * settings = Settings::self( m_bibtexFile );
            QString lyxPipeFilename = settings->detectLyXInPipe();
            kdDebug() << "sendSelectedToLyx: lyxPipeFilename= " << lyxPipeFilename << endl;
            QFile pipe( lyxPipeFilename );
            if ( pipe.exists() && pipe.open( IO_WriteOnly ) )
            {
                QTextStream * writer = new QTextStream( &pipe );
                QString msg = "LYXCMD:kbibtex:citation-insert:" + refsToSend.join( "," );
                *writer << msg << endl;
                delete writer;
                pipe.close();
            }
            else
                KMessageBox::error( this, ( lyxPipeFilename.isEmpty() ? i18n( "Cannot establish a link to LyX" ) : QString( i18n( "Cannot establish a link to LyX using the pipe \"%1\"" ) ).arg( lyxPipeFilename ) ) + i18n( "\nMaybe LyX is not running?" ), i18n( "Error communicating with LyX" ) );
        }*/

    void DocumentListView::slotDoubleClick( QListViewItem *item )
    {
        DocumentListViewItem *dlvi = dynamic_cast<DocumentListViewItem*>( item );
        if ( dlvi != NULL ) emit executed( dlvi );
    }

    void DocumentListView::filter( const QString & text, BibTeX::Element::FilterType filterType, BibTeX::EntryField::FieldType fieldType )
    {
        m_filter = text;
        m_filterType = filterType;
        m_filterFieldType = fieldType;
        updateVisiblity();
    }

    void DocumentListView::setReadOnly( bool isReadOnly )
    {
        m_isReadOnly = isReadOnly;
    }

    void DocumentListView::activateShowColumnMenu( int id )
    {
        if ( id >= 0 )
            if ( columnWidth( id ) > 0 )
            {
                hideColumn( id );
                m_headerMenu->setItemChecked( id, FALSE );
            }
            else
            {
                showColumn( id );
                m_headerMenu->setItemChecked( id, TRUE );
            }
    }

    void DocumentListView::showBibtexListContextMenu( KListView *, QListViewItem *, const QPoint & p )
    {
        if ( m_contextMenu != NULL )
        {
            emit selectionChanged();
            m_contextMenu->popup( p );
        }
    }

    void DocumentListView::setSortingColumn( int column )
    {
        Settings * settings = Settings::self( m_bibtexFile );
        settings->editing_MainListSortingColumn = column;
        settings->editing_MainListSortingOrder = ( sortOrder() == Qt::Ascending ) ? 1 : -1;
    }

    bool DocumentListView::acceptDrag( QDropEvent * event ) const
    {
        if ( event->source() == this ) return false;
        return QTextDrag::canDecode( event ) || QUriDrag::canDecode( event );
    }

    void DocumentListView::startDrag()
    {
        Settings * settings = Settings::self( m_bibtexFile );
        QDragObject *d = new QTextDrag( settings->editing_DragAction == Settings::COPYREFERENCE ? selectedToBibTeXRefs() : selectedToBibTeXText(), this );
        d->dragCopy();
    }
    void DocumentListView::saveColumnIndex()
    {
        Settings * settings = Settings::self( m_bibtexFile );
        QHeader *hdr = header();

        for ( int i = 0; i < columns(); i++ )
            settings->editing_MainListColumnsIndex[ i ] = hdr->mapToIndex( i );
    }

    void DocumentListView::restoreColumnIndex()
    {
        Settings * settings = Settings::self( m_bibtexFile );
        QHeader *hdr = header();

        for ( int i = 0; i < columns(); i++ )
            hdr->moveSection( i, settings->editing_MainListColumnsIndex[ i ] );
    }

    void DocumentListView::saveColumnWidths( int col )
    {
        Settings * settings = Settings::self( m_bibtexFile );

        int from = col == -1 ? 0 : col, to = col == -1 ? columns() : ( col + 1 );

        for ( int i = from; i < to; i++ )
        {
            if ( columnWidthMode( i ) == QListView::Manual )
                settings->editing_MainListColumnsWidth[ i ] = columnWidth( i );
            else
                settings->editing_MainListColumnsWidth[ i ] = 0xffff;
        }
    }

    void DocumentListView::restoreColumnWidths()
    {
        Settings * settings = Settings::self( m_bibtexFile );

        for ( int col = 0; col < columns(); col++ )
        {
            int colWidth = settings->editing_MainListColumnsWidth[ col ];
            showColumn( col, colWidth );
        }
    }

    void DocumentListView::restoreSortingColumn()
    {
        Settings * settings = Settings::self( m_bibtexFile );
        setSortColumn( settings->editing_MainListSortingColumn );
        setSortOrder( settings->editing_MainListSortingOrder > 0 ? Qt::Ascending : Qt::Descending );
    }

    void DocumentListView::makeNewItemsUnread()
    {
        for ( QValueList<DocumentListViewItem*>::ConstIterator it = m_unreadItems.begin() ; it != m_unreadItems.end(); ++it )
        {
            ( *it ) ->setUnreadStatus( FALSE );
            ( *it ) ->repaint();
        }

        m_unreadItems.clear();
    }

    void DocumentListView::slotDropped( QDropEvent * event, QListViewItem * item )
    {
        QString text;
        QStrList urlList;

        if ( QTextDrag::decode( event, text ) && KURL( text ).isValid() )
            urlList.append( text );

        if ( !urlList.isEmpty() || QUriDrag::decode( event, urlList ) )
        {
            QString url = urlList.at( 0 );
            QString tmpFile;
            if ( ! KIO::NetAccess::download( url, tmpFile, 0 ) )
            {
                KMessageBox::error( this, KIO::NetAccess::lastErrorString() );
                return ;
            }
            QFile f( tmpFile );
            if ( ! f.open( IO_ReadOnly ) )
            {
                KMessageBox::error( this, f.errorString() );
                KIO::NetAccess::removeTempFile( tmpFile );
                return ;
            }
            QByteArray ba = f.readAll();
            text = QString( ba );
            f.close();
            KIO::NetAccess::removeTempFile( tmpFile );
        }
        else if ( !QTextDrag::decode( event, text ) )
            return;

        event->accept( TRUE );
        DocumentListViewItem * dlvi = dynamic_cast<KBibTeX::DocumentListViewItem *>( item );
        BibTeX::FileImporter *importer = NULL;
        if ( BibTeX::FileImporterBibTeX::guessCanDecode( text ) )
        {
            Settings * settings = Settings::self( m_bibtexFile );
            importer = new BibTeX::FileImporterBibTeX( settings->editing_FirstNameFirst );
        }
        //        else if (BibTeX::FileImporterExternal::guessCanDecode( text ))
        //            importer = new BibTeX::FileImporterExternal();
        else
            return ;

        BibTeX::File *droppedData = importer->load( text );
        delete importer;
        if ( droppedData != NULL )
            insertItems( droppedData, dlvi );
    }

    // void DocumentListView::contentsDragEnterEvent( QDragEnterEvent * event )
    // {
    //     KListView::contentsDragEnterEvent( event );
    //     qDebug( "contentsDragEnterEvent" );
    //     event->accept( FALSE );
    // }

    // void DocumentListView::contentsDropEvent( QDropEvent * event )
    // {
    //     KListView::contentsDropEvent( event );
    //     //     dropEvent( event );
    //     qDebug( "ContentsDropEvent" );
    //     event->accept( FALSE );
    // }

    // void DocumentListView::startDrag()
    // {
    //     QDragObject * d = new QTextDrag( selectedAsString(), this );
    //     d->dragCopy();
    //     // do NOT delete d.
    // }

    bool DocumentListView::eventFilter( QObject * watched, QEvent * e )
    {
        if ( watched == header() )
        {
            switch ( e->type() )
            {
            case QEvent::MouseButtonPress:
                {
                    if ( static_cast<QMouseEvent *>( e ) ->button() == RightButton && m_headerMenu != NULL )
                        m_headerMenu->popup( QCursor::pos() );

                    break;
                }

            default:
                break;
            }
        }

        return KListView::eventFilter( watched, e );
    }

    void DocumentListView::keyPressEvent( QKeyEvent *e )
    {
        if ( e->key() == QKeyEvent::Key_Enter || e->key() == QKeyEvent::Key_Return )
        {
            DocumentListViewItem *item = dynamic_cast<DocumentListViewItem*>( selectedItem() );
            if ( item == NULL )
                item = dynamic_cast<DocumentListViewItem*>( currentItem() );
            if ( item != NULL )
                emit executed( item );
        }
        else
            KListView::keyPressEvent( e );
    }

    void DocumentListView::showColumn( int col, int colWidth )
    {
        if ( colWidth == 0xffff )
        {
            adjustColumn( col );
            if ( columnWidth( col ) > width() / 3 )
                colWidth = width() / 4;
            if ( columnWidth( col ) < width() / 12 )
                colWidth = width() / 8;
        }

        if ( colWidth < 0xffff )
            setColumnWidth( col, colWidth );

        header() ->setResizeEnabled( colWidth > 0, col );
        setColumnWidthMode( col, colWidth < 0xffff ? QListView::Manual : QListView::Maximum );
        saveColumnWidths( col );
    }

    void DocumentListView::hideColumn( int col )
    {
        showColumn( col, 0 );
    }

    void DocumentListView::buildColumns()
    {
        addColumn( i18n( "Element Type" ) );
        addColumn( i18n( "Entry Id" ) );

        for ( int i = 0; i <= ( int ) BibTeX::EntryField::ftYear - ( int ) BibTeX::EntryField::ftAbstract; i++ )
        {
            BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType )( i + ( int ) BibTeX::EntryField::ftAbstract );
            QString label = Settings::fieldTypeToI18NString( fieldType );
            addColumn( label );
        }
    }
}

#include "documentlistview.moc"
