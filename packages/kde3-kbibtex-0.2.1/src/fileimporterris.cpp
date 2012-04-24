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
#include <qapplication.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <entry.h>

#include "fileimporterris.h"

namespace BibTeX
{

    FileImporterRIS::FileImporterRIS() : FileImporter()
    {
        // TODO
    }


    FileImporterRIS::~FileImporterRIS()
    {
        // TODO
    }

    File* FileImporterRIS::load( QIODevice *iodevice )
    {
        cancelFlag = FALSE;
        m_refNr = 0;
        QTextStream textStream( iodevice );

        File *result = new File();
        QIODevice *streamDevice = textStream.device();
        while ( !cancelFlag && !textStream.atEnd() )
        {
            emit progress( streamDevice->at(), streamDevice->size() );
            qApp->processEvents();
            Element * element = nextElement( textStream );
            if ( element != NULL )
                result->appendElement( element );
            qApp->processEvents();
        }
        emit progress( 100, 100 );

        if ( cancelFlag )
        {
            delete result;
            result = NULL;
        }

        return result;
    }

    bool FileImporterRIS::guessCanDecode( const QString & text )
    {
        return text.find( "TY  - " ) >= 0;
    }

    void FileImporterRIS::cancel()
    {
        cancelFlag = TRUE;
    }

    Element *FileImporterRIS::nextElement( QTextStream &textStream )
    {
        RISitemList list = readElement( textStream );
        if ( list.empty() )
            return NULL;

        Entry::EntryType entryType = Entry::etMisc;
        Entry *entry = new Entry( entryType, QString( "RIS_%1" ).arg( m_refNr++ ) );
        QStringList authorList, editorList, keywordList;
        QString journalName, startPage, endPage, date;
        int fieldNr = 0;

        for ( RISitemList::iterator it = list.begin(); it != list.end(); ++it )
        {
            if (( *it ).key == "TY" )
            {
                if (( *it ).value == "BOOK" || ( *it ).value == "SER" )
                    entryType = Entry::etBook;
                else if (( *it ).value == "CHAP" )
                    entryType = Entry::etInBook;
                else if (( *it ).value == "CONF" )
                    entryType = Entry::etInProceedings;
                else if (( *it ).value == "JFULL" || ( *it ).value == "JOUR" || ( *it ).value == "MGZN" )
                    entryType = Entry::etArticle;
                else if (( *it ).value == "RPRT" )
                    entryType = Entry::etTechReport;
                else if (( *it ).value == "THES" )
                    entryType = Entry::etPhDThesis;
                entry->setEntryType( entryType );
            }
            else if (( *it ).key == "AU" || ( *it ).key == "A1" )
            {
                authorList.append(( *it ).value );
            }
            else if (( *it ).key == "ED" || ( *it ).key == "A2" )
            {
                editorList.append(( *it ).value );
            }
            //             else if ( ( *it ).key == "ID" )
            //             {
            //                 entry->setId( ( *it ).value );
            //             }
            else if (( *it ).key == "Y1" || ( *it ).key == "PY" )
            {
                date = ( *it ).value;
            }
            else if (( *it ).key == "Y2" )
            {
                if ( date.isEmpty() )
                    date = ( *it ).value;
            }
            else if (( *it ).key == "N1" || ( *it ).key == "N2" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftNote );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftNote );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "AB" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftAbstract );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftAbstract );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "KW" )
            {
                QString text = ( *it ).value;
                QRegExp splitRegExp;
                if ( text.contains( ";" ) )
                    splitRegExp = QRegExp( "\\s*[;\\n]\\s*" );
                else if ( text.contains( "," ) )
                    splitRegExp = QRegExp( "\\s*[,\\n]\\s*" );
                else
                    splitRegExp = QRegExp( "\\n" );
                QStringList newKeywords = QStringList::split( splitRegExp, text, FALSE );
                for ( QStringList::Iterator it = newKeywords.begin(); it != newKeywords.end();
                        ++it )
                    keywordList.append( *it );
            }
            else if (( *it ).key == "TI" || ( *it ).key == "T1" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftTitle );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftTitle );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "T3" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftSeries );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftSeries );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "JA" || ( *it ).key == "J1" || ( *it ).key == "J2" )
            {
                if ( journalName.isEmpty() )
                    journalName = ( *it ).value;
            }
            else if (( *it ).key == "JF" || ( *it ).key == "JO" )
            {
                journalName = ( *it ).value;
            }
            else if (( *it ).key == "VL" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftVolume );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftVolume );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "CP" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftVolume );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftChapter );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "IS" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftNumber );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftNumber );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "PB" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftPublisher );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftPublisher );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "SN" )
            {
                BibTeX::EntryField::FieldType fieldType = entryType == Entry::etBook || entryType == Entry::etInBook ? BibTeX::EntryField::ftISBN : BibTeX::EntryField::ftISSN;
                BibTeX::EntryField * field = entry->getField( fieldType );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( fieldType );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "CY" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftLocation );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftLocation );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "AD" )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftAddress );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftAddress );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "L1" )
            {
                BibTeX::EntryField * field = entry->getField( "PDF" );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( "PDF" );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "UR" )
            {
                BibTeX::EntryField * field = NULL;
                if (( *it ).value.contains( "dx.doi.org" ) )
                {
                    field = entry->getField( "DOI" );
                    if ( field == NULL )
                    {
                        field = new BibTeX::EntryField( "DOI" );
                        entry->addField( field );
                    }
                }
                else
                {
                    field = entry->getField( BibTeX::EntryField::ftURL );
                    if ( field == NULL )
                    {
                        field = new BibTeX::EntryField( BibTeX::EntryField::ftURL );
                        entry->addField( field );
                    }
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
            else if (( *it ).key == "SP" )
            {
                startPage = ( *it ).value;
            }
            else if (( *it ).key == "EP" )
            {
                endPage = ( *it ).value;
            }
            else
            {
                QString fieldName = QString( "RISfield_%1_%2" ).arg( fieldNr++ ).arg(( *it ).key.left( 2 ) );
                BibTeX::EntryField * field = new BibTeX::EntryField( fieldName );
                entry->addField( field );
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText(( *it ).value ) );
                field->setValue( value );
            }
        }

        if ( !authorList.empty() )
        {
            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftAuthor );
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( BibTeX::EntryField::ftAuthor );
                entry->addField( field );
            }
            BibTeX::Value *value = new BibTeX::Value();
            BibTeX::PersonContainer *container = new BibTeX::PersonContainer();
            value->items.append( container );
            for ( QStringList::Iterator pit = authorList.begin(); pit != authorList.end();++pit )
                container->persons.append( new BibTeX::Person( *pit ) );
            field->setValue( value );
        }

        if ( !editorList.empty() )
        {
            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftEditor );
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( BibTeX::EntryField::ftEditor );
                entry->addField( field );
            }
            BibTeX::Value *value = new BibTeX::Value();
            BibTeX::PersonContainer *container = new BibTeX::PersonContainer();
            value->items.append( container );
            for ( QStringList::Iterator pit = authorList.begin(); pit != authorList.end();++pit )
                container->persons.append( new BibTeX::Person( *pit ) );
            field->setValue( value );
        }

        if ( !keywordList.empty() )
        {
            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftKeywords );
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( BibTeX::EntryField::ftKeywords );
                entry->addField( field );
            }
            BibTeX::Value *value = new BibTeX::Value();
            BibTeX::KeywordContainer *container = new BibTeX::KeywordContainer();
            value->items.append( container );
            for ( QStringList::Iterator pit = keywordList.begin(); pit != keywordList.end();++pit )
                container->keywords.append( new BibTeX::Keyword( *pit ) );
            field->setValue( value );
        }

        if ( !journalName.isEmpty() )
        {
            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftJournal );
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( BibTeX::EntryField::ftJournal );
                entry->addField( field );
            }
            BibTeX::Value *value = new BibTeX::Value();
            value->items.append( new BibTeX::PlainText( journalName ) );
            field->setValue( value );
        }

        if ( !startPage.isEmpty() || !endPage.isEmpty() )
        {
            BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftPages );
            if ( field == NULL )
            {
                field = new BibTeX::EntryField( BibTeX::EntryField::ftPages );
                entry->addField( field );
            }
            QString page;
            if ( startPage.isEmpty() )
                page = endPage;
            else if ( endPage.isEmpty() )
                page = startPage;
            else
                page = startPage + QChar( 0x2013 ) + endPage;

            BibTeX::Value *value = new BibTeX::Value();
            value->items.append( new BibTeX::PlainText( page ) );
            field->setValue( value );
        }

        QStringList dateFragments = QStringList::split( "/", date );
        if ( dateFragments.count() > 0 )
        {
            bool ok;
            int year = dateFragments[ 0 ].toInt( &ok );
            if ( ok && year > 1000 && year < 3000 )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftYear );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftYear );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText( QString::number( year ) ) );
                field->setValue( value );
            }
        }
        if ( dateFragments.count() > 1 )
        {
            bool ok;
            int month = dateFragments[ 0 ].toInt( &ok );
            if ( ok && month > 0 && month < 13 )
            {
                BibTeX::EntryField * field = entry->getField( BibTeX::EntryField::ftMonth );
                if ( field == NULL )
                {
                    field = new BibTeX::EntryField( BibTeX::EntryField::ftMonth );
                    entry->addField( field );
                }
                BibTeX::Value *value = new BibTeX::Value();
                value->items.append( new BibTeX::PlainText( QString::number( month ) ) );
                field->setValue( value );
            }
        }

        return entry;
    }

    FileImporterRIS::RISitemList FileImporterRIS::readElement( QTextStream &textStream )
    {
        RISitemList result;
        QString line = textStream.readLine();
        while ( !line.startsWith( "TY  - " ) && !textStream.eof() )
            line = textStream.readLine();
        if ( textStream.eof() )
            return result;

        QString key, value;
        while ( !line.startsWith( "ER  -" ) && !textStream.eof() )
        {
            if ( line.mid( 2, 3 ) == "  -" )
            {
                if ( !value.isEmpty() )
                {
                    RISitem item;
                    item.key = key;
                    item.value = value;
                    result.append( item );
                }

                key = line.left( 2 );
                value = line.mid( 6 ).stripWhiteSpace();
            }
            else if ( line.length() > 1 )
                value += "\n" + line.stripWhiteSpace();

            line = textStream.readLine();
        }
        if ( !value.isEmpty() )
        {
            RISitem item;
            item.key = key;
            item.value = value;
            result.append( item );
        }

        return result;
    }
}
