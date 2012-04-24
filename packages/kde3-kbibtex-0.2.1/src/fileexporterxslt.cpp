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
#include <qbuffer.h>

#include <fileexporterxml.h>
#include <xsltransform.h>

#include "fileexporterxslt.h"

namespace BibTeX
{

    FileExporterXSLT::FileExporterXSLT( XSLTransform *transformer )
            : FileExporter()
    {
        m_transformer = transformer;
        m_exporterXML = new FileExporterXML();
    }


    FileExporterXSLT::~FileExporterXSLT()
    {
        delete m_exporterXML;
    }

    bool FileExporterXSLT::save( QIODevice* iodevice, const File* bibtexfile, QStringList *errorLog )
    {
        QBuffer buffer;

        buffer.open( IO_WriteOnly );
        if ( m_exporterXML->save( &buffer, bibtexfile, errorLog ) )
        {
            buffer.close();
            buffer.open( IO_ReadOnly );
            QTextStream ts( &buffer );
            ts.setEncoding( QTextStream::UnicodeUTF8 );
            QString xml = ts.read();
            buffer.close();
            //             qDebug( "xml: %s", xml.latin1() );
            QString html = m_transformer->transform( xml );
            //             qDebug( "html: %s", html.latin1() );
            QTextStream htmlTS( iodevice );
            htmlTS.setEncoding( QTextStream::UnicodeUTF8 );
            htmlTS << html << endl;
            return TRUE;
        }
        else
            return FALSE;
    }

    bool FileExporterXSLT::save( QIODevice* iodevice, const Element* element, QStringList *errorLog )
    {
        QBuffer buffer;

        buffer.open( IO_WriteOnly );
        if ( m_exporterXML->save( &buffer, element, errorLog ) )
        {
            buffer.close();
            buffer.open( IO_ReadOnly );
            QTextStream xmlTS( &buffer );
            xmlTS.setEncoding( QTextStream::UnicodeUTF8 );
            QString xml = xmlTS.read();
            buffer.close();
            // qDebug( "xml: %s", xml.latin1() );
            QString html = m_transformer->transform( xml );
            // qDebug( "html: %s", html.latin1() );
            QTextStream output( iodevice );
            output.setEncoding( QTextStream::UnicodeUTF8 );
            output << html << endl;
            return TRUE;
        }
        else
            return FALSE;
    }

}
