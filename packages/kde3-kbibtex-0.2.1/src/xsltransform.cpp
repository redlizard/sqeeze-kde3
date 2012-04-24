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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include <qfile.h>
#include <qstring.h>

#include "xsltransform.h"

namespace BibTeX
{

    XSLTransform::XSLTransform( const QString& xsltFilename )
    {
        m_xlstStylesheet = xsltParseStylesheetFile(( const xmlChar* ) xsltFilename.latin1() );
        if ( m_xlstStylesheet == NULL )
            qDebug( "Could not load XSLT file '%s'.", xsltFilename.latin1() );
    }

    XSLTransform::~XSLTransform()
    {
        xsltFreeStylesheet( m_xlstStylesheet );
    }

    QString XSLTransform::transform( const QString& xmlText )
    {
        QString result = QString::null;
        QCString xmlCText = xmlText.utf8();
        xmlDocPtr document = xmlParseMemory( xmlCText, xmlCText.length() );
        if ( document )
        {
            if ( m_xlstStylesheet )
            {
                xmlDocPtr resultDocument = xsltApplyStylesheet( m_xlstStylesheet, document, NULL );
                if ( resultDocument )
                {
                    // Save the result into the QString
                    xmlChar * mem;
                    int size;
                    xmlDocDumpMemoryEnc( resultDocument, &mem, &size, "UTF-8" );
                    result = QString::fromUtf8( QCString(( char * )( mem ), size + 1 ) );
                    xmlFree( mem );

                    xmlFreeDoc( resultDocument );
                }
                else
                    qDebug( "Applying XSLT stylesheet to XML document failed" );
            }
            else
                qDebug( "XSLT stylesheet is not available or not valid" );

            xmlFreeDoc( document );
        }
        else
            qDebug( "XML document is not available or not valid" );

        return result;
    }

}
