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

#include "fileimporter.h"

namespace BibTeX
{

    FileImporter::FileImporter() : QObject()
    {
        // nothing
    }

    FileImporter::~FileImporter()
    {
        // nothing
    }

    File* FileImporter::load( const QString& text )
    {
        QBuffer buffer;
        buffer.open( IO_WriteOnly );
        QTextStream stream( &buffer );
        stream.setEncoding( QTextStream::UnicodeUTF8 );
        stream << text;
        buffer.close();

        buffer.open( IO_ReadOnly );
        File *result = load( &buffer );
        buffer.close();

        return result;
    }

}
#include "fileimporter.moc"
