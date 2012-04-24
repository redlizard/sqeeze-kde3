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
#ifndef KBIBTEXVALUEWIDGET_H
#define KBIBTEXVALUEWIDGET_H

#include <qwidget.h>
#include <qdialog.h>

#include <value.h>
#include <entryfield.h>

class QListView;
class QPushButton;

namespace KBibTeX
{

    /**
    	@author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class ValueWidget : public QWidget
    {
        Q_OBJECT
    public:
        static QDialog::DialogCode execute( const QString& title, BibTeX::EntryField::FieldType fieldType,  BibTeX::Value *value, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );

    private slots:
        void apply( );
        void reset( );
        void slotAdd();
        void slotEdit();
        void slotToggle();
        void slotDelete();
        void slotUp();
        void slotDown();
        void updateGUI();

    private:
        BibTeX::EntryField::FieldType m_fieldType;
        BibTeX::Value *m_value;
        QListView *m_listViewValue;
        QPushButton *m_pushButtonAdd;
        QPushButton *m_pushButtonEdit;
        QPushButton *m_pushButtonToggle;
        QPushButton *m_pushButtonDelete;
        QPushButton *m_pushButtonUp;
        QPushButton *m_pushButtonDown;
        int m_newValueCounter;
        bool m_isReadOnly;

        ValueWidget( BibTeX::EntryField::FieldType fieldType, BibTeX::Value *value, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~ValueWidget();

        void setupGUI();
        void applyList( QStringList& list );
    };

}

#endif
