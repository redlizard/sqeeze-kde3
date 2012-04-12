/* This file is part of the KDE desktop environment

   Copyright (C) 2001, 2002 Michael Brade <brade@kde.org>

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

#ifndef _KDIRLISTERTEST_H_
#define _KDIRLISTERTEST_H_

#include <qwidget.h>
#include <qstring.h>

#include <kurl.h>
#include <kfileitem.h>

#include <iostream>

using namespace std;

class PrintSignals : public QObject
{
   Q_OBJECT
public:
   PrintSignals() : QObject() { }

public slots:
   void started( const KURL &url )
   {
      cout << "*** started( " << url.url().local8Bit() << " )" << endl;
   }
   void canceled() { cout << "canceled()" << endl; }
   void canceled( const KURL& url )
   {
      cout << "*** canceled( " << url.prettyURL().local8Bit() << " )" << endl;
   }
   void completed() { cout << "*** completed()" << endl; }
   void completed( const KURL& url )
   {
      cout << "*** completed( " << url.prettyURL().local8Bit() << " )" << endl;
   }
   void redirection( const KURL& url )
   {
      cout << "*** redirection( " << url.prettyURL().local8Bit() << " )" << endl;
   }
   void redirection( const KURL& src, const KURL& dest )
   {
      cout << "*** redirection( " << src.prettyURL().local8Bit() << ", "
           << dest.prettyURL().local8Bit() << " )" << endl;
   }
   void clear() { cout << "*** clear()" << endl; }
   void newItems( const KFileItemList& items )
   {
      cout << "*** newItems: " << endl;
      for ( KFileItemListIterator it( items ) ; it.current() ; ++it )
          cout << it.current() << " " << it.current()->name().local8Bit() << endl;
   }
   void deleteItem( KFileItem* item )
   {
      cout << "*** deleteItem: " << item->url().prettyURL().local8Bit() << endl;
   }
   void itemsFilteredByMime( const KFileItemList&  )
   {
      cout << "*** itemsFilteredByMime: " << endl;
      // TODO
   }
   void refreshItems( const KFileItemList&  )
   {
      cout << "*** refreshItems: " << endl;
      // TODO
   }
   void infoMessage( const QString& msg )
   { cout << "*** infoMessage: " << msg.local8Bit() << endl; }

   void percent( int percent )
   { cout << "*** percent: " << percent << endl; }

   void totalSize( KIO::filesize_t size )
   { cout << "*** totalSize: " << (long)size << endl; }

   void processedSize( KIO::filesize_t size )
   { cout << "*** processedSize: " << (long)size << endl; }

   void speed( int bytes_per_second )
   { cout << "*** speed: " << bytes_per_second << endl; }
};

class KDirListerTest : public QWidget
{
   Q_OBJECT
public:
   KDirListerTest( QWidget *parent=0, const char *name=0 );
   ~KDirListerTest();

public slots:
   void startRoot();
   void startHome();
   void startTar();
   void test();
   void completed();

private:
   KDirLister *lister;
   PrintSignals *debug;
};

#endif
