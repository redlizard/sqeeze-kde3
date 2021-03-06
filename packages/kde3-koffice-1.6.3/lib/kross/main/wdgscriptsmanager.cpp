/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "wdgscriptsmanager.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qobjectlist.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <ktoolbar.h>

#if KDE_IS_VERSION(3, 4, 0)
  // The KNewStuffSecure we use internaly for the GetHotNewStuff-functionality
  // was introduced with KDE 3.4.
  #define KROSS_SUPPORT_NEWSTUFF
#endif

#ifdef KROSS_SUPPORT_NEWSTUFF
  #include <knewstuff/provider.h>
  #include <knewstuff/engine.h>
  #include <knewstuff/downloaddialog.h>
  #include <knewstuff/knewstuffsecure.h>
#endif

#include "scriptguiclient.h"
#include "scriptaction.h"

namespace Kross { namespace Api {

#ifdef KROSS_SUPPORT_NEWSTUFF
class ScriptNewStuff : public KNewStuffSecure
{
    public:
        ScriptNewStuff(ScriptGUIClient* scripguiclient, const QString& type, QWidget *parentWidget = 0)
            : KNewStuffSecure(type, parentWidget)
            , m_scripguiclient(scripguiclient) {}
        virtual ~ScriptNewStuff() {}
    private:
        ScriptGUIClient* m_scripguiclient;
        virtual void installResource() { m_scripguiclient->installScriptPackage( m_tarName ); }
};
#endif

class ListItem : public QListViewItem
{
    private:
        ScriptActionCollection* m_collection;
        ScriptAction::Ptr m_action;
    public:
        ListItem(QListView* parentview, ScriptActionCollection* collection)
            : QListViewItem(parentview), m_collection(collection), m_action(0) {}

        ListItem(ListItem* parentitem, QListViewItem* afteritem, ScriptAction::Ptr action)
            : QListViewItem(parentitem, afteritem), m_collection( parentitem->collection() ), m_action(action) {}

        ScriptAction::Ptr action() const { return m_action; }
        ScriptActionCollection* collection() const { return m_collection; }
        //ScriptActionMenu* actionMenu() const { return m_menu; }
};

class ToolTip : public QToolTip
{
    public:
        ToolTip(KListView* parent) : QToolTip(parent->viewport()), m_parent(parent) {}
        virtual ~ToolTip () { remove(m_parent->viewport()); }
    protected:
        virtual void maybeTip(const QPoint& p) {
            ListItem* item = dynamic_cast<ListItem*>( m_parent->itemAt(p) );
            if(item) {
                QRect r( m_parent->itemRect(item) );
                if(r.isValid() && item->action()) {
                    tip(r, QString("<qt>%1</qt>").arg(item->action()->toolTip()));
                }
            }
        }
    private:
        KListView* m_parent;
};

class WdgScriptsManagerPrivate
{
    friend class WdgScriptsManager;
    ScriptGUIClient* m_scripguiclient;
    ToolTip* m_tooltip;
#ifdef KROSS_SUPPORT_NEWSTUFF
    ScriptNewStuff* newstuff;
#endif
    //enum { LoadBtn = 0, UnloadBtn, InstallBtn, UninstallBtn, ExecBtn, NewStuffBtn };
};

WdgScriptsManager::WdgScriptsManager(ScriptGUIClient* scr, QWidget* parent, const char* name, WFlags fl )
    : WdgScriptsManagerBase(parent, name, fl)
    , d(new WdgScriptsManagerPrivate)
{
    d->m_scripguiclient = scr;
    d->m_tooltip = new ToolTip(scriptsList);
#ifdef KROSS_SUPPORT_NEWSTUFF
    d->newstuff = 0;
#endif

    scriptsList->header()->hide();
    //scriptsList->header()->setClickEnabled(false);
    scriptsList->setAllColumnsShowFocus(true);
    //scriptsList->setRootIsDecorated(true);
    scriptsList->setSorting(-1);
    scriptsList->addColumn("text");
    //scriptsList->setColumnWidthMode(1, QListView::Manual);

    slotFillScriptsList();

    slotSelectionChanged(0);
    connect(scriptsList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)));

    btnExec->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "exec", KIcon::MainToolbar, 16 ));
    connect(btnExec, SIGNAL(clicked()), this, SLOT(slotExecuteScript()));
    btnLoad->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "fileopen", KIcon::MainToolbar, 16 ));
    connect(btnLoad, SIGNAL(clicked()), this, SLOT(slotLoadScript()));
    btnUnload->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "fileclose", KIcon::MainToolbar, 16 ));
    connect(btnUnload, SIGNAL(clicked()), this, SLOT(slotUnloadScript()));
    btnInstall->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "fileimport", KIcon::MainToolbar, 16 ));
    connect(btnInstall, SIGNAL(clicked()), this, SLOT(slotInstallScript()));
    btnUninstall->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "fileclose", KIcon::MainToolbar, 16 ));
    connect(btnUninstall, SIGNAL(clicked()), this, SLOT(slotUninstallScript()));
#ifdef KROSS_SUPPORT_NEWSTUFF
    btnNewStuff->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "knewstuff", KIcon::MainToolbar, 16 ));
    connect(btnNewStuff, SIGNAL(clicked()), this, SLOT(slotGetNewScript()));
#endif
/*
    toolBar->setIconText( KToolBar::IconTextRight );

    toolBar->insertButton("exec", WdgScriptsManagerPrivate::ExecBtn, false, i18n("Execute"));
    toolBar->addConnection(WdgScriptsManagerPrivate::ExecBtn, SIGNAL(clicked()), this, SLOT(slotExecuteScript()));
    toolBar->insertLineSeparator();
    toolBar->insertButton("fileopen", WdgScriptsManagerPrivate::LoadBtn, true, i18n("Load"));
    toolBar->addConnection(WdgScriptsManagerPrivate::LoadBtn, SIGNAL(clicked()), this, SLOT(slotLoadScript()));
    toolBar->insertButton("fileclose", WdgScriptsManagerPrivate::UnloadBtn, false, i18n("Unload"));
    toolBar->addConnection(WdgScriptsManagerPrivate::UnloadBtn, SIGNAL(clicked()), this, SLOT(slotUnloadScript()));
    toolBar->insertLineSeparator();
    toolBar->insertButton("fileimport", WdgScriptsManagerPrivate::InstallBtn, true, i18n("Install"));
    toolBar->addConnection(WdgScriptsManagerPrivate::InstallBtn, SIGNAL(clicked()), this, SLOT(slotInstallScript()));
    toolBar->insertButton("fileclose", WdgScriptsManagerPrivate::UninstallBtn, false, i18n("Uninstall"));
    toolBar->addConnection(WdgScriptsManagerPrivate::UninstallBtn, SIGNAL(clicked()), this, SLOT(slotUninstallScript()));
#ifdef KROSS_SUPPORT_NEWSTUFF
    toolBar->insertLineSeparator();
    toolBar->insertButton("knewstuff", WdgScriptsManagerPrivate::NewStuffBtn, true, i18n("Get More Scripts"));
    toolBar->addConnection(WdgScriptsManagerPrivate::NewStuffBtn, SIGNAL(clicked()), this, SLOT(slotGetNewScript()));
#endif
*/
    connect(scr, SIGNAL( collectionChanged(ScriptActionCollection*) ),
            this, SLOT( slotFillScriptsList() ));
}

WdgScriptsManager::~WdgScriptsManager()
{
    delete d->m_tooltip;
    delete d;
}

void WdgScriptsManager::slotFillScriptsList()
{
    scriptsList->clear();

    addItem( d->m_scripguiclient->getActionCollection("executedscripts") );
    addItem( d->m_scripguiclient->getActionCollection("loadedscripts") );
    addItem( d->m_scripguiclient->getActionCollection("installedscripts") );
}

void WdgScriptsManager::addItem(ScriptActionCollection* collection)
{
    if(! collection)
        return;

    ListItem* i = new ListItem(scriptsList, collection);
    i->setText(0, collection->actionMenu()->text());
    i->setOpen(true);

    QValueList<ScriptAction::Ptr> list = collection->actions();
    QListViewItem* lastitem = 0;
    for(QValueList<ScriptAction::Ptr>::Iterator it = list.begin(); it != list.end(); ++it)
        lastitem = addItem(*it, i, lastitem);
}

QListViewItem* WdgScriptsManager::addItem(ScriptAction::Ptr action, QListViewItem* parentitem, QListViewItem* afteritem)
{
    if(! action)
        return 0;

    ListItem* i = new ListItem(dynamic_cast<ListItem*>(parentitem), afteritem, action);
    i->setText(0, action->text()); // FIXME: i18nise it for ko2.0
    //i->setText(1, action->getDescription()); // FIXME: i18nise it for ko2.0
    //i->setText(2, action->name());

    QPixmap pm;
    if(action->hasIcon()) {
        KIconLoader* icons = KGlobal::iconLoader();
        pm = icons->loadIconSet(action->icon(), KIcon::Small).pixmap(QIconSet::Small, QIconSet::Active);
    }
    else {
        pm = action->iconSet(KIcon::Small, 16).pixmap(QIconSet::Small, QIconSet::Active);
    }
    if(! pm.isNull())
        i->setPixmap(0, pm); // display the icon

    return i;
}

void WdgScriptsManager::slotSelectionChanged(QListViewItem* item)
{
    ListItem* i = dynamic_cast<ListItem*>(item);
    Kross::Api::ScriptActionCollection* installedcollection = d->m_scripguiclient->getActionCollection("installedscripts");

    //toolBar->setItemEnabled(WdgScriptsManagerPrivate::ExecBtn, i && i->action());
    //toolBar->setItemEnabled(WdgScriptsManagerPrivate::UninstallBtn, i && i->action() && i->collection() == installedcollection);
    //toolBar->setItemEnabled(WdgScriptsManagerPrivate::UnloadBtn, i && i->action() && i->collection() != installedcollection);
    btnExec->setEnabled(i && i->action());
    btnUnload->setEnabled(i && i->action() && i->collection() != installedcollection);
    btnUninstall->setEnabled(i && i->action() && i->collection() == installedcollection);
}

void WdgScriptsManager::slotLoadScript()
{
    if(d->m_scripguiclient->loadScriptFile())
        slotFillScriptsList();
}

void WdgScriptsManager::slotInstallScript()
{
    KFileDialog* filedialog = new KFileDialog(
        QString::null, // startdir
        "*.tar.gz *.tgz *.bz2", // filter
        this, // parent widget
        "WdgScriptsManagerInstallFileDialog", // name
        true // modal
    );
    filedialog->setCaption( i18n("Install Script Package") );

    if(! filedialog->exec())
        return;

    if(! d->m_scripguiclient->installScriptPackage( filedialog->selectedURL().path() )) {
        krosswarning("Failed to install scriptpackage");
        return;
    }

    slotFillScriptsList();
}

void WdgScriptsManager::slotUninstallScript()
{
    ListItem* item = dynamic_cast<ListItem*>( scriptsList->currentItem() );
    if( !item || !item->action() )
        return;

    Kross::Api::ScriptActionCollection* installedcollection = d->m_scripguiclient->getActionCollection("installedscripts");
    if( !item->collection() || item->collection() != installedcollection)
        return;

    const QString packagepath = item->action()->getPackagePath();
    if( !packagepath)
        return;

    if( KMessageBox::warningContinueCancel(0,
        i18n("Uninstall the script package \"%1\" and delete the package's folder \"%2\"?")
            .arg(item->action()->text()).arg(packagepath),
        i18n("Uninstall")) != KMessageBox::Continue )
    {
        return;
    }

    if(! d->m_scripguiclient->uninstallScriptPackage(packagepath)) {
        krosswarning("Failed to uninstall scriptpackage");
        return;
    }

    slotFillScriptsList();
}

void WdgScriptsManager::slotExecuteScript()
{
    ListItem* item = dynamic_cast<ListItem*>( scriptsList->currentItem() );
    if(item && item->action())
        item->action()->activate();
}

void WdgScriptsManager::slotUnloadScript()
{
    ListItem* item = dynamic_cast<ListItem*>( scriptsList->currentItem() );
    if(item && item->action()) {
        item->collection()->detach( item->action() );
        slotFillScriptsList();
    }
}

void WdgScriptsManager::slotGetNewScript()
{
#ifdef KROSS_SUPPORT_NEWSTUFF
    const QString appname = KApplication::kApplication()->name();
    const QString type = QString("%1/script").arg(appname);

    if(! d->newstuff) {
        d->newstuff = new ScriptNewStuff(d->m_scripguiclient, type);
        connect(d->newstuff, SIGNAL(installFinished()), this, SLOT(slotResourceInstalled()));
    }

    KNS::Engine *engine = new KNS::Engine(d->newstuff, type, this);
    KNS::DownloadDialog *d = new KNS::DownloadDialog( engine, this );
    d->setType(type);

    KNS::ProviderLoader *p = new KNS::ProviderLoader(this);
    QObject::connect(p, SIGNAL(providersLoaded(Provider::List*)),
                     d, SLOT(slotProviders(Provider::List*)));

    p->load(type, QString("http://download.kde.org/khotnewstuff/%1scripts-providers.xml").arg(appname));
    d->exec();
#endif
}

void WdgScriptsManager::slotResourceInstalled()
{
    // Delete KNewStuff's configuration entries. These entries reflect what has
    // already been installed. As we cannot yet keep them in sync after uninstalling
    // scripts, we deactivate the check marks entirely.
    KGlobal::config()->deleteGroup("KNewStuffStatus");
}

}}

#include "wdgscriptsmanager.moc"
