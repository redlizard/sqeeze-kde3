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
#ifndef KBIBTEXWEBQUERYGOOGLESCHOLAR_H
#define KBIBTEXWEBQUERYGOOGLESCHOLAR_H

#include <qurl.h>
#include <qhttp.h>
#include <qstringlist.h>

#include <fileimporterbibtex.h>
#include <webquery.h>

class QBuffer;

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class WebQueryGoogleScholar : public WebQuery
    {
        Q_OBJECT
    public:
        WebQueryGoogleScholar( QWidget* parent );
        virtual ~WebQueryGoogleScholar();

        void query( const QString& searchTerm, int numberOfResults );
        void cancelQuery();

        QString title();
        QString disclaimer();
        QString disclaimerURL();

    protected slots:
        void done( bool );
        void headerReceived( const QHttpResponseHeader & );

    private:
        enum GoogleState { gsInit, gsGetPref, gsSetPref, gsQuery, gsResult };

        void startRequest( QUrl& url );
        QString getCookies();

        int m_currentRequestNumber;
        QString m_searchTerm;
        int m_numberOfResults;
        GoogleState m_state;
        BibTeX::FileImporterBibTeX *m_importer;
        QUrl m_referer;
        QMap<QString, QString> m_cookieMap;
        QString m_redirectLocation;
        QHttp *m_http;
        QBuffer *m_buffer;
        QStringList m_hits;
    };

}

#endif
