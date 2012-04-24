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

#include <fileexporterbibtex.h>
#include <settings.h>
#include "fileexporterexternal.h"

namespace BibTeX
{

    FileExporterExternal::FileExporterExternal( Exporter exporter, File::FileFormat fileformat )
            : FileExporter(), writeTo( NULL ), m_exporter( exporter ), m_fileformat( fileformat )
    {
        wc = new QWaitCondition();
    }

    FileExporterExternal::~FileExporterExternal()
    {
        delete wc;
    }

    bool FileExporterExternal::save( QIODevice* iodevice, const File* bibtexfile, QStringList *errorLog )
    {
        QBuffer buffer;
        buffer.open( IO_WriteOnly );
        FileExporter * bibtexExporter = new FileExporterBibTeX();
        bool result = bibtexExporter->save( &buffer, bibtexfile, errorLog );
        buffer.close();
        delete bibtexExporter;

        if ( result )
            result = generateOutput( buffer, iodevice );

        return result;
    }

    bool FileExporterExternal::save( QIODevice* iodevice, const Element* element, QStringList *errorLog )
    {
        QBuffer buffer;
        buffer.open( IO_WriteOnly );
        FileExporter * bibtexExporter = new FileExporterBibTeX();
        bool result = bibtexExporter->save( &buffer, element, errorLog );
        buffer.close();
        delete bibtexExporter;

        if ( result )
            result = generateOutput( buffer, iodevice );

        return result;
    }

    bool FileExporterExternal::generateOutput( QBuffer &input, QIODevice *output )
    {
        bool result = FALSE;

        QString commandLine;
        switch ( m_fileformat )
        {
        case File::formatHTML:
            switch ( m_exporter )
            {
            case exporterBibTeX2HTML:
                commandLine = "bibtex2html -s plain -a";
                break;
            case exporterBib2XHTML:
                commandLine = "bib2xhtml -s plain -u";
                break;
            case exporterBibConv:
                commandLine = "bibconv -informat=bibtex -outformat=html";
                break;
            default:
                return FALSE;
            }
            break;
        default:
            return FALSE;
        }
        QStringList args = QStringList::split( ' ', commandLine );

        writeTo = new QTextStream( output );
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
            input.open( IO_ReadOnly );
            QByteArray buf = input.buffer();
            //             qDebug( "buf.size=%i", buf.size() );
            process->writeToStdin( buf );
            input.close();
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

        QApplication::restoreOverrideCursor();
        return result;
    }

    void FileExporterExternal::slotProcessExited()
    {
        wc->wakeAll();
    }

    void FileExporterExternal::slotReadProcessOutput()
    {
        if ( writeTo != NULL )
            while ( process->canReadLineStdout() )
            {
                QString line = process->readLineStdout();
                //                 qDebug( "line=%s", line.latin1() );
                ( *writeTo ) << line.latin1() << endl;
            }
        //         while ( process->canReadLineStderr() )
        //         {
        //             QString line = process->readLineStderr();
        //             qDebug( "stderr=%s", line.latin1() );
        //         }
    }

    void FileExporterExternal::slotWroteToStdin()
    {
        process->closeStdin();
    }
}

#include "fileexporterexternal.moc"
