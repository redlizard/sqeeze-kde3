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
#include <qmap.h>
#include <qbuffer.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "webquerygooglescholar.h"

namespace KBibTeX
{
    WebQueryGoogleScholar::WebQueryGoogleScholar( QWidget* parent ): WebQuery( parent ), m_http( NULL ), m_buffer( NULL )
    {
        m_importer = new BibTeX::FileImporterBibTeX( FALSE );
        m_importer->setIgnoreComments( TRUE );
    }

    WebQueryGoogleScholar::~WebQueryGoogleScholar()
    {
        delete m_importer;
        if ( m_buffer != NULL ) delete m_buffer;
        if ( m_http != NULL ) delete m_http;
    }

    QString WebQueryGoogleScholar::title()
    {
        return i18n( "Google Scholar" );
    }

    QString WebQueryGoogleScholar::disclaimer()
    {
        return i18n( "About Google Scholar" );
    }

    QString WebQueryGoogleScholar::disclaimerURL()
    {
        return "http://scholar.google.com/intl/en/scholar/about.html";
    }

    void WebQueryGoogleScholar::query( const QString& searchTerm, int numberOfResults )
    {
        WebQuery::query( searchTerm, numberOfResults );

        m_searchTerm = searchTerm;
        m_numberOfResults = numberOfResults;
        m_currentRequestNumber = 0;
        emit setTotalSteps( numberOfResults + 7 );

        m_http = new QHttp();
        connect( m_http, SIGNAL( done( bool ) ), this, SLOT( done( bool ) ) );
        connect( m_http, SIGNAL( responseHeaderReceived( const QHttpResponseHeader & ) ), this, SLOT( headerReceived( const QHttpResponseHeader & ) ) );

        m_state = gsInit;
        m_referer = QUrl( "http://www.google.com/" );
        QUrl url( "http://scholar.google.com/" );
        startRequest( url );
    }

    void WebQueryGoogleScholar::cancelQuery()
    {
        if ( m_http != NULL )
        {
            qDebug( "state= %d", m_http->state() );
            m_http->abort();
            emit endSearch( true );
        }
    }

    void WebQueryGoogleScholar::done( bool error )
    {
        m_buffer->close();

        if ( m_aborted )
            emit endSearch( false );
        else if ( error )
        {
            KMessageBox::error( m_parent, QString( i18n( "Querying database '%1' failed." ) ).arg( title() ) );
            emit endSearch( true );
        }
        else
        {
            emit setProgress( ++m_currentRequestNumber );

            if ( !m_redirectLocation.isEmpty() )
            {
                QUrl url( m_redirectLocation );
                startRequest( url );
            }
            else if ( m_state == gsInit &&  m_referer.host().contains( "scholar." ) )
            {
                m_state = gsGetPref;
                QUrl url = QUrl( QString( "http://%1/scholar_preferences" ).arg( m_referer.host() ) );
                startRequest( url );
            }
            else if ( m_state == gsGetPref )
            {
                m_state = gsSetPref;
                QUrl url = QUrl( QString( "http://%1/scholar_setprefs?inststart=0&hl=en&lang=all&instq=&inst=cat-oclc&num=%2&scis=yes&scisf=4&submit=Save+Preferences" ).arg( m_referer.host() ).arg( m_numberOfResults ) );
                startRequest( url );
            }
            else if ( m_state == gsSetPref )
            {
                m_state = gsQuery;
                QUrl url = QUrl( QString( "http://%1/scholar?q=%2&hl=en&lr=&num=%3&ie=UTF-8&btnG=Search&lr=" ).arg( m_referer.host() ).arg( m_searchTerm.replace( "%", "%25" ).replace( " ", "+" ).replace( "?", "%3F" ).replace( "&", "%26" ) ).arg( m_numberOfResults ) );
                startRequest( url );
            }
            else if ( m_state == gsQuery )
            {
                m_state = gsResult;
                m_buffer->open( IO_ReadOnly );
                QTextStream ts( m_buffer );
                QString html = ts.read();
                m_buffer->close();

                int p = 0;
                while (( p = html.find( "\"/scholar.bib?", p ) ) >= 0 )
                {
                    int p2 = html.find( "\"", p + 2 );
                    QString hit = html.mid( p + 1, p2 - p - 1 );
                    m_hits.append( hit );
                    p = p2;
                }

                if ( m_hits.size() > 0 )
                {
                    QString hit = *m_hits.begin();
                    m_hits.remove( m_hits.begin() );

                    QUrl url = QUrl( QString( "http://%1%2" ).arg( m_referer.host() ).arg( hit ) );
                    startRequest( url );
                }
                else
                    emit endSearch( false );
            }
            else if ( m_state == gsResult )
            {
                m_buffer->open( IO_ReadOnly );
                BibTeX::File *tmpBibFile = m_importer->load( m_buffer );
                for ( BibTeX::File::ElementList::iterator it = tmpBibFile->begin(); it != tmpBibFile->end(); ++it )
                {
                    BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *it );
                    if ( entry != NULL )
                        emit foundEntry( new BibTeX::Entry( entry ) );
                }
                delete tmpBibFile;
                m_buffer->close();

                if ( m_hits.size() > 0 )
                {
                    QString hit = *m_hits.begin();
                    m_hits.remove( m_hits.begin() );

                    QUrl url = QUrl( QString( "http://%1%2" ).arg( m_referer.host() ).arg( hit ) );
                    startRequest( url );
                }
                else
                    emit endSearch( false );
            }
        }
    }

    void WebQueryGoogleScholar::headerReceived( const QHttpResponseHeader &resp )
    {
        m_redirectLocation = "";

        QStringList keys = resp.keys();
        for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it )
        {
            QString key = *it;
            QString value = resp.value( key );
            if ( key == "location" )
                m_redirectLocation = value;
            else if ( key == "set-cookie" )
            {
                QRegExp rx( "^([^=]+)=([^;]+)" );
                rx.search( value );
                m_cookieMap.insert( rx.cap( 1 ), rx.cap( 2 ) );
            }
        }

    }

    void WebQueryGoogleScholar::startRequest( QUrl& url )
    {
        QHttpRequestHeader header( "GET", url.encodedPathAndQuery() );
        QString userAgent = "Mozilla/4.61 [dk] (OS/2; U)";
        header.setValue( "Host", url.host() );
        header.setValue( "User-Agent", userAgent );
        header.setValue( "Cookie", getCookies() );
        header.setValue( "Referer", m_referer.toString() );
        m_referer = url;

        if ( m_buffer != NULL )
            delete m_buffer;
        m_buffer = new QBuffer();
        m_buffer->open( IO_WriteOnly );
        m_http->setHost( url.host(), 80 );
        m_http->request( header, NULL, m_buffer );
    }

    QString WebQueryGoogleScholar::getCookies()
    {
        QStringList cookieList;
        for ( QMap<QString, QString>::Iterator it = m_cookieMap.begin(); it != m_cookieMap.end();++it )
            cookieList.append( it.key() + "=" + it.data() );
        return cookieList.join( ";" );
    }
}
