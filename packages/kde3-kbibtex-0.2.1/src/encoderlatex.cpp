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
#include <qregexp.h>

#include <encoderlatex.h>

namespace BibTeX
{

    EncoderLaTeX *encoderLaTeX = NULL;

    static const struct EncoderLaTeXCommandMapping
    {
        const char *letters;
        unsigned int unicode;
    }
    commandmappingdatalatex[] =
    {
        {"AA", 0x00C5},
        {"AE", 0x00C6},
        {"ss", 0x00DF},
        {"aa", 0x00E5},
        {"ae", 0x00E6},
        {"OE", 0x0152},
        {"oe", 0x0153},
        {"L", 0x0141},
        {"l", 0x0142},
        {"grqq", 0x201C},
        {"glqq", 0x201E},
        {"frqq", 0x00BB},
        {"flqq", 0x00AB}
    };

    static const int commandmappingdatalatexcount = sizeof( commandmappingdatalatex ) / sizeof( commandmappingdatalatex[ 0 ] ) ;

    const char *expansionsCmd[] = {"\\{\\\\%1\\}", "\\\\%1\\{\\}", "\\\\%1"};
    static const  int expansionscmdcount = sizeof( expansionsCmd ) / sizeof( expansionsCmd[0] );

    static const struct EncoderLaTeXModCharMapping
    {
        const char *modifier;
        const char *letter;
        unsigned int unicode;
    }
    modcharmappingdatalatex[] =
    {
        {"\\\\`", "A", 0x00C0},
        {"\\\\'", "A", 0x00C1},
        {"\\\\\\^", "A", 0x00C21},
        {"\\\\~", "A", 0x00C3},
        {"\\\\\"", "A", 0x00C4},
        {"\\\\r", "A", 0x00C5},
        {"\\\\c", "C", 0x00C7},
        {"\\\\`", "E", 0x00C8},
        {"\\\\'", "E", 0x00C9},
        {"\\\\\\^", "E", 0x00CA},
        {"\\\\\"", "E", 0x00CB},
        {"\\\\'", "I", 0x00CD},
        {"\\\\\\^", "I", 0x00CE},
        {"\\\\~", "N", 0x00D1},
        {"\\\\`", "O", 0x00D2},
        {"\\\\'", "O", 0x00D3},
        {"\\\\\\^", "O", 0x00D4},
        {"\\\\\"", "O", 0x00D6},
        {"\\\\", "O", 0x00D8},
        {"\\\\`", "U", 0x00D9},
        {"\\\\'", "U", 0x00DA},
        {"\\\\\\^", "U", 0x00DB},
        {"\\\\\"", "U", 0x00DC},
        {"\\\\'", "Y", 0x00DD},
        {"\\\\\"", "s", 0x00DF},
        {"\\\\`", "a", 0x00E0},
        {"\\\\'", "a", 0x00E1},
        {"\\\\\\^", "a", 0x00E2},
        {"\\\\~", "a", 0x00E3},
        {"\\\\\"", "a", 0x00E4},
        {"\\\\r", "a", 0x00E5},
        {"\\\\c", "c", 0x00E7},
        {"\\\\`", "e", 0x00E8},
        {"\\\\'", "e", 0x00E9},
        {"\\\\\\^", "e", 0x00EA},
        {"\\\\\"", "e", 0x00EB},
        {"\\\\'", "i", 0x00ED},
        {"\\\\'", "\\\\i", 0x00ED},
        {"\\\\\\^", "i", 0x00EE},
        {"\\\\~", "n", 0x00F1},
        {"\\\\`", "o", 0x00F2},
        {"\\\\'", "o", 0x00F3},
        {"\\\\\\^", "o", 0x00F4},
        {"\\\\\"", "o", 0x00F6},
        {"\\\\", "o", 0x00F8},
        {"\\\\`", "u", 0x00F9},
        {"\\\\'", "u", 0x00FA},
        {"\\\\\\^", "u", 0x00FB},
        {"\\\\\"", "u", 0x00FC},
        {"\\\\'", "y", 0x00FD},
        {"\\\\u", "A", 0x0102},
        {"\\\\u", "a", 0x0103},
        {"\\\\'", "C", 0x0106},
        {"\\\\'", "c", 0x0107},
        {"\\\\v", "E", 0x010A},
        {"\\\\v", "e", 0x010B},
        {"\\\\v", "C", 0x010C},
        {"\\\\v", "c", 0x010D},
        {"\\\\c", "E", 0x0118},
        {"\\\\c", "e", 0x0119},
        {"\\\\u", "G", 0x011E},
        {"\\\\u", "g", 0x011F},
        {"\\\\'", "N", 0x0143},
        {"\\\\'", "n", 0x0144},
        {"\\\\H", "O", 0x0150},
        {"\\\\H", "o", 0x0151},
        {"\\\\c", "S", 0x015E},
        {"\\\\c", "s", 0x015F},
        {"\\\\v", "S", 0x0160},
        {"\\\\v", "s", 0x0161},
        {"\\\\r", "U", 0x016E},
        {"\\\\r", "u", 0x016F},
        {"\\\\v", "A", 0x01CD},
        {"\\\\v", "a", 0x01CE},
        {"\\\\v", "G", 0x01E6},
        {"\\\\v", "g", 0x01E7}
    };

    const char *expansionsMod1[] = {"\\{%1\\{%2\\}\\}", "\\{%1 %2\\}", "%1\\{%2\\}"};
    static const  int expansionsmod1count = sizeof( expansionsMod1 ) / sizeof( expansionsMod1[0] );
    const char *expansionsMod2[] = {"\\{%1%2\\}", "%1%2\\{\\}", "%1%2"};
    static const  int expansionsmod2count = sizeof( expansionsMod2 ) / sizeof( expansionsMod2[0] );

    static const int modcharmappingdatalatexcount = sizeof( modcharmappingdatalatex ) / sizeof( modcharmappingdatalatex[ 0 ] ) ;

    static const struct EncoderLaTeXCharMapping
    {
        const char *regexp;
        unsigned int unicode;
        const char *latex;
    }
    charmappingdatalatex[] =
    {
        {"\\\\#", 0x0023, "\\#"},
        /*{"\\\\&", 0x0026, "\\&"},*/
        {"!`", 0x00A1, "!`"},
        {"\"<", 0x00AB, "\"<"},
        {"\">", 0x00BB, "\">"},
        {"[?]`", 0x00BF, "?`"},
        {"--", 0x2013, "--"}
    };

    static const int charmappingdatalatexcount = sizeof( charmappingdatalatex ) / sizeof( charmappingdatalatex[ 0 ] ) ;

    EncoderLaTeX::EncoderLaTeX()
    {
        buildCharMapping();
    }

    EncoderLaTeX::~EncoderLaTeX()

    {
        // nothing
    }

    QString EncoderLaTeX::decode( const QString & text )
    {
        // split text into math and non-math regions
        QStringList intermediate = QStringList::split( '$', text, TRUE );
        QStringList::Iterator it = intermediate.begin();
        while ( it != intermediate.end() )
        {
            /**
             * Sometimes we split strings like "\$", which is not intended.
             * So, we have to manually fix things by checking for strings
             * ending with "\" and append both the removed dollar sign and
             * the following string (which was never supposed to be an
             * independent string). Finally, we remove the unnecessary
             * string and continue.
             */
            if (( *it ).endsWith( "\\" ) )
            {
                QStringList::Iterator cur = it;
                ++it;
                ( *cur ).append( '$' ).append( *it );
                intermediate.remove( it );
                it = cur;
            }
            else
                ++it;
        }

        if ( intermediate.size() > 100 )
            qDebug( "This BibTeX source code contains many inline math fragements (%d) which thwarts KBibTeX when parsing the code.", intermediate.size() );
        for ( QStringList::Iterator it = intermediate.begin(); it != intermediate.end(); it++ )
        {
            for ( QValueList<CharMappingItem>::ConstIterator cmit = m_charMapping.begin(); cmit != m_charMapping.end(); ++cmit )
                ( *it ).replace(( *cmit ).regExp, ( *cmit ).unicode );

            // skip math regions
            it++;

            if ( it == intermediate.end() )
                break;

            if (( *it ).length() > 256 )
                qDebug( "Very long math equation using $ found, maybe due to broken inline math: %s", ( *it ).left( 48 ).latin1() );
        }

        QString result = intermediate.join( "$" );

        // result.replace( "\\\\", "\\" );

        return result;
    }

    QString EncoderLaTeX::encode( const QString & text )

    {
        QString result = text;
        bool beginningQuotationNext = TRUE;

        // result.replace( "\\", "\\\\" );

        for ( QValueList<CharMappingItem>::ConstIterator it = m_charMapping.begin(); it != m_charMapping.end(); ++it )
            result.replace(( *it ).unicode, ( *it ).latex );

        for ( unsigned int i = 0; i < result.length(); i++ )
            if ( result.at( i ) == '"' && ( i == 0 || result.at( i - 1 ) != '\\' ) )
            {
                if ( beginningQuotationNext )
                    result.replace( i, 1, "``" );
                else
                    result.replace( i, 1, "''" );

                beginningQuotationNext = !beginningQuotationNext;
            }

        /** \url accepts unquotet &
           May introduce new problem tough */
        if ( result.contains( "\\url{" ) )
            result.replace( "\\&", "&" );

        return result;
    }

    QString EncoderLaTeX::encodeSpecialized( const QString & text, const EntryField::FieldType fieldType )

    {
        QString result = encode( text );

        switch ( fieldType )
        {
        case EntryField::ftPages:
            result.replace( QChar( 0x2013 ), "--" );

            break;

        default:
            break;
        }

        return result;
    }

    void EncoderLaTeX::buildCharMapping()
    {
        /** encoding and decoding for digraphs such as -- or ?` */
        for ( int i = 0; i < charmappingdatalatexcount; i++ )
        {
            CharMappingItem charMappingItem;
            charMappingItem.regExp = QRegExp( charmappingdatalatex[ i ].regexp );
            charMappingItem.unicode = QChar( charmappingdatalatex[ i ].unicode );
            charMappingItem.latex = QString( charmappingdatalatex[ i ].latex );
            m_charMapping.append( charMappingItem );
        }

        /** encoding and decoding for commands such as \AA or \ss */
        for ( int i = 0; i < commandmappingdatalatexcount; ++i )
        {
            /** different types of writing such as {\AA} or \AA{} possible */
            for ( int j = 0; j < expansionscmdcount; ++j )
            {
                CharMappingItem charMappingItem;
                charMappingItem.regExp = QRegExp( QString( expansionsCmd[j] ).arg( commandmappingdatalatex[i].letters ) );
                charMappingItem.unicode = QChar( commandmappingdatalatex[i].unicode );
                charMappingItem.latex = QString( "{\\%1}" ).arg( commandmappingdatalatex[i].letters );
                m_charMapping.append( charMappingItem );
            }
        }

        /** encoding and decoding for letters such as \"a */
        for ( int i = 0; i < modcharmappingdatalatexcount; ++i )
        {
            QString modifierRegExp = QString( modcharmappingdatalatex[i].modifier );
            QString modifier = modifierRegExp;
            modifier.replace( "\\\\", "\\" );

            /** first batch of replacement rules, where no separator is required between modifier and character (e.g. \"a) */
            if ( !modifierRegExp.at( modifierRegExp.length() - 1 ).isLetter() )
                for ( int j = 0; j < expansionsmod2count; ++j )
                {
                    CharMappingItem charMappingItem;
                    charMappingItem.regExp = QRegExp( QString( expansionsMod2[j] ).arg( modifierRegExp ).arg( modcharmappingdatalatex[i].letter ) );
                    charMappingItem.unicode = QChar( modcharmappingdatalatex[i].unicode );
                    charMappingItem.latex = QString( "{%1%2}" ).arg( modifier ).arg( modcharmappingdatalatex[i].letter );
                    m_charMapping.append( charMappingItem );
                }

            /** second batch of replacement rules, where a separator is required between modifier and character (e.g. \v{g}) */
            for ( int j = 0; j < expansionsmod1count; ++j )
            {
                CharMappingItem charMappingItem;
                charMappingItem.regExp = QRegExp( QString( expansionsMod1[j] ).arg( modifierRegExp ).arg( modcharmappingdatalatex[i].letter ) );
                charMappingItem.unicode = QChar( modcharmappingdatalatex[i].unicode );
                charMappingItem.latex = QString( "%1{%2}" ).arg( modifier ).arg( modcharmappingdatalatex[i].letter );
                m_charMapping.append( charMappingItem );
            }
        }
    }

    EncoderLaTeX* EncoderLaTeX::currentEncoderLaTeX()
    {
        if ( encoderLaTeX == NULL )
            encoderLaTeX = new EncoderLaTeX();

        return encoderLaTeX;
    }

    void EncoderLaTeX::deleteCurrentEncoderLaTeX()
    {
        if ( encoderLaTeX != NULL )
        {
            delete encoderLaTeX;
            encoderLaTeX = NULL;
        }
    }

}
