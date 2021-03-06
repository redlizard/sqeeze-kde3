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
#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>

#include <entry.h>
#include <file.h>
#include <xsltransform.h>
#include <entryfield.h>

#define max(a,b) ((a)>(b)?(a):(b))

namespace BibTeX
{

    Entry::Entry( )
            : Element(), m_entryType( etUnknown ), m_entryTypeString( QString::null ), m_id( QString::null )
    {
        // nothing
    }

    Entry::Entry( const EntryType entryType, const QString &id )
            : Element( ), m_entryType( entryType ), m_id( id )
    {
        m_entryTypeString = entryTypeToString( entryType );
    }

    Entry::Entry( const QString& entryTypeString, const QString& id ) : Element( ), m_entryTypeString( entryTypeString ), m_id( id )
    {
        m_entryType = entryTypeFromString( entryTypeString );
        if ( m_entryType != etUnknown )
            m_entryTypeString = entryTypeToString( m_entryType );
    }

    Entry::Entry( const Entry *other )
    {
        copyFrom( other );
    }

    Entry::~Entry()
    {
        for ( EntryFields::iterator it = m_fields.begin(); it != m_fields.end(); it++ )
        {
            delete( *it );
        }
    }

    Element* Entry::clone()
    {
        return new Entry( this );
    }

    QString Entry::text() const
    {
        QString result = "Id: ";
        result.append( m_id ).append( "  (" ).append( entryTypeString() ).append( ")\n" );

        for ( EntryFields::const_iterator it = m_fields.begin(); it != m_fields.end(); it++ )
        {
            result.append(( *it )->fieldTypeName() ).append( ": " );
            result.append(( *it )->value()->text() ).append( "\n" );
        }

        return result;
    }

    void Entry::setEntryType( const EntryType entryType )
    {
        m_entryType = entryType;
        m_entryTypeString = entryTypeToString( entryType );
    }

    void Entry::setEntryTypeString( const QString& entryTypeString )
    {
        m_entryTypeString = entryTypeString;
        m_entryType = entryTypeFromString( entryTypeString );
    }

    Entry::EntryType Entry::entryType() const
    {
        return m_entryType;
    }

    QString Entry::entryTypeString() const
    {
        return m_entryTypeString;
    }

    void Entry::setId( const QString& id )
    {
        m_id = id;
    }

    QString Entry::id() const
    {
        return m_id;
    }

    bool Entry::containsPattern( const QString & pattern, EntryField::FieldType fieldType, BibTeX::Element::FilterType filterType, bool caseSensitive ) const
    {
        if ( filterType == ftExact )
        {
            /** check for exact match */
            bool result = fieldType == EntryField::ftUnknown && m_id.contains( pattern, caseSensitive );

            for ( EntryFields::ConstIterator it = m_fields.begin(); !result && it != m_fields.end(); it++ )
                if ( fieldType == EntryField::ftUnknown || ( *it ) ->fieldType() == fieldType )
                    result |= ( *it ) ->value() ->containsPattern( pattern, caseSensitive );

            return result;
        }
        else
        {
            /** for each word in the search pattern ... */
            QStringList words = QStringList::split( QRegExp( "\\s+" ), pattern );
            bool *hits = new bool[words.count()];
            int i = 0;
            for ( QStringList::Iterator wit = words.begin(); wit != words.end(); ++wit, ++i )
            {
                hits[i] = fieldType == EntryField::ftUnknown && m_id.contains( *wit, caseSensitive );

                /** check if word is contained in any field */
                for ( EntryFields::ConstIterator fit = m_fields.begin(); fit != m_fields.end(); ++fit )
                    if ( fieldType == EntryField::ftUnknown || ( *fit ) ->fieldType() == fieldType )
                        hits[i] |= ( *fit ) ->value() ->containsPattern( *wit, caseSensitive );
            }

            unsigned int hitCount = 0;
            for ( i = words.count() - 1; i >= 0; --i )
                if ( hits[i] ) ++hitCount;
            delete[] hits;

            /** return success depending on filter type and number of hits */
            return ( filterType == ftAnyWord && hitCount > 0 || filterType == ftEveryWord && hitCount == words.count() );
        }
    }

    QStringList Entry::urls() const
    {
        QStringList result;
        QString fieldNames[] = {"localfile", "pdf", "ps", "postscript", "doi", "url", "howpublished", "ee", "biburl"};

        for ( unsigned int i = 0; i < sizeof( fieldNames ) / sizeof( fieldNames[0] ); i++ )
        {
            EntryField * field = getField( fieldNames[ i ] );
            if (( field && !field->value() ->items.isEmpty() ) )
            {
                PlainText *plainText = dynamic_cast<PlainText*>( field->value()->items.first() );
                if ( plainText != NULL )
                {
                    QString plain = plainText->text();
                    int urlPos = plain.find( "\\url{", 0, FALSE );
                    if ( urlPos > -1 )
                    {
                        plain = plain.mid( urlPos + 5 );
                        urlPos = plain.find( "}", 0, FALSE );
                        if ( urlPos > 0 )
                            plain = plain.left( urlPos - 1 );
                    }

                    if ( fieldNames[ i ] == "doi" && !plain.startsWith( "http", FALSE ) )
                        plain.prepend( "http://dx.doi.org/" );

                    result.append( plain );
                }
            }
        }

        return result;
    }

    bool Entry::addField( EntryField * field )
    {
        m_fields.append( field );
        return TRUE;
    }

    EntryField* Entry::getField( const EntryField::FieldType fieldType ) const
    {
        EntryField * result = NULL;

        for ( EntryFields::ConstIterator it = m_fields.begin(); ( it != m_fields.end() ) && ( result == NULL ); it++ )
            if (( *it ) ->fieldType() == fieldType ) result = *it;

        return result;
    }

    EntryField* Entry::getField( const QString & fieldName ) const
    {
        EntryField * result = NULL;

        for ( EntryFields::ConstIterator it = m_fields.begin(); ( it != m_fields.end() ) && ( result == NULL ); it++ )
            if (( *it ) ->fieldTypeName().lower() == fieldName.lower() )
                result = *it;

        return result;
    }

    bool Entry::deleteField( const QString & fieldName )
    {
        for ( EntryFields::ConstIterator it = m_fields.begin(); it != m_fields.end(); it++ )
            if (( *it ) ->fieldTypeName().lower() == fieldName.lower() )
            {
                delete( *it );
                m_fields.remove( *it );
                return TRUE;
            }

        return FALSE;
    }

    bool Entry::deleteField( const EntryField::FieldType fieldType )
    {
        for ( EntryFields::iterator it = m_fields.begin(); it != m_fields.end(); it++ )
            if (( *it ) ->fieldType() == fieldType )
            {
                delete( *it );
                m_fields.remove( it );
                return TRUE;
            }

        return FALSE;
    }

    Entry::EntryFields::const_iterator Entry::begin() const
    {
        return m_fields.constBegin();
    }

    Entry::EntryFields::const_iterator Entry::end() const
    {
        return m_fields.constEnd();
    }

    int Entry::getFieldCount() const
    {
        return m_fields.count();
    }

    void Entry::clearFields()
    {
        for ( EntryFields::iterator it = m_fields.begin(); it != m_fields.end(); it++ )
            delete( *it );
        m_fields.clear();
    }

    void Entry::copyFrom( const Entry *other )
    {
        if ( other == NULL ) return;

        m_entryType = other->m_entryType;
        m_entryTypeString = other->m_entryTypeString;
        m_id = other->m_id;
        clearFields();
        for ( EntryFields::const_iterator it = other->m_fields.begin(); it != other->m_fields.end(); it++ )
            m_fields.append( new EntryField( *it ) );
    }

    void Entry::merge( BibTeX::Entry *other, bool forceAdding )
    {
        for ( EntryFields::iterator it = other->m_fields.begin(); it != other->m_fields.end(); it++ )
        {
            EntryField *otherField = new EntryField( *it );
            EntryField::FieldType otherFieldType = otherField->fieldType();
            QString otherFieldTypeName = otherField->fieldTypeName();
            EntryField *thisField = otherFieldType != EntryField::ftUnknown ? getField( otherFieldType ) : getField( otherFieldTypeName );

            if ( thisField != NULL && !forceAdding ) continue;

            if ( thisField != NULL )
            {
                otherFieldTypeName.prepend( "OPT" );
                otherField->setFieldType( EntryField::ftUnknown, otherFieldTypeName );
            }
            m_fields.append( otherField );
        }
    }

    QString Entry::entryTypeToString( const EntryType entryType )
    {
        switch ( entryType )
        {
        case etArticle:
            return QString( "Article" );
        case etBook:
            return QString( "Book" );
        case etBooklet:
            return QString( "Booklet" );
        case etCollection:
            return QString( "Collection" );
        case etElectronic:
            return QString( "Electronic" );
        case etInBook:
            return QString( "InBook" );
        case etInCollection:
            return QString( "InCollection" );
        case etInProceedings:
            return QString( "InProceedings" );
        case etManual:
            return QString( "Manual" );
        case etMastersThesis:
            return QString( "MastersThesis" );
        case etMisc:
            return QString( "Misc" );
        case etPhDThesis:
            return QString( "PhDThesis" );
        case etProceedings:
            return QString( "Proceedings" );
        case etTechReport:
            return QString( "TechReport" );
        case etUnpublished:
            return QString( "Unpublished" );
        default:
            return QString( "Unknown" );
        }
    }

    Entry::EntryType Entry::entryTypeFromString( const QString & entryTypeString )
    {
        QString entryTypeStringLower = entryTypeString.lower();
        if ( entryTypeStringLower == "article" )
            return etArticle;
        else if ( entryTypeStringLower == "book" )
            return etBook;
        else if ( entryTypeStringLower == "booklet" )
            return etBooklet;
        else if ( entryTypeStringLower == "collection" )
            return etCollection;
        else if (( entryTypeStringLower == "electronic" ) || ( entryTypeStringLower == "online" ) || ( entryTypeStringLower == "internet" ) || ( entryTypeStringLower == "webpage" ) )
            return etElectronic;
        else if ( entryTypeStringLower == "inbook" )
            return etInBook;
        else if ( entryTypeStringLower == "incollection" )
            return etInCollection;
        else if (( entryTypeStringLower == "inproceedings" ) || ( entryTypeStringLower == "conference" ) )
            return etInProceedings;
        else if ( entryTypeStringLower == "manual" )
            return etManual;
        else if ( entryTypeStringLower == "mastersthesis" )
            return etMastersThesis;
        else if ( entryTypeStringLower == "misc" )
            return etMisc;
        else if ( entryTypeStringLower == "phdthesis" )
            return etPhDThesis;
        else if ( entryTypeStringLower == "proceedings" )
            return etProceedings;
        else if ( entryTypeStringLower == "techreport" )
            return etTechReport;
        else if ( entryTypeStringLower == "unpublished" )
            return etUnpublished;
        else
            return etUnknown;
    }

    Entry::FieldRequireStatus Entry::getRequireStatus( Entry::EntryType entryType, EntryField::FieldType fieldType )
    {
        switch ( entryType )
        {
        case etArticle:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftTitle:
            case EntryField::ftJournal:
            case EntryField::ftYear:
                return Entry::frsRequired;
            case EntryField::ftVolume:
            case EntryField::ftMonth:
            case EntryField::ftDoi:
            case EntryField::ftNumber:
            case EntryField::ftPages:
            case EntryField::ftNote:
            case EntryField::ftKey:
            case EntryField::ftISSN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etBook:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftEditor:
            case EntryField::ftTitle:
            case EntryField::ftPublisher:
            case EntryField::ftYear:
                return Entry::frsRequired;
            case EntryField::ftVolume:
            case EntryField::ftSeries:
            case EntryField::ftAddress:
            case EntryField::ftDoi:
            case EntryField::ftEdition:
            case EntryField::ftMonth:
            case EntryField::ftNote:
            case EntryField::ftKey:
            case EntryField::ftISBN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etBooklet:
            switch ( fieldType )
            {
            case EntryField::ftTitle:
                return Entry::frsRequired;
            case EntryField::ftAuthor:
            case EntryField::ftHowPublished:
            case EntryField::ftAddress:
            case EntryField::ftDoi:
            case EntryField::ftMonth:
            case EntryField::ftYear:
            case EntryField::ftNote:
            case EntryField::ftKey:
            case EntryField::ftISBN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etElectronic:
            switch ( fieldType )
            {
            case EntryField::ftTitle:
            case EntryField::ftURL:
                return Entry::frsRequired;
            case EntryField::ftAuthor:
            case EntryField::ftHowPublished:
            case EntryField::ftDoi:
            case EntryField::ftMonth:
            case EntryField::ftYear:
            case EntryField::ftKey:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etInBook:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftEditor:
            case EntryField::ftTitle:
            case EntryField::ftPages:
            case EntryField::ftChapter:
            case EntryField::ftPublisher:
            case EntryField::ftYear:
                return Entry::frsRequired;
            case EntryField::ftVolume:
            case EntryField::ftSeries:
            case EntryField::ftAddress:
            case EntryField::ftDoi:
            case EntryField::ftEdition:
            case EntryField::ftMonth:
            case EntryField::ftNote:
            case EntryField::ftCrossRef:
            case EntryField::ftKey:
            case EntryField::ftISBN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etInCollection:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftTitle:
            case EntryField::ftBookTitle:
            case EntryField::ftPublisher:
            case EntryField::ftYear:
                return Entry::frsRequired;
            case EntryField::ftEditor:
            case EntryField::ftPages:
            case EntryField::ftOrganization:
            case EntryField::ftAddress:
            case EntryField::ftMonth:
            case EntryField::ftLocation:
            case EntryField::ftNote:
            case EntryField::ftCrossRef:
            case EntryField::ftDoi:
            case EntryField::ftKey:
            case EntryField::ftType:
            case EntryField::ftISBN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etInProceedings:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftTitle:
            case EntryField::ftYear:
            case EntryField::ftBookTitle:
                return Entry::frsRequired;
            case EntryField::ftPages:
            case EntryField::ftEditor:
            case EntryField::ftVolume:
            case EntryField::ftNumber:
            case EntryField::ftSeries:
            case EntryField::ftType:
            case EntryField::ftChapter:
            case EntryField::ftAddress:
            case EntryField::ftDoi:
            case EntryField::ftEdition:
            case EntryField::ftLocation:
            case EntryField::ftMonth:
            case EntryField::ftNote:
            case EntryField::ftCrossRef:
            case EntryField::ftPublisher:
            case EntryField::ftISBN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etManual:
            switch ( fieldType )
            {
            case EntryField::ftTitle:
                return Entry::frsRequired;
            case EntryField::ftAuthor:
            case EntryField::ftOrganization:
            case EntryField::ftAddress:
            case EntryField::ftDoi:
            case EntryField::ftEdition:
            case EntryField::ftMonth:
            case EntryField::ftYear:
            case EntryField::ftNote:
            case EntryField::ftISBN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etMastersThesis:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftTitle:
            case EntryField::ftSchool:
            case EntryField::ftYear:
                return Entry::frsRequired;
            case EntryField::ftAddress:
            case EntryField::ftMonth:
            case EntryField::ftDoi:
            case EntryField::ftNote:
            case EntryField::ftKey:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etMisc:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftTitle:
            case EntryField::ftHowPublished:
            case EntryField::ftMonth:
            case EntryField::ftYear:
            case EntryField::ftDoi:
            case EntryField::ftNote:
            case EntryField::ftKey:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etPhDThesis:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftTitle:
            case EntryField::ftSchool:
            case EntryField::ftYear:
                return Entry::frsRequired;
            case EntryField::ftAddress:
            case EntryField::ftMonth:
            case EntryField::ftNote:
            case EntryField::ftDoi:
            case EntryField::ftKey:
            case EntryField::ftISBN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etCollection:
        case etProceedings:
            switch ( fieldType )
            {
            case EntryField::ftTitle:
            case EntryField::ftYear:
                return Entry::frsRequired;
            case EntryField::ftEditor:
            case EntryField::ftPublisher:
            case EntryField::ftOrganization:
            case EntryField::ftAddress:
            case EntryField::ftMonth:
            case EntryField::ftLocation:
            case EntryField::ftNote:
            case EntryField::ftDoi:
            case EntryField::ftKey:
            case EntryField::ftSeries:
            case EntryField::ftBookTitle:
            case EntryField::ftISBN:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etTechReport:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftTitle:
            case EntryField::ftInstitution:
            case EntryField::ftYear:
                return Entry::frsRequired;
            case EntryField::ftType:
            case EntryField::ftDoi:
            case EntryField::ftNumber:
            case EntryField::ftAddress:
            case EntryField::ftMonth:
            case EntryField::ftNote:
            case EntryField::ftKey:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
            case EntryField::ftISSN:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        case etUnpublished:
            switch ( fieldType )
            {
            case EntryField::ftAuthor:
            case EntryField::ftTitle:
            case EntryField::ftNote:
                return Entry::frsRequired;
            case EntryField::ftMonth:
            case EntryField::ftYear:
            case EntryField::ftDoi:
            case EntryField::ftKey:
            case EntryField::ftURL:
            case EntryField::ftLocalFile:
                return Entry::frsOptional;
            default:
                return Entry::frsIgnored;
            }
        default:
            return Entry::frsOptional;
        }
    }
}
