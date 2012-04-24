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
#include <qstring.h>
#include <qprocess.h>
#include <qstringlist.h>
#include <qfont.h>
#include <qregexp.h>
#include <qwaitcondition.h>
#include <qprocess.h>
#include <qapplication.h>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kbibtex_part.h>
#include <kdebug.h>
#include <kio/netaccess.h>

#include <entryfield.h>
#include <element.h>
#include <entry.h>
#include <macro.h>
#include "settings.h"

namespace KBibTeX
{
    Settings* Settings::staticSettings = new Settings();;
    QStringList Settings::m_lyxRcFileNames = QStringList::split( '|', QDir::home().canonicalPath() + "/.lyx/lyxrc" + '|' + QDir::home().canonicalPath() + "/.lyx/preferences" );

    Settings::Settings()
    {
        checkExternalToolsAvailable();

        KStandardDirs * kstd = KGlobal::dirs();
        external_XSLTStylesheetHTML = kstd->findResource( "data", "kbibtexpart/xslt/html.xsl" );
        if ( external_XSLTStylesheetHTML == NULL )
            KMessageBox::error( NULL, i18n( "Could not determine filename for XSLT file" ), i18n( "Initialization failed" ) );

        int numCompletions = BibTeX::EntryField::ftYear - BibTeX::EntryField::ftAbstract + 4;
        m_completion = new KCompletion * [ numCompletions ];
        for ( int i = 0; i < numCompletions;i++ )
        {
            m_completion[ i ] = new KCompletion();
            m_completion[ i ] ->setIgnoreCase( TRUE );
        }
        completionMacro = new KCompletion();
        completionMacro->setIgnoreCase( FALSE );

        currentBibTeXFile = NULL;
    }

    Settings::~Settings()
    {
        int numCompletions = BibTeX::EntryField::ftYear - BibTeX::EntryField::ftAbstract + 4;
        for ( int i = 0; i < numCompletions;i++ )
            delete m_completion[ i ];
        delete[] m_completion;
        delete completionMacro;

        for ( QValueList<SearchURL*>::ConstIterator it = searchURLs.begin(); it != searchURLs.end(); ++it )
            delete *it;
    }

    Settings* Settings::self( BibTeX::File *bibtexFile )
    {
        if ( bibtexFile != NULL )
            staticSettings->currentBibTeXFile = bibtexFile;
        return staticSettings;
    }

    void Settings::load( KConfig * config )
    {
        config->setGroup( "FileIO" );
        fileIO_Encoding = ( BibTeX::File::Encoding )( config->readNumEntry( "Encoding", ( int )( BibTeX::File::encLaTeX - 1 ) ) + 1 );
        fileIO_ExportLanguage = config->readEntry( "ExportLanguage", "english" );
        fileIO_ExportBibliographyStyle = config->readEntry( "ExportBibliographyStyle", "plain" );
        fileIO_ExporterHTML = ( BibTeX::FileExporterExternal::Exporter ) config->readNumEntry( "ExporterHTML", ( int ) BibTeX::FileExporterExternal::exporterNone );
        fileIO_BibtexStringOpenDelimiter = config->readEntry( "BibtexStringOpenDelimiter", "\"" ).at( 0 );
        fileIO_BibtexStringCloseDelimiter = config->readEntry( "BibtexStringCloseDelimiter", "\"" ).at( 0 );
        fileIO_KeywordCasing = ( BibTeX::FileExporterBibTeX::KeywordCasing )config->readNumEntry( "KeywordCasing", ( int )BibTeX::FileExporterBibTeX::kcCamelCase );
        fileIO_EmbedFiles = config->readBoolEntry( "EmbedFiles", FALSE );
        fileIO_EnclosingCurlyBrackets = config->readBoolEntry( "EnclosingCurlyBrackets", FALSE );

        config->setGroup( "Editing" );
        editing_SearchBarClearField = config->readBoolEntry( "SearchBarClearField", false );
        editing_EnableAllFields = config->readBoolEntry( "EnableAllFields", false );
        editing_MainListDoubleClickAction = config->readNumEntry( "MainListDoubleClickAction", 0 );
        editing_MainListSortingColumn = config->readNumEntry( "MainListSortingColumn", 1 );
        editing_MainListSortingOrder = config->readNumEntry( "MainListSortingOrder", 1 );
        editing_MainListColumnsWidth = config->readIntListEntry( "MainListColumnsWidth" );
        editing_MainListColumnsIndex = config->readIntListEntry( "MainListColumnsIndex" );
        editing_FilterHistory = config->readListEntry( "FilterHistory" );
        editing_ShowComments = config->readBoolEntry( "ShowComments", TRUE );
        editing_ShowMacros = config->readBoolEntry( "ShowMacros", TRUE );
        editing_HorSplitterSizes = config->readIntListEntry( "HorizontalSplitterSizes" );
        editing_VertSplitterSizes = config->readIntListEntry( "VerticalSplitterSizes" );
        QFont defaultFont = KGlobalSettings::generalFont();
        editing_SpecialFont = config->readFontEntry( "SpecialFont", &defaultFont );
        editing_UseSpecialFont = config->readBoolEntry( "UseSpecialFont", FALSE );
        editing_FirstNameFirst = config->readBoolEntry( "FirstNameFirst", FALSE );
        editing_DocumentSearchPath = config->readEntry( "DocumentSearchPath", "" );
        editing_DragAction = (DragAction)config->readNumEntry( "DragAction", ( int )COPYREFERENCE );

        if ( editing_MainListColumnsWidth.isEmpty() || editing_MainListColumnsIndex.isEmpty() )
        {
            editing_MainListColumnsWidth.clear();
            editing_MainListColumnsIndex.clear();

            int column = 0;
            // for type and id column
            editing_MainListColumnsWidth.append( 0xffff );
            editing_MainListColumnsWidth.append( 0xffff );
            editing_MainListColumnsIndex.append( column++ );
            editing_MainListColumnsIndex.append( column++ );

            for ( int i = 0; i <= ( int ) BibTeX::EntryField::ftYear - ( int ) BibTeX::EntryField::ftAbstract; i++ )
            {
                BibTeX::EntryField::FieldType fieldType = ( BibTeX::EntryField::FieldType )( i + ( int ) BibTeX::EntryField::ftAbstract );
                if ( fieldType == BibTeX::EntryField::ftAuthor || fieldType == BibTeX::EntryField::ftTitle )
                    editing_MainListColumnsWidth.append( 0xffff );
                else
                    editing_MainListColumnsWidth.append( 0 );
                editing_MainListColumnsIndex.append( column++ );
            }
        }

        config->setGroup( "SearchURLs" );
        searchURLs.clear();
        for ( int i = 1; i < 1024; i++ )
        {
            QString descr = config->readEntry( QString( "SearchURLDescription%1" ).arg( i ), QString::null );
            QString url = config->readEntry( QString( "SearchURL%1" ).arg( i ), QString::null );
            bool includeAuthor = config->readBoolEntry( QString( "IncludeAuthor%1" ).arg( i ), FALSE );
            if ( descr != QString::null && url != QString::null )
            {
                SearchURL * searchURL = new SearchURL;
                searchURL->description = descr;
                searchURL->url = url;
                searchURL->includeAuthor = includeAuthor;
                searchURLs.append( searchURL );
            }
            else
                break;
        }

        if ( searchURLs.count() == 0 )
            restoreDefaultSearchURLs();

        config->setGroup( "Keyword" );
        keyword_GlobalList = config->readListEntry( "GlobalList" );
        keyword_GlobalList.sort();

        config->setGroup( "OnlineSearchDatabase" );
        webQuery_LastEngine = config->readNumEntry( "LastEngine", 0 );
        webQuery_LastSearchTerm = config->readEntry( "LastSearchTerm", "" );
        webQuery_LastNumberOfResults = config->readNumEntry( "LastNumberOfResults", 10 );
        webQuery_ImportAll = config->readBoolEntry( "ImportAll", FALSE );

        config->setGroup( "IdSuggestions" );
        idSuggestions_formatStrList = config->readListEntry( "FormatStrList" );
        if ( idSuggestions_formatStrList.count() == 0 )
        {
            idSuggestions_formatStrList = QStringList::split( ',', "A,A2|y,A3|y,A4|y|\":|T5,al|\":|T,al|y,al|Y,Al\"-|\"-|y,Al\"+|Y,al|y|T,al|Y|T3,al|Y|T3l,a|\":|Y|\":|T1,a|y,A|\":|Y" );
            idSuggestions_default = -1;
            idSuggestions_forceDefault = FALSE;
        }
        else
        {
            idSuggestions_default = config->readNumEntry( "Default", -1 );
            idSuggestions_forceDefault = config->readBoolEntry( "ForceDefault", FALSE );
        }

        config->setGroup( "UserDefinedInputFields" );
        QStringList names = config->readListEntry( "Names" );
        QStringList labels = config->readListEntry( "Labels" );
        QStringList inputtypes = config->readListEntry( "InputTypes" );

        userDefinedInputFields.clear();
        for ( unsigned int i = 0; i < names.size();++i )
        {
            UserDefinedInputFields *udif = new UserDefinedInputFields();
            udif->name = names[i];
            udif->label = labels[i];
            udif->inputType = ( inputtypes[i] == "single" ? FieldLineEdit::itSingleLine : FieldLineEdit::itMultiLine );
            userDefinedInputFields << udif;
        }
    }

    void Settings::save( KConfig * config )
    {
        config->setGroup( "FileIO" );
        config->writeEntry( "Encoding", ( int ) fileIO_Encoding - 1 );
        config->writeEntry( "ExportLanguage", fileIO_ExportLanguage );
        config->writeEntry( "ExportBibliographyStyle", fileIO_ExportBibliographyStyle );
        config->writeEntry( "ExporterHTML", ( int ) fileIO_ExporterHTML );
        config->writeEntry( "BibtexStringOpenDelimiter", QString( fileIO_BibtexStringOpenDelimiter ) );
        config->writeEntry( "BibtexStringCloseDelimiter", QString( fileIO_BibtexStringCloseDelimiter ) );
        config->writeEntry( "KeywordCasing", ( int )fileIO_KeywordCasing );
        config->writeEntry( "EmbedFiles", fileIO_EmbedFiles );
        config->writeEntry( "EnclosingCurlyBrackets", fileIO_EnclosingCurlyBrackets );

        config->setGroup( "Editing" );
        config->writeEntry( "SearchBarClearField", editing_SearchBarClearField );
        config->writeEntry( "EnableAllFields", editing_EnableAllFields );
        config->writeEntry( "MainListDoubleClickAction", editing_MainListDoubleClickAction );
        config->writeEntry( "MainListSortingColumn", editing_MainListSortingColumn );
        config->writeEntry( "MainListSortingOrder", editing_MainListSortingOrder );
        config->writeEntry( "MainListColumnsWidth", editing_MainListColumnsWidth );
        config->writeEntry( "MainListColumnsIndex", editing_MainListColumnsIndex );
        config->writeEntry( "FilterHistory", editing_FilterHistory );
        config->writeEntry( "ShowComments", editing_ShowComments );
        config->writeEntry( "ShowMacros", editing_ShowMacros );
        config->writeEntry( "HorizontalSplitterSizes", editing_HorSplitterSizes );
        config->writeEntry( "VerticalSplitterSizes", editing_VertSplitterSizes );
        config->writeEntry( "SpecialFont", editing_SpecialFont );
        config->writeEntry( "UseSpecialFont", editing_UseSpecialFont );
        config->writeEntry( "FirstNameFirst", editing_FirstNameFirst );
        config->writeEntry( "DocumentSearchPath", editing_DocumentSearchPath );
        config->writeEntry( "DragAction", editing_DragAction );

        config->setGroup( "SearchURLs" );
        int i = 1;
        for ( QValueList<SearchURL*>::ConstIterator it = searchURLs.begin(); it != searchURLs.end(); ++it, ++i )
        {
            config->writeEntry( QString( "SearchURLDescription%1" ).arg( i ), ( *it ) ->description );
            config->writeEntry( QString( "SearchURL%1" ).arg( i ), ( *it ) ->url );
            config->writeEntry( QString( "IncludeAuthor%1" ).arg( i ), ( *it ) ->includeAuthor );
        }

        config->setGroup( "Keyword" );
        config->writeEntry( "GlobalList", keyword_GlobalList );

        config->setGroup( "OnlineSearchDatabase" );
        config->writeEntry( "LastEngine", webQuery_LastEngine );
        config->writeEntry( "LastSearchTerm", webQuery_LastSearchTerm );
        config->writeEntry( "LastNumberOfResults", webQuery_LastNumberOfResults );
        config->writeEntry( "ImportAll", webQuery_ImportAll );

        config->setGroup( "IdSuggestions" );
        config->writeEntry( "FormatStrList", idSuggestions_formatStrList );
        config->writeEntry( "Default", idSuggestions_default );
        config->writeEntry( "ForceDefault", idSuggestions_forceDefault );

        config->setGroup( "UserDefinedInputFields" );
        QStringList names, labels, inputtype;
        for ( QValueList<UserDefinedInputFields*>::iterator it = userDefinedInputFields.begin(); it != userDefinedInputFields.end();++it )
        {
            names << ( *it )->name;
            labels << ( *it )->label;
            inputtype << (( *it )->inputType == FieldLineEdit::itMultiLine ? "multi" : "single" );
        }
        config->writeEntry( "Names", names );
        config->writeEntry( "Labels", labels );
        config->writeEntry( "InputTypes", inputtype );
    }

    void Settings::checkExternalToolsAvailable()
    {
        external_bibconvAvailable = checkExternalToolAvailable( "bibconv" );
        external_bibtex2htmlAvailable = checkExternalToolAvailable( "bibtex2html" );
        external_bib2xhtmlAvailable = checkExternalToolAvailable( "bib2xhtml" );
        external_latex2rtfAvailable = checkExternalToolAvailable( "latex2rtf" );
    }

    bool Settings::checkExternalToolAvailable( const QString &binary )
    {
        QProcess process( binary );
        if ( !process.start() )
            return FALSE;

        if ( process.normalExit() )
            return TRUE;

        if ( process.isRunning() )
        {
            process.kill();
            return TRUE;
        }

        return FALSE;
    }

    QString Settings::detectLyXInPipe()
    {
        QString result = "";
        for ( QStringList::Iterator it = m_lyxRcFileNames.begin(); result.isEmpty() && it != m_lyxRcFileNames.end(); ++it )
        {
            QString lyxRcFileName = *it;
            QFile lyxRcFile( lyxRcFileName );

            kdDebug() << "detectLyXInPipe:  looking for " << lyxRcFileName << endl;
            if ( lyxRcFile.exists() && lyxRcFile.open( IO_ReadOnly ) )
            {
                QTextStream input( &lyxRcFile );
                while ( result.isEmpty() && !input.atEnd() )
                {
                    QString line = input.readLine();
                    if ( line.startsWith( "\\serverpipe ", FALSE ) )
                    {
                        QStringList cols = QStringList::split( QRegExp( "\\s+\"|\"" ), line );
                        if ( cols.size() >= 2 )
                        {
                            result = cols[ 1 ] + ".in";
                            kdDebug() << "detectLyXInPipe:  testing " << result << " from config file" << endl;
                            if ( !QFile::exists( result ) )
                                kdDebug() << "LyX in pipe '" << result << "' from " << lyxRcFileName << " does not exist" << endl;
                        }
                    }
                }
                lyxRcFile.close();
            }
        }

        if ( result.isEmpty() )
        {
            result = QDir::home().canonicalPath() + "/.lyx/lyxpipe.in";
            kdDebug() << "detectLyXInPipe:  testing " << result << endl;
            if ( !QFile::exists( result ) )
                result = "";
        }

        if ( result.isEmpty() )
        {
            result = QDir::home().canonicalPath() + "/.lyx/.lyxpipe.in";
            kdDebug() << "detectLyXInPipe:  testing " << result << endl;
            if ( !QFile::exists( result ) )
                result = "";
        }

        if ( result.isEmpty() )
        {
            result = QDir::home().canonicalPath() + "/.lyxpipe.in";
            kdDebug() << "detectLyXInPipe:  testing " << result << endl;
            if ( !QFile::exists( result ) )
                result = "";
        }

        kdDebug() << "detectLyXInPipe:  using " << ( result.isEmpty() ? "<EMPTY>" : result ) << endl;
        return result;
    }

    void Settings::addToCompletion( BibTeX::File *file )
    {
        for ( BibTeX::File::ElementList::iterator it = file->begin(); it != file->end(); it++ )
            addToCompletion( *it );
    }

    void Settings::addToCompletion( BibTeX::Element *element )
    {
        BibTeX::Entry * entry = dynamic_cast<BibTeX::Entry*>( element );
        BibTeX::Macro * macro = dynamic_cast<BibTeX::Macro*>( element );
        if ( entry != NULL )
        {
            for ( BibTeX::Entry::EntryFields::ConstIterator ite = entry->begin(); ite != entry->end(); ite++ )
            {
                BibTeX::EntryField::FieldType fieldType = ( *ite ) ->fieldType();
                BibTeX::Value *value = ( *ite ) ->value();
                addToCompletion( value, fieldType );
            }
        }
        else if ( macro != NULL )
            completionMacro->addItem( macro->key() );
    }

    void Settings::addToCompletion( BibTeX::Value *value, BibTeX::EntryField::FieldType fieldType )
    {
        int index = completionFieldTypeToIndex( fieldType );

        for ( QValueList<BibTeX::ValueItem*>::ConstIterator itv = value->items.begin(); itv != value->items.end(); itv++ )
        {
            BibTeX::PlainText *plainText = dynamic_cast<BibTeX::PlainText*>( *itv );
            if ( plainText != NULL )
                m_completion[ index ] ->addItem( plainText->text() );
            else
            {
                BibTeX::PersonContainer *personContainer = dynamic_cast<BibTeX::PersonContainer*>( *itv );
                if ( personContainer != NULL )
                    for ( QValueList<BibTeX::Person*>::Iterator itp = personContainer->persons.begin();itp != personContainer->persons.end(); ++itp )
                    {
                        m_completion[ index ] ->addItem(( *itp )->text( TRUE ) );
                        m_completion[ index ] ->addItem(( *itp )->text( FALSE ) );
                    }
                else
                {
                    BibTeX::KeywordContainer *keywordContainer = dynamic_cast<BibTeX::KeywordContainer*>( *itv );
                    if ( keywordContainer != NULL )
                        for ( QValueList<BibTeX::Keyword*>::Iterator itk = keywordContainer->keywords.begin();itk != keywordContainer->keywords.end(); ++itk )
                            m_completion[ index ] ->addItem(( *itk )->text() );
                }
            }
        }
    }

    KCompletion *Settings::completion( BibTeX::EntryField::FieldType fieldType )
    {
        return m_completion[ completionFieldTypeToIndex( fieldType )];
    }

    int Settings::completionFieldTypeToIndex( BibTeX::EntryField::FieldType fieldType )
    {
        int index = 0;
        if ( fieldType == BibTeX::EntryField::ftEditor )
            fieldType = BibTeX::EntryField::ftAuthor;
        if ( fieldType != BibTeX::EntryField::ftUnknown )
            index = ( int )( fieldType - BibTeX::EntryField::ftAbstract ) + 1;
        return index;
    }

    void Settings::restoreDefaultSearchURLs()
    {
        QStringList defaultDescriptions;
        defaultDescriptions.append( "Google Scholar" );
        defaultDescriptions.append( "Google .bib Search" );
        defaultDescriptions.append( "Google Document Search" );
        defaultDescriptions.append( "Google" );
        defaultDescriptions.append( "CiteSeer" );
        defaultDescriptions.append( "PubMed" );
        defaultDescriptions.append( "PubMed Central" );
        defaultDescriptions.append( "DBLP (Computer Science)" );
        defaultDescriptions.append( "citebase" );
        defaultDescriptions.append( "BASE" );
        defaultDescriptions.append( "Forschungsportal.Net" );
        defaultDescriptions.append( "scirus" );
        defaultDescriptions.append( "ScientificCommons" );
        defaultDescriptions.append( "Amatex (US)" );
        defaultDescriptions.append( "SpringerLink" );

        QValueList<SearchURL*> toDelete;
        for ( QValueList<SearchURL*>::ConstIterator it = searchURLs.begin(); it != searchURLs.end(); ++it )
        {
            if ( defaultDescriptions.contains(( *it ) ->description ) )
            {
                toDelete.append( *it );
            }
        }

        for ( QValueList<SearchURL*>::ConstIterator it = toDelete.begin(); it != toDelete.end(); ++it )
        {
            delete *it;
            searchURLs.remove( *it );
        }

        SearchURL *searchURL = new SearchURL;
        searchURL->description = "Google";
        searchURL->url = "http://www.google.com/search?q=%1&ie=UTF-8&oe=UTF-8";
        searchURL->includeAuthor = TRUE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "Google Scholar";
        searchURL->url = "http://scholar.google.com/scholar?q=%1&ie=UTF-8&oe=UTF-8";
        searchURL->includeAuthor = TRUE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "Google .bib Search";
        searchURL->url = "http://www.google.com/search?q=%1%20filetype%3Abib&ie=UTF-8&oe=UTF-8";
        searchURL->includeAuthor = TRUE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "Google Document Search";
        searchURL->url = "http://www.google.com/search?q=%1%20filetype%3Apdf%20OR%20filetype%3Aps&ie=UTF-8&oe=UTF-8";
        searchURL->includeAuthor = TRUE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "PubMed";
        searchURL->url = "http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?db=pubmed&cmd=search&pmfilter_Fulltext=on&pmfilter_Relevance=on&term=%1&search=Find+Articles";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "PubMed Central";
        searchURL->url = "http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?db=pubmed&cmd=search&term=%1";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "Amatex (US)";
        searchURL->url = "http://www.2ndminute.org:8080/amatex/search.do?querry=%1&suchart=kwd&lang=DE";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "DBLP (Computer Science)";
        searchURL->url = "http://www.informatik.uni-trier.de/ley/dbbin/dblpquery.cgi?title=%1";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "CiteSeer";
        searchURL->url = "http://citeseer.ist.psu.edu/cis?q=%1&submit=Search+Documents";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "citebase";
        searchURL->url = "http://www.citebase.org/search?type=metadata&author=&title=%1&publication=&yearfrom=&yearuntil=&order=DESC&rank=paperimpact&submitted=Search";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "BASE";
        searchURL->url = "http://digital.ub.uni-bielefeld.de/index.php?q=%1&s=free";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "Forschungsportal.Net";
        searchURL->url = "http://www.forschungsportal.net/fpj/q/?q=%1&pp=5&art=dok&html=1&pdf=1&ps=1&dvi=1";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "scirus";
        searchURL->url = "http://www.scirus.com/srsapp/search?q=%1&ds=jnl&ds=nom&ds=web&g=s&t=all";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "ScientificCommons";
        searchURL->url = "http://en.scientificcommons.org/#search_string=%1";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );

        searchURL = new SearchURL;
        searchURL->description = "SpringerLink";
        searchURL->url = "http://www.springerlink.com/content/?k=%1";
        searchURL->includeAuthor = FALSE;
        searchURLs.append( searchURL );
    }

    QString Settings::fieldTypeToI18NString( const BibTeX::EntryField::FieldType fieldType )
    {
        switch ( fieldType )
        {
        case BibTeX::EntryField::ftAbstract:
            return QString( i18n( "Abstract" ) );
        case BibTeX::EntryField::ftAddress:
            return QString( i18n( "Address" ) );
        case BibTeX::EntryField::ftAnnote:
            return QString( i18n( "Annote" ) );
        case BibTeX::EntryField::ftAuthor:
            return QString( i18n( "Author" ) );
        case BibTeX::EntryField::ftBookTitle:
            return QString( i18n( "Book Title" ) );
        case BibTeX::EntryField::ftChapter:
            return QString( i18n( "Chapter" ) );
        case BibTeX::EntryField::ftCrossRef:
            return QString( i18n( "Crossref" ) );
        case BibTeX::EntryField::ftDoi:
            return QString( i18n( "DOI" ) );
        case BibTeX::EntryField::ftEdition:
            return QString( i18n( "Edition" ) );
        case BibTeX::EntryField::ftEditor:
            return QString( i18n( "Editor" ) );
        case BibTeX::EntryField::ftHowPublished:
            return QString( i18n( "How Published" ) );
        case BibTeX::EntryField::ftInstitution:
            return QString( i18n( "Institution" ) );
        case BibTeX::EntryField::ftISBN:
            return QString( i18n( "ISBN" ) );
        case BibTeX::EntryField::ftISSN:
            return QString( i18n( "ISSN" ) );
        case BibTeX::EntryField::ftJournal:
            return QString( i18n( "Journal" ) );
        case BibTeX::EntryField::ftKey:
            return QString( i18n( "Key" ) );
        case BibTeX::EntryField::ftKeywords:
            return QString( i18n( "Keywords" ) );
        case BibTeX::EntryField::ftLocalFile:
            return QString( i18n( "Local File" ) );
        case BibTeX::EntryField::ftLocation:
            return QString( i18n( "Location" ) );
        case BibTeX::EntryField::ftMonth:
            return QString( i18n( "Month" ) );
        case BibTeX::EntryField::ftNote:
            return QString( i18n( "Note" ) );
        case BibTeX::EntryField::ftNumber:
            return QString( i18n( "Number" ) );
        case BibTeX::EntryField::ftOrganization:
            return QString( i18n( "Organization" ) );
        case BibTeX::EntryField::ftPages:
            return QString( i18n( "Pages" ) );
        case BibTeX::EntryField::ftPublisher:
            return QString( i18n( "Publisher" ) );
        case BibTeX::EntryField::ftSeries:
            return QString( i18n( "Series" ) );
        case BibTeX::EntryField::ftSchool:
            return QString( i18n( "School" ) );
        case BibTeX::EntryField::ftTitle:
            return QString( i18n( "Title" ) );
        case BibTeX::EntryField::ftType:
            return QString( i18n( "Type" ) );
        case BibTeX::EntryField::ftURL:
            return QString( i18n( "URL" ) );
        case BibTeX::EntryField::ftVolume:
            return QString( i18n( "Volume" ) );
        case BibTeX::EntryField::ftYear:
            return QString( i18n( "Year" ) );
        default:
            return QString( i18n( "Unknown" ) );
        }
    }

    KURL Settings::doiURL( const QString& doiText )
    {
        KURL result( doiText );
        if ( result.isValid() )
            return result;

        result = KURL( QString( "http://dx.doi.org/%1" ).arg( doiText ) );
        if ( result.isValid() )
            return result;

        return KURL();
    }

    bool Settings::kpsewhich( const QString& filename )
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

    KURL Settings::locateFile( const QString& filename, const QString& bibTeXFileName, QWidget *window )
    {
        KURL url( filename );
        if ( url.isValid() && ( !url.isLocalFile() || KIO::NetAccess::exists( url, TRUE, window ) ) )
            return url;

        if ( bibTeXFileName != QString::null )
        {
            QString path = KURL( bibTeXFileName ).directory( FALSE, FALSE );
            url = KURL( path + "/" + filename );
            if ( url.isValid() && KIO::NetAccess::exists( url, TRUE, window ) )
                return url;
        }

        Settings* settings = self( NULL );
        url = KURL( settings->editing_DocumentSearchPath + "/" + filename );
        if ( url.isValid() && KIO::NetAccess::exists( url, TRUE, window ) )
            return url;

        return KURL();
    }

    KURL Settings::locateFile( const QString& filename, QWidget *window )
    {
        return locateFile( filename, currentBibTeXFile == NULL ? QString::null : currentBibTeXFile->fileName, window );
    }

}
