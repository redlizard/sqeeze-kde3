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
#ifndef KBIBTEXSEARCHBAR_H
#define KBIBTEXSEARCHBAR_H

#include <qwidget.h>

#include <kxmlguifactory.h>

#include <documentlistview.h>
#include <entryfield.h>

class QTimer;
class KPushButton;
class KPopupMenu;
class KHistoryCombo;
class KComboBox;

namespace KBibTeX
{
    /**
    @author Thomas Fischer
    */
    class SearchBar : public QWidget
    {
        Q_OBJECT
    public:
        SearchBar( QWidget *parent = 0, const char *name = "search_bar" );
        ~SearchBar();

        void setFactory( KXMLGUIFactory *factory, KXMLGUIClient *client );
        void restoreState();

    signals:
        void doSearch( const QString &text, BibTeX::Element::FilterType, BibTeX::EntryField::FieldType fieldType );
        void onlineSearch();

    public slots:
        void setSearch( const QString&text, BibTeX::Element::FilterType, BibTeX::EntryField::FieldType fieldType );

    private:
        KPushButton *m_pushButtonClearSearchText;
        KHistoryCombo *m_comboboxFilter;
        QTimer *m_timerInput;
        KPushButton *m_pushButtonAddElement;
        KPushButton *m_pushButtonSearchOnlineDatabases;
        KComboBox *m_comboboxFilterType;
        KComboBox *m_comboboxRestrictTo;

        void setupGUI();

    private slots:
        void slotClear();
        void slotTimeout();
        void slotAnnounceDoSearch();
        void slotKeyPressed();
    };

}

#endif
