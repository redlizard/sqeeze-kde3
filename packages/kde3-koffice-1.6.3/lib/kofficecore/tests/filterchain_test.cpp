/* This file is part of the KDE project
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

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

#include <KoFilterChain.h>
#include <KoFilterManager.h>
#include <kinstance.h>
#include <kdebug.h>

int main( int /*argc*/, char ** /*argv*/ )
{
    KInstance instance( "filterchain_test" );  // we need an instance when using the trader
    KOffice::Graph g( "application/x-kspread" );
    g.dump();
    g.setSourceMimeType( "application/x-kword" );
    g.dump();

    KoFilterManager *manager = new KoFilterManager( 0 );
    kdDebug() << "Trying to build some filter chains..." << endl;
    QCString mimeType( "foo/bar" );
    KoFilterChain::Ptr chain = g.chain( manager, mimeType );
    if ( !chain )
        kdDebug() << "Chain for 'foo/bar' is not available, OK" << endl;
    else {
        kdError() << "Chain for 'foo/bar' is available!" << endl;
        chain->dump();
    }

    mimeType = "application/x-krita";
    chain = g.chain( manager, mimeType );
    if ( !chain )
        kdDebug() << "Chain for 'application/x-krita' is not available, OK" << endl;
    else {
        kdError() << "Chain 'application/x-krita' is available!" << endl;
        chain->dump();
    }

    mimeType = "text/x-csv";
    chain = g.chain( manager, mimeType );
    if ( !chain )
        kdError() << "Chain for 'text/x-csv' is not available!" << endl;
    else {
        kdDebug() << "Chain for 'text/x-csv' is available, OK" << endl;
        chain->dump();
    }

    // Try to find the closest KOffice part
    mimeType = "";
    chain = g.chain( manager, mimeType );
    if ( !chain )
        kdDebug() << "It was already a KOffice part, OK" << endl;
    else
        kdError() << "We really got a chain? ugh :}" << endl;

    g.setSourceMimeType( "text/x-csv" );
    mimeType = "";
    chain = g.chain( manager, mimeType );
    if ( !chain )
        kdError() << "Hmm... why didn't we find a chain?" << endl;
    else {
        kdDebug() << "Chain for 'text/x-csv' -> closest part is available ("
                  << mimeType << "), OK" << endl;
        chain->dump();
    }

    kdDebug() << "Checking mimeFilter() for Import:" << endl;
    QStringList list = KoFilterManager::mimeFilter( "application/x-kword",  KoFilterManager::Import );
    QStringList::ConstIterator it = list.begin();
    QStringList::ConstIterator end = list.end();
    for ( ; it != end; ++it )
        kdDebug() << "   " << *it << endl;
    kdDebug() << "   " << list.count() << " entries." << endl;

    kdDebug() << "Checking mimeFilter() for Export:" << endl;
    list = KoFilterManager::mimeFilter( "application/x-kword",  KoFilterManager::Export );
    it = list.begin();
    end = list.end();
    for ( ; it != end; ++it )
        kdDebug() << "   " << *it << endl;
    kdDebug() << "   " << list.count() << " entries." << endl;

    kdDebug() << "Checking KoShell's mimeFilter():" << endl;
    list = KoFilterManager::mimeFilter();
    it = list.begin();
    end = list.end();
    for ( ; it != end; ++it )
        kdDebug() << "   " << *it << endl;
    kdDebug() << "   " << list.count() << " entries." << endl;

    delete manager;
    return 0;
}
