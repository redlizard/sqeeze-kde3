/***************************************************************************
 *   Copyright (C) 2004-2005 by Thomas Fischer                             *
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBBIBUTILS

#ifndef BIBTEXFILEEXPORTERBIBUTILS_H
#define BIBTEXFILEEXPORTERBIBUTILS_H

#include <bibutils.h>

#include <fileexporter.h>

namespace BibTeX
{
    class FileExporterBibTeX;

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class FileExporterBibUtils : public FileExporter
    {
    public:
        enum OutputFormat {ofRIS = BIBL_RISOUT, ofISI = BIBL_ISIOUT, ofEndNote = BIBL_ENDNOTEOUT};

        FileExporterBibUtils( OutputFormat outputFormat );
        ~FileExporterBibUtils();

        bool save( QIODevice* iodevice, const Element* element, QStringList* errorLog = NULL );
        bool save( QIODevice* iodevice, const File* bibtexfile, QStringList* errorLog = NULL );

    public slots:
        void cancel();

    private:
        bool m_cancelFlag;
        QString m_workingDir;
        OutputFormat m_outputFormat;
        FileExporterBibTeX *m_bibTeXExporter;

        bool exportBib( const QString& bibFile, const QString& outputFile );
        bool fileToIODevice( const QString& fileName, QIODevice* iodevice );
        QString createTempDir();
        void deleteTempDir( const QString& directory );
    };

}

#endif // BIBTEXFILEEXPORTERBIBUTILS_H
#endif // HAVE_LIBBIBUTILS
