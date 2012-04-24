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
#ifndef KBIBTEXENTRYWIDGET_H
#define KBIBTEXENTRYWIDGET_H

#include <qwidget.h>
#include <qdialog.h>
#include <qvaluelist.h>
#include <qmap.h>

#include <kdialogbase.h>

#include <entry.h>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QTabWidget;
class QString;
class QTimer;
class QListView;

namespace KBibTeX
{
    class EntryWidgetTab;

    class EntryWidget : public QWidget
    {
        Q_OBJECT
    public:
        ~EntryWidget();

        static QDialog::DialogCode execute( BibTeX::Entry *entry, BibTeX::File *bibtexfile, bool isReadOnly, bool isNew, QWidget *parent = 0, const char *name = 0 );
        void closeEvent( QCloseEvent * e );

    protected:
        EntryWidget( BibTeX::File *bibtexfile, bool isReadOnly, bool isNew, QWidget *parent = 0, const char *name = 0 );
        void showEvent( QShowEvent * );

    private slots:
        void apply();
        void reset();
        void apply( BibTeX::Entry *entry );
        void reset( BibTeX::Entry *entry );
        void slotEnableAllFields();
        void slotForceDefaultIdSuggestion();
        void slotEntryTypeChanged();
        void slotCurrentPageChanged( QWidget* newPage );
        void warningsExecute( QListViewItem* item );
        void updateWarnings();
        void insertIdSuggestion( int id );
        void updateIdSuggestionsMenu();

    signals:
        void updateTabs( BibTeX::Entry::EntryType entryType, bool enableAll, bool isReadOnly );

    private:
        EntryWidget( BibTeX::Entry *entry, BibTeX::File *bibtexfile, bool isReadOnly, bool isNew, QWidget *parent = 0, const char *name = 0 );
        BibTeX::Entry *m_originalEntry;
        BibTeX::File *m_bibtexfile;
        bool m_isReadOnly;
        bool m_isNew;
        bool m_defaultIdSuggestionAvailable;

        QComboBox *m_comboBoxEntryType;
        QLineEdit *m_lineEditID;
        QPushButton *m_pushButtonForceDefaultIdSuggestion;
        QPushButton *m_pushButtonIdSuggestions;
        QPopupMenu *m_menuIdSuggestions;
        QMap<int, QString> m_idToSuggestion;
        QCheckBox *m_checkBoxEnableAll;
        QListView *m_listViewWarnings;
        QTabWidget *m_tabWidget;
        KBibTeX::EntryWidgetTab *m_sourcePage;
        QValueList<KBibTeX::EntryWidgetTab*> m_internalEntryWidgets;
        QWidget *m_lastPage;
        QTimer *m_updateWarningsTimer;

        void setupGUI( QWidget *parent, bool showWarnings = TRUE );
        void addTabWidgets();
        void addTabWidget( EntryWidgetTab *widget, const QString& title );
        void setupEntryTypes();
        void updateGUI();
        void internalApply( BibTeX::Entry *entry );
        void internalReset( BibTeX::Entry *entry );
    };

    class EntryWidgetDialog: public KDialogBase
    {
        Q_OBJECT
    public:
        EntryWidgetDialog( QWidget *parent = 0, const char *name = 0, bool modal = true, const QString &caption = QString::null, int buttonMask = Ok | Apply | Cancel ) : KDialogBase( parent, name, modal, caption, buttonMask ) {/* nothing */};
        ~EntryWidgetDialog() {/* nothing */};
        void setMainWidget( EntryWidget *widget )
        {
            KDialogBase::setMainWidget( widget );
            m_widget = widget;
        };

    protected:
        void closeEvent( QCloseEvent * e )
        {
            m_widget->closeEvent( e );
        }

    private:
        EntryWidget *m_widget;
    };

}

#endif
