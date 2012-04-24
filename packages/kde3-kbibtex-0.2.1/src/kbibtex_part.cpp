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
#include <string>
#include <fcntl.h>

#include <qstring.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qbuffer.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qclipboard.h>
#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <klistview.h>
#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kconfigdialog.h>
#include <kstatusbar.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kxmlguifactory.h>
#include <kpopupmenu.h>
#include <kio/netaccess.h>
#include <kmainwindow.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <entry.h>
#include <documentlistviewitem.h>
#include <documentlistview.h>
#include <fileexporter.h>
#include <fileexporterbibtex.h>
#include <fileexporterxml.h>
#include <fileexporterxslt.h>
#include <settings.h>
#include <encoderlatex.h>
#include "kbibtex_part.h"

const QString inPipeFilename = QDir::homeDirPath() + "/.kbibtex.in";

KBibTeXPart::KBibTeXPart( QWidget *parentWidget, const char *widgetName,
                          QObject *parent, const char *name )
        : KParts::ReadWritePart( parent, name ), m_defInitCounter( 0 ), m_settingsDlg( NULL ), m_initializationDone( FALSE ), m_inPipe( NULL )
{
    m_mainWindow = dynamic_cast<KMainWindow*>( parent );
    if ( m_mainWindow == NULL )
        kdDebug( ) << "Cannot determine main window" << endl;

    // we need an instance
    setInstance( KBibTeXPartFactory::instance() );

    // set our XML-UI resource file
    setXMLFile( "kbibtex_part.rc" );

    // setup GUI elements
    setupGUI( parentWidget, widgetName );

    // create our actions
    setupActions();

    // we are read-write by default
    setReadWrite( TRUE );

    // we are not modified since we haven't done anything yet
    setModified( FALSE );

    // read configuration
    readSettings();

    QTimer::singleShot( 100, this, SLOT( slotDeferredInitialization() ) );
}

KBibTeXPart::~KBibTeXPart()
{
    BibTeX::EncoderLaTeX::deleteCurrentEncoderLaTeX();

    if ( m_settingsDlg != NULL )
        delete m_settingsDlg;
}

void KBibTeXPart::setReadWrite( bool rw )
{
    if ( rw )
    {
        connect( m_documentWidget, SIGNAL( modified( ) ), this, SLOT( setModified( ) ) );
        m_actionEditElement->setText( i18n( "&Edit" ) );
    }
    else
    {
        disconnect( m_documentWidget, SIGNAL( modified( ) ), this, SLOT( setModified( ) ) );
        m_actionEditElement->setText( i18n( "&View" ) );
    }
    ReadWritePart::setReadWrite( rw );
    m_documentWidget->setReadOnly( !rw );
}

void KBibTeXPart::setModified( bool modified )
{
    // get a handle on our Save action and make sure it is valid
    KAction * save = actionCollection() ->action( KStdAction::stdName( KStdAction::Save ) );

    if ( save )
    {
        // if so, we either enable or disable it based on the current
        // state
        save->setEnabled( modified );

        // in any event, we want our parent to do it's thing
        KParts::ReadWritePart::setModified( modified );
    }
}

bool KBibTeXPart::queryClose()
{
    writeSettings();

    if ( !isReadWrite() || !isModified() )
        return TRUE;

    QString docName = url().fileName();
    if ( docName.isEmpty() ) docName = i18n( "Untitled" );

    int res = KMessageBox::warningYesNoCancel( widget(),
              i18n( "The document '%1' has been modified.\n"
                    "Do you want to save your changes or discard them?" ).arg( docName ),
              i18n( "Close Document" ), KStdGuiItem::save(), KStdGuiItem::discard() );

    bool abortClose = false;
    bool handled = false;

    switch ( res )
    {
    case KMessageBox::Yes :
        sigQueryClose( &handled, &abortClose );
        if ( !handled )
        {
            if ( m_url.isEmpty() )
                return saveAs();
            else
                save();
        }
        else if ( abortClose ) return FALSE;
        return waitSaveComplete();
    case KMessageBox::No :
        return TRUE;
    default:
        return FALSE;
    }

}

bool KBibTeXPart::closeURL()
{
    writeSettings();

    if ( isReadWrite() && isModified() )
    {
        if ( !queryClose() )
            return false;
    }
    // Not modified => ok and delete temp file.
    return KParts::ReadOnlyPart::closeURL();
}

bool KBibTeXPart::openFile()
{
    return m_documentWidget->open( m_file, FALSE );
}

/**
 * Save document to a filename as given in String m_file.
 */
bool KBibTeXPart::saveFile()
{
    if ( isReadWrite() == FALSE )
        return FALSE;

    if ( !url().isValid() || url().isEmpty() )
        return saveAs();
    else
        return m_documentWidget->save( m_file );
}

void KBibTeXPart::slotFileSaveAs()
{
    saveAs();
}

bool KBibTeXPart::saveAs()
{
    bool result = FALSE;

    QString startDir = ! url().isEmpty() ? url().url() : QDir::currentDirPath();
    KURL saveURL = KFileDialog::getSaveURL( startDir, "*.bib|" + i18n( "BibTeX (*.bib)" ) + "\n*.ris|" + i18n( "Reference Manager (*.ris)" )
#ifdef HAVE_LIBBIBUTILS
                                            + "\n*.ref *.refer *.rfr *.txt|" + i18n( "EndNote (Refer format) (*.ref *.refer *.rfr *.txt)" ) + "\n*.isi *.cgi|" + i18n( "ISI Web of Knowledge (*.isi *.cgi)" )
#endif
                                            , widget() );

    if ( saveURL.isValid() && !saveURL.isEmpty() )
    {

        if ( KIO::NetAccess::exists( saveURL, FALSE, widget() ) && KMessageBox::warningContinueCancel( widget(), i18n( "A file named '%1' already exists. Are you sure you want to overwrite it?" ).arg( saveURL.filename() ), QString::null, i18n( "Overwrite" ) ) != KMessageBox::Continue )
            return result;

        if ( ReadWritePart::saveAs( saveURL ) )
        {
            emit signalAddRecentURL( saveURL );
            result = TRUE;
        }
    }

    return result;
}

void KBibTeXPart::slotFileExport()
{
    QString startDir = ! url().isEmpty() ? url().url() : QDir::currentDirPath();
    KURL exportURL = KFileDialog::getSaveURL( startDir, "*.pdf|" + i18n( "Portable Document File (*.pdf)" ) + "\n*.ps|" + i18n( "PostScript (*.ps)" ) + "\n*.ris|" + i18n( "Reference Manager (*.ris)" )
#ifdef HAVE_LIBBIBUTILS
                     + "\n*.ref *.refer *.rfr *.txt|" + i18n( "EndNote (Refer format) (*.ref *.refer *.rfr *.txt)" ) + "\n*.isi *.cgi|" + i18n( "ISI Web of Knowledge (*.isi *.cgi)" )
#endif
                     + "\n*.xml|" + i18n( "eXtended Markup Language (*.xml)" ) + "\n*.html|" + i18n( "Hypertext Markup Langage (*.html)" ) + "\n*.rtf|" + i18n( "Rich Text Format (*.rtf)" ), widget() );

    if ( exportURL.isValid() && !exportURL.isEmpty() )
    {
        if ( KIO::NetAccess::exists( exportURL, FALSE, widget() ) && KMessageBox::warningContinueCancel( widget(), i18n( "A file named '%1' already exists. Are you sure you want to overwrite it?" ).arg( exportURL.filename() ), QString::null, i18n( "Overwrite" ) ) != KMessageBox::Continue )
            return ;

        KTempFile tempBibFile = KTempFile( locateLocal( "tmp", "bibexport" ) );
        tempBibFile.setAutoDelete( TRUE );
        BibTeX::File::FileFormat format = BibTeX::File::formatBibTeX;
        if ( exportURL.fileName().endsWith( ".rtf", FALSE ) )
            format = BibTeX::File::formatRTF;
        else if ( exportURL.fileName().endsWith( ".pdf", FALSE ) )
            format = BibTeX::File::formatPDF;
        else if ( exportURL.fileName().endsWith( ".ps", FALSE ) )
            format = BibTeX::File::formatPS;
#ifdef HAVE_LIBBIBUTILS
        else if ( exportURL.fileName().endsWith( ".ref", FALSE ) || exportURL.fileName().endsWith( ".refer", FALSE ) || exportURL.fileName().endsWith( ".txt", FALSE ) || exportURL.fileName().endsWith( ".rfr", FALSE ) )
            format = BibTeX::File::formatEndNote;
        else if ( exportURL.fileName().endsWith( ".isi", FALSE ) || exportURL.fileName().endsWith( ".cgi", FALSE ) )
            format = BibTeX::File::formatISI;
#endif
        else if ( exportURL.fileName().endsWith( ".ris", FALSE ) )
            format = BibTeX::File::formatRIS;
        else if ( exportURL.fileName().endsWith( ".xml", FALSE ) )
            format = BibTeX::File::formatXML;
        else if ( exportURL.fileName().endsWith( ".html", FALSE ) )
            format = BibTeX::File::formatHTML;

        QStringList errorLog;

        bool error = !m_documentWidget->save( tempBibFile.file(), format, i18n( "Exporting" ), &errorLog ) ;
        if ( !error )
        {
            tempBibFile.close();
            error = !KIO::NetAccess::file_copy( KURL( tempBibFile.name() ), exportURL, -1, TRUE, FALSE, widget() );
        }

        if ( error )
        {
#if (KDE_VERSION_MINOR>=4) && (KDE_VERSION_MAJOR>=3)
            if ( !errorLog.empty() )
                KMessageBox::errorList( widget(), QString( i18n( "File could not be exported in '%1'." ) ).arg( exportURL.prettyURL() ).append( i18n( "\n\nSee the following error log for details." ) ), errorLog, i18n( "Exporting failed" ) );
            else
#endif
                KMessageBox::error( widget(), QString( i18n( "File could not be exported to '%1'." ) ).arg( exportURL.prettyURL() ), i18n( "Exporting failed" ) );
        }

    }

}

void KBibTeXPart::slotFileStatistics()
{
    m_documentWidget->showStatistics();
}

void KBibTeXPart::slotFileMerge()
{
    QString startDir = ! url().isEmpty() ? url().url() : QDir::currentDirPath();
    KURL mergeURL = KFileDialog::getOpenURL( startDir, QString( "*.bib *.ris" ) +
#ifdef HAVE_LIBBIBUTILS
                    " *.ref *.refer *.rfr *.txt *.isi *.cgi" +
#endif
                    "|" + i18n( "Supported Bibliographies" ) + "\n*.bib|" + i18n( "BibTeX (*.bib)" ) + "\n*.ris|" + i18n( "Reference Manager (*.ris)" )
#ifdef HAVE_LIBBIBUTILS
                    + "\n*.ref *.refer *.rfr *.txt|" + i18n( "EndNote (Refer format) (*.ref *.refer *.rfr *.txt)" ) + "\n*.isi *.cgi|" + i18n( "ISI Web of Knowledge (*.isi *.cgi)" )
#endif
                    , widget() );

    if ( !mergeURL.isValid() || mergeURL.isEmpty() )
        return;

    QString extension = mergeURL.fileName();
    int extPos = extension.find( '.' );
    if ( extPos < 0 )
        return;
    extension = extension.mid( extPos );

    if ( !KIO::NetAccess::exists( mergeURL, TRUE, widget() ) )
    {
        KMessageBox::error( widget(), i18n( "The given file could not be read, check if it exists or if it is readable for the current user." ) );
        return ;
    }

    KTempFile tempBibFile = KTempFile( locateLocal( "tmp", "bibmerge" ), extension );
    tempBibFile.setAutoDelete( TRUE );
    bool error = !KIO::NetAccess::file_copy( mergeURL, KURL( tempBibFile.name() ),  -1, TRUE, FALSE, widget() );

    if ( !error )
        error = !m_documentWidget->open( tempBibFile.name(), TRUE );

    tempBibFile.close();

    if ( error )
    {
        KMessageBox::error( widget(), i18n( "The given file could not be merged." ) );
        return;
    }
    else
        setModified( TRUE );
}

void KBibTeXPart::slotPreferences()
{
    if ( !m_settingsDlg )
        m_settingsDlg = new KBibTeX::SettingsDlg( widget(), "settings_dlg" );

    m_documentWidget->saveState();
    if ( m_settingsDlg->exec() == QDialog::Accepted )
        m_documentWidget->restoreState();
}

void KBibTeXPart::setupGUI( QWidget *parentWidget, const char *name )
{
    m_documentWidget = new KBibTeX::DocumentWidget( !isReadWrite(), parentWidget, name );

    // notify the part that this is our internal widget
    setWidget( m_documentWidget );
}

void KBibTeXPart::setupActions()
{
    if ( isReadWrite() )
    {
        KStdAction::save( this, SLOT( save() ), actionCollection() ) ->setToolTip( i18n( "Save the current file" ) );
        KStdAction::saveAs( this, SLOT( slotFileSaveAs() ), actionCollection() ) ->setToolTip( i18n( "Save the current file under a new filename" ) );
    }
    m_actionFileStatistics = new KAction( i18n( "Statistics..." ), CTRL + SHIFT + Key_I, this, SLOT( slotFileStatistics() ), actionCollection(), "file_statistics" );
    m_actionFileExport = new KAction( i18n( "Export..." ), SmallIconSet( "goto" ), CTRL + Key_E, this, SLOT( slotFileExport() ), actionCollection(), "file_export" );
    m_actionFileExport->setToolTip( i18n( "Export to a different file format" ) );
    m_actionFileMerge = new KAction( i18n( "Merge..." ), SmallIconSet( "reload" ), CTRL + Key_M, this, SLOT( slotFileMerge() ), actionCollection(), "file_merge" );
    m_actionFileMerge->setToolTip( i18n( "Merge BibTeX entries from another file" ) );

    m_actionEditCut = KStdAction::cut( m_documentWidget, SLOT( cutElements() ), actionCollection() );
    m_actionEditCut->setToolTip( i18n( "Cut the selected BibTeX elements to the clipboard" ) );
    m_actionEditCut->setEnabled( FALSE );
    m_actionEditCopy = KStdAction::copy( m_documentWidget, SLOT( copyElements() ), actionCollection() );
    m_actionEditCopy->setToolTip( i18n( "Copy the selected BibTeX elements to the clipboard" ) );
    m_actionEditCopy->setEnabled( FALSE );
    m_actionEditUndo =  KStdAction::undo( m_documentWidget, SLOT( undoEditor() ), actionCollection() );
    connect( m_documentWidget, SIGNAL( undoChanged( bool ) ), this, SLOT( slotUndoChanged( bool ) ) );
    m_actionEditCopyRef = new KAction( i18n( "Copy &Reference" ), CTRL + SHIFT + Key_C, m_documentWidget, SLOT( copyRefElements() ), actionCollection(), "edit_copyref" );
    m_actionEditCopyRef->setToolTip( i18n( "Copy a reference of the selected BibTeX elements to the clipboard" ) );
    m_actionEditCopyRef->setEnabled( FALSE );
    m_actionEditPaste = KStdAction::paste( m_documentWidget, SLOT( pasteElements() ), actionCollection() );
    m_actionEditPaste->setToolTip( i18n( "Paste BibTeX elements from the clipboard" ) );
    m_actionEditSelectAll = KStdAction::selectAll( m_documentWidget, SLOT( selectAll() ), actionCollection() );
    m_actionEditSelectAll->setToolTip( i18n( "Select all BibTeX elements" ) );
    m_actionElementSendToLyX = new KAction( i18n( "Send Reference to &LyX" ), CTRL + SHIFT + Key_L, m_documentWidget, SLOT( sendSelectedToLyx() ), actionCollection(), "element_sendtolyx" );
    m_actionElementSendToLyX->setToolTip( i18n( "Send a reference of the selected BibTeX elements to LyX" ) );
    m_actionElementSendToLyX->setEnabled( FALSE );
    m_actionEditFind = KStdAction::find( m_documentWidget, SLOT( find() ), actionCollection() );
    m_actionEditFind->setToolTip( i18n( "Find text" ) );
    m_actionEditFindNext = KStdAction::findNext( m_documentWidget, SLOT( findNext() ), actionCollection() );
    m_actionEditFindNext->setToolTip( i18n( "Find next occurrence" ) );
    m_actionEditFind->setEnabled( FALSE );
    m_actionEditFindNext->setEnabled( FALSE );

    m_actionSearchOnlineDatabases = new KAction( i18n( "Search Online Databases" ), SmallIconSet( "network" ), CTRL + SHIFT + Key_S, m_documentWidget, SLOT( onlineSearch() ), actionCollection(), "search_onlinedatabases" );
    m_actionSearchOnlineDatabases->setToolTip( i18n( "Search online databases to import bibliography data" ) );

    m_actionMenuSearchWebsites = new KActionMenu( i18n( "Search Document Online" ), actionCollection(), "search_document_online" );
    m_actionMenuSearchWebsites->setToolTip( i18n( "Search websites for the selected BibTeX element" ) );
    connect( m_actionMenuSearchWebsites->popupMenu(), SIGNAL( activated( int ) ), this, SLOT( slotSearchWebsites( int ) ) );
    m_actionMenuSearchWebsites->setEnabled( FALSE );

    m_actionViewShowColumns = new KActionMenu( i18n( "&Show Columns" ), actionCollection(), "view_showcolumns" );
    m_actionViewShowColumns->setToolTip( i18n( "Select columns to show in the main list" ) );
    m_actionViewDocument = new KActionMenu( i18n( "View Document" ), actionCollection(), "view_document" );
    m_actionViewDocument->setToolTip( i18n( "View the documents associated with this BibTeX entry" ) );
    m_actionViewDocument->setEnabled( FALSE );
    m_actionAssignKeywords = new KActionMenu( i18n( "Assign Keywords" ), actionCollection(), "assign_keywords" );
    m_actionAssignKeywords->setToolTip( i18n( "Assign keywords to this entry" ) );
    m_actionAssignKeywords->setEnabled( FALSE );

    m_actionPreferences = KStdAction::preferences( this, SLOT( slotPreferences() ), actionCollection() );
    m_actionPreferences->setToolTip( i18n( "Configure KBibTeX" ) );
    KAction *actionSourceEditor = new KAction( i18n( "Configure Source View..." ), 0, m_documentWidget, SLOT( configureEditor() ), actionCollection(), "options_configure_editor" );
    actionSourceEditor ->setToolTip( i18n( "Configure the Source Editor" ) );

    ( new KAction( i18n( "Article" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_article" ) ) ->setToolTip( i18n( "Add an article to the BibTeX file" ) );
    ( new KAction( i18n( "Book" ), "element_new_entry_book", CTRL + Key_B, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_book" ) ) ->setToolTip( i18n( "Add a book to the BibTeX file" ) );
    ( new KAction( i18n( "Booklet" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_booklet" ) ) ->setToolTip( i18n( "Add a booklet to the BibTeX file" ) );
    ( new KAction( i18n( "Electronic (IEEE)" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_electronic" ) ) ->setToolTip( i18n( "Add an electronic reference (IEEE) to the BibTeX file" ) );
    ( new KAction( i18n( "InBook" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_inbook" ) ) ->setToolTip( i18n( "Add a part of a book to the BibTeX file" ) );
    ( new KAction( i18n( "InCollection" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_incollection" ) ) ->setToolTip( i18n( "Add a publication which is part of some collection to the BibTeX file" ) );
    ( new KAction( i18n( "InProceedings" ), "element_new_entry_inproceedings", CTRL + Key_I, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_inproceedings" ) ) ->setToolTip( i18n( "Add a publication which is part of some proceedings to the BibTeX file" ) );
    ( new KAction( i18n( "Manual" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_manual" ) ) ->setToolTip( i18n( "Add a manual to the BibTeX file" ) );
    ( new KAction( i18n( "MastersThesis" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_mastersthesis" ) ) ->setToolTip( i18n( "Add a master or diploma thesis to the BibTeX file" ) );
    ( new KAction( i18n( "Misc" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_misc" ) ) ->setToolTip( i18n( "Add a not specified document or reference to the BibTeX file" ) );
    ( new KAction( i18n( "PhDThesis" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_phdthesis" ) ) ->setToolTip( i18n( "Add a PhD thesis to the BibTeX file" ) );
    ( new KAction( i18n( "Proceedings" ), "element_new_entry_phdthesis" , CTRL + Key_P, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_proceedings" ) ) ->setToolTip( i18n( "Add some proceedings to the BibTeX file" ) );
    ( new KAction( i18n( "TechReport" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_techreport" ) ) ->setToolTip( i18n( "Add a technical report to the BibTeX file" ) );
    ( new KAction( i18n( "Unpublished" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_entry_unpublished" ) ) ->setToolTip( i18n( "Add an unpublished document to the BibTeX file" ) );
    ( new KAction( i18n( "Preamble" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_preamble" ) ) ->setToolTip( i18n( "Add a preamble to the BibTeX file" ) );
    ( new KAction( i18n( "Comment" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_comment" ) ) ->setToolTip( i18n( "Add a comment to the BibTeX file" ) );
    ( new KAction( i18n( "Macro" ), 0, this, SLOT( slotNewElement() ), actionCollection(), "element_new_macro" ) ) ->setToolTip( i18n( "Add a macro to the BibTeX file" ) );

    m_actionEditElement = new KAction( i18n( "&Edit" ), 0, m_documentWidget, SLOT( editElement() ), actionCollection(), "element_edit" );
    m_actionEditElement->setToolTip( i18n( "Edit the selected BibTeX element" ) );
    m_actionEditElement->setEnabled( FALSE );
    m_actionDeleteElement = new KAction( i18n( "&Delete" ), 0, m_documentWidget, SLOT( deleteElements() ), actionCollection(), "element_delete" );
    m_actionDeleteElement->setToolTip( i18n( "Delete the selected BibTeX elements" ) );
    m_actionDeleteElement->setEnabled( FALSE );

    m_actionShowComments = new KToggleAction( i18n( "Show &Comments" ), 0, this, SLOT( slotToggleShowSpecialElements() ), actionCollection(), "view_showcomments" );
    m_actionShowComments->setToolTip( i18n( "Show the comments of a BibTeX document" ) );
    m_actionShowMacros = new KToggleAction( i18n( "Show &Macros" ), 0, this, SLOT( slotToggleShowSpecialElements() ), actionCollection(), "view_showmacros" );
    m_actionShowMacros->setToolTip( i18n( "Show the macros of a BibTeX document" ) );

    m_actionUseInPipe = new KToggleAction( i18n( "Accept input from other programs" ), 0, this , SLOT( slotUseInPipe() ), actionCollection(), "useinpipe" );

    connect( m_documentWidget, SIGNAL( listViewSelectionChanged( int ) ), this, SLOT( slotUpdateMenu( int ) ) );

    if ( m_mainWindow != NULL )
    {
        KStatusBar * statusBar = m_mainWindow->statusBar();
        if ( statusBar != NULL )
        {
            connect( actionCollection(), SIGNAL( actionStatusText( const QString & ) ), statusBar, SLOT( message( const QString & ) ) );
            connect( actionCollection(), SIGNAL( clearStatusText() ), statusBar, SLOT( clear() ) );
        }
    }

    actionCollection() ->setHighlightingEnabled( true );

    KActionMenu *acmElementNew = new KActionMenu( i18n( "&New" ), QIconSet( SmallIcon( "filenew" ) ), actionCollection(), "element_new" );
    acmElementNew->setToolTip( i18n( "Add a new element to this BibTeX document" ) );
    acmElementNew->setEnabled( FALSE );
}

bool KBibTeXPart::slotNewElement()
{
    if ( isReadWrite() && sender() )
    {
        QString elementType;

        if ( strncmp( sender() ->name(), "element_new_entry_", 18 ) == 0 )
            elementType = QString( sender() ->name() + 18 );
        else if ( strcmp( sender() ->name(), "element_new_comment" ) == 0 )
            elementType = "comment";
        else if ( strcmp( sender() ->name(), "element_new_macro" ) == 0 )
            elementType = "macro";
        else if ( strcmp( sender() ->name(), "element_new_preamble" ) == 0 )
            elementType = "preamble";
        else
            return FALSE;

        bool result = m_documentWidget->newElement( elementType );
        if ( result )
            setModified( TRUE );
        return result;
    }

    return FALSE;
}

void KBibTeXPart::readSettings()
{
    KBibTeX::Settings * settings = KBibTeX::Settings::self();
    KConfig * config = KBibTeXPartFactory::instance() ->config();
    settings->load( config );

    m_actionShowComments->setChecked( settings->editing_ShowComments );
    m_actionShowMacros->setChecked( settings->editing_ShowMacros );
}

void KBibTeXPart::writeSettings()
{
    if ( !m_initializationDone )
        return ;

    KBibTeX::Settings * settings = KBibTeX::Settings::self();
    m_documentWidget->saveState();

    KConfig * config = KBibTeXPartFactory::instance() ->config();

    settings->editing_ShowComments = m_actionShowComments->isChecked();
    settings->editing_ShowMacros = m_actionShowMacros->isChecked();

    settings->save( config );

    config->sync();
}

void KBibTeXPart::slotUpdateMenu( int numSelectedItems )
{
    m_documentWidget->updateViewDocumentMenu();
    m_documentWidget->updateAssignKeywords();

    m_actionEditElement->setEnabled( numSelectedItems == 1 );
    m_actionDeleteElement->setEnabled( isReadWrite() && numSelectedItems > 0 );
    m_actionEditCut->setEnabled( isReadWrite() && numSelectedItems > 0 );
    m_actionEditCopy->setEnabled( numSelectedItems > 0 );
    m_actionEditCopyRef->setEnabled( numSelectedItems > 0 );
    m_actionElementSendToLyX->setEnabled( numSelectedItems > 0 );
    m_actionMenuSearchWebsites->setEnabled( numSelectedItems == 1 );
    m_actionViewDocument->setEnabled( numSelectedItems == 1 && m_actionViewDocument->popupMenu() ->count() > 0 );
    m_actionAssignKeywords->setEnabled( numSelectedItems == 1 );
}

void KBibTeXPart::slotUndoChanged( bool undoAvailable )
{
    m_actionEditUndo->setEnabled( undoAvailable );
}

void KBibTeXPart::slotUseInPipe()
{
    if ( m_inPipe == NULL && QFile::exists( inPipeFilename ) )
    {
        KMessageBox::error( widget(), QString( i18n( "Some other KBibTeX instance is using the pipe.\nIf this assumption is wrong, please delete '%1'." ) ).arg( inPipeFilename ), i18n( "Pipe already in use" ) );
    }
    else
    {
        if ( m_inPipe != NULL )
        {
// remove pipe
            m_inPipe->close();
            m_inPipe->remove();
            delete m_inPipe;
            m_inPipe = NULL;
            QFile::remove( inPipeFilename );
        }
        else
        {
// create pipe
            if ( ::mkfifo( inPipeFilename.latin1(), 0600 ) == 0 )
            {
                m_inPipe = new QFile( inPipeFilename );
                if ( !m_inPipe->open( IO_ReadOnly ) || fcntl( m_inPipe->handle(), F_SETFL, O_NONBLOCK ) < 0 )
                {
                    m_inPipe->close();
                    m_inPipe = NULL;
                }
                if ( m_inPipe == NULL )
                    QFile::remove( inPipeFilename );
            }

            if ( m_inPipe == NULL )
                KMessageBox::error( widget(), QString( i18n( "Could not create pipe at '%1'." ) ).arg( inPipeFilename ), i18n( "Error creating pipe" ) );
        }
    }

    m_actionUseInPipe->setChecked( m_inPipe != NULL );
}

void KBibTeXPart::slotSearchWebsites( int id )
{
    KBibTeX::Settings * settings = KBibTeX::Settings::self();
    m_documentWidget->searchWebsites( settings->searchURLs[ id - 1 ] ->url, settings->searchURLs[ id - 1 ] ->includeAuthor );
}

void KBibTeXPart::slotToggleShowSpecialElements()
{
    KBibTeX::Settings * settings = KBibTeX::Settings::self();
    settings->editing_ShowComments = m_actionShowComments->isChecked();
    settings->editing_ShowMacros = m_actionShowMacros->isChecked();

    m_documentWidget->updateViews();
}

bool KBibTeXPart::save()
{
    if ( !url().isValid() || url().isEmpty() )
        return saveAs();
    else
        return KParts::ReadWritePart::save();
}

void KBibTeXPart::slotDeferredInitialization()
{
    if ( factory() != NULL )
    {
        m_documentWidget->setFactory( factory(), this );
        m_documentWidget->deferredInitialization();
        m_initializationDone = TRUE;
    }
    else
    {
        m_defInitCounter++;

        if ( m_defInitCounter > 5 )
        {
            KMessageBox::sorry( widget(), i18n( "The KBibTeX part has problems to initialize itself. Only limited functionality will be available." ), i18n( "KBibTeX Part" ) );
            return ;
        }
        QTimer::singleShot( 250, this, SLOT( slotDeferredInitialization() ) );
    }
}

// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance* KBibTeXPartFactory::s_instance = 0L;
KAboutData* KBibTeXPartFactory::s_about = 0L;

KBibTeXPartFactory::KBibTeXPartFactory()
        : KParts::Factory()
{
    // nothing
}

KBibTeXPartFactory::~KBibTeXPartFactory()
{
    delete s_instance;
    delete s_about;

    s_instance = 0L;
}

KParts::Part* KBibTeXPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
        QObject *parent, const char *name,
        const char *classname, const QStringList & /* args */ )
{
    // Create an instance of our Part
    KBibTeXPart * obj = new KBibTeXPart( parentWidget, widgetName, parent, name );

    // See if we are to be read-write or not
    if ( QCString( classname ) == "KParts::ReadOnlyPart" )
        obj->setReadWrite( false );

    return obj;
}

KInstance* KBibTeXPartFactory::instance()
{
    if ( !s_instance )
    {
        s_about = new KAboutData( "kbibtexpart", I18N_NOOP( "KBibTeXPart" ), "0.2.1" );
        s_about->addAuthor( "Thomas Fischer", 0, "fischer@unix-ag.uni-kl.de" );
        s_instance = new KInstance( s_about );
    }
    return s_instance;
}

extern "C"
{
    void* init_libkbibtexpart()
    {
        return new KBibTeXPartFactory;
    }
};

#include "kbibtex_part.moc"

