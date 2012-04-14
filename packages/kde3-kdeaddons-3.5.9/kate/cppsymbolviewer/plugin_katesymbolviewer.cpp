/***************************************************************************
 *                        plugin_katesymbolviewer.cpp  -  description
 *                           -------------------
 *  begin                : Apr 2 2003
 *  author               : 2003 Massimo Callegari
 *  email                : massimocallegari@yahoo.it
 *
 *  Changes:
 *  Nov 09 2004 v.1.3 - For changelog please refer to KDE CVS
 *  Nov 05 2004 v.1.2 - Choose parser from the current highlight. Minor i18n changes.
 *  Nov 28 2003 v.1.1 - Structured for multilanguage support
 *                      Added preliminary Tcl/Tk parser (thanks Rohit). To be improved.
 *                      Various bugfixing.
 *  Jun 19 2003 v.1.0 - Removed QTimer (polling is Evil(tm)... )
 *                      - Captured documentChanged() event to refresh symbol list
 *                      - Tooltips vanished into nowhere...sigh :(
 *  May 04 2003 v 0.6 - Symbol List becomes a KListView object. Removed Tooltip class.
 *                      Added a QTimer that every 200ms checks:
 *                      * if the list width has changed
 *                      * if the document has changed
 *                      Added an entry in the popup menu to switch between List and Tree mode
 *                      Various bugfixing.
 *  Apr 24 2003 v 0.5 - Added three check buttons in popup menu to show/hide symbols
 *  Apr 23 2003 v 0.4 - "View Symbol" moved in Settings menu. "Refresh List" is no
 *                      longer in Kate menu. Moved into a popup menu activated by a
 *                      mouse right button click. + Bugfixing.
 *  Apr 22 2003 v 0.3 - Added macro extraction + several bugfixing
 *  Apr 19 2003 v 0.2 - Added to CVS. Extract functions and structures
 *  Apr 07 2003 v 0.1 - First version.
 *
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "plugin_katesymbolviewer.h"
#include "plugin_katesymbolviewer.moc"

#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <kfiledialog.h>

#include <qlayout.h>
#include <qgroupbox.h>

K_EXPORT_COMPONENT_FACTORY( katecppsymbolviewerplugin, KGenericFactory<KatePluginSymbolViewer>( "katesymbolviewer" ) )

KatePluginSymbolViewerView::KatePluginSymbolViewerView(Kate::MainWindow *w)
{
    KGlobal::locale()->insertCatalogue("katecppsymbolviewer");
    KToggleAction* act = new KToggleAction ( i18n("Hide Symbols"), 0, this, SLOT( slotInsertSymbol() ), actionCollection(),      "view_insert_symbolviewer" );
    act->setCheckedState(i18n("Show Symbols"));

    setInstance (new KInstance("kate"));
    setXMLFile("plugins/katecppsymbolviewer/ui.rc");
    w->guiFactory()->addClient (this);
  win = w;
  symbols = 0;

 m_Active = false;
 popup = new QPopupMenu();
 popup->insertItem(i18n("Refresh List"), this, SLOT(slotRefreshSymbol()));
 popup->insertSeparator();
 m_macro = popup->insertItem(i18n("Show Macros"), this, SLOT(toggleShowMacros()));
 m_struct = popup->insertItem(i18n("Show Structures"), this, SLOT(toggleShowStructures()));
 m_func = popup->insertItem(i18n("Show Functions"), this, SLOT(toggleShowFunctions()));
 popup->insertSeparator();
 popup->insertItem(i18n("List/Tree Mode"), this, SLOT(slotChangeMode()));
 m_sort = popup->insertItem(i18n("Enable sorting"), this, SLOT(slotEnableSorting()));

 popup->setItemChecked(m_macro, true);
 popup->setItemChecked(m_struct, true);
 popup->setItemChecked(m_func, true);
 macro_on = true;
 struct_on = true;
 func_on = true;
 slotInsertSymbol();
}

KatePluginSymbolViewerView::~KatePluginSymbolViewerView()
{
  win->guiFactory()->removeClient (this);
  delete dock;
  delete popup;
}

void KatePluginSymbolViewerView::toggleShowMacros(void)
{
 bool s = !popup->isItemChecked(m_macro);
 popup->setItemChecked(m_macro, s);
 macro_on = s;
 slotRefreshSymbol();
}

void KatePluginSymbolViewerView::toggleShowStructures(void)
{
 bool s = !popup->isItemChecked(m_struct);
 popup->setItemChecked(m_struct, s);
 struct_on = s;
 slotRefreshSymbol();
}

void KatePluginSymbolViewerView::toggleShowFunctions(void)
{
 bool s = !popup->isItemChecked(m_func);
 popup->setItemChecked(m_func, s);
 func_on = s;
 slotRefreshSymbol();
}

void KatePluginSymbolViewerView::slotInsertSymbol()
{
 QPixmap cls( ( const char** ) class_xpm );

 if (m_Active == false)
     {
      dock = win->toolViewManager()->createToolView("kate_plugin_cppsymbolviewer", Kate::ToolViewManager::Left, cls, i18n("Symbol List"));

      symbols = new KListView(dock);
      treeMode = 0;

      connect(symbols, SIGNAL(executed(QListViewItem *)), this, SLOT(goToSymbol(QListViewItem *)));
      connect(symbols, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint&, int)),
               SLOT(slotShowContextMenu(QListViewItem *, const QPoint&, int)));
      connect(win->viewManager(), SIGNAL(viewChanged()), this, SLOT(slotDocChanged()));
      //connect(symbols, SIGNAL(resizeEvent(QResizeEvent *)), this, SLOT(slotViewChanged(QResizeEvent *)));

      m_Active = true;
      //symbols->addColumn(i18n("Symbols"), symbols->parentWidget()->width());
      symbols->addColumn(i18n("Symbols"));
      symbols->addColumn(i18n("Position"));
      symbols->setColumnWidthMode(1, QListView::Manual);
      symbols->setColumnWidth ( 1, 0 );
      symbols->setSorting(-1, FALSE);
      symbols->setRootIsDecorated(0);
      symbols->setTreeStepSize(10);
      symbols->setShowToolTips(TRUE);

      /* First Symbols parsing here...*/
      parseSymbols();
     }
  else
     {
      delete dock;
      dock = 0;
      symbols = 0;
      m_Active = false;
     }
}

void KatePluginSymbolViewerView::slotRefreshSymbol()
{
  if (!symbols)
    return;
 symbols->clear();
 parseSymbols();
}

void KatePluginSymbolViewerView::slotChangeMode()
{
 treeMode = !treeMode;
 symbols->clear();
 parseSymbols();
}

void KatePluginSymbolViewerView::slotEnableSorting()
{
 lsorting = !lsorting;
 popup->setItemChecked(m_sort, lsorting);
 symbols->clear();
 if (lsorting == TRUE)
     symbols->setSorting(0, TRUE);
 else
     symbols->setSorting(-1, FALSE);

 parseSymbols();
}

void KatePluginSymbolViewerView::slotDocChanged()
{
 //kdDebug(13000)<<"Document changed !!!!"<<endl;
 slotRefreshSymbol();
}

void KatePluginSymbolViewerView::slotViewChanged(QResizeEvent *)
{
 kdDebug(13000)<<"View changed !!!!"<<endl;
 symbols->setColumnWidth(0, symbols->parentWidget()->width());
}

void KatePluginSymbolViewerView::slotShowContextMenu(QListViewItem *, const QPoint &p, int)
{
 popup->popup(p);
}

void KatePluginSymbolViewerView::parseSymbols(void)
{
  unsigned int hlMode = 0;

  if (!win->viewManager()->activeView())
    return;

  Kate::Document *kv = win->viewManager()->activeView()->getDoc();

  // be sure we have some document around !
  if (!kv)
    return;

  /** Get the current highlighting mode */
  hlMode = kv->hlMode();
  QString hlModeName = kv->hlModeName(hlMode);

  //QListViewItem mcrNode = new QListViewItem(symbols, symbols->lastItem(), hlModeName);

  if (hlModeName == "C++" || hlModeName == "C")
     parseCppSymbols();
  else if (hlModeName == "Tcl/Tk")
     parseTclSymbols();
  else if (hlModeName == "Java")
     parseCppSymbols();
}

void KatePluginSymbolViewerView::goToSymbol(QListViewItem *it)
{
  Kate::View *kv = win->viewManager()->activeView();

  // be sure we really have a view !
  if (!kv)
    return;

  kdDebug(13000)<<"Slot Activated at pos: "<<symbols->itemIndex(it) <<endl;
  kv->gotoLineNumber(it->text(1).toInt(NULL, 10) + 10);
  kv->setFocus();
  kv->gotoLineNumber(it->text(1).toInt(NULL, 10));
}

KatePluginSymbolViewer::KatePluginSymbolViewer( QObject* parent, const char* name, const QStringList& )
    : Kate::Plugin ( (Kate::Application*)parent, name ),
    pConfig("katecppsymbolviewerpluginrc")
{
 pConfig.setGroup("global");
}

KatePluginSymbolViewer::~KatePluginSymbolViewer()
{
  pConfig.sync();
}

void KatePluginSymbolViewer::addView (Kate::MainWindow *win)
{
  KatePluginSymbolViewerView *view = new KatePluginSymbolViewerView (win);
  m_views.append (view);
  view->types_on = pConfig.readBoolEntry("view_types", true);
  view->expanded_on = pConfig.readBoolEntry("expand_tree", false);
}

void KatePluginSymbolViewer::removeView(Kate::MainWindow *win)
{
  for (uint z=0; z < m_views.count(); z++)
    if (m_views.at(z)->win == win)
    {
      KatePluginSymbolViewerView *view = m_views.at(z);
      m_views.remove (view);
      pConfig.writeEntry("view_types", view->types_on);
      pConfig.writeEntry("expand_tree", view->expanded_on);
      delete view;
      return;
    }
}

Kate::PluginConfigPage* KatePluginSymbolViewer::configPage(
    uint, QWidget *w, const char* /*name*/)
{
  KatePluginSymbolViewerConfigPage* p = new KatePluginSymbolViewerConfigPage(this, w);
  initConfigPage( p );
  connect( p, SIGNAL(configPageApplyRequest(KatePluginSymbolViewerConfigPage*)),
      SLOT(applyConfig(KatePluginSymbolViewerConfigPage *)) );
  return (Kate::PluginConfigPage*)p;
}

void KatePluginSymbolViewer::initConfigPage( KatePluginSymbolViewerConfigPage* p )
{
  p->viewReturns->setChecked(pConfig.readBoolEntry("view_types", true));
  p->expandTree->setChecked(pConfig.readBoolEntry("expand_tree", false));
}

void KatePluginSymbolViewer::applyConfig( KatePluginSymbolViewerConfigPage* p )
{
  for (uint z=0; z < m_views.count(); z++)
  {
    m_views.at(z)->types_on = p->viewReturns->isChecked();
    m_views.at(z)->expanded_on = p->expandTree->isChecked();
  //kdDebug(13000)<<"KatePluginSymbolViewer: Configuration applied.("<<m_SymbolView->types_on<<")"<<endl;
    m_views.at(z)->slotRefreshSymbol();
  }

  pConfig.writeEntry("view_types", p->viewReturns->isChecked());
  pConfig.writeEntry("expand_tree", p->expandTree->isChecked());
}

// BEGIN KatePluginSymbolViewerConfigPage
KatePluginSymbolViewerConfigPage::KatePluginSymbolViewerConfigPage(
    QObject* /*parent*/ /*= 0L*/, QWidget *parentWidget /*= 0L*/)
  : Kate::PluginConfigPage( parentWidget )
{
  QVBoxLayout* top = new QVBoxLayout(this, 0,
      KDialogBase::spacingHint());

  QGroupBox* gb = new QGroupBox( i18n("Parser Options"),
      this, "cppsymbolviewer_config_page_layout" );
  gb->setColumnLayout(1, Qt::Horizontal);
  gb->setInsideSpacing(KDialogBase::spacingHint());
  viewReturns = new QCheckBox(i18n("Display functions parameters"), gb);
  expandTree = new QCheckBox(i18n("Automatically expand nodes in tree mode"), gb);

  top->add(gb);
  top->addStretch(1);
//  throw signal changed
  connect(viewReturns, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
  connect(expandTree, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
}

KatePluginSymbolViewerConfigPage::~KatePluginSymbolViewerConfigPage() {}

void KatePluginSymbolViewerConfigPage::apply()
{
    emit configPageApplyRequest( this );
}
// END KatePluginSymbolViewerConfigPage

