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
#ifndef SETTINGSEDITING_H
#define SETTINGSEDITING_H

#include <qwidget.h>

class QCheckBox;
class QPushButton;
class QComboBox;

class KLineEdit;

namespace KBibTeX
{
    class SettingsEditing : public QWidget
    {
        Q_OBJECT
    public:
        SettingsEditing( QWidget *parent = 0, const char *name = 0 );
        ~SettingsEditing();

        void applyData();
        void readData();

    signals:
        void configChanged();

    protected slots:
        void slotConfigChanged();
        void slotSelectSpecialFont();
        void slotSelectDocumentSearchPath();

    private:
        QCheckBox *m_checkBoxEnableAllFields;
        QCheckBox *m_checkBoxSearchBarClearField;
        QComboBox *m_comboBoxSortingColumn;
        QComboBox *m_comboBoxSortingOrder;
        QComboBox *m_comboBoxDoubleClickAction;
        QComboBox *m_comboBoxDragAction;
        QCheckBox *m_checkBoxUseSpecialFont;
        QPushButton *m_pushButtonSpecialFont;
        QFont m_specialFont;
        QComboBox *m_comboBoxNameOrder;
        KLineEdit *m_lineEditDocumentSearchPath;

        void updateFontData();
    };
}

#endif
