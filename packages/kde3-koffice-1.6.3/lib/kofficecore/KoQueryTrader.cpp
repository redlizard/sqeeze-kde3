/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include <kparts/factory.h>

#include <KoQueryTrader.h>
#include <KoDocument.h>
#include <KoFilter.h>
#include <ktrader.h>
#include <kservicetype.h>
#include <kdebug.h>

#include <qfile.h>

#include <limits.h> // UINT_MAX

/**
 * Port from KOffice Trader to KTrader/KActivator (kded) by Simon Hausmann
 * (c) 1999 Simon Hausmann <hausmann@kde.org>
 * Port to KService and simplifications by David Faure <faure@kde.org>
 */


/*******************************************************************
 *
 * KoDocumentEntry
 *
 *******************************************************************/

KoDocumentEntry::KoDocumentEntry( KService::Ptr service )
  : m_service( service )
{
}

KoDocument* KoDocumentEntry::createDoc( KoDocument* parent, const char* name ) const
{
    KLibFactory* factory = KLibLoader::self()->factory( QFile::encodeName(m_service->library()) );

    if( !factory ) {
        kdWarning(30003) << KLibLoader::self()->lastErrorMessage() << endl;
        return 0;
    }

    QObject* obj;
    if ( factory->inherits( "KParts::Factory" ) )
      obj = static_cast<KParts::Factory *>(factory)->createPart( 0L, "", parent, name, "KoDocument" );
    else {
      kdWarning(30003) << "factory doesn't inherit KParts::Factory ! It is a " << factory->className() << endl; // This shouldn't happen...
      obj = factory->create( parent, name, "KoDocument" );
    }

    if ( !obj || !obj->inherits( "KoDocument" ) )
    {
        delete obj;
        return 0;
    }

    return static_cast<KoDocument*>(obj);
}

KoDocumentEntry KoDocumentEntry::queryByMimeType( const QString & mimetype )
{
  QString constr = QString::fromLatin1( "[X-KDE-NativeMimeType] == '%1' or '%2' in [X-KDE-ExtraNativeMimeTypes]" ).arg( mimetype ).arg( mimetype );

  QValueList<KoDocumentEntry> vec = query( false,constr );
  if ( vec.isEmpty() )
  {
    kdWarning(30003) << "Got no results with " << constr << endl;
    // Fallback to the old way (which was probably wrong, but better be safe)
    QString constr = QString::fromLatin1( "'%1' in ServiceTypes" ).arg( mimetype );
    vec = query( constr );
    if ( vec.isEmpty() )
    {
      // Still no match. Either the mimetype itself is unknown, or we have no service for it.
      // Help the user debugging stuff by providing some more diagnostics
      if ( KServiceType::serviceType( mimetype ) == 0L )
      {
        kdError(30003) << "Unknown KOffice MimeType " << mimetype << "." << endl;
        kdError(30003) << "Check your installation (for instance, run 'kde-config --path mime' and check the result)." << endl;
      } else
      {
        kdError(30003) << "Found no KOffice part able to handle " << mimetype << "!" << endl;
        kdError(30003) << "Check your installation (does the desktop file have X-KDE-NativeMimeType and KOfficePart, did you install KOffice in a different prefix than KDE, without adding the prefix to /etc/kderc ?)" << endl;
      }
      return KoDocumentEntry();
    }
  }

  return vec[0];
}

QValueList<KoDocumentEntry> KoDocumentEntry::query( const QString & _constr )
{
  return query(true,_constr);
}

QValueList<KoDocumentEntry> KoDocumentEntry::query( bool _onlyDocEmb, const QString & _constr )
{

  QValueList<KoDocumentEntry> lst;
  QString constr;
  if ( !_constr.isEmpty() ) {
      constr = "(";
      constr += _constr;
      constr += ") and ";
  }
  constr += " exist Library";

  // Query the trader
  KTrader::OfferList offers = KTrader::self()->query( "KOfficePart", constr );

  KTrader::OfferList::ConstIterator it = offers.begin();
  unsigned int max = offers.count();
  for( unsigned int i = 0; i < max; i++, ++it )
  {
    //kdDebug(30003) << "   desktopEntryPath=" << (*it)->desktopEntryPath()
    //               << "   library=" << (*it)->library() << endl;

    if ( (*it)->noDisplay() )
        continue;
    // Maybe this could be done as a trader constraint too.
    if ( (!_onlyDocEmb) || ((*it)->property("X-KDE-NOTKoDocumentEmbeddable").toString()!="1") )
    {
      KoDocumentEntry d( *it );
      // Append converted offer
      lst.append( d );
      // Next service
    }
  }

  if ( lst.count() > 1 && !_constr.isEmpty() )
    kdWarning(30003) << "KoDocumentEntry::query " << constr << " got " << max << " offers!" << endl;

  return lst;
}




/*******************************************************************
 *
 * KoFilterEntry
 *
 *******************************************************************/

KoFilterEntry::KoFilterEntry( KService::Ptr service )
  : m_service( service )
{
  import = service->property( "X-KDE-Import" ).toStringList();
  export_ = service->property( "X-KDE-Export" ).toStringList();
  int w = service->property( "X-KDE-Weight" ).toInt();
  weight = w < 0 ? UINT_MAX : static_cast<unsigned int>( w );
  available = service->property( "X-KDE-Available" ).toString();
}

QValueList<KoFilterEntry::Ptr> KoFilterEntry::query( const QString & _constr )
{
  kdDebug(30500) << "KoFilterEntry::query( " << _constr << " )" << endl;
  QValueList<KoFilterEntry::Ptr> lst;

  KTrader::OfferList offers = KTrader::self()->query( "KOfficeFilter", _constr );

  KTrader::OfferList::ConstIterator it = offers.begin();
  unsigned int max = offers.count();
  //kdDebug(30500) << "Query returned " << max << " offers" << endl;
  for( unsigned int i = 0; i < max; i++ )
  {
    //kdDebug(30500) << "   desktopEntryPath=" << (*it)->desktopEntryPath()
    //               << "   library=" << (*it)->library() << endl;
    // Append converted offer
    lst.append( new KoFilterEntry( *it ) );
    // Next service
    it++;
  }

  return lst;
}

KoFilter* KoFilterEntry::createFilter( KoFilterChain* chain, QObject* parent, const char* name )
{
    KLibFactory* factory = KLibLoader::self()->factory( QFile::encodeName( m_service->library() ) );

    if ( !factory ) {
        kdWarning(30003) << KLibLoader::self()->lastErrorMessage() << endl;
        return 0;
    }

    QObject* obj = factory->create( parent, name, "KoFilter" );
    if ( !obj || !obj->inherits( "KoFilter" ) )
    {
        delete obj;
        return 0;
    }

    KoFilter* filter = static_cast<KoFilter*>( obj );
    filter->m_chain = chain;
    return filter;
}
