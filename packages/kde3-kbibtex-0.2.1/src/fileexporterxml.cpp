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
#include <qstringlist.h>

#include <file.h>
#include <element.h>
#include <entry.h>
#include <encoderxml.h>
#include <macro.h>
#include <comment.h>
#include "fileexporterxml.h"

namespace BibTeX
{

    FileExporterXML::FileExporterXML() : FileExporter()
    {
        // nothing
    }


    FileExporterXML::~FileExporterXML()
    {
        // nothing
    }

    bool FileExporterXML::save( QIODevice* iodevice, const File* bibtexfile, QStringList * /*errorLog*/ )
    {
        bool result = TRUE;
        m_cancelFlag = FALSE;
        QTextStream stream( iodevice );
        stream.setEncoding( QTextStream::UnicodeUTF8 );

        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        stream << "<bibliography>" << endl;

        for ( File::ElementList::const_iterator it = bibtexfile->elements.begin(); it != bibtexfile->elements.end() && result && !m_cancelFlag; it++ )
        {
            Element *element = *it;
            write( stream, element, bibtexfile );
        }

        stream << "</bibliography>" << endl;

        return result && !m_cancelFlag;
    }

    bool FileExporterXML::save( QIODevice* iodevice, const Element* element, QStringList * /*errorLog*/ )
    {
        QTextStream stream( iodevice );
        stream.setEncoding( QTextStream::UnicodeUTF8 );

        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        return write( stream, element );
    }

    void FileExporterXML::cancel()
    {
        m_cancelFlag = TRUE;
    }

    bool FileExporterXML::write( QTextStream&stream, const Element *element, const File* bibtexfile )
    {
        bool result = FALSE;

        const Entry *entry = dynamic_cast<const Entry*>( element );
        if ( entry != NULL )
        {
            if ( bibtexfile != NULL )
            {
                entry = bibtexfile->completeReferencedFieldsConst( entry );
            }
            result |= writeEntry( stream, entry );
            if ( bibtexfile != NULL )
                delete entry;
        }
        else
        {
            const Macro * macro = dynamic_cast<const Macro*>( element );
            if ( macro != NULL )
                result |= writeMacro( stream, macro );
            else
            {
                const Comment * comment = dynamic_cast<const Comment*>( element );
                if ( comment != NULL )
                    result |= writeComment( stream, comment );
                else
                {
                    // preambles are ignored, make no sense in XML files
                }
            }
        }

        return result;
    }

    bool FileExporterXML::writeEntry( QTextStream &stream, const Entry* entry )
    {
        stream << " <entry id=\"" << entry->id() << "\" type=\"" << entry->entryTypeString().lower() << "\">" << endl;
        for ( Entry::EntryFields::const_iterator it = entry->begin(); it != entry->end(); it++ )
        {
            EntryField *field = *it;
            switch ( field->fieldType() )
            {
            case EntryField::ftAuthor:
            case EntryField::ftEditor:
                {
                    QString tag = field->fieldTypeName().lower();
                    stream << "  <" << tag << "s>" << endl;
                    QStringList persons = QStringList::split( QRegExp( "\\s+(,|and|&)+\\s+", FALSE ), EncoderXML::currentEncoderXML() ->encode( valueToString( field->value() ) ) );
                    for ( QStringList::Iterator it = persons.begin(); it != persons.end(); it++ )
                        stream << "   <person>" << *it << "</person>" << endl;
                    stream << "  </" << tag << "s>" << endl;
                }
                break;
            case EntryField::ftMonth:
                {
                    stream << "  <month";
                    bool ok = FALSE;

                    int month = -1;
                    QString tag = "";
                    QString content = "";
                    for ( QValueList<ValueItem*>::ConstIterator it = field->value()->items.begin(); it != field->value()->items.end(); ++it )
                    {
                        if ( dynamic_cast<MacroKey*>( *it ) != NULL )
                        {
                            for ( int i = 0; i < 12; i++ )
                                if ( QString::compare(( *it )->text(), MonthsTriple[ i ] ) == 0 )
                                {
                                    if ( month < 1 )
                                    {
                                        tag = MonthsTriple[ i ];
                                        month = i + 1;
                                    }
                                    content.append( Months[ i ] );
                                    ok = TRUE;
                                    break;
                                }
                        }
                        else
                            content.append( EncoderXML::currentEncoderXML() ->encode(( *it )->text() ) );
                    }

                    if ( !ok )
                        content = EncoderXML::currentEncoderXML() ->encode( field->value()->simplifiedText() ) ;
                    if ( !tag.isEmpty() )
                        stream << " tag=\"" << tag << "\"";
                    if ( month > 0 )
                        stream << " month=\"" << month << "\"";
                    stream << '>' << content;
                    stream << "</month>" << endl;
                }
                break;
            default:
                {
                    QString tag = field->fieldTypeName().lower();
                    stream << "  <" << tag << ">" << EncoderXML::currentEncoderXML() ->encode( valueToString( field->value() ) ) << "</" << tag << ">" << endl;
                }
                break;
            }

        }
        stream << " </entry>" << endl;

        return TRUE;
    }

    bool FileExporterXML::writeMacro( QTextStream &stream, const Macro *macro )
    {
        stream << " <string key=\"" << macro->key() << "\">";
        stream << EncoderXML::currentEncoderXML() ->encode( valueToString( macro->value() ) );
        stream << "</string>" << endl;

        return TRUE;
    }

    bool FileExporterXML::writeComment( QTextStream &stream, const Comment *comment )
    {
        stream << " <comment>" ;
        stream << EncoderXML::currentEncoderXML() ->encode( comment->text() );
        stream << "</comment>" << endl;

        return TRUE;
    }

    QString FileExporterXML::valueToString( Value *value )
    {
        QString result;
        bool isFirst = TRUE;

        for ( QValueList<ValueItem*>::ConstIterator it = value->items.begin(); it != value->items.end(); it++ )
        {
            if ( !isFirst )
                result.append( ' ' );
            isFirst = FALSE;

            result.append(( *it ) ->simplifiedText() );
        }

        return result;
    }


}
