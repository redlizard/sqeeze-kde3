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
#include <qregexp.h>

#include <klocale.h>

#include <encoderlatex.h>
#include <file.h>
#include <settings.h>
#include <entry.h>
#include "idsuggestions.h"

namespace KBibTeX
{
    const QStringList IdSuggestions::smallWords = QStringList::split( '|', "and|on|in|the|of|at|a|an|with|for" );
    const QRegExp IdSuggestions::unwantedChars( "\\s|['\"$\\\\{}:-,]+" );

    IdSuggestions::IdSuggestions()
    {
// nothing
    }


    IdSuggestions::~IdSuggestions()
    {
// nothing
    }

    /**
    * Determine list of authors for a given entry
    */
    QStringList IdSuggestions::authorsLastName( BibTeX::Entry *entry )
    {
        QStringList result;

        /** retrieve field holding authors information for entry */
        BibTeX::EntryField *field = entry->getField( BibTeX::EntryField::ftAuthor );
        if ( field == NULL )
            return result; /** no author field available */

        /** fetch container holding list of author names */
        BibTeX::PersonContainer *personContainer = field != NULL ? dynamic_cast<BibTeX::PersonContainer*>( field->value()->items.isEmpty() ? NULL : field->value()->items.first() ) : NULL;
        if ( personContainer == NULL || personContainer->persons.isEmpty() )
            return result; /** container not found or is empty */

        /** iterate through container and fetch each author's last name */
        for ( QValueList<BibTeX::Person*>::ConstIterator it = personContainer->persons.begin(); it != personContainer->persons.end(); ++it )
            result.append(( *it )->lastName() );

        return result;
    }

    /**
    * Normalize a given text by removing spaces or other unwanted characters
    */
    QString IdSuggestions::normalizeText( const QString& text )
    {
        BibTeX::EncoderLaTeX encoder;
        QString result = text;
        result = encoder.encode( result );
        result = result.remove( unwantedChars );

        return result;
    }

    /**
    * Determine year for a given entry
    */
    int IdSuggestions::extractYear( BibTeX::Entry *entry )
    {
        /** retrieve field holding year information for entry */
        BibTeX::EntryField *field = entry->getField( BibTeX::EntryField::ftYear );
        if ( field == NULL )
            return -1; /** no year field available */

        /** *fetch value item holding year */
        BibTeX::ValueItem *valueItem = field != NULL ? ( field->value()->items.isEmpty() ? NULL : field->value()->items.first() ) : NULL;
        if ( valueItem == NULL )
            return -1; /** no value item found or is empty */

        /** parse value item's text */
        bool ok = FALSE;
        int year = QString( valueItem->text() ).toInt( &ok );
        if ( !ok ) year = -1;

        return year;
    }

    /**
    * Determine title for a given entry
    */
    QString IdSuggestions::extractTitle( BibTeX::Entry *entry )
    {
        /** retrieve field holding title information for entry */
        BibTeX::EntryField *field = entry->getField( BibTeX::EntryField::ftTitle );
        if ( field == NULL )
            return QString::null; /** no title field available */

        /** *fetch value item holding title */
        BibTeX::ValueItem *valueItem = field->value()->items.isEmpty() ? NULL : field->value()->items.first();
        if ( valueItem == NULL )
            return QString::null; /** no value item found or is empty */

        return valueItem->text(); // TODO: Perform some postprocessing?
    }

    QStringList IdSuggestions::createSuggestions( BibTeX::File *file, BibTeX::Entry *entry )
    {
        Settings * settings = Settings::self();
        const QStringList& formatStrList = settings->idSuggestions_formatStrList;
        QStringList result;
        QStringList allKeys = file->allKeys();
        entry = new BibTeX::Entry( entry );
        file->completeReferencedFields( entry );

        for ( QStringList::ConstIterator it = formatStrList.begin(); it != formatStrList.end(); ++it )
        {
            QString id = formatId( entry, *it );
            if ( id.isEmpty() || result.contains( id ) )
                continue;

            QString id2 = id;
            if ( allKeys.contains( id ) )
                for ( int i = 0; i < 26; ++i )
                {
                    QString id2 = id.append( QChar( 'a' + i ) );
                    if ( !allKeys.contains( id2 ) )
                    {
                        result.append( id2 );
                        break;
                    }
                }

            if ( !result.contains( id2 ) )
                result.append( id2 );
        }

        delete entry;

        return result;
    }

    QString IdSuggestions::createDefaultSuggestion( BibTeX::File *file, BibTeX::Entry *entry )
    {
        Settings * settings = Settings::self();
        if ( settings->idSuggestions_default < 0 || settings->idSuggestions_default >= ( int )settings->idSuggestions_formatStrList.size() )
            return QString::null;

        entry = new BibTeX::Entry( entry );
        file->completeReferencedFields( entry );

        QString result = formatId( entry, settings->idSuggestions_formatStrList[settings->idSuggestions_default] );

        delete entry;
        return result;
    }

    QString IdSuggestions::formatId( BibTeX::Entry *entry, const QString& formatStr )
    {
        QString id;
        QStringList tokenList = QStringList::split( '|', formatStr );
        for ( QStringList::ConstIterator tit = tokenList.begin(); tit != tokenList.end(); ++tit )
            id.append( translateToken( entry, *tit ) );

        return id;
    }

    QString IdSuggestions::translateToken( BibTeX::Entry *entry, const QString& token )
    {
        switch ( token[0] )
        {
        case 'a': return translateAuthorsToken( entry, token.mid( 1 ), TRUE );
        case 'A': return translateAuthorsToken( entry, token.mid( 1 ), FALSE );
        case 'y':
            {
                int year = extractYear( entry );
                if ( year > -1 )
                    return QString::number( year % 100 + 100 ).mid( 1 );
                else
                    return QString::null;
            }
        case 'Y':
            {
                int year = extractYear( entry );
                if ( year > -1 )
                    return QString::number( year % 10000 + 10000 ).mid( 1 );
                else
                    return QString::null;
            }
        case 't': return translateTitleToken( entry, token.mid( 1 ), FALSE );
        case 'T': return translateTitleToken( entry, token.mid( 1 ), TRUE );
        case '"': return token.mid( 1 );
        default: return QString::null;
        }
    }

    QString IdSuggestions::translateAuthorsToken( BibTeX::Entry *entry, const QString& token, bool onlyFirst )
    {
        struct IdSuggestionTokenInfo ati = evalToken( token );
        QString result;
        bool first = TRUE;
        QStringList authors = authorsLastName( entry );
        for ( QStringList::ConstIterator it = authors.begin(); it != authors.end(); ++it )
        {
            if ( first )
                first = FALSE;
            else
                result.append( ati.inBetween );

            QString author =  normalizeText( *it ).left( ati.len );
            result.append( author );
            if ( onlyFirst )
                break;
        }

        if ( ati.toUpper )
            result = result.upper();
        else if ( ati.toLower )
            result = result.lower();

        return result;
    }

    struct IdSuggestionTokenInfo IdSuggestions::evalToken( const QString& token )
    {
        unsigned int pos = 0;
        struct IdSuggestionTokenInfo result;
        result.len = 0x00ffffff;
        result.toLower = FALSE;
        result.toUpper = FALSE;
        result.inBetween = QString::null;

        if ( token.length() > pos )
        {
            int dv = token[pos].digitValue();
            if ( dv > -1 )
            {
                result.len = dv;
                ++pos;
            }
        }

        if ( token.length() > pos )
        {
            result.toLower = token[pos] == 'l';
            result.toUpper = token[pos] == 'u';
            if ( result.toUpper || result.toLower )
                ++pos;
        }

        if ( token.length() > pos + 1 && token[pos] == '"' )
            result.inBetween = token.mid( pos + 1 );

        return result;
    }

    QString IdSuggestions::translateTitleToken( BibTeX::Entry *entry, const QString& token, bool removeSmallWords )
    {
        struct IdSuggestionTokenInfo tti = evalToken( token );

        QString result;
        bool first = TRUE;
        QStringList titleWords = QStringList::split( QRegExp( "\\s+" ), extractTitle( entry ).replace( QRegExp( "[\\\\{}]+" ), "" ) );
        for ( QStringList::ConstIterator it = titleWords.begin(); it != titleWords.end(); ++it )
        {
            if ( first )
                first = FALSE;
            else
                result.append( tti.inBetween );

            QString lowerText = ( *it ).lower();
            if ( !removeSmallWords || !smallWords.contains( lowerText ) )
                result.append( normalizeText( *it ).left( tti.len ) );
        }

        if ( tti.toUpper )
            result = result.upper();
        else if ( tti.toLower )
            result = result.lower();

        return result;
    }

    QString IdSuggestions::formatStrToHuman( const QString& formatStr )
    {
        bool first = TRUE;
        QString text;
        QStringList elements = QStringList::split( '|', formatStr );
        for ( QStringList::iterator it = elements.begin();it != elements.end();++it )
        {
            if ( first ) first = FALSE; else text.append( "\n" );
            if (( *it )[0] == 'a' || ( *it )[0] == 'A' )
            {
                struct IdSuggestionTokenInfo info = evalToken(( *it ).mid( 1 ) );
                if (( *it )[0] == 'a' )
                    text.append( i18n( "First author only" ) );
                else
                    text.append( i18n( "All authors" ) );
                int n = info.len;
                if ( info.len < 0x00ffffff ) text.append( i18n( ", but only first letter of each last name", ", but only first %n letters of each last name", n ) );
                if ( info.toUpper ) text.append( i18n( ", in upper case" ) );
                else if ( info.toLower ) text.append( i18n( ", in lower case" ) );
                if ( info.inBetween != QString::null ) text.append( QString( i18n( ", with '%1' in between" ) ).arg( info.inBetween ) );
            }
            else if (( *it )[0] == 'y' ) text.append( i18n( "Year (2 digits)" ) );
            else if (( *it )[0] == 'Y' ) text.append( i18n( "Year (4 digits)" ) );
            else if (( *it )[0] == 't' || ( *it )[0] == 'T' )
            {
                struct IdSuggestionTokenInfo info = evalToken(( *it ).mid( 1 ) );
                text.append( i18n( "Title" ) );
                int n = info.len;
                if ( info.len < 0x00ffffff ) text.append( i18n( ", but only first letter of each word", ", but only first %n letters of each word", n ) );
                if ( info.toUpper ) text.append( i18n( ", in upper case" ) );
                else if ( info.toLower ) text.append( i18n( ", in lower case" ) );
                if ( info.inBetween != QString::null ) text.append( QString( i18n( ", with '%1' in between" ) ).arg( info.inBetween ) );
                if (( *it )[0] == 'T' ) text.append( i18n( ", small words removed" ) );
            }
            else if (( *it )[0] == '"' ) text.append( QString( i18n( "Text: '%1'" ) ).arg(( *it ).mid( 1 ) ) );
            else text.append( "?" );
        }

        return text;
    }
}
