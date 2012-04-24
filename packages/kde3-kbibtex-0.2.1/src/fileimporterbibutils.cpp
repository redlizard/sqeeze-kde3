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

#include <stdio.h>

#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>

#include "fileimporterbibutils.h"

namespace BibTeX
{

    FileImporterBibUtils::FileImporterBibUtils( InputFormat inputFormat )
            : FileImporter(), m_workingDir( createTempDir() ), m_inputFormat( inputFormat ), m_bibTeXImporter( new FileImporterBibTeX() )
    {
// nothing
    }

    FileImporterBibUtils::~FileImporterBibUtils()
    {
        deleteTempDir( m_workingDir );
        delete m_bibTeXImporter;
    }

    File* FileImporterBibUtils::load( QIODevice *iodevice )
    {
        File *result = NULL;
        m_cancelFlag = FALSE;
        QString inputFileName = QString( m_workingDir ).append( "/input.tmp" );
        QString bibFileName = QString( m_workingDir ).append( "/input.bib" );

        /** write temporary data to file to be read by bibutils */
        QFile inputFile( inputFileName );
        if ( inputFile.open( IO_WriteOnly ) )
        {
            const Q_LONG bufferSize = 16384;
            char *buffer = new char[bufferSize];
            Q_LONG readBytes = 0;
            while (( readBytes = iodevice->readBlock( buffer, bufferSize ) ) > 0 )
            {
                if ( inputFile.writeBlock( buffer, readBytes ) != readBytes )
                {
                    readBytes = -1;
                    break;
                }
            }
            inputFile.close();
            delete buffer;
            if ( readBytes == -1 )
                return NULL;
        }
        else
            return NULL;

        param p;
        bibl b;
        bibl_init( &b );
        bibl_initparams( &p, ( int )m_inputFormat, BIBL_BIBTEXOUT );
        p.verbose = 3;
        FILE *fp = fopen( inputFileName.ascii(), "r" );
        int err = BIBL_ERR_BADINPUT;
        if ( fp )
        {
            err = bibl_read( &b, fp, inputFileName.ascii(), ( int )m_inputFormat, &p );
            fclose( fp );
        }
        if ( err != BIBL_OK )
            return NULL;

        fp = fopen( bibFileName.ascii(), "w" );
        err = -1;
        if ( fp != NULL )
        {
            err = bibl_write( &b, fp, BIBL_BIBTEXOUT, &p );
            fclose( fp );
        }
        bibl_free( &b );
        if ( err != BIBL_OK )
            return NULL;

        QFile bibFile( bibFileName );
        if ( bibFile.open( IO_ReadOnly ) )
        {
            result = m_bibTeXImporter->load( &bibFile );
            bibFile.close();
        }

        return result;
    }

    bool FileImporterBibUtils::guessCanDecode( const QString & text )
    {
        return guessInputFormat( text ) != ifUndefined;
    }

    FileImporterBibUtils::InputFormat FileImporterBibUtils::guessInputFormat( const QString &text )
    {
        if ( text.find( "TY  - " ) >= 0 )
            return ifRIS;
        else if ( text.find( "%A " ) >= 0 )
            return ifEndNote;
        else if ( text.find( "FN ISI Export Format" ) >= 0 )
            return ifISI;
        else
            return ifUndefined;
// TODO: Add more formats
    }

    void FileImporterBibUtils::cancel()
    {
        m_bibTeXImporter->cancel();
        m_cancelFlag = TRUE;
    }

    QString FileImporterBibUtils::createTempDir()
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

    void FileImporterBibUtils::deleteTempDir( const QString& directory )
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

#endif // HAVE_LIBBIBUTILS
