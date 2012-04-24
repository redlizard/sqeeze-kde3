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
#include <qregexp.h>
#include <qbuffer.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>

#include <fileimporterbibtex.h>
#include <encoderxml.h>
#include <settings.h>
#include "webqueryarxiv.h"

namespace KBibTeX
{

    WebQueryArXiv::WebQueryArXiv( QWidget* parent ): WebQuery( parent ), m_arXivServer( "www.arxiv.org" )
    {
        m_importer = new BibTeX::FileImporterBibTeX( FALSE );
        m_importer->setIgnoreComments( TRUE );
    }

    WebQueryArXiv::~WebQueryArXiv()
    {
        delete m_importer;
    }

    QString WebQueryArXiv::title()
    {
        return i18n( "arXiv" );
    }

    QString WebQueryArXiv::disclaimer()
    {
        return i18n( "arXiv is an archive for electronic preprints" );
    }

    QString WebQueryArXiv::disclaimerURL()
    {
        return "http://www.arxiv.org/";
    }

    void WebQueryArXiv::cancelQuery()
    {
        m_urls.clear();
        // FIXME: The following code crashes KBibTeX:
        // if ( m_currentJob != NULL ) m_currentJob->kill( FALSE );
    }

    void WebQueryArXiv::query( const QString& searchTerm, int numberOfResults )
    {
        m_urls.clear();
        WebQuery::query( searchTerm, numberOfResults );

        emit setTotalSteps( numberOfResults + 1 );
        m_numberOfResults = numberOfResults;
        QStringList queryWords = QStringList::split( QRegExp( "\\s+" ), searchTerm );
        if ( queryWords.size() == 0 )
        {
            emit setProgress( numberOfResults + 1 );
            emit endSearch( false );
            return;
        }

        QString query;
        for ( unsigned int i = 0; i < queryWords.size() - 1; ++i )
            query = query.append( "AND " ).append( queryWords[i] ).append( " " );
        query.append( queryWords[queryWords.size()-1] );

        KURL url = KURL( QString( "http://www.arxiv.org/find/all/1/all:+%2/0/1/0/all/0/1?per_page=%1" ).arg( numberOfResults ).arg( query.replace( "%", "%25" ).replace( " ", "+" ).replace( "?", "%3F" ).replace( "&", "%26" ) ) );
        m_currentJob = KIO::storedGet( url, FALSE, FALSE );
        connect( m_currentJob, SIGNAL( result( KIO::Job * ) ), this, SLOT( arXivResult( KIO::Job * ) ) );
    }

    void WebQueryArXiv::arXivResult( KIO::Job *job )
    {
        m_currentJob = NULL;

        if ( job->error() == 0 && !m_aborted )
        {
            emit setProgress( 1 );

            QBuffer data;
            data.open( IO_WriteOnly );
            data.writeBlock( dynamic_cast<KIO::StoredTransferJob*>( job )->data() );
            data.close();
            data.open( IO_ReadOnly );
            QTextStream ts( &data );
            QString result = ts.read();
            data.close();

            int p = -1;
            m_totalHits = 0;
            m_receivedHits = 0;
            while ( !m_aborted && ( int ) m_totalHits < m_numberOfResults && ( p = result.find( "arXiv:", p + 1 ) ) >= 0 )
            {
                int p2 = result.find( "<", p + 2 );
                QString hit = result.mid( p + 6, p2 - p - 6 );
                ++m_totalHits;
                p = p2 + 1;


                KURL url = KURL( QString( "http://%2/abs/%1" ).arg( hit ).arg( m_arXivServer ) );
                m_urls.append( url );
            }

            if ( m_totalHits == 0 )
                emit endSearch( false );
            else if ( !m_urls.isEmpty() )
            {
                KURL url = m_urls.first();
                m_urls.remove( url );
                m_currentJob = KIO::storedGet( url, FALSE, FALSE );
                connect( m_currentJob, SIGNAL( result( KIO::Job * ) ), this, SLOT( arXivAbstractResult( KIO::Job * ) ) );
            }
        }
        else
            emit endSearch( true );
    }

    void WebQueryArXiv::arXivAbstractResult( KIO::Job *job )
    {
        m_currentJob = NULL;

        if ( job->error() == 0 && !m_aborted )
        {
            ++m_receivedHits;
            emit setProgress( m_receivedHits + 1 );

            QBuffer data;
            data.open( IO_WriteOnly );
            data.writeBlock( dynamic_cast<KIO::StoredTransferJob*>( job )->data() );
            data.close();
            data.open( IO_ReadOnly );
            QTextStream ts( &data );
            QString result = BibTeX::EncoderXML::currentEncoderXML()->decode( ts.read() );
            data.close();

            /** find id */
            int p = result.find( "arXiv:", 0 );
            if ( p < 0 ) return;
            int p2 = result.find( "<", p + 2 );
            if ( p2 < 0 ) return;
            QString id = result.mid( p + 6, p2 - p - 6 );

            /** find cite_as */
            QString citeas = "";
            p = result.find( "Cite&nbsp;as", 0 );
            p = result.find( ">arXiv:", p );
            p2 = result.find( "</", p );
            if ( p >= 0 && p2 >= 0 )
                citeas = result.mid( p + 7, p2 - p - 7 );

            BibTeX::Entry * entry = new BibTeX::Entry( BibTeX::Entry::etMisc, citeas.isEmpty() ? ( id.isEmpty() ? QString( "arXiv" ).append( m_receivedHits ) : id ) : citeas );

            /** find abstract */
            p = result.find( "Abstract:</span>", 0 );
            if ( p < 0 ) return;
            p2 = result.find( "</blockq", p + 14 );
            if ( p2 < 0 ) return;
            QString abstract = result.mid( p + 16, p2 - p - 16 ).replace( QRegExp( "\\s+" ), " " ).replace( QRegExp( "^\\s+|\\s+$" ), "" ).replace( QRegExp( "<[^>]+>" ), "" );
            BibTeX::EntryField * field = new BibTeX::EntryField( BibTeX::EntryField::ftAbstract );
            entry->addField( field );
            field->setValue( new BibTeX::Value( abstract ) );

            /** find authors */
            BibTeX::PersonContainer *personContainer = new BibTeX::PersonContainer( Settings::self()->editing_FirstNameFirst );
            p = -1;
            while (( p = result.find( "/au:", p + 1 ) ) > 0 )
            {
                p = result.find( ">", p + 1 );
                p2 = result.find( "<", p + 1 );
                QString author = result.mid( p + 1, p2 - p - 1 );
                personContainer->persons.append( new BibTeX::Person( author ) );
            }
            if ( personContainer->persons.isEmpty() )
                delete personContainer;
            else
            {
                BibTeX::EntryField * field = new BibTeX::EntryField( BibTeX::EntryField::ftAuthor );
                entry->addField( field );
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( personContainer );
                field->setValue( value );
            }

            /** find title */
            p = result.find( "Title:</span>", 0 );
            p2 = result.find( "<", p + 10 );
            if ( p >= 0 && p2 >= 0 )
            {
                QString title = result.mid( p + 13, p2 - p - 13 ).replace( QRegExp( "\\s+" ), " " ).replace( QRegExp( "^\\s+|\\s+$" ), "" );
                BibTeX::EntryField * field  = new BibTeX::EntryField( BibTeX::EntryField::ftTitle );
                entry->addField( field );
                field->setValue( new BibTeX::Value( title ) );
            }

            /** find month and year */
            p = result.find( "Submitted on", 0 );
            while (( p2 = result.find( "last revised", p + 1 ) ) >= 0 )
                p = p2;
            p2 = result.find( QRegExp( "\\d\\d\\d\\d" ), p );
            bool ok = FALSE;
            int year = result.mid( p2, 4 ).toInt( &ok );
            if ( !ok ) year = 0;
            if ( year > 1000 )
            {
                BibTeX::EntryField * field = new BibTeX::EntryField( BibTeX::EntryField::ftYear );
                entry->addField( field );
                field->setValue( new BibTeX::Value( QString::number( year ) ) );
            }
            p2 = result.find( QRegExp( "\\b[A-Z][a-z]{2}\\b" ), p );
            if ( p2 >= 0 )
            {
                QString month = result.mid( p2, 3 ).lower();
                BibTeX::EntryField * field = new BibTeX::EntryField( BibTeX::EntryField::ftMonth );
                entry->addField( field );
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::MacroKey( month ) );
                field->setValue( value );
            }

            /** find DOI */
            p = result.find( "http://dx.doi.org/", 0 );
            p2 = result.find( "\"", p + 1 );
            if ( p >= 0 && p2 >= 0 )
            {
                QString doi = result.mid( p, p2 - p );
                BibTeX::EntryField * field = new BibTeX::EntryField( BibTeX::EntryField::ftDoi );
                entry->addField( field );
                field->setValue( new BibTeX::Value( doi ) );
            }

            /** find keywords */
            p = result.find( "<td class=\"tablecell subjects\">", 0 );
            p2 = result.find( "</td>", p + 1 );
            if ( p >= 0 && p2 >= 0 )
            {
                QString keywords = result.mid( p + 31, p2 - p - 31 ).replace( QRegExp( "</?span[^>]*>" ), "" );
                BibTeX::EntryField * field = new BibTeX::EntryField( BibTeX::EntryField::ftKeywords );
                entry->addField( field );
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::KeywordContainer( keywords ) );
                field->setValue( value );
            }

            /** find ACM classes */
            p = result.find( "<td class=\"tablecell acm-classes\">", 0 );
            p2 = result.find( "</td>", p + 1 );
            if ( p >= 0 && p2 >= 0 )
            {
                QString acmclasses = result.mid( p + 34, p2 - p - 34 );
                BibTeX::EntryField * field = new BibTeX::EntryField( "acm-classes" );
                entry->addField( field );
                field->setValue( new BibTeX::Value( acmclasses ) );
            }

            /** find versions */
            for ( int v = 1; !m_aborted && v < 20; ++v )
            {
                p = result.find( QString( ">[v%1]<" ).arg( v ), 0 );
                if ( p < 0 ) break;

                int p3 = result.findRev( "href=\"", p );
                if ( p3 >= 0 && p3 > p - 40 )
                {
                    p2 = result.find( "\">", p3 );
                    if ( p2 >= 0 )
                    {
                        QString url = result.mid( p3 + 6, p2 - p3 - 6 );
                        BibTeX::EntryField * field = new BibTeX::EntryField( QString( "v%1url" ).arg( v ) );
                        entry->addField( field );
                        field->setValue( new BibTeX::Value( QString( "http://www.arxiv.org" ).append( url ) ) );
                    }
                }
                p = result.find( "</b>", p + 1 );
                p2 = result.find( "<br", p + 1 );
                if ( p >= 0 && p2 >= 0 )
                {
                    QString version = result.mid( p + 5, p2 - p - 5 );
                    BibTeX::EntryField * field = new BibTeX::EntryField( QString( "v%1descr" ).arg( v ) );
                    entry->addField( field );
                    field->setValue( new BibTeX::Value( version ) );
                }
            }

            /** find journal reference */
            int fromPage = 0;
            int toPage = 0;
            p = result.find( "<td class=\"tablecell jref\">", 0 );
            p2 = result.find( "</td>", p + 1 );
            if ( p >= 0 && p2 >= 0 )
            {
                QString jref = result.mid( p + 27, p2 - p - 27 );
                BibTeX::EntryField * field = new BibTeX::EntryField( BibTeX::EntryField::ftJournal );
                entry->addField( field );
                field->setValue( new BibTeX::Value( jref ) );
                entry->setEntryType( BibTeX::Entry::etArticle );

                p = jref.find( "[0-9]+[-][0-9]+", 0 );
                if ( p >= 0 )
                {
                    p2 = jref.find( "-", p );
                    bool ok = FALSE;
                    fromPage = jref.mid( p, p2 - p ).toInt( &ok );
                    if ( !ok ) fromPage = 0;
                    p = jref.find( QRegExp( "\\b|$" ), p2 + 1 );
                    toPage = jref.mid( p2 + 1, p - p2 - 1 ).toInt( &ok );
                    if ( !ok ) toPage = 0;
                    if ( fromPage > 0 && toPage > 0 && ok )
                    {
                        BibTeX::EntryField * field  = new BibTeX::EntryField( BibTeX::EntryField::ftPages );
                        entry->addField( field );
                        field->setValue( new BibTeX::Value( QString( "%1--%2" ).arg( fromPage ).arg( toPage ) ) );
                    }
                }
            }

            /** find tech report reference */
            p = result.find( "<td class=\"tablecell report-number\">", 0 );
            p2 = result.find( "</td>", p + 1 );
            if ( p >= 0 && p2 >= 0 )
            {
                QString techRepNr = result.mid( p + 36, p2 - p - 36 );
                BibTeX::EntryField * field = new BibTeX::EntryField( BibTeX::EntryField::ftNumber );
                entry->addField( field );
                field->setValue( new BibTeX::Value( techRepNr ) );
                entry->setEntryType( BibTeX::Entry::etTechReport );
            }

            if ( result.find( QRegExp( "Ph\\.?D\\.? Thesis", FALSE ), 0 ) >= 0 )
                entry->setEntryType( BibTeX::Entry::etPhDThesis );

            field = new BibTeX::EntryField( BibTeX::EntryField::ftURL );
            entry->addField( field );
            field->setValue( new BibTeX::Value( QString( "http://arxiv.org/abs/%1" ).arg( id ) ) );

            field = new BibTeX::EntryField( "pdf" );
            entry->addField( field );
            field->setValue( new BibTeX::Value( QString( "http://arxiv.org/pdf/%1" ).arg( id ) ) );

            emit foundEntry( entry );

            if ( m_totalHits == m_receivedHits )
                emit endSearch( false );
            else if ( !m_urls.isEmpty() )
            {
                KURL url = m_urls.first();
                m_urls.remove( url );
                m_currentJob = KIO::storedGet( url, FALSE, FALSE );
                connect( m_currentJob, SIGNAL( result( KIO::Job * ) ), this, SLOT( arXivAbstractResult( KIO::Job * ) ) );
            }
        }
        else
            emit endSearch( true );
    }

}
#include "webqueryarxiv.moc"
