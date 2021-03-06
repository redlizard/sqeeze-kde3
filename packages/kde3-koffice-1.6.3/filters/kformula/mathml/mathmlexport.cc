/* This file is part of the KDE project
   Copyright (C) 2002 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>

#include <KoFilterChain.h>
#include <KoStoreDevice.h>

#include <kformuladocument.h>
#include <kformulacontainer.h>
#include <kformulamimesource.h>

#include "mathmlexport.h"


typedef KGenericFactory<MathMLExport, KoFilter> MathMLExportFactory;
K_EXPORT_COMPONENT_FACTORY( libkfomathmlexport, MathMLExportFactory( "kofficefilters" ) )


MathMLExport::MathMLExport( KoFilter */*parent*/, const char */*name*/, const QStringList& )
    : KoFilter()
{
}


KoFilter::ConversionStatus MathMLExport::convert( const QCString& from, const QCString& to )
{
    if ( to != "application/mathml+xml" || from != "application/x-kformula" )
        return KoFilter::NotImplemented;

    KoStoreDevice* in = m_chain->storageFile( "root", KoStore::Read );
    if(!in) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0, i18n( "Failed to read data." ), i18n( "MathML Export Error" ) );
        return KoFilter::StorageCreationError;
    }

    QDomDocument dom;
    if ( !dom.setContent( in, false ) ) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0, i18n( "Malformed XML data." ), i18n( "MathML Export Error" ) );
        return KoFilter::WrongFormat;
    }

    QFile f( m_chain->outputFile() );
    if( !f.open( IO_Truncate | IO_ReadWrite ) ) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0, i18n( "Failed to write file." ), i18n( "MathML Export Error" ) );
        return KoFilter::FileNotFound;
    }

    KFormula::DocumentWrapper* wrapper = new KFormula::DocumentWrapper( kapp->config(), 0 );
    KFormula::Document* doc = new KFormula::Document;
    wrapper->document( doc );
    KFormula::Container* formula = doc->createFormula();
    if ( !doc->loadXML( dom ) ) {
        kdError() << "Failed." << endl;
    }

    QTextStream stream(&f);
    stream.setEncoding( QTextStream::UnicodeUTF8 );
    formula->saveMathML( stream );
    f.close();

    delete formula;
    delete wrapper;

    return KoFilter::OK;
}

#include "mathmlexport.moc"
