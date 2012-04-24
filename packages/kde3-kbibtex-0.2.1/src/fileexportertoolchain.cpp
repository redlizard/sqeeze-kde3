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
#include <qstringlist.h>

#include "fileexportertoolchain.h"

namespace BibTeX
{

    FileExporterToolchain::FileExporterToolchain()
            : FileExporter(), m_errorLog( NULL )
    {
        m_waitCond = new QWaitCondition();
        workingDir = createTempDir();
    }

    FileExporterToolchain::~FileExporterToolchain()
    {
        delete m_waitCond;
        deleteTempDir( workingDir );
    }

    bool FileExporterToolchain::runProcesses( const QStringList &progs, QStringList *errorLog )
    {
        bool result = TRUE;
        int i = 0;

        emit progress( 0, progs.size() );
        for ( QStringList::ConstIterator it = progs.begin(); result && it != progs.end(); it++ )
        {
            qApp->processEvents();
            QStringList args = QStringList::split( ' ', *it );
            result &= runProcess( args, errorLog );
            emit progress( i++, progs.size() );
        }
        qApp->processEvents();
        return result;
    }

    bool FileExporterToolchain::runProcess( const QStringList &args, QStringList *errorLog )
    {
        bool result = FALSE;
        QApplication::setOverrideCursor( Qt::waitCursor );

        m_process = new QProcess( args );
        m_process->setWorkingDirectory( workingDir );
        connect( m_process, SIGNAL( processExited() ), this, SLOT( slotProcessExited() ) );
        connect( m_process, SIGNAL( readyReadStdout() ), this, SLOT( slotReadProcessOutput() ) );
        connect( m_process, SIGNAL( readyReadStderr() ), this, SLOT( slotReadProcessOutput() ) );

        if ( m_process->start() )
        {
            m_errorLog = errorLog;
            int counter = 0;
            qApp->processEvents();
            while ( m_process->isRunning() )
            {
                m_waitCond->wait( 250 );
                qApp->processEvents();

                counter++;
                if ( counter > 400 )
                    m_process->tryTerminate();
            }

            result = m_process->normalExit() && counter < 400;
            if ( !result )
                errorLog->append( QString( "Process '%1' failed." ).arg( args.join( " " ) ) );
        }
        else
            errorLog->append( QString( "Process '%1' was not started." ).arg( args.join( " " ) ) );


        disconnect( m_process, SIGNAL( readyReadStdout() ), this, SLOT( slotReadProcessOutput() ) );
        disconnect( m_process, SIGNAL( readyReadStderr() ), this, SLOT( slotReadProcessOutput() ) );
        disconnect( m_process, SIGNAL( processExited() ), this, SLOT( slotProcessExited() ) );
        delete( m_process );
        m_process = NULL;

        QApplication::restoreOverrideCursor();
        return result;
    }

    bool FileExporterToolchain::writeFileToIODevice( const QString &filename, QIODevice *device )
    {
        QFile file( filename );
        if ( file.open( IO_ReadOnly ) )
        {
            bool result = TRUE;
            Q_ULONG buffersize = 0x10000;
            Q_LONG amount = 0;
            char* buffer = new char[ buffersize ];
            do
            {
                result = (( amount = file.readBlock( buffer, buffersize ) ) > -1 ) && ( device->writeBlock( buffer, amount ) > -1 );
            }
            while ( result && amount > 0 );

            file.close();
            delete[] buffer;
            return result;
        }
        else
            return FALSE;
    }

    QString FileExporterToolchain::createTempDir()
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

    void FileExporterToolchain::deleteTempDir( const QString& directory )
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

    void FileExporterToolchain::slotProcessExited()
    {
        m_waitCond->wakeAll();
    }

    void FileExporterToolchain::cancel()
    {
        if ( m_process != NULL )
        {
            qDebug( "Canceling process" );
            m_process->tryTerminate();
            m_process->kill();
            m_waitCond->wakeAll();
        }
    }

    void FileExporterToolchain::slotReadProcessOutput()
    {
        if ( m_process )
        {
            while ( m_process->canReadLineStdout() )
            {
                QString line = m_process->readLineStdout();
                if ( m_errorLog != NULL )
                    m_errorLog->append( line );
            }
            while ( m_process->canReadLineStderr() )
            {
                QString line = m_process->readLineStderr();
                if ( m_errorLog != NULL )
                    m_errorLog->append( line );
            }
        }
    }

    bool FileExporterToolchain::kpsewhich( const QString& filename )
    {
        bool result = FALSE;
        int counter = 0;

        QWaitCondition waitCond;
        QProcess kpsewhich;
        kpsewhich.addArgument( "kpsewhich" );
        kpsewhich.addArgument( filename );
        if ( kpsewhich.start() )
        {
            qApp->processEvents();
            while ( kpsewhich.isRunning() )
            {
                waitCond.wait( 250 );
                qApp->processEvents();

                counter++;
                if ( counter > 50 )
                    kpsewhich.tryTerminate();
            }

            result = kpsewhich.exitStatus() == 0 && counter < 50;
        }

        return result;
    }

}

#include "fileexportertoolchain.moc"
