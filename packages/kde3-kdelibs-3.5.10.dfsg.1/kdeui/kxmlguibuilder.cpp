/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

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

#include "kapplication.h"
#include "kxmlguibuilder.h"
#include "kmenubar.h"
#include "kpopupmenu.h"
#include "ktoolbar.h"
#include "kstatusbar.h"
#include "kmainwindow.h"
#include "kaction.h"
#include "kglobalsettings.h"
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qobjectlist.h>

class KXMLGUIBuilderPrivate
{
public:
    KXMLGUIBuilderPrivate() {
    }
  ~KXMLGUIBuilderPrivate() {
  }

    QWidget *m_widget;

    QString tagMainWindow;
    QString tagMenuBar;
    QString tagMenu;
    QString tagToolBar;
    QString tagStatusBar;

    QString tagSeparator;
    QString tagTearOffHandle;
    QString tagMenuTitle;

    QString attrName;
    QString attrLineSeparator;

    QString attrText1;
    QString attrText2;
    QString attrContext;

    QString attrIcon;

    KInstance *m_instance;
    KXMLGUIClient *m_client;
};

KXMLGUIBuilder::KXMLGUIBuilder( QWidget *widget )
{
  d = new KXMLGUIBuilderPrivate;
  d->m_widget = widget;

  d->tagMainWindow = QString::fromLatin1( "mainwindow" );
  d->tagMenuBar = QString::fromLatin1( "menubar" );
  d->tagMenu = QString::fromLatin1( "menu" );
  d->tagToolBar = QString::fromLatin1( "toolbar" );
  d->tagStatusBar = QString::fromLatin1( "statusbar" );

  d->tagSeparator = QString::fromLatin1( "separator" );
  d->tagTearOffHandle = QString::fromLatin1( "tearoffhandle" );
  d->tagMenuTitle = QString::fromLatin1( "title" );

  d->attrName = QString::fromLatin1( "name" );
  d->attrLineSeparator = QString::fromLatin1( "lineseparator" );

  d->attrText1 = QString::fromLatin1( "text" );
  d->attrText2 = QString::fromLatin1( "Text" );
  d->attrContext = QString::fromLatin1( "context" );

  d->attrIcon = QString::fromLatin1( "icon" );

  d->m_instance = 0;
  d->m_client = 0;
}

KXMLGUIBuilder::~KXMLGUIBuilder()
{
  delete d;
}

QWidget *KXMLGUIBuilder::widget()
{
  return d->m_widget;
}

QStringList KXMLGUIBuilder::containerTags() const
{
  QStringList res;
  res << d->tagMenu << d->tagToolBar << d->tagMainWindow << d->tagMenuBar << d->tagStatusBar;

  return res;
}

QWidget *KXMLGUIBuilder::createContainer( QWidget *parent, int index, const QDomElement &element, int &id )
{
  id = -1;
  if ( element.tagName().lower() == d->tagMainWindow )
  {
    KMainWindow *mainwindow = 0;
    if ( ::qt_cast<KMainWindow *>( d->m_widget ) )
      mainwindow = static_cast<KMainWindow *>(d->m_widget);

    return mainwindow;
  }

  if ( element.tagName().lower() == d->tagMenuBar )
  {
    KMenuBar *bar;

    if ( ::qt_cast<KMainWindow *>( d->m_widget ) )
      bar = static_cast<KMainWindow *>(d->m_widget)->menuBar();
    else
      bar = new KMenuBar( d->m_widget );

    bar->show();
    return bar;
  }

  if ( element.tagName().lower() == d->tagMenu )
  {
    // Look up to see if we are inside a mainwindow. If yes, then
    // use it as parent widget (to get kaction to plug itself into the
    // mainwindow). Don't use a popupmenu as parent widget, otherwise
    // the popup won't be hidden if it is used as a standalone menu as well.
    // And we don't want to set the parent for a standalone popupmenu,
    // otherwise its shortcuts appear.
    QWidget* p = parent;
    while ( p && !::qt_cast<KMainWindow *>( p ) )
        p = p->parentWidget();

    QCString name = element.attribute( d->attrName ).utf8();

    if (!kapp->authorizeKAction(name))
       return 0;

    KPopupMenu *popup = new KPopupMenu( p, name);

    QString i18nText;
    QDomElement textElem = element.namedItem( d->attrText1 ).toElement();
    if ( textElem.isNull() ) // try with capital T
      textElem = element.namedItem( d->attrText2 ).toElement();
    QCString text = textElem.text().utf8();
    QCString context = textElem.attribute(d->attrContext).utf8();

    if ( text.isEmpty() ) // still no luck
      i18nText = i18n( "No text!" );
    else if ( context.isEmpty() )
      i18nText = i18n( text );
    else
      i18nText = i18n( context, text );

    QString icon = element.attribute( d->attrIcon );
    QIconSet pix;

    if ( !icon.isEmpty() )
    {
      KInstance *instance = d->m_instance;
      if ( !instance )
        instance = KGlobal::instance();

      pix = SmallIconSet( icon, 16, instance );
    }

    if ( parent && ::qt_cast<KMenuBar *>( parent ) )
    {
      if ( !icon.isEmpty() )
        id = static_cast<KMenuBar *>(parent)->insertItem( pix, i18nText, popup, -1, index );
      else
        id = static_cast<KMenuBar *>(parent)->insertItem( i18nText, popup, -1, index );
    }
    else if ( parent && ::qt_cast<QPopupMenu *>( parent ) )
    {
      if ( !icon.isEmpty() )
        id = static_cast<QPopupMenu *>(parent)->insertItem( pix, i18nText, popup, -1, index );
      else
        id = static_cast<QPopupMenu *>(parent)->insertItem( i18nText, popup, -1, index );
    }

    return popup;
  }

  if ( element.tagName().lower() == d->tagToolBar )
  {
    bool honor = (element.attribute( d->attrName ) == "mainToolBar");

    QCString name = element.attribute( d->attrName ).utf8();

    KToolBar *bar = static_cast<KToolBar*>(d->m_widget->child( name, "KToolBar" ));
    if( !bar )
    {
       bar = new KToolBar( d->m_widget, name, honor, false );
    }

    if ( ::qt_cast<KMainWindow *>( d->m_widget ) )
    {
        if ( d->m_client && !d->m_client->xmlFile().isEmpty() )
            bar->setXMLGUIClient( d->m_client );
    }

    bar->loadState( element );

    return bar;
  }

  if ( element.tagName().lower() == d->tagStatusBar )
  {
      if ( ::qt_cast<KMainWindow *>( d->m_widget ) )
    {
      KMainWindow *mainWin = static_cast<KMainWindow *>(d->m_widget);
      mainWin->statusBar()->show();
      return mainWin->statusBar();
    }
    KStatusBar *bar = new KStatusBar( d->m_widget );
    return bar;
  }

  return 0L;
}

void KXMLGUIBuilder::removeContainer( QWidget *container, QWidget *parent, QDomElement &element, int id )
{
  // Warning parent can be 0L

  if ( ::qt_cast<QPopupMenu *>( container ) )
  {
    if ( parent )
    {
        if ( ::qt_cast<KMenuBar *>( parent ) )
            static_cast<KMenuBar *>(parent)->removeItem( id );
        else if ( ::qt_cast<QPopupMenu *>( parent ) )
            static_cast<QPopupMenu *>(parent)->removeItem( id );
    }

    delete container;
  }
  else if ( ::qt_cast<KToolBar *>( container ) )
  {
    KToolBar *tb = static_cast<KToolBar *>( container );

    tb->saveState( element );
    delete tb;
  }
  else if ( ::qt_cast<KMenuBar *>( container ) )
  {
    KMenuBar *mb = static_cast<KMenuBar *>( container );
    mb->hide();
    // Don't delete menubar - it can be reused by createContainer.
    // If you decide that you do need to delete the menubar, make
    // sure that QMainWindow::d->mb does not point to a deleted
    // menubar object.
  }
  else if ( ::qt_cast<KStatusBar *>( container ) )
  {
    if ( ::qt_cast<KMainWindow *>( d->m_widget ) )
        container->hide();
    else
      delete static_cast<KStatusBar *>(container);
  }
  else
     kdWarning() << "Unhandled container to remove : " << container->className() << endl;
}

QStringList KXMLGUIBuilder::customTags() const
{
  QStringList res;
  res << d->tagSeparator << d->tagTearOffHandle << d->tagMenuTitle;
  return res;
}

int KXMLGUIBuilder::createCustomElement( QWidget *parent, int index, const QDomElement &element )
{
  if ( element.tagName().lower() == d->tagSeparator )
  {
    if ( ::qt_cast<QPopupMenu *>( parent ) )
    {
      // Don't insert multiple separators in a row
      QPopupMenu *menu = static_cast<QPopupMenu *>(parent);
      int count = menu->count();
      if (count)
      {
         int previousId = -1;
         if ((index == -1) || (index > count))
            previousId = menu->idAt(count-1);
         else if (index > 0)
            previousId = menu->idAt(index-1);
         if (previousId != -1)
         {
            if (menu->text(previousId).isEmpty() &&
                !menu->iconSet(previousId) &&
                !menu->pixmap(previousId))
               return 0;
         }
      }
      // Don't insert a separator at the top of the menu
      if(count == 0)
        return 0;
      else
        return menu->insertSeparator( index );
    }
    else if ( ::qt_cast<QMenuBar *>( parent ) )
       return static_cast<QMenuBar *>(parent)->insertSeparator( index );
    else if ( ::qt_cast<KToolBar *>( parent ) )
    {
      KToolBar *bar = static_cast<KToolBar *>( parent );

      bool isLineSep = true;

      QDomNamedNodeMap attributes = element.attributes();
      unsigned int i = 0;
      for (; i < attributes.length(); i++ )
      {
        QDomAttr attr = attributes.item( i ).toAttr();

        if ( attr.name().lower() == d->attrLineSeparator &&
             attr.value().lower() == QString::fromLatin1("false") )
        {
          isLineSep = false;
          break;
        }
      }

      int id = KAction::getToolButtonID();

      if ( isLineSep )
          bar->insertLineSeparator( index, id );
      else
          bar->insertSeparator( index, id );

      return id;
    }
  }
  else if ( element.tagName().lower() == d->tagTearOffHandle )
  {
    if ( ::qt_cast<QPopupMenu *>( parent )  && KGlobalSettings::insertTearOffHandle())
      return static_cast<QPopupMenu *>(parent)->insertTearOffHandle( -1, index );
  }
  else if ( element.tagName().lower() == d->tagMenuTitle )
  {
    if ( ::qt_cast<KPopupMenu *>( parent ) )
    {
      QString i18nText;
      QCString text = element.text().utf8();

      if ( text.isEmpty() )
        i18nText = i18n( "No text!" );
      else
        i18nText = i18n( text );

      QString icon = element.attribute( d->attrIcon );
      QPixmap pix;

      if ( !icon.isEmpty() )
      {
        KInstance *instance = d->m_instance;
        if ( !instance )
          instance = KGlobal::instance();

        pix = SmallIcon( icon, instance );
      }

      if ( !icon.isEmpty() )
        return static_cast<KPopupMenu *>(parent)->insertTitle( pix, i18nText, -1, index );
      else
        return static_cast<KPopupMenu *>(parent)->insertTitle( i18nText, -1, index );
    }
  }
  return 0;
}

void KXMLGUIBuilder::removeCustomElement( QWidget *parent, int id )
{
  if ( ::qt_cast<QPopupMenu *>( parent ) )
    static_cast<QPopupMenu *>(parent)->removeItem( id );
  else if ( ::qt_cast<QMenuBar *>( parent ) )
    static_cast<QMenuBar *>(parent)->removeItem( id );
  else if ( ::qt_cast<KToolBar *>( parent ) )
    static_cast<KToolBar *>(parent)->removeItemDelayed( id );
}

KXMLGUIClient *KXMLGUIBuilder::builderClient() const
{
  return d->m_client;
}

void KXMLGUIBuilder::setBuilderClient( KXMLGUIClient *client )
{
  d->m_client = client;
  if ( client )
      setBuilderInstance( client->instance() );
}

KInstance *KXMLGUIBuilder::builderInstance() const
{
  return d->m_instance;
}

void KXMLGUIBuilder::setBuilderInstance( KInstance *instance )
{
  d->m_instance = instance;
}

void KXMLGUIBuilder::finalizeGUI( KXMLGUIClient * )
{
    if ( !d->m_widget || !::qt_cast<KMainWindow *>( d->m_widget ) )
        return;
#if 0
    KToolBar *toolbar = 0;
    QListIterator<KToolBar> it( ( (KMainWindow*)d->m_widget )->toolBarIterator() );
    while ( ( toolbar = it.current() ) ) {
        kdDebug() << "KXMLGUIBuilder::finalizeGUI toolbar=" << (void*)toolbar << endl;
        ++it;
        toolbar->positionYourself();
    }
#else
    static_cast<KMainWindow *>(d->m_widget)->finalizeGUI( false );
#endif
}

void KXMLGUIBuilder::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

