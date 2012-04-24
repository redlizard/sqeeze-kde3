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
#ifndef KBIBTEXDOCUMENTLISTVIEW_H
#define KBIBTEXDOCUMENTLISTVIEW_H

#include <klistview.h>
#include <kxmlguifactory.h>

#include <fileimporter.h>
#include <settings.h>
#include <file.h>

class QString;
class KURL;
class KActionMenu;
class KPopupMenu;

enum BibTeX::Element::FilterType;

namespace KBibTeX
{
    class DocumentWidget;
    class DocumentListViewItem;

    class DocumentListView : public KListView
    {
        Q_OBJECT
    public:
        DocumentListView( KBibTeX::DocumentWidget *docWidget, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~DocumentListView();

        void setFactory( KXMLGUIFactory *factory, KXMLGUIClient *client );
        void setViewShowColumnsMenu( KActionMenu *actionMenu );

        bool setBibTeXFile( BibTeX::File *bibtexFile );
        BibTeX::File* getBibTeXFile( );
        KBibTeX::DocumentListViewItem *insertItem( BibTeX::Element *item, KBibTeX::DocumentListViewItem *after );

        void deleteSelected();
        const QValueList<BibTeX::Element*> selectedItems();
//         void sendSelectedToLyx();

        void updateVisiblity();
        void updateVisiblity( KBibTeX::DocumentListViewItem *item );

        void deferredInitialization();
        void restoreState();

    signals:
        void modified();
        void executed( DocumentListViewItem* );

    public slots:
        void copy();
        void copyReferences();
        void cut();
        bool paste();
        void selectAll();
        void filter( const QString& text, BibTeX::Element::FilterType filterType, BibTeX::EntryField::FieldType fieldType );
        void setReadOnly( bool isReadOnly );
        void activateShowColumnMenu( int id );

        void insertItems( BibTeX::File *items, KBibTeX::DocumentListViewItem *after = NULL );
        void insertItem( BibTeX::Element *item );

    private slots:
        void slotDoubleClick( QListViewItem * item = NULL );
        void setSortingColumn( int column );
        void showBibtexListContextMenu( KListView *, QListViewItem *, const QPoint &p );
        void slotDropped( QDropEvent* event, QListViewItem* item );
        void saveColumnIndex();
        void restoreColumnIndex();
        void saveColumnWidths( int col = -1 );
        void restoreColumnWidths();
        void restoreSortingColumn();
        void makeNewItemsUnread();

    protected:
        //     void resizeEvent( QResizeEvent * e );
        //     void contentsDragEnterEvent( QDragEnterEvent *e );
        //         void dropEvent( QDropEvent *e );
        bool acceptDrag( QDropEvent* e ) const;
        //     void contentsDropEvent( QDropEvent *e );
        void startDrag();

        virtual bool eventFilter( QObject *watched, QEvent *e );

        virtual void keyPressEvent( QKeyEvent *e );

    private:
        static const int maxColumns = 256;
        KBibTeX::DocumentWidget *m_docWidget;
        BibTeX::File *m_bibtexFile;
        KPopupMenu *m_contextMenu;
        KPopupMenu *m_headerMenu;
        KActionMenu *m_columnVisibleAction;
        bool m_isReadOnly;
        QValueList<DocumentListViewItem*> m_unreadItems;

        QString m_filter;
        BibTeX::Element::FilterType m_filterType;
        BibTeX::EntryField::FieldType m_filterFieldType;
        int m_newElementCounter;

        void setItems();
        //         void appendBibTeXFile( BibTeX::File *bibtexfile );
        QString selectedToBibTeXText();
        QString selectedToBibTeXRefs();

        void showColumn( int col, int colWidth = 0xffff );
        void hideColumn( int col );
        void buildColumns();
    };
}

#endif
