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
#include <qregexp.h>
#include <qbuffer.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kurl.h>

#include <fileimporterbibtex.h>
#include "webqueryamatex.h"

namespace KBibTeX
{
    WebQueryAmatex::WebQueryAmatex( QWidget* parent ): WebQuery( parent )
    {
        // nothing
    }

    WebQueryAmatex::~WebQueryAmatex()
    {
        // nothing
    }

    QString WebQueryAmatex::title()
    {
        return i18n( "Amatex" );
    }

    QString WebQueryAmatex::disclaimer()
    {
        return i18n( "About Amatex" );
    }

    QString WebQueryAmatex::disclaimerURL()
    {
        return QString( "http://www.2ndminute.org:8080/amatex/pages/about.jsp" );
    }

    void WebQueryAmatex::query( const QString& searchTerm, int numberOfResults )
    {
        WebQuery::query( searchTerm, numberOfResults );

        emit setTotalSteps( numberOfResults + 1 );
        QString term = searchTerm;
        KURL url = KURL( QString( "http://www.2ndminute.org:8080/amatex/search.do?lang=US&querry=%1&suchart=kwd" ).arg( term.replace( "%", "%25" ).replace( " ", "+" ).replace( "?", "%3F" ).replace( "&", "%26" ) ) );

        QString key = term.replace( QRegExp( "[^A-Za-z0-9]" ), "" );
        int count = 0;
        QString tmpFile;
        if ( KIO::NetAccess::download( url, tmpFile, m_parent ) && !m_aborted )
        {
            QFile inputFile( tmpFile );
            inputFile.open( IO_ReadOnly );
            QTextStream ts( &inputFile );
            QString completeText = ts.read();
            inputFile.close();
            KIO::NetAccess::removeTempFile( tmpFile );
            completeText = completeText.replace( "<br>", "\n" ).replace( QRegExp( "<[^>]+>" ), "" );
            KIO::NetAccess::removeTempFile( tmpFile );

            BibTeX::FileImporterBibTeX importer( FALSE );
            importer.setIgnoreComments( TRUE );
            QBuffer buffer( completeText.utf8() );
            buffer.open( IO_ReadOnly );
            BibTeX::File *tmpBibFile = importer.load( &buffer );
            buffer.close();
            setProgress( 1 );

            for ( BibTeX::File::ElementList::iterator it = tmpBibFile->begin(); !m_aborted && count < numberOfResults && it != tmpBibFile->end(); ++it )
            {
                BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *it );
                if ( entry != NULL )
                {
                    entry->setId( QString( "Amatex_%1_%2" ).arg( key ).arg( ++count ) );
                    emit foundEntry( entry );
                    setProgress( count + 1 );
                }
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
