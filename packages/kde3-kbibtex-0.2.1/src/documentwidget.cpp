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
#include <cmath>

#include <qlayout.h>
#include <qfile.h>
#include <qdragobject.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qurl.h>
#include <qtextedit.h>
#include <qprogressdialog.h>
#include <qbuffer.h>
#include <qtooltip.h>
#include <qapplication.h>
#include <qpushbutton.h>

#include <krun.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kglobalsettings.h>
#include <kurl.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klineedit.h>
#include <kdebug.h>
#include <ktextedit.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <searchbar.h>
#include <sidebar.h>
#include <documentlistview.h>
#include <documentlistviewitem.h>
#include <documentsourceview.h>
#include <macrowidget.h>
#include <preamblewidget.h>
#include <commentwidget.h>
#include <entrywidget.h>
#include <fileimporterbibtex.h>
#ifdef HAVE_LIBBIBUTILS
#include <fileimporterbibutils.h>
#else // HAVE_LIBBIBUTILS
#include <fileimporterris.h>
#endif // HAVE_LIBBIBUTILS
#include <fileexporterbibtex.h>
#ifdef HAVE_LIBBIBUTILS
#include <fileexporterbibutils.h>
#else // HAVE_LIBBIBUTILS
#include <fileexporterris.h>
#endif // HAVE_LIBBIBUTILS
#include <fileexporterxml.h>
#include <fileexporterrtf.h>
#include <fileexporterpdf.h>
#include <fileexporterps.h>
#include <fileexporterxslt.h>
#include <fileimporterexternal.h>
#include <fileexporterexternal.h>
#include <xsltransform.h>
#include <entry.h>
#include <entryfield.h>
#include <comment.h>
#include <macro.h>
#include <preamble.h>
#include <settings.h>
#include <webquery.h>
#include <mergeentries.h>

#include "documentwidget.h"

namespace KBibTeX
{

    DocumentWidget::DocumentWidget( bool isReadOnly, QWidget *parent, const char *name )
            : QTabWidget( parent, name ), m_lineEditNewKeyword( NULL ), m_isReadOnly( isReadOnly ), m_filename( QString::null ), m_progressDialog( NULL ), m_newElementCounter( 1 ), m_editMode( emList ), m_viewDocumentActionMenu( NULL ), m_assignKeywordsActionMenu( NULL ), m_searchWebsitesActionMenu( NULL ), m_actionEditCut( NULL ), m_actionEditCopy( NULL ), m_actionEditCopyRef( NULL ), m_actionEditPaste( NULL ), m_actionEditSelectAll( NULL ), m_actionEditFind( NULL ), m_actionEditFindNext( NULL ), m_dirWatch( this )
    {
        m_bibtexfile = new BibTeX::File();

        setupGUI();

        m_listViewElements->setBibTeXFile( m_bibtexfile );
        m_sourceView->setBibTeXFile( m_bibtexfile );
    }

    DocumentWidget::~DocumentWidget()
    {
        delete m_bibtexfile;
    }

    void DocumentWidget::setupGUI()
    {
        setAcceptDrops( TRUE );
        setFocusPolicy( QWidget::ClickFocus );

        // List view tab widget ===============================

        m_container = new QWidget( this );
        QVBoxLayout *layout = new QVBoxLayout( m_container, 0, 0 );
        addTab( m_container, i18n( "L&ist view" ) );

        m_searchBar = new SearchBar( m_container );
        layout->addWidget( m_searchBar );

        m_horSplitter = new QSplitter( Qt::Horizontal, m_container );
        layout->addWidget( m_horSplitter );
        m_horSplitter->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

        m_sideBar = new SideBar( m_isReadOnly, m_horSplitter );
        m_vertSplitter = new QSplitter( Qt::Vertical, m_horSplitter );
        m_vertSplitter->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

        m_listViewElements = new DocumentListView( this, m_isReadOnly, m_vertSplitter );

        m_preview = new KTextEdit( m_vertSplitter );
        m_preview->setReadOnly( TRUE );
        m_preview->setAlignment( Qt::AlignTop && Qt::AlignLeft );

        // Source view tab widget =============================

        m_sourceView = new DocumentSourceView( this, m_isReadOnly, this , "source_view" );
        addTab( m_sourceView, i18n( "So&urce view" ) );
        m_sourceView->setFont( KGlobalSettings::fixedFont() );

        // Setup actions
        connect( m_searchBar, SIGNAL( onlineSearch() ), this, SLOT( onlineSearch() ) );
        connect( m_searchBar, SIGNAL( doSearch( const QString&, BibTeX::Element::FilterType, BibTeX::EntryField::FieldType ) ), m_listViewElements, SLOT( filter( const QString&, BibTeX::Element::FilterType, BibTeX::EntryField::FieldType ) ) );
        connect( m_listViewElements, SIGNAL( executed( DocumentListViewItem* ) ), this, SLOT( executeElement( DocumentListViewItem* ) ) );
        connect( m_listViewElements, SIGNAL( selectionChanged() ), this, SLOT( slotSelectionChanged() ) );
        connect( m_listViewElements, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotPreviewElement( QListViewItem* ) ) );
        connect( m_listViewElements, SIGNAL( clicked( QListViewItem* ) ), this, SLOT( slotPreviewElement( QListViewItem* ) ) );
        connect( this, SIGNAL( currentChanged( QWidget * ) ), this, SLOT( slotTabChanged( QWidget* ) ) );
        connect( m_sourceView, SIGNAL( modified() ), this, SLOT( slotModified() ) );
        connect( m_listViewElements, SIGNAL( modified() ), this, SLOT( slotModified() ) );
        connect( m_sideBar, SIGNAL( valueRenamed() ), this, SLOT( slotModified() ) );
        connect( m_sideBar, SIGNAL( valueRenamed() ), this, SLOT( refreshBibTeXFile() ) );
        connect( m_sideBar, SIGNAL( selected( const QString&, BibTeX::Element::FilterType, BibTeX::EntryField::FieldType ) ), m_searchBar, SLOT( setSearch( const QString&, BibTeX::Element::FilterType, BibTeX::EntryField::FieldType ) ) );
        connect( &m_dirWatch, SIGNAL( dirty( const QString& ) ), this, SLOT( slotFileGotDirty( const QString & ) ) );
    }


    bool DocumentWidget::open( const QString &fileName, bool mergeOnly )
    {
        bool result = FALSE;
        if ( !mergeOnly )
            m_dirWatch.removeFile( m_filename );

        BibTeX::FileImporter *importer = fileImporterFactory( fileName );
        if ( importer != NULL )
        {
            QFile file( fileName );
            if ( file.open( IO_ReadOnly ) )
            {
                result = open( &file, mergeOnly, QString( i18n( "<qt>Loading file <b>%1</b></qt>" ) ).arg( fileName ), importer );
                if ( result )
                {
                    m_bibtexfile->fileName = fileName;
                    m_filename = fileName;
                }
                file.close();
            }
            else
                kdDebug() << "Cannot open file " << fileName << endl;

            delete importer;
        }


        if ( !mergeOnly )
            m_dirWatch.addFile( m_filename );

        return result;
    }

    bool DocumentWidget::open( QIODevice *iodevice, bool mergeOnly, const QString &label, BibTeX::FileImporter *importer )
    {
        bool result = FALSE;
        setEnabled( FALSE );

        bool usingDefaultImporter = importer == NULL;
        if ( usingDefaultImporter )
        {
            Settings * settings = Settings::self( NULL );
            importer = new BibTeX::FileImporterBibTeX( settings->editing_FirstNameFirst );
        }

        startProgress( label, importer );
        BibTeX::File *newFile = importer->load( iodevice );
        endProgress( importer );

        if ( usingDefaultImporter )
            delete importer;

        if ( newFile != NULL )
        {
            if ( mergeOnly )
            {
                MergeEntries::mergeBibTeXFiles( m_bibtexfile, newFile );
                delete newFile;
            }
            else
            {
                delete m_bibtexfile;
                m_bibtexfile = newFile;
            }

            if ( currentPage() == m_sourceView )
                m_sourceView->setBibTeXFile( m_bibtexfile );
            else if ( currentPage() == m_container )
                m_listViewElements->setBibTeXFile( m_bibtexfile );

            Settings * settings = Settings::self( m_bibtexfile );
            settings->addToCompletion( m_bibtexfile );
            m_sideBar->refreshLists( m_bibtexfile );

            result = TRUE;
        }
        else
        {
            kdDebug() << "Could not load bibliography file from io device" << endl;
            result = FALSE;
        }

        setEnabled( TRUE );
        return result;
    }

    bool DocumentWidget::save( const QString &fileName, QStringList *errorLog )
    {
        bool result = FALSE;
        m_dirWatch.removeFile( m_filename );

        BibTeX::File::FileFormat format = BibTeX::File::formatUndefined;
        if ( fileName.endsWith( ".rtf", FALSE ) )
            format = BibTeX::File::formatRTF;
        else if ( fileName.endsWith( ".pdf", FALSE ) )
            format = BibTeX::File::formatPDF;
        else if ( fileName.endsWith( ".bib", FALSE ) )
            format = BibTeX::File::formatBibTeX;
#ifdef HAVE_LIBBIBUTILS
        else if ( fileName.endsWith( ".ref", FALSE ) || fileName.endsWith( ".refer", FALSE ) || fileName.endsWith( ".txt", FALSE ) || fileName.endsWith( ".rfr", FALSE ) )
            format = BibTeX::File::formatEndNote;
        else if ( fileName.endsWith( ".isi", FALSE ) || fileName.endsWith( ".cgi", FALSE ) )
            format = BibTeX::File::formatISI;
#endif
        else if ( fileName.endsWith( ".ris", FALSE ) )
            format = BibTeX::File::formatRIS;
        else if ( fileName.endsWith( ".ps", FALSE ) )
            format = BibTeX::File::formatPS;
        else if ( fileName.endsWith( ".xml", FALSE ) )
            format = BibTeX::File::formatXML;
        else if ( fileName.endsWith( ".html", FALSE ) || fileName.endsWith( ".xhtml", FALSE ) || fileName.endsWith( ".htm", FALSE ) )
            format = BibTeX::File::formatHTML;

        if ( format != BibTeX::File::formatUndefined )
        {
            QFile file( fileName );
            if ( file.open( IO_WriteOnly ) )
            {
                result = save( &file, format, QString( i18n( "<qt>Writing file <b>%1</b></qt>" ) ).arg( fileName ), errorLog );
                if ( result )
                {
                    m_bibtexfile->fileName = fileName;
                    m_filename = fileName;
                }
                file.close();
            }
            else
                kdDebug() << "Cannot write to file " << fileName << endl;
        }
        else
            kdDebug() << "Unknown file format to save to (filename is " << fileName << ")" << endl;

        m_dirWatch.addFile( m_filename );
        return result;
    }

    bool DocumentWidget::save( QIODevice *iodevice, BibTeX::File::FileFormat format, const QString &label, QStringList *errorLog )
    {
        Settings * settings = Settings::self( m_bibtexfile );

        bool result = FALSE;
        setEnabled( FALSE );

        updateFromGUI();

        BibTeX::XSLTransform *transform = NULL;

        BibTeX::FileExporter * exporter = NULL;
        switch ( format )
        {
        case BibTeX::File::formatBibTeX:
            {
                BibTeX::FileExporterBibTeX * bibtexExporter = new BibTeX::FileExporterBibTeX();
                bibtexExporter->setStringDelimiter( settings->fileIO_BibtexStringOpenDelimiter, settings->fileIO_BibtexStringCloseDelimiter );
                bibtexExporter->setKeywordCasing( settings->fileIO_KeywordCasing );
                bibtexExporter->setEncoding( settings->fileIO_Encoding );
                bibtexExporter->setEnclosingCurlyBrackets( settings->fileIO_EnclosingCurlyBrackets );
                exporter = bibtexExporter;
            }
            break;
#ifdef HAVE_LIBBIBUTILS
        case BibTeX::File::formatRIS:
            exporter = new BibTeX::FileExporterBibUtils( BibTeX::FileExporterBibUtils::ofRIS );
            break;
        case BibTeX::File::formatEndNote:
            exporter = new BibTeX::FileExporterBibUtils( BibTeX::FileExporterBibUtils::ofEndNote );
            break;
        case BibTeX::File::formatISI:
            exporter = new BibTeX::FileExporterBibUtils( BibTeX::FileExporterBibUtils::ofISI );
            break;
#else
        case BibTeX::File::formatRIS:
            exporter = new BibTeX::FileExporterRIS( );
            break;
#endif
        case BibTeX::File::formatXML:
            exporter = new BibTeX::FileExporterXML();
            break;
        case BibTeX::File::formatHTML:
            switch ( settings->fileIO_ExporterHTML )
            {
            case BibTeX::FileExporterExternal::exporterNone:
            case BibTeX::FileExporterExternal::exporterXSLT:
                {
                    KStandardDirs * kstd = KGlobal::dirs();
                    QString resPath = kstd->findResource( "data", "kbibtexpart/xslt/html.xsl" );
                    if ( resPath != NULL )
                        transform = new BibTeX::XSLTransform( resPath );

                    if ( transform != NULL )
                        exporter = new BibTeX::FileExporterXSLT( transform );
                    else
                    {
                        kdDebug() << "XSLT for HTML export is not available" << endl;
                    }
                }
                break;
            default:
                {
                    exporter = new BibTeX::FileExporterExternal( settings->fileIO_ExporterHTML, BibTeX::File::formatHTML );
                }
            }
            break;
        case BibTeX::File::formatRTF:
            {
                if ( !settings->external_latex2rtfAvailable )
                {
                    QString msg = i18n( "To export a BibTeX document to the Rich Text Format (RTF) you need to have installed the program latex2rtf." ) ;
                    KMessageBox::information( this, msg );
                    errorLog->append( msg );
                    return FALSE;
                }

                BibTeX::FileExporterRTF *rtfExporter = new BibTeX::FileExporterRTF();
                rtfExporter->setLaTeXLanguage( settings->fileIO_ExportLanguage );
                rtfExporter->setLaTeXBibliographyStyle( settings->fileIO_ExportBibliographyStyle );
                exporter = rtfExporter;
            }
            break;
        case BibTeX::File::formatPDF:
            {
                if ( settings->fileIO_EmbedFiles && !Settings::kpsewhich( "embedfile.sty" ) )
                {
                    KMessageBox::sorry( this, i18n( "Embedding files into the PDF file is enabled, but the required file 'embedfile.sty' was not found. Embedding files will be disabled." ), i18n( "Embedding files disabled" ) );
                    settings->fileIO_EmbedFiles = FALSE;
                }

                BibTeX::FileExporterPDF *pdfExporter = new BibTeX::FileExporterPDF( settings->fileIO_EmbedFiles );
                pdfExporter->setLaTeXLanguage( settings->fileIO_ExportLanguage );
                pdfExporter->setLaTeXBibliographyStyle( settings->fileIO_ExportBibliographyStyle );
                QStringList searchPaths;
                searchPaths.append( settings->editing_DocumentSearchPath );
                if ( m_bibtexfile->fileName != QString::null )
                    searchPaths.append( KURL( m_bibtexfile->fileName ).directory( FALSE, FALSE ) );
                pdfExporter->setDocumentSearchPaths( searchPaths );
                exporter = pdfExporter;
            }
            break;
        case BibTeX::File::formatPS:
            {
                BibTeX::FileExporterPS *psExporter = new BibTeX::FileExporterPS();
                psExporter->setLaTeXLanguage( settings->fileIO_ExportLanguage );
                psExporter->setLaTeXBibliographyStyle( settings->fileIO_ExportBibliographyStyle );
                exporter = psExporter;
            }
            break;
        default:
            kdDebug() << "Unsupported export format selected" << endl;
        }

        if ( exporter != NULL )
        {
            startProgress( label, exporter );
            result = exporter->save( iodevice, m_bibtexfile, errorLog );
            endProgress( exporter );

            if ( transform != NULL )
                delete transform;
            delete exporter;
        }

        setEnabled( TRUE );
        return result;
    }

    bool DocumentWidget::newElement( const QString& elementType )
    {
        Settings * settings = Settings::self( m_bibtexfile );

        if ( m_editMode == emList )
        {
            if ( elementType.lower() == "macro" )
            {
                QString name = QString( i18n( "May only contain ASCII characters, in case of doubt keep English form", "NewMacro%1" ) ).arg( m_newElementCounter++ );
                BibTeX::Macro *macro = new BibTeX::Macro( name );
                if ( MacroWidget::execute( macro, m_isReadOnly ) == QDialog::Accepted )
                {
                    new DocumentListViewItem( m_bibtexfile, macro, m_listViewElements );
                    m_bibtexfile->appendElement( macro );
                    settings->addToCompletion( macro );
                    m_sideBar->refreshLists( m_bibtexfile );
                    return TRUE;
                }
                else
                {
                    delete macro;
                    return FALSE;
                }
            }
            else if ( elementType.lower() == "comment" )
            {
                BibTeX::Comment * comment = new BibTeX::Comment( i18n( "Put your comment here..." ) );
                if ( CommentWidget::execute( comment, m_isReadOnly ) == QDialog::Accepted )
                {
                    new DocumentListViewItem( m_bibtexfile, comment, m_listViewElements );
                    m_bibtexfile->appendElement( comment );
                    return TRUE;
                }
                else
                {
                    delete comment;
                    return FALSE;
                }
            }
            else if ( elementType.lower() == "preamble" )
            {
                BibTeX::Preamble * preamble = new BibTeX::Preamble( );
                if ( PreambleWidget::execute( preamble, m_isReadOnly ) == QDialog::Accepted )
                {
                    new DocumentListViewItem( m_bibtexfile, preamble, m_listViewElements );
                    m_bibtexfile->appendElement( preamble );
                    return TRUE;
                }
                else
                {
                    delete preamble;
                    return FALSE;
                }
            }
            else
            {
                QString name = nextNewEntry();
                BibTeX::Entry *entry = new BibTeX::Entry( elementType, name );
                m_dirWatch.stopScan();
                if ( EntryWidget::execute( entry, m_bibtexfile, m_isReadOnly, TRUE ) == QDialog::Accepted )
                {
                    new DocumentListViewItem( m_bibtexfile, entry, m_listViewElements );
                    m_bibtexfile->appendElement( entry );
                    settings->addToCompletion( entry );
                    m_sideBar->refreshLists( m_bibtexfile );
                    m_dirWatch.startScan();
                    return TRUE;
                }
                else
                {
                    delete entry;
                    m_dirWatch.startScan();
                    return FALSE;
                }
            }
        }
        else if ( m_editMode == emSource )
        {
            if ( elementType.lower() == "macro" )
            {
                QString name = QString( i18n( "May only contain ASCII characters, in case of doubt keep English form", "NewString%1" ) ).arg( m_newElementCounter++ );
                m_sourceView->insertLines( QString( "@string{ %1 = \"%2\" }" ).arg( name ).arg( i18n( "No text yet" ) ) );
            }
            else if ( elementType.lower() == "comment" )
            {
                m_sourceView->insertLines( i18n( "@comment{ Put your comment here... }" ) );
            }
            else if ( elementType.lower() == "preamble" )
            {
                m_sourceView->insertLines( i18n( "@preamble{\"Put your preamble here using double quotes...\"}" ), 0 );
            }
            else
            {
                QString name = nextNewEntry();
                BibTeX::Entry *entry = new BibTeX::Entry( elementType, name );

                for ( int t = 0; t < 2; t++ )
                    for ( int i = ( int ) BibTeX::EntryField::ftAbstract; i <= ( int ) BibTeX::EntryField::ftYear; i++ )
                    {
                        BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType ) i;
                        BibTeX::Entry::FieldRequireStatus fieldRequireStatus = BibTeX::Entry::getRequireStatus( entry->entryType(), fieldType );

                        if (( t == 0 && fieldRequireStatus == BibTeX::Entry::frsRequired ) || ( t == 1 && fieldRequireStatus == BibTeX::Entry::frsOptional ) )
                        {
                            BibTeX::Value * value = new BibTeX::Value();
                            value->items.append( new BibTeX::PlainText( fieldRequireStatus == BibTeX::Entry::frsRequired ? i18n( "REQUIRED" ) : i18n( "optional" ) ) );
                            BibTeX::EntryField *field = new BibTeX::EntryField( fieldType );
                            field->setValue( value );
                            entry->addField( field );
                        }
                    }

                BibTeX::FileExporter * exporter = new BibTeX::FileExporterBibTeX( );
                QBuffer buffer;
                buffer.open( IO_WriteOnly );
                bool result = exporter->save( &buffer, entry );
                buffer.close();

                if ( result )
                {
                    buffer.open( IO_ReadOnly );
                    QTextStream textStream( &buffer );
                    textStream.setEncoding( QTextStream::UnicodeUTF8 );
                    QString text = textStream.read();
                    buffer.close();
                    QStringList lines = QStringList::split( '\n', text );
                    for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it )
                        m_sourceView->insertLines( *it );
                }

                delete exporter;
            }

            return TRUE;
        }

        return FALSE;
    }

    void DocumentWidget::updateViews()
    {
        m_listViewElements->updateVisiblity();
    }

    void DocumentWidget::showStatistics()
    {
        int n = m_bibtexfile->count();
        KMessageBox::information( this, i18n( "This BibTeX file contains 1 element.", "This BibTeX file contains %n elements.", n ), i18n( "File Statistics" ) );
    }

    void DocumentWidget::refreshBibTeXFile()
    {
        m_listViewElements->setBibTeXFile( m_bibtexfile );
        m_sourceView->setBibTeXFile( m_bibtexfile );
    }

    void DocumentWidget::setFactory( KXMLGUIFactory *factory, KXMLGUIClient *client )
    {
        m_searchBar->setFactory( factory, client );
        m_listViewElements->setFactory( factory, client );
        m_sourceView->setFactory( factory, client );

        m_viewDocumentActionMenu = dynamic_cast<KActionMenu*>( client->action( "view_document" ) );
        if ( m_viewDocumentActionMenu != NULL )
            connect( m_viewDocumentActionMenu->popupMenu(), SIGNAL( activated( int ) ), this, SLOT( slotViewDocument( int ) ) );
        m_assignKeywordsActionMenu = dynamic_cast<KActionMenu*>( client->action( "assign_keywords" ) );
        if ( m_assignKeywordsActionMenu != NULL )
            connect( m_assignKeywordsActionMenu->popupMenu(), SIGNAL( activated( int ) ), this, SLOT( slotAssignKeywords( int ) ) );

        m_actionEditCut = client->action( "edit_cut" );
        m_actionEditCopy = client->action( "edit_copy" );
        m_actionEditCopyRef = client->action( "edit_copyref" );
        m_actionEditPaste = client->action( "edit_paste" );
        m_actionEditSelectAll = client->action( "edit_select_all" );
        m_actionEditFind = client->action( "edit_find" );
        m_actionEditFindNext = client->action( "edit_find_next" );
        m_listViewElements->setViewShowColumnsMenu( dynamic_cast<KActionMenu*>( client->action( "view_showcolumns" ) ) );
        m_searchWebsitesActionMenu = dynamic_cast<KActionMenu*>( client->action( "search_document_online" ) );
    }

    void DocumentWidget::updateViewDocumentMenu( )
    {
        if ( m_viewDocumentActionMenu == NULL )
        {
            kdDebug() << "FIXME: m_viewDocumentActionMenu is not set" << endl;
            return;
        }

        KPopupMenu * popup = m_viewDocumentActionMenu->popupMenu();
        popup->clear();
        m_viewDocumentActionMenuURLs.clear();

        BibTeX::Element * currentElement = NULL;

        QListViewItem * item = m_listViewElements->selectedItem();
        if ( item == NULL )
            item = m_listViewElements->currentItem();
        DocumentListViewItem * dlvi = dynamic_cast<DocumentListViewItem*>( item );
        if ( dlvi )
            currentElement = dlvi->element();

        BibTeX::Entry *entry = NULL;
        if ( currentElement != NULL )
            entry = dynamic_cast<BibTeX::Entry*>( currentElement );

        m_viewDocumentActionMenu->setEnabled( FALSE );
        if ( entry != NULL )
        {
            KURL::List documentURLs = getEntryURLs( entry );
            if ( !documentURLs.isEmpty() )
            {
                for ( KURL::List::Iterator i = documentURLs.begin(); i != documentURLs.end(); ++i )
                {
                    QString prettyURL = ( *i ).prettyURL();
                    if ( prettyURL.endsWith( ".pdf", FALSE ) || prettyURL.find( "/pdf/" ) > 0 )
                        popup->insertItem( SmallIcon( "pdf" ), prettyURL );
                    else if ( prettyURL.endsWith( ".ps", FALSE ) )
                        popup->insertItem( SmallIcon( "postscript" ), prettyURL );
                    else if ( prettyURL.endsWith( ".html", FALSE ) || prettyURL.startsWith( "http://", FALSE ) )
                        popup->insertItem( SmallIcon( "html" ), prettyURL );
                    else
                        popup->insertItem( prettyURL );
                    m_viewDocumentActionMenuURLs.append( prettyURL );
                }
                m_viewDocumentActionMenu->setEnabled( TRUE );
            }
        }
    }

    void DocumentWidget::updateAssignKeywords()
    {
        if ( m_assignKeywordsActionMenu == NULL )
        {
            kdDebug() << "FIXME: m_assignKeywordsActionMenu is not set" << endl;
            return;
        }

        KPopupMenu * popup = m_assignKeywordsActionMenu->popupMenu();
        popup->clear();
        m_assignKeywordsActionMenuURLs.clear();
        QStringList entryKeywords;
        QStringList fileKeywords;

        /**
        * Fetch keywords from current entry into entryKeywords list
        */
        QListViewItem * item = m_listViewElements->selectedItem();
        if ( item == NULL )
            item = m_listViewElements->currentItem();
        DocumentListViewItem * dlvi = dynamic_cast<DocumentListViewItem*>( item );
        BibTeX::Element * currentElement = NULL;
        BibTeX::Entry *entry = NULL;
        BibTeX::EntryField *field = NULL;
        BibTeX::Value * valueKeywords = NULL;
        if ( dlvi != NULL && ( currentElement = dlvi->element() ) != NULL && ( entry = dynamic_cast<BibTeX::Entry*>( currentElement ) ) != NULL && ( field = entry->getField( BibTeX::EntryField::ftKeywords ) ) != NULL && ( valueKeywords = field->value() ) != NULL )
            for ( QValueList<BibTeX::ValueItem*>::ConstIterator it = valueKeywords->items.begin();it != valueKeywords->items.end();++it )
            {
                BibTeX::KeywordContainer *container = dynamic_cast<BibTeX::KeywordContainer*>( *it );
                if ( container != NULL )
                    for ( QValueList<BibTeX::Keyword*>::ConstIterator kit = container->keywords.begin();kit != container->keywords.end();++kit )
                        entryKeywords.append(( *kit )->text() );
            }

        /**
        * Fetch all keywords from current file into fileKeywords
        */
        for ( QValueList<BibTeX::Element*>::ConstIterator eit = m_bibtexfile->constBegin(); eit != m_bibtexfile->constEnd(); ++eit )
        {
            BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *eit );
            BibTeX::EntryField *field = NULL;
            BibTeX::Value *value = NULL;
            if ( entry != NULL && (( field = entry->getField( BibTeX::EntryField::ftKeywords ) ) != NULL ) && (( value = field->value() ) != NULL ) )
            {
                for ( QValueList<BibTeX::ValueItem*>::ConstIterator vit = value->items.constBegin(); vit != value->items.constEnd();++vit )
                {
                    BibTeX::KeywordContainer *container = dynamic_cast<BibTeX::KeywordContainer *>( *vit );
                    for ( QValueList<BibTeX::Keyword*>::ConstIterator kit = container->keywords.constBegin(); kit != container->keywords.constEnd();++kit )
                    {
                        QString text = ( *kit )->text();
                        if ( !fileKeywords.contains( text ) )
                            fileKeywords.append( text );
                    }
                }
            }
        }

        /**
        * Merge keyword lists
        */
        Settings * settings = Settings::self( m_bibtexfile );
        QStringList allKeywords = QStringList( fileKeywords );
        for ( QStringList::Iterator it =  settings->keyword_GlobalList .begin(); it !=  settings->keyword_GlobalList .end(); ++it )
            if ( !allKeywords.contains( *it ) )
                allKeywords.append( *it );
        allKeywords.sort();

        /**
        * Build menu
        */
        bool popupEmpty = FALSE;
        if ( allKeywords.isEmpty() )
            popupEmpty = TRUE;
        else if ( allKeywords.count() < 24 )
        {
            for ( QStringList::Iterator it = allKeywords.begin(); it != allKeywords.end(); ++it )
            {
                int i = popup->insertItem( *it );
                popup->setItemChecked( i, entryKeywords.contains( *it ) );
                m_assignKeywordsActionMenuURLs[i] = *it;
            }
        }
        else
        {
            int maxCountPerSubMenu = ( int )sqrt( allKeywords.count() ) + 1;
            int countPerSubMenu = 0;
            KPopupMenu *subMenu = new KPopupMenu( popup );
            connect( subMenu, SIGNAL( activated( int ) ), this, SLOT( slotAssignKeywords( int ) ) );
            QString startWord, endWord;
            for ( QStringList::Iterator it = allKeywords.begin(); it != allKeywords.end(); ++it )
            {
                if ( countPerSubMenu == 0 )
                    startWord = *it;
                endWord = *it;
                int i = subMenu->insertItem( *it );
                subMenu->setItemChecked( i, entryKeywords.contains( *it ) );
                m_assignKeywordsActionMenuURLs[i] = *it;

                ++countPerSubMenu;
                if ( countPerSubMenu >= maxCountPerSubMenu )
                {
                    popup->insertItem( QString( i18n( "%1 ... %2" ) ).arg( startWord ).arg( endWord ), subMenu );
                    subMenu = new KPopupMenu( popup );
                    connect( subMenu, SIGNAL( activated( int ) ), this, SLOT( slotAssignKeywords( int ) ) );
                    countPerSubMenu = 0;
                }
            }

            if ( countPerSubMenu > 0 )
            {
                popup->insertItem( QString( i18n( "%1 ... %2" ) ).arg( startWord ).arg( endWord ), subMenu );
            }
            else
                delete subMenu;
        }

        if ( !popupEmpty )
            popup->insertSeparator();

        QWidget *container = new QWidget( popup );
        container->setBackgroundColor( KGlobalSettings::baseColor() );
        QHBoxLayout *layout = new QHBoxLayout( container, 1, 1 );
        QLabel *label = new QLabel( i18n( "New keyword:" ), container );
        label->setBackgroundColor( KGlobalSettings::baseColor() );
        layout->addWidget( label );
        m_lineEditNewKeyword = new KLineEdit( container );
        layout->addWidget( m_lineEditNewKeyword );
        container->setFocusProxy( m_lineEditNewKeyword );
        container->setFocusPolicy( QWidget::ClickFocus );
        popup->insertItem( container );

        connect( m_lineEditNewKeyword, SIGNAL( returnPressed() ), this, SLOT( slotAddKeyword() ) );
    }

    void DocumentWidget::deferredInitialization()
    {
        restoreState();
        m_listViewElements->deferredInitialization();
    }

    void DocumentWidget::saveState()
    {
        Settings * settings = Settings::self( m_bibtexfile );
        settings->editing_HorSplitterSizes = m_horSplitter->sizes();
        settings->editing_VertSplitterSizes = m_vertSplitter->sizes();
    }

    void DocumentWidget::restoreState()
    {
        m_listViewElements->restoreState();
        m_searchBar->restoreState();
        m_sideBar->restoreState();

        Settings * settings = Settings::self( m_bibtexfile );
        m_horSplitter->setSizes( settings->editing_HorSplitterSizes );
        m_vertSplitter->setSizes( settings->editing_VertSplitterSizes );

        if ( m_searchWebsitesActionMenu != NULL )
        {
            KPopupMenu * popup = m_searchWebsitesActionMenu->popupMenu();
            popup->clear();
            int i = 0;
            for ( QValueList<Settings::SearchURL*>::ConstIterator it = settings->searchURLs.begin(); it != settings->searchURLs.end(); ++it )
                popup->insertItem(( *it ) ->description, ++i );
        }

        if ( settings->editing_UseSpecialFont )
            m_preview->setFont( settings->editing_SpecialFont );
        else
            m_preview->setFont( KGlobalSettings::generalFont() );

    }

    void DocumentWidget::executeElement( DocumentListViewItem* item )
    {
        Settings * settings = Settings::self( m_bibtexfile );
        bool openingDocumentOK = FALSE;

        if ( settings->editing_MainListDoubleClickAction == 1 )
        {
            BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( item->element() );
            if ( entry != NULL )
            {
                QStringList urls = entry->urls();
                for ( QStringList::Iterator it = urls.begin(); it != urls.end(); ++it )
                {
                    KURL url = KURL( *it );
                    if ( url.isValid() && ( !url.isLocalFile() || QFile::exists( url.path() ) ) )
                    {
                        new KRun( url, this );
                        openingDocumentOK = TRUE;
                        break;
                    }
                }
            }
        }

        if ( !openingDocumentOK )
            editElement( item );
    }

    void DocumentWidget::editElement()
    {
        QListViewItem * item = m_listViewElements->selectedItem();
        if ( item == NULL )
            item = m_listViewElements->currentItem();
        DocumentListViewItem * dlvi = dynamic_cast<DocumentListViewItem*>( item );
        if ( dlvi )
            editElement( dlvi );
    }

    void DocumentWidget::editElement( DocumentListViewItem*item )
    {
        BibTeX::Element * element = item->element();
        editElement( element );
        item->updateItem();
    }

    void DocumentWidget::editElement( BibTeX::Element*element )
    {
        m_dirWatch.stopScan();
        editElementDialog( element );
        m_dirWatch.startScan( );

        Settings * settings = Settings::self( m_bibtexfile );
        settings->addToCompletion( element );
        m_sideBar->refreshLists( m_bibtexfile );
    }

    bool DocumentWidget::editElementDialog( BibTeX::Element *element )
    {
        QDialog::DialogCode dialogResult = QDialog::Rejected;
        BibTeX::Entry * entry = dynamic_cast<BibTeX::Entry*>( element );
        if ( entry )
            dialogResult = KBibTeX::EntryWidget::execute( entry, m_bibtexfile, m_isReadOnly, FALSE );
        else
        {
            BibTeX::Comment * comment = dynamic_cast<BibTeX::Comment*>( element );
            if ( comment )
                dialogResult = KBibTeX::CommentWidget::execute( comment, m_isReadOnly );
            else
            {
                BibTeX::Macro* macro = dynamic_cast<BibTeX::Macro*>( element );
                if ( macro )
                    dialogResult = KBibTeX::MacroWidget::execute( macro, m_isReadOnly );
                else
                {
                    BibTeX::Preamble* preamble = dynamic_cast<BibTeX::Preamble*>( element );
                    if ( preamble )
                        dialogResult = KBibTeX::PreambleWidget::execute( preamble, m_isReadOnly );
                }
            }
        }

        if ( dialogResult == QDialog::Accepted )
            slotModified();

        return dialogResult == QDialog::Accepted;
    }

    void DocumentWidget::deleteElements()
    {
        if ( !m_isReadOnly )
        {
            if ( m_editMode == emList )
            {
                m_listViewElements->deleteSelected();
                slotModified();
            }
        }
    }

    void DocumentWidget::sendSelectedToLyx()
    {
        if ( m_editMode == emList )
        {
            QValueList<BibTeX::Element*> elements = m_listViewElements->selectedItems();
            QStringList refsToSend;

            for ( QValueList<BibTeX::Element*>::Iterator it = elements.begin(); it != elements.end(); ++it )
            {
                BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *it );
                if ( entry != NULL ) refsToSend.append( entry->id() );
            }

            Settings * settings = Settings::self( m_bibtexfile );
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
                KMessageBox::error( this, ( lyxPipeFilename.isEmpty() ? i18n( "Cannot establish a link to LyX" ) : QString( i18n( "Cannot establish a link to LyX using the pipe '%1'" ) ).arg( lyxPipeFilename ) ) + i18n( "\nMaybe LyX is not running?" ), i18n( "Error communicating with LyX" ) );

        }
    }

    void DocumentWidget::cutElements()
    {
        if ( !m_isReadOnly )
        {
            if ( m_editMode == emList )
            {
                m_listViewElements->cut();
            }
            else if ( m_editMode == emSource )
            {
                m_sourceView->cut();
            }
            slotModified();
        }
    }

    void DocumentWidget::copyElements()
    {
        if ( m_editMode == emList )
        {
            m_listViewElements->copy();
        }
        else if ( m_editMode == emSource )
        {
            m_sourceView->copy();
        }
    }

    void DocumentWidget::copyRefElements()
    {
        if ( m_editMode == emList )
        {
            m_listViewElements->copyReferences();
        }
        else  if ( m_editMode == emSource )
        {
// TODO: Get selected text, parse text into BibTeX::File and join list of entries' ids
        }
    }

    void DocumentWidget::pasteElements()
    {
        if ( !m_isReadOnly )
        {
            if ( m_editMode == emList )
            {
                if ( m_listViewElements->paste() )
                    slotModified();
            }
            else if ( m_editMode == emSource )
            {
                if ( m_sourceView->paste() )
                    slotModified();
            }
        }
    }

    void DocumentWidget::selectAll()
    {
        if ( m_editMode == emList )
            m_listViewElements->selectAll();
        else if ( m_editMode == emSource )
            m_sourceView->selectAll();
    }

    void DocumentWidget::setReadOnly( bool isReadOnly )
    {
        m_isReadOnly = isReadOnly;
        m_listViewElements->setReadOnly( m_isReadOnly );
        m_sourceView->setReadOnly( m_isReadOnly );
        m_sideBar->setReadOnly( m_isReadOnly );
    }

    void DocumentWidget::configureEditor()
    {
        m_sourceView->configureEditor();
    }

    void DocumentWidget::undoEditor()
    {
        m_sourceView->undo();
    }

    void DocumentWidget::find()
    {
        if ( m_editMode == emSource )
            m_sourceView->find();
    }
    void DocumentWidget::findNext()
    {
        if ( m_editMode == emSource )
            m_sourceView->findNext();
    }

    void DocumentWidget::slotShowProgress( int current, int total )
    {
        if ( m_progressDialog != NULL )
        {
            m_progressDialog->setProgress( current, total );
            qApp->processEvents();
        }
    }

    void DocumentWidget::startProgress( const QString & label, QObject * progressFrom )
    {
        m_progressDialog = new QProgressDialog( this );
        m_progressDialog->setLabelText( label );
        connect( progressFrom, SIGNAL( progress( int, int ) ), this, SLOT( slotShowProgress( int, int ) ) );
        connect( m_progressDialog, SIGNAL( canceled() ), progressFrom, SLOT( cancel( ) ) );
        QApplication::setOverrideCursor( Qt::waitCursor );
    }

    void DocumentWidget::endProgress( QObject * progressFrom )
    {
        disconnect( progressFrom, SIGNAL( progress( int, int ) ), this, SLOT( slotShowProgress( int, int ) ) );
        disconnect( m_progressDialog, SIGNAL( canceled() ), progressFrom, SLOT( cancel( ) ) );
        delete m_progressDialog;
        m_progressDialog = NULL;
        QApplication::restoreOverrideCursor();
    }

    void DocumentWidget::searchWebsites( const QString& searchURL, bool includeAuthor )
    {
        DocumentListViewItem * item = dynamic_cast<DocumentListViewItem*>( m_listViewElements->selectedItem() );
        if ( item == NULL )
            item = dynamic_cast<DocumentListViewItem*>( m_listViewElements->currentItem() );

        if ( item != NULL )
            searchWebsites( item->element(), searchURL, includeAuthor );
    }

    void DocumentWidget::searchWebsites( BibTeX::Element * element, const QString& searchURL, bool includeAuthor )
    {
        QString queryString = QString::null;

        BibTeX::Entry* entry = dynamic_cast<BibTeX::Entry*>( element );
        if ( entry != NULL )
        {
            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftTitle );
            if ( field && field->value() )
                queryString = field->value() ->text();
            if ( includeAuthor )
            {
                field = entry->getField( BibTeX::EntryField::ftAuthor );
                if ( field && field->value() )
                {
                    BibTeX::PersonContainer *personContainer = dynamic_cast<BibTeX::PersonContainer*>( field->value()->items.first() );
                    if ( personContainer != NULL )
                    {
                        QValueList<BibTeX::Person*> list = personContainer->persons;
                        for ( QValueList<BibTeX::Person*>::ConstIterator it = list.begin(); it != list.end(); ++it )
                            queryString = queryString.append( " " ).append(( *it )->lastName() );

                    }
                }
            }
        }
        else
        {
            BibTeX::Comment * comment = dynamic_cast<BibTeX::Comment*>( element );
            if ( comment != NULL )
                queryString = comment->text();
            else
            {
                BibTeX::Macro * macro = dynamic_cast<BibTeX::Macro*>( element );
                if ( macro != NULL && macro->value() )
                    queryString = macro->value() ->text();
                else
                    kdDebug() << "Not yet supported" << endl;
            }
        }

        if ( queryString != QString::null )
            kapp->invokeBrowser( QString( searchURL ).arg( queryString.replace( '{', "" ).replace( '}', "" ) ) );
    }

    void DocumentWidget::onlineSearch()
    {
        if ( !m_isReadOnly )
        {
            BibTeX::FileExporter * exporter = new BibTeX::FileExporterBibTeX( );
            QValueList<BibTeX::Entry*> list;
            if ( WebQueryWizard::execute( this, list ) == QDialog::Accepted )
            {
                Settings * settings = Settings::self( m_bibtexfile );
                for ( QValueList<BibTeX::Entry*>::Iterator it = list.begin(); it != list.end(); ++it )
                {
                    if ( m_editMode == emList )
                        m_listViewElements->insertItem( new BibTeX::Entry( *it ) );
                    else
                    {
                        QBuffer buffer;
                        buffer.open( IO_WriteOnly );
                        bool result = exporter->save( &buffer, *it );
                        buffer.close();
                        if ( result )
                        {
                            buffer.open( IO_ReadOnly );
                            QTextStream textStream( &buffer );
                            textStream.setEncoding( QTextStream::UnicodeUTF8 );
                            QString text = textStream.read();
                            buffer.close();
                            QStringList lines = QStringList::split( '\n', text );
                            for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it )
                                m_sourceView->insertLines( *it );
                            m_sourceView->insertLines( "" );
                        }

                    }
                    settings->addToCompletion( *it );
                }

                slotModified();
            }
            delete exporter;
        }
    }

    void DocumentWidget::updateFromGUI()
    {
        BibTeX::File * file = NULL;
        if ( currentPage() == m_sourceView )
            file = m_sourceView->getBibTeXFile();
        else if ( currentPage() == m_container )
            file = m_listViewElements->getBibTeXFile();

        if ( file != NULL && file != m_bibtexfile )
        {
            delete m_bibtexfile;
            m_bibtexfile = file;
        }
    }

    QString DocumentWidget::nextNewEntry()
    {
        QString name = QString( i18n( "May only contain ASCII characters, in case of doubt keep English form", "NewEntry%1" ) ).arg( m_newElementCounter++ );

        while ( m_bibtexfile->containsKey( name ) != NULL )
        {
            ++m_newElementCounter;
            name = QString( i18n( "May only contain ASCII characters, in case of doubt keep English form", "NewEntry%1" ) ).arg( m_newElementCounter++ );
        }

        return name;
    }

    KURL::List DocumentWidget::getEntryURLs( BibTeX::Entry *entry )
    {
        QStringList urls = entry->urls();
        KURL::List result;

        for ( QStringList::Iterator it = urls.begin(); it != urls.end(); ++it )
        {
            KURL url = Settings::locateFile( *it, m_bibtexfile->fileName, this );
            if ( url.isValid() )
                result.append( url );
        }

        return result;
    }

    void DocumentWidget::slotSelectionChanged()
    {
        int numSelected = 0;
        QListViewItemIterator it( m_listViewElements, QListViewItemIterator::Selected );
        while ( it.current() && numSelected < 3 )
        {
            numSelected++;
            it++;
        }

        emit listViewSelectionChanged( numSelected );
    }

    BibTeX::FileImporter *DocumentWidget::fileImporterFactory( const QString &fileName )
    {
        BibTeX::FileImporter * importer = NULL;
        BibTeX::File::FileFormat format = BibTeX::File::formatUndefined;

        if ( fileName.endsWith( ".bib", FALSE ) )
            format = BibTeX::File::formatBibTeX;
#ifdef HAVE_LIBBIBUTILS
        else if ( fileName.endsWith( ".ref", FALSE ) || fileName.endsWith( ".refer", FALSE ) || fileName.endsWith( ".txt", FALSE ) || fileName.endsWith( ".rfr", FALSE ) )
            format = BibTeX::File::formatEndNote;
        else if ( fileName.endsWith( ".isi", FALSE ) || fileName.endsWith( ".cgi", FALSE ) )
            format = BibTeX::File::formatISI;
#endif // HAVE_LIBBIBUTILS
        else if ( fileName.endsWith( ".ris", FALSE ) )
            format = BibTeX::File::formatRIS;

        if ( format != BibTeX::File::formatUndefined )
        {
            switch ( format )
            {
            case BibTeX::File::formatBibTeX:
                {
                    Settings * settings = Settings::self( NULL );
                    importer = new BibTeX::FileImporterBibTeX( settings->editing_FirstNameFirst );
                }
                break;
#ifdef HAVE_LIBBIBUTILS
            case BibTeX::File::formatEndNote:
                importer = new BibTeX::FileImporterBibUtils( BibTeX::FileImporterBibUtils::ifEndNote );
                break;
            case BibTeX::File::formatRIS:
                importer = new BibTeX::FileImporterBibUtils( BibTeX::FileImporterBibUtils::ifRIS );
                break;
            case BibTeX::File::formatISI:
                importer = new BibTeX::FileImporterBibUtils( BibTeX::FileImporterBibUtils::ifISI );
                break;
#else
            case BibTeX::File::formatRIS:
                importer = new BibTeX::FileImporterRIS( );
                break;
#endif
            default:
                {
                    KMessageBox::sorry( this, QString( i18n( "The file '%1' does not contain a known type of bibliography." ) ).arg( fileName ), i18n( "Unknown file format" ) );
                }
            }
        }

        return importer;
    }

    void DocumentWidget::slotPreviewElement( QListViewItem * item )
    {
        if ( item == NULL )
            item = m_listViewElements->selectedItem();
        if ( item == NULL )
            item = m_listViewElements->currentItem();
        if ( item != NULL )
        {
            DocumentListViewItem * dlvi = dynamic_cast<DocumentListViewItem*>( item );
            if ( dlvi != NULL && m_listViewElements->isEnabled() )
            {
                BibTeX::Element *currentElement = dlvi->element()->clone();
                BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( currentElement );
                if ( entry != NULL )
                    m_bibtexfile->completeReferencedFields( entry );

                KStandardDirs * kstd = KGlobal::dirs();

                BibTeX::XSLTransform *transform = new BibTeX::XSLTransform( kstd->findResource( "data", "kbibtexpart/xslt/html.xsl" ) );
                if ( transform != NULL )
                {
                    BibTeX::FileExporterXSLT * exporter = new BibTeX::FileExporterXSLT( transform );
                    QBuffer buffer;
                    buffer.open( IO_WriteOnly );
                    bool result = exporter->save( &buffer, currentElement );
                    buffer.close();

                    if ( result )
                    {
                        buffer.open( IO_ReadOnly );
                        QTextStream htmlTS( &buffer );
                        htmlTS.setEncoding( QTextStream::UnicodeUTF8 );
                        QString htmlText = htmlTS.read();
                        buffer.close();
                        QString text = htmlText.remove( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" ).append( "</qt>" ).prepend( "<qt>" ).replace( QRegExp( "\\\\emph\\{([^}]+)\\}" ), "<em>\\1</em>" ).replace( QRegExp( "\\\\[A-Za-z0-9]+" ), "" ).replace( '{', "" ).replace( '}', "" );
                        m_preview->setText( text );
                    }
                    else
                    {
                        int i = ( int ) BibTeX::EntryField::ftTitle - ( int ) BibTeX::EntryField::ftAbstract + 2;
                        m_preview->setText( m_listViewElements->currentItem()->text( i ) );
                    }

                    delete exporter;
                    delete transform;
                }
                else
                    m_preview->setText( i18n( "No preview available" ) );

                delete currentElement;
            }
        }
    }

    void DocumentWidget::slotTabChanged( QWidget *tab )
    {
        setEnabled( FALSE );

        BibTeX::File *bibTeXFile = NULL;
        if ( m_editMode == emSource )
            bibTeXFile = m_sourceView->getBibTeXFile();
        else if ( m_editMode == emList )
            bibTeXFile = m_listViewElements->getBibTeXFile();

        if ( tab == m_sourceView && m_editMode != emSource )
        {
            // switching from list view to source view
            if ( bibTeXFile != NULL )
            {
                // check whether the selected widget has changed the bibtex file
                if ( bibTeXFile != m_bibtexfile )
                {
                    delete m_bibtexfile;
                    m_bibtexfile = bibTeXFile;
                }
                m_sourceView->setBibTeXFile( m_bibtexfile );
            }
            m_editMode = emSource;

            if ( m_actionEditCut && m_actionEditCopy && m_actionEditPaste )
            {
                m_actionEditCut->setEnabled( TRUE );
                m_actionEditCopy->setEnabled( TRUE );
                m_actionEditPaste->setEnabled( TRUE );
            }
        }
        else if ( tab == m_container && m_editMode != emList )
        {
            // switching from source view to list view
            if ( bibTeXFile != NULL )
            {
                // check whether the selected widget has changed the bibtex file
                if ( bibTeXFile != m_bibtexfile )
                {
                    delete m_bibtexfile;
                    m_bibtexfile = bibTeXFile;

                    Settings * settings = Settings::self( m_bibtexfile );
                    settings->addToCompletion( m_bibtexfile );
                    m_sideBar->refreshLists( m_bibtexfile );
                }
                m_listViewElements->setBibTeXFile( m_bibtexfile );
                emit listViewSelectionChanged( 0 );
            }
            m_editMode = emList;
        }

        setEnabled( TRUE );

        if ( tab == m_sourceView )
            m_sourceView->setFocus();
        else if ( tab == m_container )
            m_listViewElements->setFocus();

        if ( m_actionEditFind && m_actionEditFindNext )
        {
            m_actionEditFind->setEnabled( m_editMode == emSource );
            m_actionEditFindNext->setEnabled( m_editMode == emSource );
        }
        if ( m_actionEditCopyRef )
            m_actionEditCopyRef->setEnabled( m_editMode == emList );


        emit undoChanged( m_editMode == emSource );
    }

    void DocumentWidget::slotModified()
    {
        if ( isEnabled() )
        {
            m_sideBar->refreshLists( m_bibtexfile );
            emit modified( );
        }
    }

    void DocumentWidget::slotFileGotDirty( const QString& path )
    {
        if ( path == m_filename )
        {
            m_dirWatch.removeFile( m_filename );
            QTimer::singleShot( 100, this, SLOT( slotRefreshDirtyFile( ) ) );
        }
    }

    void DocumentWidget::slotRefreshDirtyFile()
    {
        if ( KMessageBox::questionYesNo( this, QString( i18n( "File '%1' has been modified. Reload file to import changes or ignore changes?" ) ).arg( m_filename ), i18n( "Reload file?" ), KGuiItem( i18n( "Reload" ), "reload" ), KGuiItem( i18n( "Ignore" ), "ignore" ) ) == KMessageBox::Yes )
            open( m_filename, FALSE );
        else
            m_dirWatch.addFile( m_filename );
    }

    void DocumentWidget::slotViewDocument( int id )
    {
        new KRun( m_viewDocumentActionMenuURLs[ m_viewDocumentActionMenu->popupMenu() ->indexOf( id )], this );
    }

    void DocumentWidget::slotAssignKeywords( int id )
    {
        QListViewItem * item = m_listViewElements->selectedItem();
        if ( item == NULL )
            item = m_listViewElements->currentItem();

        BibTeX::Element * currentElement = NULL;
        DocumentListViewItem * dlvi = dynamic_cast<DocumentListViewItem*>( item );
        if ( dlvi && (( currentElement = dlvi->element() ) != NULL ) )
        {
            BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( currentElement );
            BibTeX::EntryField *field = NULL;
            BibTeX::KeywordContainer *keywordContainer = NULL;
            if (( field = entry->getField( BibTeX::EntryField::ftKeywords ) ) == NULL )
            {
                entry->addField( field = new BibTeX::EntryField( BibTeX::EntryField::ftKeywords ) );
            }

            BibTeX::Value *value = field->value();

            if ( !value->items.isEmpty() )
                keywordContainer = dynamic_cast<BibTeX::KeywordContainer*>( value->items.first() );

            if ( keywordContainer == NULL )
                value->items.append( keywordContainer = new BibTeX::KeywordContainer() );

            if ( m_assignKeywordsActionMenu->popupMenu() ->isItemChecked( id ) )
                keywordContainer->remove( m_assignKeywordsActionMenuURLs[ id ] );
            else
                keywordContainer->append( m_assignKeywordsActionMenuURLs[ id ] );

            slotModified();
        }
    }

    void DocumentWidget::slotAddKeyword()
    {
        QString newKeyword = m_lineEditNewKeyword->text();

        QListViewItem * item = m_listViewElements->selectedItem();
        if ( item == NULL )
            item = m_listViewElements->currentItem();
        DocumentListViewItem * dlvi = dynamic_cast<DocumentListViewItem*>( item );
        BibTeX::Element * currentElement = NULL;
        BibTeX::Entry * entry = NULL;
        if ( dlvi != NULL && ( currentElement = dlvi->element() ) != NULL && ( entry = dynamic_cast<BibTeX::Entry*>( currentElement ) ) != NULL )
        {

            BibTeX::EntryField *field = entry->getField( BibTeX::EntryField::ftKeywords );
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( BibTeX::EntryField::ftKeywords );
                entry->addField( field );
            }

            BibTeX::Value *value = field->value();
            BibTeX::KeywordContainer *keywordContainer = NULL;

            if ( !value->items.isEmpty() )
                keywordContainer = dynamic_cast<BibTeX::KeywordContainer*>( value->items.first() );

            if ( keywordContainer == NULL )
                value->items.append( keywordContainer = new BibTeX::KeywordContainer() );

            keywordContainer->append( newKeyword );

            if ( m_lineEditNewKeyword->isVisible() &&
                    m_lineEditNewKeyword->parentWidget() &&
                    m_lineEditNewKeyword->parentWidget()->parentWidget() &&
                    m_lineEditNewKeyword->parentWidget()->parentWidget()->inherits( "QPopupMenu" ) )
                m_lineEditNewKeyword->parentWidget()->parentWidget()->close();

            slotModified();
        }
    }
}

#include "documentwidget.moc"
