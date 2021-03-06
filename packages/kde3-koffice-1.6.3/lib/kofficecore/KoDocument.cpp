/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoDocument.h"

#include "KoDocument_p.h"
#include "KoDocumentIface.h"
#include "KoDocumentChild.h"
#include "KoView.h"
#include "KoMainWindow.h"
#include "KoFilterManager.h"
#include "KoDocumentInfo.h"
#include "KoOasisStyles.h"
#include "KoOasisStore.h"
#include "KoXmlNS.h"
#include "KoOpenPane.h"

#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kfileitem.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kparts/partmanager.h>
#include <kprinter.h>
#include <ksavefile.h>

#include <qbuffer.h>
#include <qcursor.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qmap.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qxml.h>
#include <qlayout.h>

#include <config.h>
#include <assert.h>
#include <locale.h>


// Define the protocol used here for embedded documents' URL
// This used to "store" but KURL didn't like it,
// so let's simply make it "tar" !
#define STORE_PROTOCOL "tar"
// The internal path is a hack to make KURL happy and still pass
// some kind of relative path to KoDocumentChild
#define INTERNAL_PROTOCOL "intern"
#define INTERNAL_PREFIX "intern:/"
// Warning, keep it sync in koStore.cc and koDocumentChild.cc

QPtrList<KoDocument> *KoDocument::s_documentList=0L;

using namespace std;
class KoViewWrapperWidget;

/**********************************************************
 *
 * KoDocument
 *
 **********************************************************/

const int KoDocument::s_defaultAutoSave = 300; // 5 minutes

class KoDocument::Private
{
public:
    Private() :
        m_dcopObject( 0L ),
        filterManager( 0L ),
        m_specialOutputFlag( 0 ), // default is native format
        m_isImporting( false ), m_isExporting( false ),
        m_numOperations( 0 ),
        modifiedAfterAutosave( false ),
        m_autosaving( false ),
        m_shouldCheckAutoSaveFile( true ),
        m_autoErrorHandlingEnabled( true ),
        m_backupFile( true ),
        m_backupPath( QString::null ),
        m_doNotSaveExtDoc( false ),
        m_current( false ),
        m_storeInternal( false ),
        m_bLoading( false ),
        m_startUpWidget( 0 )
    {
        m_confirmNonNativeSave[0] = true;
        m_confirmNonNativeSave[1] = true;
        if ( KGlobal::locale()->measureSystem() == KLocale::Imperial ) {
            m_unit = KoUnit::U_INCH;
        } else {
            m_unit = KoUnit::U_CM;
        }
    }

    QPtrList<KoView> m_views;
    QPtrList<KoDocumentChild> m_children;
    QPtrList<KoMainWindow> m_shells;
    QValueList<QDomDocument> m_viewBuildDocuments;

    KoViewWrapperWidget *m_wrapperWidget;
    KoDocumentIface * m_dcopObject;
    KoDocumentInfo *m_docInfo;
    KoView* hitTestView;

    KoUnit::Unit m_unit;

    KoFilterManager * filterManager; // The filter-manager to use when loading/saving [for the options]

    QCString mimeType; // The actual mimetype of the document
    QCString outputMimeType; // The mimetype to use when saving
    bool m_confirmNonNativeSave [2]; // used to pop up a dialog when saving for the
                                     // first time if the file is in a foreign format
                                     // (Save/Save As, Export)
    int m_specialOutputFlag; // See KoFileDialog in koMainWindow.cc
    bool m_isImporting, m_isExporting; // File --> Import/Export vs File --> Open/Save

    QTimer m_autoSaveTimer;
    QString lastErrorMessage; // see openFile()
    int m_autoSaveDelay; // in seconds, 0 to disable.
    int m_numOperations;
    bool modifiedAfterAutosave;
    bool m_bSingleViewMode;
    bool m_autosaving;
    bool m_shouldCheckAutoSaveFile; // usually true
    bool m_autoErrorHandlingEnabled; // usually true
    bool m_backupFile;
    QString m_backupPath;
    bool m_doNotSaveExtDoc; // makes it possible to save only internally stored child documents
    bool m_current;
    bool m_storeInternal; // Store this doc internally even if url is external
    bool m_bLoading; // True while loading (openURL is async)

    KoOpenPane* m_startUpWidget;
    QString m_templateType;
};

// Used in singleViewMode
class KoViewWrapperWidget : public QWidget
{
public:
    KoViewWrapperWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
    {
        KGlobal::locale()->insertCatalogue("koffice");
        // Tell the iconloader about share/apps/koffice/icons
        KGlobal::iconLoader()->addAppDir("koffice");
        m_view = 0L;
        // Avoid warning from KParts - we'll have the KoView as focus proxy anyway
        setFocusPolicy( ClickFocus );
    }

    virtual ~KoViewWrapperWidget() {
        setFocusProxy( 0 ); // to prevent a crash due to clearFocus (#53466)
    }

    virtual void resizeEvent( QResizeEvent * )
    {
        QObject *wid = child( 0, "QWidget" );
        if ( wid )
            static_cast<QWidget *>(wid)->setGeometry( 0, 0, width(), height() );
    }

    virtual void childEvent( QChildEvent *ev )
    {
        if ( ev->type() == QEvent::ChildInserted )
            resizeEvent( 0L );
    }

    // Called by openFile()
    void setKoView( KoView * view ) {
        m_view = view;
        setFocusProxy( m_view );
    }
    KoView * koView() const { return m_view; }
private:
    KoView* m_view;
};

KoBrowserExtension::KoBrowserExtension( KoDocument * doc, const char * name )
    : KParts::BrowserExtension( doc, name )
{
    emit enableAction( "print", true );
}

void KoBrowserExtension::print()
{
    KoDocument * doc = static_cast<KoDocument *>( parent() );
    KoViewWrapperWidget * wrapper = static_cast<KoViewWrapperWidget *>( doc->widget() );
    KoView * view = wrapper->koView();
    // TODO remove code duplication (KoMainWindow), by moving this to KoView
    KPrinter printer;
    // ### TODO: apply global koffice settings here
    view->setupPrinter( printer );
    if ( printer.setup( view ) )
        view->print( printer );
}

KoDocument::KoDocument( QWidget * parentWidget, const char *widgetName, QObject* parent, const char* name, bool singleViewMode )
    : KParts::ReadWritePart( parent, name )
{
    if(s_documentList==0L)
        s_documentList=new QPtrList<KoDocument>;
    s_documentList->append(this);

    d = new Private;
    m_bEmpty = TRUE;
    connect( &d->m_autoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
    setAutoSave( s_defaultAutoSave );
    d->m_bSingleViewMode = singleViewMode;


    // the parent setting *always* overrides! (Simon)
    if ( parent )
    {
        if ( parent->inherits( "KoDocument" ) )
            d->m_bSingleViewMode = ((KoDocument *)parent)->isSingleViewMode();
        else if ( parent->inherits( "KParts::Part" ) )
            d->m_bSingleViewMode = true;
    }

    if ( singleViewMode )
    {
        d->m_wrapperWidget = new KoViewWrapperWidget( parentWidget, widgetName );
        setWidget( d->m_wrapperWidget );
        kdDebug(30003) << "creating KoBrowserExtension" << endl;
        (void) new KoBrowserExtension( this ); // ## only if embedded into a browser?
    }

    d->m_docInfo = new KoDocumentInfo( this, "document info" );

    m_pageLayout.ptWidth = 0;
    m_pageLayout.ptHeight = 0;
    m_pageLayout.ptTop = 0;
    m_pageLayout.ptBottom = 0;
    m_pageLayout.ptLeft = 0;
    m_pageLayout.ptRight = 0;

    // A way to 'fix' the job's window, since we have no widget known to KParts
    if ( !singleViewMode )
        connect( this, SIGNAL( started( KIO::Job* ) ), SLOT( slotStarted( KIO::Job* ) ) );
}

KoDocument::~KoDocument()
{
    d->m_autoSaveTimer.stop();

    QPtrListIterator<KoDocumentChild> childIt( d->m_children );
    for (; childIt.current(); ++childIt )
        disconnect( childIt.current(), SIGNAL( destroyed() ),
                    this, SLOT( slotChildDestroyed() ) );

    // Tell our views that the document is already destroyed and
    // that they shouldn't try to access it.
    QPtrListIterator<KoView> vIt( d->m_views );
    for (; vIt.current(); ++vIt )
        vIt.current()->setDocumentDeleted();

    delete d->m_startUpWidget;
    d->m_startUpWidget = 0;

    d->m_children.setAutoDelete( true );
    d->m_children.clear();

    d->m_shells.setAutoDelete( true );
    d->m_shells.clear();

    delete d->m_dcopObject;
    delete d->filterManager;
    delete d;
    s_documentList->removeRef(this);
    // last one?
    if(s_documentList->isEmpty()) {
        delete s_documentList;
        s_documentList=0;
    }
}

bool KoDocument::isSingleViewMode() const
{
    return d->m_bSingleViewMode;
}

bool KoDocument::isEmbedded() const
{
    return dynamic_cast<KoDocument *>( parent() ) != 0;
}

KoView *KoDocument::createView( QWidget *parent, const char *name )
{
    KoView *view=createViewInstance(parent, name);
    addView(view);
    return view;
}

bool KoDocument::exp0rt( const KURL & _url )
{
    bool ret;

    d->m_isExporting = true;

    //
    // Preserve a lot of state here because we need to restore it in order to
    // be able to fake a File --> Export.  Can't do this in saveFile() because,
    // for a start, KParts has already set m_url and m_file and because we need
    // to restore the modified flag etc. and don't want to put a load on anyone
    // reimplementing saveFile() (Note: import() and export() will remain
    // non-virtual).
    //
    KURL oldURL = m_url;
    QString oldFile = m_file;

    bool wasModified = isModified ();
    QCString oldMimeType = mimeType ();


    // save...
    ret = saveAs( _url );


    //
    // This is sooooo hacky :(
    // Hopefully we will restore enough state.
    //
    kdDebug(30003) << "Restoring KoDocument state to before export" << endl;

    // always restore m_url & m_file because KParts has changed them
    // (regardless of failure or success)
    m_url = oldURL;
    m_file = oldFile;

    // on successful export we need to restore modified etc. too
    // on failed export, mimetype/modified hasn't changed anyway
    if (ret)
    {
        setModified (wasModified);
        d->mimeType = oldMimeType;
    }


    d->m_isExporting = false;

    return ret;
}

bool KoDocument::saveFile()
{
    kdDebug(30003) << "KoDocument::saveFile() doc='" << url().url() <<"'"<< endl;
    
    // set local again as it can happen that it gets resetted
    // so that all saved numbers have a . and not e.g a , as a 
    // decimal seperator
    setlocale( LC_NUMERIC, "C" );

    // Save it to be able to restore it after a failed save
    const bool wasModified = isModified ();

    // The output format is set by koMainWindow, and by openFile
    QCString outputMimeType = d->outputMimeType;
    //Q_ASSERT( !outputMimeType.isEmpty() ); // happens when using the DCOP method saveAs
    if ( outputMimeType.isEmpty() )
        outputMimeType = d->outputMimeType = nativeFormatMimeType();

    QApplication::setOverrideCursor( waitCursor );

    if ( backupFile() ) {
        if ( url().isLocalFile() )
            KSaveFile::backupFile( url().path(), d->m_backupPath );
        else {
            KIO::UDSEntry entry;
            if ( KIO::NetAccess::stat( url(), entry, shells().current() ) ) { // this file exists => backup
                emit sigStatusBarMessage( i18n("Making backup...") );
                KURL backup;
                if ( d->m_backupPath.isEmpty())
                    backup = url();
                else
                    backup = d->m_backupPath +"/"+url().fileName();
                backup.setPath( backup.path() + QString::fromLatin1("~") );
                KFileItem item( entry, url() );
                Q_ASSERT( item.name() == url().fileName() );
                KIO::NetAccess::file_copy( url(), backup, item.permissions(), true /*overwrite*/, false /*resume*/, shells().current() );
            }
        }
    }

    emit sigStatusBarMessage( i18n("Saving...") );
    bool ret = false;
    bool suppressErrorDialog = false;
    if ( !isNativeFormat( outputMimeType ) ) {
        kdDebug(30003) << "Saving to format " << outputMimeType << " in " << m_file << endl;
        // Not native format : save using export filter
        if ( !d->filterManager )
            d->filterManager = new KoFilterManager( this );

        KoFilter::ConversionStatus status = d->filterManager->exp0rt( m_file, outputMimeType );
        ret = status == KoFilter::OK;
        suppressErrorDialog = (status == KoFilter::UserCancelled || status == KoFilter::BadConversionGraph );
    } else {
        // Native format => normal save
        Q_ASSERT( !m_file.isEmpty() );
        ret = saveNativeFormat( m_file );
    }

    if ( ret ) {
        removeAutoSaveFiles();
        // Restart the autosave timer
        // (we don't want to autosave again 2 seconds after a real save)
        setAutoSave( d->m_autoSaveDelay );
    }

    QApplication::restoreOverrideCursor();
    if ( !ret )
    {
        if ( !suppressErrorDialog )
        {
            showSavingErrorDialog();
        }

        // couldn't save file so this new URL is invalid
        // FIXME: we should restore the current document's true URL instead of
        // setting it to nothing otherwise anything that depends on the URL
        // being correct will not work (i.e. the document will be called
        // "Untitled" which may not be true)
        //
        // Update: now the URL is restored in KoMainWindow but really, this
        // should still be fixed in KoDocument/KParts (ditto for m_file).
        // We still resetURL() here since we may or may not have been called
        // by KoMainWindow - Clarence
        resetURL();

	// As we did not save, restore the "was modified" status
        setModified( wasModified );
    }

    if ( ret )
    {
        d->mimeType = outputMimeType;
        setConfirmNonNativeSave ( isExporting (), false );
    }
    emit sigClearStatusBarMessage();

    return ret;
}

QCString KoDocument::mimeType() const
{
    return d->mimeType;
}

void KoDocument::setMimeType( const QCString & mimeType )
{
    d->mimeType = mimeType;
}

void KoDocument::setOutputMimeType( const QCString & mimeType, int specialOutputFlag )
{
    d->outputMimeType = mimeType;
    d->m_specialOutputFlag = specialOutputFlag;
}

QCString KoDocument::outputMimeType() const
{
    return d->outputMimeType;
}

int KoDocument::specialOutputFlag() const
{
    return d->m_specialOutputFlag;
}

bool KoDocument::confirmNonNativeSave( const bool exporting ) const
{
    // "exporting ? 1 : 0" is different from "exporting" because a bool is
    // usually implemented like an "int", not "unsigned : 1"
    return d->m_confirmNonNativeSave [ exporting ? 1 : 0 ];
}

void KoDocument::setConfirmNonNativeSave( const bool exporting, const bool on )
{
    d->m_confirmNonNativeSave [ exporting ? 1 : 0] = on;
}

bool KoDocument::wantExportConfirmation() const
{
    return true;
}

bool KoDocument::isImporting() const
{
    return d->m_isImporting;
}

bool KoDocument::isExporting() const
{
    return d->m_isExporting;
}

void KoDocument::setCheckAutoSaveFile( bool b )
{
    d->m_shouldCheckAutoSaveFile = b;
}

void KoDocument::setAutoErrorHandlingEnabled( bool b )
{
    d->m_autoErrorHandlingEnabled = b;
}

bool KoDocument::isAutoErrorHandlingEnabled() const
{
    return d->m_autoErrorHandlingEnabled;
}

void KoDocument::slotAutoSave()
{
    if ( isModified() && d->modifiedAfterAutosave && !d->m_bLoading )
    {
        connect( this, SIGNAL( sigProgress( int ) ), shells().current(), SLOT( slotProgress( int ) ) );
        emit sigStatusBarMessage( i18n("Autosaving...") );
        d->m_autosaving = true;
        bool ret = saveNativeFormat( autoSaveFile( m_file ) );
        setModified( true );
        if ( ret ) {
            d->modifiedAfterAutosave = false;
            d->m_autoSaveTimer.stop(); // until the next change
        }
        d->m_autosaving = false;
        emit sigClearStatusBarMessage();
        disconnect( this, SIGNAL( sigProgress( int ) ), shells().current(), SLOT( slotProgress( int ) ) );
        if ( !ret )
            emit sigStatusBarMessage( i18n("Error during autosave! Partition full?") );
    }
}

KAction *KoDocument::action( const QDomElement &element ) const
{
    // First look in the document itself
    KAction* act = KParts::ReadWritePart::action( element );
    if ( act )
        return act;

    Q_ASSERT( d->m_bSingleViewMode );
    // Then look in the first view (this is for the single view mode)
    if ( !d->m_views.isEmpty() )
        return d->m_views.getFirst()->action( element );
    else
        return 0L;
}

QDomDocument KoDocument::domDocument() const
{
    // When embedded into e.g. konqueror, we want the view's GUI (hopefully a reduced one)
    // to be used.
    Q_ASSERT( d->m_bSingleViewMode );
    if ( d->m_views.isEmpty() )
        return QDomDocument();
    else
        return d->m_views.getFirst()->domDocument();
}

void KoDocument::setManager( KParts::PartManager *manager )
{
    KParts::ReadWritePart::setManager( manager );
    if ( d->m_bSingleViewMode && d->m_views.count() == 1 )
        d->m_views.getFirst()->setPartManager( manager );

    if ( manager )
    {
        QPtrListIterator<KoDocumentChild> it( d->m_children );
        for (; it.current(); ++it )
            if ( it.current()->document() )
                manager->addPart( it.current()->document(), false );
    }
}

void KoDocument::setReadWrite( bool readwrite )
{
    KParts::ReadWritePart::setReadWrite( readwrite );

    QPtrListIterator<KoView> vIt( d->m_views );
    for (; vIt.current(); ++vIt )
        vIt.current()->updateReadWrite( readwrite );

    QPtrListIterator<KoDocumentChild> dIt( d->m_children );
    for (; dIt.current(); ++dIt )
        if ( dIt.current()->document() )
            dIt.current()->document()->setReadWrite( readwrite );

    setAutoSave( d->m_autoSaveDelay );
}

void KoDocument::setAutoSave( int delay )
{
    d->m_autoSaveDelay = delay;
    if ( isReadWrite() && !isEmbedded() && d->m_autoSaveDelay > 0 )
        d->m_autoSaveTimer.start( d->m_autoSaveDelay * 1000 );
    else
        d->m_autoSaveTimer.stop();
}

void KoDocument::addView( KoView *view )
{
    if ( !view )
        return;

    d->m_views.append( view );
    view->updateReadWrite( isReadWrite() );
}

void KoDocument::removeView( KoView *view )
{
    d->m_views.removeRef( view );
}

const QPtrList<KoView>& KoDocument::views() const
{
    return d->m_views;
}

int KoDocument::viewCount() const
{
    return d->m_views.count();
}

void KoDocument::insertChild( KoDocumentChild *child )
{
    setModified( true );

    d->m_children.append( child );

    connect( child, SIGNAL( changed( KoChild * ) ),
             this, SLOT( slotChildChanged( KoChild * ) ) );
    connect( child, SIGNAL( destroyed() ),
             this, SLOT( slotChildDestroyed() ) );

    // It may be that insertChild is called without the KoDocumentChild
    // having a KoDocument attached, yet. This happens for example
    // when KPresenter loads a document with embedded objects. For those
    // KPresenterChild objects are allocated and insertChild is called.
    // Later in loadChildren() KPresenter iterates over the child list
    // and calls loadDocument for each child. That's exactly where we
    // will try to do what we cannot do now: Register the child document
    // at the partmanager (Simon)
    if ( manager() && !isSingleViewMode() && child->document() )
        manager()->addPart( child->document(), false );
}

void KoDocument::slotChildChanged( KoChild *c )
{
    assert( c->inherits( "KoDocumentChild" ) );
    emit childChanged( static_cast<KoDocumentChild *>( c ) );
}

void KoDocument::slotChildDestroyed()
{
    setModified( true );

    const KoDocumentChild *child = static_cast<const KoDocumentChild *>( sender() );
    d->m_children.removeRef( child );
}

const QPtrList<KoDocumentChild>& KoDocument::children() const
{
    return d->m_children;
}

KParts::Part *KoDocument::hitTest( QWidget *widget, const QPoint &globalPos )
{
    QPtrListIterator<KoView> it( d->m_views );
    for (; it.current(); ++it )
        if ( static_cast<QWidget *>(it.current()) == widget )
        {
            KoView* view = it.current();
            d->hitTestView = view; // koffice-1.x hack
            QPoint canvasPos( view->canvas()->mapFromGlobal( globalPos ) );
            canvasPos.rx() += view->canvasXOffset();
            canvasPos.ry() += view->canvasYOffset();

            KParts::Part *part = view->hitTest( canvasPos );
            d->hitTestView = 0;
            if ( part )
                return part;
        }

    return 0L;
}

KoView* KoDocument::hitTestView()
{
    return d->hitTestView;
}

KoDocument* KoDocument::hitTest( const QPoint &pos, const QWMatrix &matrix )
{
    // Call KoDocumentChild::hitTest for any child document
    QPtrListIterator<KoDocumentChild> it( d->m_children );
    for (; it.current(); ++it )
    {
        KoDocument *doc = it.current()->hitTest( pos, matrix );
        if ( doc )
            return doc;
    }

    // Unless we hit an embedded document, the hit is on this document itself.
    return this;
}

KoDocumentChild *KoDocument::child( KoDocument *doc )
{
    QPtrListIterator<KoDocumentChild> it( d->m_children );
    for (; it.current(); ++it )
        if ( it.current()->document() == doc )
            return it.current();

    return 0L;
}

KoDocumentInfo *KoDocument::documentInfo() const
{
    return d->m_docInfo;
}

void KoDocument::setViewBuildDocument( KoView *view, const QDomDocument &doc )
{
    if ( d->m_views.find( view ) == -1 )
        return;

    uint viewIdx = d->m_views.at();

    if ( d->m_viewBuildDocuments.count() == viewIdx )
        d->m_viewBuildDocuments.append( doc );
    else if ( d->m_viewBuildDocuments.count() > viewIdx )
        d->m_viewBuildDocuments[ viewIdx ] = doc;
}

QDomDocument KoDocument::viewBuildDocument( KoView *view )
{
    QDomDocument res;

    if ( d->m_views.find( view ) == -1 )
        return res;

    uint viewIdx = d->m_views.at();

    if ( viewIdx >= d->m_viewBuildDocuments.count() )
        return res;

    res = d->m_viewBuildDocuments[ viewIdx ];

    // make this entry empty. otherwise we get a segfault in QMap ;-(
    d->m_viewBuildDocuments[ viewIdx ] = QDomDocument();

    return res;
}

void KoDocument::paintEverything( QPainter &painter, const QRect &rect, bool transparent, KoView *view, double zoomX, double zoomY )
{
    paintContent( painter, rect, transparent, zoomX, zoomY );
    paintChildren( painter, rect, view, zoomX, zoomY );
}

void KoDocument::paintChildren( QPainter &painter, const QRect &/*rect*/, KoView *view, double zoomX, double zoomY )
{
    QPtrListIterator<KoDocumentChild> it( d->m_children );
    for (; it.current(); ++it )
    {
        // #### todo: paint only if child is visible inside rect
        painter.save();
        paintChild( it.current(), painter, view, zoomX, zoomY );
        painter.restore();
    }
}

void KoDocument::paintChild( KoDocumentChild *child, QPainter &painter, KoView *view, double zoomX, double zoomY )
{
    if ( child->isDeleted() )
        return;

    // QRegion rgn = painter.clipRegion();

    child->transform( painter );
    child->document()->paintEverything( painter, child->contentRect(), child->isTransparent(), view, zoomX, zoomY );

    if ( view && view->partManager() )
    {
        // ### do we need to apply zoomX and zoomY here ?
        KParts::PartManager *manager = view->partManager();

        painter.scale( 1.0 / child->xScaling(), 1.0 / child->yScaling() );

        int w = int( (double)child->contentRect().width() * child->xScaling() );
        int h = int( (double)child->contentRect().height() * child->yScaling() );
        if ( ( manager->selectedPart() == (KParts::Part *)child->document() &&
               manager->selectedWidget() == (QWidget *)view ) ||
             ( manager->activePart() == (KParts::Part *)child->document() &&
               manager->activeWidget() == (QWidget *)view ) )
        {
            // painter.setClipRegion( rgn );
            painter.setClipping( FALSE );

            painter.setPen( black );
            painter.fillRect( -5, -5, w + 10, 5, white );
            painter.fillRect( -5, h, w + 10, 5, white );
            painter.fillRect( -5, -5, 5, h + 10, white );
            painter.fillRect( w, -5, 5, h + 10, white );
            painter.fillRect( -5, -5, w + 10, 5, BDiagPattern );
            painter.fillRect( -5, h, w + 10, 5, BDiagPattern );
            painter.fillRect( -5, -5, 5, h + 10, BDiagPattern );
            painter.fillRect( w, -5, 5, h + 10, BDiagPattern );

            if ( manager->selectedPart() == (KParts::Part *)child->document() &&
                 manager->selectedWidget() == (QWidget *)view )
            {
                QColor color;
                if ( view->koDocument() == this )
                    color = black;
                else
                    color = gray;
                painter.fillRect( -5, -5, 5, 5, color );
                painter.fillRect( -5, h, 5, 5, color );
                painter.fillRect( w, h, 5, 5, color );
                painter.fillRect( w, -5, 5, 5, color );
                painter.fillRect( w / 2 - 3, -5, 5, 5, color );
                painter.fillRect( w / 2 - 3, h, 5, 5, color );
                painter.fillRect( -5, h / 2 - 3, 5, 5, color );
                painter.fillRect( w, h / 2 - 3, 5, 5, color );
            }

            painter.setClipping( TRUE );
        }
    }
}

bool KoDocument::isModified() const
{
    if ( KParts::ReadWritePart::isModified() )
    {
        //kdDebug(30003)<<k_funcinfo<<" Modified doc='"<<url().url()<<"' extern="<<isStoredExtern()<<endl;
        return true;
    }
    // Then go through internally stored children (considered to be part of this doc)
    QPtrListIterator<KoDocumentChild> it = children();
    for (; it.current(); ++it )
    {
        KoDocument *doc = it.current()->document();
        if ( doc && !it.current()->isStoredExtern() && !it.current()->isDeleted() && doc->isModified() )
            return true;
    }
    return false;
}

bool KoDocument::saveChildren( KoStore* _store )
{
    //kdDebug(30003)<<k_funcinfo<<" checking children of doc='"<<url().url()<<"'"<<endl;
    int i = 0;
    QPtrListIterator<KoDocumentChild> it( children() );
    for( ; it.current(); ++it ) {
        KoDocument* childDoc = it.current()->document();
        if (childDoc && !it.current()->isDeleted())
        {
            if ( !childDoc->isStoredExtern() )
            {
                //kdDebug(30003) << "KoDocument::saveChildren internal url: /" << i << endl;
                if ( !childDoc->saveToStore( _store, QString::number( i++ ) ) )
                    return FALSE;

                if (!isExporting ())
                    childDoc->setModified( false );
            }
            //else kdDebug(30003)<<k_funcinfo<<" external (don't save) url:" << childDoc->url().url()<<endl;
        }
    }
    return true;
}

bool KoDocument::saveChildrenOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
    //kdDebug(30003)<<k_funcinfo<<" checking children of doc='"<<url().url()<<"'"<<endl;
    QPtrListIterator<KoDocumentChild> it( children() );
    for( ; it.current(); ++it ) {
        KoDocument* childDoc = it.current()->document();
        if ( childDoc && !it.current()->isDeleted() )
        {
            if ( !it.current()->saveOasis( store, manifestWriter ) )
                return false;
            if ( !childDoc->isStoredExtern() && !isExporting () )
                childDoc->setModified( false );
        }
    }
    return true;
}

bool KoDocument::saveExternalChildren()
{
    if ( d->m_doNotSaveExtDoc )
    {
        //kdDebug(30003)<<k_funcinfo<<" Don't save external docs in doc='"<<url().url()<<"'"<<endl;
        d->m_doNotSaveExtDoc = false;
        return true;
    }

    //kdDebug(30003)<<k_funcinfo<<" checking children of doc='"<<url().url()<<"'"<<endl;
    KoDocumentChild *ch;
    QPtrListIterator<KoDocumentChild> it = children();
    for (; (ch = it.current()); ++it )
    {
        if ( !ch->isDeleted() )
        {
            KoDocument* doc = ch->document();
            if ( doc && doc->isStoredExtern() && doc->isModified() )
            {
                kdDebug(30003)<<" save external doc='"<<url().url()<<"'"<<endl;
                doc->setDoNotSaveExtDoc(); // Only save doc + it's internal children
                if ( !doc->save() )
                    return false; // error
            }
            //kdDebug(30003)<<k_funcinfo<<" not modified doc='"<<url().url()<<"'"<<endl;
            // save possible external docs inside doc
            if ( doc && !doc->saveExternalChildren() )
                return false;
        }
    }
    return true;
}

bool KoDocument::saveNativeFormat( const QString & file )
{
    d->lastErrorMessage = QString::null;
    //kdDebug(30003) << "Saving to store" << endl;

    KoStore::Backend backend = KoStore::Auto;
#if 0
    if ( d->m_specialOutputFlag == SaveAsKOffice1dot1 )
    {
        kdDebug(30003) << "Saving as KOffice-1.1 format, using a tar.gz" << endl;
        backend = KoStore::Tar; // KOffice-1.0/1.1 used tar.gz for the native mimetype
        //// TODO more backwards compat stuff (embedded docs etc.)
    }
    else
#endif
        if ( d->m_specialOutputFlag == SaveAsDirectoryStore )
    {
        backend = KoStore::Directory;
        kdDebug(30003) << "Saving as uncompressed XML, using directory store." << endl;
    }
    else if ( d->m_specialOutputFlag == SaveAsFlatXML )
    {
        kdDebug(30003) << "Saving as a flat XML file." << endl;
        QFile f( file );
        if ( f.open( IO_WriteOnly | IO_Translate ) )
        {
            bool success = saveToStream( &f );
            f.close();
            return success;
        }
        else
            return false;
    }

    kdDebug(30003) << "KoDocument::saveNativeFormat nativeFormatMimeType=" << nativeFormatMimeType() << endl;
    // OLD: bool oasis = d->m_specialOutputFlag == SaveAsOASIS;
    // OLD: QCString mimeType = oasis ? nativeOasisMimeType() : nativeFormatMimeType();
    QCString mimeType = d->outputMimeType;
    QCString nativeOasisMime = nativeOasisMimeType();
    bool oasis = !mimeType.isEmpty() && ( mimeType == nativeOasisMime || mimeType == nativeOasisMime + "-template" );
    // TODO: use std::auto_ptr or create store on stack [needs API fixing],
    // to remove all the 'delete store' in all the branches
    KoStore* store = KoStore::createStore( file, KoStore::Write, mimeType, backend );
    if ( store->bad() )
    {
        d->lastErrorMessage = i18n( "Could not create the file for saving" ); // more details needed?
        delete store;
        return false;
    }

    if ( oasis )
    {
        kdDebug(30003) << "Saving to OASIS format" << endl;
        // Tell KoStore not to touch the file names
        store->disallowNameExpansion();
        KoOasisStore oasisStore( store );
        KoXmlWriter* manifestWriter = oasisStore.manifestWriter( mimeType );

        if ( !saveOasis( store, manifestWriter ) )
        {
            kdDebug(30003) << "saveOasis failed" << endl;
            delete store;
            return false;
        }

        // Save embedded objects
        if ( !saveChildrenOasis( store, manifestWriter ) )
        {
            kdDebug(30003) << "saveChildrenOasis failed" << endl;
            delete store;
            return false;
        }

        if ( store->open( "meta.xml" ) )
        {
            if ( !d->m_docInfo->saveOasis( store ) || !store->close() ) {
                delete store;
                return false;
            }
            manifestWriter->addManifestEntry( "meta.xml", "text/xml" );
        }
        else
        {
            d->lastErrorMessage = i18n( "Not able to write '%1'. Partition full?" ).arg( "meta.xml" );
            delete store;
            return false;
        }

        if ( store->open( "Thumbnails/thumbnail.png" ) )
        {
            if ( !saveOasisPreview( store, manifestWriter ) || !store->close() ) {
                d->lastErrorMessage = i18n( "Error while trying to write '%1'. Partition full?" ).arg( "Thumbnails/thumbnail.png" );
                delete store;
                return false;
            }
            // No manifest entry!
        }
        else
        {
            d->lastErrorMessage = i18n( "Not able to write '%1'. Partition full?" ).arg( "Thumbnails/thumbnail.png" );
            delete store;
            return false;
        }

        // Write out manifest file
        if ( !oasisStore.closeManifestWriter() )
        {
            d->lastErrorMessage = i18n( "Error while trying to write '%1'. Partition full?" ).arg( "META-INF/manifest.xml" );
            delete store;
            return false;
        }

        delete store;
    }
    else
    {
        // Save internal children first since they might get a new url
        if ( !saveChildren( store ) && !oasis )
        {
            if ( d->lastErrorMessage.isEmpty() )
                d->lastErrorMessage = i18n( "Error while saving embedded documents" ); // more details needed
            delete store;
            return false;
        }

        kdDebug(30003) << "Saving root" << endl;
        if ( store->open( "root" ) )
        {
            KoStoreDevice dev( store );
            if ( !saveToStream( &dev ) || !store->close() )
            {
                kdDebug(30003) << "saveToStream failed" << endl;
                delete store;
                return false;
            }
        }
        else
        {
            d->lastErrorMessage = i18n( "Not able to write '%1'. Partition full?" ).arg( "maindoc.xml" );
            delete store;
            return false;
        }
        if ( store->open( "documentinfo.xml" ) )
        {
            QDomDocument doc = d->m_docInfo->save();
            KoStoreDevice dev( store );

            QCString s = doc.toCString(); // this is already Utf8!
            (void)dev.writeBlock( s.data(), s.size()-1 );
            (void)store->close();
        }

        if ( store->open( "preview.png" ) )
        {
            // ### TODO: missing error checking (The partition could be full!)
            savePreview( store );
            (void)store->close();
        }

        if ( !completeSaving( store ) )
        {
            delete store;
            return false;
        }
        kdDebug(30003) << "Saving done of url: " << url().url() << endl;
        delete store;
    }
    if ( !saveExternalChildren() )
    {
        return false;
    }
    return true;
}

bool KoDocument::saveToStream( QIODevice * dev )
{
    QDomDocument doc = saveXML();
    // Save to buffer
    QCString s = doc.toCString(); // utf8 already
    // We use QCString::size()-1 here, not the official QCString::length
    // It works here, as we are not modifying QCString as QByteArray
    int nwritten = dev->writeBlock( s.data(), s.size()-1 );
    if ( nwritten != (int)s.size()-1 )
        kdWarning(30003) << "KoDocument::saveToStream wrote " << nwritten << "   - expected " << s.size()-1 << endl;
    return nwritten == (int)s.size()-1;
}

// Called for embedded documents
bool KoDocument::saveToStore( KoStore* _store, const QString & _path )
{
    kdDebug(30003) << "Saving document to store " << _path << endl;

    // Use the path as the internal url
    if ( _path.startsWith( STORE_PROTOCOL ) )
        m_url = KURL( _path );
    else // ugly hack to pass a relative URI
        m_url = KURL( INTERNAL_PREFIX +  _path );

    // To make the children happy cd to the correct directory
    _store->pushDirectory();
    _store->enterDirectory( _path );

    // Save childen first since they might get a new url
    if ( !saveChildren( _store ) )
        return false;

    // In the current directory we're the king :-)
    if ( _store->open( "root" ) )
    {
        KoStoreDevice dev( _store );
        if ( !saveToStream( &dev ) )
        {
            _store->close();
            return false;
        }
        if ( !_store->close() )
            return false;
    }

    if ( !completeSaving( _store ) )
        return false;

    // Now that we're done leave the directory again
    _store->popDirectory();

    kdDebug(30003) << "Saved document to store" << endl;

    return true;
}

bool KoDocument::saveOasisPreview( KoStore* store, KoXmlWriter* manifestWriter )
{
    const QPixmap pix = generatePreview( QSize( 128, 128 ) );
    QImage preview ( pix.convertToImage().convertDepth( 32, Qt::ColorOnly ) );
    if ( !preview.hasAlphaBuffer() )
    {
        preview.setAlphaBuffer( true );
    }
    // ### TODO: freedesktop.org Thumbnail specification (date...)
    KoStoreDevice io ( store );
    if ( !io.open( IO_WriteOnly ) )
        return false;
    if ( ! preview.save( &io, "PNG", 0 ) )
        return false;
    io.close();
    manifestWriter->addManifestEntry( "Thumbnails/", "" );
    manifestWriter->addManifestEntry( "Thumbnails/thumbnail.png", "" );
    return true;
}

bool KoDocument::savePreview( KoStore* store )
{
    QPixmap pix = generatePreview(QSize(256, 256));
    // Reducing to 8bpp reduces file sizes quite a lot.
    const QImage preview ( pix.convertToImage().convertDepth( 8, Qt::AvoidDither | Qt::DiffuseDither) );
    KoStoreDevice io ( store );
    if ( !io.open( IO_WriteOnly ) )
        return false;
    if ( ! preview.save( &io, "PNG" ) ) // ### TODO What is -9 in quality terms?
        return false;
    io.close();
    return true;
}

QPixmap KoDocument::generatePreview( const QSize& size )
{
    double docWidth, docHeight;
    int pixmapSize = QMAX(size.width(), size.height());

    if (m_pageLayout.ptWidth > 1.0) {
        docWidth = m_pageLayout.ptWidth / 72 * KoGlobal::dpiX();
        docHeight = m_pageLayout.ptHeight / 72 * KoGlobal::dpiY();

    } else {
        // If we don't have a page layout, just draw the top left hand corner
        docWidth = 500.0;
        docHeight = 500.0;
    }

    double ratio = docWidth / docHeight;

    QPixmap pix;
    int previewWidth, previewHeight;
    if (ratio > 1.0)
    {
        previewWidth = (int) pixmapSize;
        previewHeight = (int) (pixmapSize / ratio);
    }
    else
    {
        previewWidth = (int) (pixmapSize * ratio);
        previewHeight = (int) pixmapSize;
    }

    pix.resize((int)docWidth, (int)docHeight);

    pix.fill( QColor( 245, 245, 245 ) );

    QRect rc(0, 0, pix.width(), pix.height());

    QPainter p;
    p.begin(&pix);
    paintEverything(p, rc, false);
    p.end();

    // ### TODO: why re-convert it to a QPixmap, when mostly it will be re-converted back to a QImage in the calling function
    pix.convertFromImage(pix.convertToImage().smoothScale(previewWidth, previewHeight));

    return pix;
}

QString KoDocument::autoSaveFile( const QString & path ) const
{
    // set local again as it can happen that it gets resetted
    // so that all saved numbers have a . and not e.g a , as a 
    // decimal seperator
    setlocale( LC_NUMERIC, "C" );

    // Using the extension allows to avoid relying on the mime magic when opening
    KMimeType::Ptr mime = KMimeType::mimeType( nativeFormatMimeType() );
    QString extension = mime->property( "X-KDE-NativeExtension" ).toString();
    if ( path.isEmpty() )
    {
        // Never saved? Use a temp file in $HOME then
        // Yes, two open unnamed docs will overwrite each other's autosave file,
        // but hmm, we can only do something if that's in the same process anyway...
        QString ret = QDir::homeDirPath() + "/." + QString::fromLatin1(instance()->instanceName()) + ".autosave" + extension;
        return ret;
    }
    else
    {
        KURL url( path );
        Q_ASSERT( url.isLocalFile() );
        QString dir = url.directory(false);
        QString filename = url.fileName();
        return dir + "." + filename + ".autosave" + extension;
    }
}

bool KoDocument::checkAutoSaveFile()
{
    QString asf = autoSaveFile( QString::null ); // the one in $HOME
    //kdDebug(30003) << "asf=" << asf << endl;
    if ( QFile::exists( asf ) )
    {
        QDateTime date = QFileInfo(asf).lastModified();
        QString dateStr = date.toString(Qt::LocalDate);
        int res = KMessageBox::warningYesNoCancel(
            0, i18n( "An autosaved file for an unnamed document exists in %1.\nThis file is dated %2\nDo you want to open it?" )
            .arg(asf, dateStr) );
        switch(res) {
        case KMessageBox::Yes : {
            KURL url;
            url.setPath( asf );
            bool ret = openURL( url );
            if ( ret )
                resetURL();
            return ret;
        }
        case KMessageBox::No :
            QFile::remove( asf );
            return false;
        default: // Cancel
            return false;
        }
    }
    return false;
}

bool KoDocument::import( const KURL & _url )
{
    bool ret;

    kdDebug (30003) << "KoDocument::import url=" << _url.url() << endl;
    d->m_isImporting = true;

    // open...
    ret = openURL (_url);

    // reset m_url & m_file (kindly? set by KParts::openURL()) to simulate a
    // File --> Import
    if (ret)
    {
        kdDebug (30003) << "KoDocument::import success, resetting url" << endl;
        resetURL ();
        setTitleModified ();
    }

    d->m_isImporting = false;

    return ret;
}

bool KoDocument::openURL( const KURL & _url )
{
    kdDebug(30003) << "KoDocument::openURL url=" << _url.url() << endl;
    d->lastErrorMessage = QString::null;

    // Reimplemented, to add a check for autosave files and to improve error reporting
    if ( !_url.isValid() )
    {
        d->lastErrorMessage = i18n( "Malformed URL\n%1" ).arg( _url.url() ); // ## used anywhere ?
        return false;
    }
    if ( !closeURL() )
        return false;

    KURL url( _url );
    bool autosaveOpened = false;
    d->m_bLoading = true;
    if ( url.isLocalFile() && d->m_shouldCheckAutoSaveFile )
    {
        QString file = url.path();
        QString asf = autoSaveFile( file );
        if ( QFile::exists( asf ) )
        {
            //kdDebug(30003) << "KoDocument::openURL asf=" << asf << endl;
            // ## TODO compare timestamps ?
            int res = KMessageBox::warningYesNoCancel( 0,
                                                       i18n( "An autosaved file exists for this document.\nDo you want to open it instead?" ));
            switch(res) {
                case KMessageBox::Yes :
                    url.setPath( asf );
                    autosaveOpened = true;
                    break;
                case KMessageBox::No :
                    QFile::remove( asf );
                    break;
                default: // Cancel
                    d->m_bLoading = false;
                    return false;
            }
        }
    }

    bool ret = KParts::ReadWritePart::openURL( url );

    if ( autosaveOpened )
        resetURL(); // Force save to act like 'Save As'
    else
    {
        // We have no koffice shell when we are being embedded as a readonly part.
        //if ( d->m_shells.isEmpty() )
        //    kdWarning(30003) << "KoDocument::openURL no shell yet !" << endl;
        // Add to recent actions list in our shells
        QPtrListIterator<KoMainWindow> it( d->m_shells );
        for (; it.current(); ++it )
            it.current()->addRecentURL( _url );
    }
    return ret;
}

bool KoDocument::openFile()
{
    //kdDebug(30003) << "KoDocument::openFile for " << m_file << endl;
    if ( !QFile::exists(m_file) )
    {
        QApplication::restoreOverrideCursor();
        if ( d->m_autoErrorHandlingEnabled )
            // Maybe offer to create a new document with that name ?
            KMessageBox::error(0L, i18n("The file %1 does not exist.").arg(m_file) );
        d->m_bLoading = false;
        return false;
    }

    QApplication::setOverrideCursor( waitCursor );

    d->m_specialOutputFlag = 0;
    QCString _native_format = nativeFormatMimeType();

    KURL u;
    u.setPath( m_file );
    QString typeName = KMimeType::findByURL( u, 0, true )->name();

    // Allow to open backup files, don't keep the mimetype application/x-trash.
    if ( typeName == "application/x-trash" )
    {
        QString path = u.path();
        QStringList patterns = KMimeType::mimeType( typeName )->patterns();
        // Find the extension that makes it a backup file, and remove it
        for( QStringList::Iterator it = patterns.begin(); it != patterns.end(); ++it ) {
            QString ext = *it;
            if ( !ext.isEmpty() && ext[0] == '*' )
            {
                ext.remove(0, 1);
                if ( path.endsWith( ext ) ) {
                    path.truncate( path.length() - ext.length() );
                    break;
                }
            }
        }
        typeName = KMimeType::findByPath( path, 0, true )->name();
    }

    // Special case for flat XML files (e.g. using directory store)
    if ( u.fileName() == "maindoc.xml" || u.fileName() == "content.xml" || typeName == "inode/directory" )
    {
        typeName = _native_format; // Hmm, what if it's from another app? ### Check mimetype
        d->m_specialOutputFlag = SaveAsDirectoryStore;
        kdDebug(30003) << "KoDocument::openFile loading " << u.fileName() << ", using directory store for " << m_file << "; typeName=" << typeName << endl;
    }
    kdDebug(30003) << "KoDocument::openFile " << m_file << " type:" << typeName << endl;

    QString importedFile = m_file;

    if ( !isNativeFormat( typeName.latin1() ) ) {
        if ( !d->filterManager )
            d->filterManager = new KoFilterManager( this );
        KoFilter::ConversionStatus status;
        importedFile = d->filterManager->import( m_file, status );
        if ( status != KoFilter::OK )
        {
            QApplication::restoreOverrideCursor();

            QString msg;
            switch( status )
            {
                case KoFilter::OK: break;

                case KoFilter::CreationError:
                    msg = i18n( "Creation error" ); break;

                case KoFilter::FileNotFound:
                    msg = i18n( "File not found" ); break;

                case KoFilter::StorageCreationError:
                    msg = i18n( "Cannot create storage" ); break;

                case KoFilter::BadMimeType:
                    msg = i18n( "Bad MIME type" ); break;

                case KoFilter::EmbeddedDocError:
                    msg = i18n( "Error in embedded document" ); break;

                case KoFilter::WrongFormat:
                    msg = i18n( "Format not recognized" ); break;

                case KoFilter::NotImplemented:
                    msg = i18n( "Not implemented" ); break;

                case KoFilter::ParsingError:
                    msg = i18n( "Parsing error" ); break;

                case KoFilter::PasswordProtected:
                    msg = i18n( "Document is password protected" ); break;

                case KoFilter::InternalError:
                case KoFilter::UnexpectedEOF:
                case KoFilter::UnexpectedOpcode:
                case KoFilter::StupidError: // ?? what is this ??
                case KoFilter::UsageError:
                    msg = i18n( "Internal error" ); break;

                case KoFilter::OutOfMemory:
                    msg = i18n( "Out of memory" ); break;

                case KoFilter::UserCancelled:
                case KoFilter::BadConversionGraph:
                    // intentionally we do not prompt the error message here
                    break;

                default: msg = i18n( "Unknown error" ); break;
            }

            if( d->m_autoErrorHandlingEnabled && !msg.isEmpty())
            {
                QString errorMsg( i18n( "Could not open\n%2.\nReason: %1" ) );
                QString docUrl = url().prettyURL( 0, KURL::StripFileProtocol );
                KMessageBox::error( 0L, errorMsg.arg(msg).arg(docUrl) );
            }

            d->m_bLoading = false;
            return false;
        }
        kdDebug(30003) << "KoDocument::openFile - importedFile '" << importedFile
                       << "', status: " << static_cast<int>( status ) << endl;
    }

    QApplication::restoreOverrideCursor();

    bool ok = true;

    if (!importedFile.isEmpty()) // Something to load (tmp or native file) ?
    {
        // The filter, if any, has been applied. It's all native format now.
        if ( !loadNativeFormat( importedFile ) )
        {
            ok = false;
            if ( d->m_autoErrorHandlingEnabled )
            {
                showLoadingErrorDialog();
            }
        }
    }

    if ( importedFile != m_file )
    {
        // We opened a temporary file (result of an import filter)
        // Set document URL to empty - we don't want to save in /tmp !
        // But only if in readwrite mode (no saving problem otherwise)
        // --
        // But this isn't true at all.  If this is the result of an
        // import, then importedFile=temporary_file.kwd and
        // m_file/m_url=foreignformat.ext so m_url is correct!
        // So don't resetURL() or else the caption won't be set when
        // foreign files are opened (an annoying bug).
        // - Clarence
        //
#if 0
        if ( isReadWrite() )
            resetURL();
#endif

        // remove temp file - uncomment this to debug import filters
        if(!importedFile.isEmpty()) {
            QFile::remove( importedFile );
	}
    }

    if ( ok && d->m_bSingleViewMode )
    {
        // See addClient below
        KXMLGUIFactory* guiFactory = factory();
        if( guiFactory ) // 0L when splitting views in konq, for some reason
            guiFactory->removeClient( this );

        if ( !d->m_views.isEmpty() )
        {
            // We already had a view (this happens when doing reload in konqueror)
            KoView* v = d->m_views.first();
            if( guiFactory )
                guiFactory->removeClient( v );
            removeView( v );
            delete v;
            Q_ASSERT( d->m_views.isEmpty() );
        }

        KoView *view = createView( d->m_wrapperWidget );
        d->m_wrapperWidget->setKoView( view );
        view->show();

        // Ok, now we have a view, so action() and domDocument() will work as expected
        // -> rebuild GUI
        if ( guiFactory )
            guiFactory->addClient( this );
    }

    if ( ok )
    {
        setMimeTypeAfterLoading( typeName );
    }
    d->m_bLoading = false;
    return ok;
}

// shared between openFile and koMainWindow's "create new empty document" code
void KoDocument::setMimeTypeAfterLoading( const QString& mimeType )
{
    d->mimeType = mimeType.latin1();

    d->outputMimeType = d->mimeType;

    const bool needConfirm = !isNativeFormat( d->mimeType );
    setConfirmNonNativeSave( false, needConfirm  );
    setConfirmNonNativeSave( true, needConfirm );
}

// The caller must call store->close() if loadAndParse returns true.
bool KoDocument::oldLoadAndParse(KoStore* store, const QString& filename, QDomDocument& doc)
{
    //kdDebug(30003) << "oldLoadAndParse: Trying to open " << filename << endl;

    if (!store->open(filename))
    {
        kdWarning(30003) << "Entry " << filename << " not found!" << endl;
        d->lastErrorMessage = i18n( "Could not find %1" ).arg( filename );
        return false;
    }
    // Error variables for QDomDocument::setContent
    QString errorMsg;
    int errorLine, errorColumn;
    bool ok = doc.setContent( store->device(), &errorMsg, &errorLine, &errorColumn );
    if ( !ok )
    {
        kdError(30003) << "Parsing error in " << filename << "! Aborting!" << endl
            << " In line: " << errorLine << ", column: " << errorColumn << endl
            << " Error message: " << errorMsg << endl;
        d->lastErrorMessage = i18n( "Parsing error in %1 at line %2, column %3\nError message: %4" )
                              .arg( filename ).arg( errorLine ).arg( errorColumn )
                              .arg( i18n ( "QXml", errorMsg.utf8() ) );
        store->close();
        return false;
    }
    kdDebug(30003) << "File " << filename << " loaded and parsed" << endl;
    return true;
}

bool KoDocument::loadNativeFormat( const QString & file )
{
    QFileInfo fileInfo( file );
    if ( !fileInfo.exists() ) // check duplicated from openURL, but this is useful for templates
    {
        d->lastErrorMessage = i18n("The file %1 does not exist.").arg(file);
        return false;
    }
    if ( !fileInfo.isFile() )
    {
        d->lastErrorMessage = i18n( "%1 is not a file." ).arg(file);
        return false;
    }

    QApplication::setOverrideCursor( waitCursor );

    kdDebug(30003) << "KoDocument::loadNativeFormat( " << file << " )" << endl;

    QFile in;
    bool isRawXML = false;
    if ( d->m_specialOutputFlag != SaveAsDirectoryStore ) // Don't try to open a directory ;)
    {
        in.setName(file);
        if ( !in.open( IO_ReadOnly ) )
        {
            QApplication::restoreOverrideCursor();
            d->lastErrorMessage = i18n( "Could not open the file for reading (check read permissions)." );
            return false;
        }

        // Try to find out whether it is a mime multi part file
        char buf[5];
        if ( in.readBlock( buf, 4 ) < 4 )
        {
            QApplication::restoreOverrideCursor();
            in.close();
            d->lastErrorMessage = i18n( "Could not read the beginning of the file." );
            return false;
        }
        // ### TODO: allow UTF-16
        isRawXML = (strncasecmp( buf, "<?xm", 4 ) == 0);
        if ( !isRawXML ) {
            // Still check for broken MathML files, which seem to be rather common
            in.reset();
            // Remove spaces
            do {
                if ( in.readBlock( buf , 1 ) < 1 )
                {
                    QApplication::restoreOverrideCursor();
                    in.close();
                    d->lastErrorMessage = i18n( "Could not read the beginning of the file." );
                    return false;
                }
            } while ( QChar( buf[0] ).isSpace() );
            if ( buf[0] == '<' ) { // First not-space character
                if ( in.readBlock( buf , 4 ) < 4 )
                {
                    QApplication::restoreOverrideCursor();
                    in.close();
                    d->lastErrorMessage = i18n( "Could not read the beginning of the file." );
                    return false;
                }
                isRawXML = (strncasecmp( buf, "math", 4 ) == 0); // file begins with <math ?
            }
        }
        //kdDebug(30003) << "PATTERN=" << buf << endl;
    }
    // Is it plain XML?
    if ( isRawXML )
    {
        in.at(0);
        QString errorMsg;
        int errorLine;
        int errorColumn;
        QDomDocument doc;
        bool res;
        if ( doc.setContent( &in, true, &errorMsg, &errorLine, &errorColumn ) )
        {
            res = loadXML( &in, doc );
            if ( res )
                res = completeLoading( 0L );
        }
        else
        {
            kdError (30003) << "Parsing Error! Aborting! (in KoDocument::loadNativeFormat (QFile))" << endl
                            << "  Line: " << errorLine << " Column: " << errorColumn << endl
                            << "  Message: " << errorMsg << endl;
            d->lastErrorMessage = i18n( "parsing error in the main document at line %1, column %2\nError message: %3" )
                                  .arg( errorLine ).arg( errorColumn ).arg( i18n ( errorMsg.utf8() ) );
            res=false;
        }

        QApplication::restoreOverrideCursor();
        in.close();
        m_bEmpty = false;
        return res;
    } else
    { // It's a koffice store (tar.gz, zip, directory, etc.)
        in.close();

        return loadNativeFormatFromStore( file );
    }
}

bool KoDocument::loadNativeFormatFromStore( const QString& file )
{
    KoStore::Backend backend = (d->m_specialOutputFlag == SaveAsDirectoryStore) ? KoStore::Directory : KoStore::Auto;
    KoStore * store = KoStore::createStore( file, KoStore::Read, "", backend );

    if ( store->bad() )
    {
        d->lastErrorMessage = i18n( "Not a valid KOffice file: %1" ).arg( file );
        delete store;
        QApplication::restoreOverrideCursor();
        return false;
    }

    bool oasis = true;
    // OASIS/OOo file format?
    if ( store->hasFile( "content.xml" ) )
    {
        store->disallowNameExpansion();

        KoOasisStore oasisStore( store );
        // We could check the 'mimetype' file, but let's skip that and be tolerant.

        if ( !loadOasisFromStore( store ) ) {
            delete store;
            QApplication::restoreOverrideCursor();
            return false;
        }

    } else if ( store->hasFile( "root" ) ) // Fallback to "old" file format (maindoc.xml)
    {
        oasis = false;

        QDomDocument doc;
        bool ok = oldLoadAndParse( store, "root", doc );
        if ( ok )
            ok = loadXML( store->device(), doc );
        if ( !ok )
        {
            delete store;
            QApplication::restoreOverrideCursor();
            return false;
        }
        store->close();

        if ( !loadChildren( store ) )
        {
            kdError(30003) << "ERROR: Could not load children" << endl;
            // Don't abort, proceed nonetheless
        }

    } else
    {
        kdError(30003) << "ERROR: No maindoc.xml" << endl;
        d->lastErrorMessage = i18n( "Invalid document: no file 'maindoc.xml'." );
        delete store;
        QApplication::restoreOverrideCursor();
        return false;
    }

    if ( oasis && store->hasFile( "meta.xml" ) ) {
        QDomDocument metaDoc;
        KoOasisStore oasisStore( store );
        if ( oasisStore.loadAndParse( "meta.xml", metaDoc, d->lastErrorMessage ) ) {
            d->m_docInfo->loadOasis( metaDoc );
        }
    }
    else if ( !oasis && store->hasFile( "documentinfo.xml" ) )
    {
        QDomDocument doc;
        if ( oldLoadAndParse( store, "documentinfo.xml", doc ) ) {
            store->close();
            d->m_docInfo->load( doc );
        }
    }
    else
    {
        //kdDebug( 30003 ) << "cannot open document info" << endl;
        delete d->m_docInfo;
        d->m_docInfo = new KoDocumentInfo( this, "document info" );
    }

    bool res = completeLoading( store );
    delete store;
    QApplication::restoreOverrideCursor();
    m_bEmpty = false;
    return res;
}

// For embedded documents
bool KoDocument::loadFromStore( KoStore* _store, const QString& url )
{
    if ( _store->open( url ) )
    {
        QDomDocument doc;
        doc.setContent( _store->device() );
        if ( !loadXML( _store->device(), doc ) )
        {
            _store->close();
            return false;
        }
        _store->close();
    } else {
        kdWarning() << "couldn't open " << url << endl;
    }

    _store->pushDirectory();
    // Store as document URL
    if ( url.startsWith( STORE_PROTOCOL ) ) {
        m_url = KURL( url );
    } else {
        m_url = KURL( INTERNAL_PREFIX + url );
        _store->enterDirectory( url );
    }

    if ( !loadChildren( _store ) )
    {
        kdError(30003) << "ERROR: Could not load children" << endl;
#if 0
        return false;
#endif
    }

    bool result = completeLoading( _store );

    // Restore the "old" path
    _store->popDirectory();

    return result;
}

bool KoDocument::loadOasisFromStore( KoStore* store )
{
    KoOasisStyles oasisStyles;
    QDomDocument contentDoc;
    QDomDocument settingsDoc;
    KoOasisStore oasisStore( store );
    bool ok = oasisStore.loadAndParse( "content.xml", contentDoc, d->lastErrorMessage );
    if ( !ok )
        return false;

    QDomDocument stylesDoc;
    (void)oasisStore.loadAndParse( "styles.xml", stylesDoc, d->lastErrorMessage );
    // Load styles from style.xml
    oasisStyles.createStyleMap( stylesDoc, true );
    // Also load styles from content.xml
    oasisStyles.createStyleMap( contentDoc, false );

    // TODO post 1.4, pass manifestDoc to the apps so that they don't have to do it themselves
    // (when calling KoDocumentChild::loadOasisDocument)
    //QDomDocument manifestDoc;
    //KoOasisStore oasisStore( store );
    //if ( !oasisStore.loadAndParse( "tar:/META-INF/manifest.xml", manifestDoc, d->lastErrorMessage ) )
    //    return false;

    if ( store->hasFile( "settings.xml" ) ) {
        (void)oasisStore.loadAndParse( "settings.xml", settingsDoc, d->lastErrorMessage );
    }
    if ( !loadOasis( contentDoc, oasisStyles, settingsDoc, store ) )
        return false;

    return true;
}

bool KoDocument::isInOperation() const
{
    return d->m_numOperations > 0;
}

void KoDocument::emitBeginOperation()
{

    /* if we're already in an operation, don't send the signal again */
    if (!isInOperation())
        emit sigBeginOperation();
    d->m_numOperations++;
}

void KoDocument::emitEndOperation()
{
    d->m_numOperations--;

    /* don't end the operation till we've cleared all the nested operations */
    if (d->m_numOperations == 0)
        emit sigEndOperation();
    else if (d->m_numOperations < 0)
        /* ignore 'end' calls with no matching 'begin' call */
        d->m_numOperations = 0;
}


bool KoDocument::isStoredExtern() const
{
    return !storeInternal() && hasExternURL();
}

void KoDocument::setModified( bool mod )
{
    if ( isAutosaving() ) // ignore setModified calls due to autosaving
        return;

    //kdDebug(30003)<<k_funcinfo<<" url:" << m_url.path() << endl;
    //kdDebug(30003)<<k_funcinfo<<" mod="<<mod<<" MParts mod="<<KParts::ReadWritePart::isModified()<<" isModified="<<isModified()<<endl;

    if ( mod && !d->modifiedAfterAutosave ) {
        // First change since last autosave -> start the autosave timer
        setAutoSave( d->m_autoSaveDelay );
    }
    d->modifiedAfterAutosave = mod;

    if ( mod == isModified() )
        return;

    KParts::ReadWritePart::setModified( mod );

    if ( mod ) {
        m_bEmpty = FALSE;
    } else {
        // When saving this document, all non-external child documents get saved too.
        QPtrListIterator<KoDocumentChild> it = children();
        for (; it.current(); ++it )
        {
            KoDocument *doc = it.current()->document();
            if ( doc && !it.current()->isStoredExtern() && !it.current()->isDeleted() && doc->isModified() )
                doc->setModified( false );
        }
    }

    // This influences the title
    setTitleModified();
    emit modified( mod );
}

void KoDocument::setDoNotSaveExtDoc( bool on )
{
    d->m_doNotSaveExtDoc = on;
}

int KoDocument::queryCloseDia()
{
    //kdDebug(30003)<<k_funcinfo<<endl;

    QString name;
    if ( documentInfo() )
    {
        name = documentInfo()->title();
    }
    if ( name.isEmpty() )
        name = url().fileName();

    if ( name.isEmpty() )
        name = i18n( "Untitled" );

    int res = KMessageBox::warningYesNoCancel( 0L,
                    i18n( "<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>" ).arg(name));

    switch(res)
    {
        case KMessageBox::Yes :
            setDoNotSaveExtDoc(); // Let save() only save myself and my internal docs
            save(); // NOTE: External files always in native format. ###TODO: Handle non-native format
            setModified( false ); // Now when queryClose() is called by closeEvent it won't do anything.
            break;
        case KMessageBox::No :
            removeAutoSaveFiles();
            setModified( false ); // Now when queryClose() is called by closeEvent it won't do anything.
            break;
        default : // case KMessageBox::Cancel :
            return res; // cancels the rest of the files
    }
    return res;
}

int KoDocument::queryCloseExternalChildren()
{
    //kdDebug(30003)<<k_funcinfo<<" checking for children in: "<<url().url()<<endl;
    setDoNotSaveExtDoc(false);
    QPtrListIterator<KoDocumentChild> it( children() );
    for (; it.current(); ++it )
    {
        if ( !it.current()->isDeleted() )
        {
            KoDocument *doc = it.current()->document();
            if ( doc )
            {
		bool foo = doc->isStoredExtern();
		kdDebug(36001) << "========== isStoredExtern() returned "
			       << foo << " ==========" << endl;

                if ( foo ) //###TODO: Handle non-native mimetype docs
                {
                    {
                        kdDebug(30003)<<k_funcinfo<<" found modified child: "<<doc->url().url()<<" extern="<<doc->isStoredExtern()<<endl;
                        if ( doc->queryCloseDia() == KMessageBox::Cancel )
                            return  KMessageBox::Cancel;
                    }
                }
                if ( doc->queryCloseExternalChildren() == KMessageBox::Cancel )
                    return KMessageBox::Cancel;
            }
        }
    }
    return KMessageBox::Ok;
}

void KoDocument::setTitleModified( const QString caption, bool mod )
{
    //kdDebug(30003)<<k_funcinfo<<" url: "<<url().url()<<" caption: "<<caption<<" mod: "<<mod<<endl;
    KoDocument *doc = dynamic_cast<KoDocument *>( parent() );
    if ( doc )
    {
        doc->setTitleModified( caption, mod );
        return;
    }
    // we must be root doc so update caption in all related windows
    QPtrListIterator<KoMainWindow> it( d->m_shells );
    for (; it.current(); ++it )
    {
        it.current()->updateCaption();
        it.current()->updateReloadFileAction(this);
        it.current()->updateVersionsFileAction(this);
    }
}

void KoDocument::setTitleModified()
{
    //kdDebug(30003)<<k_funcinfo<<" url: "<<url().url()<<" extern: "<<isStoredExtern()<<" current: "<<d->m_current<<endl;
    KoDocument *doc = dynamic_cast<KoDocument *>( parent() );
    QString caption;
    if ( (url().isEmpty() || isStoredExtern()) && d->m_current )
    {
        // Get caption from document info (title(), in about page)
        if ( documentInfo() )
        {
            KoDocumentInfoPage * page = documentInfo()->page( QString::fromLatin1("about") );
            if (page)
                caption = static_cast<KoDocumentInfoAbout *>(page)->title();
        }
        if ( caption.isEmpty() )
            caption = url().prettyURL( 0, KURL::StripFileProtocol );             // Fall back to document URL

        //kdDebug(30003)<<k_funcinfo<<" url: "<<url().url()<<" caption: "<<caption<<endl;
        if ( doc )
        {
            doc->setTitleModified( caption, isModified() );
            return;
        }
        else
        {
            // we must be root doc so update caption in all related windows
            setTitleModified( caption, isModified() );
            return;
        }
    }
    if ( doc )
    {
        // internal doc or not current doc, so pass on the buck
        doc->setTitleModified();
    }
}

bool KoDocument::loadChildren( KoStore* )
{
    return true;
}

bool KoDocument::completeLoading( KoStore* )
{
    return true;
}

bool KoDocument::completeSaving( KoStore* )
{
    return true;
}

QDomDocument KoDocument::createDomDocument( const QString& tagName, const QString& version ) const
{
    return createDomDocument( instance()->instanceName(), tagName, version );
}

//static
QDomDocument KoDocument::createDomDocument( const QString& appName, const QString& tagName, const QString& version )
{
    QDomImplementation impl;
    QString url = QString("http://www.koffice.org/DTD/%1-%1.dtd").arg(appName).arg(version);
    QDomDocumentType dtype = impl.createDocumentType( tagName,
                                                      QString("-//KDE//DTD %1 %1//EN").arg(appName).arg(version),
                                                      url );
    // The namespace URN doesn't need to include the version number.
    QString namespaceURN = QString("http://www.koffice.org/DTD/%1").arg(appName);
    QDomDocument doc = impl.createDocument( namespaceURN, tagName, dtype );
    doc.insertBefore( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ), doc.documentElement() );
    return doc;
}

KoXmlWriter* KoDocument::createOasisXmlWriter( QIODevice* dev, const char* rootElementName )
{
    KoXmlWriter* writer = new KoXmlWriter( dev );
    writer->startDocument( rootElementName );
    writer->startElement( rootElementName );
    writer->addAttribute( "xmlns:office", KoXmlNS::office );
    writer->addAttribute( "xmlns:meta", KoXmlNS::meta );

    if ( qstrcmp( rootElementName, "office:document-meta" ) != 0 ) {
        writer->addAttribute( "xmlns:config", KoXmlNS::config );
        writer->addAttribute( "xmlns:text", KoXmlNS::text );
        writer->addAttribute( "xmlns:table", KoXmlNS::table );
        writer->addAttribute( "xmlns:draw", KoXmlNS::draw );
        writer->addAttribute( "xmlns:presentation", KoXmlNS::presentation );
        writer->addAttribute( "xmlns:dr3d", KoXmlNS::dr3d );
        writer->addAttribute( "xmlns:chart", KoXmlNS::chart );
        writer->addAttribute( "xmlns:form", KoXmlNS::form );
        writer->addAttribute( "xmlns:script", KoXmlNS::script );
        writer->addAttribute( "xmlns:style", KoXmlNS::style );
        writer->addAttribute( "xmlns:number", KoXmlNS::number );
        writer->addAttribute( "xmlns:math", KoXmlNS::math );
        writer->addAttribute( "xmlns:svg", KoXmlNS::svg );
        writer->addAttribute( "xmlns:fo", KoXmlNS::fo );
        writer->addAttribute( "xmlns:koffice", KoXmlNS::koffice );
    }
    // missing: office:version="1.0"

    writer->addAttribute( "xmlns:dc", KoXmlNS::dc );
    writer->addAttribute( "xmlns:xlink", KoXmlNS::xlink );
    return writer;
}

QDomDocument KoDocument::saveXML()
{
    kdError(30003) << "KoDocument::saveXML not implemented" << endl;
    d->lastErrorMessage = i18n( "Internal error: saveXML not implemented" );
    return QDomDocument();
}

KService::Ptr KoDocument::nativeService()
{
    if ( !m_nativeService )
        m_nativeService = readNativeService( instance() );

    return m_nativeService;
}

QCString KoDocument::nativeFormatMimeType() const
{
    KService::Ptr service = const_cast<KoDocument *>(this)->nativeService();
    if ( !service )
        return QCString();
    QCString nativeMimeType = service->property( "X-KDE-NativeMimeType" ).toString().latin1();
    if ( nativeMimeType.isEmpty() ) {
        // shouldn't happen, let's find out why it happened
        if ( !service->serviceTypes().contains( "KOfficePart" ) )
            kdWarning(30003) << "Wrong desktop file, KOfficePart isn't mentionned" << endl;
        else if ( !KServiceType::serviceType( "KOfficePart" ) )
            kdWarning(30003) << "The KOfficePart service type isn't installed!" << endl;
    }
    return nativeMimeType;
}

QCString KoDocument::nativeOasisMimeType() const
{
    KService::Ptr service = const_cast<KoDocument *>(this)->nativeService();
    if ( !service )
        return QCString();
    return service->property( "X-KDE-NativeOasisMimeType" ).toString().latin1();
}


//static
KService::Ptr KoDocument::readNativeService( KInstance *instance )
{
    QString instname = instance ? instance->instanceName() : kapp->instanceName();

    // The new way is: we look for a foopart.desktop in the kde_services dir.
    QString servicepartname = instname + "part.desktop";
    KService::Ptr service = KService::serviceByDesktopPath( servicepartname );
    if ( service )
        kdDebug(30003) << servicepartname << " found." << endl;
    if ( !service )
    {
        // The old way is kept as fallback for compatibility, but in theory this is really never used anymore.

        // Try by path first, so that we find the global one (which has the native mimetype)
        // even if the user created a kword.desktop in ~/.kde/share/applnk or any subdir of it.
        // If he created it under ~/.kde/share/applnk/Office/ then no problem anyway.
        service = KService::serviceByDesktopPath( QString::fromLatin1("Office/%1.desktop").arg(instname) );
    }
    if ( !service )
        service = KService::serviceByDesktopName( instname );

    return service;
}

QCString KoDocument::readNativeFormatMimeType( KInstance *instance ) //static
{
    KService::Ptr service = readNativeService( instance );
    if ( !service )
        return QCString();

    if ( service->property( "X-KDE-NativeMimeType" ).toString().isEmpty() )
    {
        // It may be that the servicetype "KOfficePart" is missing, which leads to this property not being known
        if ( KServiceType::serviceType( "KOfficePart" ) == 0L )
            kdError(30003) << "The serviceType KOfficePart is missing. Check that you have a kofficepart.desktop file in the share/servicetypes directory." << endl;
        else {
            QString instname = instance ? instance->instanceName() : kapp->instanceName();
            if ( instname != "koshell" ) // hack for koshell
                kdWarning(30003) << service->desktopEntryPath() << ": no X-KDE-NativeMimeType entry!" << endl;
        }
    }

    return service->property( "X-KDE-NativeMimeType" ).toString().latin1();
}

QStringList KoDocument::readExtraNativeMimeTypes( KInstance *instance ) //static
{
    KService::Ptr service = readNativeService( instance );
    if ( !service )
        return QStringList();
    return service->property( "X-KDE-ExtraNativeMimeTypes" ).toStringList();
}

void KoDocument::setupXmlReader( QXmlSimpleReader& reader, bool namespaceProcessing )
{
    if ( namespaceProcessing )
    {
        reader.setFeature( "http://xml.org/sax/features/namespaces", TRUE );
        reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", FALSE );
    }
    else
    {
        reader.setFeature( "http://xml.org/sax/features/namespaces", FALSE );
        reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
    }
    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", TRUE );
}


bool KoDocument::isNativeFormat( const QCString& mimetype ) const
{
    if ( mimetype == nativeFormatMimeType() )
        return true;
    return extraNativeMimeTypes().contains( mimetype );
}

QStringList KoDocument::extraNativeMimeTypes() const
{
    QStringList lst;
    // This implementation is temporary while we treat both koffice-1.3 and OASIS formats as native.
    // But it's good to have this virtual method, in case some app want to
    // support more than one native format.
    KService::Ptr service = const_cast<KoDocument *>(this)->nativeService();
    if ( !service ) // can't happen
        return lst;
    return service->property( "X-KDE-ExtraNativeMimeTypes" ).toStringList();
}

int KoDocument::supportedSpecialFormats() const
{
    // Apps which support special output flags can add reimplement and add to this.
    // E.g. this is how did "saving in the 1.1 format".
    // SaveAsDirectoryStore is a given since it's implemented by KoDocument itself.
    return SaveAsDirectoryStore;
}

void KoDocument::addShell( KoMainWindow *shell )
{
    if ( d->m_shells.findRef( shell ) == -1 )
    {
        //kdDebug(30003) << "addShell: shell " << (void*)shell << " added to doc " << this << endl;
        d->m_shells.append( shell );
    }
}

void KoDocument::removeShell( KoMainWindow *shell )
{
    //kdDebug(30003) << "removeShell: shell " << (void*)shell << " removed from doc " << this << endl;
    d->m_shells.removeRef( shell );
}

const QPtrList<KoMainWindow>& KoDocument::shells() const
{
    return d->m_shells;
}

int KoDocument::shellCount() const
{
    return d->m_shells.count();
}

DCOPObject * KoDocument::dcopObject()
{
    if ( !d->m_dcopObject )
        d->m_dcopObject = new KoDocumentIface( this );
    return d->m_dcopObject;
}

QCString KoDocument::dcopObjectId() const
{
    return const_cast<KoDocument *>(this)->dcopObject()->objId();
}

void KoDocument::setErrorMessage( const QString& errMsg )
{
    d->lastErrorMessage = errMsg;
}

QString KoDocument::errorMessage() const
{
    return d->lastErrorMessage;
}

void KoDocument::showSavingErrorDialog()
{
    if ( d->lastErrorMessage.isEmpty() )
    {
        KMessageBox::error( 0L, i18n( "Could not save\n%1" ).arg( m_file ) );
    }
    else if ( d->lastErrorMessage != "USER_CANCELED" )
    {
        KMessageBox::error( 0L, i18n( "Could not save %1\nReason: %2" ).arg( m_file, d->lastErrorMessage ) );
    }
}

void KoDocument::showLoadingErrorDialog()
{
    if ( d->lastErrorMessage.isEmpty() )
    {
        KMessageBox::error( 0L, i18n( "Could not open\n%1" ).arg( url().prettyURL( 0, KURL::StripFileProtocol ) ) );
    }
    else if ( d->lastErrorMessage != "USER_CANCELED" )
    {
        KMessageBox::error( 0L, i18n( "Could not open %1\nReason: %2" ).arg( url().prettyURL( 0, KURL::StripFileProtocol ), d->lastErrorMessage ) );
    }
}

bool KoDocument::isAutosaving() const
{
    return d->m_autosaving;
}

bool KoDocument::isLoading() const
{
    return d->m_bLoading;
}

void KoDocument::removeAutoSaveFiles()
{
        // Eliminate any auto-save file
        QString asf = autoSaveFile( m_file ); // the one in the current dir
        if ( QFile::exists( asf ) )
            QFile::remove( asf );
        asf = autoSaveFile( QString::null ); // and the one in $HOME
        if ( QFile::exists( asf ) )
            QFile::remove( asf );
}

void KoDocument::setBackupFile( bool _b )
{
    d->m_backupFile = _b;
}

bool KoDocument::backupFile()const
{
    return d->m_backupFile;
}


void KoDocument::setBackupPath( const QString & _path)
{
    d->m_backupPath = _path;
}

QString KoDocument::backupPath()const
{
    return d->m_backupPath;
}

void KoDocument::setCurrent( bool on )
{
    //kdDebug(30003)<<k_funcinfo<<" url: "<<url().url()<<" set to: "<<on<<endl;
    KoDocument *doc = dynamic_cast<KoDocument *>( parent() );
    if ( doc )
    {
        if ( !isStoredExtern() )
        {
            // internal doc so set next external to current (for safety)
            doc->setCurrent( true );
            return;
        }
        // only externally stored docs shall have file name in title
        d->m_current = on;
        if ( !on )
        {
            doc->setCurrent( true );    // let my next external parent take over
            return;
        }
        doc->forceCurrent( false ); // everybody else should keep off
    }
    else
        d->m_current = on;

    setTitleModified();
}

void KoDocument::forceCurrent( bool on )
{
    //kdDebug(30003)<<k_funcinfo<<" url: "<<url().url()<<" force to: "<<on<<endl;
    d->m_current = on;
    KoDocument *doc = dynamic_cast<KoDocument *>( parent() );
    if ( doc )
    {
        doc->forceCurrent( false );
    }
}

bool KoDocument::isCurrent() const
{
    return d->m_current;
}

bool KoDocument::storeInternal() const
{
    return d->m_storeInternal;
}

void KoDocument::setStoreInternal( bool i )
{
    d->m_storeInternal = i;
    //kdDebug(30003)<<k_funcinfo<<"="<<d->m_storeInternal<<" doc: "<<url().url()<<endl;
}

bool KoDocument::hasExternURL() const
{
    return !url().protocol().isEmpty() && url().protocol() != STORE_PROTOCOL && url().protocol() != INTERNAL_PROTOCOL;
}

void KoDocument::slotStarted( KIO::Job* job )
{
    if ( job )
    {
        job->setWindow( d->m_shells.current() );
    }
}

static const struct {
    const char* localName;
    const char* documentType;
} TN2DTArray[] = {
    { "text", I18N_NOOP( "a word processing" ) },
    { "spreadsheet", I18N_NOOP( "a spreadsheet" ) },
    { "presentation", I18N_NOOP( "a presentation" ) },
    { "chart", I18N_NOOP( "a chart" ) },
    { "drawing", I18N_NOOP( "a drawing" ) }
};
static const unsigned int numTN2DT = sizeof( TN2DTArray ) / sizeof( *TN2DTArray );

QString KoDocument::tagNameToDocumentType( const QString& localName )
{
    for ( unsigned int i = 0 ; i < numTN2DT ; ++i )
        if ( localName == TN2DTArray[i].localName )
            return i18n( TN2DTArray[i].documentType );
    return localName;
}

QValueList<KoTextDocument *> KoDocument::allTextDocuments() const
{
    return QValueList<KoTextDocument *>();
}

KoPageLayout KoDocument::pageLayout(int /*pageNumber*/) const
{
    return m_pageLayout;
}

KoUnit::Unit KoDocument::unit() const
{
    return d->m_unit;
}

void KoDocument::setUnit( KoUnit::Unit unit )
{
    if ( d->m_unit != unit )
    {
        d->m_unit = unit;
        emit unitChanged( unit );
    }
}

QString KoDocument::unitName() const
{
    return KoUnit::unitName( unit() );
}

void KoDocument::showStartUpWidget( KoMainWindow* parent, bool alwaysShow )
{
    if(!alwaysShow) {
        KConfigGroup cfgGrp( instance()->config(), "TemplateChooserDialog" );
        QString fullTemplateName = cfgGrp.readPathEntry( "AlwaysUseTemplate" );

        if( !fullTemplateName.isEmpty() ) {
            openTemplate( fullTemplateName );
            shells().getFirst()->setRootDocument( this );
            return;
        }
    }

    if(d->m_startUpWidget){
        d->m_startUpWidget->show();
    } else {
        d->m_startUpWidget = createOpenPane( parent->centralWidget(), instance(), templateType() );
    }

    parent->setDocToOpen( this );
    parent->factory()->container("mainToolBar", parent)->hide();
}

void KoDocument::openExistingFile( const QString& file )
{
    KURL url( file );
    bool ok = openURL( url );
    setModified( false );

    if( ok )
        QTimer::singleShot( 0, this, SLOT( deleteOpenPane() ) );
}

void KoDocument::openTemplate( const QString& file )
{
    bool ok = loadNativeFormat( file );
    setModified( false );

    if ( ok ) {
        deleteOpenPane();
        resetURL();
        setEmpty();
    } else {
        showLoadingErrorDialog();
        initEmpty();
    }
}

void KoDocument::initEmpty()
{
    setEmpty();
    setModified(false);
}

void KoDocument::startCustomDocument() {
    deleteOpenPane();
}

KoOpenPane* KoDocument::createOpenPane( QWidget* parent, KInstance* instance,
                                        const QString& templateType )
{
    KoOpenPane* openPane = new KoOpenPane( parent, instance, templateType );
    QWidget *customDoc = createCustomDocumentWidget(openPane);
    if(customDoc) {
        openPane->setCustomDocumentWidget( customDoc );
        connect( customDoc, SIGNAL( documentSelected() ), this, SLOT( startCustomDocument() ) );
    }
    openPane->show();

    connect( openPane, SIGNAL( openExistingFile( const QString& ) ),
             this, SLOT( openExistingFile( const QString& ) ) );
    connect( openPane, SIGNAL( openTemplate( const QString& ) ),
             this, SLOT( openTemplate( const QString& ) ) );

    return openPane;
}

void KoDocument::setTemplateType( const QString& _templateType )
{
    d->m_templateType = _templateType;
}

QString KoDocument::templateType() const
{
    return d->m_templateType;
}

void KoDocument::deleteOpenPane()
{
    if( d->m_startUpWidget ) {
        d->m_startUpWidget->hide();
        QTimer::singleShot(1000, this, SLOT(deleteOpenPaneDelayed()));

        shells().getFirst()->factory()->container("mainToolBar", shells().getFirst())->show();
        shells().getFirst()->setRootDocument( this );
    } else {
      emit closeEmbedInitDialog();
    }
}

void KoDocument::deleteOpenPaneDelayed()
{
    delete d->m_startUpWidget;
    d->m_startUpWidget = 0;
}

QWidget* KoDocument::createCustomDocumentWidget(QWidget */*parent*/) {
    return 0;
}

bool KoDocument::showEmbedInitDialog(QWidget* parent)
{
    KDialogBase dlg(parent, "EmbedInitDialog", true, i18n("Embedding Object"), 0, KDialogBase::NoDefault);
    KoOpenPane* pane = createOpenPane(&dlg, instance(), templateType());
    pane->layout()->setMargin(0);
    dlg.setMainWidget(pane);
    dlg.setInitialSize(dlg.configDialogSize("EmbedInitDialog"));
    connect(this, SIGNAL(closeEmbedInitDialog()), &dlg, SLOT(slotOk()));

    bool ok = dlg.exec() == QDialog::Accepted;

    dlg.saveDialogSize("EmbedInitDialog");

    return ok;
}

#include "KoDocument_p.moc"
#include "KoDocument.moc"
