/* This file is part of the KDE libraries
   Copyright (C) 2005 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Joseph Wenninger <jowenn@kde.org>

   GUIClient partly based on ktoolbarhandler.cpp: Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "katemdi.h"
#include "katemdi.moc"

#include <kaction.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include <qvbox.h>
#include <qhbox.h>
#include <qevent.h>

namespace KateMDI {

//BEGIN SPLITTER

Splitter::Splitter(Orientation o, QWidget* parent, const char* name)
  : QSplitter(o, parent, name)
{
}

Splitter::~Splitter()
{
}

bool Splitter::isLastChild(QWidget* w) const
{
  return ( idAfter( w ) == 0 );
}

int Splitter::idAfter ( QWidget * w ) const
{
  return QSplitter::idAfter (w);
}

//END SPLITTER


//BEGIN TOGGLETOOLVIEWACTION

ToggleToolViewAction::ToggleToolViewAction ( const QString& text, const KShortcut& cut, ToolView *tv,
                                             QObject* parent, const char* name )
 : KToggleAction(text,cut,parent,name)
 , m_tv(tv)
{
  connect(this,SIGNAL(toggled(bool)),this,SLOT(slotToggled(bool)));
  connect(m_tv,SIGNAL(visibleChanged(bool)),this,SLOT(visibleChanged(bool)));

  setChecked(m_tv->visible());
}

ToggleToolViewAction::~ToggleToolViewAction()
{
  unplugAll();
}

void ToggleToolViewAction::visibleChanged(bool)
{
  if (isChecked() != m_tv->visible())
    setChecked (m_tv->visible());
}

void ToggleToolViewAction::slotToggled(bool t)
{
  if (t)
  {
    m_tv->mainWindow()->showToolView (m_tv);
    m_tv->setFocus ();
  }
  else
  {
    m_tv->mainWindow()->hideToolView (m_tv);
    m_tv->mainWindow()->centralWidget()->setFocus ();
  }
}

//END TOGGLETOOLVIEWACTION


//BEGIN GUICLIENT

static const char *actionListName = "kate_mdi_window_actions";

static const char *guiDescription = ""
        "<!DOCTYPE kpartgui><kpartgui name=\"kate_mdi_window_actions\">"
        "<MenuBar>"
        "    <Menu name=\"window\">"
        "        <ActionList name=\"%1\" />"
        "    </Menu>"
        "</MenuBar>"
        "</kpartgui>";

GUIClient::GUIClient ( MainWindow *mw )
 : QObject ( mw )
 , KXMLGUIClient ( mw )
 , m_mw (mw)
{
  connect( m_mw->guiFactory(), SIGNAL( clientAdded( KXMLGUIClient * ) ),
           this, SLOT( clientAdded( KXMLGUIClient * ) ) );

  if ( domDocument().documentElement().isNull() )
  {
    QString completeDescription = QString::fromLatin1( guiDescription )
          .arg( actionListName );

    setXML( completeDescription, false /*merge*/ );
  }

  if (actionCollection()->kaccel()==0)
    actionCollection()->setWidget(m_mw);

  m_toolMenu = new KActionMenu(i18n("Tool &Views"),actionCollection(),"kate_mdi_toolview_menu");
  m_showSidebarsAction = new KToggleAction( i18n("Show Side&bars"),
                                            CTRL|ALT|SHIFT|Key_F, actionCollection(), "kate_mdi_sidebar_visibility" );
  m_showSidebarsAction->setCheckedState(i18n("Hide Side&bars"));
  m_showSidebarsAction->setChecked( m_mw->sidebarsVisible() );
  connect( m_showSidebarsAction, SIGNAL( toggled( bool ) ),
           m_mw, SLOT( setSidebarsVisible( bool ) ) );

  m_toolMenu->insert( m_showSidebarsAction );
  m_toolMenu->insert( new KActionSeparator( m_toolMenu ) );

  // read shortcuts
  actionCollection()->readShortcutSettings( "Shortcuts", kapp->config() );
}

GUIClient::~GUIClient()
{
}

void GUIClient::updateSidebarsVisibleAction()
{
  m_showSidebarsAction->setChecked( m_mw->sidebarsVisible() );
}

void GUIClient::registerToolView (ToolView *tv)
{
  QString aname = QString("kate_mdi_toolview_") + tv->id;

  // try to read the action shortcut
  KShortcut sc;
  KConfig *cfg = kapp->config();
  QString _grp = cfg->group();
  cfg->setGroup("Shortcuts");
  sc = KShortcut( cfg->readEntry( aname, "" ) );
  cfg->setGroup( _grp );

  KToggleAction *a = new ToggleToolViewAction(i18n("Show %1").arg(tv->text),
    sc,tv, actionCollection(), aname.latin1() );

  a->setCheckedState(i18n("Hide %1").arg(tv->text));

  m_toolViewActions.append(a);
  m_toolMenu->insert(a);

  m_toolToAction.insert (tv, a);

  updateActions();
}

void GUIClient::unregisterToolView (ToolView *tv)
{
  KAction *a = m_toolToAction[tv];

  if (!a)
    return;

  m_toolViewActions.remove(a);
  delete a;

  m_toolToAction.remove (tv);

  updateActions();
}

void GUIClient::clientAdded( KXMLGUIClient *client )
{
  if ( client == this )
    updateActions();
}

void GUIClient::updateActions()
{
  if ( !factory() )
    return;

  unplugActionList( actionListName );

  QPtrList<KAction> addList;
  addList.append(m_toolMenu);

  plugActionList( actionListName, addList );
}

//END GUICLIENT


//BEGIN TOOLVIEW

ToolView::ToolView (MainWindow *mainwin, Sidebar *sidebar, QWidget *parent)
 : QVBox (parent)
 , m_mainWin (mainwin)
 , m_sidebar (sidebar)
 , m_visible (false)
 , persistent (false)
{
}

ToolView::~ToolView ()
{
  m_mainWin->toolViewDeleted (this);
}

void ToolView::setVisible (bool vis)
{
  if (m_visible == vis)
    return;

  m_visible = vis;
  emit visibleChanged (m_visible);
}

bool ToolView::visible () const
{
  return m_visible;
}

void ToolView::childEvent ( QChildEvent *ev )
{
  // set the widget to be focus proxy if possible
  if (ev->inserted() && ev->child() && ev->child()->qt_cast("QWidget"))
    setFocusProxy ((QWidget *)(ev->child()->qt_cast("QWidget")));

  QVBox::childEvent (ev);
}

//END TOOLVIEW


//BEGIN SIDEBAR

Sidebar::Sidebar (KMultiTabBar::KMultiTabBarPosition pos, MainWindow *mainwin, QWidget *parent)
  : KMultiTabBar ((pos == KMultiTabBar::Top || pos == KMultiTabBar::Bottom) ? KMultiTabBar::Horizontal : KMultiTabBar::Vertical, parent)
  , m_mainWin (mainwin)
  , m_splitter (0)
  , m_ownSplit (0)
  , m_lastSize (0)
{
  setPosition( pos );
  hide ();
}

Sidebar::~Sidebar ()
{
}

void Sidebar::setSplitter (Splitter *sp)
{
  m_splitter = sp;
  m_ownSplit = new Splitter ((position() == KMultiTabBar::Top || position() == KMultiTabBar::Bottom) ? Qt::Horizontal : Qt::Vertical, m_splitter);
  m_ownSplit->setOpaqueResize( KGlobalSettings::opaqueResize() );
  m_ownSplit->setChildrenCollapsible( false );
  m_splitter->setResizeMode( m_ownSplit, QSplitter::KeepSize );
  m_ownSplit->hide ();
}

ToolView *Sidebar::addWidget (const QPixmap &icon, const QString &text, ToolView *widget)
{
  static int id = 0;

  if (widget)
  {
    if (widget->sidebar() == this)
      return widget;

    widget->sidebar()->removeWidget (widget);
  }

  int newId = ++id;

  appendTab (icon, newId, text);

  if (!widget)
  {
    widget = new ToolView (m_mainWin, this, m_ownSplit);
    widget->hide ();
    widget->icon = icon;
    widget->text = text;
  }
  else
  {
    widget->hide ();
    widget->reparent (m_ownSplit, 0, QPoint());
    widget->m_sidebar = this;
  }

  // save it's pos ;)
  widget->persistent = false;

  m_idToWidget.insert (newId, widget);
  m_widgetToId.insert (widget, newId);
  m_toolviews.push_back (widget);

  show ();

  connect(tab(newId),SIGNAL(clicked(int)),this,SLOT(tabClicked(int)));
  tab(newId)->installEventFilter(this);

  return widget;
}

bool Sidebar::removeWidget (ToolView *widget)
{
  if (!m_widgetToId.contains(widget))
    return false;

  removeTab(m_widgetToId[widget]);

  m_idToWidget.remove (m_widgetToId[widget]);
  m_widgetToId.remove (widget);
  m_toolviews.remove (widget);

  bool anyVis = false;
  QIntDictIterator<ToolView> it( m_idToWidget );
  for ( ; it.current(); ++it )
  {
    if (!anyVis)
      anyVis =  it.current()->isVisible();
  }

  if (m_idToWidget.isEmpty())
  {
    m_ownSplit->hide ();
    hide ();
  }
  else if (!anyVis)
    m_ownSplit->hide ();

  return true;
}

bool Sidebar::showWidget (ToolView *widget)
{
  if (!m_widgetToId.contains(widget))
    return false;

  // hide other non-persistent views
  QIntDictIterator<ToolView> it( m_idToWidget );
  for ( ; it.current(); ++it )
    if ((it.current() != widget) && !it.current()->persistent)
    {
      it.current()->hide();
      setTab (it.currentKey(), false);
      it.current()->setVisible(false);
    }

  setTab (m_widgetToId[widget], true);

  m_ownSplit->show ();
  widget->show ();

  widget->setVisible (true);

  return true;
}

bool Sidebar::hideWidget (ToolView *widget)
{
  if (!m_widgetToId.contains(widget))
    return false;

  bool anyVis = false;

   updateLastSize ();

  for ( QIntDictIterator<ToolView> it( m_idToWidget ); it.current(); ++it )
  {
    if (it.current() == widget)
    {
      it.current()->hide();
      continue;
    }

    if (!anyVis)
      anyVis =  it.current()->isVisible();
  }

  // lower tab
  setTab (m_widgetToId[widget], false);

  if (!anyVis)
    m_ownSplit->hide ();

  widget->setVisible (false);

  return true;
}

void Sidebar::tabClicked(int i)
{
  ToolView *w = m_idToWidget[i];

  if (!w)
    return;

  if (isTabRaised(i))
  {
    showWidget (w);
    w->setFocus ();
  }
  else
  {
    hideWidget (w);
    m_mainWin->centralWidget()->setFocus ();
  }
}

bool Sidebar::eventFilter(QObject *obj, QEvent *ev)
{
  if (ev->type()==QEvent::ContextMenu)
  {
    QContextMenuEvent *e = (QContextMenuEvent *) ev;
    KMultiTabBarTab *bt = dynamic_cast<KMultiTabBarTab*>(obj);
    if (bt)
    {
      kdDebug()<<"Request for popup"<<endl;

      m_popupButton = bt->id();

      ToolView *w = m_idToWidget[m_popupButton];

      if (w)
      {
        KPopupMenu *p = new KPopupMenu (this);

        p->insertTitle(SmallIcon("view_remove"), i18n("Behavior"), 50);

        p->insertItem(w->persistent ? SmallIconSet("window_nofullscreen") : SmallIconSet("window_fullscreen"), w->persistent ? i18n("Make Non-Persistent") : i18n("Make Persistent"), 10);

        p->insertTitle(SmallIcon("move"), i18n("Move To"), 51);

        if (position() != 0)
          p->insertItem(SmallIconSet("back"), i18n("Left Sidebar"),0);

        if (position() != 1)
          p->insertItem(SmallIconSet("forward"), i18n("Right Sidebar"),1);

        if (position() != 2)
          p->insertItem(SmallIconSet("up"), i18n("Top Sidebar"),2);

        if (position() != 3)
          p->insertItem(SmallIconSet("down"), i18n("Bottom Sidebar"),3);

        connect(p, SIGNAL(activated(int)),
              this, SLOT(buttonPopupActivate(int)));

        p->exec(e->globalPos());
        delete p;

        return true;
      }
    }
  }

  return false;
}

void Sidebar::show()
{
  if (m_idToWidget.isEmpty() || !m_mainWin->sidebarsVisible() )
    return;

  KMultiTabBar::show(  );
}

void Sidebar::buttonPopupActivate (int id)
{
  ToolView *w = m_idToWidget[m_popupButton];

  if (!w)
    return;

  // move ids
  if (id < 4)
  {
    // move + show ;)
    m_mainWin->moveToolView (w, (KMultiTabBar::KMultiTabBarPosition) id);
    m_mainWin->showToolView (w);
  }

  // toggle persistent
  if (id == 10)
    w->persistent = !w->persistent;
}

void Sidebar::updateLastSize ()
{
   QValueList<int> s = m_splitter->sizes ();

  int i = 0;
  if ((position() == KMultiTabBar::Right || position() == KMultiTabBar::Bottom))
    i = 2;

  // little threshold
  if (s[i] > 2)
    m_lastSize = s[i];
}

class TmpToolViewSorter
{
  public:
    ToolView *tv;
    unsigned int pos;
};

void Sidebar::restoreSession (KConfig *config)
{
  // get the last correct placed toolview
  unsigned int firstWrong = 0;
  for ( ; firstWrong < m_toolviews.size(); ++firstWrong )
  {
    ToolView *tv = m_toolviews[firstWrong];

    unsigned int pos = config->readUnsignedNumEntry (QString ("Kate-MDI-ToolView-%1-Sidebar-Position").arg(tv->id), firstWrong);

    if (pos != firstWrong)
      break;
  }

  // we need to reshuffle, ahhh :(
  if (firstWrong < m_toolviews.size())
  {
    // first: collect the items to reshuffle
    QValueList<TmpToolViewSorter> toSort;
    for (unsigned int i=firstWrong; i < m_toolviews.size(); ++i)
    {
      TmpToolViewSorter s;
      s.tv = m_toolviews[i];
      s.pos = config->readUnsignedNumEntry (QString ("Kate-MDI-ToolView-%1-Sidebar-Position").arg(m_toolviews[i]->id), i);
      toSort.push_back (s);
    }

    // now: sort the stuff we need to reshuffle
    for (unsigned int m=0; m < toSort.size(); ++m)
      for (unsigned int n=m+1; n < toSort.size(); ++n)
        if (toSort[n].pos < toSort[m].pos)
        {
          TmpToolViewSorter tmp = toSort[n];
          toSort[n] = toSort[m];
          toSort[m] = tmp;
        }

    // then: remove this items from the button bar
    // do this backwards, to minimize the relayout efforts
    for (int i=m_toolviews.size()-1; i >= (int)firstWrong; --i)
    {
      removeTab (m_widgetToId[m_toolviews[i]]);
    }

    // insert the reshuffled things in order :)
    for (unsigned int i=0; i < toSort.size(); ++i)
    {
      ToolView *tv = toSort[i].tv;

      m_toolviews[firstWrong+i] = tv;

      // readd the button
      int newId = m_widgetToId[tv];
      appendTab (tv->icon, newId, tv->text);
      connect(tab(newId),SIGNAL(clicked(int)),this,SLOT(tabClicked(int)));
      tab(newId)->installEventFilter(this);

      // reshuffle in splitter
      m_ownSplit->moveToLast (tv);
    }
  }

  // update last size if needed
  updateLastSize ();

  // restore the own splitter sizes
  QValueList<int> s = config->readIntListEntry (QString ("Kate-MDI-Sidebar-%1-Splitter").arg(position()));
  m_ownSplit->setSizes (s);

  // show only correct toolviews, remember persistent values ;)
  bool anyVis = false;
  for ( unsigned int i=0; i < m_toolviews.size(); ++i )
  {
    ToolView *tv = m_toolviews[i];

    tv->persistent = config->readBoolEntry (QString ("Kate-MDI-ToolView-%1-Persistent").arg(tv->id), false);
    tv->setVisible (config->readBoolEntry (QString ("Kate-MDI-ToolView-%1-Visible").arg(tv->id), false));

    if (!anyVis)
      anyVis = tv->visible();

    setTab (m_widgetToId[tv],tv->visible());

    if (tv->visible())
      tv->show();
    else
      tv->hide ();
  }

  if (anyVis)
    m_ownSplit->show();
  else
    m_ownSplit->hide();
}

void Sidebar::saveSession (KConfig *config)
{
  // store the own splitter sizes
  QValueList<int> s = m_ownSplit->sizes();
  config->writeEntry (QString ("Kate-MDI-Sidebar-%1-Splitter").arg(position()), s);

  // store the data about all toolviews in this sidebar ;)
  for ( unsigned int i=0; i < m_toolviews.size(); ++i )
  {
    ToolView *tv = m_toolviews[i];

    config->writeEntry (QString ("Kate-MDI-ToolView-%1-Position").arg(tv->id), tv->sidebar()->position());
    config->writeEntry (QString ("Kate-MDI-ToolView-%1-Sidebar-Position").arg(tv->id), i);
    config->writeEntry (QString ("Kate-MDI-ToolView-%1-Visible").arg(tv->id), tv->visible());
    config->writeEntry (QString ("Kate-MDI-ToolView-%1-Persistent").arg(tv->id), tv->persistent);
  }
}

//END SIDEBAR


//BEGIN MAIN WINDOW

MainWindow::MainWindow (QWidget* parentWidget, const char* name)
 : KParts::MainWindow( parentWidget, name)
 , m_sidebarsVisible(true)
 , m_restoreConfig (0)
 , m_guiClient (new GUIClient (this))
{
  // init the internal widgets
  QHBox *hb = new QHBox (this);
  setCentralWidget(hb);

  m_sidebars[KMultiTabBar::Left] = new Sidebar (KMultiTabBar::Left, this, hb);

  m_hSplitter = new Splitter (Qt::Horizontal, hb);
  m_hSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

  m_sidebars[KMultiTabBar::Left]->setSplitter (m_hSplitter);

  QVBox *vb = new QVBox (m_hSplitter);
  m_hSplitter->setCollapsible(vb, false);

  m_sidebars[KMultiTabBar::Top] = new Sidebar (KMultiTabBar::Top, this, vb);

  m_vSplitter = new Splitter (Qt::Vertical, vb);
  m_vSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

  m_sidebars[KMultiTabBar::Top]->setSplitter (m_vSplitter);

  m_centralWidget = new QVBox (m_vSplitter);
  m_vSplitter->setCollapsible(m_centralWidget, false);

  m_sidebars[KMultiTabBar::Bottom] = new Sidebar (KMultiTabBar::Bottom, this, vb);
  m_sidebars[KMultiTabBar::Bottom]->setSplitter (m_vSplitter);

  m_sidebars[KMultiTabBar::Right] = new Sidebar (KMultiTabBar::Right, this, hb);
  m_sidebars[KMultiTabBar::Right]->setSplitter (m_hSplitter);
}

MainWindow::~MainWindow ()
{
  // cu toolviews
  while (!m_toolviews.isEmpty())
    delete m_toolviews[0];

  // seems like we really should delete this by hand ;)
  delete m_centralWidget;

  for (unsigned int i=0; i < 4; ++i)
    delete m_sidebars[i];
}

QWidget *MainWindow::centralWidget () const
{
  return m_centralWidget;
}

ToolView *MainWindow::createToolView (const QString &identifier, KMultiTabBar::KMultiTabBarPosition pos, const QPixmap &icon, const QString &text)
{
  if (m_idToWidget[identifier])
    return 0;

  // try the restore config to figure out real pos
  if (m_restoreConfig && m_restoreConfig->hasGroup (m_restoreGroup))
  {
    m_restoreConfig->setGroup (m_restoreGroup);
    pos = (KMultiTabBar::KMultiTabBarPosition) m_restoreConfig->readNumEntry (QString ("Kate-MDI-ToolView-%1-Position").arg(identifier), pos);
  }

  ToolView *v  = m_sidebars[pos]->addWidget (icon, text, 0);
  v->id = identifier;

  m_idToWidget.insert (identifier, v);
  m_toolviews.push_back (v);

  // register for menu stuff
  m_guiClient->registerToolView (v);

  return v;
}

ToolView *MainWindow::toolView (const QString &identifier) const
{
  return m_idToWidget[identifier];
}

void MainWindow::toolViewDeleted (ToolView *widget)
{
  if (!widget)
    return;

  if (widget->mainWindow() != this)
    return;

  // unregister from menu stuff
  m_guiClient->unregisterToolView (widget);

  widget->sidebar()->removeWidget (widget);

  m_idToWidget.remove (widget->id);
  m_toolviews.remove (widget);
}

void MainWindow::setSidebarsVisible( bool visible )
{
  m_sidebarsVisible = visible;

  m_sidebars[0]->setShown(visible);
  m_sidebars[1]->setShown(visible);
  m_sidebars[2]->setShown(visible);
  m_sidebars[3]->setShown(visible);

  m_guiClient->updateSidebarsVisibleAction();

  // show information message box, if the users hides the sidebars
  if( !m_sidebarsVisible )
  {
    KMessageBox::information( this,
                              i18n("<qt>You are about to hide the sidebars. With "
                                   "hidden sidebars it is not possible to directly "
                                   "access the tool views with the mouse anymore, "
                                   "so if you need to access the sidebars again "
                                   "invoke <b>Window &gt; Tool Views &gt; Show Sidebars</b> "
                                   "in the menu. It is still possible to show/hide "
                                   "the tool views with the assigned shortcuts.</qt>"),
                              QString::null, "Kate hide sidebars notification message" );
  }
}

bool MainWindow::sidebarsVisible() const
{
  return m_sidebarsVisible;
}

void MainWindow::setToolViewStyle (KMultiTabBar::KMultiTabBarStyle style)
{
  m_sidebars[0]->setStyle(style);
  m_sidebars[1]->setStyle(style);
  m_sidebars[2]->setStyle(style);
  m_sidebars[3]->setStyle(style);
}

KMultiTabBar::KMultiTabBarStyle MainWindow::toolViewStyle () const
{
  // all sidebars have the same style, so just take Top
  return m_sidebars[KMultiTabBar::Top]->tabStyle();
}

bool MainWindow::moveToolView (ToolView *widget, KMultiTabBar::KMultiTabBarPosition pos)
{
  if (!widget || widget->mainWindow() != this)
    return false;

  // try the restore config to figure out real pos
  if (m_restoreConfig && m_restoreConfig->hasGroup (m_restoreGroup))
  {
    m_restoreConfig->setGroup (m_restoreGroup);
    pos = (KMultiTabBar::KMultiTabBarPosition) m_restoreConfig->readNumEntry (QString ("Kate-MDI-ToolView-%1-Position").arg(widget->id), pos);
  }

  m_sidebars[pos]->addWidget (widget->icon, widget->text, widget);

  return true;
}

bool MainWindow::showToolView (ToolView *widget)
{
  if (!widget || widget->mainWindow() != this)
    return false;

  // skip this if happens during restoring, or we will just see flicker
  if (m_restoreConfig && m_restoreConfig->hasGroup (m_restoreGroup))
    return true;

  return widget->sidebar()->showWidget (widget);
}

bool MainWindow::hideToolView (ToolView *widget)
{
  if (!widget || widget->mainWindow() != this)
    return false;

  // skip this if happens during restoring, or we will just see flicker
  if (m_restoreConfig && m_restoreConfig->hasGroup (m_restoreGroup))
    return true;

  return widget->sidebar()->hideWidget (widget);
}

void MainWindow::startRestore (KConfig *config, const QString &group)
{
  // first save this stuff
  m_restoreConfig = config;
  m_restoreGroup = group;

  if (!m_restoreConfig || !m_restoreConfig->hasGroup (m_restoreGroup))
  {
    // set sane default sizes
    QValueList<int> hs;
    hs << 200 << 100 << 200;
    QValueList<int> vs;
    vs << 150 << 100 << 200;

    m_sidebars[0]->setLastSize (hs[0]);
    m_sidebars[1]->setLastSize (hs[2]);
    m_sidebars[2]->setLastSize (vs[0]);
    m_sidebars[3]->setLastSize (vs[2]);

    m_hSplitter->setSizes(hs);
    m_vSplitter->setSizes(vs);
    return;
  }

  // apply size once, to get sizes ready ;)
  m_restoreConfig->setGroup (m_restoreGroup);
  restoreWindowSize (m_restoreConfig);

  m_restoreConfig->setGroup (m_restoreGroup);

  // get main splitter sizes ;)
  QValueList<int> hs = m_restoreConfig->readIntListEntry ("Kate-MDI-H-Splitter");
  QValueList<int> vs = m_restoreConfig->readIntListEntry ("Kate-MDI-V-Splitter");

  m_sidebars[0]->setLastSize (hs[0]);
  m_sidebars[1]->setLastSize (hs[2]);
  m_sidebars[2]->setLastSize (vs[0]);
  m_sidebars[3]->setLastSize (vs[2]);

  m_hSplitter->setSizes(hs);
  m_vSplitter->setSizes(vs);

  setToolViewStyle( (KMultiTabBar::KMultiTabBarStyle)m_restoreConfig->readNumEntry ("Kate-MDI-Sidebar-Style", (int)toolViewStyle()) );

  // after reading m_sidebarsVisible, update the GUI toggle action
  m_sidebarsVisible = m_restoreConfig->readBoolEntry ("Kate-MDI-Sidebar-Visible", true );
  m_guiClient->updateSidebarsVisibleAction();
}

void MainWindow::finishRestore ()
{
  if (!m_restoreConfig)
    return;

  if (m_restoreConfig->hasGroup (m_restoreGroup))
  {
    // apply all settings, like toolbar pos and more ;)
    applyMainWindowSettings(m_restoreConfig, m_restoreGroup);

    // reshuffle toolviews only if needed
    m_restoreConfig->setGroup (m_restoreGroup);
    for ( unsigned int i=0; i < m_toolviews.size(); ++i )
    {
      KMultiTabBar::KMultiTabBarPosition newPos = (KMultiTabBar::KMultiTabBarPosition) m_restoreConfig->readNumEntry (QString ("Kate-MDI-ToolView-%1-Position").arg(m_toolviews[i]->id), m_toolviews[i]->sidebar()->position());

      if (m_toolviews[i]->sidebar()->position() != newPos)
      {
        moveToolView (m_toolviews[i], newPos);
      }
    }

    // restore the sidebars
    m_restoreConfig->setGroup (m_restoreGroup);
    for (unsigned int i=0; i < 4; ++i)
      m_sidebars[i]->restoreSession (m_restoreConfig);
  }

  // clear this stuff, we are done ;)
  m_restoreConfig = 0;
  m_restoreGroup = "";
}

void MainWindow::saveSession (KConfig *config, const QString &group)
{
  if (!config)
    return;

  saveMainWindowSettings (config, group);

  config->setGroup (group);

  // save main splitter sizes ;)
  QValueList<int> hs = m_hSplitter->sizes();
  QValueList<int> vs = m_vSplitter->sizes();

  if (hs[0] <= 2 && !m_sidebars[0]->splitterVisible ())
    hs[0] = m_sidebars[0]->lastSize();
  if (hs[2] <= 2 && !m_sidebars[1]->splitterVisible ())
    hs[2] = m_sidebars[1]->lastSize();
  if (vs[0] <= 2 && !m_sidebars[2]->splitterVisible ())
    vs[0] = m_sidebars[2]->lastSize();
  if (vs[2] <= 2 && !m_sidebars[3]->splitterVisible ())
    vs[2] = m_sidebars[3]->lastSize();

  config->writeEntry ("Kate-MDI-H-Splitter", hs);
  config->writeEntry ("Kate-MDI-V-Splitter", vs);

  // save sidebar style
  config->writeEntry ("Kate-MDI-Sidebar-Style", (int)toolViewStyle());
  config->writeEntry ("Kate-MDI-Sidebar-Visible", m_sidebarsVisible );

  // save the sidebars
  for (unsigned int i=0; i < 4; ++i)
    m_sidebars[i]->saveSession (config);
}

//END MAIN WINDOW

} // namespace KateMDI

// kate: space-indent on; indent-width 2;
