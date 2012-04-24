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
#ifndef KBIBTEXENTRYWIDGETOTHER_H
#define KBIBTEXENTRYWIDGETOTHER_H

#include <qstringlist.h>

#include <kurl.h>

#include <entrywidgettab.h>

class QPushButton;
class KListView;
class QLineEdit;
class KBibTeX::FieldLineEdit;

namespace KBibTeX
{

    class ValueListViewItem;

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class EntryWidgetOther : public EntryWidgetTab
    {
        Q_OBJECT
    public:
        EntryWidgetOther( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~EntryWidgetOther();

        bool isModified();

    public slots:
        void updateGUI( BibTeX::Entry::EntryType /*entryType*/, bool enableAll );
        void apply( BibTeX::Entry *entry );
        void reset( BibTeX::Entry *entry );
        void updateWarnings( BibTeX::Entry::EntryType entryType, QListView * /*listViewWarnings*/ );

    private:
        bool m_isModified;

        QPushButton *m_pushButtonAdd;
        QPushButton *m_pushButtonDelete;
        QPushButton *m_pushButtonOpen;
        QLineEdit *m_lineEditKey;
        FieldLineEdit *m_fieldLineEditValue;
        KListView *m_listViewFields;
        QStringList m_deletedFields;
        KURL m_internalURL;

        void setupGUI();

    private slots:
        void updateGUI();
        void fieldExecute( QListViewItem * item );
        void addClicked();
        void deleteClicked();
        void openClicked();

    };

    class ValueListViewItem : public QListViewItem
    {
    public:
        ValueListViewItem( const QString& title, BibTeX::Value *value, QListView *parent );
        ~ValueListViewItem();

        const BibTeX::Value *value();
        QString title();
        void setValue( BibTeX::Value *value );
        void setTitle( const QString &title );

    private:
        QString m_title;
        BibTeX::Value *m_value;

        void setTexts( const QString& title );
    };

}

#endif
