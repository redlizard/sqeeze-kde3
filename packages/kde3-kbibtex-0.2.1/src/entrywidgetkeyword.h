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
#ifndef KBIBTEXENTRYWIDGETKEYWORD_H
#define KBIBTEXENTRYWIDGETKEYWORD_H

#include <entrywidgettab.h>

class QPushButton;
class KListView;

namespace KBibTeX
{
    class KeywordListViewItem: public QCheckListItem
    {
    public:
        KeywordListViewItem( KListView * parent, const QString & text, bool isGlobal );
        ~KeywordListViewItem();

        bool isGlobal()
        {
            return m_isGlobal;
        };
        void setGlobal( bool global );

    protected:
        bool m_isGlobal;
    };

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class EntryWidgetKeyword : public EntryWidgetTab
    {
        Q_OBJECT
    public:
        EntryWidgetKeyword( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~EntryWidgetKeyword();

        bool isModified();

    public slots:
        void updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll );
        void apply( BibTeX::Entry *entry );
        void reset( BibTeX::Entry *entry );
        void updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings );

    private slots:
        void slotSelectionChanged();
        void slotKeywordRenamed( QListViewItem * item, const QString & text, int col );
        void slotNewKeyword();
        void slotEditKeyword();
        void slotToggleGlobal();

    private:
        BibTeX::File *m_bibtexfile;
        QStringList m_availableKeywords;
        QStringList m_globalKeywords;
        QStringList m_fileKeywords;
        QStringList m_usedKeywords;
        bool m_isModified;
        KListView *m_listviewKeywords;
        QPushButton *m_buttonNew;
        QPushButton *m_buttonEdit;
        QPushButton *m_buttonToggleGlobal;
        int m_numKeywords;
        QString m_beforeRenaming;

        void setupGUI();
        void setListView();
        void readListView();
        bool editKeywords( QStringList *keywords );
    };

}

#endif
