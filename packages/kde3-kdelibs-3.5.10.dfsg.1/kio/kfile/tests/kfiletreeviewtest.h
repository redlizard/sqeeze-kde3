/* This file is part of the KDE libraries
    Copyright (C) 2001 Klaas Freitag <freitag@suse.de>

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

#ifndef KFILETREEVIEWTEST
#define KFILETREEVIEWTEST

class testFrame: public KMainWindow
{
   Q_OBJECT
public:
   testFrame();
   void showPath( const KURL & );
   void setDirOnly( ) { dirOnlyMode = true; }
public slots:
   void slotPopulateFinished(KFileTreeViewItem *);
   void slotSetChildCount( KFileTreeViewItem *item, int c );

   void urlsDropped( QWidget*, QDropEvent*, KURL::List& );
   void copyURLs( KURL::List& list, KURL& to ); 
private:
   KFileTreeView *treeView;
   bool          dirOnlyMode;
};


#endif
