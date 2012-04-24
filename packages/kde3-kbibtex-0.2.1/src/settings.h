/***************************************************************************
*   Copyright (C) 2004-2006 by Thomas Fischer                             *
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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <qstringlist.h>

#include <kcompletion.h>

#include <file.h>
#include <fieldlineedit.h>
#include <fileexporterbibtex.h>
#include <fileexporterexternal.h>
#include <entryfield.h>

class KConfig;
class KStandardDirs;
class KURL;
class QString;
class QChar;
class QFont;
class BibTeX::Element;

namespace KBibTeX
{
    class Settings
    {
    public:
        struct SearchURL
        {
            QString description;
            QString url;
            bool includeAuthor;
        };

        struct UserDefinedInputFields
        {
            QString name;
            QString label;
            FieldLineEdit::InputType inputType;
        };

        static Settings* self( BibTeX::File *bibtexFile = NULL );

        void load( KConfig * config );
        void save( KConfig * config );

        static QString fieldTypeToI18NString( const BibTeX::EntryField::FieldType fieldType );
        static KURL doiURL( const QString& doiText );
        static bool kpsewhich( const QString& filename );
        static KURL locateFile( const QString& filename, const QString& bibTeXFileName = QString::null, QWidget *window = NULL );
        KURL locateFile( const QString& filename, QWidget *window = NULL );

    public:
        enum DragAction {COPYREFERENCE = 1, COPYBIBTEX = 2};

        // FileIO section
        BibTeX::File::Encoding fileIO_Encoding;
        QString fileIO_ExportLanguage;
        QString fileIO_ExportBibliographyStyle;
        QChar fileIO_BibtexStringOpenDelimiter;
        QChar fileIO_BibtexStringCloseDelimiter;
        BibTeX::FileExporterBibTeX::KeywordCasing fileIO_KeywordCasing;
        BibTeX::FileExporterExternal::Exporter fileIO_ExporterHTML;
        bool fileIO_EmbedFiles;
        bool fileIO_EnclosingCurlyBrackets;

        // Editing section
        bool editing_SearchBarClearField;
        bool editing_EnableAllFields;
        int editing_MainListDoubleClickAction;
        int editing_MainListSortingColumn;
        int editing_MainListSortingOrder;
        QStringList editing_FilterHistory;
        bool editing_ShowMacros;
        bool editing_ShowComments;
        QValueList<int> editing_MainListColumnsWidth;
        QValueList<int> editing_MainListColumnsIndex;
        QValueList<int> editing_HorSplitterSizes;
        QValueList<int> editing_VertSplitterSizes;
        QFont editing_SpecialFont;
        bool editing_UseSpecialFont;
        bool editing_FirstNameFirst;
        QString editing_DocumentSearchPath;
        DragAction editing_DragAction;

        // SearchURLs section
        QValueList<SearchURL*> searchURLs;

        // UserDefinedInputFields section
        QValueList<UserDefinedInputFields*> userDefinedInputFields;

        // External section
        QString external_XSLTStylesheetHTML;
        bool external_bibconvAvailable;
        bool external_bibtex2htmlAvailable;
        bool external_bib2xhtmlAvailable;
        bool external_latex2rtfAvailable;

        // Id Suggestions
        QStringList idSuggestions_formatStrList;
        int idSuggestions_default;
        bool idSuggestions_forceDefault;

        // Auto-completion for user fields (non-persistant)
        void addToCompletion( BibTeX::File *file );
        void addToCompletion( BibTeX::Element *element );
        void addToCompletion( BibTeX::Value *value, BibTeX::EntryField::FieldType fieldType );
        KCompletion* completion( BibTeX::EntryField::FieldType fieldType );
        KCompletion* completionMacro;

        // Keyword list
        QStringList keyword_GlobalList;

        // Online Database Search;
        int webQuery_LastEngine;
        QString webQuery_LastSearchTerm;
        int webQuery_LastNumberOfResults;
        bool webQuery_ImportAll;

        // Current BibTeX file
        BibTeX::File *currentBibTeXFile;

        void restoreDefaultSearchURLs();

        QString detectLyXInPipe();

    private:
        static Settings* staticSettings;
        KCompletion **m_completion;
        static QStringList m_lyxRcFileNames;

        Settings();
        ~Settings();

        void checkExternalToolsAvailable();
        bool checkExternalToolAvailable( const QString &binary );

        int completionFieldTypeToIndex( BibTeX::EntryField::FieldType fieldType );
    };
}

#endif
