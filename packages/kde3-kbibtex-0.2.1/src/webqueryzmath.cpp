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
#include <qfile.h>
#include <qapplication.h>
#include <qspinbox.h>

#include <klocale.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kurl.h>

#include <fileimporterbibtex.h>
#include "webqueryzmath.h"

namespace KBibTeX
{

    WebQueryZMATH::WebQueryZMATH( QWidget* parent ): WebQuery( parent )
    {
        // nothing
    }

    WebQueryZMATH::~WebQueryZMATH()
    {
        // nothing
    }

    QString WebQueryZMATH::title()
    {
        return i18n( "Zentralblatt MATH" );
    }

    QString WebQueryZMATH::disclaimer()
    {
        return i18n( "About Zentralblatt MATH" );
    }

    QString WebQueryZMATH::disclaimerURL()
    {
        return "http://www.zentralblatt-math.org/zmath/en/";
    }

    void WebQueryZMATH::query( const QString& searchTerm, int numberOfResults )
    {
        WebQuery::query( searchTerm, numberOfResults );

        emit setTotalSteps( 1 );

        QString term = searchTerm;
        KURL url = KURL( QString( "http://www.zentralblatt-math.org/zmath/en/search/?q=%1&count=%2&type=bibtex&format=short&display=all" ).arg( term.replace( "%", "%25" ).replace( " ", "+" ).replace( "?", "%3F" ).replace( "&", "%26" ) ).arg( numberOfResults ) );
        QString tmpFile;
        if ( KIO::NetAccess::download( url, tmpFile, m_parent ) && !m_aborted )
        {
            BibTeX::FileImporterBibTeX importer( FALSE );
            importer.setIgnoreComments( TRUE );
            QFile inputFile( tmpFile );
            inputFile.open( IO_ReadOnly );
            BibTeX::File *tmpBibFile = importer.load( &inputFile );
            inputFile.close();
            KIO::NetAccess::removeTempFile( tmpFile );

            emit setProgress( 1 );

            for ( BibTeX::File::ElementList::iterator it = tmpBibFile->begin(); it != tmpBibFile->end(); ++it )
            {
                BibTeX::Entry *entry = dynamic_cast<BibTeX::Entry*>( *it );
                if ( entry != NULL )
                    emit foundEntry( new BibTeX::Entry( entry ) );
            }

            delete tmpBibFile;
            emit endSearch( false );
        }
        else if ( !m_aborted )
        {
            QString message = KIO::NetAccess::lastErrorString();
            if ( message.isEmpty() )
                message.prepend( '\n' );
            message.prepend( QString( i18n( "Querying database '%1' failed." ) ).arg( title() ) );
            KMessageBox::error( m_parent, message );
            emit endSearch( true );
        }
        else
            emit endSearch( false );
    }

}
