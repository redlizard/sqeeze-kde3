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
#include <qlayout.h>
#include <qtextedit.h>
#include <qbuffer.h>

#include <kdialog.h>
#include <kglobalsettings.h>

#include <fileexporterbibtex.h>
#include <fileimporterbibtex.h>
#include <settings.h>
#include "entrywidgetsource.h"

namespace KBibTeX
{

    EntryWidgetSource::EntryWidgetSource( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent, const char *name )
            : EntryWidgetTab( bibtexfile, isReadOnly, parent, name )
    {
        setupGUI();
    }

    EntryWidgetSource::~EntryWidgetSource()
    {
        // nothing
    }

    bool EntryWidgetSource::isModified()
    {
        return m_textEditSource->isModified();
    }

    void EntryWidgetSource::updateGUI( BibTeX::Entry::EntryType /*entryType*/, bool /*enableAll*/ )
    {
        // nothing
    }

    void EntryWidgetSource::apply( BibTeX::Entry *entry )
    {
        Settings * settings = Settings::self();
        QBuffer buffer;
        BibTeX::FileImporterBibTeX importer( settings->editing_FirstNameFirst );

        buffer.open( IO_WriteOnly );
        QTextStream ts( &buffer );
        ts.setEncoding( QTextStream::UnicodeUTF8 );
        ts << m_textEditSource->text() << endl;
        buffer.close();

        buffer.open( IO_ReadOnly );
        BibTeX::File *bibtexFile = importer.load( &buffer );
        buffer.close();

        if ( bibtexFile != NULL )
        {
            if ( bibtexFile->count() == 1 )
            {
                BibTeX::Entry * parsedEntry = dynamic_cast<BibTeX::Entry*>( bibtexFile->at( 0 ) );
                if ( parsedEntry != NULL )
                {
                    entry->copyFrom( parsedEntry );
                    settings->addToCompletion( parsedEntry );
                }
            }
            delete bibtexFile;
        }
    }

    void EntryWidgetSource::reset( BibTeX::Entry *entry )
    {
        Settings * settings = Settings::self();

        QBuffer buffer;
        BibTeX::FileExporterBibTeX exporter;
        exporter.setEncoding(( BibTeX::File::Encoding ) settings->fileIO_Encoding );
        exporter.setStringDelimiter( settings->fileIO_BibtexStringOpenDelimiter, settings->fileIO_BibtexStringCloseDelimiter );
        exporter.setKeywordCasing( settings->fileIO_KeywordCasing );
        exporter.setEnclosingCurlyBrackets( settings->fileIO_EnclosingCurlyBrackets );

        buffer.open( IO_WriteOnly );
        exporter.save( &buffer, entry );
        buffer.close();

        buffer.open( IO_ReadOnly );
        QTextStream ts( &buffer );
        ts.setEncoding( QTextStream::UnicodeUTF8 );
        m_textEditSource->setText( ts.read() );
        buffer.close();
    }

    void EntryWidgetSource::updateWarnings( BibTeX::Entry::EntryType /*entryType*/, QListView * /*listViewWarnings*/ )
    {
        // nothing
    }

    void EntryWidgetSource::setupGUI()
    {
        QGridLayout * gridLayout = new QGridLayout( this, 1, 1, KDialog::marginHint(), KDialog::spacingHint(), "gridLayout" );

        m_textEditSource = new QTextEdit( this );
        gridLayout->addWidget( m_textEditSource, 0, 0 );
        m_textEditSource->setFont( KGlobalSettings::fixedFont() );
        m_textEditSource->setReadOnly( m_isReadOnly );
    }


}
#include "entrywidgetsource.moc"
