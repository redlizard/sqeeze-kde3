/* This file is part of the KDE project
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kdevcoderepository.h"

struct KDevCodeRepositoryData
{
    Catalog* mainCatalog;
    QValueList<Catalog*> catalogs;
    
    KDevCodeRepositoryData(): mainCatalog( 0 ) {}
};

KDevCodeRepository::KDevCodeRepository( )
    : d( new KDevCodeRepositoryData )
{
}

KDevCodeRepository::~KDevCodeRepository( )
{
    delete( d );
}

Catalog* KDevCodeRepository::mainCatalog()
{
    return d->mainCatalog;
}

void KDevCodeRepository::setMainCatalog( Catalog * mainCatalog )
{
    d->mainCatalog = mainCatalog;
}

void KDevCodeRepository::registerCatalog( Catalog * catalog )
{
    d->catalogs.append( catalog );
    emit catalogRegistered( catalog );
}

void KDevCodeRepository::unregisterCatalog( Catalog * catalog )
{
    d->catalogs.remove( catalog );
    emit catalogUnregistered( catalog );
}

void KDevCodeRepository::touchCatalog( Catalog * catalog )
{
    emit catalogChanged( catalog );
}

QValueList< Catalog * > KDevCodeRepository::registeredCatalogs( )
{
    return d->catalogs;
}

#include "kdevcoderepository.moc"
