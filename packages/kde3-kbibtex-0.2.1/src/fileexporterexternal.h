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
#ifndef BIBTEXFILEEXPORTEREXTERNAL_H
#define BIBTEXFILEEXPORTEREXTERNAL_H

#include <fileexporter.h>
#include <settings.h>

class QWaitCondition;
class QProcess;
class QBuffer;

namespace BibTeX
{

    /**
    @author Thomas Fischer
    */
    class FileExporterExternal : public FileExporter
    {
        Q_OBJECT
    public:
        enum Exporter { exporterNone = 0, exporterXSLT = 1, exporterBib2XHTML = 2, exporterBibTeX2HTML = 3, exporterBibConv = 4 };

        FileExporterExternal( Exporter exporter, File::FileFormat fileformat );
        ~FileExporterExternal();

        bool save( QIODevice* iodevice, const File* bibtexfile, QStringList *errorLog = NULL );
        bool save( QIODevice* iodevice, const Element* element, QStringList *errorLog = NULL );

    private:
        QWaitCondition *wc;
        QProcess *process;
        QTextStream *writeTo;
        Exporter m_exporter;
        File::FileFormat m_fileformat;

        bool generateOutput( QBuffer &input, QIODevice* output );

    private slots:
        void slotProcessExited();
        void slotReadProcessOutput();
        void slotWroteToStdin();

    };

}

#endif
