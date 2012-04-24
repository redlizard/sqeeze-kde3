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
#ifndef KBIBTEXWEBQUERYSPIRESHEP_H
#define KBIBTEXWEBQUERYSPIRESHEP_H

#include <webquery.h>

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class WebQuerySpiresHep : public WebQuery
    {
        Q_OBJECT
    public:
        WebQuerySpiresHep( QWidget* parent );
        virtual ~WebQuerySpiresHep();

        void query( const QString& searchTerm, int numberOfResults );
        void cancelQuery() { /* nothing */ };

        QString title();
        QString disclaimer();
        QString disclaimerURL();

    private:
        static const QString mirrorNames[];
        static const QString mirrorURLs[];
    };

}

#endif
