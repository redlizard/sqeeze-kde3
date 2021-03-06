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
#include <qregexp.h>

#include <entry.h>

#include "fileexporterris.h"

namespace BibTeX
{

    FileExporterRIS::FileExporterRIS() : FileExporter()
    {
        // nothing
    }

    FileExporterRIS::~FileExporterRIS()
    {
        // nothing
    }

    bool FileExporterRIS::save( QIODevice* iodevice, const Element* element, QStringList* /*errorLog*/ )
    {
        bool result = FALSE;
        QTextStream stream( iodevice );

        const Entry *entry = dynamic_cast<const Entry*>( element );
        if ( entry != NULL )
            result = writeEntry( stream, entry );

        return result && !m_cancelFlag;
    }

    bool FileExporterRIS::save( QIODevice* iodevice, const File* bibtexfile, QStringList* /*errorLog*/ )
    {
        qDebug( "Exporting RIS" );
        bool result = TRUE;
        m_cancelFlag = FALSE;
        QTextStream stream( iodevice );

        for ( File::ElementList::const_iterator it = bibtexfile->elements.begin(); it != bibtexfile->elements.end() && result && !m_cancelFlag; it++ )
        {
            qDebug( "Casting element" );
            Entry *entry = dynamic_cast<Entry*>( *it );
            if ( entry != NULL )
            {
                BibTeX::Entry *myEntry = bibtexfile->completeReferencedFieldsConst( entry );
                result &= writeEntry( stream, myEntry );
                delete myEntry;
            }
            else
                qDebug( "Casting FAILED" );
        }

        qDebug( "Exporting RIS done" );
        return result && !m_cancelFlag;
    }

    void FileExporterRIS::cancel()
    {
        m_cancelFlag = TRUE;
    }

    bool FileExporterRIS::writeEntry( QTextStream &stream, const Entry* entry )
    {
        bool result = TRUE;
        qDebug( "Writing Entry" );

        switch ( entry->entryType() )
        {
        case Entry::etBook:
            writeKeyValue( stream, "TY", "BOOK" );
            break;
        case Entry::etInBook:
            writeKeyValue( stream, "TY", "CHAP" );
            break;
        case Entry::etInProceedings:
            writeKeyValue( stream, "TY", "CONF" );
            break;
        case Entry::etArticle:
            writeKeyValue( stream, "TY", "JOUR" );
            break;
        case Entry::etTechReport:
            writeKeyValue( stream, "TY", "RPRT" );
            break;
        case Entry::etPhDThesis:
            writeKeyValue( stream, "TY", "THES" );
            break;
        default:
            writeKeyValue( stream, "TY", "GEN" );
        }

        QString year = "";
        QString month = "";

        for ( Entry::EntryFields::ConstIterator it = entry->begin(); result && it != entry->end(); it++ )
        {
            EntryField *field = *it;

            if ( field->fieldType() == EntryField::ftUnknown && field->fieldTypeName().startsWith( "RISfield_" ) )
                result &= writeKeyValue( stream, field->fieldTypeName().right( 2 ), field->value()->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftAuthor )
            {
                QStringList authors = QStringList::split( QRegExp( "\\s+(,|and|&)\\s+", FALSE ), field->value() ->simplifiedText() );
                for ( QStringList::Iterator it = authors.begin(); result && it != authors.end(); ++it )
                    result &= writeKeyValue( stream, "AU", *it );
            }
            else if ( field->fieldType() == EntryField::ftEditor )
            {
                QStringList authors = QStringList::split( QRegExp( "\\s+(,|and|&)\\s+", FALSE ), field->value() ->simplifiedText() );
                for ( QStringList::Iterator it = authors.begin(); result && it != authors.end(); ++it )
                    result &= writeKeyValue( stream, "ED", *it );
            }
            else if ( field->fieldType() == EntryField::ftTitle )
                result &= writeKeyValue( stream, "TI", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftJournal )
                result &= writeKeyValue( stream, "JO", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftChapter )
                result &= writeKeyValue( stream, "CP", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftISSN )
                result &= writeKeyValue( stream, "SN", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftISBN )
                result &= writeKeyValue( stream, "SN", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftVolume )
                result &= writeKeyValue( stream, "VL", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftNumber )
                result &= writeKeyValue( stream, "IS", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftNote )
                result &= writeKeyValue( stream, "N1", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftAbstract )
                result &= writeKeyValue( stream, "N2", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftPublisher )
                result &= writeKeyValue( stream, "PB", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftLocation )
                result &= writeKeyValue( stream, "CY", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftKeywords )
                result &= writeKeyValue( stream, "KW", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftYear )
                year = field->value() ->simplifiedText();
            else if ( field->fieldType() == EntryField::ftMonth )
                month = field->value() ->simplifiedText();
            else if ( field->fieldType() == EntryField::ftAddress )
                result &= writeKeyValue( stream, "AD", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftURL )
                result &= writeKeyValue( stream, "UR", field->value() ->simplifiedText() );
            else if ( field->fieldType() == EntryField::ftPages )
            {
                QStringList pageRange = QStringList::split( QRegExp( QString( "--|-|%1" ).arg( QChar( 0x2013 ) ) ), field->value() ->simplifiedText() );
                if ( pageRange.count() == 2 )
                {
                    result &= writeKeyValue( stream, "SP", pageRange[ 0 ] );
                    result &= writeKeyValue( stream, "EP", pageRange[ 1 ] );
                }
            }
            else if ( field->fieldTypeName().lower() == "doi" )
                result &= writeKeyValue( stream, "UR", field->value() ->simplifiedText() );
        }

        if ( !year.isEmpty() || !month.isEmpty() )
        {
            result &= writeKeyValue( stream, "PY", QString( "%1/%2//" ).arg( year ).arg( month ) );
        }

        result &= writeKeyValue( stream, "ER", QString() );
        stream << endl;

        return result;
    }

    bool FileExporterRIS::writeKeyValue( QTextStream &stream, const QString& key, const QString&value )
    {
        stream << key << "  - ";
        if ( !value.isEmpty() )
            stream << value;
        stream << endl;
        qDebug( "%s  - %s", key.latin1(), value.latin1() );

        return TRUE;
    }

}
