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
#ifndef KBIBTEXIDSUGGESTIONS_H
#define KBIBTEXIDSUGGESTIONS_H

#include <qstringlist.h>

#include <file.h>
#include <entry.h>

namespace KBibTeX
{
    struct IdSuggestionTokenInfo
    {
        unsigned int len;
        bool toLower;
        bool toUpper;
        QString inBetween;
    };

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class IdSuggestions
    {
    public:
        static QStringList createSuggestions( BibTeX::File *file, BibTeX::Entry *entry );
        static QString createDefaultSuggestion( BibTeX::File *file, BibTeX::Entry *entry );
        static QString formatStrToHuman( const QString& formatStr );
        static struct IdSuggestionTokenInfo evalToken( const QString& token );
        static QString formatId( BibTeX::Entry *entry, const QString& formatStr );

        static const QStringList smallWords;

    private:
        static const QRegExp unwantedChars;

        IdSuggestions();
        ~IdSuggestions();

        static QStringList authorsLastName( BibTeX::Entry *entry );
        static QString normalizeText( const QString& text );
        static int extractYear( BibTeX::Entry *entry );
        static QString extractTitle( BibTeX::Entry *entry );
        static QString translateToken( BibTeX::Entry *entry, const QString& token );
        static QString translateAuthorsToken( BibTeX::Entry *entry, const QString& token, bool onlyFirst );
        static QString translateTitleToken( BibTeX::Entry *entry, const QString& token, bool removeSmallWords );
    };

}

#endif
