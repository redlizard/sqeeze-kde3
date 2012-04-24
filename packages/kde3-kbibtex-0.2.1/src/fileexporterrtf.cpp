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

#include <settings.h>
#include <element.h>
#include <fileexporterbibtex.h>
#include "fileexporterrtf.h"

namespace BibTeX
{

    FileExporterRTF::FileExporterRTF() : FileExporterToolchain(), m_latexLanguage( "english" ), m_latexBibStyle( "plain" )
    {
        laTeXFilename = QString( workingDir ).append( "/bibtex-to-rtf.tex" );
        bibTeXFilename = QString( workingDir ).append( "/bibtex-to-rtf.bib" );
        outputFilename = QString( workingDir ).append( "/bibtex-to-rtf.rtf" );
    }

    FileExporterRTF::~FileExporterRTF()
    {
        // nothing
    }

    bool FileExporterRTF::save( QIODevice* iodevice, const File* bibtexfile, QStringList *errorLog )
    {
        bool result = FALSE;

        QFile bibtexFile( bibTeXFilename );
        if ( bibtexFile.open( IO_WriteOnly ) )
        {
            FileExporter * bibtexExporter = new FileExporterBibTeX();
            result = bibtexExporter->save( &bibtexFile, bibtexfile, errorLog );
            bibtexFile.close();
            delete bibtexExporter;
        }

        if ( result )
            result = generateRTF( iodevice, errorLog );

        return result;
    }

    bool FileExporterRTF::save( QIODevice* iodevice, const Element* element, QStringList *errorLog )
    {
        bool result = FALSE;

        QFile bibtexFile( bibTeXFilename );
        if ( bibtexFile.open( IO_WriteOnly ) )
        {
            FileExporter * bibtexExporter = new FileExporterBibTeX();
            result = bibtexExporter->save( &bibtexFile, element, errorLog );
            bibtexFile.close();
            delete bibtexExporter;
        }

        if ( result )
            result = generateRTF( iodevice, errorLog );

        return result;
    }

    void FileExporterRTF::setLaTeXLanguage( const QString& language )
    {
        m_latexLanguage = language;
    }

    void FileExporterRTF::setLaTeXBibliographyStyle( const QString& bibStyle )
    {
        m_latexBibStyle = bibStyle;
    }

    bool FileExporterRTF::generateRTF( QIODevice* iodevice, QStringList *errorLog )
    {
        QStringList cmdLines = QStringList::split( '|', "latex bibtex-to-rtf.tex|bibtex bibtex-to-rtf|latex bibtex-to-rtf.tex|latex2rtf bibtex-to-rtf.tex" );

        if ( writeLatexFile( laTeXFilename ) && runProcesses( cmdLines, errorLog ) && writeFileToIODevice( outputFilename, iodevice ) )
            return TRUE;
        else
            return FALSE;
    }

    bool FileExporterRTF::writeLatexFile( const QString &filename )
    {
        QFile latexFile( filename );
        if ( latexFile.open( IO_WriteOnly ) )
        {
            QTextStream ts( &latexFile );
            ts.setEncoding( QTextStream::UnicodeUTF8 );
            ts << "\\documentclass{article}\n";
            if ( kpsewhich( "babel.sty" ) )
                ts << "\\usepackage[" << m_latexLanguage << "]{babel}\n";
            if ( kpsewhich( "url.sty" ) )
                ts << "\\usepackage{url}\n";
            ts << "\\bibliographystyle{" << m_latexBibStyle << "}\n";
            ts << "\\begin{document}\n";
            ts << "\\nocite{*}\n";
            ts << "\\bibliography{bibtex-to-rtf}\n";
            ts << "\\end{document}\n";
            latexFile.close();
            return TRUE;
        }
        else
            return FALSE;

    }

}
