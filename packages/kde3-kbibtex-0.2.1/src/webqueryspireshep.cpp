/***************************************************************************
 *   Copyright (C) 2004-2005 by Thomas Fischer                             *
 *   fischer@unix-ag.uni-kl.de                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qfile.h>
#include <qapplication.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kurl.h>

#include <fileimporterbibtex.h>
#include "webqueryspireshep.h"

namespace KBibTeX
{

    const QString WebQuerySpiresHep::mirrorNames[] = {i18n( "DESY (Germany)" ), i18n( "FNAL (U.S.A.)" ), i18n( "IHEP (Russia)" ), i18n( "Durham U (U.K.)" ), i18n( "SLAC (U.S.A.)" ), i18n( "YITP (Japan)" ), i18n( "LIPI (Indonesia)" )};
    const QString WebQuerySpiresHep::mirrorURLs[] = {"http://www-library.desy.de/cgi-bin/spiface", "http://www-spires.fnal.gov/spires", "http://usparc.ihep.su/spires", "http://www-spires.dur.ac.uk/spires", "http://www.slac.stanford.edu/spires", "http://www.yukawa.kyoto-u.ac.jp/spires", "http://www.spires.lipi.go.id/spires"};

    WebQuerySpiresHep::WebQuerySpiresHep( QWidget* parent ): WebQuery( parent )
    {
        // nothing
    }

    WebQuerySpiresHep::~WebQuerySpiresHep()
    {
        // nothing
    }

    QString WebQuerySpiresHep::title()
    {
        return i18n( "SPIRES-HEP" );
    }

    QString WebQuerySpiresHep::disclaimer()
    {
        return i18n( "SPIRES-HEP Search" );
    }

    QString WebQuerySpiresHep::disclaimerURL()
    {
        return "http://www.slac.stanford.edu/spires/";
    }


    void WebQuerySpiresHep::query( const QString& searchTerm, int numberOfResults )
    {
        WebQuery::query( searchTerm, numberOfResults );

        emit setTotalSteps( 1 );

        QString term = searchTerm;
        KURL url = KURL( QString( "%2/find/hep/www?rawcmd=%1&FORMAT=WWWBRIEFBIBTEX&SEQUENCE=" ).arg( term.replace( "%", "%25" ).replace( " ", "+" ).replace( "?", "%3F" ).replace( "&", "%26" ) ).arg( mirrorURLs[0] ) );
        QString tmpFile;
        if ( KIO::NetAccess::download( url, tmpFile, m_parent ) && !m_aborted )
        {
            BibTeX::FileImporterBibTeX importer( FALSE );
            importer.setIgnoreComments( TRUE );
            QFile inputFile( tmpFile );
            inputFile.open( IO_ReadOnly );
            BibTeX::File *tmpBibFile = importer.load( &inputFile );
            inputFile.close();
            KIO::NetAccess::removeTempFile( tmpFile );

            emit setProgress( 1 );

            for ( BibTeX::File::ElementList::iterator it = tmpBibFile->begin(); it != tmpBibFile->end(); ++it )
            {
                BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *it );
                if ( entry != NULL )
                    emit foundEntry( new BibTeX::Entry( entry ) );
            }

            delete tmpBibFile;
            emit endSearch( false );
        }
        else if ( !m_aborted )
        {
            QString message = KIO::NetAccess::lastErrorString();
            if ( message.isEmpty() )
                message.prepend( '\n' );
            message.prepend( QString( i18n( "Querying database '%1' failed." ) ).arg( title() ) );
            KMessageBox::error( m_parent, message );
            emit endSearch( true );
        }
        else
            emit endSearch( false );
    }

}
