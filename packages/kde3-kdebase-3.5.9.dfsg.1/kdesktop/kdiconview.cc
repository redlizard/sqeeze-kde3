/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000, 2001 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdiconview.h"
#include "krootwm.h"
#include "desktop.h"
#include "kdesktopsettings.h"

#include <kio/paste.h>
#include <kaccel.h>
#include <kapplication.h>
#include <kcolordrag.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kdirlister.h>
#include <kglobalsettings.h>
#include <kpropertiesdialog.h>
#include <klocale.h>
#include <konqbookmarkmanager.h>
#include <konq_defaults.h>
#include <konq_drag.h>
#include <konq_operations.h>
#include <konq_popupmenu.h>
#include <konq_settings.h>
#include <konq_undo.h>
#include <kprotocolinfo.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <kurldrag.h>
#include <kwin.h>
#include <kwinmodule.h>

#include <fixx11h.h>

#include <qclipboard.h>
#include <qdir.h>
#include <qevent.h>
#include <qregexp.h>

#include <unistd.h>

#include "kshadowengine.h"
#include "kdesktopshadowsettings.h"
#include "kfileividesktop.h"

// for multihead
extern int kdesktop_screen_number;

// -----------------------------------------------------------------------------

QRect KDIconView::desktopRect()
{
    return ( kdesktop_screen_number == 0 )
            ? QApplication::desktop()->geometry() // simple case, or xinerama
            : QApplication::desktop()->screenGeometry( kdesktop_screen_number ); // multi-head
}

// -----------------------------------------------------------------------------

void KDIconView::saveIconPosition(KSimpleConfig *config, int x, int y)
{
  // save the icon position in absolute coordinates
  config->writeEntry("Xabs", x);
  config->writeEntry("Yabs", y);

  // save also mentioning desktop size
  QRect desk = desktopRect();
  QString sizeStr = QString( "_%1x%2" ).arg(desk.width()).arg(desk.height());

  config->writeEntry("Xabs" + sizeStr, x);
  config->writeEntry("Yabs" + sizeStr, y);
}

// -----------------------------------------------------------------------------

void KDIconView::readIconPosition(KSimpleConfig *config, int &x, int &y)
{
  // check if we have the position for the current desktop size
  QRect desk = desktopRect();
  QString sizeStr = QString( "_%1x%2" ).arg(desk.width()).arg(desk.height());

  x = config->readNumEntry("Xabs" + sizeStr, -99999);

  if ( x != -99999 )
    y = config->readNumEntry("Yabs" + sizeStr);
  else
  {
    // not found; use the resolution independent position
    x = config->readNumEntry("Xabs", -99999);

    if ( x != -99999 )
      y = config->readNumEntry("Yabs");
    else  // for compatibility, read the old iconArea-relative-position
    {
      // problem here: when reading a position before we know the correct
      // desktopIconsArea, the relative position do not make sense
      // workaround: use desktopRect() as the allowed size

      QRect desk = desktopRect();
      QString X_w = QString("X %1").arg(desk.width() );
      QString Y_h = QString("Y %1").arg(desk.height());

      x = config->readNumEntry(X_w, -99999);
      if ( x != -99999 ) x = config->readNumEntry("X");
      if ( x < 0 ) x += desk.width();

      y = config->readNumEntry(Y_h, -99999);
      if ( y != -99999 ) y = config->readNumEntry("Y");
      if ( y < 0 ) y += desk.height();
    }
  }
}

// -----------------------------------------------------------------------------

KDIconView::KDIconView( QWidget *parent, const char* name )
    : KonqIconViewWidget( parent, name, WResizeNoErase, true ),
      m_actionCollection( this, "KDIconView::m_actionCollection" ),
      m_accel( 0L ),
      m_bNeedRepaint( false ),
      m_bNeedSave( false ),
      m_autoAlign( false ),
      m_hasExistingPos( false ),
      m_bEditableDesktopIcons( kapp->authorize("editable_desktop_icons") ),
      m_bShowDot( false ),
      m_bVertAlign( true ),
      m_dirLister( 0L ),
      m_mergeDirs(),
      m_dotDirectory( 0L ),
      m_lastDeletedIconPos(),
      m_eSortCriterion( NameCaseInsensitive ),
      m_bSortDirectoriesFirst( true ),
      m_itemsAlwaysFirst(),
      m_gotIconsArea(false)
{
    setResizeMode( Fixed );
    setIconArea( desktopRect() );  // the default is the whole desktop

    // Initialise the shadow data objects...
    m_shadowEngine = new KShadowEngine(new KDesktopShadowSettings(KGlobal::config()));

    connect( QApplication::clipboard(), SIGNAL(dataChanged()),
             this, SLOT(slotClipboardDataChanged()) );

    setURL( desktopURL() ); // sets m_url

    m_desktopDirs = KGlobal::dirs()->findDirs( "appdata", "Desktop" );
    initDotDirectories();

    connect( this, SIGNAL( executed( QIconViewItem * ) ),
             SLOT( slotExecuted( QIconViewItem * ) ) );
    connect( this, SIGNAL( returnPressed( QIconViewItem * ) ),
             SLOT( slotReturnPressed( QIconViewItem * ) ) );
    connect( this, SIGNAL( mouseButtonPressed(int, QIconViewItem*, const QPoint&)),
             SLOT( slotMouseButtonPressed(int, QIconViewItem*, const QPoint&)) );
    connect( this, SIGNAL( mouseButtonClicked(int, QIconViewItem*, const QPoint&)),
             SLOT( slotMouseButtonClickedKDesktop(int, QIconViewItem*, const QPoint&)) );
    connect( this, SIGNAL( contextMenuRequested(QIconViewItem*, const QPoint&)),
             SLOT( slotContextMenuRequested(QIconViewItem*, const QPoint&)) );

    connect( this, SIGNAL( enableAction( const char * , bool ) ),
             SLOT( slotEnableAction( const char * , bool ) ) );

    // Hack: KonqIconViewWidget::slotItemRenamed is not virtual :-(
    disconnect( this, SIGNAL(itemRenamed(QIconViewItem *, const QString &)),
             this, SLOT(slotItemRenamed(QIconViewItem *, const QString &)) );
    connect( this, SIGNAL(itemRenamed(QIconViewItem *, const QString &)),
             this, SLOT(slotItemRenamed(QIconViewItem *, const QString &)) );

    if (!m_bEditableDesktopIcons)
    {
       setItemsMovable(false);
       setAcceptDrops(false);
       viewport()->setAcceptDrops(false);
    }
}

KDIconView::~KDIconView()
{
    if (m_dotDirectory && !m_bEditableDesktopIcons)
      m_dotDirectory->rollback(false); // Don't save positions

    delete m_dotDirectory;
    delete m_dirLister;
    delete m_shadowEngine;
}

void KDIconView::initDotDirectories()
{
    QStringList dirs = m_desktopDirs;
    KURL u = desktopURL();
    if (u.isLocalFile())
       dirs.prepend(u.path());

    QString prefix = iconPositionGroupPrefix();
    QString dotFileName = locateLocal("appdata", "IconPositions");
    if (kdesktop_screen_number != 0)
       dotFileName += "_Desktop" + QString::number(kdesktop_screen_number);

    if (m_dotDirectory && !m_bEditableDesktopIcons)
      m_dotDirectory->rollback(false); // Don't save positions

    delete m_dotDirectory;

    m_dotDirectory = new KSimpleConfig( dotFileName );
    // If we don't allow editable desktop icons, empty m_dotDirectory
    if (!m_bEditableDesktopIcons)
    {
        QStringList groups = m_dotDirectory->groupList();
        QStringList::ConstIterator gIt = groups.begin();
        QStringList::ConstIterator gEnd = groups.end();
        for (; gIt != gEnd; ++gIt )
        {
            m_dotDirectory->deleteGroup(*gIt, true);
        }
    }
    QRect desk = desktopRect();
    QString X_w = QString( "X %1" ).arg( desk.width() );
    QString Y_h = QString( "Y %1" ).arg( desk.height() );
    for ( QStringList::ConstIterator it = dirs.begin() ; it != dirs.end() ; ++it )
    {
        kdDebug(1204) << "KDIconView::initDotDirectories found dir " << *it << endl;
        QString dotFileName = *it + "/.directory";

        if (QFile::exists(dotFileName))
        {
           KSimpleConfig dotDir(dotFileName, true); // Read only

           QStringList groups = dotDir.groupList();
           QStringList::ConstIterator gIt = groups.begin();
           QStringList::ConstIterator gEnd = groups.end();
           for (; gIt != gEnd; ++gIt )
           {
              if ( (*gIt).startsWith(prefix) )
              {
                 dotDir.setGroup( *gIt );
                 m_dotDirectory->setGroup( *gIt );

                 if (!m_dotDirectory->hasKey( X_w ))
                 {
                    int x,y;
                    readIconPosition(&dotDir, x, y);
                    m_dotDirectory->writeEntry( X_w, x );
                    m_dotDirectory->writeEntry( Y_h, y ); // Not persistant!
                 }
              }
           }
        }
    }
}

void KDIconView::initConfig( bool init )
{
    //kdDebug() << "initConfig " << init << endl;

    if ( !init ) {
        KonqFMSettings::reparseConfiguration();
        KDesktopSettings::self()->readConfig();
    }

    KConfig * config = KGlobal::config();

    if ( !init ) {
      KDesktopShadowSettings *shadowSettings = static_cast<KDesktopShadowSettings *>(m_shadowEngine->shadowSettings());
      shadowSettings->setConfig(config);
    }

    setMaySetWallpaper(!config->isImmutable() && !KGlobal::dirs()->isRestrictedResource("wallpaper"));
    m_bShowDot = KDesktopSettings::showHidden();
    m_bVertAlign = KDesktopSettings::vertAlign();
    QStringList oldPreview = previewSettings();
    setPreviewSettings( KDesktopSettings::preview() );

    // read arrange configuration
    m_eSortCriterion  = (SortCriterion)KDesktopSettings::sortCriterion();
    m_bSortDirectoriesFirst = KDesktopSettings::directoriesFirst();
    m_itemsAlwaysFirst = KDesktopSettings::alwaysFirstItems(); // Distributor plug-in

    if (KProtocolInfo::isKnownProtocol(QString::fromLatin1("media")))
        m_enableMedia=KDesktopSettings::mediaEnabled();
    else
        m_enableMedia=false;
    QString tmpList=KDesktopSettings::exclude();
    kdDebug(1204)<<"m_excludeList"<<tmpList<<endl;
    m_excludedMedia=QStringList::split(",",tmpList,false);
    kdDebug(1204)<<" m_excludeList / item count:" <<m_excludedMedia.count()<<endl;
    if ( m_dirLister ) // only when called while running - not on first startup
    {
        configureMedia();
        m_dirLister->setShowingDotFiles( m_bShowDot );
        m_dirLister->emitChanges();
    }

    setArrangement(m_bVertAlign ? TopToBottom : LeftToRight);

    if ( KonqIconViewWidget::initConfig( init ) )
        lineupIcons(); // called if the font changed.

    setAutoArrange( false );

    if ( previewSettings().count() )
    {
        for ( QStringList::ConstIterator it = oldPreview.begin(); it != oldPreview.end(); ++it)
            if ( !previewSettings().contains( *it ) ){
                kdDebug(1204) << "Disabling preview for " << *it << endl;
                if ( *it == "audio/" )
                    disableSoundPreviews();
                else
                {
                    KService::Ptr serv = KService::serviceByDesktopName( *it );
                    Q_ASSERT( serv != 0L );
                    if ( serv )
                    {
                        setIcons( iconSize( ), serv->property("MimeTypes").toStringList() /* revert no-longer wanted previews to icons */ );
                    }
                }
            }
        startImagePreview( QStringList(), true );
    }
    else
    {
        stopImagePreview();
        setIcons( iconSize(), "*" /* stopImagePreview */ );
    }

    if ( !init )
        updateContents();
}

void KDIconView::start()
{
    // We can only start once
    Q_ASSERT(!m_dirLister);
    if (m_dirLister)
        return;

    kdDebug(1204) << "KDIconView::start" << endl;

    // Create the directory lister
    m_dirLister = new KDirLister();

    m_bNeedSave = false;

    connect( m_dirLister, SIGNAL( clear() ), this, SLOT( slotClear() ) );
    connect( m_dirLister, SIGNAL( started(const KURL&) ),
             this, SLOT( slotStarted(const KURL&) ) );
    connect( m_dirLister, SIGNAL( completed() ), this, SLOT( slotCompleted() ) );
    connect( m_dirLister, SIGNAL( newItems( const KFileItemList & ) ),
             this, SLOT( slotNewItems( const KFileItemList & ) ) );
    connect( m_dirLister, SIGNAL( deleteItem( KFileItem * ) ),
             this, SLOT( slotDeleteItem( KFileItem * ) ) );
    connect( m_dirLister, SIGNAL( refreshItems( const KFileItemList & ) ),
             this, SLOT( slotRefreshItems( const KFileItemList & ) ) );

    // Start the directory lister !
    m_dirLister->setShowingDotFiles( m_bShowDot );
    kapp->allowURLAction("list", KURL(), url());
    startDirLister();
    createActions();
}

void KDIconView::configureMedia()
{
    kdDebug(1204) << "***********KDIconView::configureMedia() " <<endl;
    m_dirLister->setMimeExcludeFilter(m_excludedMedia);
    m_dirLister->emitChanges();
    updateContents();
    if (m_enableMedia)
    {
    	for (KURL::List::Iterator it1=m_mergeDirs.begin();it1!=m_mergeDirs.end();++it1)
	    {
	    	if ((*it1).url()=="media:/") return;
	    }
    	m_mergeDirs.append(KURL("media:/"));
    	m_dirLister->openURL(KURL("media:/"),true);
    }
    else
    {
            for (KURL::List::Iterator it2=m_mergeDirs.begin();it2!=m_mergeDirs.end();++it2)
	    {
		if ((*it2).url()=="media:/")
		{
			  delete m_dirLister;
			  m_dirLister=0;
			  start();
//			m_mergeDirs.remove(it2);
//			m_dirLister->stop("media");
			return;
		}

	    }
	    return;
    }

}

void KDIconView::createActions()
{
    if (m_bEditableDesktopIcons)
    {
        KAction *undo = KStdAction::undo( KonqUndoManager::self(), SLOT( undo() ), &m_actionCollection, "undo" );
        connect( KonqUndoManager::self(), SIGNAL( undoAvailable( bool ) ),
             undo, SLOT( setEnabled( bool ) ) );
        connect( KonqUndoManager::self(), SIGNAL( undoTextChanged( const QString & ) ),
             undo, SLOT( setText( const QString & ) ) );
        undo->setEnabled( KonqUndoManager::self()->undoAvailable() );

        KAction* paCut = KStdAction::cut( this, SLOT( slotCut() ), &m_actionCollection, "cut" );
        KShortcut cutShortCut = paCut->shortcut();
        cutShortCut.remove( KKey( SHIFT + Key_Delete ) ); // used for deleting files
        paCut->setShortcut( cutShortCut );

        KStdAction::copy( this, SLOT( slotCopy() ), &m_actionCollection, "copy" );
        KStdAction::paste( this, SLOT( slotPaste() ), &m_actionCollection, "paste" );
        KAction *pasteTo = KStdAction::paste( this, SLOT( slotPopupPasteTo() ), &m_actionCollection, "pasteto" );
        pasteTo->setEnabled( false ); // only enabled during popupMenu()

        KShortcut reloadShortcut = KStdAccel::shortcut(KStdAccel::Reload);
        new KAction( i18n( "&Reload" ), "reload", reloadShortcut, this, SLOT( refreshIcons() ), &m_actionCollection, "reload" );

        (void) new KAction( i18n( "&Rename" ), /*"editrename",*/ Key_F2, this, SLOT( renameSelectedItem() ), &m_actionCollection, "rename" );
        (void) new KAction( i18n( "&Properties" ), ALT+Key_Return, this, SLOT( slotProperties() ), &m_actionCollection, "properties" );
        KAction* trash = new KAction( i18n( "&Move to Trash" ), "edittrash", Key_Delete, &m_actionCollection, "trash" );
        connect( trash, SIGNAL( activated( KAction::ActivationReason, Qt::ButtonState ) ),
                 this, SLOT( slotTrashActivated( KAction::ActivationReason, Qt::ButtonState ) ) );

        KConfig config("kdeglobals", true, false);
        config.setGroup( "KDE" );
        (void) new KAction( i18n( "&Delete" ), "editdelete", SHIFT+Key_Delete, this, SLOT( slotDelete() ), &m_actionCollection, "del" );

        // Initial state of the actions (cut/copy/paste/...)
        slotSelectionChanged();
        //init paste action
        slotClipboardDataChanged();
    }
}

void KDIconView::rearrangeIcons( SortCriterion sc, bool bSortDirectoriesFirst )
{
    m_eSortCriterion = sc;
    m_bSortDirectoriesFirst = bSortDirectoriesFirst;
    rearrangeIcons();
}

void KDIconView::rearrangeIcons()
{
    setupSortKeys();
    sort();  // calls arrangeItemsInGrid() which does not honor iconArea()

    if ( m_autoAlign )
        lineupIcons(  m_bVertAlign ? QIconView::TopToBottom : QIconView::LeftToRight );  // also saves position
    else
    {
        KonqIconViewWidget::lineupIcons(m_bVertAlign ? QIconView::TopToBottom : QIconView::LeftToRight);
        saveIconPositions();
    }
}

void KDIconView::lineupIcons()
{
    if ( !m_gotIconsArea ) return;
    KonqIconViewWidget::lineupIcons();
    saveIconPositions();
}

void KDIconView::setAutoAlign( bool b )
{
    m_autoAlign = b;

    // Auto line-up icons
    if ( b ) {
            lineupIcons();
        connect( this, SIGNAL( iconMoved() ),
                 this, SLOT( lineupIcons() ) );
    }
    else {
        // change maxItemWidth, because when grid-align was active, it changed this for the grid
        int sz = iconSize() ? iconSize() : KGlobal::iconLoader()->currentSize( KIcon::Desktop );
        setMaxItemWidth( QMAX( QMAX( sz, previewIconSize( iconSize() ) ), KonqFMSettings::settings()->iconTextWidth() ) );
        setFont( font() );  // Force calcRect()

        disconnect( this, SIGNAL( iconMoved() ),
                    this, SLOT( lineupIcons() ) );
    }
}

void KDIconView::startDirLister()
{
    // if desktop is resized before start() is called (XRandr)
    if (!m_dirLister) return;

    m_dirLister->openURL( url() );

    // Gather the list of directories to merge into the desktop
    // (the main URL is desktopURL(), no need for it in the m_mergeDirs list)
    m_mergeDirs.clear();
    for ( QStringList::ConstIterator it = m_desktopDirs.begin() ; it != m_desktopDirs.end() ; ++it )
    {
        kdDebug(1204) << "KDIconView::desktopResized found merge dir " << *it << endl;
        KURL u;
        u.setPath( *it );
        m_mergeDirs.append( u );
        // And start listing this dir right now
        kapp->allowURLAction("list", KURL(), u);
        m_dirLister->openURL( u, true );
    }
    configureMedia();
}

void KDIconView::lineupIcons(QIconView::Arrangement align)
{
    m_bVertAlign = ( align == QIconView::TopToBottom );
    setArrangement( m_bVertAlign ? TopToBottom : LeftToRight );

    if ( m_autoAlign )
    {
        KonqIconViewWidget::lineupIcons( align );
        saveIconPositions();
    }
    else
        rearrangeIcons();  // also saves the position

    KDesktopSettings::setVertAlign( m_bVertAlign );
    KDesktopSettings::writeConfig();
}

// Only used for DCOP
QStringList KDIconView::selectedURLs()
{
    QStringList seq;

    QIconViewItem *it = firstItem();
    for (; it; it = it->nextItem() )
        if ( it->isSelected() ) {
            KFileItem *fItem = ((KFileIVI *)it)->item();
            seq.append( fItem->url().url() ); // copy the URL
        }

    return seq;
}

void KDIconView::recheckDesktopURL()
{
    // Did someone change the path to the desktop ?
    kdDebug(1204) << desktopURL().url() << endl;
    kdDebug(1204) << url().url() << endl;
    if ( desktopURL() != url() )
    {
        kdDebug(1204) << "Desktop path changed from " << url().url() <<
            " to " << desktopURL().url() << endl;
        setURL( desktopURL() ); // sets m_url
        initDotDirectories();
        m_dirLister->openURL( url() );
    }
}

KURL KDIconView::desktopURL()
{
    // Support both paths and URLs
    QString desktopPath = KGlobalSettings::desktopPath();
    if (kdesktop_screen_number != 0) {
        QString dn = "Desktop";
        dn += QString::number(kdesktop_screen_number);
        desktopPath.replace("Desktop", dn);
    }

    KURL desktopURL;
    if (desktopPath[0] == '/')
        desktopURL.setPath(desktopPath);
    else
        desktopURL = desktopPath;

    Q_ASSERT( desktopURL.isValid() );
    if ( !desktopURL.isValid() ) { // should never happen
        KURL u;
        u.setPath(  QDir::homeDirPath() + "/" + "Desktop" + "/" );
        return u;
    }

    return desktopURL;
}

void KDIconView::contentsMousePressEvent( QMouseEvent *e )
{
    if (!m_dirLister) return;
    //kdDebug(1204) << "KDIconView::contentsMousePressEvent" << endl;
    // QIconView, as of Qt 2.2, doesn't emit mouseButtonPressed for LMB on background
    if ( e->button() == LeftButton && KRootWm::self()->hasLeftButtonMenu() )
    {
        QIconViewItem *item = findItem( e->pos() );
        if ( !item )
        {
            // Left click menu
            KRootWm::self()->mousePressed( e->globalPos(), e->button() );
            return;
        }
    }
    KonqIconViewWidget::contentsMousePressEvent( e );
}

void KDIconView::mousePressEvent( QMouseEvent *e )
{
    KRootWm::self()->mousePressed( e->globalPos(), e->button() );
}

void KDIconView::wheelEvent( QWheelEvent* e )
{
    if (!m_dirLister) return;
    //kdDebug(1204) << "KDIconView::wheelEvent" << endl;

    QIconViewItem *item = findItem( e->pos() );
    if ( !item )
    {
      emit wheelRolled( e->delta() );
      return;
    }

    KonqIconViewWidget::wheelEvent( e );
}

void KDIconView::slotProperties()
{
    KFileItemList selectedFiles = selectedFileItems();

    if( selectedFiles.isEmpty() )
      return;

    (void) new KPropertiesDialog( selectedFiles );
}

void KDIconView::slotContextMenuRequested(QIconViewItem *_item, const QPoint& _global)
{
    if (_item)
    {
      ((KFileIVI*)_item)->setSelected( true );
      popupMenu( _global, selectedFileItems() );
    }
}

void KDIconView::slotMouseButtonPressed(int _button, QIconViewItem* _item, const QPoint& _global)
{
    //kdDebug(1204) << "KDIconView::slotMouseButtonPressed" << endl;
    if (!m_dirLister) return;
    m_lastDeletedIconPos = QPoint(); // user action -> not renaming an icon
    if(!_item)
        KRootWm::self()->mousePressed( _global, _button );
}

void KDIconView::slotMouseButtonClickedKDesktop(int _button, QIconViewItem* _item, const QPoint&)
{
    if (!m_dirLister) return;
    //kdDebug(1204) << "KDIconView::slotMouseButtonClickedKDesktop" << endl;
    if ( _item && _button == MidButton )
        slotExecuted(_item);
}

// -----------------------------------------------------------------------------

void KDIconView::slotReturnPressed( QIconViewItem *item )
{
    if (item && item->isSelected())
        slotExecuted(item);
}

// -----------------------------------------------------------------------------

void KDIconView::slotExecuted( QIconViewItem *item )
{
    kapp->propagateSessionManager();
    m_lastDeletedIconPos = QPoint(); // user action -> not renaming an icon
    if (item) {
        visualActivate(item);
        ((KFileIVI*)item)->returnPressed();
    }
}

// -----------------------------------------------------------------------------

void KDIconView::slotCut()
{
    cutSelection();
}

// -----------------------------------------------------------------------------

void KDIconView::slotCopy()
{
    copySelection();
}

// -----------------------------------------------------------------------------

void KDIconView::slotPaste()
{
    KonqOperations::doPaste(this, url(), KRootWm::self()->desktopMenuPosition());
}

void KDIconView::slotPopupPasteTo()
{
    Q_ASSERT( !m_popupURL.isEmpty() );
    if ( !m_popupURL.isEmpty() )
        paste( m_popupURL );
}

/**
 * The files on the desktop come from a variety of sources.
 * If an attempt is made to delete a .desktop file that does
 * not originate from the users own Desktop directory then
 * a .desktop file with "Hidden=true" is written to the users
 * own Desktop directory to hide the file.
 *
 * Returns true if all selected items have been deleted.
 * Returns false if there are selected items remaining that
 * still need to be deleted in a regular fashion.
 */
bool KDIconView::deleteGlobalDesktopFiles()
{
    KURL desktop_URL = desktopURL();
    if (!desktop_URL.isLocalFile())
        return false; // Dunno how to do this.

    QString desktopPath = desktop_URL.path();

    bool itemsLeft = false;
    QIconViewItem *it = 0;
    QIconViewItem *nextIt = firstItem();
    for (; (it = nextIt); )
    {
        nextIt = it->nextItem();
        if ( !it->isSelected() )
            continue;

        KFileItem *fItem = ((KFileIVI *)it)->item();
        if (fItem->url().path().startsWith(desktopPath))
        {
            itemsLeft = true;
            continue; // File is in users own Desktop directory
        }

        if (!isDesktopFile(fItem))
        {
            itemsLeft = true;
            continue; // Not a .desktop file
        }

        KDesktopFile df(desktopPath + fItem->url().fileName());
        df.writeEntry("Hidden", true);
        df.sync();

        delete it;
    }
    return !itemsLeft;
}

void KDIconView::slotTrashActivated( KAction::ActivationReason reason, Qt::ButtonState state )
{
    if (deleteGlobalDesktopFiles())
       return; // All items deleted

    if ( reason == KAction::PopupMenuActivation && ( state & Qt::ShiftButton ) )
       KonqOperations::del(this, KonqOperations::DEL, selectedUrls());
    else
       KonqOperations::del(this, KonqOperations::TRASH, selectedUrls());
}

void KDIconView::slotDelete()
{
    if (deleteGlobalDesktopFiles())
       return; // All items deleted
    KonqOperations::del(this, KonqOperations::DEL, selectedUrls());
}

// -----------------------------------------------------------------------------

// This method is called when right-clicking over one or more items
// Not to be confused with the global popup-menu, KRootWm, when doing RMB on the desktop
void KDIconView::popupMenu( const QPoint &_global, const KFileItemList& _items )
{
    if (!kapp->authorize("action/kdesktop_rmb")) return;
    if (!m_dirLister) return;
    if ( _items.count() == 1 )
        m_popupURL = _items.getFirst()->url();

    KAction* pasteTo = m_actionCollection.action( "pasteto" );
    if (pasteTo)
        pasteTo->setEnabled( m_actionCollection.action( "paste" )->isEnabled() );

    bool hasMediaFiles = false;
    KFileItemListIterator it(_items);
    for (; it.current() && !hasMediaFiles; ++it) {
        hasMediaFiles = it.current()->url().protocol() == "media";
    }

    KParts::BrowserExtension::PopupFlags itemFlags = KParts::BrowserExtension::DefaultPopupItems;
    if ( hasMediaFiles )
        itemFlags |= KParts::BrowserExtension::NoDeletion;
    KonqPopupMenu * popupMenu = new KonqPopupMenu( KonqBookmarkManager::self(), _items,
                                                   url(),
                                                   m_actionCollection,
                                                   KRootWm::self()->newMenu(),
                                                   this,
                                                   KonqPopupMenu::ShowProperties | KonqPopupMenu::ShowNewWindow,
                                                   itemFlags );

    popupMenu->exec( _global );
    delete popupMenu;
    m_popupURL = KURL();
    if (pasteTo)
        pasteTo->setEnabled( false );
}

void KDIconView::slotNewMenuActivated()
{
    //kdDebug(1204) << "KDIconView::slotNewMenuActivated" << endl;
    // New / <template> was chosen, a new file is going to appear soon,
    // make it appear at the position of the popupmenu.
    m_nextItemPos = KRootWm::self()->desktopMenuPosition();
}

// -----------------------------------------------------------------------------

void KDIconView::slotEnableAction( const char * name, bool enabled )
{
  //kdDebug(1204) << "slotEnableAction " << name << " enabled=" << enabled << endl;
  QCString sName( name );
  // No such actions here... konqpopupmenu provides them.
  if ( sName == "properties" || sName == "editMimeType" )
    return;

  KAction * act = m_actionCollection.action( sName.data() );
  if (act)
    act->setEnabled( enabled );
}

// -----------------------------------------------------------------------------

// Straight from kpropsdlg :)
bool KDIconView::isDesktopFile( KFileItem * _item ) const
{
  // only local files
  if ( !_item->isLocalFile() )
    return false;

  // only regular files
  if ( !S_ISREG( _item->mode() ) )
    return false;

  QString t( _item->url().path() );

  // only if readable
  if ( access( QFile::encodeName(t), R_OK ) != 0 )
    return false;

  // return true if desktop file
  return ( _item->mimetype() == QString::fromLatin1("application/x-desktop") );
}

QString KDIconView::stripDesktopExtension( const QString & text )
{
    if (text.right(7) == QString::fromLatin1(".kdelnk"))
      return text.left(text.length() - 7);
    else if (text.right(8) == QString::fromLatin1(".desktop"))
      return text.left(text.length() - 8);
    return text;
}

bool KDIconView::makeFriendlyText( KFileIVI *fileIVI )
{
    KFileItem *item = fileIVI->item();
    QString desktopFile;
    if ( item->isDir() && item->isLocalFile() )
    {
        KURL u( item->url() );
        u.addPath( ".directory" );
        // using KStandardDirs as this one checks for path being
        // a file instead of a directory
        if ( KStandardDirs::exists( u.path() ) )
            desktopFile = u.path();
    }
    else if ( isDesktopFile( item ) )
    {
        desktopFile = item->url().path();
    }

    if ( !desktopFile.isEmpty() )
    {
        KSimpleConfig cfg( desktopFile, true );
        cfg.setDesktopGroup();
        if (cfg.readBoolEntry("Hidden"))
            return false;

        if (cfg.readBoolEntry( "NoDisplay", false ))
            return false;

        QStringList tmpList;
        if (cfg.hasKey("OnlyShowIn"))
        {
            if (!cfg.readListEntry("OnlyShowIn", ';').contains("KDE"))
                return false;
        }
        if (cfg.hasKey("NotShowIn"))
        {
            if (cfg.readListEntry("NotShowIn", ';').contains("KDE"))
                return false;
        }
        if (cfg.hasKey("TryExec"))
        {
            if (KStandardDirs::findExe( cfg.readEntry( "TryExec" ) ).isEmpty())
                return false;
        }

        QString name = cfg.readEntry("Name");
        if ( !name.isEmpty() )
            fileIVI->setText( name );
        else
            // For compatibility
            fileIVI->setText( stripDesktopExtension( fileIVI->text() ) );
    }
    return true;
}

// -----------------------------------------------------------------------------

void KDIconView::slotClear()
{
    clear();
}

// -----------------------------------------------------------------------------

void KDIconView::slotNewItems( const KFileItemList & entries )
{
  bool firstRun = (count() == 0);  // no icons yet, this seems to be the initial loading

  // delay updates until all new items have been created
  setUpdatesEnabled( false );
  QRect area = iconArea();
  setIconArea( QRect(  0, 0, -1, -1 ) );

  QString desktopPath;
  KURL desktop_URL = desktopURL();
  if (desktop_URL.isLocalFile())
    desktopPath = desktop_URL.path();
  // We have new items, so we'll need to repaint in slotCompleted
  m_bNeedRepaint = true;
  kdDebug(1214) << "KDIconView::slotNewItems count=" << entries.count() << endl;
  KFileItemListIterator it(entries);
  KFileIVI* fileIVI = 0L;
  for (; it.current(); ++it)
  {
    KURL url = it.current()->url();
    if (!desktopPath.isEmpty() && url.isLocalFile() && !url.path().startsWith(desktopPath))
    {
      QString fileName = url.fileName();
      if (QFile::exists(desktopPath + fileName))
         continue; // Don't duplicate entry

      QString mostLocal = locate("appdata", "Desktop/"+fileName);
      if (!mostLocal.isEmpty() && (mostLocal != url.path()))
         continue; // Don't duplicate entry
    }

    // No delayed mimetype determination on the desktop
    it.current()->determineMimeType();
    fileIVI = new KFileIVIDesktop( this, it.current(), iconSize(), m_shadowEngine );
    if (!makeFriendlyText( fileIVI ))
    {
      delete fileIVI;
      continue;
    }

    kdDebug(1214) << " slotNewItems: " << url.url() << " text: " << fileIVI->text() << endl;
    fileIVI->setRenameEnabled( false );

    if ( !m_nextItemPos.isNull() ) // position remembered from e.g. RMB-popupmenu position, when doing New/...
    {
      kdDebug(1214) << "slotNewItems : using popupmenu position " << m_nextItemPos.x() << "," << m_nextItemPos.y() << endl;
      fileIVI->move( m_nextItemPos.x(), m_nextItemPos.y() );
      m_nextItemPos = QPoint();
    }
    else
    {
      kdDebug(1214) << "slotNewItems : trying to read position from .directory file"<<endl;
      QString group = iconPositionGroupPrefix();
      QString filename = url.fileName();
      if ( filename.endsWith(".part") && !m_dotDirectory->hasGroup( group + filename ) )
          filename = filename.left( filename.length() - 5 );
      group.append( filename );
      kdDebug(1214) << "slotNewItems : looking for group " << group << endl;
      if ( m_dotDirectory->hasGroup( group ) )
      {
        m_dotDirectory->setGroup( group );
        m_hasExistingPos = true;
        int x,y;
        readIconPosition(m_dotDirectory, x, y);

        kdDebug(1214)<<"slotNewItems() x: "<<x<<" y: "<<y<<endl;

        QRect oldPos = fileIVI->rect();
        fileIVI->move( x, y );
        if ( !firstRun && !isFreePosition( fileIVI ) ) // if we can't put it there, then let QIconView decide
        {
            kdDebug(1214)<<"slotNewItems() pos was not free :-("<<endl;
            fileIVI->move( oldPos.x(), oldPos.y() );
            m_dotDirectory->deleteGroup( group );
            m_bNeedSave = true;
        }
        else
        {
            kdDebug(1214)<<"Using saved position"<<endl;
        }
      }
      else
      {
            // Not found, we'll need to save the new pos
            kdDebug(1214)<<"slotNewItems(): New item without position information, try to find a sane location"<<endl;

            moveToFreePosition(fileIVI);

            m_bNeedSave = true;
      }
    }
  }

  setIconArea( area );

  // align on grid
  if ( m_autoAlign )
      lineupIcons();

  setUpdatesEnabled( true );
}

// -----------------------------------------------------------------------------

// see also KonqKfmIconView::slotRefreshItems
void KDIconView::slotRefreshItems( const KFileItemList & entries )
{
    kdDebug(1204) << "KDIconView::slotRefreshItems" << endl;
    bool bNeedPreviewJob = false;
    KFileItemListIterator rit(entries);
    for (; rit.current(); ++rit)
    {
        bool found = false;
        QIconViewItem *it = firstItem();
        for ( ; it ; it = it->nextItem() )
        {
            KFileIVI * fileIVI = static_cast<KFileIVI *>(it);
            if ( fileIVI->item() == rit.current() ) // compare the pointers
            {
                kdDebug(1204) << "KDIconView::slotRefreshItems refreshing icon " << fileIVI->item()->url().url() << endl;
                found = true;
                fileIVI->setText( rit.current()->text() );
                if (!makeFriendlyText( fileIVI ))
                {
                    delete fileIVI;
                    break;
                }
                if ( fileIVI->isThumbnail() ) {
                    bNeedPreviewJob = true;
                    fileIVI->invalidateThumbnail();
                }
                else
                    fileIVI->refreshIcon( true );
                if ( rit.current()->isMimeTypeKnown() )
                    fileIVI->setMouseOverAnimation( rit.current()->iconName() );
                break;
            }
        }
	if ( !found )
            kdDebug(1204) << "Item not found: " << rit.current()->url().url() << endl;
    }
    if ( bNeedPreviewJob && previewSettings().count() )
    {
        startImagePreview( QStringList(), false );
    }
    else
    {
        // In case we replace a big icon with a small one, need to repaint.
        updateContents();
        // Can't do that with m_bNeedRepaint since slotCompleted isn't called
        m_bNeedRepaint = false;
    }
}


void KDIconView::refreshIcons()
{
    QIconViewItem *it = firstItem();
    for ( ; it ; it = it->nextItem() )
    {
        KFileIVI * fileIVI = static_cast<KFileIVI *>(it);
        fileIVI->item()->refresh();
        fileIVI->refreshIcon( true );
        makeFriendlyText( fileIVI );
    }
}


void KDIconView::FilesAdded( const KURL & directory )
{
    if ( directory.path().length() <= 1 && directory.protocol() == "trash" )
        refreshTrashIcon();
}

void KDIconView::FilesRemoved( const KURL::List & fileList )
{
    if ( !fileList.isEmpty() ) {
        const KURL url = fileList.first();
        if ( url.protocol() == "trash" )
            refreshTrashIcon();
    }
}

void KDIconView::refreshTrashIcon()
{
    QIconViewItem *it = firstItem();
    for ( ; it ; it = it->nextItem() )
    {
        KFileIVI * fileIVI = static_cast<KFileIVI *>(it);
        KFileItem* item = fileIVI->item();
        if ( isDesktopFile( item ) ) {
            KSimpleConfig cfg( item->url().path(), true );
            cfg.setDesktopGroup();
            if ( cfg.readEntry( "Type" ) == "Link" &&
                 cfg.readEntry( "URL" ) == "trash:/" ) {
                fileIVI->refreshIcon( true );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void KDIconView::slotDeleteItem( KFileItem * _fileitem )
{
    kdDebug(1204) << "KDIconView::slotDeleteItems" << endl;
    // we need to find out the KFileIVI containing the fileitem
    QIconViewItem *it = firstItem();
    while ( it ) {
      KFileIVI * fileIVI = static_cast<KFileIVI *>(it);
      if ( fileIVI->item() == _fileitem ) { // compare the pointers
        // Delete this item.
        //kdDebug(1204) << fileIVI->text() << endl;

        QString group = iconPositionGroupPrefix();
        group.append( fileIVI->item()->url().fileName() );
        if ( m_dotDirectory->hasGroup( group ) )
            m_dotDirectory->deleteGroup( group );

        m_lastDeletedIconPos = fileIVI->pos();
        delete fileIVI;
        break;
      }
      it = it->nextItem();
    }
    m_bNeedRepaint = true;
}

// -----------------------------------------------------------------------------

void KDIconView::slotStarted( const KURL& _url )
{
    kdDebug(1204) << "KDIconView::slotStarted url: " << _url.url() << " url().url(): "<<url().url()<<endl;
}

void KDIconView::slotCompleted()
{
    // Root item ? Store in konqiconviewwidget (used for drops onto the background, for instance)
    if ( m_dirLister->rootItem() )
      setRootItem( m_dirLister->rootItem() );

    if ( previewSettings().count() )
        startImagePreview( QStringList(), true );
    else
    {
        stopImagePreview();
        setIcons( iconSize(), "*" /* stopImagePreview */ );
    }

    // during first run need to rearrange all icons so default config settings will be used
    kdDebug(1204)<<"slotCompleted() m_hasExistingPos: "<<(m_hasExistingPos?(int)1:(int)0)<<endl;
    if (!m_hasExistingPos)
        rearrangeIcons();

//    kdDebug(1204) << "KDIconView::slotCompleted save:" << m_bNeedSave << " repaint:" << m_bNeedRepaint << endl;
    if ( m_bNeedSave )
    {
        // Done here because we want to align icons only once initially, and each time new icons appear.
        // This MUST precede the call to saveIconPositions().
        emit iconMoved();
        saveIconPositions();
        m_hasExistingPos = true; // if we didn't have positions, we have now.
        m_bNeedSave = false;
    }
    if ( m_bNeedRepaint )
    {
        viewport()->repaint();
        m_bNeedRepaint = false;
    }
}

void KDIconView::slotClipboardDataChanged()
{
    // This is very related to KonqDirPart::slotClipboardDataChanged

    KURL::List lst;
    QMimeSource *data = QApplication::clipboard()->data();
    if ( data->provides( "application/x-kde-cutselection" ) && data->provides( "text/uri-list" ) )
        if ( KonqDrag::decodeIsCutSelection( data ) )
            (void) KURLDrag::decode( data, lst );

    disableIcons( lst );

    QString actionText = KIO::pasteActionText();
    bool paste = !actionText.isEmpty();
    if ( paste ) {
        KAction* pasteAction = m_actionCollection.action( "paste" );
        if ( pasteAction )
            pasteAction->setText( actionText );
    }
    slotEnableAction( "paste", paste );
}

void KDIconView::renameDesktopFile(const QString &path, const QString &name)
{
    KDesktopFile cfg( path, false );

    // if we don't have the desktop entry group, then we assume that
    // it's not a config file (and we don't nuke it!)
    if ( !cfg.hasGroup( "Desktop Entry" ) )
      return;

    if ( cfg.readName() == name )
      return;

    cfg.writeEntry( "Name", name, true, false, false );
    cfg.writeEntry( "Name", name, true, false, true );
    cfg.sync();
}

void KDIconView::slotItemRenamed(QIconViewItem* _item, const QString &name)
{
    kdDebug(1204) << "KDIconView::slotItemRenamed(item, \"" << name << "\" )" << endl;
    QString newName(name);
    if ( _item)
    {
       KFileIVI *fileItem = static_cast< KFileIVI* >( _item );
       //save position of item renamed
       m_lastDeletedIconPos = fileItem->pos();
       if ( fileItem->item() && !fileItem->item()->isLink() )
       {
          QString desktopFile( fileItem->item()->url().path() );
          if (!desktopFile.isEmpty())
          {
             // first and foremost, we make sure that this is a .desktop file
             // before we write anything to it
             KMimeType::Ptr type = KMimeType::findByURL( fileItem->item()->url() );
             bool bDesktopFile = false;

             if (type->name() == "application/x-desktop")
             {
                bDesktopFile = true;
                if (!newName.endsWith(".desktop"))
                   newName += ".desktop";
             }
             else if(type->name() == "inode/directory") {
                desktopFile += "/.directory";
                bDesktopFile = true;
             }

             if (QFile(desktopFile).exists() && bDesktopFile)
             {
                renameDesktopFile(desktopFile, name);
                return;
             }
          }
       }
   }
   KonqIconViewWidget::slotItemRenamed(_item, newName);
}

void KDIconView::slotAboutToCreate(const QPoint &pos, const QValueList<KIO::CopyInfo> &files)
{
   if (pos.isNull())
      return;

    if (m_dropPos != pos)
    {
       m_dropPos = pos;
       m_lastDropPos = pos;
    }

    QString dir = url().path(-1); // Strip trailing /

    QValueList<KIO::CopyInfo>::ConstIterator it = files.begin();
    int gridX = gridXValue();
    int gridY = 120; // 120 pixels should be enough for everyone (tm)

    for ( ; it!= files.end() ; ++it )
    {
        kdDebug(1214) << "KDIconView::saveFuturePosition x=" << m_lastDropPos.x() << " y=" << m_lastDropPos.y() << " filename=" << (*it).uDest.prettyURL() << endl;
        if ((*it).uDest.isLocalFile() && ((*it).uDest.directory() == dir))
        {
           m_dotDirectory->setGroup( iconPositionGroupPrefix() + (*it).uDest.fileName() );
           saveIconPosition(m_dotDirectory, m_lastDropPos.x(), m_lastDropPos.y());
           int dX = m_lastDropPos.x() - m_dropPos.x();
           int dY = m_lastDropPos.y() - m_dropPos.y();
           if ((QABS(dX) > QABS(dY)) || (m_lastDropPos.x() + 2*gridX > width()))
              m_lastDropPos = QPoint(m_dropPos.x(), m_lastDropPos.y() + gridY);
           else
              m_lastDropPos = QPoint(m_lastDropPos.x() + gridX, m_lastDropPos.y());
        }
    }
    m_dotDirectory->sync();
}

// -----------------------------------------------------------------------------

void KDIconView::showEvent( QShowEvent *e )
{
    //HACK to avoid QIconView calling arrangeItemsInGrid (Simon)
    //EVEN MORE HACK: unfortunately, QScrollView has no concept of
    //TopToBottom, therefore, it always adds LeftToRight.  So, if any of
    //the icons have a setting, we'll use QScrollView.. but otherwise,
    //we use the iconview
   //kdDebug(1204)<<"showEvent() m_hasExistingPos: "<<(m_hasExistingPos?(int)1:(int)0)<<endl;
    if (m_hasExistingPos)
        QScrollView::showEvent( e );
    else
        KIconView::showEvent( e );
}

void KDIconView::contentsDropEvent( QDropEvent * e )
{
    kdDebug(1204)<<"void KDIconView::contentsDropEvent( QDropEvent * e )\n";
    // mind: if it's a filedrag which itself is an image, libkonq is called. There's a popup for drops as well
    // that contains the same line "Set as Wallpaper" in void KonqOperations::asyncDrop
    bool isColorDrag = KColorDrag::canDecode(e);
    bool isImageDrag = QImageDrag::canDecode(e);
    bool isUrlDrag = KURLDrag::canDecode(e);

    bool isImmutable = KGlobal::config()->isImmutable();

    if ( (isColorDrag || isImageDrag) && !isUrlDrag ) {
        // Hack to clear the drag shape
        bool bMovable = itemsMovable();
        bool bSignals = signalsBlocked();
        setItemsMovable(false);
        blockSignals(true);
        KIconView::contentsDropEvent( e );
        blockSignals(bSignals);
        setItemsMovable(bMovable);
        // End hack

        if ( !isImmutable ) // just ignore event in kiosk-mode
        {
            if ( isColorDrag)
               emit colorDropEvent( e );
            else if (isImageDrag)
               emit imageDropEvent( e );
        }
    } else {
        setLastIconPosition( e->pos() );
        KonqIconViewWidget::contentsDropEvent( e );
    }

    // Check if any items have been moved outside the desktop area.
    // If we find any, move them right back in there. (#40418)
    QRect desk = desktopRect();
    bool adjustedAnyItems = false;
    for( QIconViewItem *item = firstItem(); item; item = item->nextItem() )
    {
        if( !desk.contains( item->rect(), true ))
        {
            QRect r = item->rect();

            if( r.top() < 0 )
                r.moveTop( 0 );
            if( r.bottom() > rect().bottom() )
                r.moveBottom( rect().bottom() );
            if( r.left() < 0 )
                r.moveLeft( 0 );
            if( r.right() > rect().right() )
                r.moveRight( rect().right() );

            item->move( r.x(), r.y() );
            adjustedAnyItems = true;
        }
    }
    if( adjustedAnyItems )
    {
        // Make sure the viewport isn't unnecessarily resized by now,
        // then schedule a repaint to remove any garbage pixels.
        resizeContents( width(), height() );
        viewport()->update();
    }

    if (QIconDrag::canDecode(e)) {
      emit iconMoved();
      if ( !m_autoAlign )    // if autoAlign, positions were saved in lineupIcons
        saveIconPositions();
    }
}

// don't scroll when someone uses his nifty mouse wheel
void KDIconView::viewportWheelEvent( QWheelEvent * e )
{
    e->accept();
}

void KDIconView::updateWorkArea( const QRect &wr )
{
    m_gotIconsArea = true;  // now we have it!

    if ( iconArea() == wr ) return;  // nothing changed; avoid repaint/saveIconPosition ...

    QRect oldArea = iconArea();
    setIconArea( wr );

    kdDebug(1204) << "KDIconView::updateWorkArea wr: " << wr.x() << "," << wr.y()
              << " " << wr.width() << "x" << wr.height() << endl;
    kdDebug(1204) << "  oldArea:                     " << oldArea.x() << "," << oldArea.y()
              << " " << oldArea.width() << "x" << oldArea.height() << endl;

    if ( m_autoAlign )
        lineupIcons();
    else {
        bool needRepaint = false;
        QIconViewItem* item;
        int dx, dy;

        dx = wr.left() - oldArea.left();
        dy = wr.top() - oldArea.top();

        if ( dx != 0 || dy != 0 ) {
          if ( (dx > 0) || (dy > 0) ) // the iconArea was shifted right/down; less space now
              for ( item = firstItem(); item; item = item->nextItem() ) {
                  // check if there is any item inside the now unavailable area
                  // If so, we have to move _all_ items
                  // If not, we don't have to move any item (avoids bug:117868)
                  if ( (item->x() < wr.x()) || (item->y() < wr.y()) ) {
                    needRepaint = true;
                    break;
                  }
              }
          else  // the iconArea was shifted left/up; more space now - use it
              needRepaint = true;

            if ( needRepaint )
                for ( item = firstItem(); item; item = item->nextItem() )
                    item->moveBy( dx, dy );
        }

        for ( item = firstItem(); item; item = item->nextItem() ) {
            QRect r( item->rect() );
            int dx = 0, dy = 0;
            if ( r.bottom() > wr.bottom() )
                dy = wr.bottom() - r.bottom() - 1;
            if ( r.right() > wr.right() )
                dx = wr.right() - r.right() - 1;
            if ( dx != 0 || dy != 0 ) {
                needRepaint = true;
                item->moveBy( dx, dy );
            }
        }
        if ( needRepaint ) {
            viewport()->repaint( FALSE );
            repaint( FALSE );
            saveIconPositions();
        }
    }
}

void KDIconView::setupSortKeys()
{
    // can't use sorting in KFileIVI::setKey()
    setProperty("sortDirectoriesFirst", QVariant(false, 0));

    for (QIconViewItem *it = firstItem(); it; it = it->nextItem())
    {
        QString strKey;

        if (!m_itemsAlwaysFirst.isEmpty())
        {
            QString strFileName = static_cast<KFileIVI *>( it )->item()->url().fileName();
            int nFind = m_itemsAlwaysFirst.findIndex(strFileName);
            if (nFind >= 0)
                strKey = "0" + QString::number(nFind);
        }

        if (strKey.isEmpty())
        {
            switch (m_eSortCriterion)
            {
            case NameCaseSensitive:
                strKey = it->text();
                break;
            case NameCaseInsensitive:
                strKey = it->text().lower();
                break;
            case Size:
                strKey = KIO::number(static_cast<KFileIVI *>( it )->item()->size()).rightJustify(20, '0');
                break;
            case Type:
                // Sort by Type + Name (#17014)
                strKey = static_cast<KFileIVI *>( it )->item()->mimetype() + '~' + it->text().lower();
                break;
            case Date:
                QDateTime dayt;
                dayt.setTime_t( static_cast<KFileIVI *>( it )->
                    item()->time( KIO::UDS_MODIFICATION_TIME ) );
                strKey = dayt.toString( "yyyyMMddhhmmss" );
                break;
            }

            if (m_bSortDirectoriesFirst)
            {
                if (S_ISDIR(static_cast<KFileIVI *>( it )->item()->mode()))
                    strKey.prepend(sortDirection() ? '1' : '2');
                else
                    strKey.prepend(sortDirection() ? '2' : '1' );
            }
            else
                strKey.prepend('1');
        }

        it->setKey(strKey);
    }
}

bool KDIconView::isFreePosition( const QIconViewItem *item ) const
{
    QRect r = item->rect();
    QIconViewItem *it = firstItem();
    for (; it; it = it->nextItem() )
    {
        if ( !it->rect().isValid() || it == item )
            continue;

        if ( it->intersects( r ) )
            return false;
    }

    return true;
}

bool KDIconView::isFreePosition( const QIconViewItem *item ,const QRect& rect) const
{
    QIconViewItem *it = firstItem();
    for (; it; it = it->nextItem() )
    {
        if ( !rect.isValid() || it == item )
            continue;

        if ( it->intersects( rect ) )
            return false;
    }

    return true;
}

void KDIconView::setLastIconPosition( const QPoint &_pos )
{
    m_lastDeletedIconPos = _pos;
}

void KDIconView::moveToFreePosition(QIconViewItem *item )
{
    bool success;
    // It may be that a file has been renamed. In this case,
    // m_lastDeletedIconPos is the position to use for this "apparently new" item.
    // (We rely on deleteItem being now emitted before newItems).
    if ( !m_lastDeletedIconPos.isNull() )
        // Problem is: I'd like to compare those two file's attributes
        // (size, creation time, modification time... etc.) but since renaming
        // is done by kpropsdlg, all of those can have changed (and creation time
        // is different since the new file is a copy!)
    {
        kdDebug(1214) << "Moving " << item->text() << " to position of last deleted icon." << endl;
        item->move( m_lastDeletedIconPos );
        m_lastDeletedIconPos = QPoint();
        return;
    }

    //try to find a free place to put the item, honouring the m_bVertAlign property
    QRect rect=item->rect();
    if (m_bVertAlign)
    {
	kdDebug(1214)<<"moveToFreePosition for vertical alignment"<<endl;

	rect.moveTopLeft(QPoint(spacing(),spacing()));
      do
      {
          success=false;
          while (rect.bottom()<height())
          {
   	     if (!isFreePosition(item,rect))
		{
	                rect.moveBy(0,rect.height()+spacing());
		}
	     else
	      {
                 success=true;
                 break;
	      }
          }

          if (!success)
          {
		rect.moveTopLeft(QPoint(rect.right()+spacing(),spacing()));
          } else break;
      }
      while (item->rect().right()<width());
      if (success)
	item->move(rect.x(),rect.y());
      else
        item->move(width()-spacing()-item->rect().width(),height()-spacing()-item->rect().height());

    }

}


void KDIconView::saveIconPositions()
{
  kdDebug(1214) << "KDIconView::saveIconPositions" << endl;

  if (!m_bEditableDesktopIcons)
    return; // Don't save position

  QString prefix = iconPositionGroupPrefix();
  QIconViewItem *it = firstItem();
  if ( !it )
    return; // No more icons. Maybe we're closing and they've been removed already

  while ( it )
  {
    KFileIVI *ivi = static_cast<KFileIVI *>( it );
    KFileItem *item = ivi->item();

    m_dotDirectory->setGroup( prefix + item->url().fileName() );
    kdDebug(1214) << "KDIconView::saveIconPositions " << item->url().fileName() << " " << it->x() << " " << it->y() << endl;
    saveIconPosition(m_dotDirectory, it->x(), it->y());

    it = it->nextItem();
  }

  m_dotDirectory->sync();
}

#include "kdiconview.moc"
