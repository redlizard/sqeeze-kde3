/***************************************************************************
*   Copyright (C) 2004-2006 by Thomas Fischer                             *
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
#ifndef BIBTEXFILEEXPORTERXML_H
#define BIBTEXFILEEXPORTERXML_H

#include <element.h>
#include <fileexporter.h>
#include <value.h>

namespace BibTeX
{
    class Entry;
    class Macro;
    class Comment;

    /**
    @author Thomas Fischer
    */
    class FileExporterXML : public FileExporter
    {
    public:
        FileExporterXML();
        ~FileExporterXML();

        bool save( QIODevice* iodevice, const File* bibtexfile, QStringList *errorLog = NULL );
        bool save( QIODevice* iodevice, const Element* element, QStringList *errorLog = NULL );

    public slots:
        void cancel();

    private:
        bool m_cancelFlag;

        bool write( QTextStream&stream, const Element *element, const File* bibtexfile = NULL );
        bool writeEntry( QTextStream &stream, const Entry* entry );
        bool writeMacro( QTextStream &stream, const Macro *macro );
        bool writeComment( QTextStream &stream, const Comment *comment );

        QString valueToString( Value *value );
    };

}

#endif
