/***************************************************************************
*   Copyright (C) 2004-2008 by Thomas Fischer                             *
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
#include <qwidget.h>
#include <qdom.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <qcstring.h>
#include <qregexp.h>

#include <kdialog.h>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

#include <entryfield.h>
#include <value.h>
#include <settings.h>
#include "webquerypubmed.h"

namespace KBibTeX
{

    WebQueryPubMed::WebQueryPubMed( QWidget *parent ) : WebQuery( parent )
    {
        // nothing
    }

    WebQueryPubMed::~WebQueryPubMed()
    {
        // nothing
    }

    QString WebQueryPubMed::title()
    {
        return i18n( "NCBI (PubMed)" );
    }

    QString WebQueryPubMed::disclaimer()
    {
        return i18n( "NCBI's Disclaimer and Copyright" );
    }

    QString WebQueryPubMed::disclaimerURL()
    {
        return "http://eutils.ncbi.nlm.nih.gov/About/disclaimer.html";
    }

    void WebQueryPubMed::query( const QString& searchTerm, int numberOfResults )
    {
        WebQuery::query( searchTerm, numberOfResults );

        emit setTotalSteps( 2 );

        QString term = searchTerm;
        KURL url = KURL( QString( "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=pubmed&term=%1&retmax=%2&tool=KBibTeX&email=kbibtex@unix-ag.uni-kl.de" ).arg( term.replace( "%", "%25" ).replace( " ", "+" ).replace( "?", "%3F" ).replace( "&", "%26" ) ).arg( numberOfResults ) );

        QString tmpFile;
        if ( KIO::NetAccess::download( url, tmpFile, m_parent ) && !m_aborted )
        {
            QFile inputFile( tmpFile );
            QValueList<int> intList;
            QXmlInputSource inputSource( &inputFile );
            QXmlSimpleReader reader;
            WebQueryPubMedStructureParserQuery handler( &intList );
            reader.setContentHandler( &handler );
            reader.parse( &inputSource );
            inputFile.close();
            KIO::NetAccess::removeTempFile( tmpFile );

            emit setProgress( 1 );

            QString ids;
            QValueList<int>::iterator it = intList.begin();
            if ( it != intList.end() )
            {
                ids.append( QString::number( *it ) );
                ++it;
                for ( ; it != intList.end(); ++it )
                {
                    ids.append( "," );
                    ids.append( QString::number( *it ) );
                }
            }

            url = KURL( QString( "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=pubmed&retmode=xml&id=%1&tool=KBibTeX&email=kbibtex@unix-ag.uni-kl.de" ).arg( ids ) );
            if ( KIO::NetAccess::download( url, tmpFile, m_parent ) && !m_aborted )
            {
                QFile inputFile( tmpFile );
                QDomDocument doc( "efetch'ed" );
                doc.setContent( &inputFile );
                QDomElement docElem = doc.documentElement();
                emit setProgress( 2 );
                WebQueryPubMedResultParser resultParser;
                connect( &resultParser, SIGNAL( foundEntry( BibTeX::Entry* ) ), this, SIGNAL( foundEntry( BibTeX::Entry* ) ) );
                resultParser.parse( doc.documentElement() );
                inputFile.close();
                KIO::NetAccess::removeTempFile( tmpFile );
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

    WebQueryPubMedStructureParserQuery::WebQueryPubMedStructureParserQuery( QValueList<int> *intList ) : QXmlDefaultHandler(), m_intList( intList )
    {
        m_intList->clear();
    }

    WebQueryPubMedStructureParserQuery::~WebQueryPubMedStructureParserQuery( )
    {
        // nothing
    }

    bool WebQueryPubMedStructureParserQuery::startElement( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & /*qName*/, const QXmlAttributes & /*atts*/ )
    {
        concatString = QString();
        return TRUE;
    }

    bool WebQueryPubMedStructureParserQuery::endElement( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & qName )
    {
        if ( qName == "Id" )
        {
            bool ok;
            int id = concatString.toInt( &ok );
            if ( ok && id > 0 && m_intList != NULL )
                m_intList->append( id );
        }

        return TRUE;
    }

    bool WebQueryPubMedStructureParserQuery::characters( const QString & ch )
    {
        concatString.append( ch );
        return TRUE;
    }

    WebQueryPubMedResultParser::WebQueryPubMedResultParser( ) : QObject()
    {
        // nothing
    }

    void WebQueryPubMedResultParser::parse( const QDomElement& rootElement )
    {
        if ( rootElement.tagName() == "PubmedArticleSet" )
            for ( QDomNode n = rootElement.firstChild(); !n.isNull(); n = n.nextSibling() )
            {
                QDomElement e = n.toElement();
                if ( !e.isNull() && e.tagName() == "PubmedArticle" )
                {
                    BibTeX::Entry * entry = new BibTeX::Entry( BibTeX::Entry::etMisc, "PubMed" );
                    parsePubmedArticle( e, entry );
                    emit foundEntry( entry );
                }
            }
    }

    WebQueryPubMedResultParser::~WebQueryPubMedResultParser()
    {
        // nothing
    }

    void WebQueryPubMedResultParser::parsePubmedArticle( const QDomElement& element, BibTeX::Entry *entry )
    {
        for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
        {
            QDomElement e = n.toElement();
            if ( !e.isNull() && e.tagName() == "MedlineCitation" )
                parseMedlineCitation( e, entry );
        }
    }

    void WebQueryPubMedResultParser::parseMedlineCitation( const QDomElement& element, BibTeX::Entry *entry )
    {
        for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
        {
            QDomElement e = n.toElement();
            if ( !e.isNull() )
            {
                if ( e.tagName() == "PMID" )
                    entry->setId( QString( "PubMed_%1" ).arg( e.text() ) );
                else if ( e.tagName() == "Article" )
                    parseArticle( e, entry );
                else if ( e.tagName() == "MedlineJournalInfo" )
                {
                    for ( QDomNode n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() )
                    {
                        QDomElement e2 = n2.toElement();
                        if ( e2.tagName() == "MedlineTA" )
                        {
                            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftJournal );
                            if ( field == NULL )
                            {
                                field = new BibTeX::EntryField( BibTeX::EntryField::ftJournal );
                                entry->addField( field );
                            }
                            field->setValue( new BibTeX::Value( e2.text() ) );
                        }
                    }
                }
            }
        }

    }

    void WebQueryPubMedResultParser::parseArticle( const QDomElement& element, BibTeX::Entry *entry )
    {
        for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
        {
            QDomElement e = n.toElement();

            if ( e.tagName() == "Journal" )
            {
                parseJournal( e, entry );
                entry->setEntryType( BibTeX::Entry::etArticle );
            }
            else if ( e.tagName() == "ArticleTitle" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftTitle );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftTitle );
                    entry->addField( field );
                }
                field->setValue( new BibTeX::Value( e.text() ) );
            }
            else if ( e.tagName() == "Pagination" )
            {
                QDomElement medlinePgn = e.firstChild().toElement(); // may fail?
                if ( !medlinePgn.text().isEmpty() )
                {
                    BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftPages );
                    if ( field == NULL )
                    {
                        field = new BibTeX::EntryField( BibTeX::EntryField::ftPages );
                        entry->addField( field );
                    }
                    field->setValue( new BibTeX::Value( medlinePgn.text() ) );
                }
            }
            else if ( e.tagName() == "Abstract" )
            {
                QDomElement abstractText = e.firstChild().toElement();
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftAbstract );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftAbstract );
                    entry->addField( field );
                }
                field->setValue( new BibTeX::Value( abstractText.text() ) );
            }
            else if ( e.tagName() == "Affiliation" )
            {
                BibTeX::EntryField * field = entry->getField( "affiliation" );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( "affiliation" );
                    entry->addField( field );
                }
                field->setValue( new BibTeX::Value( e.text() ) );
            }
            else if ( e.tagName() == "AuthorList" )
                parseAuthorList( e, entry );
        }
    }

    void WebQueryPubMedResultParser::parseJournal( const QDomElement& element, BibTeX::Entry *entry )
    {
        for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
        {
            QDomElement e = n.toElement();

            if ( e.tagName() == "ISSN" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftISSN );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftISSN );
                    entry->addField( field );
                }
                field->setValue( new BibTeX::Value( e.text() ) );
            }
            else if ( e.tagName() == "JournalIssue" )
                parseJournalIssue( e, entry );
            else if ( e.tagName() == "Title" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftJournal );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftJournal );
                    entry->addField( field );
                }
                field->setValue( new BibTeX::Value( e.text() ) );
            }
        }
    }

    void WebQueryPubMedResultParser::parseJournalIssue( const QDomElement& element, BibTeX::Entry *entry )
    {
        for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
        {
            QDomElement e = n.toElement();

            if ( e.tagName() == "Volume" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftVolume );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftVolume );
                    entry->addField( field );
                }
                field->setValue( new BibTeX::Value( e.text() ) );
            }
            else if ( e.tagName() == "Issue" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftNumber );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftNumber );
                    entry->addField( field );
                }
                field->setValue( new BibTeX::Value( e.text() ) );
            }
            else if ( e.tagName() == "PubDate" )
                parsePubDate( e, entry );
        }
    }

    void WebQueryPubMedResultParser::parsePubDate( const QDomElement& element, BibTeX::Entry *entry )
    {
        for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
        {
            QDomElement e = n.toElement();

            if ( e.tagName() == "Year" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftYear );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftYear );
                    entry->addField( field );
                }
                field->setValue( new BibTeX::Value( e.text() ) );
            }
            else if ( e.tagName() == "Month" )
            {
                QString month = e.text().lower();
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftMonth );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftMonth );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::MacroKey( month ) );
                field->setValue( value );
            }
            else if ( e.tagName() == "MedlineDate" )
            {
                QStringList frags = QStringList::split( QRegExp( "\\s+" ), e.text() );
                for ( QStringList::Iterator it = frags.begin(); it != frags.end(); ++it )
                {
                    bool ok;
                    int num = ( *it ).toInt( &ok );
                    if ( ok && num > 1000 && num < 3000 )
                    {
                        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftYear );
                        if ( field == NULL )
                        {
                            field = new BibTeX::EntryField( BibTeX::EntryField::ftYear );
                            entry->addField( field );
                        }
                        BibTeX::Value *value = new BibTeX::Value();
                        value->items.append( new BibTeX::MacroKey( QString::number( num ) ) );
                        field->setValue( value );
                    }
                    else if ( !ok && ( *it ).length() == 3 )
                    {
                        BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftMonth );
                        if ( field == NULL )
                        {
                            field = new BibTeX::EntryField( BibTeX::EntryField::ftMonth );
                            entry->addField( field );
                        }
                        BibTeX::Value *value = new BibTeX::Value();
                        value->items.append( new BibTeX::MacroKey(( *it ).lower() ) );
                        field->setValue( value );
                    }
                }
            }
        }
    }

    void WebQueryPubMedResultParser::parseAuthorList( const QDomElement& element, BibTeX::Entry *entry )
    {
        if ( element.attribute( "CompleteYN", "Y" ) == "Y" )
        {
            QStringList authorList;
            for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
            {
                QDomElement e = n.toElement();
                if ( e.tagName() == "Author" && e.attribute( "ValidYN", "Y" ) == "Y" )
                {
                    QString lastName, firstName;
                    for ( QDomNode n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() )
                    {
                        QDomElement e2 = n2.toElement();
                        if ( e2.tagName() == "LastName" )
                            lastName = e2.text();
                        else if ( e2.tagName() == "FirstName" || e2.tagName() == "ForeName" )
                            firstName = e2.text();
                    }
                    QString name = lastName;
                    if ( !firstName.isEmpty() )
                        name.prepend( "|" ).prepend( firstName );
                    authorList.append( name );
                }
            }

            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftAuthor );
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( BibTeX::EntryField::ftAuthor );
                entry->addField( field );
            }
            BibTeX::Value *value = new BibTeX::Value();
            Settings *settings = Settings::self();
            BibTeX::PersonContainer *personContainer = new BibTeX::PersonContainer( settings->editing_FirstNameFirst );
            value->items.append( personContainer );
            for ( QStringList::Iterator sli = authorList.begin(); sli != authorList.end(); ++sli )
            {
                QStringList nameParts = QStringList::split( '|', *sli );
                QString firstName = nameParts.count() > 1 ? nameParts[0] : "";
                QString lastName = nameParts[nameParts.count() - 1];
                personContainer->persons.append( new BibTeX::Person( firstName, lastName, settings->editing_FirstNameFirst ) );
            }
            field->setValue( value );
        }
    }

}

