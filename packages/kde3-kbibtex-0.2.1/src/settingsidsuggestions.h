/***************************************************************************
 *   Copyright (C) 2004-2005 by Thomas Fischer                             *
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
#ifndef KBIBTEXSETTINGSIDSUGGESTIONS_H
#define KBIBTEXSETTINGSIDSUGGESTIONS_H

#include <qwidget.h>

#include <klistview.h>

#include <entry.h>

class QRegExpValidator;
class QCheckBox;

class KPushButton;

namespace KBibTeX
{

    class IdSuggestionsListViewItem: public KListViewItem
    {
    public:
        IdSuggestionsListViewItem( KListView *, const QString&, BibTeX::Entry *example );
        IdSuggestionsListViewItem( KListView *, KListViewItem *, const QString&, BibTeX::Entry *example );
        void setText( int, const QString& );
        QString originalText() const;
        int width( const QFontMetrics &, const QListView *, int ) const;

    private:
        QString m_original;
        BibTeX::Entry *m_example;
        QString parse( const QString& );
    };

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class SettingsIdSuggestions : public QWidget
    {
        Q_OBJECT
    public:
        SettingsIdSuggestions( QWidget *parent = 0, const char *name = 0 );
        ~SettingsIdSuggestions();

        void applyData();
        void readData();

    signals:
        void configChanged();

    protected slots:
        void slotConfigChanged();

    private slots:
        void slotNewIdSuggestion();
        void slotEditIdSuggestion();
        void slotDeleteIdSuggestion();
        void slotMoveUpIdSuggestion();
        void slotMoveDownIdSuggestion();
        void slotToggleDefault();
        void updateGUI();

    private:
        KListView *m_listIdSuggestions;
        KPushButton *m_buttonNewIdSuggestion;
        KPushButton *m_buttonEditIdSuggestion;
        KPushButton *m_buttonDeleteIdSuggestion;
        KPushButton *m_buttonMoveUpIdSuggestion;
        KPushButton *m_buttonMoveDownIdSuggestion;
        KPushButton *m_buttonToggleDefault;
        QCheckBox *m_checkBoxForceDefault;
        QRegExpValidator *m_validator;
        QListViewItem *m_defaultSuggestionItem;
        static QString exampleBibTeXEntry;
        BibTeX::Entry *m_example;

        void setupGUI();
    };

}

#endif
