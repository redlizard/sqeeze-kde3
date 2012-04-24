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
#ifndef KBIBTEXSIDEBAR_H
#define KBIBTEXSIDEBAR_H

#include <qwidget.h>
#include <qlistview.h>

#include <entryfield.h>
#include <file.h>

class QComboBox;
class QToolButton;
class KListView;
class QListViewItem;
class KPopupMenu;

namespace KBibTeX
{
    class SideBar : public QWidget
    {
        Q_OBJECT
    public:
        SideBar( bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~SideBar();

        void restoreState();
        void setReadOnly( bool isReadOnly );

    signals:
        void selected( const QString& text, BibTeX::Element::FilterType, BibTeX::EntryField::FieldType fieldType );
        void valueRenamed();

    public slots:
        void refreshLists( BibTeX::File *bibtexFile = NULL );

    protected:
        void resizeEvent( QResizeEvent *event );

    private:
        BibTeX::File *m_bibtexFile;
        QToolButton *m_buttonToggleShowAll;
        QComboBox *m_listTypeList;
        KListView *m_listAvailableItems;
        static const BibTeX::EntryField::FieldType importantFields[ 6 ];
        bool m_isReadOnly;
        KPopupMenu *m_popupMenu;
        QString m_oldText;

        void setupGUI();

    private slots:
        void prepareSearch( QListViewItem *item );
        void toggleShowAll( bool showAll );
        void showContextMenu( QListViewItem * item, const QPoint & pos );
        void startRenaming();
        void endRenaming( QListViewItem * item, int col, const QString & text );
    };

    class SideBarListViewItem: QListViewItem
    {
    public:
        SideBarListViewItem( QListView *parent, QString label1, QString label2 );

        int compare( QListViewItem* item, int column, bool ascending ) const;
    };

}

#endif
