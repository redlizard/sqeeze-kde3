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

#include <qhbuttongroup.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kpushbutton.h>

#include <settings.h>
#include <file.h>
#include "settingsfileio.h"

namespace KBibTeX
{
    static const QString LanguagesI18N = i18n( "English|German|French|Spanish|Swedish" );
    static const QString Languages[] =
    {
        "english", "german", "french", "spanish", "swedish"
    };
    static const QString BibliographyStyles = i18n( "plain|achemso (American Chemical Society)|alpha|unsrt|abbrv|acm (Association of Computing Machinery)|apacite|apalike|authordate|ieeetr|natbib|siam" );
    static const QString Encodings = i18n( "LaTeX|UTF-8" );
    static const QString StringDelimiters = "\"...\"|{...}|(...)";
    static const QString KeywordCasing = "@inproceedings|@Inproceedings|@InProceedings|@INPROCEEDINGS";

    SettingsFileIO::SettingsFileIO( QWidget *parent, const char *name )
            : QWidget( parent, name )
    {
        setupGUI();
    }

    SettingsFileIO::~SettingsFileIO()
    {
        // nothing
    }

    void SettingsFileIO::applyData()
    {
        Settings * settings = Settings::self();

        switch ( m_comboBoxEncoding->currentItem() )
        {
        case 1:
            settings->fileIO_Encoding = BibTeX::File::encUTF8;
            break;
        default:
            settings->fileIO_Encoding = BibTeX::File::encLaTeX;
        }

        QString delimiters = m_comboBoxStringDelimiters->currentText();
        settings->fileIO_BibtexStringOpenDelimiter = delimiters.at( 0 ) ;
        settings->fileIO_BibtexStringCloseDelimiter = delimiters.at( 4 ) ;

        switch ( m_comboBoxKeywordCasing->currentItem() )
        {
        case 0:
            settings->fileIO_KeywordCasing = BibTeX::FileExporterBibTeX::kcLowerCase;
            break;
        case 1:
            settings->fileIO_KeywordCasing = BibTeX::FileExporterBibTeX::kcInitialCapital;
            break;
        case 3:
            settings->fileIO_KeywordCasing = BibTeX::FileExporterBibTeX::kcCapital;
            break;
        default:
            settings->fileIO_KeywordCasing = BibTeX::FileExporterBibTeX::kcCamelCase;
        }

        settings->fileIO_EnclosingCurlyBrackets = m_checkBoxProtectCasing->isChecked();

        settings->fileIO_ExportLanguage = Languages[ m_comboBoxLanguage->currentItem()];
        settings->fileIO_ExportBibliographyStyle = m_comboBoxBibliographyStyle->currentText().section( ' ', 0, 0 );

        if ( m_comboBoxExportSystemHTML->isEnabled() )
        {
            QString text = m_comboBoxExportSystemHTML->currentText();
            if ( text == "bib2xhtml" )
                settings->fileIO_ExporterHTML = BibTeX::FileExporterExternal::exporterBib2XHTML;
            else if ( text == "bibtex2html" )
                settings->fileIO_ExporterHTML = BibTeX::FileExporterExternal::exporterBibTeX2HTML;
            else if ( text == "bibconv" )
                settings->fileIO_ExporterHTML = BibTeX::FileExporterExternal::exporterBibConv;
            else
                settings->fileIO_ExporterHTML = BibTeX::FileExporterExternal::exporterXSLT;
        }
        else
            settings->fileIO_ExporterHTML = BibTeX::FileExporterExternal::exporterNone;

        settings->fileIO_EmbedFiles = m_checkBoxEmbedFiles->isChecked();
    }

    void SettingsFileIO::readData()
    {
        Settings * settings = Settings::self();

        switch ( settings->fileIO_Encoding )
        {
        case BibTeX::File::encUTF8:
            m_comboBoxEncoding->setCurrentItem( 1 );
            break;
        default:
            m_comboBoxEncoding->setCurrentItem( 0 );
        }

        QString toMatch = QString( settings->fileIO_BibtexStringOpenDelimiter ).append( "..." ).append( settings->fileIO_BibtexStringCloseDelimiter );
        QStringList delimiterList = QStringList::split( '|', StringDelimiters );
        int i = 0;
        for ( QStringList::Iterator it = delimiterList.begin(); it != delimiterList.end(); ++it, i++ )
            if (( *it ) == toMatch )
            {
                m_comboBoxStringDelimiters->setCurrentItem( i );
                break;
            }

        switch ( settings->fileIO_KeywordCasing )
        {
        case BibTeX::FileExporterBibTeX::kcLowerCase:
            m_comboBoxKeywordCasing->setCurrentItem( 0 );
            break;
        case  BibTeX::FileExporterBibTeX::kcInitialCapital:
            m_comboBoxKeywordCasing->setCurrentItem( 1 );
            break;
        case BibTeX::FileExporterBibTeX::kcCapital:
            m_comboBoxKeywordCasing->setCurrentItem( 3 );
            break;
        default:
            m_comboBoxKeywordCasing->setCurrentItem( 2 );
        }

        m_checkBoxProtectCasing->setChecked( settings->fileIO_EnclosingCurlyBrackets );

        for ( int i = 0; i < m_comboBoxLanguage->count(); i++ )
            if ( Languages[ i ] == settings->fileIO_ExportLanguage )
            {
                m_comboBoxLanguage->setCurrentItem( i );
                break;
            }

        m_comboBoxBibliographyStyle->setCurrentText( settings->fileIO_ExportBibliographyStyle );

        m_comboBoxExportSystemHTML->clear();

        m_comboBoxExportSystemHTML->insertItem( i18n( "XSLT Stylesheet" ) );
        if ( settings->fileIO_ExporterHTML == BibTeX::FileExporterExternal::exporterXSLT )
            m_comboBoxExportSystemHTML->setCurrentItem( m_comboBoxExportSystemHTML->count() - 1 );
        if ( settings->external_bib2xhtmlAvailable )
        {
            m_comboBoxExportSystemHTML->insertItem( "bib2xhtml" );
            if ( settings->fileIO_ExporterHTML == BibTeX::FileExporterExternal::exporterBib2XHTML )
                m_comboBoxExportSystemHTML->setCurrentItem( m_comboBoxExportSystemHTML->count() - 1 );
        }
        if ( settings->external_bibtex2htmlAvailable )
        {
            m_comboBoxExportSystemHTML->insertItem( "bibtex2html" );
            if ( settings->fileIO_ExporterHTML == BibTeX::FileExporterExternal::exporterBibTeX2HTML )
                m_comboBoxExportSystemHTML->setCurrentItem( m_comboBoxExportSystemHTML->count() - 1 );
        }
        if ( settings->external_bibconvAvailable )
        {
            m_comboBoxExportSystemHTML->insertItem( "bibconv" );
            if ( settings->fileIO_ExporterHTML == BibTeX::FileExporterExternal::exporterBibConv )
                m_comboBoxExportSystemHTML->setCurrentItem( m_comboBoxExportSystemHTML->count() - 1 );
        }

        if ( m_comboBoxExportSystemHTML->count() == 0 )
        {
            m_comboBoxExportSystemHTML->insertItem( i18n( "No exporter available" ) );
            m_comboBoxExportSystemHTML->setEnabled( FALSE );
        }

        m_checkBoxEmbedFiles->setChecked( settings->fileIO_EmbedFiles );
    }

    void SettingsFileIO::slotConfigChanged()
    {
        emit configChanged();
    }

    void SettingsFileIO::setupGUI()
    {
        QVBoxLayout * layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

        QGroupBox *groupBox = new QGroupBox( 2, Horizontal, i18n( "BibTeX Import and Export" ), this );
        layout->addWidget( groupBox );
        QLabel *label = new QLabel( i18n( "&Encoding:" ), groupBox );
        m_comboBoxEncoding = new QComboBox( groupBox );
        label->setBuddy( m_comboBoxEncoding );
        label = new QLabel( i18n( "Text &delimiters:" ), groupBox );
        m_comboBoxStringDelimiters = new QComboBox( groupBox );
        label->setBuddy( m_comboBoxStringDelimiters );
        label = new QLabel( i18n( "Keyword casing:" ), groupBox );
        m_comboBoxKeywordCasing = new QComboBox( groupBox );
        label->setBuddy( m_comboBoxKeywordCasing );
        label = new QLabel( i18n( "Protect title's casing:" ), groupBox );
        m_checkBoxProtectCasing = new QCheckBox( i18n( "Put curly brackets around" ), groupBox );
        QToolTip::add( m_checkBoxProtectCasing, i18n( "Put curly brackets around title and other selected fields." ) );
        QWhatsThis::add( m_checkBoxProtectCasing, i18n( "Put curly brackets around title and other selected fields to protect them from case changes in certain BibTeX styles." ) );

        groupBox = new QGroupBox( 2, Horizontal, i18n( "PDF, PostScript and Rich Text Format Export" ), this );
        layout->addWidget( groupBox );
        QWhatsThis::add( groupBox, i18n( "Select the layout of the resulting PDF, PostScript or Rich Text Format document when exporting a BibTeX file." ) );
        label = new QLabel( i18n( "&Language:" ), groupBox );
        m_comboBoxLanguage = new QComboBox( groupBox );
        label->setBuddy( m_comboBoxLanguage );
        label = new QLabel( i18n( "&Biblography style:" ), groupBox );
        m_comboBoxBibliographyStyle = new QComboBox( TRUE, groupBox );
        label->setBuddy( m_comboBoxBibliographyStyle );

        groupBox = new QGroupBox( 1, Horizontal, i18n( "PDF Export" ), this );
        layout->addWidget( groupBox );
        m_checkBoxEmbedFiles = new QCheckBox( i18n( "Embed local files into PDF if possible" ), groupBox );
        QWhatsThis::add( m_checkBoxEmbedFiles, i18n( "<qt>If checked, KBibTeX tries to embed all referenced files for the BibTeX entries into the exported PDF file.<br/>This requires that you have installed <tt>embedfile.sty</tt>.</qt>" ) );

        groupBox = new QGroupBox( 2, Horizontal, i18n( "Export Systems" ), this );
        layout->addWidget( groupBox );
        label = new QLabel( i18n( "HTML:" ), groupBox );
        m_comboBoxExportSystemHTML = new QComboBox( FALSE, groupBox );
        label->setBuddy( m_comboBoxExportSystemHTML );

        layout->addStretch( 1 );

        QStringList encList = QStringList::split( '|', Encodings );
        m_comboBoxEncoding->insertStringList( encList );

        QStringList delimiterList = QStringList::split( '|', StringDelimiters );
        m_comboBoxStringDelimiters->insertStringList( delimiterList );

        QStringList keywordCasingList = QStringList::split( '|', KeywordCasing );
        m_comboBoxKeywordCasing->insertStringList( keywordCasingList );

        QStringList langList = QStringList::split( '|', LanguagesI18N );
        m_comboBoxLanguage->insertStringList( langList );

        QStringList bibList = QStringList::split( '|', BibliographyStyles );
        bibList.sort();
        m_comboBoxBibliographyStyle->insertStringList( bibList );

        connect( m_comboBoxEncoding, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxStringDelimiters, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxKeywordCasing, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxLanguage, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxBibliographyStyle, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
        connect( m_comboBoxExportSystemHTML, SIGNAL( activated( int ) ), this, SLOT( slotConfigChanged() ) );
    }

}
#include "settingsfileio.moc"
