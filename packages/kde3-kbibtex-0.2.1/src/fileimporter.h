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
#ifndef BIBTEXFILEIMPORTER_H
#define BIBTEXFILEIMPORTER_H

#include <qobject.h>

class QIODevice;

namespace BibTeX
{
    class File;

    /**
    @author Thomas Fischer
    */
    class FileImporter : public QObject
    {
        Q_OBJECT
    public:
        FileImporter();
        ~FileImporter();

        File* load( const QString& text );
        virtual File* load( QIODevice *iodevice ) = 0;

        static bool guessCanDecode( const QString & )
        {
            return FALSE;
        };

    signals:
        void parseError( int errorId );
        void progress( int current, int total );

    public slots:
        virtual void cancel()
        {
            // nothing
        };
    };

}

#endif
