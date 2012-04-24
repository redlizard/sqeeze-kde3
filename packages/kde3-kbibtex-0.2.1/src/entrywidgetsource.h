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
#ifndef KBIBTEXENTRYWIDGETSOURCE_H
#define KBIBTEXENTRYWIDGETSOURCE_H

#include <entrywidgettab.h>

class QTextEdit;

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class EntryWidgetSource : public EntryWidgetTab
    {
        Q_OBJECT
    public:
        EntryWidgetSource( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~EntryWidgetSource();

        bool isModified();

    public slots:
        void updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll );
        void apply( BibTeX::Entry *entry );
        void reset( BibTeX::Entry *entry );
        void updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings );

    private:
        QTextEdit *m_textEditSource;

        void setupGUI();
    };

}

#endif
