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
#include <qbuffer.h>
#include <qlayout.h>
#include <qfile.h>

#include <klibloader.h>
#include <kxmlguifactory.h>
#include <kapplication.h>
#include <klibloader.h>
#include <kfinddialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktrader.h>
#include <kmimetype.h>
#include <kservice.h>
#include <kparts/componentfactory.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/undointerface.h>

#include <fileexporterbibtex.h>
#include <fileimporterbibtex.h>
#include <settings.h>
#include "documentsourceview.h"

namespace KBibTeX
{

    DocumentSourceView::DocumentSourceView( KBibTeX::DocumentWidget *docWidget, bool isReadOnly, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_isReadOnly( isReadOnly ), m_docWidget( docWidget ), m_bibtexFile( NULL ), m_document( NULL ), m_view( NULL )
    {
        setupGUI();

        readConfig();
    }

    DocumentSourceView::~DocumentSourceView()
    {
        kapp->config() ->sync();
    }

    /**
     * Copy the selected text to the clipboard
     */
    void DocumentSourceView::copy()
    {
        KTextEditor::ClipboardInterface * clip = dynamic_cast<KTextEditor::ClipboardInterface*>( m_view );
        if ( clip )
            clip->copy();
    }

    /**
     * Cut the selected text to the clipboard
     */
    void DocumentSourceView::cut()
    {
        KTextEditor::ClipboardInterface * clip = dynamic_cast<KTextEditor::ClipboardInterface*>( m_view );
        if ( clip )
            clip->cut();
    }

    /**
     * Paste the clipboard's content into the document
     */
    bool DocumentSourceView::paste()
    {
        KTextEditor::ClipboardInterface * clip = dynamic_cast<KTextEditor::ClipboardInterface*>( m_view );
        if ( clip )
        {
            clip->paste();
            return TRUE;
        }
        else
            return FALSE;
    }

    /**
     * Paste a line of text into the document at the designated position
     */
    void DocumentSourceView::insertLines( const QString& text, int line )
    {
        m_editInterface->insertLine( line < 0 ? m_editInterface->numLines() : ( uint ) line, text );

        KTextEditor::ViewCursorInterface * cursorIf = dynamic_cast<KTextEditor::ViewCursorInterface*>( m_view );
        if ( cursorIf )
            cursorIf->setCursorPosition( line < 0 ? m_editInterface->numLines() - 1 : ( uint ) line, 0 );
    }

    void DocumentSourceView::selectAll()
    {
        KTextEditor::SelectionInterface * selI = dynamic_cast<KTextEditor::SelectionInterface*>( m_view );
        if ( selI )
            selI->selectAll();
    }

    void DocumentSourceView::setReadOnly( bool isReadOnly )
    {
        m_isReadOnly = isReadOnly;
        m_document->setReadWrite( !m_isReadOnly );
    }

    void DocumentSourceView::setFactory( KXMLGUIFactory *factory, KXMLGUIClient * client )
    {
        KTextEditor::PopupMenuInterface * popupInterface = dynamic_cast<KTextEditor::PopupMenuInterface*>( m_view );
        popupInterface->installPopup(( QPopupMenu* )( factory ->container( "ktexteditor_popup", client ) ) );
    }

    void DocumentSourceView::configureEditor()
    {
        KTextEditor::ConfigInterface * conf = KTextEditor::configInterface( m_document );
        if ( !conf )
        {
            kdDebug() << "*** No KTextEditor::ConfigInterface for part!" << endl;
            return ;
        }

        // show the modal config dialog for this part if it has a ConfigInterface
        conf->configDialog();
        conf->writeConfig();
    }

    void DocumentSourceView::undo()
    {
        KTextEditor::UndoInterface * undoIf = dynamic_cast<KTextEditor::UndoInterface*>( m_document );
        if ( undoIf != NULL )
            undoIf->undo();
        else
            kdDebug() << "Cannot determine undo interface" << endl;
    }

    void DocumentSourceView::find()
    {
        KFindDialog dlg( TRUE, this );
        dlg.setFindHistory( m_findHistory );
        dlg.setHasSelection( FALSE );
#if KDE_IS_VERSION(3,4,0)
        dlg.setSupportsWholeWordsFind( FALSE );
        dlg.setSupportsBackwardsFind( FALSE );
        dlg.setSupportsCaseSensitiveFind( FALSE );
        dlg.setSupportsRegularExpressionFind( FALSE );
#endif
        if ( dlg.exec() == QDialog::Accepted )
        {
            m_lastSearchTerm = dlg.pattern();
            m_findHistory = dlg.findHistory();



            KTextEditor::ViewCursorInterface * cursorIf = dynamic_cast<KTextEditor::ViewCursorInterface*>( m_view );

            if ( cursorIf )
            {
                unsigned int fromLine = 0;
                unsigned int fromCol = 0;
                if ( dlg.options() && KFindDialog::FromCursor )
                {
                    cursorIf->cursorPosition( &fromLine, &fromCol );
                    fromCol++;
                }

                search( fromLine, fromCol );
            }
            else
                kdDebug() << "Failed to instantiate ViewCursorInterface, SearchInterface, or SelectionInterface" << endl;
        }
    }

    void DocumentSourceView::findNext()
    {
        if ( m_lastSearchTerm.isEmpty() )
            find();
        else
        {
            KTextEditor::ViewCursorInterface * cursorIf = dynamic_cast<KTextEditor::ViewCursorInterface*>( m_view );

            if ( cursorIf )
            {
                unsigned int fromLine = 0;
                unsigned int fromCol = 0;
                cursorIf->cursorPosition( &fromLine, &fromCol );

                search( fromLine, fromCol );
            }
            else
                kdDebug() << "Failed to instantiate ViewCursorInterface, SearchInterface, or SelectionInterface" << endl;
        }
    }

    bool DocumentSourceView::setBibTeXFile( BibTeX::File *bibtexFile )
    {
        Settings * settings = Settings::self();

        bool result = FALSE;

        QBuffer buffer;
        buffer.open( IO_WriteOnly );
        BibTeX::FileExporterBibTeX * exporter = new BibTeX::FileExporterBibTeX();
        exporter->setStringDelimiter( settings->fileIO_BibtexStringOpenDelimiter, settings->fileIO_BibtexStringCloseDelimiter );
        exporter->setKeywordCasing( settings->fileIO_KeywordCasing );
        exporter->setEncoding(( BibTeX::File::Encoding ) settings->fileIO_Encoding );
        exporter->setEnclosingCurlyBrackets( settings->fileIO_EnclosingCurlyBrackets );
        result = exporter->save( &buffer, bibtexFile );
        delete exporter;
        buffer.close();

        if ( result )
        {
            buffer.open( IO_ReadOnly );
            QTextStream in( &buffer );
            in.setEncoding( QTextStream::UnicodeUTF8 );
            QString text = in.read();
            buffer.close();

            if ( m_editInterface )
            {
                // very strange: to set the text, you have to set
                // readwrite to TRUE...
                m_document->setReadWrite( TRUE );
                m_editInterface->setText( text );
                m_document->setReadWrite( !m_isReadOnly );
            }

            m_bibtexFile = bibtexFile;
        }

        return result;
    }

    BibTeX::File* DocumentSourceView::getBibTeXFile()
    {
        if ( m_editInterface )
        {
            QBuffer buffer;

            buffer.open( IO_WriteOnly );
            QTextStream stream( &buffer );
            stream.setEncoding( QTextStream::UnicodeUTF8 );
            stream << m_editInterface->text();
            buffer.close();

            Settings * settings = Settings::self( m_bibtexFile );

            buffer.open( IO_ReadOnly );
            BibTeX::FileImporter *importer = new BibTeX::FileImporterBibTeX( settings->editing_FirstNameFirst );
            BibTeX::File *result = importer->load( &buffer );
            delete importer;
            buffer.close();

            return result;
        }

        return NULL;
    }

    void DocumentSourceView::focusInEvent( QFocusEvent* /*event*/ )
    {
        if ( m_view != NULL )
            m_view->setFocus();
    }

    void DocumentSourceView::setupGUI()
    {
        // create the Kate::Document
        m_document = Kate::createDocument( this, "Kate::Document" );

        m_view = ( Kate::View * ) m_document->createView( this, 0L );
        m_editInterface = editInterface( m_document );
        m_document->setReadWrite( !m_isReadOnly );
        if ( !m_isReadOnly )
            connect( m_document, SIGNAL( textChanged() ), this, SIGNAL( modified() ) );
        QBoxLayout * layout = new QVBoxLayout( this );
        layout->addWidget( static_cast<QWidget*>( m_view ) );

        int c = m_document->hlModeCount() ;
        int hlIdx = -1;
        for ( int i = 0; i < c; i++ )
            if ( m_document->hlModeName( i ) .compare( "BibTeX" ) == 0 )
            {
                hlIdx = i;
                break;
            }
        if ( hlIdx > -1 )
            m_document-> setHlMode( hlIdx );
    }

    void DocumentSourceView::readConfig()
    {
        KConfig * config = kapp->config();
        readConfig( config );
    }

    void DocumentSourceView::writeConfig()
    {
        KConfig * config = kapp->config();
        writeConfig( config );
    }

    void DocumentSourceView::readConfig( KConfig *config )
    {
        if ( m_view )
        {
            KTextEditor::ConfigInterface * conf = KTextEditor::configInterface( m_document );
            if ( conf )
                conf->readConfig( config );
        }
    }

    void DocumentSourceView::writeConfig( KConfig *config )
    {
        if ( m_view )
        {
            KTextEditor::ConfigInterface * conf = KTextEditor::configInterface( m_document );
            if ( conf )
                conf->writeConfig( config );
        }
        config->sync();
    }

    void DocumentSourceView::search( int fromLine, int fromCol )
    {
        unsigned int foundAtLine, foundAtCol, matchLen;

        KTextEditor::SearchInterface * searchIf = KTextEditor::searchInterface( m_document );
        KTextEditor::SelectionInterface *selectionIf = KTextEditor::selectionInterface( m_document );
        KTextEditor::ViewCursorInterface * cursorIf = dynamic_cast<KTextEditor::ViewCursorInterface*>( m_view );

        do
        {
            bool result = searchIf->searchText( fromLine, fromCol, m_lastSearchTerm, &foundAtLine, &foundAtCol, &matchLen, FALSE );
            if ( result )
            {
                selectionIf->setSelection( foundAtLine, foundAtCol, foundAtLine, foundAtCol + matchLen );
                cursorIf->setCursorPositionReal( foundAtLine, foundAtCol + matchLen );
                break;
            }

            fromLine = 0;
            fromCol = 0;
        }
        while ( KMessageBox::questionYesNo( this, QString( i18n( "Could not find text '%1' in the document.\nStart from the beginning?" ) ).arg( m_lastSearchTerm ), i18n( "Find text in source view" ), KGuiItem( i18n( "Restart search" ) ) ) == KMessageBox::Yes );
    }
}
#include "documentsourceview.moc"
