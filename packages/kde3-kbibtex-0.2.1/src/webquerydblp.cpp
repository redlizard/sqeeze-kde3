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
#include <qspinbox.h>

#include <klocale.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kurl.h>

#include <fileimporterbibtex.h>
#include "webquerydblp.h"

namespace KBibTeX
{
    WebQueryDBLP::WebQueryDBLP( QWidget* parent ): WebQuery( parent )
    {
        // nothing
    }

    WebQueryDBLP::~WebQueryDBLP()
    {
        // nothing
    }

    QString WebQueryDBLP::title()
    {
        return i18n( "DBLP" );
    }

    QString WebQueryDBLP::disclaimer()
    {
        return i18n( "DBLP - Copyright" );
    }

    QString WebQueryDBLP::disclaimerURL()
    {
        return "http://www.informatik.uni-trier.de/~ley/db/copyright.html";
    }

    void WebQueryDBLP::query( const QString& searchTerm, int numberOfResults )
    {
        WebQuery::query( searchTerm, numberOfResults );
        emit setTotalSteps( numberOfResults + 1 );

        QString term = searchTerm;
        KURL url = KURL( QString( "http://www.informatik.uni-trier.de/ley/dbbin/dblpquery.cgi?title=%1" ).arg( term.replace( "%", "%25" ).replace( " ", "+" ).replace( "?", "%3F" ).replace( "&", "%26" ) ) );
        QString tmpFile;
        if ( KIO::NetAccess::download( url, tmpFile, m_parent ) && !m_aborted )
        {
            QFile inputFile( tmpFile );
            inputFile.open( IO_ReadOnly );
            QTextStream ts( &inputFile );
            QString completeText = ts.read();
            inputFile.close();
            KIO::NetAccess::removeTempFile( tmpFile );

            emit setProgress( 1 );

            BibTeX::FileImporterBibTeX importer( FALSE );
            importer.setIgnoreComments( TRUE );
            int pos = completeText.find( "[DBLP:" );
            int maxCount = numberOfResults;
            int progress = 1;
            while ( !m_aborted && pos > -1 && ( maxCount-- ) > 0 )
            {
                QString key = "";
                pos += 6;
                while ( completeText[pos] != ']' )
                    key.append( completeText[pos++] );

                KURL keyUrl = KURL( QString( "http://dblp.uni-trier.de/rec/bibtex/%1" ).arg( key ) );
                QString keyTmpFile;
                if ( KIO::NetAccess::download( keyUrl, keyTmpFile, NULL ) && !m_aborted )
                {
                    QFile tmpInputFile( keyTmpFile );
                    tmpInputFile.open( IO_ReadOnly );
                    BibTeX::File *tmpBibFile = importer.load( &tmpInputFile );
                    tmpInputFile.close();
                    KIO::NetAccess::removeTempFile( keyTmpFile );

                    emit setProgress( ++progress );

                    /** if the data returned from DBLP contains only two entry, check if they are InCollection and Book. In this case, one is the conference article and the other is the conference proceedings. */
                    BibTeX::Entry *firstEntry = dynamic_cast<BibTeX::Entry*>( *tmpBibFile->begin() );
                    BibTeX::Entry *secondEntry = dynamic_cast<BibTeX::Entry*>( *( ++tmpBibFile->begin() ) );
                    if ( firstEntry != NULL && secondEntry != NULL && firstEntry->entryType() == BibTeX::Entry::etInCollection && secondEntry->entryType() == BibTeX::Entry::etBook )
                    {
                        /** We merge both conference article and proceedings, as otherwise 2*n entries will be shown in the search result and it is no longer clear, which conference and with article match. */
                        BibTeX::Entry *myEntry = tmpBibFile->completeReferencedFieldsConst( firstEntry );
                        myEntry->merge( secondEntry, FALSE );
                        myEntry->deleteField( BibTeX::EntryField::ftCrossRef );
                        emit foundEntry( myEntry );
                    }
                    else
                        for ( BibTeX::File::ElementList::iterator it = tmpBibFile->begin(); it != tmpBibFile->end(); ++it )
                        {
                            BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *it );
                            if ( entry != NULL )
                                emit foundEntry( new BibTeX::Entry( entry ) );
                        }

                    delete tmpBibFile;
                }

                pos = completeText.find( "[DBLP:", pos + 1 );
            }

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
