// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "historyurlitem.h"

#include <kmultipledrag.h>
#include <kurldrag.h>

HistoryURLItem::HistoryURLItem( const KURL::List &_urls, QMap<QString, QString> _metaData, bool _cut )
    : urls( _urls ), metaData( _metaData ), cut( _cut )
{
}

/* virtual */
void HistoryURLItem::write( QDataStream& stream ) const
{
    stream << QString( "url" ) << urls << metaData << (int)cut;
}

QString HistoryURLItem::text() const {
    return urls.toStringList().join( " " );
}

QMimeSource* HistoryURLItem::mimeSource() const {
    KMultipleDrag* drag = new KMultipleDrag;
    drag->addDragObject( new KURLDrag( urls, metaData ));
    // from KonqDrag (libkonq)
    QStoredDrag* cutdrag = new QStoredDrag( "application/x-kde-cutselection" );
    QByteArray a;
    QCString s ( cut ? "1" : "0" );
    a.resize( s.length() + 1 ); // trailing zero
    memcpy( a.data(), s.data(), s.length() + 1 );
    cutdrag->setEncodedData( a );
    drag->addDragObject( cutdrag );
    return drag;
}

bool HistoryURLItem::operator==( const HistoryItem& rhs) const
{
    if ( const HistoryURLItem* casted_rhs = dynamic_cast<const HistoryURLItem*>( &rhs ) ) {
        return casted_rhs->urls == urls
            && casted_rhs->metaData.count() == metaData.count()
            && qEqual( casted_rhs->metaData.begin(), casted_rhs->metaData.end(), metaData.begin())
            && casted_rhs->cut == cut;
    }
    return false;
}
