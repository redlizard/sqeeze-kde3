/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#include <dcopclient.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "KoApplication.h"
#include "KoApplicationIface.h"
#include "KoDocument.h"
#include "KoDocumentIface.h"
#include "KoMainWindow.h"
#include "KoQueryTrader.h"
#include "KoView.h"

KoApplicationIface::KoApplicationIface()
 : DCOPObject( "KoApplicationIface" )
{
}

KoApplicationIface::~KoApplicationIface()
{
}

DCOPRef KoApplicationIface::createDocument( const QString &nativeFormat )
{
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType( nativeFormat );
    if ( entry.isEmpty() )
    {
        KMessageBox::questionYesNo( 0, i18n( "Unknown KOffice MimeType %s. Check your installation." ).arg( nativeFormat ) );
        return DCOPRef();
    }
    KoDocument* doc = entry.createDoc( 0 );
    return DCOPRef( kapp->dcopClient()->appId(), doc->dcopObject()->objId() );
}

QValueList<DCOPRef> KoApplicationIface::getDocuments()
{
    QValueList<DCOPRef> lst;
    QPtrList<KoDocument> *documents = KoDocument::documentList();
    if ( documents )
    {
      QPtrListIterator<KoDocument> it( *documents );
      for (; it.current(); ++it )
        lst.append( DCOPRef( kapp->dcopClient()->appId(), it.current()->dcopObject()->objId() ) );
    }
    return lst;
}

QValueList<DCOPRef> KoApplicationIface::getViews()
{
    QValueList<DCOPRef> lst;
    QPtrList<KoDocument> *documents = KoDocument::documentList();
    if ( documents )
    {
      QPtrListIterator<KoDocument> it( *documents );
      for (; it.current(); ++it )
      {
          QPtrListIterator<KoView> itview( it.current()->views() );
          for ( ; itview.current(); ++itview )
              lst.append( DCOPRef( kapp->dcopClient()->appId(), itview.current()->dcopObject()->objId() ) );
      }
    }
    return lst;
}

QValueList<DCOPRef> KoApplicationIface::getWindows()
{
    QValueList<DCOPRef> lst;
    QPtrList<KMainWindow> *mainWindows = KMainWindow::memberList;
    if ( mainWindows )
    {
      QPtrListIterator<KMainWindow> it( *mainWindows );
      for (; it.current(); ++it )
        lst.append( DCOPRef( kapp->dcopClient()->appId(),
                             static_cast<KoMainWindow *>(it.current())->dcopObject()->objId() ) );
    }
    return lst;
}
