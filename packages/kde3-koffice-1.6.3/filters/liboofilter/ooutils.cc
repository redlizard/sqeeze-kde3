/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (c) 2003 Lukas Tinkl <lukas@kde.org>
   Copyright (c) 2003 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "ooutils.h"
#include <KoDocument.h>
#include <KoStyleStack.h>
#include <qdom.h>
#include <qcolor.h>
#include <qimage.h>
#include <KoUnit.h>
#include <qregexp.h>
#include <kdebug.h>
#include <kzip.h>
#include <KoDom.h>
#include <qxml.h>

const char* const ooNS::office="http://openoffice.org/2000/office";
const char* const ooNS::style="http://openoffice.org/2000/style";
const char* const ooNS::text="http://openoffice.org/2000/text";
const char* const ooNS::table="http://openoffice.org/2000/table";
const char* const ooNS::draw="http://openoffice.org/2000/drawing";
const char* const ooNS::presentation="http://openoffice.org/2000/presentation";
const char* const ooNS::fo="http://www.w3.org/1999/XSL/Format";
const char* const ooNS::xlink="http://www.w3.org/1999/xlink";
const char* const ooNS::number="http://openoffice.org/2000/datastyle";
const char* const ooNS::svg="http://www.w3.org/2000/svg";
const char* const ooNS::dc="http://purl.org/dc/elements/1.1/";
const char* const ooNS::meta="http://openoffice.org/2000/meta";
const char* const ooNS::config="http://openoffice.org/2001/config";

QString OoUtils::expandWhitespace(const QDomElement& tag)
{
    //tags like <text:s text:c="4">

    int howmany=1;
    if (tag.hasAttributeNS( ooNS::text, "c"))
        howmany = tag.attributeNS( ooNS::text, "c", QString::null).toInt();

    QString result;
    return result.fill(32, howmany);
}

bool OoUtils::parseBorder(const QString & tag, double * width, int * style, QColor * color)
{
    //string like "0.088cm solid #800000"

    if (tag.isEmpty() || tag=="none" || tag=="hidden") // in fact no border
        return false;

    QString _width = tag.section(' ', 0, 0);
    QString _style = tag.section(' ', 1, 1);
    QString _color = tag.section(' ', 2, 2);

    *width = KoUnit::parseValue(_width, 1.0);

    if ( _style == "dashed" )
        *style = 1;
    else if ( _style == "dotted" )
        *style = 2;
    else if ( _style == "dot-dash" ) // not in xsl/fo, but in OASIS (in other places)
        *style = 3;
    else if ( _style == "dot-dot-dash" ) // not in xsl/fo, but in OASIS (in other places)
        *style = 4;
    else if ( _style == "double" )
        *style = 5;
    else
        *style = 0;

    if (_color.isEmpty())
        *color = QColor();
    else
        color->setNamedColor(_color);

    return true;
}

void OoUtils::importIndents( QDomElement& parentElement, const KoStyleStack& styleStack )
{
    if ( styleStack.hasAttributeNS( ooNS::fo, "margin-left" ) || // 3.11.19
         styleStack.hasAttributeNS( ooNS::fo, "margin-right" ) )
         // *text-indent must always be bound to either margin-left or margin-right
    {
        double marginLeft = KoUnit::parseValue( styleStack.attributeNS( ooNS::fo, "margin-left" ) );
        double marginRight = KoUnit::parseValue( styleStack.attributeNS( ooNS::fo, "margin-right" ) );
        double first = 0;
        if (styleStack.attributeNS( ooNS::style, "auto-text-indent") == "true") // style:auto-text-indent takes precedence
            // ### "indented by a value that is based on the current font size"
            // ### and "requires margin-left and margin-right
            // ### but how much is the indent?
            first = 10;
        else if (styleStack.hasAttributeNS( ooNS::fo, "text-indent"))
            first = KoUnit::parseValue( styleStack.attributeNS( ooNS::fo, "text-indent"));

        if ( marginLeft != 0 || marginRight != 0 || first != 0 )
        {
            QDomElement indent = parentElement.ownerDocument().createElement( "INDENTS" );
            if( marginLeft != 0 )
                indent.setAttribute( "left", marginLeft );
            if( marginRight != 0 )
                indent.setAttribute( "right", marginRight );
            if( first != 0 )
                indent.setAttribute( "first", first );
            parentElement.appendChild( indent );
        }
    }
}

void OoUtils::importLineSpacing( QDomElement& parentElement, const KoStyleStack& styleStack )
{
    if( styleStack.hasAttributeNS( ooNS::fo, "line-height") )
    {
        // Fixed line height
        QString value = styleStack.attributeNS( ooNS::fo, "line-height" ); // 3.11.1
        if ( value != "normal" )
        {
            QDomElement lineSpacing = parentElement.ownerDocument().createElement( "LINESPACING" );

            if ( value.endsWith("%" ) )
            {
                double percent = value.left(value.length()-1).toDouble();
                if( percent == 100 )
                    lineSpacing.setAttribute("type","single");
                else if( percent == 150 )
                    lineSpacing.setAttribute("type","oneandhalf");
                else if( percent == 200 )
                    lineSpacing.setAttribute("type","double");
                else
                {
                  lineSpacing.setAttribute("type", "multiple");
                  lineSpacing.setAttribute("spacingvalue", percent/100);
                }
            }
            else // fixed value (TODO use KoUnit::parseValue to get it in pt)
            {
                kdWarning(30519) << "Unhandled value for fo:line-height: " << value << endl;
                lineSpacing.setAttribute("type","single"); // fallback
            }
            parentElement.appendChild( lineSpacing );
        }
    }
    // Line-height-at-least is mutually exclusive with line-height
    else if ( styleStack.hasAttributeNS( ooNS::style, "line-height-at-least") ) // 3.11.2
    {
        QString value = styleStack.attributeNS( ooNS::style, "line-height-at-least" );
        // kotext has "at least" but that's for the linespacing, not for the entire line height!
        // Strange. kotext also has "at least" for the whole line height....
        // Did we make the wrong choice in kotext?
        //kdWarning(30519) << "Unimplemented support for style:line-height-at-least: " << value << endl;
        // Well let's see if this makes a big difference.
        QDomElement lineSpacing = parentElement.ownerDocument().createElement("LINESPACING");
        lineSpacing.setAttribute("type", "atleast");
        lineSpacing.setAttribute("spacingvalue", KoUnit::parseValue(value));
        parentElement.appendChild(lineSpacing);
    }
    // Line-spacing is mutually exclusive with line-height and line-height-at-least
    else if ( styleStack.hasAttributeNS( ooNS::style, "line-spacing") ) // 3.11.3
    {
        double value = KoUnit::parseValue( styleStack.attributeNS( ooNS::style, "line-spacing" ) );
        if ( value != 0.0 )
        {
            QDomElement lineSpacing = parentElement.ownerDocument().createElement( "LINESPACING" );
            lineSpacing.setAttribute( "type", "custom" );
            lineSpacing.setAttribute( "spacingvalue", value );
            parentElement.appendChild( lineSpacing );
        }
    }

}

void OoUtils::importTopBottomMargin( QDomElement& parentElement, const KoStyleStack& styleStack )
{
    if( styleStack.hasAttributeNS( ooNS::fo, "margin-top") || // 3.11.22
        styleStack.hasAttributeNS( ooNS::fo, "margin-bottom"))
    {
        double mtop = KoUnit::parseValue( styleStack.attributeNS( ooNS::fo, "margin-top" ) );
        double mbottom = KoUnit::parseValue( styleStack.attributeNS( ooNS::fo, "margin-bottom" ) );
        if( mtop != 0 || mbottom != 0 )
        {
            QDomElement offset = parentElement.ownerDocument().createElement( "OFFSETS" );
            if( mtop != 0 )
                offset.setAttribute( "before", mtop );
            if( mbottom != 0 )
                offset.setAttribute( "after", mbottom );
            parentElement.appendChild( offset );
        }
    }
}

void OoUtils::importTabulators( QDomElement& parentElement, const KoStyleStack& styleStack )
{
    if ( !styleStack.hasChildNodeNS( ooNS::style, "tab-stops" ) ) // 3.11.10
        return;
    QDomElement tabStops = styleStack.childNodeNS( ooNS::style, "tab-stops" );
    //kdDebug(30519) << k_funcinfo << tabStops.childNodes().count() << " tab stops in layout." << endl;
    for ( QDomNode it = tabStops.firstChild(); !it.isNull(); it = it.nextSibling() )
    {
        QDomElement tabStop = it.toElement();
        Q_ASSERT( tabStop.tagName() == "style:tab-stop" );
        QString type = tabStop.attributeNS( ooNS::style, "type", QString::null ); // left, right, center or char

        QDomElement elem = parentElement.ownerDocument().createElement( "TABULATOR" );
        int kOfficeType = 0;
        if ( type == "left" )
            kOfficeType = 0;
        else if ( type == "center" )
            kOfficeType = 1;
        else if ( type == "right" )
            kOfficeType = 2;
        else if ( type == "char" ) {
            QString delimiterChar = tabStop.attributeNS( ooNS::style, "char", QString::null ); // single character
            elem.setAttribute( "alignchar", delimiterChar );
            kOfficeType = 3; // "alignment on decimal point"
        }

        elem.setAttribute( "type", kOfficeType );

        double pos = KoUnit::parseValue( tabStop.attributeNS( ooNS::style, "position", QString::null ) );
        elem.setAttribute( "ptpos", pos );

        // TODO Convert leaderChar's unicode value to the KOffice enum
        // (blank/dots/line/dash/dash-dot/dash-dot-dot, 0 to 5)
        QString leaderChar = tabStop.attributeNS( ooNS::style, "leader-char", QString::null ); // single character
        if ( !leaderChar.isEmpty() )
        {
            int filling = 0;
            QChar ch = leaderChar[0];
            switch (ch.latin1()) {
            case '.':
                filling = 1; break;
            case '-':
            case '_':  // TODO in KWord: differentiate --- and ___
                filling = 2; break;
            default:
                // KWord doesn't have support for "any char" as filling.
                // Instead it has dash-dot and dash-dot-dot - but who uses that in a tabstop?
                break;
            }
            elem.setAttribute( "filling", filling );
        }
        parentElement.appendChild( elem );
    }

}

void OoUtils::importBorders( QDomElement& parentElement, const KoStyleStack& styleStack )
{
    if (styleStack.hasAttributeNS( ooNS::fo, "border","left"))
    {
        double width;
        int style;
        QColor color;
        if (OoUtils::parseBorder(styleStack.attributeNS( ooNS::fo, "border", "left"), &width, &style, &color))
        {
            QDomElement lbElem = parentElement.ownerDocument().createElement("LEFTBORDER");
            lbElem.setAttribute("width", width);
            lbElem.setAttribute("style", style);
            if (color.isValid()) {
                lbElem.setAttribute("red", color.red());
                lbElem.setAttribute("green", color.green());
                lbElem.setAttribute("blue", color.blue());
            }
            parentElement.appendChild(lbElem);
        }
    }

    if (styleStack.hasAttributeNS( ooNS::fo, "border", "right"))
    {
        double width;
        int style;
        QColor color;
        if (OoUtils::parseBorder(styleStack.attributeNS( ooNS::fo, "border", "right"), &width, &style, &color))
        {
            QDomElement lbElem = parentElement.ownerDocument().createElement("RIGHTBORDER");
            lbElem.setAttribute("width", width);
            lbElem.setAttribute("style", style);
            if (color.isValid()) {
                lbElem.setAttribute("red", color.red());
                lbElem.setAttribute("green", color.green());
                lbElem.setAttribute("blue", color.blue());
            }
            parentElement.appendChild(lbElem);
        }
    }

    if (styleStack.hasAttributeNS( ooNS::fo, "border", "top"))
    {
        double width;
        int style;
        QColor color;
        if (OoUtils::parseBorder(styleStack.attributeNS( ooNS::fo, "border", "top"), &width, &style, &color))
        {
            QDomElement lbElem = parentElement.ownerDocument().createElement("TOPBORDER");
            lbElem.setAttribute("width", width);
            lbElem.setAttribute("style", style);
            if (color.isValid()) {
                lbElem.setAttribute("red", color.red());
                lbElem.setAttribute("green", color.green());
                lbElem.setAttribute("blue", color.blue());
            }
            parentElement.appendChild(lbElem);
        }
    }

    if (styleStack.hasAttributeNS( ooNS::fo, "border", "bottom"))
    {
        double width;
        int style;
        QColor color;
        if (OoUtils::parseBorder(styleStack.attributeNS( ooNS::fo, "border", "bottom"), &width, &style, &color))
        {
            QDomElement lbElem = parentElement.ownerDocument().createElement("BOTTOMBORDER");
            lbElem.setAttribute("width", width);
            lbElem.setAttribute("style", style);
            if (color.isValid()) {
                lbElem.setAttribute("red", color.red());
                lbElem.setAttribute("green", color.green());
                lbElem.setAttribute("blue", color.blue());
            }
            parentElement.appendChild(lbElem);
        }
    }
}

void OoUtils::importUnderline( const QString& in, QString& underline, QString& styleline )
{
    underline = "single";
    if ( in == "none" )
        underline = "0";
    else if ( in == "single" )
        styleline = "solid";
    else if ( in == "double" )
    {
        underline = in;
        styleline = "solid";
    }
    else if ( in == "dotted" || in == "bold-dotted" ) // bold-dotted not in libkotext
        styleline = "dot";
    else if ( in == "dash"
              // those are not in libkotext:
              || in == "long-dash"
              || in == "bold-dash"
              || in == "bold-long-dash"
        )
        styleline = "dash";
    else if ( in == "dot-dash"
              || in == "bold-dot-dash") // not in libkotext
        styleline = "dashdot"; // tricky ;)
    else if ( in == "dot-dot-dash"
              || in == "bold-dot-dot-dash") // not in libkotext
        styleline = "dashdotdot"; // this is getting fun...
    else if ( in == "wave"
              || in == "bold-wave" // not in libkotext
              || in == "double-wave" // not in libkotext
              || in == "small-wave" ) // not in libkotext
    {
        underline = in;
        styleline = "solid";
    }
    else if( in == "bold" )
    {
        underline = "single-bold";
        styleline = "solid";
    }
    else
        kdWarning(30519) << k_funcinfo << " unsupported text-underline value: " << in << endl;
}

void OoUtils::importTextPosition( const QString& text_position, QString& value, QString& relativetextsize )
{
    //OO: <vertical position (% or sub or super)> [<size as %>]
    //Examples: "super" or "super 58%" or "82% 58%" (where 82% is the vertical position)
    // TODO in kword: vertical positions other than sub/super
    QStringList lst = QStringList::split( ' ', text_position );
    if ( !lst.isEmpty() )
    {
        QString textPos = lst.front().stripWhiteSpace();
        QString textSize;
        lst.pop_front();
        if ( !lst.isEmpty() )
            textSize = lst.front().stripWhiteSpace();
        if ( !lst.isEmpty() )
            kdWarning(30519) << "Strange text position: " << text_position << endl;
        bool super = textPos == "super";
        bool sub = textPos == "sub";
        if ( textPos.endsWith("%") )
        {
            textPos.truncate( textPos.length() - 1 );
            // This is where we interpret the text position into kotext's simpler
            // "super" or "sub".
            double val = textPos.toDouble();
            if ( val > 0 )
                super = true;
            else if ( val < 0 )
                sub = true;
        }
        if ( super )
            value = "2";
        else if ( sub )
            value = "1";
        else
            value = "0";
        if ( !textSize.isEmpty() && textSize.endsWith("%") )
        {
            textSize.truncate( textSize.length() - 1 );
            double textSizeValue = textSize.toDouble() / 100; // e.g. 0.58
            relativetextsize = QString::number( textSizeValue );
        }
    }
    else
        value = "0";
}

void OoUtils::createDocumentInfo(QDomDocument &_meta, QDomDocument & docinfo)
{
    QDomNode meta   = KoDom::namedItemNS( _meta, ooNS::office, "document-meta" );
    QDomNode office = KoDom::namedItemNS( meta, ooNS::office, "meta" );

    if ( office.isNull() )
        return;
    QDomElement elementDocInfo  = docinfo.documentElement();

    QDomElement e = KoDom::namedItemNS( office, ooNS::dc, "creator" );
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        QDomElement author = docinfo.createElement( "author" );
        QDomElement t = docinfo.createElement( "full-name" );
        author.appendChild( t );
        t.appendChild( docinfo.createTextNode( e.text() ) );
        elementDocInfo.appendChild( author);
    }

    e = KoDom::namedItemNS( office, ooNS::dc, "title" );
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        QDomElement about = docinfo.createElement( "about" );
        QDomElement title = docinfo.createElement( "title" );
        about.appendChild( title );
        title.appendChild( docinfo.createTextNode( e.text() ) );
        elementDocInfo.appendChild( about );
    }

    e = KoDom::namedItemNS( office, ooNS::dc, "description" );
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        QDomElement about = elementDocInfo.namedItem( "about" ).toElement();
        if ( about.isNull() ) {
            about = docinfo.createElement( "about" );
            elementDocInfo.appendChild( about );
        }
        QDomElement title = docinfo.createElement( "abstract" );
        about.appendChild( title );
        title.appendChild( docinfo.createTextNode( e.text() ) );
    }
    e = KoDom::namedItemNS( office, ooNS::dc, "subject" );
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        QDomElement about = elementDocInfo.namedItem( "about" ).toElement();
        if ( about.isNull() ) {
            about = docinfo.createElement( "about" );
            elementDocInfo.appendChild( about );
        }
        QDomElement subject = docinfo.createElement( "subject" );
        about.appendChild( subject );
        subject.appendChild( docinfo.createTextNode( e.text() ) );
    }
    e = KoDom::namedItemNS( office, ooNS::meta, "keywords" );
    if ( !e.isNull() )
    {
        QDomElement about = elementDocInfo.namedItem( "about" ).toElement();
        if ( about.isNull() ) {
            about = docinfo.createElement( "about" );
            elementDocInfo.appendChild( about );
        }
        QDomElement tmp = KoDom::namedItemNS( e, ooNS::meta, "keyword" );
        if ( !tmp.isNull() && !tmp.text().isEmpty() )
        {
            QDomElement keyword = docinfo.createElement( "keyword" );
            about.appendChild( keyword );
            keyword.appendChild( docinfo.createTextNode( tmp.text() ) );
        }
    }
}

KoFilter::ConversionStatus OoUtils::loadAndParse(const QString& fileName, QDomDocument& doc, KoStore *m_store )
{
    kdDebug(30518) << "loadAndParse: Trying to open " << fileName << endl;

    if (!m_store->open(fileName))
    {
        kdWarning(30519) << "Entry " << fileName << " not found!" << endl;
        return KoFilter::FileNotFound;
    }
    KoFilter::ConversionStatus convertStatus = loadAndParse( m_store->device(),doc, fileName );
    m_store->close();
    return convertStatus;

}

KoFilter::ConversionStatus OoUtils::loadAndParse(QIODevice* io, QDomDocument& doc, const QString & fileName)
{
    QXmlInputSource source( io );
    // Copied from QDomDocumentPrivate::setContent, to change the whitespace thing
    QXmlSimpleReader reader;
    KoDocument::setupXmlReader( reader, true /*namespaceProcessing*/ );

    // Error variables for QDomDocument::setContent
    QString errorMsg;
    int errorLine, errorColumn;
    if ( !doc.setContent( &source, &reader, &errorMsg, &errorLine, &errorColumn ) )
    {
        kdError(30519) << "Parsing error in " << fileName << "! Aborting!" << endl
            << " In line: " << errorLine << ", column: " << errorColumn << endl
            << " Error message: " << errorMsg << endl;
        return KoFilter::ParsingError;
    }

    kdDebug(30519) << "File " << fileName << " loaded and parsed!" << endl;

    return KoFilter::OK;

}

KoFilter::ConversionStatus OoUtils::loadAndParse(const QString& filename, QDomDocument& doc, KZip * m_zip)
{
    kdDebug(30519) << "Trying to open " << filename << endl;

    if (!m_zip)
    {
        kdError(30519) << "No ZIP file!" << endl;
        return KoFilter::CreationError; // Should not happen
    }

    const KArchiveEntry* entry = m_zip->directory()->entry( filename );
    if (!entry)
    {
        kdWarning(30519) << "Entry " << filename << " not found!" << endl;
        return KoFilter::FileNotFound;
    }
    if (entry->isDirectory())
    {
        kdWarning(30519) << "Entry " << filename << " is a directory!" << endl;
        return KoFilter::WrongFormat;
    }
    const KZipFileEntry* f = static_cast<const KZipFileEntry *>(entry);
    kdDebug(30519) << "Entry " << filename << " has size " << f->size() << endl;
    QIODevice* io = f->device();
    KoFilter::ConversionStatus convertStatus = loadAndParse( io,doc, filename );
    delete io;
    return convertStatus;
}

KoFilter::ConversionStatus OoUtils::loadThumbnail( QImage& thumbnail, KZip * m_zip )
{
    const QString filename( "Thumbnails/thumbnail.png" );
    kdDebug(30519) << "Trying to open thumbnail " << filename << endl;

    if (!m_zip)
    {
        kdError(30519) << "No ZIP file!" << endl;
        return KoFilter::CreationError; // Should not happen
    }

    const KArchiveEntry* entry = m_zip->directory()->entry( filename );
    if (!entry)
    {
        kdWarning(30519) << "Entry " << filename << " not found!" << endl;
        return KoFilter::FileNotFound;
    }
    if (entry->isDirectory())
    {
        kdWarning(30519) << "Entry " << filename << " is a directory!" << endl;
        return KoFilter::WrongFormat;
    }
    const KZipFileEntry* f = static_cast<const KZipFileEntry *>(entry);
    QIODevice* io=f->device();
    kdDebug(30519) << "Entry " << filename << " has size " << f->size() << endl;

    if ( ! io->open( IO_ReadOnly ) )
    {
        kdWarning(30519) << "Thumbnail could not be opened!" <<endl;
        delete io;
        return KoFilter::StupidError;
    }

    QImageIO imageIO( io, "PNG" );
    if ( ! imageIO.read() )
    {
        kdWarning(30519) << "Thumbnail could not be read!" <<endl;
        delete io;
        return KoFilter::StupidError;
    }

    io->close();

    thumbnail = imageIO.image();

    if ( thumbnail.isNull() )
    {
        kdWarning(30519) << "Read thumbnail is null!" <<endl;
        delete io;
        return KoFilter::StupidError;
    }

    delete io;

    kdDebug(30519) << "File " << filename << " loaded!" << endl;

    return KoFilter::OK;
}
