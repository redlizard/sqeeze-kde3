/*
    This file is part of Kicker.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kapplication.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "findmenu.h"

K_EXPORT_KICKER_MENUEXT( find, FindMenu )


FindMenu::FindMenu( QWidget *parent, const char *name,
                    const QStringList &/*args*/)
  : KPanelMenu( "", parent, name )
{
}

FindMenu::~FindMenu()
{
    mConfigList.clear();
}

void FindMenu::initialize()
{
  QStringList list = KGlobal::dirs()->findAllResources( "data", "kicker/menuext/find/*.desktop", false, true );

  list.sort();

  int id = 0;

  mConfigList.clear();
  for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
    KSimpleConfig config( *it, true );
    config.setDesktopGroup();

    mConfigList.append( *it );
    QString text = config.readEntry( "Name" );

    insertItem( SmallIconSet( config.readEntry( "Icon" ) ), text, id );
    id++;
  }
}

void FindMenu::slotExec( int pos )
{
  QString app = mConfigList[ pos ];

  kapp->propagateSessionManager();

  KSimpleConfig config(app, true);
  config.setDesktopGroup();
  if (kapp && config.readEntry("Type") == "Link")
  {
    kapp->invokeBrowser(config.readEntry("URL"));
  }
  else
  {
    KApplication::startServiceByDesktopPath( app );
  }
}

#include "findmenu.moc"
