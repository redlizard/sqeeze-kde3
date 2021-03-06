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
#include <qfile.h>
#include <qstringlist.h>
#include <qurl.h>

#include <settings.h>
#include <element.h>
#include <entry.h>
#include <fileexporterbibtex.h>
#include "fileexporterpdf.h"

namespace BibTeX
{

    FileExporterPDF::FileExporterPDF( bool embedFiles )
            : FileExporterToolchain(), m_latexLanguage( "english" ), m_latexBibStyle( "plain" ), m_embedFiles( embedFiles )
    {
        laTeXFilename = QString( workingDir ).append( "/bibtex-to-pdf.tex" );
        bibTeXFilename = QString( workingDir ).append( "/bibtex-to-pdf.bib" );
        outputFilename = QString( workingDir ).append( "/bibtex-to-pdf.pdf" );
    }

    FileExporterPDF::~FileExporterPDF()
    {
        // nothing
    }

    bool FileExporterPDF::save( QIODevice* iodevice, const File* bibtexfile, QStringList *errorLog )
    {
        bool result = FALSE;
        m_embeddedFileList.clear();
        if ( m_embedFiles )
        {
            m_embeddedFileList.append( QString( "%1|%2" ).arg( "BibTeX source" ).arg( bibTeXFilename ) );
            fillEmbeddedFileList( bibtexfile );
        }

        QFile bibtexFile( bibTeXFilename );
        if ( bibtexFile.open( IO_WriteOnly ) )
        {
            FileExporter * bibtexExporter = new FileExporterBibTeX();
            result = bibtexExporter->save( &bibtexFile, bibtexfile, errorLog );
            bibtexFile.close();
            delete bibtexExporter;
        }

        if ( result )
            result = generatePDF( iodevice, errorLog );

        return result;
    }

    bool FileExporterPDF::save( QIODevice* iodevice, const Element* element, QStringList *errorLog )
    {
        bool result = FALSE;
        m_embeddedFileList.clear();
        if ( m_embedFiles )
            fillEmbeddedFileList( element );

        QFile bibtexFile( bibTeXFilename );
        if ( bibtexFile.open( IO_WriteOnly ) )
        {
            FileExporter * bibtexExporter = new FileExporterBibTeX();
            result = bibtexExporter->save( &bibtexFile, element, errorLog );
            bibtexFile.close();
            delete bibtexExporter;
        }

        if ( result )
            result = generatePDF( iodevice, errorLog );

        return result;
    }

    void FileExporterPDF::setLaTeXLanguage( const QString& language )
    {
        m_latexLanguage = language;
    }

    void FileExporterPDF::setLaTeXBibliographyStyle( const QString& bibStyle )
    {
        m_latexBibStyle = bibStyle;
    }

    void FileExporterPDF::setDocumentSearchPaths( const QStringList& searchPaths )
    {
        m_searchPaths = searchPaths;
    }

    bool FileExporterPDF::generatePDF( QIODevice* iodevice, QStringList *errorLog )
    {
        QStringList cmdLines = QStringList::split( '|', "pdflatex -halt-on-error bibtex-to-pdf.tex|bibtex bibtex-to-pdf|pdflatex -halt-on-error bibtex-to-pdf.tex|pdflatex -halt-on-error bibtex-to-pdf.tex" );

        if ( writeLatexFile( laTeXFilename ) && runProcesses( cmdLines, errorLog ) && writeFileToIODevice( outputFilename, iodevice ) )
            return TRUE;
        else
            return FALSE;
    }

    bool FileExporterPDF::writeLatexFile( const QString &filename )
    {
        QFile latexFile( filename );
        if ( latexFile.open( IO_WriteOnly ) )
        {
            m_embedFiles &= kpsewhich( "embedfile.sty" );
            QTextStream ts( &latexFile );
            ts.setEncoding( QTextStream::UnicodeUTF8 );
            ts << "\\documentclass{article}\n";
            ts << "\\usepackage[" << m_latexLanguage << "]{babel}\n";
            if ( kpsewhich( "hyperref.sty" ) )
                ts << "\\usepackage[pdfproducer={KBibTeX: http://www.t-fischer.net/kbibtex/},pdftex]{hyperref}\n";
            else if ( kpsewhich( "url.sty" ) )
                ts << "\\usepackage{url}\n";
            if ( kpsewhich( "apacite.sty" ) )
                ts << "\\usepackage[bibnewpage]{apacite}\n";
            if ( m_embedFiles )
                ts << "\\usepackage{embedfile}\n";
            ts << "\\bibliographystyle{" << m_latexBibStyle << "}\n";
            ts << "\\begin{document}\n";

            if ( m_embedFiles )
                for ( QStringList::ConstIterator it = m_embeddedFileList.begin(); it != m_embeddedFileList.end(); ++it )
                {
                    QStringList param = QStringList::split( "|", *it );
                    QFile file( param[1] );
                    if ( file.exists() )
                        ts << "\\embedfile[desc={" << param[0] << "}]{" << param[1] << "}\n";
                }

            ts << "\\nocite{*}\n";
            ts << "\\bibliography{bibtex-to-pdf}\n";
            ts << "\\end{document}\n";
            latexFile.close();
            return TRUE;
        }
        else
            return FALSE;
    }

    void FileExporterPDF::fillEmbeddedFileList( const File* bibtexfile )
    {
        for ( BibTeX::File::ElementList::ConstIterator it = bibtexfile->constBegin(); it != bibtexfile->constEnd(); ++it )
            fillEmbeddedFileList( *it );
    }

    void FileExporterPDF::fillEmbeddedFileList( const Element* element )
    {
        const Entry *entry = dynamic_cast<const Entry*>( element );
        if ( entry != NULL )
        {
            QString id = entry->id();
            QStringList urls = entry->urls();
            for ( QStringList::Iterator it = urls.begin(); it != urls.end(); ++it )
            {
                QUrl url = QUrl( *it );
                if ( url.isValid() && url.isLocalFile() && !( *it ).endsWith( "/" ) && QFile( url.path() ).exists() )
                    m_embeddedFileList.append( QString( "%1|%2" ).arg( id ).arg( url.path() ) );
                else
                    for ( QStringList::Iterator path_it = m_searchPaths.begin(); path_it != m_searchPaths.end(); ++path_it )
                    {
                        url = QUrl( QString( *path_it ).append( "/" ).append( *it ) );
                        if ( url.isValid() && url.isLocalFile() && !( *it ).endsWith( "/" ) && QFile( url.path() ).exists() )
                        {
                            m_embeddedFileList.append( QString( "%1|%2" ).arg( id ).arg( url.path() ) );
                            break;
                        }
                    }
            }
        }
    }

}
