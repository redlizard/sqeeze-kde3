/***************************************************************************
 *   Copyright (C) 2004-2005 by Thomas Fischer                             *
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
#include <qlabel.h>
#include <qframe.h>
#include <qbuffer.h>

#include <ktextedit.h>
#include <kdialog.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobalsettings.h>

#include <macro.h>
#include <comment.h>
#include <preamble.h>
#include <settings.h>
#include <fileexporterbibtex.h>
#include "mergeentries.h"

namespace KBibTeX
{

    MergeEntries::MergeEntries( )
            : QObject( )
    {
        createMergeWindow();
    }

    MergeEntries::~MergeEntries()
    {
        delete m_mergeWindow;
    }

    void MergeEntries::createMergeWindow()
    {
        m_mergeWindow = new KDialog( NULL, "m_mergeWindow", TRUE );
        QGridLayout *layout = new QGridLayout( m_mergeWindow, 6, 6, KDialog::marginHint(), KDialog::spacingHint() );
        m_mergeWindow->setCaption( i18n( "Merge BibTeX elements" ) );
        m_status = mcCancel;

        m_originalText = new KTextEdit( m_mergeWindow );
        m_originalText->setReadOnly( TRUE );
        m_originalText->setAlignment( Qt::AlignTop && Qt::AlignLeft );
        layout->addMultiCellWidget( m_originalText, 1, 1, 0, 5 );
        QLabel *label = new QLabel( i18n( "Original element:" ), m_mergeWindow );
        layout->addMultiCellWidget( label, 0, 0, 0, 5 );

        m_insertedText = new KTextEdit( m_mergeWindow );
        m_insertedText->setReadOnly( TRUE );
        m_insertedText->setAlignment( Qt::AlignTop && Qt::AlignLeft );
        layout->addMultiCellWidget( m_insertedText, 3, 3, 0, 5 );
        label = new QLabel( i18n( "Inserted element:" ), m_mergeWindow );
        layout->addMultiCellWidget( label, 2, 2, 0, 5 );

        m_originalText->setFont( KGlobalSettings::fixedFont() );
        m_insertedText->setFont( KGlobalSettings::fixedFont() );

        QFrame *frame = new QFrame( m_mergeWindow );
        frame->setFrameShape( QFrame::HLine );
        layout->addMultiCellWidget( frame, 4, 4 , 0, 5 );

        KPushButton *buttonKeepOriginal = new KPushButton( SmallIcon( "up" ), i18n( "Keep Original" ), m_mergeWindow );
        layout->addWidget( buttonKeepOriginal, 5, 1 );

        KPushButton *buttonUseInserted = new KPushButton( SmallIcon( "down" ), i18n( "Use Inserted" ), m_mergeWindow );
        layout->addWidget( buttonUseInserted, 5, 2 );

        KPushButton *buttonKeepBoth = new KPushButton( /*SmallIcon( "merge" ),*/ i18n( "Rename Inserted" ), m_mergeWindow );
        layout->addWidget( buttonKeepBoth, 5, 3 );

        m_buttonMerge = new KPushButton( /*SmallIcon( "merge" ),*/ i18n( "Merge" ), m_mergeWindow );
        layout->addWidget( m_buttonMerge, 5, 4 );

        KPushButton *buttonCancel = new KPushButton( SmallIcon( "cancel" ), i18n( "Cancel" ), m_mergeWindow );
        layout->addWidget( buttonCancel, 5, 5 );

        connect( buttonKeepOriginal, SIGNAL( clicked() ), this, SLOT( slotKeepOriginal() ) );
        connect( buttonUseInserted, SIGNAL( clicked() ), this, SLOT( slotUseInserted() ) );
        connect( buttonKeepBoth, SIGNAL( clicked() ), this, SLOT( slotKeepBoth() ) );
        connect( m_buttonMerge, SIGNAL( clicked() ), this, SLOT( slotMerge() ) );
        connect( buttonKeepOriginal, SIGNAL( clicked() ), m_mergeWindow, SLOT( accept() ) );
        connect( buttonUseInserted, SIGNAL( clicked() ), m_mergeWindow, SLOT( accept() ) );
        connect( buttonKeepBoth, SIGNAL( clicked() ), m_mergeWindow, SLOT( accept() ) );
        connect( m_buttonMerge, SIGNAL( clicked() ), m_mergeWindow, SLOT( accept() ) );
        connect( buttonCancel, SIGNAL( clicked() ), m_mergeWindow, SLOT( reject() ) );
    }


    QString MergeEntries::elementText( BibTeX::Element *element )
    {
        Settings * settings = Settings::self();

        QBuffer buffer;
        BibTeX::FileExporterBibTeX exporter;
        exporter.setEncoding(( BibTeX::File::Encoding ) settings->fileIO_Encoding );
        exporter.setStringDelimiter( settings->fileIO_BibtexStringOpenDelimiter, settings->fileIO_BibtexStringCloseDelimiter );
        exporter.setKeywordCasing( settings->fileIO_KeywordCasing );
        exporter.setEnclosingCurlyBrackets( settings->fileIO_EnclosingCurlyBrackets );

        buffer.open( IO_WriteOnly );
        exporter.save( &buffer, element );
        buffer.close();

        buffer.open( IO_ReadOnly );
        QTextStream ts( &buffer );
        ts.setEncoding( QTextStream::UnicodeUTF8 );
        QString result = ts.read();
        buffer.close();

        return result;
    }

    MergeEntries::MergeChoice MergeEntries::show( BibTeX::Element *original, BibTeX::Element *inserted )
    {
        m_buttonMerge->setEnabled( dynamic_cast<BibTeX::Entry*>( original ) != NULL );
        m_originalText->setText( elementText( original ) );
        m_insertedText->setText( elementText( inserted ) );
        if ( m_mergeWindow->exec() == QDialog::Accepted )
            return m_status;
        else
            return mcCancel;
    }

    void MergeEntries::slotKeepOriginal()
    {
        m_status = mcKeepOriginal;
    }

    void MergeEntries::slotUseInserted()
    {
        m_status = mcUseInserted;
    }

    void MergeEntries::slotKeepBoth()
    {
        m_status = mcKeepBoth;
    }

    void MergeEntries::slotMerge()
    {
        m_status = mcMerge;
    }

    void MergeEntries::mergeBibTeXFiles( BibTeX::File *originalFile, BibTeX::File *insertedFile )
    {
        MergeEntries *mergeEntries = new MergeEntries();
        BibTeX::File::ElementList toBeAppended;

        for ( BibTeX::File::ElementList::iterator it = insertedFile->begin(); it != insertedFile->end(); ++it )
        {
            BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *it );
            if ( entry != NULL )
            {
                BibTeX::Entry *originalEntry = dynamic_cast<BibTeX::Entry*>( originalFile->containsKey( entry->id() ) );
                if ( originalEntry != NULL )
                {
                    if ( !mergeBibTeXEntries( originalFile, mergeEntries, originalEntry, entry ) )
                        break;
                }
                else
                    toBeAppended.append( entry );
            }
            else
            {
                BibTeX::Macro *macro = dynamic_cast<BibTeX::Macro*>( *it );
                if ( macro != NULL )
                {
                    BibTeX::Macro *originalMacro = dynamic_cast<BibTeX::Macro*>( originalFile->containsKey( macro->key() ) );
                    if ( originalMacro != NULL )
                    {
                        if ( !mergeBibTeXMacros( originalFile, mergeEntries, originalMacro, macro ) )
                            break;
                    }
                    else
                        toBeAppended.append( macro );
                }
                else
                {
                    BibTeX::Comment *comment = dynamic_cast<BibTeX::Comment*>( *it );
                    if ( comment != NULL )
                        toBeAppended.append( comment );
                    else
                    {
                        BibTeX::Preamble *preamble = dynamic_cast<BibTeX::Preamble*>( *it );
                        if ( preamble != NULL )
                            toBeAppended.append( preamble );
                    }
                }
            }
        }

        for ( BibTeX::File::ElementList::iterator it = toBeAppended.begin(); it != toBeAppended.end(); ++it )
        {
            originalFile->appendElement(( *it )->clone() );
        }

        delete mergeEntries;
    }

    bool MergeEntries::mergeBibTeXEntries( BibTeX::File *bibTeXFile, MergeEntries *mergeEntries, BibTeX::Entry *originalEntry, BibTeX::Entry *insertedEntry )
    {
        MergeChoice choice = mergeEntries->show( originalEntry, insertedEntry );

        switch ( choice )
        {
        case mcKeepOriginal: return TRUE;
        case mcUseInserted:
            originalEntry->copyFrom( insertedEntry );
            return TRUE;
        case mcMerge:
            originalEntry->merge( insertedEntry );
            return TRUE;
        case mcKeepBoth:
            {
                BibTeX::Entry *newIns = new BibTeX::Entry( insertedEntry );
                newIns->setId( newIns->id().append( i18n( "May only contain ASCII characters, in case of doubt keep English form", "_new" ) ) );
                bibTeXFile->appendElement( newIns, originalEntry );
            }
            return TRUE;
        default:
            return FALSE;
        }
    }

    bool MergeEntries::mergeBibTeXMacros( BibTeX::File *bibTeXFile, MergeEntries *mergeEntries, BibTeX::Macro *originalMacro, BibTeX::Macro *insertedMacro )
    {
        MergeChoice choice = mergeEntries->show( originalMacro, insertedMacro );

        switch ( choice )
        {
        case mcKeepOriginal: return TRUE;
        case mcUseInserted:
            originalMacro->copyFrom( insertedMacro );
            return TRUE;
        case mcMerge: /** not implemented */
            return TRUE;
        case mcKeepBoth:
            {
                BibTeX::Macro *newIns = new BibTeX::Macro( insertedMacro );
                newIns->setKey( newIns->key().append( i18n( "May only contain ASCII characters, in case of doubt keep English form", "_new" ) ) );
                bibTeXFile->appendElement( newIns, originalMacro );
            }
            return TRUE;
        default:
            return FALSE;
        }
    }

}
#include "mergeentries.moc"
