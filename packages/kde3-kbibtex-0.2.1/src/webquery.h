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
#ifndef KBIBTEXWEBQUERY_H
#define KBIBTEXWEBQUERY_H

#include <qobject.h>
#include <qstring.h>
#include <qlistview.h>

#include <kdialogbase.h>

#include "file.h"
#include "entry.h"

class QWidget;
class QFrame;
class QListView;
class QCheckBox;
class QSpinBox;
class KLineEdit;
class KComboBox;
class KProgressDialog;

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class WebQuery : public QObject
    {
        Q_OBJECT
    public:
        WebQuery( QWidget *parent );
        virtual ~WebQuery();

        virtual void query( const QString&, int ) { m_aborted = FALSE; };
        virtual void cancelQuery();

        virtual QString title() = 0;
        virtual QString disclaimer() = 0;
        virtual QString disclaimerURL() = 0;

    signals:
        void foundEntry( BibTeX::Entry* );
        void endSearch( bool );
        void setProgress( int progress );
        void setTotalSteps( int totalSteps );

    public slots:
        void slotCancelQuery();

    protected:
        bool m_aborted;
        QWidget *m_parent;
    };

    class WebQueryWizard: public QWidget
    {
        Q_OBJECT
    public:
        ~WebQueryWizard();

        static int execute( QWidget *parent, QValueList<BibTeX::Entry*> &results );

    signals:
        void changeButtonOK( bool state );

    protected:
        QFrame *m_toolbarContainer;
        QListView *m_listViewResults;
        KComboBox *m_comboBoxEngines;
        KLineEdit *m_lineEditQuery;
        QSpinBox *m_spinBoxMaxHits;
        KURLLabel *m_disclaimerLabel;
        QCheckBox *m_checkBoxImportAll;
        QValueList<WebQuery*> m_webQueries;
        KDialogBase *m_dlg;
        KProgressDialog *m_progressDialog;

        static KDialogBase *singletonDlg;
        static WebQueryWizard *singletonWiz;
        WebQueryWizard( KDialogBase *dlg, const char* name = 0 );

    private slots:
        void importEnableChanging( );
        void otherEngineSelected( int index );
        void startSearch();
        void endSearch( bool );
        void addHit( BibTeX::Entry* );
        void queryTextChanged( const QString& );

    private:
        QPushButton *m_pushButtonSearch;

        void setupGUI( );
    };

    class ResultsListViewItem: public QListViewItem
    {
    public:
        ResultsListViewItem( QListView * parent, BibTeX::Entry *entry );
        ~ResultsListViewItem();

        BibTeX::Entry* entry();
    private:
        BibTeX::Entry *m_entry;
    };
}

#endif
