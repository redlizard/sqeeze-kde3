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
#include <qprocess.h>
#include <qwaitcondition.h>
#include <qapplication.h>
#include <qbuffer.h>

#include <fileimporterbibtex.h>
#include "fileimporterexternal.h"

namespace BibTeX
{

    FileImporterExternal::FileImporterExternal( Importer importer, File::FileFormat fileformat )
            : FileImporter(), m_importer( importer ), m_fileformat( fileformat )
    {
        wc = new QWaitCondition();
    }


    FileImporterExternal::~FileImporterExternal()
    {
        delete wc;
    }

    File* FileImporterExternal::load( QIODevice *iodevice )
    {
        File * result = NULL;
        QBuffer buffer;

        if ( fetchInput( iodevice, buffer ) )
        {
            buffer.open( IO_ReadOnly );
            FileImporter * bibtexImporter = new FileImporterBibTeX();
            result = bibtexImporter->load( &buffer );
            buffer.close();
            delete bibtexImporter;
        }

        return result;
    }

    void FileImporterExternal::cancel()
    {
        if ( process != NULL )
            process->kill();
    }

    bool FileImporterExternal::fetchInput( QIODevice *input, QBuffer &output )
    {
        bool result = FALSE;

        QString commandLine;
        switch ( m_fileformat )
        {
        default:
            return FALSE;
        }
        QStringList args = QStringList::split( ' ', commandLine );

        output.open( IO_WriteOnly );
        writeTo = new QTextStream( &output );
        writeTo->setEncoding( QTextStream::UnicodeUTF8 );

        QApplication::setOverrideCursor( Qt::waitCursor );

        process = new QProcess( args );
        connect( process, SIGNAL( processExited() ), this, SLOT( slotProcessExited() ) );
        connect( process, SIGNAL( readyReadStdout() ), this, SLOT( slotReadProcessOutput() ) );
        connect( process, SIGNAL( readyReadStderr() ), this, SLOT( slotReadProcessOutput() ) );
        connect( process, SIGNAL( wroteToStdin() ), this, SLOT( slotWroteToStdin() ) );

        if ( process->start() )
        {
            while ( !process->isRunning() )
            {
                wc->wait( 250 );
                qApp->processEvents();
            }
            qApp->processEvents();
            QByteArray buf = input->readAll();
            process->writeToStdin( buf );
            qApp->processEvents();
            while ( process->isRunning() )
            {
                wc->wait( 250 );
                qApp->processEvents();
            }

            result = process->normalExit();
        }

        disconnect( process, SIGNAL( wroteToStdin() ), this, SLOT( slotWroteToStdin() ) );
        disconnect( process, SIGNAL( readyReadStdout() ), this, SLOT( slotReadProcessOutput() ) );
        disconnect( process, SIGNAL( readyReadStderr() ), this, SLOT( slotReadProcessOutput() ) );
        disconnect( process, SIGNAL( processExited() ), this, SLOT( slotProcessExited() ) );
        delete( process );
        process = NULL;
        delete writeTo;
        writeTo = NULL;
        output.close();

        QApplication::restoreOverrideCursor();
        return result;
    }

    void FileImporterExternal::slotProcessExited()
    {
        wc->wakeAll();
    }

    void FileImporterExternal::slotReadProcessOutput()
    {
        if ( writeTo != NULL )
            while ( process->canReadLineStdout() )
            {
                QString line = process->readLineStdout();
                //                 qDebug( "line=%s", line.latin1() );
                ( *writeTo ) << line.latin1();
                ( *writeTo ) << endl;
            }
    }

    void FileImporterExternal::slotWroteToStdin()
    {
        process->closeStdin();
    }


}
#include "fileimporterexternal.moc"
