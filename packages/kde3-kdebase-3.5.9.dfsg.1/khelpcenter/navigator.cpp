/*
 *  This file is part of the KDE Help Center
 *
 *  Copyright (C) 1999 Matthias Elter (me@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qdir.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qlabel.h>
#include <qheader.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <kaction.h>
#include <kapplication.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kprocio.h>
#include <kcharsets.h>
#include <kdialog.h>
#include <kdesktopfile.h>
#include <kprotocolinfo.h>
#include <kservicegroup.h>

#include "navigatoritem.h"
#include "navigatorappitem.h"
#include "searchwidget.h"
#include "searchengine.h"
#include "docmetainfo.h"
#include "docentrytraverser.h"
#include "glossary.h"
#include "toc.h"
#include "view.h"
#include "infotree.h"
#include "mainwindow.h"
#include "plugintraverser.h"
#include "scrollkeepertreebuilder.h"
#include "kcmhelpcenter.h"
#include "formatter.h"
#include "history.h"
#include "prefs.h"

#include "navigator.h"

using namespace KHC;

Navigator::Navigator( View *view, QWidget *parent, const char *name )
   : QWidget( parent, name ), mIndexDialog( 0 ),
     mView( view ), mSelected( false )
{
    KConfig *config = kapp->config();
    config->setGroup("General");
    mShowMissingDocs = config->readBoolEntry("ShowMissingDocs",false);

    mSearchEngine = new SearchEngine( view );
    connect( mSearchEngine, SIGNAL( searchFinished() ),
             SLOT( slotSearchFinished() ) );

    DocMetaInfo::self()->scanMetaInfo();

    QBoxLayout *topLayout = new QVBoxLayout( this );

    mSearchFrame = new QFrame( this );
    topLayout->addWidget( mSearchFrame );

    QBoxLayout *searchLayout = new QHBoxLayout( mSearchFrame );
    searchLayout->setSpacing( KDialog::spacingHint() );
    searchLayout->setMargin( 6 );

    QPushButton *clearButton = new QPushButton( mSearchFrame );
    clearButton->setIconSet( KApplication::reverseLayout() ?
      SmallIconSet( "clear_left" ) : SmallIconSet("locationbar_erase") );
    searchLayout->addWidget( clearButton );
    connect( clearButton, SIGNAL( clicked() ), SLOT( clearSearch() ) );
    QToolTip::add( clearButton, i18n("Clear search") );

    mSearchEdit = new QLineEdit( mSearchFrame );
    searchLayout->addWidget( mSearchEdit );
    connect( mSearchEdit, SIGNAL( returnPressed() ), SLOT( slotSearch() ) );
    connect( mSearchEdit, SIGNAL( textChanged( const QString & ) ),
             SLOT( checkSearchButton() ) );

    mSearchButton = new QPushButton( i18n("&Search"), mSearchFrame );
    searchLayout->addWidget( mSearchButton );
    connect( mSearchButton, SIGNAL( clicked() ), SLOT( slotSearch() ) );

    clearButton->setFixedHeight( mSearchButton->height() );

    mTabWidget = new QTabWidget( this );
    topLayout->addWidget( mTabWidget );
    connect( mTabWidget, SIGNAL( currentChanged( QWidget * ) ),
             SLOT( slotTabChanged( QWidget * ) ) );

    setupContentsTab();
    setupGlossaryTab();
    setupSearchTab();

    insertPlugins();

    if ( !mSearchEngine->initSearchHandlers() ) {
      hideSearch();
    } else {
      mSearchWidget->updateScopeList();
      mSearchWidget->readConfig( KGlobal::config() );
    }
}

Navigator::~Navigator()
{
  delete mSearchEngine;
}

SearchEngine *Navigator::searchEngine() const
{
  return mSearchEngine;
}

Formatter *Navigator::formatter() const
{
  return mView->formatter();
}

bool Navigator::showMissingDocs() const
{
  return mShowMissingDocs;
}

void Navigator::setupContentsTab()
{
    mContentsTree = new KListView( mTabWidget );
    mContentsTree->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    mContentsTree->addColumn(QString::null);
    mContentsTree->setAllColumnsShowFocus(true);
    mContentsTree->header()->hide();
    mContentsTree->setRootIsDecorated(false);
    mContentsTree->setSorting(-1, false);

    connect(mContentsTree, SIGNAL(clicked(QListViewItem*)),
            SLOT(slotItemSelected(QListViewItem*)));
    connect(mContentsTree, SIGNAL(returnPressed(QListViewItem*)),
           SLOT(slotItemSelected(QListViewItem*)));
    mTabWidget->addTab(mContentsTree, i18n("&Contents"));
}

void Navigator::setupSearchTab()
{
    mSearchWidget = new SearchWidget( mSearchEngine, mTabWidget );
    connect( mSearchWidget, SIGNAL( searchResult( const QString & ) ),
             SLOT( slotShowSearchResult( const QString & ) ) );
    connect( mSearchWidget, SIGNAL( scopeCountChanged( int ) ),
             SLOT( checkSearchButton() ) );
    connect( mSearchWidget, SIGNAL( showIndexDialog() ),
      SLOT( showIndexDialog() ) );

    mTabWidget->addTab( mSearchWidget, i18n("Search Options"));
}

void Navigator::setupGlossaryTab()
{
    mGlossaryTree = new Glossary( mTabWidget );
    connect( mGlossaryTree, SIGNAL( entrySelected( const GlossaryEntry & ) ),
             this, SIGNAL( glossSelected( const GlossaryEntry & ) ) );
    mTabWidget->addTab( mGlossaryTree, i18n( "G&lossary" ) );
}

void Navigator::insertPlugins()
{
  PluginTraverser t( this, mContentsTree );
  DocMetaInfo::self()->traverseEntries( &t );

#if 0
  kdDebug( 1400 ) << "<docmetainfo>" << endl;
  DocEntry::List entries = DocMetaInfo::self()->docEntries();
  DocEntry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    (*it)->dump();
  }
  kdDebug( 1400 ) << "</docmetainfo>" << endl;
#endif
}

void Navigator::insertParentAppDocs( const QString &name, NavigatorItem *topItem )
{
  kdDebug(1400) << "Requested plugin documents for ID " << name << endl;

  KServiceGroup::Ptr grp = KServiceGroup::childGroup( name );
  if ( !grp )
    return;

  KServiceGroup::List entries = grp->entries();
  KServiceGroup::List::ConstIterator it = entries.begin();
  KServiceGroup::List::ConstIterator end = entries.end();
  for ( ; it != end; ++it ) {
    QString desktopFile = ( *it )->entryPath();
    if ( QDir::isRelativePath( desktopFile ) )
        desktopFile = locate( "apps", desktopFile );
    createItemFromDesktopFile( topItem, desktopFile );
  }
}

void Navigator::insertIOSlaveDocs( const QString &name, NavigatorItem *topItem )
{
  kdDebug(1400) << "Requested IOSlave documents for ID " << name << endl;

#if KDE_IS_VERSION( 3, 1, 90 )
  QStringList list = KProtocolInfo::protocols();
  list.sort();

  NavigatorItem *prevItem = 0;
  for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
  {
    QString docPath = KProtocolInfo::docPath(*it);
    if ( !docPath.isNull() )
    {
      // First parameter is ignored if second is an absolute path
      KURL url(KURL("help:/"), docPath);
      QString icon = KProtocolInfo::icon(*it);
      if ( icon.isEmpty() ) icon = "document2";
      DocEntry *entry = new DocEntry( *it, url.url(), icon );
      NavigatorItem *item = new NavigatorItem( entry, topItem, prevItem );
      prevItem = item;
      item->setAutoDeleteDocEntry( true );
    }
  }
#else
  Q_UNUSED( topItem );
#endif
}

void Navigator::insertAppletDocs( NavigatorItem *topItem )
{
  QDir appletDir( locate( "data", QString::fromLatin1( "kicker/applets/" ) ) );
  appletDir.setNameFilter( QString::fromLatin1( "*.desktop" ) );

  QStringList files = appletDir.entryList( QDir::Files | QDir::Readable );
  QStringList::ConstIterator it = files.begin();
  QStringList::ConstIterator end = files.end();
  for ( ; it != end; ++it )
    createItemFromDesktopFile( topItem, appletDir.absPath() + "/" + *it );
}

void Navigator::createItemFromDesktopFile( NavigatorItem *topItem,
                                           const QString &file )
{
    KDesktopFile desktopFile( file );
    QString docPath = desktopFile.readDocPath();
    if ( !docPath.isNull() ) {
      // First parameter is ignored if second is an absolute path
      KURL url(KURL("help:/"), docPath);
      QString icon = desktopFile.readIcon();
      if ( icon.isEmpty() ) icon = "document2";
      DocEntry *entry = new DocEntry( desktopFile.readName(), url.url(), icon );
      NavigatorItem *item = new NavigatorItem( entry, topItem );
      item->setAutoDeleteDocEntry( true );
    }
}

void Navigator::insertInfoDocs( NavigatorItem *topItem )
{
  InfoTree *infoTree = new InfoTree( this );
  infoTree->build( topItem );
}

NavigatorItem *Navigator::insertScrollKeeperDocs( NavigatorItem *topItem,
                                                  NavigatorItem *after )
{
  ScrollKeeperTreeBuilder *builder = new ScrollKeeperTreeBuilder( this );
  return builder->build( topItem, after );
}

void Navigator::selectItem( const KURL &url )
{
  kdDebug() << "Navigator::selectItem(): " << url.url() << endl;

  if ( url.url() == "khelpcenter:home" ) {
    clearSelection();
    return;
  }

  // help:/foo&anchor=bar gets redirected to help:/foo#bar
  // Make sure that we match both the original URL as well as
  // its counterpart.
  KURL alternativeURL = url;
  if (url.hasRef())
  {
     alternativeURL.setQuery("anchor="+url.ref());
     alternativeURL.setRef(QString::null);
  }

  // If the navigator already has the given URL selected, do nothing.
  NavigatorItem *item;
  item = static_cast<NavigatorItem *>( mContentsTree->selectedItem() );
  if ( item && mSelected ) {
    KURL currentURL ( item->entry()->url() );
    if ( (currentURL == url) || (currentURL == alternativeURL) ) {
      kdDebug() << "URL already shown." << endl;
      return;
    }
  }

  // First, populate the NavigatorAppItems if we don't want the home page
  if ( url != homeURL() ) {
    for ( QListViewItem *item = mContentsTree->firstChild(); item;
          item = item->nextSibling() ) {
      NavigatorAppItem *appItem = dynamic_cast<NavigatorAppItem *>( item );
      if ( appItem ) appItem->populate( true /* recursive */ );
    }
  }

  QListViewItemIterator it( mContentsTree );
  while ( it.current() ) {
    NavigatorItem *item = static_cast<NavigatorItem *>( it.current() );
    KURL itemUrl( item->entry()->url() );
    if ( (itemUrl == url) || (itemUrl == alternativeURL) ) {
      mContentsTree->setCurrentItem( item );
      // If the current item was not selected and remained unchanged it
      // needs to be explicitly selected
      mContentsTree->setSelected(item, true);
      item->setOpen( true );
      mContentsTree->ensureItemVisible( item );
      break;
    }
    ++it;
  }
  if ( !it.current() ) {
    clearSelection();
  } else {
    mSelected = true;
  }
}

void Navigator::clearSelection()
{
  mContentsTree->clearSelection();
  mSelected = false;
}

void Navigator::slotItemSelected( QListViewItem *currentItem )
{
  if ( !currentItem ) return;

  mSelected = true;

  NavigatorItem *item = static_cast<NavigatorItem *>( currentItem );

  kdDebug(1400) << "Navigator::slotItemSelected(): " << item->entry()->name()
                << endl;

  if ( item->childCount() > 0 || item->isExpandable() )
    item->setOpen( !item->isOpen() );

  KURL url ( item->entry()->url() );

  if ( url.protocol() == "khelpcenter" ) {
      mView->closeURL();
      History::self().updateCurrentEntry( mView );
      History::self().createEntry();
      showOverview( item, url );
  } else {
    if ( url.protocol() == "help" ) {
      kdDebug( 1400 ) << "slotItemSelected(): Got help URL " << url.url()
                      << endl;
      if ( !item->toc() ) {
        TOC *tocTree = item->createTOC();
        kdDebug( 1400 ) << "slotItemSelected(): Trying to build TOC for "
                        << item->entry()->name() << endl;
        tocTree->setApplication( url.directory() );
        QString doc = View::langLookup( url.path() );
        // Enforce the original .docbook version, in case langLookup returns a
        // cached version
        if ( !doc.isNull() ) {
          int pos = doc.find( ".html" );
          if ( pos >= 0 ) {
            doc.replace( pos, 5, ".docbook" );
          }
          kdDebug( 1400 ) << "slotItemSelected(): doc = " << doc << endl;

          tocTree->build( doc );
        }
      }
    }
    emit itemSelected( url.url() );
  }

  mLastUrl = url;
}

void Navigator::openInternalUrl( const KURL &url )
{
  if ( url.url() == "khelpcenter:home" ) {
    clearSelection();
    showOverview( 0, url );
    return;
  }

  selectItem( url );
  if ( !mSelected ) return;

  NavigatorItem *item =
    static_cast<NavigatorItem *>( mContentsTree->currentItem() );

  if ( item ) showOverview( item, url );
}

void Navigator::showOverview( NavigatorItem *item, const KURL &url )
{
  mView->beginInternal( url );

  QString fileName = locate( "data", "khelpcenter/index.html.in" );
  if ( fileName.isEmpty() )
    return;

  QFile file( fileName );

  if ( !file.open( IO_ReadOnly ) )
    return;

  QTextStream stream( &file );
  QString res = stream.read();

  QString title,name,content;
  uint childCount;

  if ( item ) {
    title = item->entry()->name();
    name = item->entry()->name();

    QString info = item->entry()->info();
    if ( !info.isEmpty() ) content = "<p>" + info + "</p>\n";

    childCount = item->childCount();
  } else {
    title = i18n("Start Page");
    name = i18n("KDE Help Center");

    childCount = mContentsTree->childCount();
  }

  if ( childCount > 0 ) {
    QListViewItem *child;
    if ( item ) child = item->firstChild();
    else child = mContentsTree->firstChild();

    mDirLevel = 0;

    content += createChildrenList( child );
  }
  else
    content += "<p></p>";

  res = res.arg(title).arg(name).arg(content);

  mView->write( res );

  mView->end();
}

QString Navigator::createChildrenList( QListViewItem *child )
{
  ++mDirLevel;

  QString t;

  t += "<ul>\n";

  while ( child ) {
    NavigatorItem *childItem = static_cast<NavigatorItem *>( child );

    DocEntry *e = childItem->entry();

    t += "<li><a href=\"" + e->url() + "\">";
    if ( e->isDirectory() ) t += "<b>";
    t += e->name();
    if ( e->isDirectory() ) t += "</b>";
    t += "</a>";

    if ( !e->info().isEmpty() ) {
      t += "<br>" + e->info();
    }

    t += "</li>\n";

    if ( childItem->childCount() > 0 && mDirLevel < 2 ) {
      t += createChildrenList( childItem->firstChild() );
    }

    child = child->nextSibling();
  }

  t += "</ul>\n";

  --mDirLevel;

  return t;
}

void Navigator::slotSearch()
{
  kdDebug(1400) << "Navigator::slotSearch()" << endl;

  if ( !checkSearchIndex() ) return;

  if ( mSearchEngine->isRunning() ) return;

  QString words = mSearchEdit->text();
  QString method = mSearchWidget->method();
  int pages = mSearchWidget->pages();
  QString scope = mSearchWidget->scope();

  kdDebug(1400) << "Navigator::slotSearch() words: " << words << endl;
  kdDebug(1400) << "Navigator::slotSearch() scope: " << scope << endl;

  if ( words.isEmpty() || scope.isEmpty() ) return;

  // disable search Button during searches
  mSearchButton->setEnabled(false);
  QApplication::setOverrideCursor(waitCursor);

  if ( !mSearchEngine->search( words, method, pages, scope ) ) {
    slotSearchFinished();
    KMessageBox::sorry( this, i18n("Unable to run search program.") );
  }
}

void Navigator::slotShowSearchResult( const QString &url )
{
  QString u = url;
  u.replace( "%k", mSearchEdit->text() );

  emit itemSelected( u );
}

void Navigator::slotSearchFinished()
{
  mSearchButton->setEnabled(true);
  QApplication::restoreOverrideCursor();

  kdDebug( 1400 ) << "Search finished." << endl;
}

void Navigator::checkSearchButton()
{
  mSearchButton->setEnabled( !mSearchEdit->text().isEmpty() &&
    mSearchWidget->scopeCount() > 0 );
  mTabWidget->showPage( mSearchWidget );
}

void Navigator::hideSearch()
{
  mSearchFrame->hide();
  mTabWidget->removePage( mSearchWidget );
}

bool Navigator::checkSearchIndex()
{
  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "Search" );
  if ( cfg->readBoolEntry( "IndexExists", false ) ) return true;

  if ( mIndexDialog && mIndexDialog->isShown() ) return true;

  QString text = i18n( "A search index does not yet exist. Do you want "
                       "to create the index now?" );

  int result = KMessageBox::questionYesNo( this, text, QString::null,
                                           i18n("Create"),
                                           i18n("Do Not Create"),
                                           "indexcreation" );
  if ( result == KMessageBox::Yes ) {
    showIndexDialog();
    return false;
  }

  return true;
}

void Navigator::slotTabChanged( QWidget *wid )
{
  if ( wid == mSearchWidget ) checkSearchIndex();
}

void Navigator::slotSelectGlossEntry( const QString &id )
{
  mGlossaryTree->slotSelectGlossEntry( id );
}

KURL Navigator::homeURL()
{
  if ( !mHomeUrl.isEmpty() ) return mHomeUrl;

  KConfig *cfg = KGlobal::config();
  // We have to reparse the configuration here in order to get a
  // language-specific StartUrl, e.g. "StartUrl[de]".
  cfg->reparseConfiguration();
  cfg->setGroup( "General" );
  mHomeUrl = cfg->readPathEntry( "StartUrl", "khelpcenter:home" );
  return mHomeUrl;
}

void Navigator::showIndexDialog()
{
  if ( !mIndexDialog ) {
    mIndexDialog = new KCMHelpCenter( mSearchEngine, this );
    connect( mIndexDialog, SIGNAL( searchIndexUpdated() ), mSearchWidget,
             SLOT( updateScopeList() ) );
  }
  mIndexDialog->show();
  mIndexDialog->raise();
}

void Navigator::readConfig()
{
  if ( Prefs::currentTab() == Prefs::Search ) {
    mTabWidget->showPage( mSearchWidget );
  } else if ( Prefs::currentTab() == Prefs::Glossary ) {
    mTabWidget->showPage( mGlossaryTree );
  } else {
    mTabWidget->showPage( mContentsTree );
  }
}

void Navigator::writeConfig()
{
  if ( mTabWidget->currentPage() == mSearchWidget ) {
    Prefs::setCurrentTab( Prefs::Search );
  } else if ( mTabWidget->currentPage() == mGlossaryTree ) {
    Prefs::setCurrentTab( Prefs::Glossary );
  } else {
    Prefs::setCurrentTab( Prefs::Content );
  }
}

void Navigator::clearSearch()
{
  mSearchEdit->setText( QString::null );
}

#include "navigator.moc"

// vim:ts=2:sw=2:et
