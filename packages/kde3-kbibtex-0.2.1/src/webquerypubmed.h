/***************************************************************************
*   Copyright (C) 2004-2006 by Thomas Fischer                             *
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
#ifndef KBIBTEXWEBQUERYPUBMED_H
#define KBIBTEXWEBQUERYPUBMED_H

#include <qwidget.h>
#include <qxml.h>
#include <qvaluelist.h>

#include <webquery.h>

class QStringList;
class QDomElement;
class QDate;
class BibTeX::File;

namespace KBibTeX
{
    class WebQueryPubMedStructureParserQuery : public QXmlDefaultHandler
    {
    public:
        WebQueryPubMedStructureParserQuery( QValueList<int> *intList );
        ~WebQueryPubMedStructureParserQuery( );

        bool startElement( const QString&, const QString&, const QString&, const QXmlAttributes& );
        bool endElement( const QString&, const QString&, const QString& );
        bool characters( const QString & ch );

    private:
        QValueList<int> *m_intList;
        QString concatString;
    };

    class WebQueryPubMedResultParser: public QObject
    {
        Q_OBJECT
    public:
        WebQueryPubMedResultParser();
        ~WebQueryPubMedResultParser();

        void parse( const QDomElement& rootElement );

    signals:
        void foundEntry( BibTeX::Entry* );

    private:
        void parsePubmedArticle( const QDomElement& element, BibTeX::Entry *entry );
        void parseMedlineCitation( const QDomElement& element, BibTeX::Entry *entry );
        void parseArticle( const QDomElement& element, BibTeX::Entry *entry );
        void parseJournal( const QDomElement& element, BibTeX::Entry *entry );
        void parseJournalIssue( const QDomElement& element, BibTeX::Entry *entry );
        void parsePubDate( const QDomElement& element, BibTeX::Entry *entry );
        void parseAuthorList( const QDomElement& element, BibTeX::Entry *entry );
    };

    class WebQueryPubMed : public WebQuery
    {
        Q_OBJECT
    public:
        WebQueryPubMed( QWidget *parent );
        virtual ~WebQueryPubMed();

        void query( const QString& searchTerm, int numberOfResults );
        void cancelQuery() { /* nothing */ };

        QString title();
        QString disclaimer();
        QString disclaimerURL();
    };

}

#endif
