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

#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>

#include <fileexporterbibtex.h>
#include "fileexporterbibutils.h"

namespace BibTeX
{

    FileExporterBibUtils::FileExporterBibUtils( OutputFormat outputFormat )
            : FileExporter(), m_workingDir( createTempDir() ), m_outputFormat( outputFormat ), m_bibTeXExporter( new FileExporterBibTeX() )
    {
        // nothing
    }

    FileExporterBibUtils::~FileExporterBibUtils()
    {
        deleteTempDir( m_workingDir );
        delete m_bibTeXExporter;
    }

    bool FileExporterBibUtils::save( QIODevice* iodevice, const Element* element, QStringList* errorLog )
    {
        QString bibFileName = QString( m_workingDir ).append( "/input.bib" );
        QFile bibFile( bibFileName );
        if ( bibFile.open( IO_WriteOnly ) )
        {
            m_bibTeXExporter->save( &bibFile, element, errorLog );
            bibFile.close();
        }
        else
            return FALSE;

        QString outputFileName = QString( m_workingDir ).append( "/input.tmp" );
        if ( !exportBib( bibFileName, outputFileName ) )
            return FALSE;

        if ( !fileToIODevice( outputFileName, iodevice ) )
            return FALSE;

        return TRUE;
    }

    bool FileExporterBibUtils::save( QIODevice* iodevice, const File* bibtexfile, QStringList* errorLog )
    {
        QString bibFileName = QString( m_workingDir ).append( "/input.bib" );
        QFile bibFile( bibFileName );
        if ( bibFile.open( IO_WriteOnly ) )
        {
            m_bibTeXExporter->save( &bibFile, bibtexfile, errorLog );
            bibFile.close();
        }
        else
            return FALSE;

        QString outputFileName = QString( m_workingDir ).append( "/input.tmp" );
        if ( !exportBib( bibFileName, outputFileName ) )
            return FALSE;

        if ( !fileToIODevice( outputFileName, iodevice ) )
            return FALSE;

        return TRUE;
    }

    void FileExporterBibUtils::cancel()
    {
        m_bibTeXExporter->cancel();
        m_cancelFlag = TRUE;
    }

    bool FileExporterBibUtils::exportBib( const QString& bibFileName, const QString& outputFileName )
    {
        param p;
        bibl b;
        bibl_init( &b );
        bibl_initparams( &p, BIBL_BIBTEXIN, m_outputFormat );
        p.verbose = 3;
        FILE *fp = fopen( bibFileName.ascii(), "r" );
        int err = BIBL_ERR_BADINPUT;
        if ( fp )
        {
            err = bibl_read( &b, fp, bibFileName.ascii(), BIBL_BIBTEXIN, &p );
            fclose( fp );
        }
        if ( err != BIBL_OK )
            return FALSE;
        qDebug( "Num read references: %ld\n", b.nrefs );

        fp = fopen( outputFileName.ascii(), "w" );
        err = -1;
        if ( fp != NULL )
        {
            err = bibl_write( &b, fp, m_outputFormat, &p );
            fclose( fp );
        }
        bibl_free( &b );
        if ( err != BIBL_OK )
            return FALSE;

        return TRUE;
    }

    bool FileExporterBibUtils::fileToIODevice( const QString& fileName, QIODevice* iodevice )
    {
        QFile outputFile( fileName );
        if ( outputFile.open( IO_ReadOnly ) )
        {
            const Q_LONG bufferSize = 16384;
            char *buffer = new char[bufferSize];
            Q_LONG readBytes = 0;
            while (( readBytes = outputFile.readBlock( buffer, bufferSize ) ) > 0 )
            {
                if ( iodevice->writeBlock( buffer, readBytes ) != readBytes )
                {
                    readBytes = -1;
                    break;
                }
            }
            outputFile.close();
            delete buffer;
            if ( readBytes == -1 )
                return FALSE;
        }
        else
            return FALSE;

        return TRUE;
    }

    QString FileExporterBibUtils::createTempDir()
    {
        QString result = QString::null;
        QFile *devrandom = new QFile( "/dev/random" );

        if ( devrandom->open( IO_ReadOnly ) )
        {
            Q_UINT32 randomNumber;
            if ( devrandom->readBlock(( char* ) & randomNumber, sizeof( randomNumber ) ) > 0 )
            {
                randomNumber |= 0x10000000;
                result = QString( "/tmp/bibtex-%1" ).arg( randomNumber, sizeof( randomNumber ) * 2, 16 );
                if ( !QDir().mkdir( result ) )
                    result = QString::null;
            }
            devrandom->close();
        }

        delete devrandom;

        return result;
    }

    void FileExporterBibUtils::deleteTempDir( const QString& directory )
    {
        QDir dir = QDir( directory );
        QStringList subDirs = dir.entryList( QDir::Dirs );
        for ( QStringList::Iterator it = subDirs.begin(); it != subDirs.end(); it++ )
        {
            if (( QString::compare( *it, "." ) != 0 ) && ( QString::compare( *it, ".." ) != 0 ) )
                deleteTempDir( *it );
        }
        QStringList allEntries = dir.entryList( QDir::All );
        for ( QStringList::Iterator it = allEntries.begin(); it != allEntries.end(); it++ )
            dir.remove( *it );

        QDir().rmdir( directory );
    }
}

#endif
