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
#ifndef KBIBTEXSETTINGSUSERDEFINEDINPUT_H
#define KBIBTEXSETTINGSUSERDEFINEDINPUT_H

#include <qwidget.h>

class KListView;
class KPushButton;
class KDialogBase;

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class SettingsUserDefinedInput : public QWidget
    {
        Q_OBJECT
    public:
        SettingsUserDefinedInput( QWidget *parent = 0, const char *name = 0 );
        ~SettingsUserDefinedInput();

        void applyData();
        void readData();

    signals:
        void configChanged();

    private slots:
        void slotNewField();
        void slotEditField();
        void slotDeleteField();
        void slotMoveUpField();
        void slotMoveDownField();
        void updateGUI();
        void updateDialogGUI();

    private:
        KListView *m_listFields;
        KPushButton *m_buttonNewField;
        KPushButton *m_buttonEditField;
        KPushButton *m_buttonDeleteField;
        KPushButton *m_buttonMoveUpField;
        KPushButton *m_buttonMoveDownField;
        KDialogBase *fieldDlg;
        KLineEdit *m_lineEditLabel;
        KLineEdit *m_lineEditName;

        void setupGUI();
        void fieldDialog( QListViewItem * item = NULL );
    };

}

#endif
