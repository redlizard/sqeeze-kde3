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
#include "fileexporterps.h"

namespace BibTeX
{

    FileExporterPS::FileExporterPS()
            : FileExporterToolchain(), m_latexLanguage( "english" ), m_latexBibStyle( "plain" )
    {
        laTeXFilename = QString( workingDir ).append( "/bibtex-to-ps.tex" );
        bibTeXFilename = QString( workingDir ).append( "/bibtex-to-ps.bib" );
        outputFilename = QString( workingDir ).append( "/bibtex-to-ps.ps" );
    }

    FileExporterPS::~FileExporterPS()
    {
        // nothing
    }

    bool FileExporterPS::save( QIODevice* iodevice, const File* bibtexfile, QStringList *errorLog )
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
            result = generatePS( iodevice, errorLog );

        return result;
    }

    bool FileExporterPS::save( QIODevice* iodevice, const Element* element, QStringList *errorLog )
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
            result = generatePS( iodevice, errorLog );

        return result;
    }

    void FileExporterPS::setLaTeXLanguage( const QString& language )
    {
        m_latexLanguage = language;
    }

    void FileExporterPS::setLaTeXBibliographyStyle( const QString& bibStyle )
    {
        m_latexBibStyle = bibStyle;
    }

    bool FileExporterPS::generatePS( QIODevice* iodevice, QStringList *errorLog )
    {
        QStringList cmdLines = QStringList::split( '|', "latex -halt-on-error bibtex-to-ps.tex|bibtex bibtex-to-ps|latex -halt-on-error bibtex-to-ps.tex|latex -halt-on-error bibtex-to-ps.tex|dvips -o bibtex-to-ps.ps bibtex-to-ps.dvi" );

        if ( writeLatexFile( laTeXFilename ) && runProcesses( cmdLines, errorLog ) && writeFileToIODevice( outputFilename, iodevice ) )
            return TRUE;
        else
            return FALSE;
    }

    bool FileExporterPS::writeLatexFile( const QString &filename )
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
            if ( kpsewhich( "apacite.sty" ) )
                ts << "\\usepackage[bibnewpage]{apacite}\n";
            ts << "\\bibliographystyle{" << m_latexBibStyle << "}\n";
            ts << "\\begin{document}\n";
            ts << "\\nocite{*}\n";
            ts << "\\bibliography{bibtex-to-ps}\n";
            ts << "\\end{document}\n";
            latexFile.close();
            return TRUE;
        }
        else
            return FALSE;

    }

}
