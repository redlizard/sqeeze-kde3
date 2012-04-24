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
#ifndef KBIBTEXDOCUMENTWIDGET_H
#define KBIBTEXDOCUMENTWIDGET_H

#include <qtabwidget.h>

#include <kdirwatch.h>

#include <fileimporter.h>
#include <file.h>
#include <settings.h>

class QIODevice;
class QProgressDialog;
class QListViewItem;
class QWidget;
class QSplitter;

class KPopupMenu;
class KActionMenu;
class KURL;
class KTextEdit;
class KLineEdit;

namespace KBibTeX
{
    class DocumentListView;
    class DocumentListViewItem;
    class DocumentSourceView;
    class SearchBar;
    class SideBar;
    class WebQuery;

    class DocumentWidget : public QTabWidget
    {
        Q_OBJECT
    public:
        DocumentWidget( bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~DocumentWidget();

        bool open( const QString &fileName, bool mergeOnly = FALSE );
        bool open( QIODevice *iodevice, bool mergeOnly = FALSE, const QString &label = QString::null, BibTeX::FileImporter *importer = NULL );
        bool save( const QString &fileName, QStringList *errorLog = NULL );
        bool save( QIODevice *iodevice, BibTeX::File::FileFormat format = BibTeX::File::formatBibTeX, const QString &label = QString::null, QStringList *errorLog = NULL );

        bool newElement( const QString& elementType );
        void searchWebsites( const QString& searchURL, bool includeAuthor );
        void updateViews();
        void showStatistics();

        void setFactory( KXMLGUIFactory *factory, KXMLGUIClient *client );
        void updateViewDocumentMenu( );
        void updateAssignKeywords( );

        void deferredInitialization();
        void saveState();
        void restoreState();

    signals:
        void modified();
        void listViewSelectionChanged( int numSelected );
        void undoChanged( bool undoAvailable );

    public slots:
        void executeElement( DocumentListViewItem* );
        void editElement();
        void editElement( DocumentListViewItem* );
        void editElement( BibTeX::Element* );
        void deleteElements();
        void sendSelectedToLyx();
        void cutElements();
        void copyElements();
        void copyRefElements();
        void pasteElements();
        void selectAll();
        void setReadOnly( bool isReadOnly );
        void configureEditor();
        void undoEditor();
        void find();
        void findNext();
        void refreshBibTeXFile();
        void onlineSearch();

    private:
        enum EditMode
        {
            emList, emSource
        };

        KLineEdit *m_lineEditNewKeyword;
        bool m_isReadOnly;
        QSplitter *m_horSplitter;
        QSplitter *m_vertSplitter;
        QWidget *m_container;
        SearchBar *m_searchBar;
        DocumentListView *m_listViewElements;
        KTextEdit * m_preview;
        DocumentSourceView *m_sourceView;
        SideBar* m_sideBar;
        BibTeX::File *m_bibtexfile;
        QString m_filename;
        QProgressDialog *m_progressDialog;
        int m_newElementCounter;
        EditMode m_editMode;
        KActionMenu *m_viewDocumentActionMenu;
        KActionMenu *m_assignKeywordsActionMenu;
        QStringList m_viewDocumentActionMenuURLs;
        QMap<int, QString> m_assignKeywordsActionMenuURLs;
        KActionMenu *m_searchWebsitesActionMenu;
        KAction *m_actionEditCut;
        KAction *m_actionEditCopy;
        KAction *m_actionEditCopyRef;
        KAction *m_actionEditPaste;
        KAction *m_actionEditSelectAll;
        KAction *m_actionEditFind;
        KAction *m_actionEditFindNext;
        KDirWatch m_dirWatch;

        void setupGUI();

        void startProgress( const QString &label, QObject *progressFrom );
        void endProgress( QObject *progressFrom );

        void searchWebsites( BibTeX::Element *element, const QString& searchURL, bool includeAuthor );

        bool editElementDialog( BibTeX::Element *element );
        void updateFromGUI();
        QString nextNewEntry();

        KURL::List getEntryURLs( BibTeX::Entry *entry );

        BibTeX::FileImporter *fileImporterFactory( const QString &fileName );

    private slots:
        void slotShowProgress( int current, int total );
        void slotSelectionChanged();
        void slotPreviewElement( QListViewItem * item = NULL );
        void slotTabChanged( QWidget *tab );
        void slotModified();
        void slotFileGotDirty( const QString& path );
        void slotRefreshDirtyFile();
        void slotViewDocument( int id );
        void slotAssignKeywords( int id );
        void slotAddKeyword();
    };

}

#endif
