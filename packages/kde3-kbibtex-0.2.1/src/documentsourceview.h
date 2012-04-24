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
#ifndef KBIBTEXDOCUMENTSOURCEVIEW_H
#define KBIBTEXDOCUMENTSOURCEVIEW_H

#include <qwidget.h>

#include <file.h>

#include <ktexteditor/editinterface.h>
#include <kate/view.h>
#include <kate/document.h>

namespace KBibTeX
{
    class DocumentWidget;

    /**
     * @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
     */
    class DocumentSourceView : public QWidget
    {
        Q_OBJECT
    public:
        DocumentSourceView( KBibTeX::DocumentWidget *docWidget, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~DocumentSourceView();

        bool setBibTeXFile( BibTeX::File *bibtexFile );
        BibTeX::File* getBibTeXFile( );

        void copy();
        void cut();
        bool paste();
        void insertLines( const QString& text, int line = -1 );
        void selectAll();

        void setReadOnly( bool isReadOnly );
        void setFactory( KXMLGUIFactory *factory, KXMLGUIClient *client );

    public slots:
        void configureEditor();
        void undo();
        void find();
        void findNext();

    signals:
        void modified();

    protected:
        virtual void focusInEvent( QFocusEvent* event );

    private:
        bool m_isReadOnly;
        KBibTeX::DocumentWidget *m_docWidget;
        BibTeX::File *m_bibtexFile;
        Kate::Document* m_document;
        Kate::View* m_view;
        KTextEditor::EditInterface* m_editInterface;
        QString m_lastSearchTerm;
        QStringList m_findHistory;

        void setupGUI();
        void readConfig();
        void writeConfig();
        void readConfig( KConfig *config );
        void writeConfig( KConfig *config );

        void search( int fromLine, int fromCol );
    };

}

#endif
