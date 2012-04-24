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
#ifndef KBIBTEXENTRYWIDGETTAB_H
#define KBIBTEXENTRYWIDGETTAB_H

#include <qwidget.h>

#include <entry.h>
#include <file.h>

class QListView;

namespace KBibTeX
{
    class FieldLineEdit;

    class EntryWidgetTab : public QWidget
    {
        Q_OBJECT
    public:
        EntryWidgetTab( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~EntryWidgetTab();

        virtual bool isModified() = 0;

    public slots:
        virtual void updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll ) = 0;
        virtual void apply( BibTeX::Entry *entry ) = 0;
        virtual void reset( BibTeX::Entry *entry ) = 0;
        virtual void updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings ) = 0;

    protected:
        void setValue( BibTeX::Entry *entry, const BibTeX::EntryField::FieldType fieldType, BibTeX::Value *value );
        void addMissingWarning( BibTeX::Entry::EntryType entryType, BibTeX::EntryField::FieldType fieldType, const QString& label, bool valid, QWidget *widget, QListView *listView );
        void addCrossRefInfo( const QString& label, QWidget *widget, QListView *listView );
        void addFieldLineEditWarning( FieldLineEdit *fieldLineEdit, const QString& label, QListView *listView );

        bool m_isReadOnly;
        static BibTeX::Entry *m_crossRefEntry;
        BibTeX::File *m_bibtexfile;
    };

}

#endif
