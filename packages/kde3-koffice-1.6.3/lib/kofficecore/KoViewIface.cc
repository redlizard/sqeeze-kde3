/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

   $Id: KoViewIface.cc 508787 2006-02-12 18:28:12Z ingwa $

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
#include "KoViewIface.h"

#include "KoView.h"

#include <kapplication.h>
#include <dcopclient.h>
#include <kdcopactionproxy.h>
#include <kaction.h>

//static
QCString KoViewIface::newIfaceName()
{
    static int s_viewIFNumber = 0;
    QCString name; name.setNum( s_viewIFNumber++ ); name.prepend("View-");
    return name;
}

KoViewIface::KoViewIface( KoView *view )
    : DCOPObject( newIfaceName() )
{
    m_actionProxy = new KDCOPActionProxy( view->actionCollection(), this );
}

KoViewIface::KoViewIface( const char *name, KoView *view )
    : DCOPObject( name )
{
    m_pView = view;
    m_actionProxy = new KDCOPActionProxy( view->actionCollection(), this );
}

KoViewIface::~KoViewIface()
{
    delete m_actionProxy;
}

DCOPRef KoViewIface::action( const QCString &name )
{
    return DCOPRef( kapp->dcopClient()->appId(), m_actionProxy->actionObjectId( name ) );
}

QCStringList KoViewIface::actions()
{
    QCStringList res;
    QValueList<KAction *> lst = m_actionProxy->actions();
    QValueList<KAction *>::ConstIterator it = lst.begin();
    QValueList<KAction *>::ConstIterator end = lst.end();
    for (; it != end; ++it )
        res.append( (*it)->name() );

    return res;
}

QMap<QCString,DCOPRef> KoViewIface::actionMap()
{
    return m_actionProxy->actionMap();
}
