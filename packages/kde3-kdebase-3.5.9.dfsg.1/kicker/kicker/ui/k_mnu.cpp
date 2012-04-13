/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <dmctl.h>

#include <qimage.h>
#include <qpainter.h>
#include <qstyle.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kaboutkde.h>
#include <kaction.h>
#include <kbookmarkmenu.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kwin.h>

#include "client_mnu.h"
#include "container_base.h"
#include "global.h"
#include "kbutton.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "konqbookmarkmanager.h"
#include "menuinfo.h"
#include "menumanager.h"
#include "popupmenutitle.h"
#include "quickbrowser_mnu.h"
#include "recentapps.h"

#include "k_mnu.h"
#include "k_mnu.moc"

PanelKMenu::PanelKMenu()
  : PanelServiceMenu(QString::null, QString::null, 0, "KMenu")
  , bookmarkMenu(0)
  , bookmarkOwner(0)
{
    static const QCString dcopObjId("KMenu");
    DCOPObject::setObjId(dcopObjId);
    // set the first client id to some arbitrarily large value.
    client_id = 10000;
    // Don't automatically clear the main menu.
    disableAutoClear();
    actionCollection = new KActionCollection(this);
    setCaption(i18n("K Menu"));
    connect(Kicker::the(), SIGNAL(configurationChanged()),
            this, SLOT(configChanged()));
    DCOPClient *dcopClient = KApplication::dcopClient();
    dcopClient->connectDCOPSignal(0, "appLauncher",
        "serviceStartedByStorageId(QString,QString)",
        dcopObjId,
        "slotServiceStartedByStorageId(QString,QString)",
        false);
}

PanelKMenu::~PanelKMenu()
{
    clearSubmenus();
    delete bookmarkMenu;
    delete bookmarkOwner;
}

void PanelKMenu::slotServiceStartedByStorageId(QString starter,
                                               QString storageId)
{
    if (starter != "kmenu")
    {
        kdDebug() << "KMenu - updating recently used applications: " <<
            storageId << endl;
        KService::Ptr service = KService::serviceByStorageId(storageId);
        updateRecentlyUsedApps(service);
    }
}


bool PanelKMenu::loadSidePixmap()
{
    if (!KickerSettings::useSidePixmap())
    {
        return false;
    }

    QString sideName = KickerSettings::sidePixmapName();
    QString sideTileName = KickerSettings::sideTileName();

    QImage image;
    image.load(locate("data", "kicker/pics/" + sideName));

    if (image.isNull())
    {
        kdDebug(1210) << "Can't find a side pixmap" << endl;
        return false;
    }

    KickerLib::colorize(image);
    sidePixmap.convertFromImage(image);

    image.load(locate("data", "kicker/pics/" + sideTileName));

    if (image.isNull())
    {
        kdDebug(1210) << "Can't find a side tile pixmap" << endl;
        return false;
    }

    KickerLib::colorize(image);
    sideTilePixmap.convertFromImage(image);

    if (sidePixmap.width() != sideTilePixmap.width())
    {
        kdDebug(1210) << "Pixmaps have to be the same size" << endl;
        return false;
    }

    // pretile the pixmap to a height of at least 100 pixels
    if (sideTilePixmap.height() < 100)
    {
        int tiles = (int)(100 / sideTilePixmap.height()) + 1;
        QPixmap preTiledPixmap(sideTilePixmap.width(), sideTilePixmap.height() * tiles);
        QPainter p(&preTiledPixmap);
        p.drawTiledPixmap(preTiledPixmap.rect(), sideTilePixmap);
        sideTilePixmap = preTiledPixmap;
    }

    return true;
}

void PanelKMenu::paletteChanged()
{
    if (!loadSidePixmap())
    {
        sidePixmap = sideTilePixmap = QPixmap();
        setMinimumSize( sizeHint() );
    }
}

void PanelKMenu::initialize()
{
//    kdDebug(1210) << "PanelKMenu::initialize()" << endl;
    updateRecent();

    if (initialized())
    {
        return;
    }

    if (loadSidePixmap())
    {
        // in case we've been through here before, let's disconnect
        disconnect(kapp, SIGNAL(kdisplayPaletteChanged()),
                   this, SLOT(paletteChanged()));
        connect(kapp, SIGNAL(kdisplayPaletteChanged()),
                this, SLOT(paletteChanged()));
    }
    else
    {
        sidePixmap = sideTilePixmap = QPixmap();
    }

    // add services
    PanelServiceMenu::initialize();

    if (KickerSettings::showMenuTitles())
    {
        int id;
        id = insertItem(new PopupMenuTitle(i18n("All Applications"), font()), -1 /* id */, 0);
        setItemEnabled( id, false );
        id = insertItem(new PopupMenuTitle(i18n("Actions"), font()), -1 /* id */, -1);
        setItemEnabled( id, false );
    }

    // create recent menu section
    createRecentMenuItems();

    bool need_separator = false;

    // insert bookmarks
    if (KickerSettings::useBookmarks() && kapp->authorizeKAction("bookmarks"))
    {
        // Need to create a new popup each time, it's deleted by subMenus.clear()
        KPopupMenu * bookmarkParent = new KPopupMenu( this, "bookmarks" );
        if(!bookmarkOwner)
            bookmarkOwner = new KBookmarkOwner;
        delete bookmarkMenu; // can't reuse old one, the popup has been deleted
        bookmarkMenu = new KBookmarkMenu( KonqBookmarkManager::self(), bookmarkOwner, bookmarkParent, actionCollection, true, false );

        insertItem(KickerLib::menuIconSet("bookmark"), i18n("Bookmarks"), bookmarkParent);

        subMenus.append(bookmarkParent);
        need_separator = true;
    }

    // insert quickbrowser
    if (KickerSettings::useBrowser())
    {
        PanelQuickBrowser *browserMnu = new PanelQuickBrowser(this);
        browserMnu->initialize();

        insertItem(KickerLib::menuIconSet("kdisknav"),
                   i18n("Quick Browser"),
                   KickerLib::reduceMenu(browserMnu));
        subMenus.append(browserMnu);
        need_separator = true;
    }

    // insert dynamic menus
    QStringList menu_ext = KickerSettings::menuExtensions();
    if (!menu_ext.isEmpty())
    {
        for (QStringList::ConstIterator it=menu_ext.begin(); it!=menu_ext.end(); ++it)
        {
            MenuInfo info(*it);
            if (!info.isValid())
               continue;

            KPanelMenu *menu = info.load();
            if (menu)
            {
                insertItem(KickerLib::menuIconSet(info.icon()), info.name(), menu);
                dynamicSubMenus.append(menu);
                need_separator = true;
            }
        }
    }

    if (need_separator)
        insertSeparator();

    // insert client menus, if any
    if (clients.count() > 0) {
        QIntDictIterator<KickerClientMenu> it(clients);
        while (it){
            if (it.current()->text.at(0) != '.')
                insertItem(
                    it.current()->icon,
                    it.current()->text,
                    it.current(),
                    it.currentKey()
                    );
            ++it;
        }
        insertSeparator();
    }

    // run command
    if (kapp->authorize("run_command"))
    {
        insertItem(KickerLib::menuIconSet("run"),
                   i18n("Run Command..."),
                   this,
                   SLOT( slotRunCommand()));
        insertSeparator();
    }

    if (DM().isSwitchable() && kapp->authorize("switch_user"))
    {
        sessionsMenu = new QPopupMenu( this );
        insertItem(KickerLib::menuIconSet("switchuser"), i18n("Switch User"), sessionsMenu);
        connect( sessionsMenu, SIGNAL(aboutToShow()), SLOT(slotPopulateSessions()) );
        connect( sessionsMenu, SIGNAL(activated(int)), SLOT(slotSessionActivated(int)) );
    }

    /*
      If  the user configured ksmserver to
    */
    KConfig ksmserver("ksmserverrc", false, false);
    ksmserver.setGroup("General");
    if (ksmserver.readEntry( "loginMode" ) == "restoreSavedSession")
    {
        insertItem(KickerLib::menuIconSet("filesave"), i18n("Save Session"), this, SLOT(slotSaveSession()));
    }

    if (kapp->authorize("lock_screen"))
    {
        insertItem(KickerLib::menuIconSet("lock"), i18n("Lock Session"), this, SLOT(slotLock()));
    }

    if (kapp->authorize("logout"))
    {
        insertItem(KickerLib::menuIconSet("exit"), i18n("Log Out..."), this, SLOT(slotLogout()));
    }

#if 0
    // WABA: tear off handles don't work together with dynamically updated
    // menus. We can't update the menu while torn off, and we don't know
    // when it is torn off.
    if (KGlobalSettings::insertTearOffHandle())
      insertTearOffHandle();
#endif

    setInitialized(true);
}

int PanelKMenu::insertClientMenu(KickerClientMenu *p)
{
    int id = client_id;
    clients.insert(id, p);
    slotClear();
    return id;
}

void PanelKMenu::removeClientMenu(int id)
{
    clients.remove(id);
    removeItem(id);
    slotClear();
}

extern int kicker_screen_number;

void PanelKMenu::slotLock()
{
    QCString appname( "kdesktop" );
    if ( kicker_screen_number )
        appname.sprintf("kdesktop-screen-%d", kicker_screen_number);
    kapp->dcopClient()->send(appname, "KScreensaverIface", "lock()", "");
}

void PanelKMenu::slotLogout()
{
    kapp->requestShutDown();
}

void PanelKMenu::slotPopulateSessions()
{
    int p = 0;
    DM dm;

    sessionsMenu->clear();
    if (kapp->authorize("start_new_session") && (p = dm.numReserve()) >= 0)
    {
        if (kapp->authorize("lock_screen"))
	  sessionsMenu->insertItem(/*SmallIconSet("lockfork"),*/ i18n("Lock Current && Start New Session"), 100 );
        sessionsMenu->insertItem(SmallIconSet("fork"), i18n("Start New Session"), 101 );
        if (!p) {
            sessionsMenu->setItemEnabled( 100, false );
            sessionsMenu->setItemEnabled( 101, false );
        }
        sessionsMenu->insertSeparator();
    }
    SessList sess;
    if (dm.localSessions( sess ))
        for (SessList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
            int id = sessionsMenu->insertItem( DM::sess2Str( *it ), (*it).vt );
            if (!(*it).vt)
                sessionsMenu->setItemEnabled( id, false );
            if ((*it).self)
                sessionsMenu->setItemChecked( id, true );
        }
}

void PanelKMenu::slotSessionActivated( int ent )
{
    if (ent == 100)
        doNewSession( true );
    else if (ent == 101)
        doNewSession( false );
    else if (!sessionsMenu->isItemChecked( ent ))
        DM().lockSwitchVT( ent );
}

void PanelKMenu::doNewSession( bool lock )
{
    int result = KMessageBox::warningContinueCancel(
        kapp->desktop()->screen(kapp->desktop()->screenNumber(this)),
        i18n("<p>You have chosen to open another desktop session.<br>"
               "The current session will be hidden "
               "and a new login screen will be displayed.<br>"
               "An F-key is assigned to each session; "
               "F%1 is usually assigned to the first session, "
               "F%2 to the second session and so on. "
               "You can switch between sessions by pressing "
               "Ctrl, Alt and the appropriate F-key at the same time. "
               "Additionally, the KDE Panel and Desktop menus have "
               "actions for switching between sessions.</p>")
                           .arg(7).arg(8),
        i18n("Warning - New Session"),
        KGuiItem(i18n("&Start New Session"), "fork"),
        ":confirmNewSession",
        KMessageBox::PlainCaption | KMessageBox::Notify);

    if (result==KMessageBox::Cancel)
        return;

    if (lock)
        slotLock();

    DM().startReserve();
}

void PanelKMenu::slotSaveSession()
{
    QByteArray data;
    kapp->dcopClient()->send( "ksmserver", "default",
                              "saveCurrentSession()", data );
}

void PanelKMenu::slotRunCommand()
{
    QByteArray data;
    QCString appname( "kdesktop" );
    if ( kicker_screen_number )
        appname.sprintf("kdesktop-screen-%d", kicker_screen_number);

    kapp->updateRemoteUserTimestamp( appname );
    kapp->dcopClient()->send( appname, "KDesktopIface",
                              "popupExecuteCommand()", data );
}

void PanelKMenu::slotEditUserContact()
{
}

void PanelKMenu::setMinimumSize(const QSize & s)
{
    KPanelMenu::setMinimumSize(s.width() + sidePixmap.width(), s.height());
}

void PanelKMenu::setMaximumSize(const QSize & s)
{
    KPanelMenu::setMaximumSize(s.width() + sidePixmap.width(), s.height());
}

void PanelKMenu::setMinimumSize(int w, int h)
{
    KPanelMenu::setMinimumSize(w + sidePixmap.width(), h);
}

void PanelKMenu::setMaximumSize(int w, int h)
{
  KPanelMenu::setMaximumSize(w + sidePixmap.width(), h);
}

void PanelKMenu::showMenu()
{
    kdDebug( 1210 ) << "PanelKMenu::showMenu()" << endl;
    PanelPopupButton *kButton = MenuManager::the()->findKButtonFor(this);
    if (kButton)
    {
        adjustSize();
        kButton->showMenu();
    }
    else
    {
        show();
    }
}

QRect PanelKMenu::sideImageRect()
{
    return QStyle::visualRect( QRect( frameWidth(), frameWidth(), sidePixmap.width(),
                                      height() - 2*frameWidth() ), this );
}

void PanelKMenu::resizeEvent(QResizeEvent * e)
{
//    kdDebug(1210) << "PanelKMenu::resizeEvent():" << endl;
//    kdDebug(1210) << geometry().width() << ", " << geometry().height() << endl;

    PanelServiceMenu::resizeEvent(e);

    setFrameRect( QStyle::visualRect( QRect( sidePixmap.width(), 0,
                                      width() - sidePixmap.width(), height() ), this ) );
}

//Workaround Qt3.3.x sizing bug, by ensuring we're always wide enough.
void PanelKMenu::resize(int width, int height)
{
    width = kMax(width, maximumSize().width());
    PanelServiceMenu::resize(width, height);
}

QSize PanelKMenu::sizeHint() const
{
    QSize s = PanelServiceMenu::sizeHint();
//    kdDebug(1210) << "PanelKMenu::sizeHint()" << endl;
//    kdDebug(1210) << s.width() << ", " << s.height() << endl;
    return s;
}

void PanelKMenu::paintEvent(QPaintEvent * e)
{
    if (sidePixmap.isNull()) {
        PanelServiceMenu::paintEvent(e);
        return;
    }

    QPainter p(this);
    p.setClipRegion(e->region());

    style().drawPrimitive( QStyle::PE_PanelPopup, &p,
                           QRect( 0, 0, width(), height() ),
                           colorGroup(), QStyle::Style_Default,
                           QStyleOption( frameWidth(), 0 ) );

    QRect r = sideImageRect();
    r.setBottom( r.bottom() - sidePixmap.height() );
    if ( r.intersects( e->rect() ) )
    {
        p.drawTiledPixmap( r, sideTilePixmap );
    }

    r = sideImageRect();
    r.setTop( r.bottom() - sidePixmap.height() );
    if ( r.intersects( e->rect() ) )
    {
        QRect drawRect = r.intersect( e->rect() );
        QRect pixRect = drawRect;
        pixRect.moveBy( -r.left(), -r.top() );
        p.drawPixmap( drawRect.topLeft(), sidePixmap, pixRect );
    }

    drawContents( &p );
}

QMouseEvent PanelKMenu::translateMouseEvent( QMouseEvent* e )
{
    QRect side = sideImageRect();

    if ( !side.contains( e->pos() ) )
        return *e;

    QPoint newpos( e->pos() );
    QApplication::reverseLayout() ?
        newpos.setX( newpos.x() - side.width() ) :
        newpos.setX( newpos.x() + side.width() );
    QPoint newglobal( e->globalPos() );
    QApplication::reverseLayout() ?
        newglobal.setX( newpos.x() - side.width() ) :
        newglobal.setX( newpos.x() + side.width() );

    return QMouseEvent( e->type(), newpos, newglobal, e->button(), e->state() );
}

void PanelKMenu::mousePressEvent(QMouseEvent * e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    PanelServiceMenu::mousePressEvent( &newEvent );
}

void PanelKMenu::mouseReleaseEvent(QMouseEvent *e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    PanelServiceMenu::mouseReleaseEvent( &newEvent );
}

void PanelKMenu::mouseMoveEvent(QMouseEvent *e)
{
    QMouseEvent newEvent = translateMouseEvent(e);
    PanelServiceMenu::mouseMoveEvent( &newEvent );
}

void PanelKMenu::configChanged()
{
    RecentlyLaunchedApps::the().m_bNeedToUpdate = false;
    RecentlyLaunchedApps::the().configChanged();
    PanelServiceMenu::configChanged();
}

// create and fill "recent" section at first
void PanelKMenu::createRecentMenuItems()
{
    RecentlyLaunchedApps::the().m_nNumMenuItems = 0;

    QStringList RecentApps;
    RecentlyLaunchedApps::the().getRecentApps(RecentApps);

    if (RecentApps.count() > 0)
    {
        bool bSeparator = KickerSettings::showMenuTitles();
        int nId = serviceMenuEndId() + 1;
        int nIndex = KickerSettings::showMenuTitles() ? 1 : 0;

        for (QValueList<QString>::ConstIterator it =
             RecentApps.fromLast(); /*nop*/; --it)
        {
            KService::Ptr s = KService::serviceByDesktopPath(*it);
            if (!s)
            {
                RecentlyLaunchedApps::the().removeItem(*it);
            }
            else
            {
                if (bSeparator)
                {
                    bSeparator = false;
                    int id = insertItem(
                        new PopupMenuTitle(
                            RecentlyLaunchedApps::the().caption(), font()),
                        serviceMenuEndId(), 0);
                    setItemEnabled( id, false );
                }
                insertMenuItem(s, nId++, nIndex);
                RecentlyLaunchedApps::the().m_nNumMenuItems++;
            }

            if (it == RecentApps.begin())
            {
                break;
            }
        }

        if (!KickerSettings::showMenuTitles())
        {
            insertSeparator(RecentlyLaunchedApps::the().m_nNumMenuItems);
        }
    }
}

void PanelKMenu::clearSubmenus()
{
    // we don't need to delete these on the way out since the libloader
    // handles them for us
    if (QApplication::closingDown())
    {
        return;
    }

    for (PopupMenuList::const_iterator it = dynamicSubMenus.constBegin();
            it != dynamicSubMenus.constEnd();
            ++it)
    {
        delete *it;
    }
    dynamicSubMenus.clear();

    PanelServiceMenu::clearSubmenus();
}

void PanelKMenu::updateRecent()
{
    if (!RecentlyLaunchedApps::the().m_bNeedToUpdate)
    {
        return;
    }

    RecentlyLaunchedApps::the().m_bNeedToUpdate = false;

    int nId = serviceMenuEndId() + 1;

    // remove previous items
    if (RecentlyLaunchedApps::the().m_nNumMenuItems > 0)
    {
        // -1 --> menu title
        int i = KickerSettings::showMenuTitles() ? -1 : 0;
        for (; i < RecentlyLaunchedApps::the().m_nNumMenuItems; i++)
        {
            removeItem(nId + i);
            entryMap_.remove(nId + i);
        }
        RecentlyLaunchedApps::the().m_nNumMenuItems = 0;

        if (!KickerSettings::showMenuTitles())
        {
            removeItemAt(0);
        }
    }

    // insert new items
    QStringList RecentApps;
    RecentlyLaunchedApps::the().getRecentApps(RecentApps);

    if (RecentApps.count() > 0)
    {
        bool bNeedSeparator = KickerSettings::showMenuTitles();
        for (QValueList<QString>::ConstIterator it = RecentApps.fromLast();
             /*nop*/; --it)
        {
            KService::Ptr s = KService::serviceByDesktopPath(*it);
            if (!s)
            {
                RecentlyLaunchedApps::the().removeItem(*it);
            }
            else
            {
                if (bNeedSeparator)
                {
                    bNeedSeparator = false;
                    int id = insertItem(new PopupMenuTitle(
                        RecentlyLaunchedApps::the().caption(),
                            font()), nId - 1, 0);
                    setItemEnabled( id, false );
                }
                insertMenuItem(s, nId++, KickerSettings::showMenuTitles() ?
                    1 : 0);
                RecentlyLaunchedApps::the().m_nNumMenuItems++;
            }

            if (it == RecentApps.begin())
                break;
        }

        if (!KickerSettings::showMenuTitles())
        {
            insertSeparator(RecentlyLaunchedApps::the().m_nNumMenuItems);
        }
    }
}

void PanelKMenu::clearRecentMenuItems()
{
    RecentlyLaunchedApps::the().clearRecentApps();
    RecentlyLaunchedApps::the().save();
    RecentlyLaunchedApps::the().m_bNeedToUpdate = true;
    updateRecent();
}


